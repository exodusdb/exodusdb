#!/bin/bash
set -euxo pipefail

: ===========================
: Installs EXODUS web service
: ===========================
:
: 'Syntax is ./install_all.sh [SITE_NAME|exodus|none] [DOMAIN_PREFIX]'
:
: 'Example:  ./install_all.sh'
:
: 'Example:  ./install_all.sh exodus t-'
:
: 'SITE_NAME defaults to "exodus"'
: 'DOMAIN_PREFIX defaults to ""'
:
	SITE_NAME=${1:-exodus}
	DOMAIN_PREFIX=${2:-}

:
: Config
: ======
:
:	EXODUS_DIR provided by caller or default to the parent dir of the current working dir
:
#	EXODUS_DIR=~/exodus
	EXODUS_DIR=${EXODUS_DIR:-$(realpath `pwd`/..)}

:
: Allow cd into subdirs of /root - but no read access of course
: =============================================================
:
: 	Only if installing into /root/
:
	[ ${EXODUS_DIR:0:6} = /root/ ] && chmod o+x /root

:
: Install apache with php and configure a site
: ============================================
:
	if [ $SITE_NAME != none ]; then
		cd $EXODUS_DIR/service
		./create_site $SITE_NAME '' '' $DOMAIN_PREFIX
	fi

:
: Disable default web sites
: =========================
:
	sudo a2dissite 000-default default-ssl.conf || true

:
: Compile the service
: ===================
:
	cd $EXODUS_DIR/service/src
	./compall

:
: Copy all $EXODUS_DIR/bin,lib,dat to ~/live
: ======================================
:
	cd $EXODUS_DIR/service
	./copyall CONFIRM

:
: Create exodus_live db for live dictionaries if not already present
: ==================================================================
:
	dblist|grep exodus_live > /dev/null || dbcreate exodus_live

:
: Import dat files into exodus and exodus_live
: ===========================================
:
	cd /tmp
	#sudo -u postgres psql exodus < $EXODUS_DIR/service/src/sql/dict_voc.sql
	#sudo -u postgres psql exodus < $EXODUS_DIR/service/src/sql/dict_users.sql
	#sudo -u postgres psql exodus < $EXODUS_DIR/service/src/sql/dict_processes.sql
	#sudo -u postgres psql exodus < $EXODUS_DIR/service/src/sql/dict_locks.sql
	#sudo -u postgres psql exodus < $EXODUS_DIR/service/src/sql/dict_requestlog.sql
	#sudo -u postgres psql exodus < $EXODUS_DIR/service/src/sql/dict_statistics.sql
	#sudo -u postgres psql exodus < $EXODUS_DIR/service/src/sql/dict_changelog.sql
	#sudo -u postgres psql exodus < $EXODUS_DIR/service/src/sql/dict_definitions.sql
	#cat $EXODUS_DIR/service/src/sql/*.sql | sudo -u postgres psql exodus
	#cat $EXODUS_DIR/service/src/sql/*.sql | sudo -u postgres psql exodus_live
	#EXO_DATA=exodus EXO_DICT=exodus sync_dat
	EXO_DATA=exodus EXO_DICT=exodus syncdat
	#EXO_DATA=exodus EXO_DICT=exodus_live sync_dat
	EXO_DATA=exodus EXO_DICT=exodus_live syncdat

:
: Configure the exodus service
: ============================
:
	if [ $SITE_NAME != none ]; then
		cd $EXODUS_DIR/service
		./create_service exo $SITE_NAME '' live
	fi

:
: Start the service
: =================
:
	if [ $SITE_NAME != none ]; then
		cd $EXODUS_DIR/service
		./service $SITE_NAME start live
	fi

:
: Install required packages
: =========================
:
:	whois		used in unknown ip no login notification emails
:	bsd-mailx	provides "mail" which is required to send email?
:	postfix     email handler
:	mailutils	NOT installed. like bsd-mailx but doesnt have identical options
:	qrencode	used in htmllib2 for KSA invoices
:
	DEBIAN_FRONTEND=noninteractive sudo apt-get -y install whois postfix bsd-mailx

:
: Configure postfix
: =================
:
	#JE: Postfix config done in install.neosys1 script
	#postconf myhostname=$SITE_NAME
	#postconf relayhost=?
	#postconf inet_interfaces=all|loopback-only?

:
: Install html2pdf
: ================
:
: https://wkhtmltopdf.org/downloads.html
:
: 20.04/bionic and 18.04/focal are available
:
: 22.04/jammy available as at 2022/06/29
:
: 24.04/noble is not available as at 2024/06/09 - but 22.04 version compiles and seems to work
:
	RELEASE=`lsb_release -cs`

#	if [[ $RELEASE == focal ]]; then
	if [[ `lsb_release -rs` < "22.04" ]]; then
		VERSION=0.12.6-1
	else
		VERSION=0.12.6.1-2
	fi
	#if curl --fail -LO https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6-1/wkhtmltox_0.12.6-1.${RELEASE}_amd64.deb; then
	if ! curl --fail -LO https://github.com/wkhtmltopdf/packaging/releases/download/$VERSION/wkhtmltox_$VERSION.${RELEASE}_amd64.deb; then
:
: Fall back to jammy which works on 23.04/lunar, at least on the simple html test below.
:
		#RELEASE=bionic
		#VERSION=0.12.6-1
		RELEASE=jammy
		VERSION=0.12.6.1-2
		curl --fail -LO https://github.com/wkhtmltopdf/packaging/releases/download/$VERSION/wkhtmltox_$VERSION.${RELEASE}_amd64.deb
	fi
:
: Install the deb package
:
	sudo dpkg -i wkhtmltox_$VERSION.${RELEASE}_amd64.deb || true
:
: The package is missing dependencies but those will and must be fixed as follows:
:
	sudo apt -y --fix-broken install
:
: Verify html2pdf works
:
#	/usr/local/bin/wkhtmltopdf http://google.com google.pdf
	printf "<html><body>Nothing Special</body></html>\n" > wkhtmltopdf.html
	/usr/local/bin/wkhtmltopdf --enable-local-file-access wkhtmltopdf.html wkhtmltopdf.pdf
	rm wkhtmltopdf.html wkhtmltopdf.pdf

:
: Determine local ip number for info
: ==================================
:
	IPNO=`ip -4 address|grep -v 127.0.0.1|grep -P '\d+\.\d+\.\d+\.\d+' -o|head -n1`

:
: Copy logo and ico into images and web root
: ==========================================

	cd $EXODUS_DIR/service
	cp favicon.ico www
	cp exodusm.png www/exodus/images/theme2

:
: Finished install_all in $(($SECONDS/60)) minutes and $(($SECONDS%60)) seconds.
: ==============================================================
:
:	Apache is now listening on https://$IPNO
:
:	cd ~/exodus/service
:	sudo systemctl stop exo_live@exodus
:	./run exodus
