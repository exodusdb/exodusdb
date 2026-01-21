#!/bin/bash
set -euxo pipefail
PS4='+ [service ${SECONDS}s] '
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
: Function to call apt-get repeatedly in case of error
: ----------------------------------------------------
:
function APT_GET {
:
	STARTED=$SECONDS
	while [[ $(($SECONDS-$STARTED)) -lt 600 ]]; do
:
: Retry apt-get. $(($SECONDS-$STARTED)) secs.
: -------------------
:
		# /dev/null to stop error causing random hang until timeout
		# with apt process stuck on tcsetattr call. see gdb -p 9999
		if timeout 120s $* < /dev/null; then
			break
		fi
		sleep 5
	done;
}

:
: Allow cd into subdirs of /root - but no read access of course
: =============================================================
:
: 	Only if installing into /root/
:
	[ ${EXODUS_DIR:0:6} = /root/ ] && chmod o+x /root

:
: Install chromium to convert html to pdf
: =======================================
:
:  Dont wait for it to complete. Install will be tested near the end of this script.
:
#	sudo snap refresh
	sudo snap services chromium 2> /dev/null || sudo snap install chromium --no-wait

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
: Copy logo and ico into images and web root
: ==========================================

	cd $EXODUS_DIR/service
	cp favicon.ico www
	cp exodusm.png www/exodus/images/theme2

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
	APT_GET sudo DEBIAN_FRONTEND=noninteractive apt-get -y install whois postfix bsd-mailx
:
: Configure postfix
: =================
:
	#JE: Postfix config done in install.neosys1 script
	#postconf myhostname=$SITE_NAME
	#postconf relayhost=?
	#postconf inet_interfaces=all|loopback-only?

#:
#: Install html2pdf
#: ================
#:
#: https://wkhtmltopdf.org/downloads.html
#:
#: 20.04/bionic and 18.04/focal are available
#:
#: 22.04/jammy available as at 2022/06/29
#:
#: 24.04/noble is not available as at 2024/06/09 - but 22.04 version compiles and seems to work
#:
#	RELEASE=`lsb_release -cs`
#
##	if [[ $RELEASE == focal ]]; then
#	if [[ `lsb_release -rs` < "22.04" ]]; then
#		VERSION=0.12.6-1
#	else
#		VERSION=0.12.6.1-2
#	fi
#	#if curl --fail -LO https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6-1/wkhtmltox_0.12.6-1.${RELEASE}_amd64.deb; then
#	if ! curl --fail -LO https://github.com/wkhtmltopdf/packaging/releases/download/$VERSION/wkhtmltox_$VERSION.${RELEASE}_amd64.deb; then
#:
#: Fall back to jammy which works on 24.04/noble, at least on the simple html test below.
#:
#		#RELEASE=bionic
#		#VERSION=0.12.6-1
#		RELEASE=jammy
#		VERSION=0.12.6.1-2
#		curl --fail -LO https://github.com/wkhtmltopdf/packaging/releases/download/$VERSION/wkhtmltox_$VERSION.${RELEASE}_amd64.deb
#	fi
#:
#: Install the deb package
#:
#	sudo dpkg -i wkhtmltox_$VERSION.${RELEASE}_amd64.deb || true
#:
#: The package is missing dependencies but those will and must be fixed as follows:
#:
#	sudo apt-get -y --fix-broken install
#:
#: Verify html2pdf works
#:
##	/usr/local/bin/wkhtmltopdf http://google.com google.pdf
#	printf "<html><body>Nothing Special</body></html>\n" > wkhtmltopdf.html
#	/usr/local/bin/wkhtmltopdf --enable-local-file-access wkhtmltopdf.html wkhtmltopdf.pdf
#	rm wkhtmltopdf.html wkhtmltopdf.pdf

:
: Install pdfgrep
: ===============
:
	which pdfgrep || APT_GET sudo DEBIAN_FRONTEND=noninteractive apt-get -y install pdfgrep

:
: Wait for chromium snap install to complete
: ===========================================
:
: Wait up to 5x2 mins for snap installation to complete. Can randomly fail. Just redo.
:
	for x in {1..5}; do
		snap changes | grep chromium || true
		timeout 120 bash -c 'while snap changes | grep chromium | grep -Pqw "Do|Doing"; do sleep 5; done; snap changes | grep chromium | grep -q Done && echo "Chromium installation done" || echo "Chromium installation failed or not found"' || echo "Timed out."
	done
	snap changes | grep chromium || true
:
	which chromium
	snap refresh chromium
:
: Check chromium converts html to pdf
: ===================================
: Ignore various chromium error messages
:
	printf "<html><body>Nothing Special</body></html>\n" > chromium2pdf.html
	chromium --no-sandbox --headless --disable-gpu --print-to-pdf=chromium2pdf.pdf chromium2pdf.html |& grep "dbus|dconf|touch|mkdir" -vP 1>&2
:
: Verify pdf seems ok
:
	pdfgrep "Nothing Special" chromium2pdf.pdf
#	rm chromium2pdf.html chromium2pdf.pdf

:
: Determine local ip number for info
: ==================================
:
	IPNO=`ip -4 address|grep -v 127.0.0.1|grep -P '\d+\.\d+\.\d+\.\d+' -o|head -n1`

:
: ==============================================================
: Finished $0 $* in $(($SECONDS/60)) minutes and $(($SECONDS%60)) seconds.
: ==============================================================
:
:	Apache is now listening on https://$IPNO
:
:	cd ~/exodus/service
:	sudo systemctl stop exo_live@exodus
:	./run exodus
