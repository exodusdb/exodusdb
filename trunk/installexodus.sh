#!/bin/bash

#Download, build and install Exodus and its core dependencies on any UNIX-like platform
# ICU:4.8  Boost:1.46.1  Exodus:Latest

echo ----------------
echo 0. Prerequisites
echo ----------------
#Redhat/Centos
# yum -y remove boost boost-devel
# yum -y install gcc-c++ postgresql-server postgresql-devel curl subversion
#Ubuntu/Debian:
# sudo apt-get -y remove libboost*
# sudo apt-get -y install g++ postgresql-server postgresql-server-dev* curl subversion
#OSX:
# install xcode from osx install dvd
# install postgres from postgresql.org
export EXO_OSX_POSTGRES=/Library/PostgreSQL/9,0/

set -e

echo ----------------------------------
echo 1. Download, build and install ICU
echo ----------------------------------
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
make && sudo make install

echo ------------------------------------
echo 2. Download, build and install Boost
echo ------------------------------------
cd ~
rm -f boost_1_46_1.tar.gz
#wget http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.tar.gz
curl -L -O http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.tar.gz | tee boost_1_46_1.tar.gz | tar xz
cd boost_1_46_1

echo bjam should say "has_icu builds: yes" otherwise check your icu installation above
./bootstrap.sh
sudo ./bjam --with-date_time --with-filesystem --with-regex --with-system --with-thread install link=shared -a


echo -------------------------------------
echo 3. Download, build and install Exodus
echo -------------------------------------
export PATH=EXO_OSX_POSTGRES:$PATH
cd ~
svn co HTTPS://exodusdb.googlecode.com/svn/trunk/ exodus
cd ~/exodus
./configure
make clean #in case half-built with wrong boost libs
make && sudo make install
