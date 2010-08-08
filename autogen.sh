#!/bin/sh

mkdir m4

# Mac OSX has glibtoolize, but no libtoolize.
libtoolize=`which glibtoolize 2> /dev/null`
if test $? -ne 0; then
	libtoolize=`which libtoolize 2> /dev/null`
fi

# Prefer autoreconf it if exists.
which autoreconf > /dev/null 2>&1
if test $? -ne 0; then
    LIBTOOLIZE=$libtoolize \
	autoreconf -I m4 -i
else
    $libtoolize
    aclocal -I m4
    autoheader
    automake --foreign --add-missing
    autoconf
fi

