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
:sudo git clone https://git.hosts.neosys.com/gits/neosys.net/ /var/www/exodus
sudo a2enmod rewrite
sudo systemctl restart apache2
:
: Configure an apache site - standard http
: ========================================
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
: Enable the apache site and disable default
: ==========================================
sudo a2dissite 000-default
sudo a2ensite exodus
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
: Finished - Exodus is listening on port 80
: =========================================
duration=$SECONDS
: "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed."
