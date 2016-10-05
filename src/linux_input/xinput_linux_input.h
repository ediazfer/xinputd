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

#ifndef XINPUT_LINUX_INPUT_H
#define XINPUT_LINUX_INPUT_H

#include <stdint.h>
#include "xinput_gamepad.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialises the linux input
 *
 */

void xinput_linux_input_initialize(void);

/**
 * Probes for new devices.
 *
 * @return a bitmask of the newly found devices
 */

uint32_t xinput_linux_input_probe(void);

/**
 * Returns the device in the specified slot
 *
 * @param slot
 * @return
 */

xinput_gamepad_device* xinput_linux_input_get_device(int slot);

/**
 * Closes the device in the specified slot
 *
 * @param slot
 */

void xinput_linux_input_device_close(int slot);

/**
 * Cleans-up the linux input
 *
 */

void xinput_linux_input_finalize(void);

/**
 * If multiple driver support is implemented, this will be replaced
 * by a function setting a virtual table.
 * Until then, this basic mapping will do.
 */

#define xinput_driver_initialize xinput_linux_input_initialize
#define xinput_driver_probe xinput_linux_input_probe
#define xinput_driver_get_device xinput_linux_input_get_device
#define xinput_driver_device_close xinput_linux_input_device_close
#define xinput_driver_finalize xinput_linux_input_finalize

struct input_event;
int xinput_linux_input_read_next(int fd, struct input_event* ie);

/**
 * The left motor is supposed to be low frequency, high magnitude
 * The right motor is supposed to be high frequency, weak magnitude
 *
 * @param fd
 * @param id the current effect id, or -1
 * @param low
 * @param high
 *
 * @return the id of the effect or -1 if it failed to register the effect
 */

int xinput_linux_input_rumble(int fd, int id, SHORT low_left, SHORT high_right);

void xinput_linux_input_feedback_clear(int fd, int id);

#ifdef __cplusplus
}
#endif

#endif /* XINPUT_LINUX_INPUT_H */

