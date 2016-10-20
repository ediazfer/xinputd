lib_LTLIBRARIES=libxinput.la
sbin_PROGRAMS=xinputd

libxinput_ladir=$(includedir)
libxinput_la_LIBADD=$(PTHREAD_LIBS) $(SHM_LIBS) $(MQ_LIBS)
libxinput_la_SOURCES=dll.c debug.c tools.c xinput_gamepad.c xinput_service.c
libxinput_la_SOURCES+=linux_input/xinput_linux_input.c linux_input/xinput_linux_input_translator.c linux_input/xinput_linux_input_debug.c linux_input/xinput_linux_input_generic.c
#libxinput_la_SOURCES+=linux_input/xinput_linux_input_xboxpad.c linux_input/xinput_linux_input_xboxpad_2.c 
libxinput_la_HEADERS=xinput.h xinput_types.h


xinputddir=.
xinputd_LDADD=-lxinput
xinputd_SOURCES=main.c server.c

noinst_HEADERS=xinput_settings.h debug.h tools.h xinput_gamepad.h xinput_service.h device_id.h
noinst_HEADERS+=linux_input/xinput_linux_input.h linux_input/xinput_linux_input_translator.h linux_input/xinput_linux_input_debug.h linux_input/xinput_linux_input_generic.h 
#noinst_HEADERS+=linux_input/xinput_linux_input_xboxpad.h linux_input/xinput_linux_input_xboxpad_2.h
