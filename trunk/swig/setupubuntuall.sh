#!/bin/bash
set -e

#----------------------
#--- Do this first  ---
#----------------------
sudo apt-get update
sudo apt-get -y install swig

cd ~/exodus/swig

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
sudo apt-get install -y php5-dev
./make.sh php
sudo apt-get install -y php5-cli
sudo ./install.sh php
php test.php

#------------
#--- Java ---
#------------
sudo apt-get install -y default-jdk
./make.sh java
sudo ./install.sh java
./testjava.sh

#----------
#--- C# ---
#----------
sudo apt-get install -y libmono-dev mono-gmcs
./make.sh csharp
sudo ./install.sh csharp
./testc#.sh

