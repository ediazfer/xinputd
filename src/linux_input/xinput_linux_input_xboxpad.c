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

#define DEBUG_EVENTS 0

#if HAVE_LINUX_INPUT_H

#include "xinput_settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#if HAVE_WINE
#include "wine/debug.h"
#include "winerror.h"
#endif

#include "xinput.h"
#include "tools.h"
#include "debug.h"
#include "xinput_linux_input_debug.h"

#include "xinput_linux_input_xboxpad.h"

#include "xinput_linux_input.h"
#include "device_id.h"

WINE_DEFAULT_DEBUG_CHANNEL(xinput);

static int xinput_gamepad_translation[16][2] = /*  minus 0x130 */
{
    { BTN_A,        XINPUT_GAMEPAD_A},  /* 0x130 */
    { BTN_B,        XINPUT_GAMEPAD_B},
    { BTN_C,        0               },
    { BTN_X,        XINPUT_GAMEPAD_X},
    { BTN_Y,        XINPUT_GAMEPAD_Y},
    { BTN_Z,        0},
    { BTN_TL,       XINPUT_GAMEPAD_LEFT_SHOULDER},
    { BTN_TR,       XINPUT_GAMEPAD_RIGHT_SHOULDER},
    { BTN_TL2,      0},                 /* 0x138 */
    { BTN_TR2,      0},
    { BTN_SELECT,   XINPUT_GAMEPAD_BACK},
    { BTN_START,    XINPUT_GAMEPAD_START},
    { BTN_MODE,     XINPUT_GAMEPAD_GUIDE}, /* is that it ? */
    { BTN_THUMBL,   XINPUT_GAMEPAD_LEFT_THUMB},
    { BTN_THUMBR,   XINPUT_GAMEPAD_RIGHT_THUMB},
    { 0x13f,        0}
};

/* this is how the "EU" xbox360 wireless gamepad is mapped with linux input */

static int xinput_gamepad_translation2[4][2] =     /*  minus 0x2c0 */
{
    { BTN_TRIGGER_HAPPY1, XINPUT_GAMEPAD_DPAD_LEFT}, /* 0x2c0 */
    { BTN_TRIGGER_HAPPY2, XINPUT_GAMEPAD_DPAD_RIGHT},
    { BTN_TRIGGER_HAPPY3, XINPUT_GAMEPAD_DPAD_UP},  
    { BTN_TRIGGER_HAPPY4, XINPUT_GAMEPAD_DPAD_DOWN}
};

/*
 * Another way to translate is to do everything manually :
 *
 */

static void xinput_linux_input_xboxpad_input_event_to_gamepad(const struct input_event* ie, XINPUT_GAMEPAD_EX* gamepad)
{
    switch(ie->type)
    {
        case EV_KEY:
        {
#if DEBUG_EVENTS
            TRACE("EVENT %04hx=%s %04hx=%s %08x\n",
                    ie->type, xinput_linux_input_event_type_get_name(ie->type),
                    ie->code, xinput_linux_input_key_get_name(ie->code),
                    ie->value
                    );
#endif

            if(ie->code >= BTN_GAMEPAD && ie->code < BTN_DIGI)
            {
                uint16_t mask = xinput_gamepad_translation[ie->code - BTN_GAMEPAD][1];
                if(ie->value == 1)
                {
                    gamepad->wButtons |= mask;
                }
                else
                {
                    gamepad->wButtons &= ~mask;
                }
            } /* one of my wireless 360 gamepads is mapped this way ... */
            else if(ie->code >= BTN_TRIGGER_HAPPY1 && ie->code <= BTN_TRIGGER_HAPPY4)
            {
                uint16_t mask = xinput_gamepad_translation2[ie->code - BTN_TRIGGER_HAPPY1][1];
                if(ie->value == 1)
                {
                    gamepad->wButtons |= mask;
                }
                else
                {
                    gamepad->wButtons &= ~mask;
                }
            }

            break;
        }
        case EV_ABS:
        {
#if DEBUG_EVENTS
            TRACE("EVENT %04hx=%s %04hx=%s %08x\n",
                    ie->type, xinput_linux_input_event_type_get_name(ie->type),
                    ie->code, xinput_linux_input_abs_get_name(ie->code),
                    ie->value
                    );
#endif
            switch(ie->code)
            {
                case ABS_X:
                {
                    gamepad->sThumbLX = (int16_t)ie->value;
                    break;
                }
                case ABS_Y:
                {   /*  -32768 =  32767 8000 = 7fff */
                    /*   32767 = -32768 7fff = 8000 */
                    gamepad->sThumbLY = ~(int16_t)ie->value;
                    break;
                }
                case ABS_RX:
                {
                    gamepad->sThumbRX = (int16_t)ie->value;
                    break;
                }
                case ABS_RY:
                {
                    gamepad->sThumbRY = ~(int16_t)ie->value;
                    break;
                }
                case ABS_Z:
                {
                    gamepad->bLeftTrigger = (int8_t)ie->value;
                    break;
                }
                case ABS_RZ:
                {
                    gamepad->bRightTrigger = (int16_t)ie->value;
                    break;
                }
                case ABS_HAT0X:
                {
                    if(ie->value > 0)
                    {
                        gamepad->wButtons |=  XINPUT_GAMEPAD_DPAD_RIGHT;
                        gamepad->wButtons &= ~XINPUT_GAMEPAD_DPAD_LEFT;
                    }
                    else if(ie->value < 0)
                    {
                        gamepad->wButtons &= ~XINPUT_GAMEPAD_DPAD_RIGHT;
                        gamepad->wButtons |=  XINPUT_GAMEPAD_DPAD_LEFT;
                    }
                    else
                    {
                        gamepad->wButtons &= ~(XINPUT_GAMEPAD_DPAD_RIGHT | XINPUT_GAMEPAD_DPAD_LEFT);
                    }
                    break;
                }
                case ABS_HAT0Y:
                {
                    if(ie->value > 0)
                    {
                        gamepad->wButtons |=  XINPUT_GAMEPAD_DPAD_DOWN;
                        gamepad->wButtons &= ~XINPUT_GAMEPAD_DPAD_UP;
                    }
                    else if(ie->value < 0)
                    {
                        gamepad->wButtons &= ~XINPUT_GAMEPAD_DPAD_DOWN;
                        gamepad->wButtons |=  XINPUT_GAMEPAD_DPAD_UP;
                    }
                    else
                    {
                        gamepad->wButtons &= ~(XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_UP);
                    }
                    break;
                }
            }

            break;
        }
        case EV_FF:
        {
            /* break; */
        }
        case EV_SYN:
        {
            /* break; */
        }
        default:
        {
#if DEBUG_EVENTS
            TRACE("EVENT %04hx=%s %04hx %08x\n",
                    ie->type, xinput_linux_input_event_type_get_name(ie->type),
                    ie->code,
                    ie->value
                    );
#endif
            break;
        }
    }
}

struct xinput_linux_input_xboxpad_data
{
    XINPUT_GAMEPAD_EX gamepad;
    XINPUT_VIBRATION vibration;
    int fd;
    int effect_id;
};

typedef struct xinput_linux_input_xboxpad_data xinput_linux_input_xboxpad_data;

static int xinput_linux_input_xboxpad_read(struct xinput_gamepad_device* device)
{
    xinput_linux_input_xboxpad_data* data = (xinput_linux_input_xboxpad_data*)device->data;
    struct input_event ie;
    int ret = xinput_linux_input_read_next(data->fd, &ie);
    if(ret == 0)
    {
        xinput_linux_input_xboxpad_input_event_to_gamepad(&ie, &data->gamepad);
    }

    return ret;
}

static void xinput_linux_input_xboxpad_update(struct xinput_gamepad_device* device, XINPUT_GAMEPAD_EX* gamepad, XINPUT_VIBRATION* vibration)
{
    xinput_linux_input_xboxpad_data* data = (xinput_linux_input_xboxpad_data*)device->data;

    if(gamepad != NULL)
    {
        memcpy(gamepad, &data->gamepad, sizeof(*gamepad));
    }
    if(vibration != NULL)
    {
        memcpy(vibration, &data->vibration, sizeof(*vibration));
    }
}

static int xinput_linux_input_xboxpad_rumble(struct xinput_gamepad_device* device, const XINPUT_VIBRATION* vibration)
{
    xinput_linux_input_xboxpad_data* data = (xinput_linux_input_xboxpad_data*)device->data;
    int id;
    id = xinput_linux_input_rumble(data->fd, data->effect_id, vibration->wLeftMotorSpeed, vibration->wRightMotorSpeed);
    if(id >= 0)
    {
        data->effect_id = id;
    }

    return 0;
}

static void xinput_linux_input_xboxpad_release(struct xinput_gamepad_device* device)
{
    xinput_linux_input_xboxpad_data* data = (xinput_linux_input_xboxpad_data*)device->data;
    
    TRACE("release %p", device);

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

static const xinput_gamepad_device_vtbl xinput_xboxpad_vtbl =
{
    &xinput_linux_input_xboxpad_read,
    &xinput_linux_input_xboxpad_update,
    &xinput_linux_input_xboxpad_rumble,
    &xinput_linux_input_xboxpad_release
};

static void xinput_linux_input_xboxpad_init(struct xinput_gamepad_device* device, int fd)
{
    xinput_linux_input_xboxpad_data* data = (xinput_linux_input_xboxpad_data*)malloc(sizeof(xinput_linux_input_xboxpad_data));
    
    TRACE("init %p with fd %i", device, fd);
    
    memset(data, 0, sizeof(xinput_linux_input_xboxpad_data));
    data->fd = fd;
    data->effect_id = -1;
    device->data = data;
    device->vtbl = &xinput_xboxpad_vtbl;
}

static struct xinput_driver_supported_device xboxpad_factories[] =
{
    {
        "XBox360 Controller", xinput_linux_input_xboxpad_init,
        MANUFACTURER_MICROSOFT, XBOX360_CONTROLLER
    },
    {
        "XBox360 Wireless Controller", xinput_linux_input_xboxpad_init,
        MANUFACTURER_MICROSOFT, XBOX360_WIRELESS_CONTROLLER
    },
    {
        "XBox360 Wireless Controller", xinput_linux_input_xboxpad_init,
        MANUFACTURER_MICROSOFT, XBOX360_WIRELESS_CONTROLLER_EU
    },
    {
        "XBoxOne Controller", xinput_linux_input_xboxpad_init,
        MANUFACTURER_MICROSOFT, XBOXONE_CONTROLLER
    },
    {
        "XBoxOne Wireless Controller", xinput_linux_input_xboxpad_init,
        MANUFACTURER_MICROSOFT, XBOXONE_WIRELESS_CONTROLLER
    },
    {
        NULL, NULL, 0, 0
    }
};

BOOL xinput_linux_input_xboxpad_can_translate(const struct xinput_linux_input_probe_s* probed)
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

BOOL xinput_linux_input_xboxpad_new_instance(const struct xinput_linux_input_probe_s* probed, int fd, xinput_gamepad_device* instance)
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
