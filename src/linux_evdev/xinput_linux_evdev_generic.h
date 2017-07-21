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

#ifndef XINPUT_LINUX_EVDEV_GENERIC_H
#define XINPUT_LINUX_EVDEV_GENERIC_H

#include "xinput_gamepad.h"
#include "xinput_linux_evdev.h"
#include <linux/input.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Tells if this driver can handle that input_id.
 *
 * @param id
 * @return
 */

BOOL xinput_linux_evdev_generic_can_translate(const struct xinput_linux_evdev_probe_s* probed);

/**
 * Initialises an instance of driver
 *
 * @param id
 * @param fd
 * @param instance
 * @return
 */

BOOL xinput_linux_evdev_generic_new_instance(const struct xinput_linux_evdev_probe_s* probed, int fd, xinput_gamepad_device* instance);

#ifdef __cplusplus
}
#endif

#endif /* XINPUT_LINUX_EVDEV_XBOXPAD_H */

