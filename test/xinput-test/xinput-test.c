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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <xinput.h>

static const char* get_vk_text(int vk)
{
#define CASEIDTOTEXT(code) case code: return #code
    switch(vk)
    {
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

int main(void)
{
    XINPUT_STATE_EX state;
    XINPUT_KEYSTROKE keystroke;
    DWORD serial[XUSER_MAX_COUNT] = {0,0,0,0};
    int mode = 0;

    while(mode != 1)
    {
        for(int i = 0; i < XUSER_MAX_COUNT; ++i)
        {
            if(XInputGetStateEx(i, &state) == ERROR_SUCCESS)
            {
                if(state.dwPacketNumber != serial[i])
                {
                    printf("%i | %9i | %04hx,%04hx %04hx,%04hx %02hhx %02hhx %04hx\n",
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
                    
                    if((state.Gamepad.wButtons & 0x400) != 0)
                    {
                        ++mode;
                    }
                }
            }
        }
        usleep(1000);
    }
    
    while(mode != 3)
    {
        for(int i = 0; i < XUSER_MAX_COUNT; ++i)
        {
            if(XInputGetKeystroke(i, 0, &keystroke) == ERROR_SUCCESS)
            {
                printf("%i | %4x | %32s | %4x | %4x | %i | %i\n",
                    i,
                    keystroke.VirtualKey,
                    get_vk_text(keystroke.VirtualKey),
                    keystroke.Unicode,
                    keystroke.Flags,
                    keystroke.UserIndex,
                    keystroke.HidCode);
                
                if((keystroke.VirtualKey == VK_PAD_GUIDE) && (keystroke.Flags == XINPUT_KEYSTROKE_KEYDOWN))
                {
                    ++mode;
                }
            }
        }
        usleep(1000);
    }

    return 0;
}