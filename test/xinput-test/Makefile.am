bin_PROGRAMS=xinput-test

xinput_testdir=$(includedir)
xinput_test_CFLAGS = $(AM_CFLAGS)
#xinput_test_LDADD=-L$(libdir) -L../../src  -lxinput
xinput_test_LDADD=$(abs_top_builddir)/src/.libs/libxinput.so
xinput_test_LDFLAGS=-rpath $(abs_top_builddir)/src/.libs
xinput_test_SOURCES=xinput-test.c

