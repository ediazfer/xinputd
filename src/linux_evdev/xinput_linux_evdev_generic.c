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
#include "xinput_linux_evdev_debug.h"

#include "xinput_linux_evdev_generic.h"
#include "xinput_linux_evdev_translator.h"

#include "xinput_linux_evdev.h"
#include "device_id.h"

WINE_DEFAULT_DEBUG_CHANNEL(xinput);

#define ABS_TLX 1
#define ABS_TLY 2
#define ABS_TRX 4
#define ABS_TRY 8
#define ABS_TL  16
#define ABS_TR  32
#define ABS_ALL 0x3f

static int xinput_gamepad_generic_translation[20][2] =
{
    { BTN_A,        XINPUT_GAMEPAD_A},
    { BTN_B,        XINPUT_GAMEPAD_B},
    { BTN_X,        XINPUT_GAMEPAD_X},
    { BTN_Y,        XINPUT_GAMEPAD_Y},
    
    { BTN_TL,       XINPUT_GAMEPAD_LEFT_SHOULDER},
    { BTN_TR,       XINPUT_GAMEPAD_RIGHT_SHOULDER},
    { BTN_SELECT,   XINPUT_GAMEPAD_BACK},
    { BTN_START,    XINPUT_GAMEPAD_START},
    
    { BTN_MODE,     XINPUT_GAMEPAD_GUIDE},
    { BTN_THUMBL,   XINPUT_GAMEPAD_LEFT_THUMB},
    { BTN_THUMBR,   XINPUT_GAMEPAD_RIGHT_THUMB},

    { BTN_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT},
    { BTN_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT},
    { BTN_DPAD_UP, XINPUT_GAMEPAD_DPAD_UP},  
    { BTN_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN},
    
    { BTN_TRIGGER_HAPPY1, XINPUT_GAMEPAD_DPAD_LEFT},
    { BTN_TRIGGER_HAPPY2, XINPUT_GAMEPAD_DPAD_RIGHT},
    { BTN_TRIGGER_HAPPY3, XINPUT_GAMEPAD_DPAD_UP},  
    { BTN_TRIGGER_HAPPY4, XINPUT_GAMEPAD_DPAD_DOWN},
    { 0, 0}
};

struct xinput_linux_evdev_generic_data
{
    struct xinput_linux_evdev_translator_abs_translator abs;    
    SHORT key_buttons[KEY_CNT];
    struct xinput_linux_evdev_translator_key_translator key;
    XINPUT_GAMEPAD_EX gamepad;
    XINPUT_VIBRATION vibration;
    int fd;
    int effect_id;
};

typedef struct xinput_linux_evdev_generic_data xinput_linux_evdev_generic_data;

static int xinput_linux_evdev_generic_read(struct xinput_gamepad_device* device)
{
    xinput_linux_evdev_generic_data* data = (xinput_linux_evdev_generic_data*)device->data;
    struct input_event ie;
    int ret = xinput_linux_evdev_read_next(data->fd, &ie);
    if(ret == 0)
    {
        switch(ie.type)
        {
            case EV_KEY:
            {
#if DEBUG_EVENTS
                TRACE("EVENT %04hx=%s %04hx=%s %08x\n",
                        ie->type, xinput_linux_evdev_event_type_get_name(ie->type),
                        ie->code, xinput_linux_evdev_key_get_name(ie->code),
                        ie->value
                        );
#endif
                xinput_linux_evdev_translator_key_input_event_to_gamepad(&data->key, &ie, &data->gamepad);

                break;
            }
            case EV_ABS:
            {
#if DEBUG_EVENTS
                TRACE("EVENT %04hx=%s %04hx=%s %08x\n",
                        ie->type, xinput_linux_evdev_event_type_get_name(ie->type),
                        ie->code, xinput_linux_evdev_abs_get_name(ie->code),
                        ie->value
                        );
#endif
                xinput_linux_evdev_translator_abs_input_event_to_gamepad(&data->abs, &ie, &data->gamepad);

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
                        ie->type, xinput_linux_evdev_event_type_get_name(ie->type),
                        ie->code,
                        ie->value
                        );
#endif
                break;
            }
        }
        
    }

    return ret;
}

static void xinput_linux_evdev_generic_update(struct xinput_gamepad_device* device, XINPUT_GAMEPAD_EX* gamepad, XINPUT_VIBRATION* vibration)
{
    xinput_linux_evdev_generic_data* data = (xinput_linux_evdev_generic_data*)device->data;

    if(gamepad != NULL)
    {
        memcpy(gamepad, &data->gamepad, sizeof(*gamepad));
    }
    if(vibration != NULL)
    {
        memcpy(vibration, &data->vibration, sizeof(*vibration));
    }
}

static int xinput_linux_evdev_generic_rumble(struct xinput_gamepad_device* device, const XINPUT_VIBRATION* vibration)
{
    xinput_linux_evdev_generic_data* data = (xinput_linux_evdev_generic_data*)device->data;
    int id;
    id = xinput_linux_evdev_rumble(data->fd, data->effect_id, vibration->wLeftMotorSpeed, vibration->wRightMotorSpeed);
    if(id >= 0)
    {
        data->effect_id = id;
    }

    return 0;
}

static void xinput_linux_evdev_generic_release(struct xinput_gamepad_device* device)
{
    xinput_linux_evdev_generic_data* data = (xinput_linux_evdev_generic_data*)device->data;
    
    TRACE("release %p", device);

    if(data->effect_id >= 0)
    {
        xinput_linux_evdev_feedback_clear(data->fd, data->effect_id);
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
    &xinput_linux_evdev_generic_read,
    &xinput_linux_evdev_generic_update,
    &xinput_linux_evdev_generic_rumble,
    &xinput_linux_evdev_generic_release
};

static void xinput_linux_evdev_generic_init(struct xinput_gamepad_device* device, int fd)
{
    xinput_linux_evdev_generic_data* data = (xinput_linux_evdev_generic_data*)malloc(sizeof(xinput_linux_evdev_generic_data));
    
    TRACE("init %p with fd %i", device, fd);
    
    memset(data, 0, sizeof(xinput_linux_evdev_generic_data));
    data->fd = fd;
    data->effect_id = -1;
    device->data = data;
    device->vtbl = &xinput_xboxpad_vtbl;
}

static BOOL xinput_linux_evdev_generic_translate(const struct xinput_linux_evdev_probe_s* probedp, xinput_linux_evdev_generic_data *data)
{
    struct xinput_linux_evdev_translator_abs_translator *abs;
    SHORT *key_buttons;
    SHORT local_key_buttons[KEY_CNT];
    WORD buttons = XINPUT_GAMEPAD_RESERVED0;
    static const WORD BUTTONS_ALL = ((WORD)~0);
    BYTE axis = 0;  // TLX, TLY, TRX, TRY, TL, TR : 1 -> 32
    uint8_t ev_key[KEY_CNT>>3];
    uint8_t ev_abs[ABS_CNT>>3];
    struct xinput_linux_evdev_translator_abs_translator local_abs;

    memcpy(ev_key, probedp->ev_key, sizeof(ev_key));
    memcpy(ev_abs, probedp->ev_abs, sizeof(ev_abs));
 
    if(data != NULL)
    {
        abs = &data->abs;
        key_buttons = &data->key_buttons[0];
        
        data->key._first = 0;
        data->key._last = KEY_MAX;
        data->key._buttons = data->key_buttons;
    }
    else
    {
        abs = &local_abs;
        key_buttons = &local_key_buttons[0];
    }
    
    
    memset(abs, 0, sizeof(local_abs));
    memset(key_buttons, 0, sizeof(local_key_buttons));
    
    // key
    
    for(int index = 0; xinput_gamepad_generic_translation[index][0] != 0; ++index)
    {
        int btn = xinput_gamepad_generic_translation[index][0];
        int pad = xinput_gamepad_generic_translation[index][1];
        
        if((buttons & pad) == 0)
        {
            if(bit_get(ev_key, btn))
            {
                bit_clear(ev_key, btn);
                key_buttons[btn] = pad;
                buttons |= pad;
            }
        }
    }
    
    // abs
    
    if(bit_get(ev_abs, ABS_X) && bit_get(ev_abs, ABS_Y))
    {
        bit_clear(ev_key, ABS_X);
        bit_clear(ev_key, ABS_Y);
        XINPUT_GAMEPAD_ABS_SET_AXIS(abs, ABS_X, sThumbLX);
        XINPUT_GAMEPAD_ABS_SET_SIXA(abs, ABS_Y, sThumbLY);
        axis |= ABS_TLX|ABS_TLY;
    }
    
    if(bit_get(ev_abs, ABS_RX) && bit_get(ev_abs, ABS_RY))
    {
        bit_clear(ev_key, ABS_RX);
        bit_clear(ev_key, ABS_RY);
        XINPUT_GAMEPAD_ABS_SET_AXIS(abs, ABS_RX, sThumbRX);
        XINPUT_GAMEPAD_ABS_SET_SIXA(abs, ABS_RY, sThumbRY);
        axis |= ABS_TRX|ABS_TRY;
    }
    
    if(bit_get(ev_abs, ABS_Z))
    {
        bit_clear(ev_key, ABS_Z);
        XINPUT_GAMEPAD_ABS_SET_AXIS(abs, ABS_Z, bLeftTrigger);
        axis |= ABS_TL;
    }
    
    if(bit_get(ev_abs, ABS_RZ))
    {
        bit_clear(ev_key, ABS_RZ);
        XINPUT_GAMEPAD_ABS_SET_AXIS(abs, ABS_RZ, bRightTrigger);
        axis |= ABS_TR;
    }
    
    // specific analogic to button translation
    
    if((buttons & (XINPUT_GAMEPAD_DPAD_RIGHT|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_UP)) == 0)
    {
        if(bit_get(ev_abs, ABS_HAT0X) && bit_get(ev_abs, ABS_HAT0Y))
        {
            bit_clear(ev_abs, ABS_HAT0X);
            bit_clear(ev_abs, ABS_HAT0Y);
            XINPUT_GAMEPAD_ABS_SET_BTTN(abs, ABS_HAT0X, XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_LEFT);
            XINPUT_GAMEPAD_ABS_SET_BTTN(abs, ABS_HAT0Y, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_UP);
            buttons |= XINPUT_GAMEPAD_DPAD_RIGHT|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_UP;
        }
        else if(bit_get(ev_abs, ABS_HAT1X) && bit_get(ev_abs, ABS_HAT1Y))
        {
            bit_clear(ev_abs, ABS_HAT1X);
            bit_clear(ev_abs, ABS_HAT1Y);
            XINPUT_GAMEPAD_ABS_SET_BTTN(abs, ABS_HAT1X, XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_LEFT);
            XINPUT_GAMEPAD_ABS_SET_BTTN(abs, ABS_HAT1Y, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_UP);
            buttons |= XINPUT_GAMEPAD_DPAD_RIGHT|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_UP;
        }
        else if(bit_get(ev_abs, ABS_HAT2X) && bit_get(ev_abs, ABS_HAT2Y))
        {
            bit_clear(ev_abs, ABS_HAT2X);
            bit_clear(ev_abs, ABS_HAT2Y);
            XINPUT_GAMEPAD_ABS_SET_BTTN(abs, ABS_HAT2X, XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_LEFT);
            XINPUT_GAMEPAD_ABS_SET_BTTN(abs, ABS_HAT2Y, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_UP);
            buttons |= XINPUT_GAMEPAD_DPAD_RIGHT|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_UP;
        }
        else if(bit_get(ev_abs, ABS_HAT3X) && bit_get(ev_abs, ABS_HAT3Y))
        {
            bit_clear(ev_abs, ABS_HAT3X);
            bit_clear(ev_abs, ABS_HAT3Y);
            XINPUT_GAMEPAD_ABS_SET_BTTN(abs, ABS_HAT3X, XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_LEFT);
            XINPUT_GAMEPAD_ABS_SET_BTTN(abs, ABS_HAT3Y, XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_UP);
            buttons |= XINPUT_GAMEPAD_DPAD_RIGHT|XINPUT_GAMEPAD_DPAD_LEFT|XINPUT_GAMEPAD_DPAD_DOWN|XINPUT_GAMEPAD_DPAD_UP;
        }
    }
    
    // the "obvious" have been set
    // now my on a first-come first-serve
    
    {
        static const WORD BUTTONS_ALL_MASK = (WORD)~0;
        int pad = 1;
        for(int btn_index = 0; (buttons != BUTTONS_ALL_MASK) && (btn_index < KEY_CNT); ++btn_index)
        {
            if(bit_get(ev_key, btn_index))
            {
                bit_clear(ev_key, btn_index);

                for(; pad <= 32768; pad <<= 1)
                {
                    if((buttons & pad) == 0)
                    {
                        key_buttons[btn_index] = pad;
                        buttons |= pad;
                        break;
                    }
                }
            }
        }
    }
    
    // done for the buttons
    
    {
        int ax = 1;
        for(int abs_index = 0; axis != ABS_ALL; ++abs_index)
        {
            if(bit_get(ev_abs, abs_index))
            {
                bit_clear(ev_abs, abs_index);
                
                for(; ax <= ABS_TR; ax <<= 1)
                {
                    if((axis & ax) == 0)
                    {
                        switch(ax)
                        {
                            case ABS_TLX:
                                XINPUT_GAMEPAD_ABS_SET_AXIS(abs, ax, sThumbLX);
                                break;
                            case ABS_TLY:
                                XINPUT_GAMEPAD_ABS_SET_SIXA(abs, ax, sThumbLY);
                                break;
                            case ABS_TRX:
                                XINPUT_GAMEPAD_ABS_SET_AXIS(abs, ax, sThumbRX);
                                break;
                            case ABS_TRY:
                                XINPUT_GAMEPAD_ABS_SET_SIXA(abs, ax, sThumbRY);
                                break;
                            case ABS_TL:
                                XINPUT_GAMEPAD_ABS_SET_AXIS(abs, ax, sThumbLX);
                                break;
                            case ABS_TR:
                                XINPUT_GAMEPAD_ABS_SET_AXIS(abs, ax, sThumbLY);
                                break;
                        }
                        
                        axis |= ax;
                    }
                }
            }
        }
    }
    
    
    
    return (buttons == BUTTONS_ALL) && (axis == ABS_ALL);
}

BOOL xinput_linux_evdev_generic_can_translate(const struct xinput_linux_evdev_probe_s* probed)
{
    return xinput_linux_evdev_generic_translate(probed, NULL);
}

BOOL xinput_linux_evdev_generic_new_instance(const struct xinput_linux_evdev_probe_s* probed, int fd, xinput_gamepad_device* instance)
{
    xinput_linux_evdev_generic_data *data = (xinput_linux_evdev_generic_data*)malloc(sizeof(xinput_linux_evdev_generic_data));
    BOOL ret;
    memset(data, 0, sizeof(xinput_linux_evdev_generic_data));
    ret = xinput_linux_evdev_generic_translate(probed, data);
    if(ret)
    {
        data->fd = fd;
        instance->data = data;
        instance->vtbl = &xinput_xboxpad_vtbl;
    }
    else
    {
        free(data);
    }
    return ret;
}

#endif /* HAVE_LINUX_INPUT_H */
