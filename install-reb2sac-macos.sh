#! /bin/sh
if [ -f "Makefile" ]; then
    env WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" make -k -j1 install
else
    CFLAGS="-I${BIOSIM}/include -O0 -g3 -mmacosx-version-min=10.9 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -arch x86_64 -DNAME_FOR_ID" \
    LDFLAGS="-L/usr/local/lib -lm" ./configure --prefix=${PWD}/../iBioSim/ && \
    env WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" make -k -j1 install
fi
