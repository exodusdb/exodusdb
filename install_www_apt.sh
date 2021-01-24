#!/bin/bash
set -ex

:
: Installing exodus web service
: =============================
:
	EXODUS=`pwd`
	test -f $EXODUS/install_www_apt.sh || ( echo Must be run in exodus install dir without path && exit )
:
: If installing into /root/then allow cd into /root - but no read access of course
:
	[ ${EXODUS:0:6} = /root/ ] && chmod o+x /root

:
: Install apache and php
: ======================
:
	sudo apt install -y apache2 php php-xml
	sudo a2enmod rewrite ssl
	sudo /etc/init.d/apache2 restart

:
: Configure an apache site - HTTP
: ===============================
:
	cat > /tmp/exodus.conf <<V0G0N
<VirtualHost *:80>
        ServerAdmin webmaster@localhost

        DocumentRoot /home/ubuntu/exodus/service/www

        ErrorLog ${APACHE_LOG_DIR}/error.log
        CustomLog ${APACHE_LOG_DIR}/access.log combined

        <Directory /home/ubuntu/exodus/service/www>
            Options Indexes FollowSymLinks
            AllowOverride All
            Require all granted
            DirectoryIndex default.htm
        </Directory>

</VirtualHost>
V0G0N
	sed -i "s|/home/ubuntu/exodus/|$EXODUS/|g" /tmp/exodus.conf
	sudo mv /tmp/exodus.conf /etc/apache2/sites-available/

:
: Configure an apache site - HTTPS
: ================================
:
: Use letsencrypt hosts.neosys.com cert if available
:
	sudo test -f /etc/letsencrypt/live/hosts.neosys.com/cert.pem      && sudo ln -sf /etc/letsencrypt/live/hosts.neosys.com/cert.pem      /etc/ssl/certs/exodus-cert.pem
	sudo test -f /etc/letsencrypt/live/hosts.neosys.com/privkey.pem   && sudo ln -sf /etc/letsencrypt/live/hosts.neosys.com/privkey.pem   /etc/ssl/private/exodus-key.pem
:
: Otherwise use self signed certificates if exodus cert and key not already present
:
	sudo test -f /etc/ssl/certs/exodus-cert.pem  || sudo cp /etc/ssl/certs/ssl-cert-snakeoil.pem   /etc/ssl/certs/exodus-cert.pem
	sudo test -f /etc/ssl/private/exodus-key.pem || sudo cp /etc/ssl/private/ssl-cert-snakeoil.key /etc/ssl/private/exodus-key.pem
:
	cat > /tmp/exodus-ssl.conf <<V0G0N
<VirtualHost *:443>
    ServerAdmin webmaster@localhost

    DocumentRoot /home/ubuntu/exodus/service/www

    ErrorLog ${APACHE_LOG_DIR}/error.log
    CustomLog ${APACHE_LOG_DIR}/access.log combined

    SSLEngine on

    #SSLCertificateFile /etc/letsencrypt/live/hosts.neosys.com/cert.pem
    #SSLCertificateKeyFile /etc/letsencrypt/live/hosts.neosys.com/privkey.pem
    #SSLCACertificateFile /etc/letsencrypt/live/hosts.neosys.com/fullchain.pem
    SSLCertificateFile /etc/ssl/certs/exodus-cert.pem
    SSLCertificateKeyFile /etc/ssl/private/exodus-key.pem

    <Directory /home/ubuntu/exodus/service/www>
        Options Indexes FollowSymLinks
        AllowOverride All
        Require all granted
        DirectoryIndex default.htm
    </Directory>

</VirtualHost>
V0G0N
	sed -i "s|/home/ubuntu/exodus/|$EXODUS/|g" /tmp/exodus-ssl.conf
	sudo mv /tmp/exodus-ssl.conf /etc/apache2/sites-available/

:
: Enable the apache sites and disable defaults
: ============================================
:
	sudo a2dissite 000-default default-ssl.conf
	sudo a2ensite exodus exodus-ssl
	#sudo systemctl restart apache2
	sudo /etc/init.d/apache2 restart

:
: Configure data permissions
: ==========================
:
	cd $EXODUS/service
	mkdir -p work # server will run here
	mkdir -p data
	chmod g+rws data
	mkdir -p data/exodus
	chmod o+rw data/exodus
	mkdir -p images
	chmod g+rws images

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
	cat > /tmp/exodus.service <<V0G0N
[Unit]
Description=Exodus Service
After=network.target
StartLimitBurst=3
StartLimitIntervalSec=30
[Service]
#
[Service]
Type=simple
Restart=on-failure
RestartSec=3
User=%USER
WorkingDirectory=/home/ubuntu/exodus/service/work
Environment="PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:%HOME/bin"
Environment="EXODUS_STARTED_BY_SYSTEMD=true"
ExecStart=/home/ubuntu/bin/server
#
[Install]
WantedBy=multi-user.target
V0G0N
	sed -i "s|/home/ubuntu/exodus/|$EXODUS/|g" /tmp/exodus.service
	sed -i "s|/home/ubuntu/bin/|$HOME/bin/|g" /tmp/exodus.service
	sed -i "s|%USER|$USER|g" /tmp/exodus.service
	sed -i "s|%HOME|$HOME|g" /tmp/exodus.service
	sudo mv /tmp/exodus.service /etc/systemd/system/
	sudo systemctl daemon-reload

:
: Configure the exodus service to autostart, and start it
: =======================================================
:
	sudo systemctl enable exodus
	sudo systemctl restart exodus
	sleep 2

:
: Install html2pdf
: ================
:
: https://wkhtmltopdf.org/downloads.html
:
: 20.04 focal
:
	curl -LO https://github.com/wkhtmltopdf/packaging/releases/download/0.12.6-1/wkhtmltox_0.12.6-1.focal_amd64.deb
	dpkg -i wkhtmltox_0.12.6-1.focal_amd64.deb || true
	apt --fix-broken install
:
: Test it works
:
	/usr/local/bin/wkhtmltopdf http://google.com google.pdf

:
: Finished Setup - Apache should be listening on port 80 and 443
: ==============================================================
:
: "$(($SECONDS/60)) minutes and $(($SECONDS%60)) seconds elapsed."

:
: Managing the service
: ====================
:
: sudo systemctl {start/stop/restart/status} exodus
:  or
: cd $EXODUS/service/src
: . config
: cd $EXODUS/work
: server
:
: tail /var/log/syslog -f
:
	systemctl status exodus
