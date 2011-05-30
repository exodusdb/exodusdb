#!/bin/bash


#install icu
cd ~
svn co http://source.icu-project.org/repos/icu/icu/tags/release-4-8/ icu48
cd icu48/source
./configure
make
sudo make install

# install boost
cd ~
rm -f boost_1_46_1.tar.gz
wget http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.tar.gz
echo untarring boost, be patient, lots of files.
tar xfz boost_1_46_1.tar.gz
cd boost_1_46_1

#bjam should say "has_icu builds: yes" otherwise check your icu installation above
./bootstrap.sh
sudo ./bjam --with-date_time --with-filesystem --with-regex --with-system --with-thread -a install


# install Exodus
export PATH=/Library/PostgreSQL/9.0/bin/:$PATH
cd ~
svn co HTTPS://exodusdb.googlecode.com/svn/trunk/ exodus
cd ~/exodus
./configure
make clean #in case half-built with wrong boost libs
make
sudo make install
