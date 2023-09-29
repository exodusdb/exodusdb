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
<<<<<<< Updated upstream
	cat /etc/issue
=======
	lsb_release -a
>>>>>>> Stashed changes
:
: Tested on:
:
: "Ubuntu 22.04 Ubuntu Jammy Jellyfish (development branch)"
: "Ubuntu 20.04.3 LTS"
:
: No longer tested - requires C++20
:
: "Ubuntu 18.04.1 LTS"
: "Ubuntu 10.04.2 LTS"
: "Linux lucid64 2.6.32-31-server" #61-Ubuntu SMP Fri Apr 8 19:44:42 UTC 2011 x86_64 GNU/Linux
: "Debian GNU/Linux 6.0 \n \l"
: "Linux debian32 2.6.32-5-686" #1 SMP Wed May 18 07:08:50 UTC 2011 i686 GNU/Linux
<<<<<<< Updated upstream
:
:
: 1. Building and Installing Exodus
: =================================
:
: Update repos in case building on a pristine installation
:
	sudo apt-get update
=======
:
: Config
: ------
:
	EXODUSDIR=${1:-${EXODUSDIR:-~/exodus}}
:
: Update apt and install git
: --------------------------
:
	sudo apt-get update
 	sudo DEBIAN_FRONTEND=noninteractive apt-get -y install git
>>>>>>> Stashed changes
:
: Git clone exodus or refresh if already present
: ----------------------------------------------
:
<<<<<<< Updated upstream
 	sudo DEBIAN_FRONTEND=noninteractive \
		apt-get -y install \
		git \
		cmake \
		postgresql-server-dev-all \
		g++ build-essential \
		libboost-dev libboost-system-dev libboost-regex-dev libboost-thread-dev libboost-locale-dev libboost-chrono-dev libboost-date-time-dev libboost-atomic-dev
:
: Download exodus if not already present
:
	cd ~
	[ ! -d exodus ] && git clone https://github.com/exodusdb/exodusdb exodus
:
: Refresh git in case reinstalling
:
	cd ~/exodus
	git stash
	git pull
:
: Config make with all cpus in parallel
:
	cd ~/exodus
	cmake .
:
: Clean all - unless suppressed for speed. Required in case half-built with wrong libs somehow.
:
	[ ${NOCLEAN:-} ] || make clean
	cmake .

:
: Make all exodus lib, cli and pgexodus
:
	make -j `nproc`
:
: Install all exodus lib and cli
:
	sudo make install

:
: 2. Installing Postgres and Configuring it for Exodus
: ====================================================
:
: Install the postgresql client package
:
        #yum -y install postgresql-server
        sudo DEBIAN_FRONTEND=noninteractive \
        apt-get -y install postgresql postgresql-client
:
: Restart postgres
:
        #/etc/init.d/postgresql reload
        #sudo /etc/init.d/postgresql reload
        sudo /etc/init.d/postgresql restart

:
: make install again to install pgexodus extension and functions, and exodus user and database
:
        cd ~/exodus
        make install

:
: 3. Configuring Exodus for Postgres
: ==================================
:
	mkdir -p ~/.config/exodus
:
	echo host=127.0.0.1 \
	port=5432 \
	dbname=exodus \
	user=exodus \
	password=somesillysecret \
	> ~/.config/exodus/exodus.cfg
:
: 4. Add some postgres utility functions
: ======================================
:
: one of the exodus cli programs
:
	dict2sql
:
: 5. Testing Exodus
: =================
: Every database requires a lists file in order to store select lists
:
	createfile lists || true
	cd ~
	testsort
:
: 6. Programming with Exodus
: ==========================
: you must make some change to hello or actually save it, not just quit
: edic hello
: hello
: compile hello
:
: 7. Finished $0 $* in $((SECONDS / 60)) minutes and $((SECONDS % 60)) seconds
: ======================================================================
=======
	if [ ! -d $EXODUS_DIR ]; then
		git clone https://github.com/exodusdb/exodusdb $EXODUS_DIR
	else
		cd $EXODUSDIR
		#git stash
		git pull || true
		#git stash pop
	fi
:
: Install all
: -----------
:
	cd $EXODUSDIR
	./install.sh $*
:
: Finished $0 $* in $((SECONDS/60)) mins and $((SECONDS%60)) secs.
: ----------------------------------------------------------------
>>>>>>> Stashed changes
