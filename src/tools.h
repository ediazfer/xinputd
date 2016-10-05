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

#ifndef XINPUT_TOOLS_H
#define XINPUT_TOOLS_H

#include <stddef.h>
#include <stdint.h>
#include <xinput_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Reads a file descriptor until the amount of bytes has been read.
 * Retries on EINTR
 * Stops trying on errors
 *
 * @param fd
 * @param buffer_
 * @param len
 * @return
 */

int read_fully(int fd, void* buffer_, size_t len);

/**
 * Writes a file descriptor until the amount of bytes has been written.
 * Retries on EINTR
 * Stops trying on errors
 *
 * @param fd
 * @param buffer_
 * @param len
 * @return
 */

int write_fully(int fd, const void* buffer_, size_t len);

/**
 * Closes a file descriptor.
 * Retries on EINTR
 * Stops trying on errors
 *
 * @param fd
 */

void close_ex(int fd);

/**
 * Returns the current epoch with a microseconds precision
 *
 * @return
 */

int64_t timeus(void);

/**
 * Returns the nth bit of a byte array.
 * Bits are given from lsb to msb
 *
 * @param array
 * @param bit
 * @return
 */

static inline BOOL bit_get(const uint8_t* array, int bit)
{
    return (array[bit >> 3] & (1 << (bit & 7))) != 0;
}

#ifdef __cplusplus
}
#endif

#endif /* XINPUT_TOOLS_H */
