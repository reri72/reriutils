#!/bin/sh

aclocal
autoheader
autoconf
automake --force-missing --add-missing

for arg in "$@"; do
    case "$arg" in
        mysql)
            CONF_ARGS="$CONF_ARGS --with-mysql=yes"
            ;;
        openssl)
            CONF_ARGS="$CONF_ARGS --with-openssl=yes"
            ;;
        *)
            CONF_ARGS="$CONF_ARGS $arg"
            ;;
    esac
done

echo "./configure with arguments: $CONF_ARGS"

./configure $CONF_ARGS