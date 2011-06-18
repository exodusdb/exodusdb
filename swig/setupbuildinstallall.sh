#!/bin/bash
set -e

#----------------------
#--- Do this first  ---
#----------------------
apt-get update
sudo apt-get install swig

#------------
#--- Perl ---
#------------
./make.sh perl
sudo ./install.sh perl
./test.pl

#-------------
#-- Python ---
#-------------
./make.sh python
sudo ./install.sh python
./test.py

#-----------
#--- PHP ---
#-----------
sudo apt-get install php5-dev php5-cli
./make.sh php
sudo ./install.sh php
sudo sed -i".orig" -e "s/enable_dl = Off/enable_dl = On/" /etc/php5/cli/php.ini
php test.php

#------------
#--- Java ---
#------------
sudo apt-get install default-jdk
./make.sh java
sudo ./install.sh java
./testjava.sh

#----------
#--- C# ---
#----------
sudo apt-get install libmono-dev mono-gmcs
./make.sh csharp
sudo ./install.sh csharp
./testc#.sh




