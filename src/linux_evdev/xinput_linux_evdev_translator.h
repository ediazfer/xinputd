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

#ifndef XINPUT_LINUX_EVDEV_TRANSLATOR_H
#define XINPUT_LINUX_EVDEV_TRANSLATOR_H

#include <stdint.h>
#include <linux/input.h>

#include "xinput.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xinput_linux_evdev_translator_abs_translator_item;

typedef void (*xinput_linux_evdev_translator_abs_translator_callback)(const struct xinput_linux_evdev_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX*, int16_t value);

struct xinput_linux_evdev_translator_abs_translator_item
{
    xinput_linux_evdev_translator_abs_translator_callback translate;
    ssize_t to;
    int16_t positive;
    int16_t negative;
};

struct xinput_linux_evdev_translator_abs_translator
{
    struct xinput_linux_evdev_translator_abs_translator_item _item[ABS_CNT];
};

void xinput_linux_evdev_translator_abs_translate_nothing(const struct xinput_linux_evdev_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX*, int16_t value);
void xinput_linux_evdev_translator_abs_translate_to_axis(const struct xinput_linux_evdev_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX*, int16_t value);
void xinput_linux_evdev_translator_abs_translate_to_axis_reverse(const struct xinput_linux_evdev_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX*, int16_t value);
void xinput_linux_evdev_translator_abs_translate_to_buttons(const struct xinput_linux_evdev_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX*, int16_t value);

void xinput_gamepad_abs_set_axis(struct xinput_linux_evdev_translator_abs_translator *abs, int bit, ssize_t offs);
void xinput_gamepad_abs_set_sixa(struct xinput_linux_evdev_translator_abs_translator *abs, int bit, ssize_t offs);
void xinput_gamepad_abs_set_bttn(struct xinput_linux_evdev_translator_abs_translator *abs, int bit, int16_t pos, int16_t neg);

#define XINPUT_GAMEPAD_ABS_SET_AXIS(_abs, _bit, _field) xinput_gamepad_abs_set_axis((_abs),(_bit), offsetof(XINPUT_GAMEPAD_EX, _field))
#define XINPUT_GAMEPAD_ABS_SET_SIXA(_abs, _bit, _field) xinput_gamepad_abs_set_sixa((_abs),(_bit), offsetof(XINPUT_GAMEPAD_EX, _field))
#define XINPUT_GAMEPAD_ABS_SET_BTTN(_abs, _bit, _pos, _neg) xinput_gamepad_abs_set_bttn((_abs),(_bit), (_pos), (_neg))

/*
 * Starts the table
 */

#define XINPUT_GAMEPAD_ABS_BEGIN(__mytable) struct xinput_linux_evdev_translator_abs_translator __mytable = { ._item = {

/*
 * Maps an absolute axis to a gamepad axis
 */

#define XINPUT_GAMEPAD_ABS_AXIS(_debugname,_field) {&xinput_linux_evdev_translator_abs_translate_to_axis,offsetof(XINPUT_GAMEPAD_EX, _field),0,0},

/*
 * Maps an absolute axis to a gamepad axis, in the opposite direction [-1; 1] => [1; -1]
 */

#define XINPUT_GAMEPAD_ABS_SIXA(_debugname,_field) {&xinput_linux_evdev_translator_abs_translate_to_axis_reverse,offsetof(XINPUT_GAMEPAD_EX, _field),0,0},

/*
 * Maps an absolute axis to a gamepad button
 */

#define XINPUT_GAMEPAD_ABS_BTTN(_debugname,_positive,_negative) {&xinput_linux_evdev_translator_abs_translate_to_buttons, 0,(_positive),(_negative)},

/*
 * Ignores an absolute axis
 */

#define XINPUT_GAMEPAD_ABS_NOPE(_debugname) {&xinput_linux_evdev_translator_abs_translate_nothing,0,(_debugname),0},

/*
 * Ends the table
 */

#define XINPUT_GAMEPAD_ABS_END(__mytable) } }; /* __mytable */

typedef void (*xinput_input_event_to_gamepad)(struct input_event*, XINPUT_GAMEPAD_EX*);

struct xinput_linux_evdev_translator_key_translator
{
    int _first;
    int _last;
    const SHORT* _buttons;
};

/*
 * Maps a range of keys(buttons) to buttons of the gamepad
 *
 * Give the first and last keys(buttons) of the range, then each mapped
 * gamepad button, or 0 for unmapped keys(buttons).
 */

#define XINPUT_GAMEPAD_KEY_TRANSLATOR(__mytable,_first,_last, args...) \
    static SHORT __mytable##_buttons[(_last) - (_first) + 1] = { args }; \
    static struct xinput_linux_evdev_translator_key_translator __mytable = { (_first), (_last), __mytable##_buttons };

/*
 * Using the given translator, updates the XINPUT_GAMEPAD_EX with to the input_event
 */

void xinput_linux_evdev_translator_abs_input_event_to_gamepad(const struct xinput_linux_evdev_translator_abs_translator* translator, const struct input_event* ie, XINPUT_GAMEPAD_EX* gamepad);

/*
 * Using the given translator, updates the XINPUT_GAMEPAD_EX with to the input_event
 */

void xinput_linux_evdev_translator_key_input_event_to_gamepad(const struct xinput_linux_evdev_translator_key_translator* translator, const struct input_event* ie, XINPUT_GAMEPAD_EX* gamepad);

#ifdef __cplusplus
}
#endif

#endif /* XINPUT_LINUX_EVDEV_TRANSLATOR_H */

