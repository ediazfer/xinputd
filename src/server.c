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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "tools.h"

#include "xinput_service.h"

static const char redirect[] = "/dev/null";

static int daemon_fork_stage(void)
{
    int pid = fork();
    
    if(pid == 0)
    {
        return 0;
    }
    else if(pid < 0)
    {
        perror("could not fork");
        return -1;
    }
    else
    {
        exit(0);
    }
}

static int daemonize(void)
{
    if(daemon_fork_stage() == 0)
    {
        setsid();
                
        if(daemon_fork_stage() == 0)
        {
            chdir("/");
            umask(0);
            close_ex(0);
            
            if(open(redirect, O_RDONLY) < 0)
            {
                fprintf(stderr, "could not read-open %s: %s", redirect, strerror(errno));
            }
            close_ex(1);
            if(open(redirect, O_WRONLY) < 0)
            {
                fprintf(stderr, "could not write-open %s: %s", redirect, strerror(errno));
            }
            close_ex(2);
            if(open(redirect, O_WRONLY) < 0)
            {
                // there is no point, is it ?
            }
            
            return 0;
        }
    }
    
    return -1;
}

int server(int daemon)
{
    if(daemon)
    {
        if(daemonize() < 0)
        {
            return -1;
        }
    }
    
    xinput_service_set_autoshutdown(0);
    xinput_service_server();
    
    return 0;
}
