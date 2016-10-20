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

#include "config.h"

#if HAVE_LINUX_INPUT_H

#include "xinput_settings.h"

#if HAVE_WINE
#include "wine/debug.h"
#endif

#include "xinput.h"
#include "tools.h"
#include "debug.h"

#include "xinput_linux_input_translator.h"

WINE_DEFAULT_DEBUG_CHANNEL(xinput);

void xinput_linux_input_translator_abs_translate_nothing(const struct xinput_linux_input_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX* gamepad, int16_t value)
{
    (void)item;
    (void)gamepad;
    (void)value;
    FIXME("ABS input not supported\n");
}

void xinput_linux_input_translator_abs_translate_to_axis(const struct xinput_linux_input_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX* gamepad, int16_t value)
{
    SHORT* p;
    char* base = (char*)gamepad;
    base += item->to;
    p = (SHORT*)base;
    *p = value;
}

void xinput_linux_input_translator_abs_translate_to_axis_reverse(const struct xinput_linux_input_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX* gamepad, int16_t value)
{
    SHORT* p;
    char* base = (char*)gamepad;
    base += item->to;
    p = (SHORT*)base;
    *p = ~value;
}

void xinput_linux_input_translator_abs_translate_to_buttons(const struct xinput_linux_input_translator_abs_translator_item* item, XINPUT_GAMEPAD_EX* gamepad, int16_t value)
{
    if(value > 0)
    {
        gamepad->wButtons = (gamepad->wButtons | item->positive) & ~item->negative;
    }
    else if(value < 0)
    {
        gamepad->wButtons = (gamepad->wButtons | item->negative) & ~item->positive;
    }
    else
    {
        gamepad->wButtons &= ~(item->positive | item->negative);
    }
}

void
xinput_linux_input_translator_abs_input_event_to_gamepad(const struct xinput_linux_input_translator_abs_translator* translator, const struct input_event* ie, XINPUT_GAMEPAD_EX* gamepad)
{
    const struct xinput_linux_input_translator_abs_translator_item* line = &translator->_item[ie->code];
    line->translate(line, gamepad, ie->value);
}

void
xinput_linux_input_translator_key_input_event_to_gamepad(const struct xinput_linux_input_translator_key_translator* translator, const struct input_event* ie, XINPUT_GAMEPAD_EX* gamepad)
{
    if(ie->code >= translator->_first && ie->code <= translator->_last)
    {
        SHORT bit = translator->_buttons[ie->code - translator->_first];
        if(ie->value != 0)
        {
            gamepad->wButtons |= bit;
        }
        else
        {
            gamepad->wButtons &= ~bit;
        }
    }
}

void xinput_gamepad_abs_set_axis(struct xinput_linux_input_translator_abs_translator *abs, int bit, ssize_t offs)
{
    abs->_item[bit].translate = &xinput_linux_input_translator_abs_translate_to_axis;
    abs->_item[bit].to = offs;
    abs->_item[bit].positive = 0;
    abs->_item[bit].negative = 0;
}

void xinput_gamepad_abs_set_sixa(struct xinput_linux_input_translator_abs_translator *abs, int bit, ssize_t offs)
{
    abs->_item[bit].translate = &xinput_linux_input_translator_abs_translate_to_axis_reverse;
    abs->_item[bit].to = offs;
    abs->_item[bit].positive = 0;
    abs->_item[bit].negative = 0;
}

void xinput_gamepad_abs_set_bttn(struct xinput_linux_input_translator_abs_translator *abs, int bit, int16_t pos, int16_t neg)
{
    abs->_item[bit].translate = &xinput_linux_input_translator_abs_translate_to_buttons;
    abs->_item[bit].to = 0;
    abs->_item[bit].positive = pos;
    abs->_item[bit].negative = neg;
}

#endif /* HAVE_LINUX_INPUT_H */
