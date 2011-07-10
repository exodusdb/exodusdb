#!/bin/bash
set -ex

#Trust GNUPG Key E7815451 https://launchpad.net/~steve-bush/+archive/ppa-exodus
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E7815451

#Add a Launchpad PPA repository hosted by Canonical for the above key
sudo sh -c 'echo "deb http://ppa.launchpad.net/steve-bush/ppa-exodus/ubuntu lucid main\n \
 deb-src http://ppa.launchpad.net/steve-bush/ppa-exodus/ubuntu lucid main" \
 > /etc/apt/sources.list.d/exodus.list'

sudo apt-get update

