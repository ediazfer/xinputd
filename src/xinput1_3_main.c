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
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#if HAVE_WINE
#include "wine/debug.h"
#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#endif

#include "xinput.h"
#include "xinput_service.h"
#include "xinput_gamepad.h"
#include "tools.h"

#include "debug.h"

#if HAVE_LINUX_INPUT_H
#define XINPUT_SUPPORTED 1
#else
#define XINPUT_SUPPORTED 0
#endif

#if !HAVE_WINE
#define WINAPI
#define DECLSPEC_HOTPATCH
#endif

#if HAVE_WINE

WINE_DEFAULT_DEBUG_CHANNEL(xinput);

#ifndef DLL_WINE_PREATTACH
#define DLL_WINE_PREATTACH 8
#endif

/**
 * When a DLL is spawned,
 * Try to find the shared memory.
 * If it exists
 */

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved) {
    (void) reserved;

    TRACE("DllMain(%p, %i, %p)\n", inst, reason, reserved);

    switch (reason) {
        case DLL_WINE_PREATTACH:
            return FALSE; /* prefer native version */
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(inst);
            break;
        case DLL_PROCESS_DETACH:
#if XINPUT_SUPPORTED
            xinput_gamepad_finalize();
#endif
            break;
    }

    return TRUE;
}

#endif

#if XINPUT_SUPPORTED
pthread_mutex_t xinput_enabled_mtx = PTHREAD_MUTEX_INITIALIZER;
static volatile BOOL xinput_enabled = TRUE;

static BOOL WINAPI XInputIsEnabled(void) {
    BOOL ret;
    pthread_mutex_lock(&xinput_enabled_mtx);
    ret = xinput_enabled;
    pthread_mutex_unlock(&xinput_enabled_mtx);
    return ret;
}

#endif

void WINAPI XInputEnable(BOOL enable) {
#if XINPUT_SUPPORTED
#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputEnable(%d), pid=%i\n", enable, getpid());
#endif

    pthread_mutex_lock(&xinput_enabled_mtx);
    xinput_enabled = enable;
    pthread_mutex_unlock(&xinput_enabled_mtx);

    if (!enable) {
        static const XINPUT_VIBRATION still = {0, 0};

        for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
            if (xinput_gamepad_connected(i)) {
                xinput_gamepad_rumble(i, &still);
            }
        }
    }

#else
    FIXME("XInputEnable(%d) stub\n", enable);
#endif
}

DWORD WINAPI XInputSetState(DWORD index, XINPUT_VIBRATION* vibration) {
#if XINPUT_SUPPORTED
#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputSetState(%d, %p), pid=%i\n", index, vibration, getpid());
#endif

    if (index >= XUSER_MAX_COUNT) {
        return ERROR_BAD_ARGUMENTS;
    }

    if (!xinput_gamepad_connected(index)) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    xinput_gamepad_rumble(index, vibration);

    return ERROR_SUCCESS;
#else
    FIXME("XInputSetState(%d, %p) stub\n", index, vibration);
    return ERROR_NOT_SUPPORTED;
#endif
}

DWORD WINAPI DECLSPEC_HOTPATCH XInputGetState(DWORD index, XINPUT_STATE* state) {
#if XINPUT_SUPPORTED
#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputGetState(%d, %p), pid=%i\n", index, state, getpid());
#endif

    if (index >= XUSER_MAX_COUNT) {
        return ERROR_BAD_ARGUMENTS;
    }

    if (!xinput_gamepad_connected(index)) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    if (XInputIsEnabled()) {
        xinput_gamepad_copy_state(index, state);
    } else {
        memset(&state->Gamepad, 0, sizeof (state->Gamepad));
    }

    return ERROR_SUCCESS;
#else
    FIXME("XInputGetState(%d, %p)\n", index, state);
    return ERROR_NOT_SUPPORTED;
#endif
}

DWORD WINAPI DECLSPEC_HOTPATCH XInputGetStateEx(DWORD index, XINPUT_STATE_EX* state_ex) {
#if XINPUT_SUPPORTED
#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputGetStateEx(%d, %p), pid=%i\n", index, state_ex, getpid());
#endif

    if (index >= XUSER_MAX_COUNT) {
        return ERROR_BAD_ARGUMENTS;
    }

    if (!xinput_gamepad_connected(index)) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    if (XInputIsEnabled()) {
        xinput_gamepad_copy_state_ex(index, state_ex);
    } else {
        memset(&state_ex->Gamepad, 0, sizeof (state_ex->Gamepad));
    }

#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputGetState(%d, %p) = ERROR_SUCCESS\n", index, state_ex);
#endif

    return ERROR_SUCCESS;
#else
    FIXME("XInputGetStateEx(%d, %p)\n", index, state_ex);
    return ERROR_NOT_SUPPORTED;
#endif
}

static DWORD xinputkeystroke_state[XUSER_MAX_COUNT] = {0, 0, 0, 0};
static int xinputkeystroke_any_first = 0;

#define XINPUTKEYSTROKE_VK_BUTTON_SIZE 33

struct xinputkeystroke_mask_to_vk {
    DWORD bits;
    DWORD mask;
    WORD value;
};

static struct xinputkeystroke_mask_to_vk xinputkeystroke_vk_button[XINPUTKEYSTROKE_VK_BUTTON_SIZE] ={
    {XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_UP, VK_PAD_DPAD_UP}, /* 0 */
    {XINPUT_GAMEPAD_DPAD_DOWN, XINPUT_GAMEPAD_DPAD_DOWN, VK_PAD_DPAD_DOWN},
    {XINPUT_GAMEPAD_DPAD_LEFT, XINPUT_GAMEPAD_DPAD_LEFT, VK_PAD_DPAD_LEFT},
    {XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_RIGHT, VK_PAD_DPAD_RIGHT},

    {XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_START, VK_PAD_START}, /* 4 */
    {XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_BACK, VK_PAD_BACK},
    {XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_LEFT_THUMB, VK_PAD_LTHUMB_PRESS},
    {XINPUT_GAMEPAD_RIGHT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB, VK_PAD_RTHUMB_PRESS},

    {XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_LEFT_SHOULDER, VK_PAD_LSHOULDER}, /* 8 */
    {XINPUT_GAMEPAD_RIGHT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER, VK_PAD_RSHOULDER},

    {XINPUT_GAMEPAD_A, XINPUT_GAMEPAD_A, VK_PAD_A},
    {XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_B, VK_PAD_B},
    {XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_X, VK_PAD_X}, /* 12 */
    {XINPUT_GAMEPAD_Y, XINPUT_GAMEPAD_Y, VK_PAD_Y},

    {XINPUT_GAMEPAD_LTRIGGER, XINPUT_GAMEPAD_LTRIGGER, VK_PAD_LTRIGGER},
    {XINPUT_GAMEPAD_RTRIGGER, XINPUT_GAMEPAD_RTRIGGER, VK_PAD_RTRIGGER},

    /* Do not change the order in a THUMB group */

    {XINPUT_GAMEPAD_LTHUMB_UP | XINPUT_GAMEPAD_LTHUMB_LEFT, XINPUT_GAMEPAD_LTHUMB_MASK, VK_PAD_LTHUMB_UPLEFT}, /* 16 */
    {XINPUT_GAMEPAD_LTHUMB_UP | XINPUT_GAMEPAD_LTHUMB_RIGHT, XINPUT_GAMEPAD_LTHUMB_MASK, VK_PAD_LTHUMB_UPRIGHT},
    {XINPUT_GAMEPAD_LTHUMB_DOWN | XINPUT_GAMEPAD_LTHUMB_RIGHT, XINPUT_GAMEPAD_LTHUMB_MASK, VK_PAD_LTHUMB_DOWNRIGHT},
    {XINPUT_GAMEPAD_LTHUMB_DOWN | XINPUT_GAMEPAD_LTHUMB_LEFT, XINPUT_GAMEPAD_LTHUMB_MASK, VK_PAD_LTHUMB_DOWNLEFT},
    {XINPUT_GAMEPAD_LTHUMB_UP, XINPUT_GAMEPAD_LTHUMB_MASK, VK_PAD_LTHUMB_UP},
    {XINPUT_GAMEPAD_LTHUMB_DOWN, XINPUT_GAMEPAD_LTHUMB_MASK, VK_PAD_LTHUMB_DOWN},
    {XINPUT_GAMEPAD_LTHUMB_LEFT, XINPUT_GAMEPAD_LTHUMB_MASK, VK_PAD_LTHUMB_LEFT},
    {XINPUT_GAMEPAD_LTHUMB_RIGHT, XINPUT_GAMEPAD_LTHUMB_MASK, VK_PAD_LTHUMB_RIGHT},

    /* Do not change the order in a THUMB group */

    {XINPUT_GAMEPAD_RTHUMB_UP | XINPUT_GAMEPAD_RTHUMB_LEFT, XINPUT_GAMEPAD_RTHUMB_MASK, VK_PAD_RTHUMB_UPLEFT}, /* 24 */
    {XINPUT_GAMEPAD_RTHUMB_UP | XINPUT_GAMEPAD_RTHUMB_RIGHT, XINPUT_GAMEPAD_RTHUMB_MASK, VK_PAD_RTHUMB_UPRIGHT},
    {XINPUT_GAMEPAD_RTHUMB_DOWN | XINPUT_GAMEPAD_RTHUMB_RIGHT, XINPUT_GAMEPAD_RTHUMB_MASK, VK_PAD_RTHUMB_DOWNRIGHT},
    {XINPUT_GAMEPAD_RTHUMB_DOWN | XINPUT_GAMEPAD_RTHUMB_LEFT, XINPUT_GAMEPAD_RTHUMB_MASK, VK_PAD_RTHUMB_DOWNLEFT},
    {XINPUT_GAMEPAD_RTHUMB_UP, XINPUT_GAMEPAD_RTHUMB_MASK, VK_PAD_RTHUMB_UP},
    {XINPUT_GAMEPAD_RTHUMB_DOWN, XINPUT_GAMEPAD_RTHUMB_MASK, VK_PAD_RTHUMB_DOWN},
    {XINPUT_GAMEPAD_RTHUMB_LEFT, XINPUT_GAMEPAD_RTHUMB_MASK, VK_PAD_RTHUMB_LEFT},
    {XINPUT_GAMEPAD_RTHUMB_RIGHT, XINPUT_GAMEPAD_RTHUMB_MASK, VK_PAD_RTHUMB_RIGHT},

    {XINPUT_GAMEPAD_GUIDE, XINPUT_GAMEPAD_GUIDE, VK_PAD_GUIDE} /* 32 */
};

/**
 * Computes the diff from a set of pressed buttons to another
 * Only returns the first match
 * Always find the released buttons first
 * Sets the keystroke of the found diff
 * Returns -1 if nothing change was found
 *
 * @param index
 * @param from
 * @param to
 * @param next the new (intermediary) buttons state
 * @param keystroke
 * @return
 */

static int XInputGetNextKeyStroke(int index, DWORD from, DWORD to, DWORD* next, PXINPUT_KEYSTROKE keystroke) {
    /* first, look for the released ones */
    int i;

    if (from == to) {
        return -1;
    }

    for (i = 0; i < XINPUTKEYSTROKE_VK_BUTTON_SIZE; ++i) {
        DWORD bits = xinputkeystroke_vk_button[i].bits;
        DWORD mask = xinputkeystroke_vk_button[i].mask;

        BOOL is = (to & mask) == bits;

        if (!is) {
            BOOL was = (from & mask) == bits;

            if (was != is) {
                /* change */

                *next = from & ~mask;
                keystroke->VirtualKey = xinputkeystroke_vk_button[i].value;
                keystroke->Unicode = 0;
                keystroke->Flags = XINPUT_KEYSTROKE_KEYUP;
                keystroke->UserIndex = index;
                keystroke->HidCode = 0;
                return i;
            }
        }
    }

    /* then, look for the pressed ones */

    for (i = 0; i < XINPUTKEYSTROKE_VK_BUTTON_SIZE; ++i) {
        DWORD bits = xinputkeystroke_vk_button[i].bits;
        DWORD mask = xinputkeystroke_vk_button[i].mask;

        BOOL is = (to & mask) == bits;

        if (is) {
            BOOL was = (from & mask) == bits;

            if (was != is) {
                /* change */

                *next = (from & ~mask) | bits;
                keystroke->VirtualKey = xinputkeystroke_vk_button[i].value;
                keystroke->Unicode = 0;
                keystroke->Flags = XINPUT_KEYSTROKE_KEYDOWN;
                keystroke->UserIndex = index;
                keystroke->HidCode = 0;
                return i;
            }
        }
    }

    /* no change */

    return -1;
}

/**
 * As no time/serial is required, this function determines the stroke with
 * diffs between states.
 *
 * @param index
 * @param reserved
 * @param keystroke
 * @return
 */

DWORD WINAPI XInputGetKeystroke(DWORD index, DWORD reserved, PXINPUT_KEYSTROKE keystroke) {
    DWORD buttons;

#if XINPUT_SUPPORTED
#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputGetKeystroke(%d, %d, %p), pid=%i\n", index, reserved, keystroke, getpid());
#endif
    (void) reserved;

    if (index < XUSER_MAX_COUNT) {
        if (!xinput_gamepad_connected(index)) {
            return ERROR_DEVICE_NOT_CONNECTED;
        }

        if (!xinput_gamepad_copy_buttons_state(index, &buttons)) {
            return ERROR_EMPTY;
        }

        /* find the first bit of difference */

        if (XInputGetNextKeyStroke(index, xinputkeystroke_state[index], buttons, &xinputkeystroke_state[index], keystroke) >= 0) {
            return ERROR_SUCCESS;
        }

        return ERROR_EMPTY;
    } else if (index == XUSER_INDEX_ANY) {
        /*
         * for each device, look at the one that got the oldest unknown update
         * loosely (a.k.a : without mutex) the first device checked in order
         * to be a bit fair
         */

        int index = xinputkeystroke_any_first++;

        for (int i = index; i < XUSER_MAX_COUNT; ++i) {
            if (XInputGetKeystroke(index, reserved, keystroke) == ERROR_SUCCESS) {
                return ERROR_SUCCESS;
            }
        }

        for (int i = 0; i < index; ++i) {
            if (XInputGetKeystroke(index, reserved, keystroke) == ERROR_SUCCESS) {
                return ERROR_SUCCESS;
            }
        }

        return ERROR_EMPTY;
    } else {
        return ERROR_BAD_ARGUMENTS;
    }

    return ERROR_NOT_SUPPORTED;
#else
    TRACE("XInputGetKeystroke(%d, %d, %p)\n", index, reserved, keystroke);
    return ERROR_NOT_SUPPORTED;
#endif
}

DWORD WINAPI XInputGetCapabilities(DWORD index, DWORD flags, XINPUT_CAPABILITIES* capabilities) {
#if XINPUT_SUPPORTED
    static XINPUT_GAMEPAD XINPUT_GAMEPAD_CAPS ={
        0xf7ff,
        0xff,
        0xff,
        0x7fff,
        0x7fff,
        0x7fff,
        0x7fff
    };

    static XINPUT_VIBRATION XINPUT_VIBRATION_CAPS ={
        0xffff,
        0xffff,
    };

    if (index >= XUSER_MAX_COUNT) {
#if XINPUT_TRACE_INTERFACE_USE
        TRACE("XInputGetCapabilities(%i, %x, %p) = ERROR_BAD_ARGUMENTS\n", index, flags, capabilities);
#endif
        return ERROR_BAD_ARGUMENTS;
    }

#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputGetCapabilities(%i, %x, %p), id=%i\n", index, flags, capabilities, getpid());
#endif

    if (!xinput_gamepad_connected(index)) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    capabilities->Type = XINPUT_DEVTYPE_GAMEPAD;
    capabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;
    capabilities->Flags = 0;
    capabilities->Gamepad = XINPUT_GAMEPAD_CAPS;
    capabilities->Vibration = XINPUT_VIBRATION_CAPS;

#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputGetCapabilities(%i, %x, %p) = ERROR_SUCCESS\n", index, flags, capabilities);
#endif

    return ERROR_SUCCESS;
#else
    TRACE("XInputGetCapabilities(%i, %x, %p)\n", index, flags, capabilities);
    return ERROR_NOT_SUPPORTED;
#endif
}

static GUID guid_null = {0, 0, 0,
    {0, 0, 0, 0, 0, 0, 0, 0}};

DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD index, GUID* render_guid, GUID* capture_guid) {
#if XINPUT_SUPPORTED
#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputGetDSoundAudioDeviceGuids(%d, %p, %p), pid=%i\n", index, render_guid, capture_guid, getpid());
#endif

    (void) render_guid;
    (void) capture_guid;

    if (index >= XUSER_MAX_COUNT) {
        return ERROR_BAD_ARGUMENTS;
    }

    if (!xinput_gamepad_connected(index)) {
        return ERROR_DEVICE_NOT_CONNECTED;
    }

    *render_guid = guid_null;
    *capture_guid = guid_null;

    return ERROR_SUCCESS;

    /* return ERROR_NOT_SUPPORTED; */
#else
    TRACE("XInputGetDSoundAudioDeviceGuids(%d, %p, %p)\n", index, render_guid, capture_guid);
    return ERROR_NOT_SUPPORTED;
#endif
}

DWORD WINAPI XInputGetBatteryInformation(DWORD index, BYTE type, XINPUT_BATTERY_INFORMATION* battery) {
#if XINPUT_SUPPORTED
#if XINPUT_TRACE_INTERFACE_USE
    TRACE("XInputGetBatteryInformation(%d, %hhi, %p), pid=%i\n", index, type, battery, getpid());
#endif
    (void) type;

    if (index >= XUSER_MAX_COUNT) {
        return ERROR_BAD_ARGUMENTS;
    }

    if (xinput_gamepad_connected(index)) {
        battery->BatteryType = BATTERY_TYPE_UNKNOWN;
        battery->BatteryLevel = BATTERY_LEVEL_FULL;

        return ERROR_SUCCESS;
    } else {
        battery->BatteryType = BATTERY_TYPE_DISCONNECTED;
        battery->BatteryLevel = BATTERY_LEVEL_EMPTY;
        return ERROR_DEVICE_NOT_CONNECTED;
    }

#else
    TRACE("XInputGetBatteryInformation(%d, %hhi, %p)\n", index, type, battery);
    return ERROR_NOT_SUPPORTED;
#endif
}

#if HAVE_WINE

void CALLBACK XInputServer(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
    (void) hwnd;
    (void) hinst;
    (void) lpszCmdLine;
    (void) nCmdShow;

    xinput_service_server();
}

#endif

#if SELF_TEST

#include <unistd.h>

/**
 * All the source can be compiled as a program using -DSELF_TEST.
 */

static const char* get_vk_text(int vk) {
#define CASEIDTOTEXT(code) case code: return #code
    switch (vk) {
            CASEIDTOTEXT(VK_PAD_A);
            CASEIDTOTEXT(VK_PAD_B);
            CASEIDTOTEXT(VK_PAD_X);
            CASEIDTOTEXT(VK_PAD_Y);
            CASEIDTOTEXT(VK_PAD_RSHOULDER);
            CASEIDTOTEXT(VK_PAD_LSHOULDER);
            CASEIDTOTEXT(VK_PAD_LTRIGGER);
            CASEIDTOTEXT(VK_PAD_RTRIGGER);
            CASEIDTOTEXT(VK_PAD_DPAD_UP);
            CASEIDTOTEXT(VK_PAD_DPAD_DOWN);
            CASEIDTOTEXT(VK_PAD_DPAD_LEFT);
            CASEIDTOTEXT(VK_PAD_DPAD_RIGHT);
            CASEIDTOTEXT(VK_PAD_START);
            CASEIDTOTEXT(VK_PAD_BACK);
            CASEIDTOTEXT(VK_PAD_LTHUMB_PRESS);
            CASEIDTOTEXT(VK_PAD_RTHUMB_PRESS);
            CASEIDTOTEXT(VK_PAD_LTHUMB_UP);
            CASEIDTOTEXT(VK_PAD_LTHUMB_DOWN);
            CASEIDTOTEXT(VK_PAD_LTHUMB_RIGHT);
            CASEIDTOTEXT(VK_PAD_LTHUMB_LEFT);
            CASEIDTOTEXT(VK_PAD_LTHUMB_UPLEFT);
            CASEIDTOTEXT(VK_PAD_LTHUMB_UPRIGHT);
            CASEIDTOTEXT(VK_PAD_LTHUMB_DOWNRIGHT);
            CASEIDTOTEXT(VK_PAD_LTHUMB_DOWNLEFT);
            CASEIDTOTEXT(VK_PAD_RTHUMB_UP);
            CASEIDTOTEXT(VK_PAD_RTHUMB_DOWN);
            CASEIDTOTEXT(VK_PAD_RTHUMB_RIGHT);
            CASEIDTOTEXT(VK_PAD_RTHUMB_LEFT);
            CASEIDTOTEXT(VK_PAD_RTHUMB_UPLEFT);
            CASEIDTOTEXT(VK_PAD_RTHUMB_UPRIGHT);
            CASEIDTOTEXT(VK_PAD_RTHUMB_DOWNRIGHT);
            CASEIDTOTEXT(VK_PAD_RTHUMB_DOWNLEFT);
            CASEIDTOTEXT(VK_PAD_GUIDE);
        default: return "?";
    }
}

int main(void) {
    XINPUT_STATE_EX state;
    XINPUT_KEYSTROKE keystroke;
    DWORD serial[XUSER_MAX_COUNT] = {0, 0, 0, 0};
    int mode = 0;

    DllMain(0, DLL_PROCESS_ATTACH, 0);

    while (mode != 1) {
        for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
            if (XInputGetStateEx(i, &state) == ERROR_SUCCESS) {
                if (state.dwPacketNumber != serial[i]) {
                    TRACE("%i | %9i | %04hx,%04hx %04hx,%04hx %02hhx %02hhx %04hx\n",
                            i,
                            state.dwPacketNumber,
                            state.Gamepad.sThumbLX,
                            state.Gamepad.sThumbLY,
                            state.Gamepad.sThumbRX,
                            state.Gamepad.sThumbRY,
                            state.Gamepad.bLeftTrigger,
                            state.Gamepad.bRightTrigger,
                            state.Gamepad.wButtons
                            );
                    serial[i] = state.dwPacketNumber;

                    if ((state.Gamepad.wButtons & 0x400) != 0) {
                        ++mode;
                    }
                }
            }
        }
        usleep(1000);
    }

    while (mode != 3) {
        for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
            if (XInputGetKeystroke(i, 0, &keystroke) == ERROR_SUCCESS) {
                TRACE("%i | %4x | %32s | %4x | %4x | %i | %i\n",
                        i,
                        keystroke.VirtualKey,
                        get_vk_text(keystroke.VirtualKey),
                        keystroke.Unicode,
                        keystroke.Flags,
                        keystroke.UserIndex,
                        keystroke.HidCode);

                if ((keystroke.VirtualKey == VK_PAD_GUIDE) && (keystroke.Flags == XINPUT_KEYSTROKE_KEYDOWN)) {
                    ++mode;
                }
            }
        }
        usleep(1000);
    }

    DllMain(0, DLL_PROCESS_DETACH, 0);
    return 0;
}

#endif
