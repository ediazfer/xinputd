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

#ifndef XINPUT_DEBUG_H
#define XINPUT_DEBUG_H

#include <stddef.h>
#include <string.h>

#if HAVE_WINE
WINE_DEFAULT_DEBUG_CHANNEL(xinput);
#else
#define WINAPI
#define DECLSPEC_HOTPATCH
#define TRACE trace_printf
#define FIXME fixme_printf
#define DisableThreadLibraryCalls(...)
#undef WINE_DEFAULT_DEBUG_CHANNEL
#define WINE_DEFAULT_DEBUG_CHANNEL(...)
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*printf_callback)(const char*,...);

#if !XINPUT_DEBUG_C_
extern printf_callback trace_printf;
extern printf_callback fixme_printf;
#endif

void hexdump(const void* buffer, size_t len);
void memdump(const void* buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* XINPUT_DEBUG_H */

