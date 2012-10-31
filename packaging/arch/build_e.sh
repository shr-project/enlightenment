#!/bin/sh
die () {
   echo >&2 "$@"
   exit 1
}

[ "$#" -eq 1 ] || die "Usage: ./build_e.sh <pkgver>"
ver=$1
dirs="efl-svn evas_generic_loaders-svn evas-svn ecore-svn efreet-svn e_dbus-svn eio-svn emotion-svn edje-svn eeze-svn ethumb-svn elementary-svn e-svn terminology-svn"

# Just to cache the sudo pass.
sudo test
for a in $dirs
do
   if [ -d $a ]
   then
      pushd $a
      echo "Making '$a'"
      sed -i "s/pkgver=.*\$/pkgver=$ver/" PKGBUILD
      makepkg #|| exit
      sudo pacman -U --noconfirm $a-$ver*.tar.xz
      popd
   fi
done


