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

#ifndef XINPUT_GAMEPAD_H
#define XINPUT_GAMEPAD_H

#include "xinput.h"

#define XINPUT_GAMEPAD_LTRIGGER             0x00010000
#define XINPUT_GAMEPAD_RTRIGGER             0x00020000
#define XINPUT_GAMEPAD_RTRIGGER             0x00020000
#define XINPUT_GAMEPAD_LTHUMB_UP            0x00100000
#define XINPUT_GAMEPAD_LTHUMB_RIGHT         0x00200000
#define XINPUT_GAMEPAD_LTHUMB_DOWN          0x00400000
#define XINPUT_GAMEPAD_LTHUMB_LEFT          0x00800000
#define XINPUT_GAMEPAD_RTHUMB_UP            0x01000000
#define XINPUT_GAMEPAD_RTHUMB_RIGHT         0x02000000
#define XINPUT_GAMEPAD_RTHUMB_DOWN          0x04000000
#define XINPUT_GAMEPAD_RTHUMB_LEFT          0x08000000

#define XINPUT_GAMEPAD_LTHUMB_MASK          (XINPUT_GAMEPAD_LTHUMB_UP|    \
                                             XINPUT_GAMEPAD_LTHUMB_RIGHT| \
                                             XINPUT_GAMEPAD_LTHUMB_DOWN|  \
                                             XINPUT_GAMEPAD_LTHUMB_LEFT)

#define XINPUT_GAMEPAD_RTHUMB_MASK          (XINPUT_GAMEPAD_RTHUMB_UP|    \
                                             XINPUT_GAMEPAD_RTHUMB_RIGHT| \
                                             XINPUT_GAMEPAD_RTHUMB_DOWN|  \
                                             XINPUT_GAMEPAD_RTHUMB_LEFT)

#ifdef __cplusplus
extern "C" {
#endif

struct xinput_gamepad_device;

struct xinput_gamepad_device_vtbl
{
    int (*read)(struct xinput_gamepad_device* device);
    void (*update)(struct xinput_gamepad_device* device, XINPUT_GAMEPAD_EX* gamepad, XINPUT_VIBRATION* vibration);
    int (*rumble)(struct xinput_gamepad_device* device, const XINPUT_VIBRATION* vibration);
    void (*release)(struct xinput_gamepad_device* device);
};

typedef struct xinput_gamepad_device_vtbl xinput_gamepad_device_vtbl;

struct xinput_gamepad_device
{
    void* data;
    const xinput_gamepad_device_vtbl* vtbl;
};

typedef struct xinput_gamepad_device xinput_gamepad_device;

/**
 * A tool structure to easily store the supported devices
 */

struct xinput_driver_supported_device
{
    const char* name;
    void (*initialize)(struct xinput_gamepad_device* instance, int fd);

    const WORD vendor;
    const WORD product;
};

typedef struct xinput_driver_supported_device xinput_driver_supported_device;

void xinput_gamepad_init(void);
void xinput_gamepad_finalize(void);

BOOL xinput_gamepad_connected(int index);
BOOL xinput_gamepad_copy_buttons_state(int index, DWORD* out_buttons);
void xinput_gamepad_copy_state(int index, XINPUT_STATE* out_state);
void xinput_gamepad_copy_state_ex(int index, XINPUT_STATE_EX* out_state);
void xinput_gamepad_rumble(int index, const XINPUT_VIBRATION *vibration);

#ifdef __cplusplus
}
#endif

#endif /* XINPUT_GAMEPAD_H */
