#!/bin/sh
set -e

if [ -d "build" ]; then
  rm build -rf
fi
set -x

cmake -S . -B build -GNinja \
  -DMAINTAINER:BOOL=ON \
  $@

# Build, Test & Install
cmake --build build

## No tests yet
#cd build && ctest && cd -

DUMMY_DESTDIR=/tmp/install-root/
rm -rf $DUMMY_DESTDIR

# Test installation
DESTDIR=$DUMMY_DESTDIR cmake --install build
