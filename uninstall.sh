#!/bin/bash

cd ~/exodus

test -f Makefile && sudo make uninstall

pushd /usr/local
sudo rm -rf include/exodus*
sudo rm -f  lib/libexodus*
sudo rm -f  lib/pkgconfig/exodus*.pc
sudo rm -f  bin/exodus
sudo rm -f  bin/configexodus
find . | grep exodus
popd
