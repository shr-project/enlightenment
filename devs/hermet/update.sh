#!/bin/sh

echo Updating EFL Packages

svn update

CFLAGS="-02 -g -W -Wall -Wextra -Wundef -Wshadow"

for e in eina eet evas_generic_loaders evas ecore eeze efreet embryo edje e_dbus eio emotion ethumb elementary e expedite clouseau terminology ephoto
do 
	if [ -e $e ] && [ -d $e ]
	then 
		echo Start $e
		rm ./build_"$e".log
		cd $e
		make clean
		make distclean
		./autogen.sh
#		CFLAGS=${CFLAGS} make -j 4 ${BUILD_LOG} && sudo make install ${BUILD_LOG} -a && sudo ldconfig
		CFLAGS=${CFLAGS} make -j 4 2>&1 | tee ../build_"$e".log && sudo make install 2>&1 | tee ../build_"$e".log -a && sudo ldconfig
		cd ..
		echo End Done
	fi 
done

exit 0
