#!/bin/sh
libtoolize
aclocal -I aclocal
autoconf
automake --add-missing --foreign
