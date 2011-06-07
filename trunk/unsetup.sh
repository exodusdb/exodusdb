#!/bin/bash

#ICU

pushd /usr/local
sudo rm -rf include/unicode
sudo rm -rf include/layout
sudo rm -f  bin/derb
sudo rm -f  bin/genbrk
sudo rm -f  bin/gencfu
sudo rm -f  bin/gencnval
sudo rm -f  bin/genctd
sudo rm -f  bin/genrb
sudo rm -f  bin/icu-config
sudo rm -f  bin/icuinfo
sudo rm -f  bin/makeconv
sudo rm -f  bin/uconv
sudo rm -f  lib/libicu*
sudo rm -rf lib/icu
sudo rm -f  lib/pkgconfig/icu*.pc
sudo rm -rf share/icu
sudo rm -f  share/man/man1/icu-config.1
sudo rm -f  share/man/man8/icupkg.8
sudo rm -f  sbin/icupkg
find . | grep icu
popd

#Boost

pushd /usr/local
sudo rm -rf include/boost
sudo rm -f  lib/libboost*
sudo rm -f  pkgconfig/boost*.pc
find . | grep boost
popd

test "$EXO_BATCHMODE" = "" && read -p "Press Enter:"
