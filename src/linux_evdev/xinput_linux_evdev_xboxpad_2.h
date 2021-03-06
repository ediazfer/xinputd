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

#ifndef XINPUT_LINUX_EVDEV_XBOXPAD_2_H
#define XINPUT_LINUX_EVDEV_XBOXPAD_2_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * As a driver, this is redundant with xinput_linux_evdev_xboxpad
 * But this is an example of easy implementation that requires only
 * editing tables to implement the features of the joystick.
 * (Mind I only have 360/One pads for testing)
 *
 * The next driver I'll write is one that generates the same tables
 * to make do with whatever buttons/axis are found on an unknown joystick.
 *
 */

/**
 * Tells if this driver can handle that input_id.
 *
 * @param id
 * @return
 */

BOOL xinput_linux_evdev_xboxpad2_can_translate(const struct input_id* id);

/**
 * Initialises an instance of driver
 *
 * @param id
 * @param fd
 * @param instance
 * @return
 */

BOOL xinput_linux_evdev_xboxpad2_new_instance(const struct input_id* id, int fd, xinput_gamepad_device* instance);

#ifdef __cplusplus
}
#endif

#endif /* XINPUT_LINUX_EVDEV_XBOXPAD_2_H */

