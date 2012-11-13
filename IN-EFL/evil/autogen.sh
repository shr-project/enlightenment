#!/bin/sh

echo "Do not use this tree anymore - use the efl tree instead."
exit 1

rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh

autoreconf -f -i

if [ -z "$NOCONFIGURE" ]; then
	./configure "$@"
fi
