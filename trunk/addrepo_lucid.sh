#!/bin/bash
set -ex

#Add a Launchpad PPA repository hosted by Canonical
sudo sh -c 'echo "deb http://ppa.launchpad.net/steve-bush/ppa-exodus/ubuntu lucid main\n \
 deb-src http://ppa.launchpad.net/steve-bush/ppa-exodus/ubuntu lucid main" \
 > /etc/apt/sources.list.d/exodus-ppa-lucid.list'

#Trust GNUPG Key E7815451 https://launchpad.net/~steve-bush/+archive/ppa-exodus
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E7815451

sudo apt-get update

