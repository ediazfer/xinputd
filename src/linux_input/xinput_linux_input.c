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

#if HAVE_LINUX_INPUT_H

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#if HAVE_WINE
#include "wine/debug.h"
#endif

#include "xinput.h"
#include "debug.h"
#include "tools.h"

#include "xinput_linux_input.h"
#include "xinput_linux_input_xboxpad.h"

#include "xinput_linux_input_xboxpad_2.h" /* an example with table implementation */

#include "xinput_linux_input_debug.h"

#define JOY_MAX 32

WINE_DEFAULT_DEBUG_CHANNEL(xinput);

struct XINPUT_GAMEPAD_PRIVATE_STATE
{
   xinput_gamepad_device device;
   int input_index;
};

typedef struct XINPUT_GAMEPAD_PRIVATE_STATE XINPUT_GAMEPAD_PRIVATE_STATE;

static XINPUT_GAMEPAD_PRIVATE_STATE xinput_linux_input_slot[XUSER_MAX_COUNT];

static uint64_t xinput_linux_input_probe_last_epoch = 0;

static int xinput_linux_input_next_free_slot(void)
{
    for(int i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        if(xinput_linux_input_slot[i].device.vtbl == NULL)
        {
            return i;
        }
    }
    return -1;
}

int xinput_linux_input_read_next(int fd, struct input_event* ie)
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

int xinput_linux_input_rumble(int fd, int id, SHORT low_left, SHORT high_right)
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

void xinput_linux_input_feedback_clear(int fd, int id)
{
    if(ioctl(fd, EVIOCRMFF, id) == -1)
    {
        int err = errno;
        TRACE("could not clear effect %i: %s\n", id, strerror(err));
    }
}

static BOOL xinput_linux_input_device_in_use(int input_index)
{
    for(int i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        if(xinput_linux_input_slot[i].input_index == input_index)
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

uint32_t xinput_linux_input_probe(void)
{
    uint64_t now = timeus();
    int n;
    int fd;
    int version;
    int abs_count;
    int key_count;
    int ff_count;
    int one = 1;
    uint32_t mask = 0;
    struct input_id id;

    uint8_t prop[INPUT_PROP_MAX];
    uint8_t ev_all[EV_CNT>>3];
    uint8_t ev_key[KEY_CNT>>3];
    uint8_t ev_abs[ABS_CNT>>3];
    uint8_t ev_ff[FF_CNT>>3];
    char device_name[128];
    char location[128];
    char filename[128];

    if(now - xinput_linux_input_probe_last_epoch < 5000000)
    {
        return 0;
    }
    
    /* to silence valgrind */
    
    memset(prop,0,sizeof(prop));
    memset(ev_all,0,sizeof(ev_all));
    memset(ev_key,0,sizeof(ev_key));
    memset(ev_abs,0,sizeof(ev_abs));
    memset(ev_ff,0,sizeof(ev_ff));
    memset(device_name,0,sizeof(device_name));
    memset(location,0,sizeof(location));
    memset(filename,0,sizeof(filename));    

    xinput_linux_input_probe_last_epoch = now;

#if XINPUT_TRACE_DEVICE_DETECTION
    TRACE("probing devices\n");
#endif

    for(int input_index = 0; input_index < JOY_MAX; ++input_index)
    {
        int slot = xinput_linux_input_next_free_slot();

        if(slot < 0)
        {
            TRACE("no more slot available (whoopsie)\n");
            break;
        }

        /* already in use */

        if(xinput_linux_input_device_in_use(input_index))
        {
            continue;
        }

        snprintf(filename, sizeof(filename), "/dev/input/event%i", input_index);

        fd = open(filename, O_RDWR);

        if(fd < 0)
        {
            /*  TRACE("no joystick at %s\n", filename); */
            continue;
        }

#if XINPUT_TRACE_DEVICE_DETECTION
        TRACE("opened joystick at %s\n", filename);
#endif

        if(ioctl(fd, EVIOCGVERSION, &version) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("version: %x\n", version);
#endif
        }

        if(ioctl(fd, EVIOCGID, &id) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("id: %x %x %x %x\n", id.bustype, id.product, id.vendor, id.version);
#endif
        }

        if(ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("name: '%s'\n", device_name);
#endif
        }

        if(ioctl(fd, EVIOCGPHYS(sizeof(location)), location) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("loc: '%s'\n", location);
#endif
        }

        if((n = ioctl(fd, EVIOCGPROP(sizeof(prop)), prop)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("prop: %i\n", n);
            hexdump(prop, n);
            TRACE("\n");
#endif
        }

        if((n = ioctl(fd, EVIOCGBIT(0, EV_CNT), ev_all)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            memdump(ev_all, sizeof(ev_all));

            for(int j = 0; j < EV_CNT; ++j)
            {
                BOOL on = bit_get(ev_all, j);
                if(on)
                {
                    TRACE("%s ", xinput_linux_input_event_type_get_name(j)); /* no LF */
                }
            }
#endif
        }
        else
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("ev all: %s\n", strerror(errno));
#endif
        }

        if(!(bit_get(ev_all, EV_KEY) && bit_get(ev_all, EV_ABS)))
        {
            TRACE("%s @%s: no buttons nor absolute axe(s)\n",
                    device_name,
                    filename);
            close_ex(fd);
            continue;
        }

        key_count = 0;
        abs_count = 0;
        ff_count = 0;

        if((n = ioctl(fd, EVIOCGBIT(EV_KEY, KEY_CNT), ev_key)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            memdump(ev_key, sizeof(ev_key));
#endif
            for(int j = 0; j < KEY_CNT; ++j)
            {
                BOOL on = bit_get(ev_key, j);
                if(on)
                {
#if XINPUT_TRACE_DEVICE_DETECTION
                    TRACE("%s ", xinput_linux_input_key_get_name(j)); /* no LF */
#endif
                    ++key_count;
                }
            }

#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE(": %i keys\n", key_count);
#endif
        }
        else
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("ev key: %s\n", strerror(errno));
#endif
        }

        if((n = ioctl(fd, EVIOCGBIT(EV_ABS, ABS_CNT), ev_abs)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            memdump(ev_abs, sizeof(ev_abs));
#endif
            for(int j = 0; j < ABS_CNT; ++j)
            {
                BOOL on = bit_get(ev_abs, j);
                if(on)
                {
#if XINPUT_TRACE_DEVICE_DETECTION
                    TRACE("%s ", xinput_linux_input_abs_get_name(j)); /* no LF */
#endif
                    ++abs_count;
                }
            }
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE(": %i abs\n", abs_count);
#endif
        }
        else
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("ev abs: %s\n", strerror(errno));
#endif
        }

        if((n = ioctl(fd, EVIOCGBIT(EV_FF, FF_CNT), ev_ff)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            memdump(ev_ff, sizeof(ev_ff));
#endif
            for(int j = 0; j < FF_CNT; ++j)
            {
                BOOL on = bit_get(ev_ff, j);
                if(on)
                {
#if XINPUT_TRACE_DEVICE_DETECTION
                    TRACE("%s ", xinput_linux_input_ff_get_name(j)); /* no LF */
#endif
                    ++ff_count;
                }
            }
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE(": %i ff\n", ff_count);
#endif
        }

        if(ioctl(fd, EVIOCGRAB, &one) < 0)
        {
            /* cannot grab it for myself */
            TRACE("cannot grab device: %s\n", strerror(errno));

            continue;
        }

        if(xinput_linux_input_xboxpad_can_translate(&id))
        {
            xinput_gamepad_device* device = &xinput_linux_input_slot[slot].device;
            xinput_linux_input_xboxpad_new_instance(&id, fd, device);
            xinput_linux_input_slot[slot].input_index = input_index;
        }
        /*
        else if(xinput_linux_input_xboxpad2_can_translate(&id))
        {
            xinput_gamepad_device* device = &xinput_linux_input_slot[slot].device;
            xinput_linux_input_xboxpad2_new_instance(&id, fd, device);
            xinput_linux_input_slot[slot].input_index = input_index;
        }
        */
        else
        {
            /* no more translators */

            continue;
        }

        mask |= 1 << slot;
    }

#if XINPUT_TRACE_DEVICE_DETECTION
    TRACE("devices probed\n");
#endif

    return mask;
}

xinput_gamepad_device* xinput_linux_input_get_device(int slot)
{
    if(slot >= 0 && slot < XUSER_MAX_COUNT)
    {
        if(xinput_linux_input_slot[slot].device.vtbl != NULL)
        {
            return &xinput_linux_input_slot[slot].device;
        }
    }

    return NULL;
}

void xinput_linux_input_device_close(int slot)
{
    xinput_gamepad_device* device = xinput_linux_input_get_device(slot);

    if(device != NULL)
    {
        device->vtbl->release(device);

        xinput_linux_input_slot[slot].device.data = NULL;
        xinput_linux_input_slot[slot].device.vtbl = NULL;
        xinput_linux_input_slot[slot].input_index = -1;
    }
}

void xinput_linux_input_initialize(void)
{
    for(int slot = 0; slot < XUSER_MAX_COUNT; ++slot)
    {
        xinput_linux_input_device_close(slot);
    }
}

void xinput_linux_input_finalize(void)
{
}

#endif /* HAVE_LINUX_INPUT_H */
