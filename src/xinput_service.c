/*
 * MIT License
 *
 * Unix XInput Gamepad interface implementation
 *
 * Copyright (c) 2016-2017 Eric Diaz Fernandez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "config.h"
#include "xinput_settings.h"

#include <pthread.h>
#include <stdlib.h>

#include "xinput.h"
#include "debug.h"
#include "tools.h"
#include "xinput_service.h"
#include "xinput_gamepad.h"

#if XINPUT_USES_SEMAPHORE_MUTEX
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#endif

#ifndef HAVE_MQ_OPEN
#undef XINPUT_USES_MQUEUE
#endif

#if XINPUT_USES_MQUEUE
#include <mqueue.h>
#endif

#if HAVE_WINE
#include "wine/debug.h"
#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <signal.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <sys/shm.h>
#include <fcntl.h>           /* For O_* constants */
#include <stdint.h>
#include <errno.h>
#include <unistd.h>


#if HAVE_LINUX_INPUT_H
#include "linux_evdev/xinput_linux_evdev.h"
#else
/* no supported driver */
#define xinput_driver_initialize()
#define xinput_driver_probe() 0
#define xinput_driver_get_device(a) NULL
#define xinput_driver_device_close(a)
#define xinput_driver_finalize()
#endif

#if HAVE_WINE
WINE_DEFAULT_DEBUG_CHANNEL(xinput);
#endif

struct xinput_service_thread_args
{
    xinput_gamepad_state* xgs;
    xinput_gamepad_device* device;
    pthread_t tid;
    int slot;
};

typedef struct xinput_service_thread_args xinput_service_thread_args;

static xinput_service_thread_args xinput_service_thread_parameter[XUSER_MAX_COUNT];

static xinput_shared_gamepad_state* service_shared = NULL;

static volatile int xinput_service_idle_strikes = XINPUT_IDLE_CLIENT_STRIKES;

#if !XINPUT_RUNDLL
static pthread_t service_thread_id = 0;
#endif
static int service_fd = -1;
#if XINPUT_USES_SEMAPHORE_MUTEX
static sem_t*  service_sem = SEM_FAILED;
#endif


#if XINPUT_USES_MQUEUE

static mqd_t service_mq = MQD_INVALID;
static pthread_t xinput_service_rumble_thread_id = 0;

/*
 * Rumble is handled almost entierely separately
 */

static BOOL xinput_service_queue_create(void)
{
    mqd_t mq;
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(xinput_gamepad_vibration);

    mq = mq_open(SERVICE_MSG_NAME, O_RDWR|O_CREAT|O_EXCL, 0666, &attr);
    if(mq == MQD_INVALID)
    {
        int err = errno;
        if(err != EEXIST)
        {
            return FALSE;
        }

        mq_unlink(SERVICE_MSG_NAME);
        mq = mq_open(SERVICE_MSG_NAME, O_RDONLY|O_CREAT|O_EXCL, 0666, &attr);
        if(mq == MQD_INVALID)
        {
            err = errno;
            TRACE("could not create message queue: %s\n", strerror(err));

            return FALSE;
        }
    }

    mq_getattr(mq, &attr);

    service_mq = mq;

    return TRUE;
}

static void xinput_service_queue_destroy(void)
{
    if(service_mq != MQD_INVALID)
    {
        mq_close(service_mq);
        service_mq = MQD_INVALID;
        mq_unlink(SERVICE_MSG_NAME);
    }
}

static void* xinput_service_rumble_thread(void* args_)
{
    xinput_gamepad_device* device;
    xinput_gamepad_vibration vibration_message;
    unsigned int priority = 0;
    (void)args_;

    for(;;)
    {
        ssize_t len = mq_receive(service_mq, (char*)&vibration_message, sizeof(vibration_message), &priority);
        if(len == -1)
        {
            int err = errno;
            switch(err)
            {
                case EINTR:
                case EAGAIN:
                /*case ETIMEDOUT:*/
                    continue;
                default:
                    break;
            }

            TRACE("receive returned an error: %s\n", strerror(err));

            /**
             * TODO: restart the service
             */

            break;
        }

        TRACE("rumble %i: [%4x, %4x]\n", vibration_message.index,
                vibration_message.vibration.wLeftMotorSpeed,
                vibration_message.vibration.wRightMotorSpeed);

        /*
         * send rumble
         */

        if((device = xinput_linux_evdev_get_device(vibration_message.index)) != NULL)
        {
            device->vtbl->rumble(device, &vibration_message.vibration);
        }
    }
    return NULL;
}

#endif

static BOOL xinput_service_lock_create(void)
{
#if XINPUT_USES_SEMAPHORE_MUTEX
    service_sem = sem_open(SERVICE_SEM_NAME, O_RDWR|O_CREAT|O_EXCL, 0644, 1);
    if(service_sem == SEM_FAILED)
    {
        int err = errno;
        if(err != EEXIST)
        {
            return FALSE;
        }

        sem_unlink(SERVICE_SEM_NAME);

        service_sem = sem_open(SERVICE_SEM_NAME, O_CREAT|O_EXCL, 0644, 1);
        if(service_sem == SEM_FAILED)
        {
            err = errno;
            TRACE("could not create semaphore: %s\n", strerror(err));

            return FALSE;
        }
    }
#endif

    return TRUE;
}

static void xinput_service_lock_destroy(void)
{
#if XINPUT_USES_SEMAPHORE_MUTEX
    if(service_sem != SEM_FAILED)
    {
        sem_close(service_sem);
        service_sem = SEM_FAILED;
        sem_unlink(SERVICE_SEM_NAME);
    }
#endif
}

static BOOL xinput_service_lock(void)
{
#if XINPUT_USES_SEMAPHORE_MUTEX
#ifdef __USE_XOPEN2K
    struct timespec to;
#else
    int64_t until = timeus() + 1000000LL;
#endif

    for(;;)
    {
#ifdef __USE_XOPEN2K
        int64_t now = timeus();
        now += 1000000LL;
        to.tv_sec = now / 1000000LL;
        to.tv_nsec = (now % 1000000LL) *1000LL;

        if(sem_timedwait(service_sem, &to) == 0)
        {
            return TRUE;
        }
#else
        if(sem_trywait(service_sem) == 0)
        {
            return TRUE;
        }
        else if(timeus() < until)
        {
            usleep(500);   /* 0.5 ms sleep */
        }
#endif
        else
        {
            int err = errno;

            if(err != EINTR)
            {
                /*
                 * ETIMEOUT
                 * starvation or a client died keeping the lock
                 *
                 * TODO: destroy the service, including the semaphore,
                 * and let it restart
                 */

                TRACE("semaphore seems stuck\n");

                return FALSE;
            }
        }
    }
#else
    return TRUE;
#endif
}

static void xinput_service_unlock(void)
{
#if XINPUT_USES_SEMAPHORE_MUTEX
    sem_post(service_sem);
#endif
}

static void* xinput_service_gamepad_reader_thread(void* args_)
{
    xinput_service_thread_args* args = (xinput_service_thread_args*)args_;

    xinput_gamepad_state* xgs = args->xgs;

    int err;
    pid_t pid = getpid();

    TRACE("BEGIN %i ==========================================\n", args->slot);

    if(xinput_service_lock())
    {
        xgs->connected = TRUE;
        xinput_service_unlock();
    }

    for(;;)
    {
        if((err = args->device->vtbl->read(args->device)) == 0)
        {
            if(xinput_service_lock())
            {
                /* copy the data */
                args->device->vtbl->update(args->device, &xgs->gamepad, &xgs->vibration);
                ++xgs->dwPacketNumber;
                xinput_service_unlock();
            }
            else
            {
                /* semaphore stuck ... ? */
            }
        }
        else
        {
            /* ENODEV ... or something */
            TRACE("%6i: failed to read %i: %i: %s\n", pid, args->slot, err, strerror(err));

            break;
        }
#if XINPUT_TRACE_DEVICE_READER_THREAD
        TRACE("%6i | %9i | %04hx,%04hx %04hx,%04hx %02hhx %02hhx %04hx\n",
                pid,
                xgs->dwPacketNumber,
                xgs->gamepad.sThumbLX,
                xgs->gamepad.sThumbLY,
                xgs->gamepad.sThumbRX,
                xgs->gamepad.sThumbRY,
                xgs->gamepad.bLeftTrigger,
                xgs->gamepad.bRightTrigger,
                xgs->gamepad.wButtons
                );
#endif
    } /*  for */

    xinput_driver_device_close(args->slot);
    args->device = NULL;

    if(xinput_service_lock())
    {
        xgs->connected = FALSE;
        xinput_service_unlock();
    }

    TRACE("END %i ============================================\n", args->slot);

    return NULL;
}

static void xinput_service_gamepad_probe(void)
{
    uint32_t mask = xinput_driver_probe();

    if(mask == 0)
    {
        return;
    }

    for(int slot = 0; slot < XUSER_MAX_COUNT; ++slot)
    {
        if(mask & (1 << slot))
        {
            int ret;

            /* new gamepad */

            xinput_gamepad_device* device = xinput_driver_get_device(slot);
            xinput_gamepad_state* xgs = &service_shared->state[slot];

            TRACE("starting thread for gamepad %i\n", slot);

            xinput_service_thread_parameter[slot].device = device;
            xinput_service_thread_parameter[slot].slot = slot;
            xinput_service_thread_parameter[slot].xgs = xgs;

            TRACE("create\n");

            ret = pthread_create(&xinput_service_thread_parameter[slot].tid, NULL, xinput_service_gamepad_reader_thread, &xinput_service_thread_parameter[slot]);

            if(ret != 0)
            {
                xinput_driver_device_close(slot);

                TRACE("pthread_create returned %i: %s\n", ret, strerror(ret));
            }
        }
    }

#if XINPUT_TRACE_DEVICE_DETECTION
    TRACE("devices probed\n");
#endif
}

static void* xinput_service_thread(void* args_)
{
    int allalone = 0;
    (void)args_;
    for(;;)
    {
        if(xinput_service_idle_strikes > 0)
        {
            int64_t now = timeus();

            if(now >= service_shared->poke_us)
            {
                if((now - service_shared->poke_us) <= (XINPUT_DEVICE_PROBE_PERIOD_S * 1000000LL))
                {
                    int dt = (int)(now - service_shared->poke_us);
                    TRACE("client was active %ius ago\n", dt);
                    service_shared->poke_us = now;
                    allalone = 0;
                }
                else
                {
                    int dt = (int)(now - service_shared->poke_us);

                    ++allalone;

                    TRACE("client has not been active for %ius (strike %i)\n", dt, allalone);

                    if(allalone >= xinput_service_idle_strikes)
                    {
                        TRACE("no active client\n");
                        /* no sign of life for a while : give up */
                        break;
                    }
                }
            }
            else
            {
                TRACE("weird clock value\n");
                /* weird, let's reset it */
                service_shared->poke_us = now;
            }
        }

        xinput_service_gamepad_probe();
        sleep(XINPUT_DEVICE_PROBE_PERIOD_S);
    }
    return NULL;
}

void xinput_service_destroy(void)
{
    TRACE("destroying\n");

#if XINPUT_USES_MQUEUE
    xinput_service_queue_destroy();
#endif

    xinput_service_lock_destroy();

    if(service_shared != NULL)
    {
        for(int slot = 0; slot < XUSER_MAX_COUNT; ++slot)
        {
            xinput_gamepad_device* device;

            if(xinput_service_thread_parameter[slot].tid != 0)
            {
                TRACE("canceling device thread %i\n", slot);

                pthread_cancel(xinput_service_thread_parameter[slot].tid);
                xinput_service_thread_parameter[slot].tid = 0;
            }

            if((device = xinput_service_thread_parameter[slot].device) != NULL)
            {
                xinput_service_thread_parameter[slot].device = NULL;
                xinput_driver_device_close(slot);
            }
        }

#if XINPUT_USES_MQUEUE
        if(xinput_service_rumble_thread_id != 0)
        {
            pthread_cancel(xinput_service_rumble_thread_id);
            xinput_service_rumble_thread_id = 0;
        }
#endif

        service_shared->master_pid = XINPUT_OWNER_BROKEN;
        TRACE("destroying '%s'\n", SERVICE_SHM_NAME);
        shm_unlink(SERVICE_SHM_NAME);

        munmap(service_shared, sizeof(xinput_shared_gamepad_state));
        service_shared = NULL;
    }

    if(service_fd != -1)
    {
        close_ex(service_fd);
        service_fd = -1;
    }

    xinput_driver_finalize();
}

int xinput_service_poke(void)
{
    xinput_shared_gamepad_state* state;
    pid_t pid;
    int ret;
    int fd;
    BOOL dead;

    TRACE("poke\n");

    ret = shm_open(SERVICE_SHM_NAME, O_RDWR, 0666);

    if(ret < 0)
    {
        ret = errno; /* ENOENT : should be created */
        return ret;
    }

    fd = ret;

    state = (xinput_shared_gamepad_state*)mmap(
                NULL,
                sizeof(xinput_shared_gamepad_state),
                PROT_READ|PROT_WRITE, MAP_SHARED,
                fd,
                0);

    if(state == NULL)
    {
        ret = errno;
        close_ex(fd);
        TRACE("could not map: %s\n", strerror(ret));
        return ret;
    }

    for(int i = 5; i >= 0; --i)
    {
        memdump(state, sizeof(xinput_shared_gamepad_state));

        pid = state->master_pid;
        if(pid != 0)
        {
            TRACE("owned by pid %i\n", pid);
            break;
        }

        /*  this will only happen if the DLL has been loaded twice "at the same time" */

        usleep(XINPUT_OWNER_PROBE_PERIOD_US);
    }

    dead = (pid == 0) || (pid == XINPUT_OWNER_BROKEN);

    if(!dead)
    {
        dead = kill(pid, 0) < 0;
        if(dead)
        {
            int ret = errno;
            TRACE("could not probe owner on pid: %s\n", strerror(ret));
        }
    }

    if(dead)
    {
        TRACE("dead\n");

        /*  dead */
        /*  mark it as dead, delete it, close it restart it */
        state->master_pid = XINPUT_OWNER_BROKEN; /*  mark broken */
        TRACE("destroying '%s'\n", SERVICE_SHM_NAME);
#if XINPUT_USES_MQUEUE
        mq_unlink(SERVICE_MSG_NAME);
#endif
#if XINPUT_USES_SEMAPHORE_MUTEX
        sem_unlink(SERVICE_SEM_NAME);
#endif
        shm_unlink(SERVICE_SHM_NAME);

        munmap(state, sizeof(xinput_shared_gamepad_state));
        state = NULL;

        close_ex(fd);
        fd = -1;

        /*  create a new server */

        return ENOENT;
    }
    else
    {
        /* only the clients are writing, and synchronization does not matter */
        state->poke_us = timeus();
        TRACE("alive\n");
        /*  alive */
        return EEXIST;
    }
}

int xinput_service_create(void)
{
    xinput_shared_gamepad_state* state;
    int ret;
    int fd;

    TRACE("creating\n");

    ret = shm_open(SERVICE_SHM_NAME, O_RDWR|O_CREAT|O_EXCL, 0666);

    if(ret < 0)
    {
        ret = errno;

        TRACE("could not create: %s\n", strerror(ret));

        return ret;
    }

    TRACE("exclusively created shared memory named '%s'\n", SERVICE_SHM_NAME);

    fd = ret;

    while(ftruncate(fd, sizeof(xinput_shared_gamepad_state)) < 0)
    {
        ret = errno;

        TRACE("could not set size: %s\n", strerror(ret));

        if(ret != EINTR)
        {
            TRACE("destroying '%s'\n", SERVICE_SHM_NAME);
            shm_unlink(SERVICE_SHM_NAME);
            close_ex(fd);
            return ret;
        }
    }

    state = (xinput_shared_gamepad_state*)mmap(
                    NULL,
                    sizeof(xinput_shared_gamepad_state),
                    PROT_READ|PROT_WRITE,
                    MAP_SHARED,
                    fd,
                    0);

    if(state == NULL)
    {
        ret = errno;

        TRACE("could not map: %s\n", strerror(ret));
        TRACE("destroying '%s'\n", SERVICE_SHM_NAME);
        shm_unlink(SERVICE_SHM_NAME);
        close_ex(fd);
        return ret;
    }

    memdump(state, sizeof(xinput_shared_gamepad_state));

    memset(state, 0, sizeof(xinput_shared_gamepad_state));

    memset(xinput_service_thread_parameter, 0, sizeof(xinput_service_thread_parameter));;

    //state->poke_us = timeus();

    /* from this point, there should be no race/conflict creating the resources */

    if(!xinput_service_lock_create())
    {
        xinput_service_destroy();
        return -1;
    }

    memset(&xinput_service_thread_parameter, 0, sizeof(xinput_service_thread_parameter));
    xinput_driver_initialize();

#if XINPUT_USES_MQUEUE
    if(!xinput_service_queue_create())
    {
        xinput_service_destroy();
        return -1;
    }
#endif

    /* everything is ready, set the pid */

    service_shared = state;
    service_fd = fd;

    memdump(state, sizeof(xinput_shared_gamepad_state));

    /* the probe will live until the program dies */

    return ERROR_SUCCESS;
}

int xinput_service_run(void)
{
#if XINPUT_USES_MQUEUE
    pthread_t tid;
    int ret;
#endif

    service_shared->master_pid = getpid();
    TRACE("owner set to %i\n", service_shared->master_pid);

#if XINPUT_USES_MQUEUE
    ret = pthread_create(&tid, NULL, xinput_service_rumble_thread, NULL);

    if(ret == 0)
    {
        xinput_service_rumble_thread_id = tid;
    }
    else
    {
        TRACE("could not spawn: %s\n", strerror(ret));

        xinput_service_destroy();
        return -1;
    }
#endif

    xinput_service_thread(NULL);
    return 0;
}

BOOL xinput_service_self(void)
{
    return service_shared != NULL;
}

static BOOL xinput_service_acquire_lock(void)
{
    int ret;

    TRACE("system-wide exclusive lock\n");

    ret = open(XINPUT_SYSTEM_WIDE_LOCK_FILE, O_CREAT|O_RDWR|O_TRUNC, 0666);
    if(ret >= 0)
    {
        TRACE("system-wide lock file opened\n");

        if(flock(ret, LOCK_EX|LOCK_NB) >= 0)
        {
            TRACE("system-wide lock acquired\n");
            return TRUE;
        }
        else
        {
            close_ex(ret);

            ret = errno;
            TRACE("system-wide lock failed: %s\n", strerror(ret));
        }
    }
    else
    {
        ret = errno;
        TRACE("system-wide lock file cannot be opened: %s\n", strerror(ret));
    }
    return FALSE;
}

void xinput_service_server(void)
{
    int ret;

    if(!xinput_service_acquire_lock())
    {
        return;
    }

    TRACE("starting\n");

    for(;;)
    {
        TRACE("creating\n");
        if((ret = xinput_service_create()) == ERROR_SUCCESS)
        {
            TRACE("running\n");
            xinput_service_run();

            break;
        }
        else if(ret == EEXIST)
        {
            TRACE("poking\n");
            if((ret = xinput_service_poke()) != ENOENT)
            {
                break;
            }
        }
        else
        {
            TRACE("cannot handle error %i\n", ret);
            break;
        }
    }

    xinput_service_destroy();
    TRACE("stopping\n");
}

#if !XINPUT_RUNDLL
static void* xinput_service_server_thread(void* args)
{
    (void)args;
    xinput_service_server();
    return NULL;
}
#endif

void xinput_service_rundll(void)
{
#if XINPUT_RUNDLL
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    LPSTR cmd;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    cmd = strdup("rundll32.exe xinput1_3.dll,XInputServer");

    // Start the child process.
    if( !CreateProcessA( NULL,                  // No module name (use command line)
        cmd,    // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW|DETACHED_PROCESS,      //
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    )
    {
        TRACE( "CreateProcess failed (%d).\n", GetLastError() );
    }
    free(cmd);
#else
    //xinput_service_set_autoshutdown(0);
    
    pthread_t tid;
    if(pthread_create(&tid, NULL, xinput_service_server_thread, NULL) == 0)
    {
        service_thread_id = tid;
    }
#endif
}

void xinput_service_set_autoshutdown(int strikes)
{
    xinput_service_idle_strikes = strikes;
}