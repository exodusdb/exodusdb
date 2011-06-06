#!/bin/bash


echo 1. Build and install ICU
cd ~
#svn co http://source.icu-project.org/repos/icu/icu/tags/release-4-8/ icu
rm icu4c-4_8-src.tgz
curl -L -O http://download.icu-project.org/files/icu4c/4.8/icu4c-4_8-src.tgz | tee icu4c-4_8-src.tgz | tar xz
cd icu/source
./configure --enable-extras=no \
  --enable-extras=no \
  --enable-icuio=no \
  --enable-layout=no \
  --enable-tests=no \
  --enable-samples=no \
 --enable-shared=yes \
 --enable-static=no
make clean
make
sudo make install

echo 2. Build and install Boost
cd ~
rm -f boost_1_46_1.tar.gz
#wget http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.tar.gz
curl -L -O http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.tar.gz | tee boost_1_46_1.tar.gz | tar xz
cd boost_1_46_1

#bjam should say "has_icu builds: yes" otherwise check your icu installation above
./bootstrap.sh
sudo ./bjam --with-date_time --with-filesystem --with-regex --with-system --with-thread install link=shared -a


echo 3. Build and install Exodus

export PATH=/Library/PostgreSQL/9.0/bin/:$PATH

#see XCODE's Cross-Development Programming Guide:Configuring a Makefile-Based Project
#export MACOSX_DEPLOYMENT_TARGET=10.4

cd ~
svn co HTTPS://exodusdb.googlecode.com/svn/trunk/ exodus
cd ~/exodus
#./configure --with-sysroot=/Developer/SDKs/MacOSX10.4.0.sdk
./configure
make clean #in case half-built with wrong boost libs
make
sudo make install

