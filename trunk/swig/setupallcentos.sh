#!/bin/bash
set -e

#-------------------------------
#--- MUST Install Swig First ---
#-------------------------------
yum -y install pcre-devel
wget http://downloads.sourceforge.net/project/swig/swig/swig-2.0.4/swig-2.0.4.tar.gz
tar xf swig-2.0.4.tar.gz
cd swig-2.0.4
./configure --prefix=$HOME/local
make && make install

cd ~/exodus/swig

#------------
#--- Perl ---
#------------
./make.sh perl && sudo ./install.sh perl && ./test.pl

#-------------
#-- Python ---
#-------------
yum install -y python-devel
./make.sh python && sudo ./install.sh python && ./test.py

#-----------
#--- PHP ---
#-----------
yum install -y php-devel php-cli
./make.sh php && sudo ./install.sh php && ./test.php
service httpd restart || echo "service httpd restart" failed

#------------
#--- Java ---
#------------
yum install -y java java-devel
./make.sh java && sudo ./install.sh java && ./testjava.sh

#----------
#--- C# ---
#----------
yum install -y mono-devel
./make.sh csharp && sudo ./install.sh csharp && ./testc#.sh
