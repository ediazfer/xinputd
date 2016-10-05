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

#include <linux/input.h>

static const char* EVENT_TYPES_STR[] =
{
    "SYN",          /*  0 */
    "KEY",
    "REL",
    "ABS",
    "MSC",
    "SW",           /*  5 */
    "0x06",
    "0x07",
    "0x08",
    "0x09",
    "0x0a",
    "0x0b",
    "0x0c",
    "0x0d",
    "0x0e",
    "0x0f",
    "0x10",
    "LED",          /*  0x11 */
    "SND",
    "0x13",
    "REP",          /*  0x14 */
    "FF",
    "PWR",
    "FF_STATUS",    /*  0x17 */
};

const char* xinput_linux_input_event_type_get_name(int idx)
{
    if(idx >= EV_SYN && idx <= EV_FF_STATUS)
    {
        return EVENT_TYPES_STR[idx];
    }
    else
    {
        return "?";
    }
}

static const char* KEY_NAMES_STR[] =
{
    "BTN_0",        /*  0x100 */
    "BTN_1",        /*  0x101 */
    "BTN_2",        /*  0x102 */
    "BTN_3",        /*  0x103 */
    "BTN_4",        /*  0x104 */
    "BTN_5",        /*  0x105 */
    "BTN_6",        /*  0x106 */
    "BTN_7",        /*  0x107 */
    "BTN_8",        /*  0x108 */
    "BTN_9",        /*  0x109 */
    "0x10a",
    "0x10b",
    "0x10c",
    "0x10d",
    "0x10e",
    "0x10f",
    "BTN_LEFT",     /*  0x110 */
    "BTN_RIGHT",    /*  0x111 */
    "BTN_MIDDLE",   /*  0x112 */
    "BTN_SIDE",     /*  0x113 */
    "BTN_EXTRA",    /*  0x114 */
    "BTN_FORWARD",  /*  0x115 */
    "BTN_BACK",     /*  0x116 */
    "BTN_TASK",     /*  0x117 */
    "0x118",
    "0x119",
    "0x11a",
    "0x11b",
    "0x11c",
    "0x11d",
    "0x11e",
    "0x11f",
    "BTN_TRIGGER",  /*  0x120 */
    "BTN_THUMB",    /*  0x121 */
    "BTN_THUMB2",   /*  0x122 */
    "BTN_TOP",      /*  0x123 */
    "BTN_TOP2",     /*  0x124 */
    "BTN_PINKIE",   /*  0x125 */
    "BTN_BASE",     /*  0x126 */
    "BTN_BASE2",    /*  0x127 */
    "BTN_BASE3",    /*  0x128 */
    "BTN_BASE4",    /*  0x129 */
    "BTN_BASE5",    /*  0x12a */
    "BTN_BASE6",    /*  0x12b */
    "0x12c",
    "0x12d",
    "0x12e",
    "BTN_DEAD",     /*  0x12f */

    "BTN_SOUTH/A",  /*  0x130 */
    "BTN_EAST/B",   /*  0x131 */
    "BTN_C",        /*  0x132 */
    "BTN_NORTH/X",  /*  0x133 */
    "BTN_WEST/Y",   /*  0x134 */
    "BTN_Z",        /*  0x135 */
    "BTN_TL",       /*  0x136 */
    "BTN_TR",       /*  0x137 */
    "BTN_TL2",      /*  0x138 */
    "BTN_TR2",      /*  0x139 */
    "BTN_SELECT",   /*  0x13a */
    "BTN_START",    /*  0x13b */
    "BTN_MODE",     /*  0x13c */
    "BTN_THUMBL",   /*  0x13d */
    "BTN_THUMBR",   /*  0x13e */
    "0x13f"         /*  0x13f */
};

const char* xinput_linux_input_key_get_name(int idx)
{
    if(idx >= BTN_MISC && idx <= BTN_DIGI)
    {
        return KEY_NAMES_STR[idx - BTN_MISC];
    }
    else
    {
        return "?";
    }
}

static const char* ABS_NAMES_STR[] =
{
    "ABS_X",                /* 0x00 */
    "ABS_Y",                /* 0x01 */
    "ABS_Z",                /* 0x02 */
    "ABS_RX",               /* 0x03 */
    "ABS_RY",               /* 0x04 */
    "ABS_RZ",               /* 0x05 */
    "ABS_THROTTLE",         /* 0x06 */
    "ABS_RUDDER",           /* 0x07 */
    "ABS_WHEEL",            /* 0x08 */
    "ABS_GAS",              /* 0x09 */
    "ABS_BRAKE",            /* 0x0a */
    "0x0b",
    "0x0c",
    "0x0d",
    "0x0e",
    "0x0f",
    "ABS_HAT0X",            /* 0x10 */
    "ABS_HAT0Y",            /* 0x11 */
    "ABS_HAT1X",            /* 0x12 */
    "ABS_HAT1Y",            /* 0x13 */
    "ABS_HAT2X",            /* 0x14 */
    "ABS_HAT2Y",            /* 0x15 */
    "ABS_HAT3X",            /* 0x16 */
    "ABS_HAT3Y",            /* 0x17 */
    "ABS_PRESSURE",         /* 0x18 */
    "ABS_DISTANCE",         /* 0x19 */
    "ABS_TILT_X",           /* 0x1a */
    "ABS_TILT_Y",           /* 0x1b */
    "ABS_TOOL_WIDTH",       /* 0x1c */
    "0x1d",
    "0x1e",
    "0x1f",
    "ABS_VOLUME",           /* 0x20 */
    "0x21",
    "0x22",
    "0x23",
    "0x24",
    "0x25",
    "0x26",
    "0x27",
    "ABS_MISC",             /* 0x28 */
    "0x29",
    "0x2a",
    "0x2b",
    "0x2c",
    "0x2d",
    "0x2e",
    "ABS_MT_SLOT",           /* 0x2f     MT slot being modified */
    "ABS_MT_TOUCH_MAJOR",    /* 0x30     Major axis of touching ellipse */
    "ABS_MT_TOUCH_MINOR",    /* 0x31     Minor axis (omit if circular) */
    "ABS_MT_WIDTH_MAJOR",    /* 0x32     Major axis of approaching ellipse */
    "ABS_MT_WIDTH_MINOR",    /* 0x33     Minor axis (omit if circular) */
    "ABS_MT_ORIENTATION",    /* 0x34     Ellipse orientation */
    "ABS_MT_POSITION_X",     /* 0x35     Center X touch position */
    "ABS_MT_POSITION_Y",     /* 0x36     Center Y touch position */
    "ABS_MT_TOOL_TYPE",      /* 0x37     Type of touching device */
    "ABS_MT_BLOB_ID",        /* 0x38     Group a set of packets as a blob */
    "ABS_MT_TRACKING_ID",    /* 0x39     Unique ID of initiated contact */
    "ABS_MT_PRESSURE",       /* 0x3a     Pressure on contact area */
    "ABS_MT_DISTANCE",       /* 0x3b     Contact hover distance */
    "ABS_MT_TOOL_X",         /* 0x3c     Center X tool position */
    "ABS_MT_TOOL_Y"          /* 0x3d     Center Y tool position */
};

const char* xinput_linux_input_abs_get_name(int idx)
{
    if(idx >= ABS_X && idx <= ABS_MT_TOOL_Y)
    {
        return ABS_NAMES_STR[idx];
    }
    else
    {
        return "?";
    }
}

static const char* FF_NAMES_STR[] = /* [ 0x50: 0x61 ] */
{
    "FF_RUMBLE",    /* 0x50 */
    "FF_PERIODIC",
    "FF_CONSTANT",
    "FF_SPRING",
    "FF_FRICTION",
    "FF_DAMPER",
    "FF_INERTIA",
    "FF_RAMP",      /* 0x57 */

    "FF_SQUARE",
    "FF_TRIANGLE",
    "FF_SINE",
    "FF_SAW_UP",
    "FF_SAW_DOWN",
    "FF_CUSTOM",
    "0x5e",
    "0x5f",
    "FF_GAIN",      /* 0x60 */
    "FF_AUTOCENTER"
};

const char* xinput_linux_input_ff_get_name(int idx)
{
    if(idx >= FF_RUMBLE && idx <= FF_AUTOCENTER)
    {
        return FF_NAMES_STR[idx - FF_RUMBLE];
    }
    else
    {
        return "?";
    }
}


#endif /* HAVE_LINUX_INPUT_H */
