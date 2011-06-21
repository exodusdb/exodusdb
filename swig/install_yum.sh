#!/bin/bash
set -e

#-------------------------------
#--- MUST Install Swig First ---
#-------------------------------
sudo yum install -y swig
if [ "`swig -version | grep 1.3`" != "" ]; then
 sudo yum -y install pcre-devel
 cd ~
 export SWIG_VER=2.0.4
 wget http://downloads.sourceforge.net/project/swig/swig/swig-$SWIG_VER/swig-$SWIG_VER.tar.gz
 tar xf swig-$SWIG_VER.tar.gz
 cd swig-$SWIG_VER
 ./configure --prefix=$HOME/local
 make && sudo make install
fi

cd ~/exodus/swig

#------------
#--- Perl ---
#------------
sudo yum -y install perl-devel
./make.sh perl && sudo ./install.sh perl && ./test.pl

#-------------
#-- Python ---
#-------------
sudo yum install -y python-devel
./make.sh python && sudo ./install.sh python && ./test.py

#-----------
#--- PHP ---
#-----------
sudo yum install -y php-devel php-cli
./make.sh php && sudo ./install.sh php && ./test.php
service httpd restart || echo "service httpd restart" failed

#------------
#--- Java ---
#------------
sudo yum install -y java java-devel
./make.sh java && sudo ./install.sh java && ./testjava.sh

#----------
#--- C# ---
#----------
sudo yum install -y mono-devel
./make.sh csharp && sudo ./install.sh csharp && ./testc#.sh
