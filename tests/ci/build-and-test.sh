#!/bin/sh
set -e

if [ -d "build" ]; then
  rm build -rf
fi
cmake -S . -B build -GNinja -DMAINTAINER:BOOL=ON $@

# Build, Test & Install
cmake --build build
## No tests yet
#cd build && ctest && cd -

DESTDIR=/tmp/install_root/ cmake --install build
