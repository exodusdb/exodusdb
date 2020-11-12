#!/bin/bash
set -x
:
: Installing exodus web service
: ==========================
SECONDS=0
:
: Install apache and php
: ======================
sudo apt install -y apache2 php php-xml
sudo a2enmod rewrite ssl
sudo systemctl restart apache2
:
: Configure an apache site - HTTP
: ===============================
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
sudo mv /tmp/exodus.conf /etc/apache2/sites-available/
:
: Configure an apache site - HTTPS
: ================================
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
    SSLCertificateFile /etc/ssl/certs/ssl-cert-snakeoil.pem
    SSLCertificateKeyFile /etc/ssl/private/ssl-cert-snakeoil.key

    <Directory /home/ubuntu/exodus/service/www>
        Options Indexes FollowSymLinks
        AllowOverride All
        Require all granted
        DirectoryIndex default.htm
    </Directory>

</VirtualHost>
V0G0N
sudo mv /tmp/exodus-ssl.conf /etc/apache2/sites-available/
:
: Enable the apache sites and disable defaults
: ============================================
sudo a2dissite 000-default default-ssl.conf
sudo a2ensite exodus exodus-ssl
sudo systemctl restart apache2
:
: Configure data permissions
: ==========================
cd ~/exodus/service
mkdir -p data
chmod g+rws data
mkdir -p data/exodus
chmod o+rw data/exodus
:
: Compile the service
: ===================
: ignore compile error on sys/server.cpp and various warnings
cd ~/exodus/service/src
. config
cd sys
../compall
:
: Setup database dict_users
: =========================
cd ~/exodus/service/src
sudo -u postgres psql exodus < sql/dict_users.sql
sudo -u postgres psql exodus < sql/dict_processes.sql
sudo -u postgres psql exodus -c "ALTER TABLE dict_users OWNER to exodus"
sudo -u postgres psql exodus -c "ALTER TABLE dict_processes OWNER to exodus"
:
: Finished - Exodus should be listening on port 80 and 443
: ========================================================
duration=$SECONDS
: "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed."
