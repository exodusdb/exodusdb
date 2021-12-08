#!/bin/bash
set -eux

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
	EXODUS=~/exodus

:
: Allow cd into /root - but no read access of course
: ==================================================
:
: 	Only if installing into /root/
:
	[ ${EXODUS:0:6} = /root/ ] && chmod o+x /root

:
: Install apache with php and configure a site
: ============================================
:
	if [ $SITE_NAME != none ]; then
		cd $EXODUS/service
		./create_site $SITE_NAME '' '' $DOMAIN_PREFIX
	fi

:
: Disable default web sites
: =========================
:
	sudo a2dissite 000-default default-ssl.conf

:
: Compile the service
: ===================
:
	cd $EXODUS/service/src
	./compall

:
: Copy all $EXODUS/bin,lib,dat to ~/live
: ======================================
:
	cd $EXODUS/service
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
	#sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_voc.sql
	#sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_users.sql
	#sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_processes.sql
	#sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_locks.sql
	#sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_requestlog.sql
	#sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_statistics.sql
	#sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_changelog.sql
	#sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_definitions.sql
	#cat $EXODUS/service/src/sql/*.sql | sudo -u postgres psql exodus
	#cat $EXODUS/service/src/sql/*.sql | sudo -u postgres psql exodus_live
	EXO_DATA=exodus EXO_DICT=exodus sync_dat
	EXO_DATA=exodus_live EXO_DICT=exodus_live sync_dat

:
: Configure the exodus service
: ============================
:
	if [ $SITE_NAME != none ]; then
		cd $EXODUS/service
		./create_service exo $SITE_NAME '' live
	fi

:
: Start the service
: =================
:
	if [ $SITE_NAME != none ]; then
		cd $EXODUS/service
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
:
	DEBIAN_FRONTEND=noninteractive apt-get -y install whois postfix bsd-mailx

:
: Configure postfix
: =================
:
	postconf myhostname=$SITE_NAME
	#postconf relayhost=?
	#postconf inet_interfaces=all|loopback-only?

:
: Install html2pdf
: ================
:
: https://wkhtmltopdf.org/downloads.html
:
: 20.04 bionic or focal
:
	RELEASE=`lsb_release -cs`
	curl -LO https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6-1/wkhtmltox_0.12.6-1.${RELEASE}_amd64.deb
	dpkg -i wkhtmltox_0.12.6-1.${RELEASE}_amd64.deb || true
	apt -y --fix-broken install
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

	cd $EXODUS/service
	cp favicon.ico www
	cp exodusm.png www/exodus/images/theme2

:
: Finished install_all in $(($SECONDS/60)) minutes and $(($SECONDS%60)) seconds.
: ==============================================================
:
:	Apache should now be listening on ipno $IPNO port 443
