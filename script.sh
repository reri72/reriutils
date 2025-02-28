#!/bin/sh

aclocal
autoheader
autoconf
automake --force-missing --add-missing
./configure

# ./configure --with-openssl=yes