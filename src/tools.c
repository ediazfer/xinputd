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

#include "config.h"
#include "xinput_settings.h"

#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

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

int read_fully(int fd, void* buffer_, size_t len)
{
    uint8_t* buffer = (uint8_t*)buffer_;
    while(len > 0)
    {
        ssize_t n = read(fd, buffer, len);
        if(n < 0)
        {
            int err = errno;
            if(err == EINTR)
            {
                continue;
            }
            return err;
        }
        buffer += n;
        len -= n;
    }
    return 0;
}

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

int write_fully(int fd, const void* buffer_, size_t len)
{
    const uint8_t* buffer = (const uint8_t*)buffer_;
    while(len > 0)
    {
        ssize_t n = write(fd, buffer, len);
        if(n < 0)
        {
            int err = errno;
            if(err == EINTR)
            {
                continue;
            }
            return err;
        }
        buffer += n;
        len -= n;
    }
    return 0;
}

/**
 * Closes a file descriptor.
 * Retries on EINTR
 * Stops trying on errors
 *
 * @param fd
 */

void close_ex(int fd)
{
    for(;;)
    {
        int ret = close(fd);
        if((ret >= 0) || (errno != EINTR))
        {
            break;
        }
    }
}

/**
 * Returns epoch with a microsecond accuracy.
 *
 * @return
 */

int64_t timeus(void)
{
    struct timeval tp;
    int64_t now;
    gettimeofday(&tp, NULL);
    now = tp.tv_sec;
    now *= 1000000LL;
    now += tp.tv_usec;
    return now;
}
