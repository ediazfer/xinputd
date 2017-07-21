bin_PROGRAMS=xinput-gui-test

xinput_gui_testdir=@datadir@/xinput

xinput_gui_test_CPPFLAGS=$(WX_CXXFLAGS) -I$(INCLUDEDIR) -DXINPUT
#xinput_gui_test_LDADD=$(WX_LIBS) -L../../src -lxinput
xinput_gui_test_LDADD=$(abs_top_builddir)/src/.libs/libxinput.so $(WX_LIBS)
xinput_gui_test_LDFLAGS=-rpath $(abs_top_builddir)/src/.libs
xinput_gui_test_SOURCES=wxXinputApp.cpp wxXinputMain.cpp
noinst_HEADERS=wxXinputApp.h wxXinputMain.h

xinput_gui_test_DATA = bitmaps/xinputcontrollerbutton-a.png bitmaps/xinputcontrollerpad-dl.png bitmaps/xinputcontrollerpad-d.png bitmaps/xinputcontrollerbutton-rb.png bitmaps/xinputcontrollerpad-l.png bitmaps/xinputcontrollerpad-ur.png bitmaps/xinputcontrollerbutton-lb.png bitmaps/xinputcontrollerbutton-y.png bitmaps/xinputcontrollerbutton-guide.png bitmaps/xinputcontrollerbutton-x.png bitmaps/xinputcontrollerbutton-start.png bitmaps/xinputcontrollerpad-u.png bitmaps/xinputcontrollerpad-r.png bitmaps/xinputcontrollerpad-ul.png bitmaps/xinputcontrollerbutton-b.png bitmaps/xinputcontroller.png bitmaps/xinputcontrollerpad-dr.png bitmaps/xinputcontrollerbutton-select.png
