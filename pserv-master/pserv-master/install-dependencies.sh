#!/bin/bash

#
# Run this script to install the required Jansson and JWT packages.
#
BASE=`pwd`
#
# Running this script multiple times will wipe the existing build
# and reclone both repositories
/bin/rm -rf ${BASE}/deps jansson libjwt
#
test -d ${BASE}/deps || mkdir ${BASE}/deps
git clone https://github.com/akheron/jansson.git
(cd jansson;
    autoreconf -fi;
    ./configure --prefix=${BASE}/deps;
    make install
)

git clone https://git@github.com/benmcollins/libjwt.git
(cd libjwt;
    autoreconf -fi;
    env PKG_CONFIG_PATH=../deps/lib/pkgconfig:${PKG_CONFIG_PATH} ./configure --prefix=${BASE}/deps;
    make install
)
