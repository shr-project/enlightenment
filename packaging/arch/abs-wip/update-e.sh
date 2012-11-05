#!/bin/bash

PKGS="$@"
if [ -z "$PKGS" ]; then
    PKGS="efl evas ecore eio edje efreet e_dbus edbus eeze emotion ethumb elementary evas_generic_loaders e ephysics terminology"
fi

if [ -z "$MAKEPKG_OPTS" ]; then
    MAKEPKG_OPTS="--check"
fi
echo "Build command: makepkg $MAKEPKG_OPTS"

if [ -z "$PACMAN_OPTS" ]; then
    PACMAN_OPTS="--noconfirm"
fi
echo "Build command: pacman $PACMAN_OPTS"

die()
{
    echo "ERROR: $*"
    exit 1
}

D="$PWD"
for p in $PKGS; do
    echo "Create: $p"

    cd "$D/$p-svn" || die "cd $p-svn"

    rm -f $p-svn-*.pkg.tar.xz
    rm -f $p-svn-namcap.log

    makepkg $MAKEPKG_OPTS || die "makepkg $p-svn"

    namcap $p-svn-*.pkg.tar.xz > $p-svn-namcap.log
    echo "--- PACKAGE CHECK ($p-svn-namcap.log) ---"
    cat $p-svn-namcap.log
    echo "--- END PACKAGE CHECK ($p-svn-namcap.log) ---"

    echo "Install generated package $p-svn-*.pkg.tar.xz"
    sudo /usr/bin/pacman -U $PACMAN_OPTS $p-svn-*.pkg.tar.xz || die "pacman $p-svn-*.pkg.tar.xz"
done
