#!/bin/sh
VERSION=2.7

WINE=wine-$VERSION.tar.xz
WSTG=v$VERSION.tar.gz

WINEDIR=$(pwd)/wine-$VERSION
WSTGDIR=$(pwd)/wine-staging-$VERSION

WSARC=$WSTGDIR.tar.xz

doe()
{
    err=$?
    if [ $err -ne 0 ]
    then
        echo "ERROR: $*" 1>&2
        exit $?
    fi
}

# check the archive exists already

if [ ! -f $WSARC ]
then

    # need the wine source directory

    if [ ! -d $WINEDIR ]
    then
        # need the wine source archive

        if [ ! -f $WINE ]
        then
            wget http://prdownloads.sourceforge.net/wine/$WINE
            doe get $WINE
        fi

        tar -xf $WINE
        doe untar $WINE
    fi

    # check staging has been applied

    if [ ! -f $WINEDIR/.staging ]
    then

        # need the wine-staging source directory

        if [ ! -d $WSTGDIR ]
        then
            # need the wine-staging source archive

            if [ ! -f $WSTG ]
            then
                wget https://github.com/wine-compholio/wine-staging/archive/$WSTG
                doe get $WSTG
            fi

            tar -xf $WSTG
            doe untar $WSTG
        fi

        # apply the wine-staging patch

        cd $WSTGDIR/patches
        doe cd $WSTGDIR/patches
        ./patchinstall.sh DESTDIR=$WINEDIR --all
        doe failed to patch
        cd ../..
        touch $WINEDIR/.staging
    fi

    # check xinputd has been applied

    if [ ! -f $WINEDIR/.xinputd ]
    then
        # need xinputd source directory

        if [ ! -d xinputd ]
        then
            git clone https://github.com/ediazfer/xinputd.git
            doe get xinputd
        fi

        # apply the xinputd patch

        cd xinputd/wine
        doe xinputd/wine
        echo "Running $(pwd)/patch-wine.sh $WINEDIR"
        ./patch-wine.sh $WINEDIR
        doe patch-wine
        cd ../..

        touch $WINEDIR/.xinputd
    fi

    tar --transform="flags=r;s|wine-$VERSION|wine-staging-$VERSION|" -cvJf $WSARC wine-$VERSION
    doe "archive creation failed"
else
    echo "archive ready"
fi


