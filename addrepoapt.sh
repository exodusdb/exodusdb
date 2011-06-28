#!/bin/bash
sudo sh -c 'echo "deb http://ppa.launchpad.net/steve-bush/ppa-exodus/ubuntu lucid main \n \
                  deb-src http://ppa.launchpad.net/steve-bush/ppa-exodus/ubuntu lucid main" \
                  > /etc/apt/sources.list.d/exodus-ppa-lucid.list'
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E7815451

