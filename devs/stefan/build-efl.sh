#!/bin/sh

deplist="autotools-dev automake autopoint libtool zlib1g-dev
	libjpeg62-dev libfreetype6-dev libx11-dev subversion git
	libglib2.0-dev libxext-dev libxcursor-dev libudev-dev
	libcurl4-gnutls-dev libc-ares-dev liblua5.1-0-dev libpng12-dev
	libtiff4-dev libfontconfig1-dev libxcb-shape0-dev
	libxrender-dev libgif-dev libglu1-mesa-dev mesa-common-dev
	librsvg2-dev libfribidi-dev libpixman-1-dev libxss-dev
	libxp-dev libxtst-dev graphviz libasound2-dev libpam0g-dev"

# Used whenever cedric pokes me again to do the scan build for the current stable branch
#defcore="ecore-1.7 eet-1.7 eina-1.7 eio-1.7 embryo-1.7 evas-1.7
#         evas_generic_loaders-1.7 edje-1.7 emotion-1.7 eeze-1.7
#         e_dbus-1.7 efreet-1.7 elementary-1.7"
defcore="efl evas_generic_loaders ephysics edje emotion eeze e_dbus edbus
	efreet PROTO/libeweather PROTO/emap elementary"
defapps="ethumb terminology e ephoto rage expedite"

defpkgs="$defcore $defapps"

# fail on errors
set -e
#set -x

# Add -std=gnu99 when having problems with clang and dns.c in ecore_con
export CFLAGS="-O2 -Wall -g -Wextra -Wshadow -fvisibility=hidden -fdata-sections -ffunction-sections"
export CXXFLAGS="$CFLAGS"
export LDFLAGS="-fvisibility=hidden -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,--as-needed"

do_scan_build()
{
	local e flags

	e="$1"
	echo
	echo "Scan build for $e"
	echo
	case $e in
	*)
		flags=""
		;;
	esac
	(cd "$e" && scan-build -analyze-headers --use-cc=/usr/bin/clang ./configure $flags && scan-build -o ~/EFL/scan-build-reports/$e make $MAKEFLAGS) || exit 1
}

do_build_and_install()
{
	local e flags

	e="$1"
	echo
	echo "Building $e"
	echo
	case $e in
	efl)
		flags="--enable-tests"
		;;
	emotion)
		flags="--disable-generic-vlc"
		;;
	*)
		flags=""
		;;
	esac
	[ $no_autogen = 1 ] || (cd "$e" && rm -f config.cache && ./autogen.sh $flags) || exit 1
	(cd "$e" && make "$MAKEFLAGS" && sudo make install && sudo ldconfig) || exit 1
	echo
	echo "Built $e"
	echo
}

do_install_dependencies()
{
	sudo apt-get -y install $deplist
}

usage()
{
local scriptname
scriptname="`basename $0`"
cat <<EOF
Usage:
$scriptname [--up] [--deps] [--help] [--force] [pkgs]

$scriptname builds and installs binaries from the E SVN.
It is written to be run from an SVN checkout or git svn clone.
When used from git svn, it can rebuild only packages that
have changed in git since the last rebuild.

Options:

  --deps    Install build dependencies (Ubuntu 11.04 only)
  --up      Update mode (avoid running autoconf & configure)
  --scan    Use scan-build for static analyze
  --help    Show this message

Modules built by default are:
$defpkgs

The default mode of build is simple build and install.

EOF
exit 0
}

# how many CPUs?
if [ -f /proc/cpuinfo ]
then
	cpus="`grep ^processor /proc/cpuinfo | wc -l`"
        cpus=$(($cpus+2))         
fi
[ "$cpus" ] || cpus=4
MAKEFLAGS="-j$cpus"

no_autogen=0
debian=0
force=0
install_deps=0
scan_build=0

while [ $# -ge 1 ]
do
	case "$1" in
	--up)
		no_autogen=1
		;;
	--deps)
		install_deps=1
		;;
	--scan)
		scan_build=1
		;;
	--help)
		usage
		;;
	-*)
		echo "Unknown option $1" >&2
		exit 1
		;;
	*)
		what="$what $1"
		;;
	esac
	shift
done

[ $install_deps = 0 ] || do_install_dependencies

if [ ! "$what" ]
then
	what="$defpkgs"
fi

if [ "$scan_build" = 1 ]
then
   for e in $defcore
   do
   	do_scan_build "$e"
   done
   exit 0
fi

for e in $what
do
	if [ ! -d "$e" ]
	then
		echo "$e directory missing?"
		exit 1
	fi

	do_build_and_install "$e"
done
