#! /bin/sh
ARCH=$(uname -m)
OS=$(uname)
if [ -f "Makefile" ]; then
    env WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" make -k -j1 install
else
    if test $OS = "Darwin" 
    then
	CFLAGS="-I${BIOSIM}/include -O0 -g3 -mmacosx-version-min=10.9 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk -arch x86_64 -DNAME_FOR_ID" \
	    LDFLAGS="-L${BIOSIM}/lib64 -lm" ./configure --prefix=${PWD}/.. && \
	    env WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" make -k -j1 install
    else 
	if test $ARCH = "x86_64" 
	then
	    CFLAGS="-I${BIOSIM}/include -O0 -g3 -DNAME_FOR_ID" LDFLAGS="-L${BIOSIM}/lib64 -lm" ./configure --prefix=${PWD}/.. && \
		env WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" make -k -j1 install
	else
	    CFLAGS="-I${BIOSIM}/include -O0 -g3 -DNAME_FOR_ID" LDFLAGS="-L${BIOSIM}/lib -lm" ./configure --prefix=${PWD}/.. && \
		env WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" make -k -j1 install
	fi
    fi
fi
