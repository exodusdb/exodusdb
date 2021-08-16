#!/bin/bash
set -eux

: =============================
: Installing exodus web service
: =============================
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
	cd $EXODUS/service
	./create_site exodus

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
: Copy all $EXODUS/bin,lib to ~/live
: ==================================
:
	cd $EXODUS/service
	./copyall

:
: Setup database dictionaries
: ===========================
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
	cat $EXODUS/service/src/sql/*.sql | sudo -u postgres psql exodus

:
: Configure the exodus service
: ============================
:
	cd $EXODUS/service
	./create_service exo exodus '' live

:
: Install required packages
: =========================
:
:	whois is used in unknown ip no login notification emails
:	bsd-mailx provides "mail" which is required to send email by neomail
:	mailutils is an alternative package that doesnt have identical options
:
	apt-get -y install whois bsd-mailx

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
	/usr/local/bin/wkhtmltopdf http://google.com google.pdf

:
: Finished install_all in $(($SECONDS/60)) minutes and $(($SECONDS%60)) seconds.
: ==============================================================
:
:	Apache should now be listening on port 80 and 443
