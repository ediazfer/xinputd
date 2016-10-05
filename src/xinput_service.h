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

#ifndef XINPUT_SERVICE_H
#define XINPUT_SERVICE_H

#include "xinput.h"
#include <stdint.h>

#ifndef XUSER_MAX_COUNT
#define XUSER_MAX_COUNT 4
#endif

#define THUMB_TO_BUTTONS_THRESHOLD 16384
#define TRIGGER_TO_BUTTONS_THRESHOLD 32

#define SERVICE_NAME "/xinput"
#define SERVICE_SHM_NAME SERVICE_NAME "shm"
#define SERVICE_SEM_NAME SERVICE_NAME "mtx"
#define SERVICE_MSG_NAME SERVICE_NAME "msg"
#define SERVICE_LCK_NAME SERVICE_NAME "lck"

#define XINPUT_OWNER_BROKEN ((pid_t)~0)

#if XINPUT_USES_MQUEUE
#define MQD_INVALID ((mqd_t)-1)
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct xinput_gamepad_state
{
    XINPUT_GAMEPAD_EX gamepad;          /* 16 bytes */
    XINPUT_VIBRATION vibration;        /* 4 bytes  */
    volatile DWORD dwPacketNumber;      /* 4 bytes  */
    volatile BOOL connected;           /* 4 bytes  */
    DWORD _padding_reserved_0;          /* 4 bytes  */
    /* 32 bytes mark, a reasonable size for a L1 line (half, or equal) */
};

typedef struct xinput_gamepad_state xinput_gamepad_state;

struct xinput_shared_gamepad_state
{
    xinput_gamepad_state state[XUSER_MAX_COUNT]; // 128 bytes
    volatile DWORD master_pid;
    char _padding_reserved_0[60];
    volatile int64_t poke_us;
    char _padding_reserved_1[56];
};

typedef struct xinput_shared_gamepad_state xinput_shared_gamepad_state;

struct xinput_gamepad_vibration
{
    XINPUT_VIBRATION vibration;
    int index;
};

typedef struct xinput_gamepad_vibration xinput_gamepad_vibration;

BOOL xinput_service_self(void);

void xinput_service_set_auto_disconnect(BOOL enable);

void xinput_service_rundll(void);

void xinput_service_server(void);

/**
 * Returns EEXIST if the server is up (and running)
 * Returns ENOENT if the server is not running.
 * Returns some other errors : give up.
 *
 * This is a costly call and should only be used by the service itself at
 * initialization.
 *
 * @return
 */

int xinput_service_poke(void);

/**
 * The service will stop after this many detections of inactivity.
 * 0 to disable
 * 
 * @param strikes
 * @return 
 */

int xinput_service_set_autoshutdown(int strikes);

#ifdef __cplusplus
}
#endif

#endif /* XINPUT_SERVICE_H */
