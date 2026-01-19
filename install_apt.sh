#!/bin/bash
set -euxo pipefail
:
: -------------------------------------------------------
: Download, build and install the full exodus environment
: -------------------------------------------------------
:
: Includes libexodus, exodus cli, c++ compiler and postgresql database.
:
: You can view the latest version of this script directly using wget or curl
:
: "wget -O - https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | less"
: "curl https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | less"
:
: or run the latest version directly.
:
: "wget -O - https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | bash"
: "curl https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | bash"
:
: Installing on:
:
	uname -a
	lsb_release -a
:
: Tested on:
:
: See install.sh for latest:
:
: "Ubuntu 23.10       - mantic"
: "Ubuntu 23.04       - lunar"
: "Ubuntu 22.04.3 LTS - jammy"
: "Ubuntu 20.04.3 LTS - bionic"
:
: No longer tested - requires C++20
:
: "Ubuntu 18.04.1 LTS"
: "Ubuntu 10.04.2 LTS"
: "Linux lucid64 2.6.32-31-server" #61-Ubuntu SMP Fri Apr 8 19:44:42 UTC 2011 x86_64 GNU/Linux
: "Debian GNU/Linux 6.0 \n \l"
: "Linux debian32 2.6.32-5-686" #1 SMP Wed May 18 07:08:50 UTC 2011 i686 GNU/Linux
:
: Parse command line
: ------------------
:
	EXODUS_DIR=${1:-~/exodus}
	EXODUS_BRANCH_OR_TAG=${2:-master}
	COMPILER=${3:-clang-20}
:
: Update apt and install git
: --------------------------
:
	sudo apt-get update
	sudo DEBIAN_FRONTEND=noninteractive apt-get -y install git
:
: Git clone exodus or refresh if already present?
: -----------------------------------------------
:
	if [ ! -d $EXODUS_DIR ]; then
		git clone --recursive --branch $EXODUS_BRANCH_OR_TAG https://github.com/exodusdb/exodusdb $EXODUS_DIR
	else
		cd $EXODUS_DIR
		#git stash
#		git pull || true
		#git stash pop
	fi
:
: Install all
: -----------
:
	cd $EXODUS_DIR
	./install.sh A $COMPILER
:
: Finished $0 $* in $((SECONDS/60)) mins and $((SECONDS%60)) secs.
: ----------------------------------------------------------------
:
