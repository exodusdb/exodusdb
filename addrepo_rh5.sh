#!/bin/bash
set -ex

#adding repository http://download.opensuse.org/repositories/home:/neosys
sudo su -c 'echo "[home_neosys]
name=neosys Home Project \(CentOS_CentOS-5\)
type=rpm-md
baseurl=http://download.opensuse.org/repositories/home:/neosys/CentOS_CentOS-5/
gpgcheck=1
gpgkey=http://download.opensuse.org/repositories/home:/neosys/CentOS_CentOS-5/repodata/repomd.xml.key
enabled=1
">/etc/yum.repos.d/obs-exodus.repo'

