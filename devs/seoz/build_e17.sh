#!/bin/bash

####################
# EFL 1.7.x + E17  #
####################

set -e
#set -x
shopt -s expand_aliases

unset LANG
export CFLAGS="-O2 -march=native -ffast-math -g3 -W -Wall -Wextra" # -Wshadow"
export CC="ccache gcc"
alias make='make -j6'

PWD=`pwd`
LOG_WARN_FILE=$PWD"/warnings.txt"
LOG_WARN_TMP_FILE=$PWD"/warnings_tmp.txt"

export EFL_BRANCHES="eina-1.7 eet-1.7 embryo-1.7 evas-1.7 ecore-1.7 efreet-1.7 eio-1.7 edje-1.7 e_dbus-1.7 emotion-1.7 ethumb-1.7 eeze-1.7 elementary-1.7 evas_generic_loaders-1.7 expedite-1.7"
export BUILD_E_MODULES="E-MODULES-EXTRA/comp-scale E-MODULES-EXTRA/engage"
export BUILD_ETC="econnman exactness efx editje PROTO/eyelight ephoto edje_viewer PROTO/azy elmdentica enlil PROTO/emote emprint PROTO/enna-explorer ensure exquisite rage PROTO/eyesight"
export BUILD_EXAMPLE="EXAMPLES/elementary/calculator EXAMPLES/elementary/converter EXAMPLES/elementary/phonebook EXAMPLES/elementary/sticky-notes"

function build()
{
	build_dir=$1
	autogen_option=$2
	for I in $build_dir; do
  	pushd $I
		echo " "
		echo "============ "$I" ============"
		echo "" >> $LOG_WARN_FILE
		echo "["$I"]" >> $LOG_WARN_FILE
		if [ -f Makefile ]; then
			make clean distclean || true
		fi
		./autogen.sh "$autogen_option"
		make 3>&1 1>&2 2>&3 | tee $LOG_WARN_TMP_FILE
		cat $LOG_WARN_TMP_FILE >> $LOG_WARN_FILE
		rm $LOG_WARN_TMP_FILE
		sudo make install
		sudo ldconfig
  	popd
	done
}

function e17_restart()
{
	echo ""
	echo "=========== ENLIGHTENMENT RESTART ============"
	enlightenment_remote -restart
}

pushd branches
	build "$EFL_BRANCHES"
popd

pushd trunk
	build e
	build "$BUILD_E_MODULES $BUILD_ETC $BUILD_EXAMPLE"
	build "$BUILD_ETC $BUILD_EXAMPLE"
popd

e17_restart
