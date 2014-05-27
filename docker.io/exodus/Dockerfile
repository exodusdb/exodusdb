FROM ubuntu
MAINTAINER steve.bush@neosys.com
RUN apt-get update && apt-get -y install subversion g++ postgresql-server-dev* libboost-all-dev build-essential cmake
RUN a=a#cachebuster
RUN svn co HTTPS://exodusdb.googlecode.com/svn/trunk/ exodus
RUN cd ~/exodus/exodus/libexodus/exodus  && cmake . && make && make install && ldconfig
RUN cd ~/exodus/cli/src && cmake . && make && make install
RUN cd ~/exodus/service/service2 && export CPLUS_INCLUDE_PATH=../sys:../gen:../fin:../agy:../job:../tim:../med:../test && export PATH=$PATH:~/bin && cd sys && compile ../???/*.cpp
RUN apt-get -y install openssh-server apache2 php5 libapache2-mod-php5 nano
RUN mkdir /var/run/sshd
RUN sed -i -e's/without-password/yes/' /etc/ssh/sshd_config
RUN echo 'root:somesillysecret' |chpasswd
#grant access to apache and php to read and write exodus request/response files (g+s) forces all files created to be group www-exodus. add exodus/root to group www-exodus
RUN groupadd www-exodus && usermod -a -G www-exodus www-data && usermod -a -G www-exodus root && chgrp -R www-exodus /exodus/service/data/ && chmod g+s -R /exodus/service/data/
RUN mv /var/www/html /var/www/html.orig && ln -s /exodus/www /var/www/html
RUN a2enmod ssl rewrite && a2ensite default-ssl
RUN apt-get -y install unattended-upgrades && echo "APT::Periodic::Update-Package-Lists \"1\";" > /etc/apt/apt.conf.d/20auto-upgrades && echo "APT::Periodic::Unattended-Upgrade \"1\";" >> /etc/apt/apt.conf.d/20auto-upgrades
RUN cp /exodus/service/exodus-apache2.conf /exodus-apache2.conf /etc/apache2/conf-enabled
EXPOSE 22
EXPOSE 80
EXPOSE 443
CMD service apache2 start && service ssh start && export EXO_HOST=$PGEXODUS_PORT_5432_TCP_ADDR && export EXO_PORT=$PGEXODUS_PORT_5432_TCP_PORT && cd ~/exodus/service/service2/sys/ && ~/bin/server

#build like this
#sudo docker.io build -t exodus .

#run like this
#assuming pgexodus database is running in container "pgexodus"
#sudo docker.io run -P --rm --link pgexodus:pgexodus exodus

