#! /usr/bin/sh

if [ -f "Makefile" ]; then
env WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" make -k -j1 install
else
CFLAGS="-I${BIOSIM}/include -O0 -g3 -DNAME_FOR_ID -DWINDOWS" \
LDFLAGS="-lm -L${BIOSIM}/reb2sac/win/lib" ./configure --prefix=${PWD}/.. && \
env WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" make -k -j1 install
fi

