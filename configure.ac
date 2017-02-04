#
# Run ./pre-build to generate a configure script from this file.
# 

AC_PREREQ([2.69])
AC_INIT([xinputd], [0.1], [eric.diaz.fernandez@gmail.com])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_MACRO_DIRS([m4])

LT_INIT

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Check for OS

AM_CONDITIONAL([OS_LINUX], [ [[ $(uname) = "Linux" ]] ])
AM_CONDITIONAL([OS_OSX], [ [[ $(uname) = "Darwin" ]] ])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stddef.h stdint.h stdlib.h sys/file.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([ftruncate gettimeofday memset munmap strdup strerror])

dnl Check for pthread_create which may be in -lpthread
ac_save_LIBS=$LIBS
AC_SEARCH_LIBS(pthread_create, pthread,
               [AC_DEFINE(HAVE_PTHREAD, 1, [Define to 1 if you have the `pthread_create' function.])
                               test "$ac_res" = "none required" || AC_SUBST(PTHREAD_LIBS,"$ac_res")])
LIBS=$ac_save_LIBS

dnl Check for shm_open which may be in -lrt
ac_save_LIBS=$LIBS
AC_SEARCH_LIBS(shm_open, rt,
               [AC_DEFINE(HAVE_SHM_OPEN, 1, [Define to 1 if you have the `shm_open' function.])
                               test "$ac_res" = "none required" || AC_SUBST(SHM_LIBS,"$ac_res")])
LIBS=$ac_save_LIBS

dnl Check for mq_open which may be in -lrt
ac_save_LIBS=$LIBS
AC_SEARCH_LIBS(mq_open, rt,
               [AC_DEFINE(HAVE_MQ_OPEN, 1, [Define to 1 if you have the `mq_open' function.])
                               test "$ac_res" = "none required" || AC_SUBST(MQ_LIBS,"$ac_res")])
LIBS=$ac_save_LIBS

AC_CHECK_HEADERS([linux/input.h])

#
AC_MSG_CHECKING([wxWidgets]);
wx_cxxflags=$(wx-config --cxxflags 2> /dev/null)
if [[ $? -eq 0 ]]
then
    wx_libs=$(wx-config --libs 2> /dev/null)
    if [[ $? -eq 0 ]]
    then
        AC_MSG_RESULT([yes])
        AC_DEFINE(HAVE_WX_CONFIG, 1, [Define to 1 if wxWidgets is present])
        AC_SUBST(WX_CXXFLAGS, "$wx_cxxflags")
        AC_SUBST(WX_LIBS, "$wx_libs")

	AM_CONDITIONAL([WXWIDGETS], [ true ])
    else
        AC_MSG_RESULT([misconfigured?])

	AM_CONDITIONAL([WXWIDGETS], [ false ])
    fi
else
    AC_MSG_RESULT([no])
    AM_CONDITIONAL([WXWIDGETS], [ false ])
fi
#

AC_ARG_ENABLE([wine], AS_HELP_STRING([--enable-wine], [Enable WINE code]))
AS_IF([test "x$enable_wine" = "xyes"],
      [
        AM_CONDITIONAL([HAVE_WINE], [ true ])
        AC_DEFINE_UNQUOTED([HAVE_WINE], [1], "Set to 1 for code running in WINE")
        AC_SUBST([HAS_WINE])
      ],
      [
        AM_CONDITIONAL([HAVE_WINE], [ false ])
        AC_DEFINE_UNQUOTED([HAVE_WINE], [0], "Set to 1 for code running in WINE")
        AC_SUBST([HAS_WINE])
      ]
      )

dnl AC_CONFIG_SRCDIR([src test/xinput-test test/xinput-test-gui])
AC_CONFIG_FILES([Makefile src/Makefile test/xinput-test/Makefile test/xinput-test-gui/Makefile])
AC_OUTPUT

