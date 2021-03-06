#!/bin/sh

if [ "$1" = "" ]
then
    echo "usage: ./patch-wine.sh wine-source-path" 1>&2
    exit 1
fi

WINE_SOURCES="$1"

dod()
{
    err=$?
    if [ $err -ne 0 ]
    then
        echo $* 1>&2
        exit $err
    fi
}

if [ ! -d "$WINE_SOURCES/dlls" ]
then
    echo "expected 'dlls' directory in '$WINE_SOURCES'" 1>&2
    exit 1
fi

if [ ! -f "$WINE_SOURCES/configure.ac" ]
then
    echo "expected 'configure.ac' file in '$WINE_SOURCES'" 1>&2
    exit 2
fi


if [ ! -f $WINE_SOURCES/configure.patched ]
then
    echo "patching configure.ac"

    patch "$WINE_SOURCES/configure.ac" < configure.patch

    dod "configure patch failed"

    touch $WINE_SOURCES/configure.patched
fi

echo "patching wine source at '$WINE_SOURCES'"

cd $WINE_SOURCES
autoreconf
dod "failed to regenerate configure script"
cd -

# overwrites the build files

echo "writing xinputd files from '$(pwd)/dlls' in '$WINE_SOURCES'"

cp -a dlls "$WINE_SOURCES"

dod "failed to copy dll build files"

dlldir="$WINE_SOURCES/dlls/xinput1_3"

mkdir -p "$dlldir/linux_evdev"

dod "could not create '$dlldir/linux_evdev'"

cp ../src/linux_evdev/*.{c,h} "$dlldir/linux_evdev"

dod "could not copy linux-specific source files to '$dlldir'"

cp ../src/*.{c,h} "$dlldir"

dod "could not copy source files to '$dlldir'"

rm -f "$dlldir/config.h"

exit 0

