#!/bin/sh
libtoolize --force
aclocal -I aclocal
autoconf
automake --add-missing --foreign
