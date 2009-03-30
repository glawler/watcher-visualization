#!/bin/sh
aclocal -I aclocal
autoconf
automake --add-missing --foreign
