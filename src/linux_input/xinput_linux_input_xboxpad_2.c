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

#if HAVE_LINUX_INPUT_H

#include "xinput_settings.h"

/*
 * As a driver, this is redundant with xinput_linux_input_xboxpad
 * But this is an example of easy implementation that requires only
 * editing tables to implement the features of the joystick.
 * (Mind I only have 360/One pads for testing)
 *
 * The next driver I'll write is one that generates the same tables
 * to make do with whatever buttons/axis are found on an unknown joystick.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "xinput_gamepad.h"
#include "xinput_linux_input.h"
#include "xinput_linux_input_translator.h"
#include "tools.h"
#include "device_id.h"

XINPUT_GAMEPAD_ABS_BEGIN(xbox360_abs)
XINPUT_GAMEPAD_ABS_AXIS(ABS_X, sThumbLX)           /* 0x00 */
XINPUT_GAMEPAD_ABS_SIXA(ABS_Y, sThumbLY)           /* 0x01 */
XINPUT_GAMEPAD_ABS_AXIS(ABS_Z, bLeftTrigger)       /* 0x02 */
XINPUT_GAMEPAD_ABS_AXIS(ABS_RX, sThumbRX)          /* 0x03 */
XINPUT_GAMEPAD_ABS_SIXA(ABS_RY, sThumbRY)          /* 0x04 */
XINPUT_GAMEPAD_ABS_AXIS(ABS_RZ, bRightTrigger)     /* 0x05 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_THROTTLE)              /* 0x06 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_RUDDER)                /* 0x07 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_WHEEL)                 /* 0x08 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_GAS)                   /* 0x09 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_BRAKE)                 /* 0x0a */
XINPUT_GAMEPAD_ABS_NOPE(0x0b)
XINPUT_GAMEPAD_ABS_NOPE(0x0c)
XINPUT_GAMEPAD_ABS_NOPE(0x0d)
XINPUT_GAMEPAD_ABS_NOPE(0x0e)
XINPUT_GAMEPAD_ABS_NOPE(0x0f)
XINPUT_GAMEPAD_ABS_BTTN(ABS_HAT0X, XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_LEFT) /* 0x10 */
XINPUT_GAMEPAD_ABS_BTTN(ABS_HAT0Y, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_UP)        /* 0x11 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_HAT1X)                 /*  0x12 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_HAT1Y)                 /*  0x13 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_HAT2X)                 /*  0x14 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_HAT2Y)                 /*  0x15 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_HAT3X)                 /*  0x16 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_HAT3Y)                 /*  0x17 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_PRESSURE)              /*  0x18 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_DISTANCE)              /*  0x19 */
XINPUT_GAMEPAD_ABS_NOPE(ABS_TILT_X)                /*  0x1a */
XINPUT_GAMEPAD_ABS_NOPE(ABS_TILT_Y)                /*  0x1b */
XINPUT_GAMEPAD_ABS_NOPE(ABS_TOOL_WIDTH)            /*  0x1c */
XINPUT_GAMEPAD_ABS_NOPE(0x1d)
XINPUT_GAMEPAD_ABS_NOPE(0x1e)
XINPUT_GAMEPAD_ABS_NOPE(0x1f)
XINPUT_GAMEPAD_ABS_NOPE(ABS_VOLUME)                /*  0x20 */
XINPUT_GAMEPAD_ABS_NOPE(0x21)
XINPUT_GAMEPAD_ABS_NOPE(0x22)
XINPUT_GAMEPAD_ABS_NOPE(0x23)
XINPUT_GAMEPAD_ABS_NOPE(0x24)
XINPUT_GAMEPAD_ABS_NOPE(0x25)
XINPUT_GAMEPAD_ABS_NOPE(0x26)
XINPUT_GAMEPAD_ABS_NOPE(0x27)
XINPUT_GAMEPAD_ABS_NOPE(ABS_MISC)                  /*  0x28 */
XINPUT_GAMEPAD_ABS_NOPE(0x29)
XINPUT_GAMEPAD_ABS_NOPE(0x2a)
XINPUT_GAMEPAD_ABS_NOPE(0x2b)
XINPUT_GAMEPAD_ABS_NOPE(0x2c)
XINPUT_GAMEPAD_ABS_NOPE(0x2d)
XINPUT_GAMEPAD_ABS_NOPE(0x2e)
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_SLOT)               /*  0x2f MT slot being modified */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_TOUCH_MAJOR)        /*  0x30 Major axis of touching ellipse */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_TOUCH_MINOR)        /*  0x31 Minor axis (omit if circular) */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_WIDTH_MAJOR)        /*  0x32 Major axis of approaching ellipse */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_WIDTH_MINOR)        /*  0x33 Minor axis (omit if circular) */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_ORIENTATION)        /*  0x34 Ellipse orientation */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_POSITION_X)         /*  0x35 Center X touch position */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_POSITION_Y)         /*  0x36 Center Y touch position */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_TOOL_TYPE)          /*  0x37 Type of touching device */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_BLOB_ID)            /*  0x38 Group a set of packets as a blob */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_TRACKING_ID)        /*  0x39 Unique ID of initiated contact */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_PRESSURE)           /*  0x3a Pressure on contact area */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_DISTANCE)           /*  0x3b Contact hover distance */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_TOOL_X)             /*  0x3c Center X tool position */
XINPUT_GAMEPAD_ABS_NOPE(ABS_MT_TOOL_Y)             /*  0x3d Center Y tool position */
XINPUT_GAMEPAD_ABS_NOPE(0x3e)
XINPUT_GAMEPAD_ABS_NOPE(0x3f)
XINPUT_GAMEPAD_ABS_END(xbox360_abs)

XINPUT_GAMEPAD_KEY_TRANSLATOR(xbox360_key,
        BTN_A, BTN_THUMBR,                 /* first and last button mapped */
        XINPUT_GAMEPAD_A,                  /* map of the first button ... */
        XINPUT_GAMEPAD_B,
        0,
        XINPUT_GAMEPAD_X,
        XINPUT_GAMEPAD_Y,
        0,
        XINPUT_GAMEPAD_LEFT_SHOULDER,
        XINPUT_GAMEPAD_RIGHT_SHOULDER,
        0,
        0,
        XINPUT_GAMEPAD_BACK,
        XINPUT_GAMEPAD_START,
        XINPUT_GAMEPAD_GUIDE,
        XINPUT_GAMEPAD_LEFT_THUMB,
        XINPUT_GAMEPAD_RIGHT_THUMB);       /* map of the last button ... */

XINPUT_GAMEPAD_KEY_TRANSLATOR(xbox360_key_bis,
        BTN_TRIGGER_HAPPY1, BTN_TRIGGER_HAPPY4,
        XINPUT_GAMEPAD_DPAD_LEFT, /* 0x2c0 */
        XINPUT_GAMEPAD_DPAD_RIGHT,
        XINPUT_GAMEPAD_DPAD_UP,
        XINPUT_GAMEPAD_DPAD_DOWN);

static void xinput_linux_input_xboxpad2_input_event_to_gamepad(const struct input_event* ie, XINPUT_GAMEPAD_EX* gamepad)
{
    switch(ie->type)
    {
        case EV_KEY:
        {
            xinput_linux_input_translator_key_input_event_to_gamepad(&xbox360_key, ie, gamepad);
            xinput_linux_input_translator_key_input_event_to_gamepad(&xbox360_key_bis, ie, gamepad);
            break;
        }
        case EV_ABS:
        {
            xinput_linux_input_translator_abs_input_event_to_gamepad(&xbox360_abs, ie, gamepad);
            break;
        }
        default:
        {
            /* does not handle anything else ... yet */
            break;
        }
    }
}
struct xinput_linux_input_xboxpad2_data
{
    XINPUT_GAMEPAD_EX gamepad;
    XINPUT_VIBRATION vibration;
    int fd;
    int effect_id;
};

typedef struct xinput_linux_input_xboxpad2_data xinput_linux_input_xboxpad2_data;

static int xinput_linux_input_xboxpad2_read(struct xinput_gamepad_device* device)
{
    xinput_linux_input_xboxpad2_data* data = (xinput_linux_input_xboxpad2_data*)device->data;
    struct input_event ie;
    int ret = xinput_linux_input_read_next(data->fd, &ie);
    if(ret == 0)
    {
        xinput_linux_input_xboxpad2_input_event_to_gamepad(&ie, &data->gamepad);
    }

    return ret;
}

static void xinput_linux_input_xboxpad2_update(struct xinput_gamepad_device* device, XINPUT_GAMEPAD_EX* gamepad, XINPUT_VIBRATION* vibration)
{
    xinput_linux_input_xboxpad2_data* data = (xinput_linux_input_xboxpad2_data*)device->data;

    if(gamepad != NULL)
    {
        memcpy(gamepad, &data->gamepad, sizeof(*gamepad));
    }
    if(vibration != NULL)
    {
        memcpy(vibration, &data->vibration, sizeof(*vibration));
    }
}

static int xinput_linux_input_xboxpad2_rumble(struct xinput_gamepad_device* device, const XINPUT_VIBRATION* vibration)
{
    xinput_linux_input_xboxpad2_data* data = (xinput_linux_input_xboxpad2_data*)device->data;
    int id;
    id = xinput_linux_input_rumble(data->fd, data->effect_id, vibration->wLeftMotorSpeed, vibration->wRightMotorSpeed);
    if(id >= 0)
    {
        data->effect_id = id;
    }

    return 0;
}

static void xinput_linux_input_xboxpad2_release(struct xinput_gamepad_device* device)
{
    xinput_linux_input_xboxpad2_data* data = (xinput_linux_input_xboxpad2_data*)device->data;

    if(data->effect_id >= 0)
    {
        xinput_linux_input_feedback_clear(data->fd, data->effect_id);
        data->effect_id = -1;
    }
    close_ex(data->fd);
    data->fd = -1;
    free(data);
    device->data = NULL;
    device->vtbl = NULL;
}

static const xinput_gamepad_device_vtbl xinput_xboxpad2_vtbl =
{
    &xinput_linux_input_xboxpad2_read,
    &xinput_linux_input_xboxpad2_update,
    &xinput_linux_input_xboxpad2_rumble,
    &xinput_linux_input_xboxpad2_release
};

static void xinput_linux_input_xboxpad2_init(struct xinput_gamepad_device* instance, int fd)
{
    xinput_linux_input_xboxpad2_data* data = (xinput_linux_input_xboxpad2_data*)malloc(sizeof(xinput_linux_input_xboxpad2_data));
    memset(data, 0, sizeof(xinput_linux_input_xboxpad2_data));
    data->fd = fd;
    data->effect_id = -1;
    instance->data = data;
    instance->vtbl = &xinput_xboxpad2_vtbl;
}

static struct xinput_driver_supported_device xboxpad_factories[] =
{
    {
        "XBox360 Controller", xinput_linux_input_xboxpad2_init,
        MANUFACTURER_MICROSOFT, XBOX360_CONTROLLER
    },
    {
        "XBox360 Wireless Controller", xinput_linux_input_xboxpad2_init,
        MANUFACTURER_MICROSOFT, XBOX360_WIRELESS_CONTROLLER
    },
    {
        "XBoxOne Controller", xinput_linux_input_xboxpad2_init,
        MANUFACTURER_MICROSOFT, XBOXONE_CONTROLLER
    },
    {
        "XBoxOne Wireless Controller", xinput_linux_input_xboxpad2_init,
        MANUFACTURER_MICROSOFT, XBOXONE_WIRELESS_CONTROLLER
    },
    {
        NULL, NULL, 0, 0
    }
};

BOOL xinput_linux_input_xboxpad2_can_translate(const struct xinput_linux_input_probe_s* probed)
{
    for(int i = 0; xboxpad_factories[i].vendor != 0; ++i)
    {
        if( (xboxpad_factories[i].vendor == probed->id.vendor) &&
            (xboxpad_factories[i].product == probed->id.product) )
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL xinput_linux_input_xboxpad2_new_instance(const struct xinput_linux_input_probe_s* probed, int fd, xinput_gamepad_device* instance)
{
    for(int i = 0; i < 4; ++i)
    {
        if( (xboxpad_factories[i].vendor == probed->id.vendor) &&
            (xboxpad_factories[i].product == probed->id.product) )
        {
            xboxpad_factories[i].initialize(instance, fd);
            return TRUE;
        }
    }

    return FALSE;
}

#endif /* HAVE_LINUX_INPUT_H */
