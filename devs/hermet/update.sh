#!/bin/sh

echo Updating EFL Packages

svn update

for e in efl evas_generic_loaders evas ecore eeze efreet edje e_dbus eio emotion ethumb elementary e expedite clouseau terminology ephoto
do 
	if [ -e $e ] && [ -d $e ]
	then 
		echo Start $e
		rm ./build_"$e".log
		cd $e
		make clean
		make distclean
		./autogen.sh
		make -j8 2>&1 | tee ../build_"$e".log && sudo make install && sudo ldconfig 2>&1 | tee ../build_"$e".log -a
		cd ..
		echo End Done
	fi 
done

exit 0
