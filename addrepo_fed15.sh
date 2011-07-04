#!/bin/bash
set -ex
sudo su -c 'echo "
[home_neosys]
name=neosys Home Project Fedora_15
type=rpm-md
baseurl=http://download.opensuse.org/repositories/home:/neosys/Fedora_15/
gpgcheck=1
gpgkey=http://download.opensuse.org/repositories/home:/neosys/Fedora_15/repodata/repomd.xml.key
enabled=1
">/etc/yum.repos.d/openbuildservice-exodus-fed15.repo'
