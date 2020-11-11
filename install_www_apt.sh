#!/bin/bash

SECONDS=0

echo
echo Installing exodus web service
echo ==========================

echo
echo Install apache and php
echo ======================
sudo apt install -y apache2 php php-xml
#sudo git clone https://git.hosts.neosys.com/gits/neosys.net/ /var/www/exodus
sudo a2enmod rewrite
sudo systemctl restart apache2

echo
echo Configure an apache site - standard http
echo ========================================
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

echo
echo Enable the apache site and disable default
echo ==========================================
sudo a2dissite 000-default
sudo a2ensite exodus
sudo systemctl restart apache2

echo
echo Configure data permissions
echo ==========================
cd ~/exodus/service
mkdir -p data
chmod g+rws data
mkdir -p data/exodus
chmod o+rw data/exodus

echo
echo Compile the service
echo ===================
echo ignore compile error on sys/server.cpp and various warnings
cd ~/exodus/service/src
. config
cd sys
../compall

echo
echo Finished - Exodus is listening on port 80
echo =========================================
duration=$SECONDS
echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed."
