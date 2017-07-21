lib_LTLIBRARIES=libxinput.la
sbin_PROGRAMS=xinputd

libxinput_ladir=$(includedir)
libxinput_la_LIBADD=$(PTHREAD_LIBS) $(SHM_LIBS) $(MQ_LIBS)
libxinput_la_SOURCES=dll.c debug.c tools.c xinput_gamepad.c xinput_service.c

if OS_LINUX
libxinput_la_SOURCES+=linux_evdev/xinput_linux_evdev.c linux_evdev/xinput_linux_evdev_translator.c linux_evdev/xinput_linux_evdev_debug.c linux_evdev/xinput_linux_evdev_generic.c
endif

#ifeq ($(OS),Darwin)
#endif

#libxinput_la_SOURCES+=linux_evdev/xinput_linux_evdev_xboxpad.c linux_evdev/xinput_linux_evdev_xboxpad_2.c 
libxinput_la_HEADERS=xinput.h xinput_types.h


xinputddir=.
xinputd_LDADD=-lxinput
xinputd_SOURCES=main.c server.c

noinst_HEADERS=xinput_settings.h debug.h tools.h xinput_gamepad.h xinput_service.h device_id.h

if OS_LINUX
noinst_HEADERS+=linux_evdev/xinput_linux_evdev.h linux_evdev/xinput_linux_evdev_translator.h linux_evdev/xinput_linux_evdev_debug.h linux_evdev/xinput_linux_evdev_generic.h 
endif

#noinst_HEADERS+=linux_evdev/xinput_linux_evdev_xboxpad.h linux_evdev/xinput_linux_evdev_xboxpad_2.h
