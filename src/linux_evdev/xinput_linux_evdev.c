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

#if HAVE_LINUX_INPUT_H

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

#if HAVE_WINE
#include "wine/debug.h"
#endif

#include "xinput.h"
#include "debug.h"
#include "tools.h"

#include "xinput_linux_evdev.h"
/* #include "xinput_linux_evdev_xboxpad.h" */
#include "xinput_linux_evdev_generic.h"
/* #include "xinput_linux_evdev_xboxpad_2.h" an example with table implementation */

#include "xinput_linux_evdev_debug.h"

#define EVENT_MAX 32

WINE_DEFAULT_DEBUG_CHANNEL(xinput);

struct XINPUT_GAMEPAD_PRIVATE_STATE
{
   xinput_gamepad_device device;
   uint64_t inode;
};

typedef struct XINPUT_GAMEPAD_PRIVATE_STATE XINPUT_GAMEPAD_PRIVATE_STATE;

static XINPUT_GAMEPAD_PRIVATE_STATE xinput_linux_evdev_slot[XUSER_MAX_COUNT] = {0};

static uint64_t xinput_linux_evdev_probe_last_epoch = 0;

static int xinput_linux_evdev_next_free_slot(void)
{
    for(int i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        if(xinput_linux_evdev_slot[i].device.vtbl == NULL)
        {
            return i;
        }
    }
    return -1;
}

int xinput_linux_evdev_read_next(int fd, struct input_event* ie)
{
    return read_fully(fd, ie, sizeof(*ie));
}

/**
 * The left motor is supposed to be low frequency, high magnitude
 * The right motor is supposed to be high frequency, weak magnitude
 *
 * @param fd
 * @param id the current effect id, or -1
 * @param low
 * @param high
 *
 * @return the id of the effect or -1 if it failed to register the effect
 */

int xinput_linux_evdev_rumble(int fd, int id, SHORT low_left, SHORT high_right)
{
    struct ff_effect effect;

    effect.type = FF_RUMBLE;
    effect.id = id;
    effect.u.rumble.strong_magnitude = low_left;
    effect.u.rumble.weak_magnitude = high_right;
    effect.replay.length = 5000;
    effect.replay.delay = 0;

    if(ioctl(fd, EVIOCSFF, &effect) != -1)
    {
        struct input_event ie;
        ie.type = EV_FF;
        ie.code = effect.id;
        ie.value = 1;

        if(write_fully(fd, &ie, sizeof(ie)) < 0)
        {
            int err = errno;
            TRACE("could not send rumble: %i [%i, %i]: %s\n", fd, low_left, high_right, strerror(err));
        }
    }
    else
    {
        int err = errno;
        TRACE("could not setup rumble: %i [%i, %i]: %s\n", fd, low_left, high_right, strerror(err));
    }

    return effect.id;
}

void xinput_linux_evdev_feedback_clear(int fd, int id)
{
    if(ioctl(fd, EVIOCRMFF, id) == -1)
    {
        int err = errno;
        TRACE("could not clear effect %i: %s\n", id, strerror(err));
    }
}

static BOOL xinput_linux_evdev_device_in_use(int inode)
{
    for(int i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        if(xinput_linux_evdev_slot[i].inode == inode)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * Linux input
 *
 * @return
 */

typedef struct xinput_linux_evdev_probe_s xinput_linux_evdev_probe_s;

static const char device_dir_name[] = "/dev/input/by-path";
static const char event_joystick[] = "-event-joystick";

uint32_t xinput_linux_evdev_probe(void)
{
    uint64_t now = timeus();
    int n;
    int fd;
        
    int one = 1;
    uint32_t mask = 0;
    
    struct xinput_linux_evdev_probe_s probed;
    
    struct inode_epoch_s
    {
        uint64_t inode;
        uint64_t epoch;
    };
    
    struct inode_epoch_s joystick_inodes[64] = {0};
    size_t joystick_inodes_count = 0;
    
    char filename[128];
   
    if(now - xinput_linux_evdev_probe_last_epoch < 5000000)
    {
        return 0;
    }
    
    /* to silence valgrind */

    xinput_linux_evdev_probe_last_epoch = now;

#if XINPUT_TRACE_DEVICE_DETECTION
    TRACE("probing devices\n");
#endif
    
    DIR* devices_dir = opendir(device_dir_name);
    
    if(devices_dir != NULL)
    {
        for(;;)
        {
            int slot = xinput_linux_evdev_next_free_slot();

            if(slot < 0)
            {
                TRACE("all joystick slots are already allocated\n");
                break;
            }
            
            const struct dirent* dir_entry = readdir(devices_dir);
            
            if(dir_entry == NULL)
            {
                break;
            }
            
            size_t dir_entry_name_len = strlen(dir_entry->d_name);
            
            if(sizeof(device_dir_name) + 1 + dir_entry_name_len > sizeof(filename))
            {
                /*  will not be able to handle the name (filename buffer is too small */
                TRACE("%s/%s is bigger than expected", device_dir_name, filename);
                continue;
            }
            
            if((dir_entry_name_len < sizeof(event_joystick)) || (memcmp(&dir_entry->d_name[dir_entry_name_len - sizeof(event_joystick) + 1], event_joystick, sizeof(event_joystick)) != 0))
            {
                /*  not a joystick */
                continue;
            }
            
            /*  already in use ? */

            if(xinput_linux_evdev_device_in_use(dir_entry->d_ino))
            {
                continue;
            }
            
            /*  do not process a device that remains the same since the last probe (as it was discarded already) */

            memcpy(filename, device_dir_name,  sizeof(device_dir_name));
            filename[sizeof(device_dir_name) - 1] = '/';
            memcpy(&filename[sizeof(device_dir_name)], dir_entry->d_name, dir_entry_name_len + 1);
            
            struct stat st;

            if(lstat(filename, &st) < 0)
            {
                continue;
            }

            uint64_t ct = st.st_ctim.tv_sec;
            ct *= 1000ULL;
            ct += st.st_ctim.tv_nsec / 1000000ULL;
            
            int joystick_inode_found = 0;
            for(int i = 0; i < joystick_inodes_count; ++i)
            {
                if(joystick_inodes[i].inode == dir_entry->d_ino)
                {
                    if(joystick_inodes[i].epoch >= ct)
                    {
                        // no change
                        continue;
                    }
                    
                    joystick_inode_found = 1;
                    break;
                }
            }

            if(!joystick_inode_found)
            {
                if(joystick_inodes_count < (sizeof(joystick_inodes) / sizeof(joystick_inodes[0])) - 1 )
                {
                    joystick_inodes[joystick_inodes_count].inode = dir_entry->d_ino;
                    joystick_inodes[joystick_inodes_count].epoch =  ct;
                    ++joystick_inodes_count;
                }
                else
                {
                    // forget the first one
                    memmove(&joystick_inodes[0], &joystick_inodes[1], sizeof(joystick_inodes) - sizeof(joystick_inodes[0]));
                    joystick_inodes[joystick_inodes_count - 1].inode = dir_entry->d_ino;
                    joystick_inodes[joystick_inodes_count - 1].epoch =  ct;
                }
            }

            fd = open(filename, O_RDONLY);

            if(fd < 0)
            {
                TRACE("cannot open joystick at %s: %s\n", filename, strerror(errno));
                continue;
            }

            memset(&probed, 0, sizeof(struct xinput_linux_evdev_probe_s));

    #if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("opened joystick at %s\n", filename);
    #endif

            if(ioctl(fd, EVIOCGVERSION, &probed.version) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE("version: %x\n", probed.version);
    #endif
            }

            if(ioctl(fd, EVIOCGID, &probed.id) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE("id: %x %x %x %x\n", probed.id.bustype, probed.id.product, probed.id.vendor, probed.id.version);
    #endif
            }

            if(ioctl(fd, EVIOCGNAME(sizeof(probed.device_name)), probed.device_name) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE("name: '%s'\n", probed.device_name);
    #endif
            }

            if(ioctl(fd, EVIOCGPHYS(sizeof(probed.location)), probed.location) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE("loc: '%s'\n", probed.location);
    #endif
            }

            if((n = ioctl(fd, EVIOCGPROP(sizeof(probed.prop)), probed.prop)) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE("prop: %i\n", n);
                hexdump(probed.prop, n);
                TRACE("\n");
    #endif
            }

            if((n = ioctl(fd, EVIOCGBIT(0, EV_CNT), probed.ev_all)) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                memdump(probed.ev_all, sizeof(probed.ev_all));

                for(int j = 0; j < EV_CNT; ++j)
                {
                    BOOL on = bit_get(probed.ev_all, j);
                    if(on)
                    {
                        TRACE("%s ", xinput_linux_evdev_event_type_get_name(j)); /* no LF */
                    }
                }

                TRACE("\n");
    #endif
            }
            else
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE("ev all: %s\n", strerror(errno));
    #endif
            }

            // uint8_t wanted_mask = (1<<EV_SYN)|(1<<EV_KEY)|(1<<EV_ABS)|(1<<EV_MSC);
            // uint8_t rejected_mask = (1<<EV_REL)|(1<<EV_PWR);

            if(!(bit_get(probed.ev_all, EV_KEY) && bit_get(probed.ev_all, EV_ABS)) || bit_get(probed.ev_all, EV_REL) || bit_get(probed.ev_all, EV_PWR))
            {
                TRACE("%s @%s: not a candidate\n",
                        probed.device_name,
                        filename);
                close_ex(fd);
                continue;
            }
            else
            {
                TRACE("%s @%s: is a candidate\n",
                        probed.device_name,
                        filename);
            }

            probed.key_count = 0;
            probed.abs_count = 0;
            probed.ff_count = 0;

            if((n = ioctl(fd, EVIOCGBIT(EV_KEY, KEY_CNT), probed.ev_key)) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                //memdump(probed.ev_key, sizeof(probed.ev_key));
    #endif
                for(int j = 0; j < KEY_CNT; ++j)
                {
                    BOOL on = bit_get(probed.ev_key, j);
                    if(on)
                    {
    #if XINPUT_TRACE_DEVICE_DETECTION
                        TRACE("%s ", xinput_linux_evdev_key_get_name(j)); /* no LF */
    #endif
                        ++probed.key_count;
                    }
                }

    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE(": %i keys\n", probed.key_count);
    #endif
            }
            else
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE("ev key: %s\n", strerror(errno));
    #endif
            }

            if((n = ioctl(fd, EVIOCGBIT(EV_ABS, ABS_CNT), probed.ev_abs)) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                memdump(probed.ev_abs, sizeof(probed.ev_abs));
    #endif
                for(int j = 0; j < ABS_CNT; ++j)
                {
                    BOOL on = bit_get(probed.ev_abs, j);
                    if(on)
                    {
    #if XINPUT_TRACE_DEVICE_DETECTION
                        TRACE("%s ", xinput_linux_evdev_abs_get_name(j)); /* no LF */
    #endif
                        ++probed.abs_count;
                    }
                }
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE(": %i abs\n", probed.abs_count);
    #endif
            }
            else
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE("ev abs: %s\n", strerror(errno));
    #endif
            }

            if((n = ioctl(fd, EVIOCGBIT(EV_FF, FF_CNT), probed.ev_ff)) != -1)
            {
    #if XINPUT_TRACE_DEVICE_DETECTION
                memdump(probed.ev_ff, sizeof(probed.ev_ff));
    #endif
                for(int j = 0; j < FF_CNT; ++j)
                {
                    BOOL on = bit_get(probed.ev_ff, j);
                    if(on)
                    {
    #if XINPUT_TRACE_DEVICE_DETECTION
                        TRACE("%s ", xinput_linux_evdev_ff_get_name(j)); /* no LF */
    #endif
                        ++probed.ff_count;
                    }
                }
    #if XINPUT_TRACE_DEVICE_DETECTION
                TRACE(": %i ff\n", probed.ff_count);
    #endif
            }

            if(ioctl(fd, EVIOCGRAB, &one) < 0)
            {
                /* cannot grab it for myself */
                TRACE("cannot grab device: %s\n", strerror(errno));

                continue;
            }

            if(xinput_linux_evdev_generic_can_translate(&probed))
            {
                xinput_gamepad_device* device = &xinput_linux_evdev_slot[slot].device;
                xinput_linux_evdev_generic_new_instance(&probed, fd, device);
                xinput_linux_evdev_slot[slot].inode = dir_entry->d_ino;
            }
            /*
            if(xinput_linux_evdev_xboxpad_can_translate(&probed))
            {
                xinput_gamepad_device* device = &xinput_linux_evdev_slot[slot].device;
                xinput_linux_evdev_xboxpad_new_instance(&probed, fd, device);
                xinput_linux_evdev_slot[slot].input_index = input_index;
            }
            */
            /*
            else if(xinput_linux_evdev_xboxpad2_can_translate(&id))
            {
                xinput_gamepad_device* device = &xinput_linux_evdev_slot[slot].device;
                xinput_linux_evdev_xboxpad2_new_instance(&id, fd, device);
                xinput_linux_evdev_slot[slot].input_index = input_index;
            }
            */
            else
            {
                /* no more translators */
                close_ex(fd);
                continue;
            }

            mask |= 1 << slot;
        }
        
        closedir(devices_dir);
    }
    else
    {
        TRACE("could not open %s: %s\n", device_dir_name, strerror(errno));
    }

#if XINPUT_TRACE_DEVICE_DETECTION
    TRACE("devices probed\n");
#endif

    return mask;
}

xinput_gamepad_device* xinput_linux_evdev_get_device(int slot)
{
    if(slot >= 0 && slot < XUSER_MAX_COUNT)
    {
        if(xinput_linux_evdev_slot[slot].device.vtbl != NULL)
        {
            return &xinput_linux_evdev_slot[slot].device;
        }
    }

    return NULL;
}

void xinput_linux_evdev_device_close(int slot)
{
    xinput_gamepad_device* device = xinput_linux_evdev_get_device(slot);

    if(device != NULL)
    {
        device->vtbl->release(device);

        xinput_linux_evdev_slot[slot].device.data = NULL;
        xinput_linux_evdev_slot[slot].device.vtbl = NULL;
    }
    
    xinput_linux_evdev_slot[slot].inode = -1;
}

void xinput_linux_evdev_initialize(void)
{
    for(int slot = 0; slot < XUSER_MAX_COUNT; ++slot)
    {
        xinput_linux_evdev_device_close(slot);
    }
}

void xinput_linux_evdev_finalize(void)
{
}

#endif /* HAVE_LINUX_INPUT_H */
