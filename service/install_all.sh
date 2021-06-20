#!/bin/bash
set -eux

: =============================
: Installing exodus web service
: =============================
:
	EXODUS=~/exodus
	test -f $EXODUS/service/install_all.sh || ( echo Must be run in ~/exodus/service install dir without path && exit )

:
: If installing into /root/then allow cd into /root - but no read access of course
: ================================================================================
:
	[ ${EXODUS:0:6} = /root/ ] && chmod o+x /root

:
: Install apache and php
: ======================
:

	./create_site exo exodus

:
: Disable default web sites
: =========================
:
	sudo a2dissite 000-default default-ssl.conf

:
: Compile the service
: ===================
:
: ignore compile error on sys/server.cpp and various warnings
	cd $EXODUS/service/src
	#. config
	./compall

:
: Setup database dict_users
: =========================
:
	cd /tmp
	sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_voc.sql
	sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_users.sql
	sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_processes.sql
	sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_locks.sql
	sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_requestlog.sql
	sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_statistics.sql
	sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_changelog.sql
	sudo -u postgres psql exodus < $EXODUS/service/src/sql/dict_definitions.sql

:
: Configure the exodus service
: ============================
:
	cd $EXODUS/service
	./create_service exo exodus '' live

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
: Test it works
:
	/usr/local/bin/wkhtmltopdf http://google.com google.pdf

:
: Finished install_all in $(($SECONDS/60)) minutes and $(($SECONDS%60)) seconds.
: ==============================================================
:
:	Apache should be listening on port 80 and 443
