#!/bin/bash

set -euo pipefail

# Change versions below if you have different versions.
cff=21-257
mcl=21-257

# Change if you want to install somewhere else
INSTALL=/mcl/


# Now the rest of this script should have enough to run.

mcltar=mcl-$mcl.tar.gz
cfftar=cimfomfa-$cff.tar.gz

# wget http://micans.org/mcl/dev/$mcltar
# wget http://micans.org/mcl/dev/$cfftar

if true; then
  thedir=cimfomfa-$cff
  ( cd $thedir
    make -j10
    make install
  )
fi

if true; then
  thedir=mcl-$mcl
  ( cd $thedir
    make -j10
    make install
  )
fi

# Copy necessary file in mingw64
cp /mingw64/bin/libwinpthread-1.dll $INSTALL/bin/