#!/bin/bash
set -ex
sudo su -c 'echo "
[home_neosys]
name=neosys's Home Project (RedHat_RHEL-5)
type=rpm-md
baseurl=http://download.opensuse.org/repositories/home:/neosys/RedHat_RHEL-5/
gpgcheck=1
gpgkey=http://download.opensuse.org/repositories/home:/neosys/RedHat_RHEL-5/repodata/repomd.xml.key
enabled=1
">/etc/yum.repos.d/openbuildservice-exodus.repo'
