#!/bin/sh

WINE_SOURCES=/tmp/wine

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

if [ ! -d "$WINE_SOURCES/configure.ac" ]
then
    echo "expected 'configure.ac' file in '$WINE_SOURCES'" 1>&2
    exit 2
fi

patch "$WINE_SOURCES/configure.ac" < configure.patch

dod "configure patch failed"

# overwrites the build files

cp -a dlls "$WINE_SOURCES"

dod "failed to copy dll build files"

dlldir="$WINE_SOURCES/dlls/xinput1_3"

mkdir -p "$dlldir/linux_input"

dod "could not create '$dlldir/linux_input'"

cp "linux_input/*.{c,h}" "$dlldir/linux_input"

dod "could not copy linux-specific source files to '$dlldir'"

cp "*.{c,h}" "$dlldir"

dod "could not copy source files to '$dlldir'"

rm -f "$dlldir/config.h"

exit 0

