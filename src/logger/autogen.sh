#!/bin/sh
libtoolize --force
if [ ! -d aclocal ]; then
	mkdir aclocal
fi
aclocal -I aclocal
autoconf
automake --add-missing --foreign
