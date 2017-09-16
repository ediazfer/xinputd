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

#error "do not build"

#define xinput_driver_initialize xinput_linux_joystick_initialize
#define xinput_driver_probe xinput_linux_joystick_probe
#define xinput_driver_get_device xinput_linux_joystick_get_device
#define xinput_driver_device_close xinput_linux_joystick_device_close
#define xinput_driver_finalize xinput_linux_joystick_finalize

void xinput_linux_joystick_initialize(void)
{
}

void xinput_linux_joystick_finalize(void)
{
}

uint32_t xinput_linux_joystick_probe(void)
{
    uint64_t now = timeus();
    int n;
    int fd;
        
    int one = 1;
    uint32_t mask = 0;
    
    struct xinput_linux_joystick_probe_s probed;
    char filename[128];

    if(now - xinput_linux_joystick_probe_last_epoch < 5000000)
    {
        return 0;
    }
    
    /* to silence valgrind */

    xinput_linux_joystick_probe_last_epoch = now;

#if XINPUT_TRACE_DEVICE_DETECTION
    TRACE("probing devices\n");
#endif

    for(int input_index = 0; input_index < EVENT_MAX; ++input_index)
    {
        int slot = xinput_linux_joystick_next_free_slot();

        if(slot < 0)
        {
            TRACE("no more slot available (whoopsie)\n");
            break;
        }

        /* already in use */

        if(xinput_linux_joystick_device_in_use(input_index))
        {
            continue;
        }

        snprintf(filename, sizeof(filename), "/dev/input/event%i", input_index);
        
        struct stat st;
        
        if(lstat(filename, &st) < 0)
        {
            continue;
        }
        
        // do not process a device that remains the same since the last probe
        
        uint64_t ct = st.st_ctim.tv_sec;
        ct *= 1000ULL;
        ct += st.st_ctim.tv_nsec / 1000000ULL;
        
        if(last_event_state_change[input_index] >= ct)
        {
            // no change
            continue;
        }
        
        last_event_state_change[input_index] = ct;
        
        fd = open(filename, O_RDONLY);

        if(fd < 0)
        {
            TRACE("cannot open joystick at %s: %s\n", filename, strerror(errno));
            continue;
        }
        
        memset(&probed, 0, sizeof(struct xinput_linux_joystick_probe_s));

#if XINPUT_TRACE_DEVICE_DETECTION
        TRACE("opened joystick at %s\n", filename);
#endif

        if(ioctl(fd, EVIOCGVERSION, &probed.version) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("version: %x\n", probed.version);
#endif
        }

        if(ioctl(fd, EVIOCGID, &probed.id) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("id: %x %x %x %x\n", probed.id.bustype, probed.id.product, probed.id.vendor, probed.id.version);
#endif
        }

        if(ioctl(fd, EVIOCGNAME(sizeof(probed.device_name)), probed.device_name) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("name: '%s'\n", probed.device_name);
#endif
        }

        if(ioctl(fd, EVIOCGPHYS(sizeof(probed.location)), probed.location) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("loc: '%s'\n", probed.location);
#endif
        }

        if((n = ioctl(fd, EVIOCGPROP(sizeof(probed.prop)), probed.prop)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("prop: %i\n", n);
            hexdump(probed.prop, n);
            TRACE("\n");
#endif
        }

        if((n = ioctl(fd, EVIOCGBIT(0, EV_CNT), probed.ev_all)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            memdump(probed.ev_all, sizeof(probed.ev_all));

            for(int j = 0; j < EV_CNT; ++j)
            {
                BOOL on = bit_get(probed.ev_all, j);
                if(on)
                {
                    TRACE("%s ", xinput_linux_joystick_event_type_get_name(j)); /* no LF */
                }
            }
            
            TRACE("\n");
#endif
        }
        else
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("ev all: %s\n", strerror(errno));
#endif
        }
        
        // uint8_t wanted_mask = (1<<EV_SYN)|(1<<EV_KEY)|(1<<EV_ABS)|(1<<EV_MSC);
        // uint8_t rejected_mask = (1<<EV_REL)|(1<<EV_PWR);

        if(!(bit_get(probed.ev_all, EV_KEY) && bit_get(probed.ev_all, EV_ABS)) || bit_get(probed.ev_all, EV_REL) || bit_get(probed.ev_all, EV_PWR))
        {
            TRACE("%s @%s: not a candidate\n",
                    probed.device_name,
                    filename);
            close_ex(fd);
            continue;
        }
        else
        {
            TRACE("%s @%s: is a candidate\n",
                    probed.device_name,
                    filename);
        }

        probed.key_count = 0;
        probed.abs_count = 0;
        probed.ff_count = 0;

        if((n = ioctl(fd, EVIOCGBIT(EV_KEY, KEY_CNT), probed.ev_key)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            //memdump(probed.ev_key, sizeof(probed.ev_key));
#endif
            for(int j = 0; j < KEY_CNT; ++j)
            {
                BOOL on = bit_get(probed.ev_key, j);
                if(on)
                {
#if XINPUT_TRACE_DEVICE_DETECTION
                    TRACE("%s ", xinput_linux_joystick_key_get_name(j)); /* no LF */
#endif
                    ++probed.key_count;
                }
            }

#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE(": %i keys\n", probed.key_count);
#endif
        }
        else
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("ev key: %s\n", strerror(errno));
#endif
        }

        if((n = ioctl(fd, EVIOCGBIT(EV_ABS, ABS_CNT), probed.ev_abs)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            memdump(probed.ev_abs, sizeof(probed.ev_abs));
#endif
            for(int j = 0; j < ABS_CNT; ++j)
            {
                BOOL on = bit_get(probed.ev_abs, j);
                if(on)
                {
#if XINPUT_TRACE_DEVICE_DETECTION
                    TRACE("%s ", xinput_linux_joystick_abs_get_name(j)); /* no LF */
#endif
                    ++probed.abs_count;
                }
            }
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE(": %i abs\n", probed.abs_count);
#endif
        }
        else
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE("ev abs: %s\n", strerror(errno));
#endif
        }

        if((n = ioctl(fd, EVIOCGBIT(EV_FF, FF_CNT), probed.ev_ff)) != -1)
        {
#if XINPUT_TRACE_DEVICE_DETECTION
            memdump(probed.ev_ff, sizeof(probed.ev_ff));
#endif
            for(int j = 0; j < FF_CNT; ++j)
            {
                BOOL on = bit_get(probed.ev_ff, j);
                if(on)
                {
#if XINPUT_TRACE_DEVICE_DETECTION
                    TRACE("%s ", xinput_linux_joystick_ff_get_name(j)); /* no LF */
#endif
                    ++probed.ff_count;
                }
            }
#if XINPUT_TRACE_DEVICE_DETECTION
            TRACE(": %i ff\n", probed.ff_count);
#endif
        }

        if(ioctl(fd, EVIOCGRAB, &one) < 0)
        {
            /* cannot grab it for myself */
            TRACE("cannot grab device: %s\n", strerror(errno));

            continue;
        }

        if(xinput_linux_joystick_generic_can_translate(&probed))
        {
            xinput_gamepad_device* device = &xinput_linux_joystick_slot[slot].device;
            xinput_linux_joystick_generic_new_instance(&probed, fd, device);
            xinput_linux_joystick_slot[slot].input_index = input_index;
        }
        /*
        if(xinput_linux_joystick_xboxpad_can_translate(&probed))
        {
            xinput_gamepad_device* device = &xinput_linux_joystick_slot[slot].device;
            xinput_linux_joystick_xboxpad_new_instance(&probed, fd, device);
            xinput_linux_joystick_slot[slot].input_index = input_index;
        }
        */
        /*
        else if(xinput_linux_joystick_xboxpad2_can_translate(&id))
        {
            xinput_gamepad_device* device = &xinput_linux_joystick_slot[slot].device;
            xinput_linux_joystick_xboxpad2_new_instance(&id, fd, device);
            xinput_linux_joystick_slot[slot].input_index = input_index;
        }
        */
        else
        {
            /* no more translators */
            close_ex(fd);
            continue;
        }

        mask |= 1 << slot;
    }

#if XINPUT_TRACE_DEVICE_DETECTION
    TRACE("devices probed\n");
#endif

    return mask;
}

xinput_gamepad_device* xinput_linux_joystick_get_device(int slot)
{
    if(slot >= 0 && slot < XUSER_MAX_COUNT)
    {
        if(xinput_linux_joystick_slot[slot].device.vtbl != NULL)
        {
            return &xinput_linux_joystick_slot[slot].device;
        }
    }

    return NULL;
}

void xinput_linux_joystick_device_close(int slot)
{
    xinput_gamepad_device* device = xinput_linux_joystick_get_device(slot);

    if(device != NULL)
    {
        device->vtbl->release(device);

        xinput_linux_joystick_slot[slot].device.data = NULL;
        xinput_linux_joystick_slot[slot].device.vtbl = NULL;
    }
    
    xinput_linux_joystick_slot[slot].input_index = -1;
}