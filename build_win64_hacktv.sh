#!/bin/bash

set -e
set -x

HOST=x86_64-w64-mingw32
PREFIX=/build_win64/install_root
export PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig

CROSS_HOST=$HOST- make -j4 EXTRA_LDFLAGS="-static" EXTRA_PKGS="libusb-1.0"
mv -f hacktv hacktv.exe || true
$HOST-strip hacktv.exe

echo "Done"