/*
 * MIT License
 *
 * Unix XInput Gamepad interface implementation
 *
 * Copyright (c) 2016 Eric Diaz Fernandez
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
#include <errno.h>
#include <string.h>

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
#include "winerror.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <stdint.h>
#include <errno.h>
#include <unistd.h>

#include "xinput.h"
#include "xinput_gamepad.h"
#include "xinput_service.h"
#include "tools.h"
#include "debug.h"

#if HAVE_WINE
WINE_DEFAULT_DEBUG_CHANNEL(xinput);
#else
#define WINAPI
#define DECLSPEC_HOTPATCH
#endif

static pthread_mutex_t xinput_gamepad_init_mtx = PTHREAD_MUTEX_INITIALIZER;
static volatile int xinput_gamepad_init_done = 0;

static xinput_shared_gamepad_state* client_shared = NULL;
static int client_fd = -1;
static int64_t client_last_probe = 0;

#if XINPUT_USES_SEMAPHORE_MUTEX
static sem_t*  client_sem = SEM_FAILED;
#endif

#if XINPUT_USES_MQUEUE
static mqd_t client_mq = MQD_INVALID;
#endif

static BOOL xinput_gamepad_lock_open(void)
{
#if XINPUT_USES_SEMAPHORE_MUTEX
    client_sem = sem_open(SERVICE_SEM_NAME, O_RDWR);
    if(client_sem == SEM_FAILED)
    {
        int ret = errno;
        TRACE("could not open semaphore: %s\n", strerror(ret));
        return FALSE;
    }
#endif
    return TRUE;
}

static void xinput_gamepad_lock_close(void)
{
#if XINPUT_USES_SEMAPHORE_MUTEX
    if(client_sem != SEM_FAILED)
    {
        sem_close(client_sem);
        client_sem = SEM_FAILED;
    }
#endif
}

static BOOL xinput_gamepad_lock(void)
{
#if XINPUT_USES_SEMAPHORE_MUTEX
    return sem_trywait(client_sem) == 0;
#else
    return TRUE;
#endif
}

static void xinput_gamepad_unlock(void)
{
#if XINPUT_USES_SEMAPHORE_MUTEX
    sem_post(client_sem);
#endif
}

#if XINPUT_USES_MQUEUE

static BOOL xinput_gamepad_service_queue_open(void)
{
    mqd_t mq = mq_open(SERVICE_MSG_NAME, O_WRONLY);
    if(mq == MQD_INVALID)
    {
        int ret = errno;
        TRACE("could not open message queue: %s\n", strerror(ret));
        return FALSE;
    }
    client_mq = mq;

    return TRUE;
}

static void xinput_gamepad_service_queue_close(void)
{
    if(client_mq != MQD_INVALID)
    {
        mq_close(client_mq);
        client_mq = MQD_INVALID;
    }
}

#endif

static void xinput_gamepad_service_disconnect(void)
{
#if XINPUT_USES_MQUEUE
    xinput_gamepad_service_queue_close();
#endif

    xinput_gamepad_lock_close();

    if(client_shared != NULL)
    {
        TRACE("disconnecting\n");

        munmap(client_shared, sizeof(xinput_shared_gamepad_state));
        client_shared = NULL;

        close_ex(client_fd);
        client_fd = -1;
    }
}

/**
 * Tells if the service is alive
 *
 * @return 0 if the service is alive, <0 if not.
 */

static int xinput_gamepad_service_is_alive(void)
{
    pid_t pid;
    BOOL dead;

#if !XINPUT_RUNDLL

    if(xinput_service_self())
    {
        client_shared->poke_us = timeus();
        
        TRACE("alive (self)\n");
        return 0;
    }
#endif

    /*  wait until the pid is set, or assumed to be broken */

    TRACE("fetching master pid, shared@%p\n", client_shared);

    /* give 5 tries to get the master */

    for(int i = 5; i >= 0; --i)
    {
        
        pid = client_shared->master_pid;

        /* if the pid is 0, the shared memory has not been set yet */

        if(pid != 0)
        {
            TRACE("owned by pid %i\n", pid);
            break;
        }

        /*  this will only happen if the DLL has been loaded twice "at the same time" */

        usleep(XINPUT_OWNER_PROBE_PERIOD_US);
    }

    TRACE("still owned by pid %i\n", pid);

    /* owner 0 or "broken" means surely dead */

    dead = (pid == 0) || (pid == XINPUT_OWNER_BROKEN);

    if(!dead)
    {
        /* else, the pid has to be checked to see if it's still alive */

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

        /* tells the server needs to be spawned */

        return -1;
    }
    else
    {
        client_shared->poke_us = timeus();

        TRACE("alive\n");
        /*  alive */
        return 0;
    }
}

static int xinput_gamepad_service_connect(void)
{
    xinput_shared_gamepad_state* state;
    int ret;
    int fd;

    TRACE("connecting\n");

    ret = shm_open(SERVICE_SHM_NAME, O_RDWR, 0666);

    if(ret < 0)
    {
        ret = errno;
        TRACE("could not connect: %i = %s\n", ret, strerror(ret));
        return ret; /* ENOENT : will create */
    }

    fd = ret;

    TRACE("mapping\n");

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

    memdump(state, sizeof(xinput_shared_gamepad_state));

    client_shared = state;
    client_fd = fd;

    if(!xinput_gamepad_lock_open())
    {
        TRACE("error opening lock\n");

        xinput_gamepad_service_disconnect();

        return -1;  /* do not return ENOENT */
    }

#if XINPUT_USES_MQUEUE
    if(!xinput_gamepad_service_queue_open())
    {
        TRACE("error opening queue\n");

        xinput_gamepad_service_disconnect();

        return -2;  /* do not return ENOENT */
    }
#endif

    client_shared->poke_us = timeus();

    TRACE("connected\n");

    return ERROR_SUCCESS;
}

static void xinput_gamepad_service_probe(void)
{
    int64_t now;

    xinput_gamepad_init();

    now = timeus();

    if(now - client_last_probe > XINPUT_OWNER_REPROBE_PERIOD_US)
    {
        if(xinput_gamepad_service_is_alive() < 0)
        {
            /* spawn the server and wait for a bit */

            xinput_gamepad_service_disconnect();

            xinput_service_rundll();
            sleep(1);

            xinput_gamepad_service_connect();
        }
        client_last_probe = now;
    }
    else
    {
        if(client_shared != NULL)
        {
            client_shared->poke_us = timeus();
        }
    }
}

void xinput_gamepad_init(void)
{
    pthread_mutex_lock(&xinput_gamepad_init_mtx);
    if(xinput_gamepad_init_done != 0)
    {
        pthread_mutex_unlock(&xinput_gamepad_init_mtx);
        return;
    }

    xinput_gamepad_init_done = 1;
    pthread_mutex_unlock(&xinput_gamepad_init_mtx);

    TRACE("initializing\n");

    xinput_service_rundll();

    for(;;)
    {
        int ret = xinput_gamepad_service_connect();

        if(ret == 0)
        {
            /*
             * The IPCs are set, but there may be nobody on the
             * other side
             */

            if(xinput_gamepad_service_is_alive() >= 0)
            {
                break;
            }
            else
            {
                /*  the server is dead : retry */

                xinput_gamepad_service_disconnect();

                xinput_service_rundll();
                sleep(1);

                continue;
            }
        }

        if(ret == ENOENT)
        {
            TRACE("starting server\n");
            xinput_service_rundll();
            sleep(1);
        }
        else if(ret < 0)
        {
            TRACE("could not connect to IPCs: %i", ret);
        }
        else
        {
            TRACE("failed to connect: %s\n", strerror(ret));
        }

        usleep(100000);
    }

    TRACE("initialized\n");
}

void xinput_gamepad_finalize(void)
{
    pthread_mutex_lock(&xinput_gamepad_init_mtx);
    if(xinput_gamepad_init_done == 0)
    {
        pthread_mutex_unlock(&xinput_gamepad_init_mtx);

        TRACE("not initialized\n");
        return;
    }

    xinput_gamepad_init_done = 0;
    pthread_mutex_unlock(&xinput_gamepad_init_mtx);

    TRACE("finalizing\n");

    xinput_gamepad_service_disconnect();

    TRACE("finalized\n");
}

BOOL xinput_gamepad_connected(int index)
{
    xinput_gamepad_state* xgs;
    BOOL ret = FALSE;

    xinput_gamepad_service_probe();

    if(client_shared == NULL)
    {
        TRACE("shared memory not mapped, initialized=%i\n", xinput_gamepad_init_done);
        return ret;
    }

    xgs = &client_shared->state[index];

    if(xinput_gamepad_lock())
    {
        ret = xgs->connected;
        xinput_gamepad_unlock();
    }

    return ret;
}

static DWORD xinput_gamepad_axis_to_buttons(SHORT value, DWORD negative, DWORD positive)
{
    if(value < -THUMB_TO_BUTTONS_THRESHOLD)
    {
        return negative;
    }
    else if(value > THUMB_TO_BUTTONS_THRESHOLD)
    {
        return positive;
    }
    else
    {
        return 0;
    }
}

BOOL xinput_gamepad_copy_buttons_state(int index, DWORD* out_buttons)
{
    xinput_gamepad_state* xgs;
    XINPUT_GAMEPAD gamepad;
    DWORD buttons;

    if(out_buttons == NULL)
    {
        return FALSE;
    }

    xinput_gamepad_service_probe();
    xgs = &client_shared->state[index];

    if(xinput_gamepad_lock())
    {
        memcpy(&gamepad, &xgs->gamepad, sizeof(XINPUT_GAMEPAD));

        xinput_gamepad_unlock();

        buttons = gamepad.wButtons;
        buttons |= xinput_gamepad_axis_to_buttons(gamepad.sThumbLX, XINPUT_GAMEPAD_LTHUMB_LEFT, XINPUT_GAMEPAD_LTHUMB_RIGHT);
        buttons |= xinput_gamepad_axis_to_buttons(gamepad.sThumbLY, XINPUT_GAMEPAD_LTHUMB_DOWN, XINPUT_GAMEPAD_LTHUMB_UP);
        buttons |= xinput_gamepad_axis_to_buttons(gamepad.sThumbRX, XINPUT_GAMEPAD_RTHUMB_LEFT, XINPUT_GAMEPAD_RTHUMB_RIGHT);
        buttons |= xinput_gamepad_axis_to_buttons(gamepad.sThumbRY, XINPUT_GAMEPAD_RTHUMB_DOWN, XINPUT_GAMEPAD_RTHUMB_UP);

        if(gamepad.bLeftTrigger > TRIGGER_TO_BUTTONS_THRESHOLD)
        {
            buttons |= XINPUT_GAMEPAD_LTRIGGER;
        }
        if(gamepad.bRightTrigger > TRIGGER_TO_BUTTONS_THRESHOLD)
        {
            buttons |= XINPUT_GAMEPAD_RTRIGGER;
        }

        *out_buttons = buttons;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void xinput_gamepad_copy_state(int index, XINPUT_STATE* out_state)
{
    xinput_gamepad_state* xgs;

    xinput_gamepad_service_probe();
    xgs = &client_shared->state[index];

    if(xinput_gamepad_lock())
    {
        memcpy(&out_state->Gamepad, &xgs->gamepad, sizeof(XINPUT_GAMEPAD));
        out_state->dwPacketNumber = xgs->dwPacketNumber;
        xinput_gamepad_unlock();
    }
    /*  If you say so ... : */
    /* The main difference between this and the Ex version is the media guide button */

    out_state->Gamepad.wButtons &= ~XINPUT_GAMEPAD_GUIDE;
}

void xinput_gamepad_copy_state_ex(int index, XINPUT_STATE_EX* out_state)
{
    xinput_gamepad_state* xgs;

    xinput_gamepad_service_probe();
    xgs = &client_shared->state[index];

    if(xinput_gamepad_lock())
    {
        memcpy(&out_state->Gamepad, &xgs->gamepad, sizeof(XINPUT_GAMEPAD));
        out_state->dwPacketNumber = xgs->dwPacketNumber;
        xinput_gamepad_unlock();
    }
}

void xinput_gamepad_rumble(int index, const XINPUT_VIBRATION *vibration)
{
#if XINPUT_USES_MQUEUE
    xinput_gamepad_vibration vibration_message;

    if(vibration == NULL)
    {
        return;
    }

    xinput_gamepad_service_probe();

    vibration_message.index = index;
    vibration_message.vibration = *vibration;

    for(;;)
    {
        if(mq_send(client_mq, (const char*)&vibration_message, sizeof(xinput_gamepad_vibration), 0) == 0)
        {
            break;
        }
        else
        {
            int err = errno;

            if((err == EINTR) && (err != EAGAIN))
            {
                break;
            }
        }
    }
#endif
}
