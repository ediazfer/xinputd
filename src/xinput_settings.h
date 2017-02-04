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

#ifndef XINPUT_SETTINGS_H
#define XINPUT_SETTINGS_H

#ifdef __WINESRC__
#ifdef HAVE_WINE
#undef HAVE_WINE
#endif
#define HAVE_WINE 1
#endif

/**
 * TRACE the device input state from the reading threads.
 */

#define XINPUT_TRACE_DEVICE_READER_THREAD 1

/**
 * TRACE gamepad probe detection superficial information.
 */

#define XINPUT_TRACE_DEVICE_DETECTION 1

/**
 * The approximal time in seconds between two probes.
 */

#define XINPUT_DEVICE_PROBE_PERIOD_S 5

#define XINPUT_OWNER_PROBE_PERIOD_US 200000LL

#define XINPUT_OWNER_REPROBE_PERIOD_US 1000000LL

/**
 * For the debug functions
 */

#define XINPUT_MEMDUMP_COLUMNS  16

/**
 * Trace calls to the xinput1_3 DLL API.
 */

#define XINPUT_TRACE_INTERFACE_USE 0

/**
 * Do not enable this on 64/32 bits systems as it is not supported by POSIX.
 * Cannot properly be shared in a 32/64 environment)
 *
 * Given the nature of the shared data, not using a lock should be
 * of little consequence.
 */

#define XINPUT_USES_SEMAPHORE_MUTEX 0 /* KEEP TO 0 */

/**
 * Rumble queries are sent this way.
 * This feature can be disabled but then of course no rumble will be available.
 *
 * Looks like OSX is not fully POSIX so it will need a slightly different way.
 */

#define XINPUT_USES_MQUEUE 1

/**
 * Set to 0, the first instance of the DLL will double as a server
 * If the program containing the server stops, another instance will take
 * the job.
 *
 * Set to 1, the first instance of the DLL will use Rundll32 to run a server
 * that will live until wine stops (or it is killed)
 *
 */

#define XINPUT_RUNDLL 1

/**
 * The service can be shared among any process but should only run once.
 * This define sets the global name of the lock.
 */

#define XINPUT_SYSTEM_WIDE_LOCK_FILE "/tmp/wine.xinput.lock"

/**
 * The server will look since when any client wrote a timestamp in the shared
 * memory.  It does it at the same time it probes for gamepads. (ie: 5 seconds)
 *
 * After seeing no changes this many times, the server shuts-down.
 */

#define XINPUT_IDLE_CLIENT_STRIKES  3

#if !HAVE_WINE
#undef XINPUT_RUNDLL
#define XINPUT_RUNDLL 0
#endif

#endif /* XINPUT_SETTINGS_H */
