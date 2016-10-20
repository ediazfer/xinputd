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

#ifndef __XINPUT_H__
#define __XINPUT_H__

#include <xinput_types.h>

#define XINPUT_1SHIFT(v__) (1<<(v__))

#define XUSER_MAX_COUNT 4
#define XUSER_INDEX_ANY 255

#define BATTERY_TYPE_DISCONNECTED 0
#define BATTERY_TYPE_WIRED 1
#define BATTERY_TYPE_ALKALNE 2
#define BATTERY_TYPE_NIMH 3
#define BATTERY_TYPE_UNKNOWN 255

#define BATTERY_LEVEL_EMPTY 0
#define BATTERY_LEVEL_LOW 1
#define BATTERY_LEVEL_MEDIUM 2
#define BATTERY_LEVEL_FULL 3

#define XINPUT_DEVTYPE_GAMEPAD 0
#define XINPUT_DEVTYPE_HEADSET 1
#define XINPUT_DEVSUBTYPE_GAMEPAD 2

#define XINPUT_CAPS_VOICE_SUPPORTED XINPUT_1SHIFT(2)
#define XINPUT_CAPS_FFB_SUPPORTED XINPUT_1SHIFT(3)
#define XINPUT_CAPS_WIRELESS XINPUT_1SHIFT(4)
#define XINPUT_CAPS_PMD_SUPPORTED XINPUT_1SHIFT(5)
#define XINPUT_CAPS_NO_NAVIGATION XINPUT_1SHIFT(6)

#define XINPUT_GAMEPAD_DPAD_UP XINPUT_1SHIFT(0)
#define XINPUT_GAMEPAD_DPAD_DOWN XINPUT_1SHIFT(1)
#define XINPUT_GAMEPAD_DPAD_LEFT XINPUT_1SHIFT(2)
#define XINPUT_GAMEPAD_DPAD_RIGHT XINPUT_1SHIFT(3)
#define XINPUT_GAMEPAD_START XINPUT_1SHIFT(4)
#define XINPUT_GAMEPAD_BACK XINPUT_1SHIFT(5)
#define XINPUT_GAMEPAD_LEFT_THUMB XINPUT_1SHIFT(6)
#define XINPUT_GAMEPAD_RIGHT_THUMB XINPUT_1SHIFT(7)
#define XINPUT_GAMEPAD_LEFT_SHOULDER XINPUT_1SHIFT(8)
#define XINPUT_GAMEPAD_RIGHT_SHOULDER XINPUT_1SHIFT(9)
#define XINPUT_GAMEPAD_GUIDE XINPUT_1SHIFT(10)
#define XINPUT_GAMEPAD_RESERVED0 XINPUT_1SHIFT(11)
#define XINPUT_GAMEPAD_A XINPUT_1SHIFT(12)
#define XINPUT_GAMEPAD_B XINPUT_1SHIFT(13)
#define XINPUT_GAMEPAD_X XINPUT_1SHIFT(14)
#define XINPUT_GAMEPAD_Y XINPUT_1SHIFT(15)

#define XINPUT_KEYSTROKE_KEYDOWN 1
#define XINPUT_KEYSTROKE_KEYUP 2
#define XINPUT_KEYSTROKE_REPEAT 4

#define VK_PAD_ITEM(__id) (0x5800 + (__id))

#define VK_PAD_A VK_PAD_ITEM(0)
#define VK_PAD_B VK_PAD_ITEM(1)
#define VK_PAD_X VK_PAD_ITEM(2)
#define VK_PAD_Y VK_PAD_ITEM(3)
#define VK_PAD_RSHOULDER VK_PAD_ITEM(4)
#define VK_PAD_LSHOULDER VK_PAD_ITEM(5)
#define VK_PAD_LTRIGGER VK_PAD_ITEM(6)
#define VK_PAD_RTRIGGER VK_PAD_ITEM(7)
#define VK_PAD_GUIDE VK_PAD_ITEM(8)
#define VK_PAD_DPAD_UP VK_PAD_ITEM(16)
#define VK_PAD_DPAD_DOWN VK_PAD_ITEM(17)
#define VK_PAD_DPAD_LEFT VK_PAD_ITEM(18)
#define VK_PAD_DPAD_RIGHT VK_PAD_ITEM(19)
#define VK_PAD_START VK_PAD_ITEM(20)
#define VK_PAD_BACK VK_PAD_ITEM(21)
#define VK_PAD_LTHUMB_PRESS VK_PAD_ITEM(22)
#define VK_PAD_RTHUMB_PRESS VK_PAD_ITEM(23)
#define VK_PAD_LTHUMB_UP VK_PAD_ITEM(32)
#define VK_PAD_LTHUMB_DOWN VK_PAD_ITEM(33)
#define VK_PAD_LTHUMB_RIGHT VK_PAD_ITEM(34)
#define VK_PAD_LTHUMB_LEFT VK_PAD_ITEM(35)
#define VK_PAD_LTHUMB_UPLEFT VK_PAD_ITEM(36)
#define VK_PAD_LTHUMB_UPRIGHT VK_PAD_ITEM(37)
#define VK_PAD_LTHUMB_DOWNRIGHT VK_PAD_ITEM(38)
#define VK_PAD_LTHUMB_DOWNLEFT VK_PAD_ITEM(39)
#define VK_PAD_RTHUMB_UP VK_PAD_ITEM(48)
#define VK_PAD_RTHUMB_DOWN VK_PAD_ITEM(49)
#define VK_PAD_RTHUMB_RIGHT VK_PAD_ITEM(50)
#define VK_PAD_RTHUMB_LEFT VK_PAD_ITEM(51)
#define VK_PAD_RTHUMB_UPLEFT VK_PAD_ITEM(52)
#define VK_PAD_RTHUMB_UPRIGHT VK_PAD_ITEM(53)
#define VK_PAD_RTHUMB_DOWNRIGHT VK_PAD_ITEM(54)
#define VK_PAD_RTHUMB_DOWNLEFT VK_PAD_ITEM(55)

#define ERROR_SUCCESS 0
#define ERROR_NOT_SUPPORTED -1
#define ERROR_BAD_ARGUMENTS -1
#define ERROR_DEVICE_NOT_CONNECTED -1
#define ERROR_EMPTY -1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GUID {
  DWORD Data1;
  WORD  Data2;
  WORD  Data3;
  BYTE  Data4[8];
} GUID;

typedef struct _XINPUT_BATTERY_INFORMATION {
  BYTE BatteryType;
  BYTE BatteryLevel;
} XINPUT_BATTERY_INFORMATION, *PXINPUT_BATTERY_INFORMATION;

typedef struct _XINPUT_GAMEPAD {
  WORD  wButtons;
  BYTE  bLeftTrigger;
  BYTE  bRightTrigger;
  SHORT sThumbLX;
  SHORT sThumbLY;
  SHORT sThumbRX;
  SHORT sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

typedef struct _XINPUT_GAMEPAD_EX {
  WORD  wButtons;
  BYTE  bLeftTrigger;
  BYTE  bRightTrigger;
  SHORT sThumbLX;
  SHORT sThumbLY;
  SHORT sThumbRX;
  SHORT sThumbRY;
  DWORD reserved;
} XINPUT_GAMEPAD_EX, *PXINPUT_GAMEPAD_EX;

typedef struct _XINPUT_VIBRATION {
  WORD wLeftMotorSpeed;
  WORD wRightMotorSpeed;
} XINPUT_VIBRATION, *PXINPUT_VIBRATION;

typedef struct _XINPUT_CAPABILITIES {
  BYTE             Type;
  BYTE             SubType;
  WORD             Flags;
  XINPUT_GAMEPAD   Gamepad;
  XINPUT_VIBRATION Vibration;
} XINPUT_CAPABILITIES, *PXINPUT_CAPABILITIES;

typedef struct _XINPUT_KEYSTROKE {
  WORD  VirtualKey;
  WCHAR Unicode;
  WORD  Flags;
  BYTE  UserIndex;
  BYTE  HidCode;
} XINPUT_KEYSTROKE, *PXINPUT_KEYSTROKE;

typedef struct _XINPUT_STATE {
  DWORD          dwPacketNumber;
  XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE, *PXINPUT_STATE;

typedef struct _XINPUT_STATE_EX {
  DWORD          dwPacketNumber;
  XINPUT_GAMEPAD_EX Gamepad;
} XINPUT_STATE_EX, *PXINPUT_STATE_EX;

void WINAPI XInputEnable(BOOL enable);
DWORD WINAPI XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId, UINT* pRenderCount, LPWSTR pCaptureDeviceId, UINT* pCaptureCount);
DWORD WINAPI XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBatteryInformation);
DWORD WINAPI XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);
DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD dwUserIndex,GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid);
DWORD WINAPI XInputGetKeystroke(DWORD dwUserIndex,DWORD dwReserved,PXINPUT_KEYSTROKE pKeystroke);
DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
DWORD WINAPI XInputGetStateEx(DWORD dwUserIndex, XINPUT_STATE_EX* pState);
DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

#ifdef __cplusplus
}
#endif


#endif // __XINPUT_H__
