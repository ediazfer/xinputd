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

#define XINPUT_DEBUG_C_ 1
#include "config.h"
#include "xinput_settings.h"

#if HAVE_WINE
#include "wine/debug.h"
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

#if !HAVE_WINE

typedef void (*printf_callback)(const char*,...);

static void xinput_printf(const char *text, ...)
{
    va_list args;
    va_start(args, text);
    vprintf(text, args);
    va_end(args);
}

printf_callback trace_printf = xinput_printf;
printf_callback fixme_printf = xinput_printf;

#endif

WINE_DEFAULT_DEBUG_CHANNEL(xinput);

void hexdump(const void* buffer, size_t len)
{
    const uint8_t* p = (const uint8_t*)buffer;
    const uint8_t* l = &p[len];
    while(p < l)
    {
        TRACE("%02x ", *p);
        ++p;
    }
}

static const uint8_t* memdump_line(const uint8_t* buffer, size_t len, size_t width)
{
    const uint8_t* p = buffer;
    const uint8_t* l = &p[len];
    const uint8_t* w = &p[width];
    TRACE("%p | ", buffer);
    while(p < l)
    {
        TRACE("%02x ", *p);
        ++p;
    }
    while(p < w)
    {
        TRACE("-- ");
        ++p;
    }
    TRACE("| ");
    p = buffer;
    while(p < l)
    {
        char c = *p;
        if((c < ' ') || (c >= 127))
        {
            c = '.';
        }
        TRACE("%c", c);
        ++p;
    }
    TRACE("\n");

    return p;
}

void memdump(const void* buffer, size_t len)
{
    const uint8_t* p = (const uint8_t*)buffer;
    const size_t w = XINPUT_MEMDUMP_COLUMNS;

    while(len > w)
    {
        p = memdump_line(p, w, w);
        len -= w;
    }
    if(len > 0)
    {
        memdump_line(p, len, w);
    }
}
