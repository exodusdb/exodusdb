FROM ubuntu
MAINTAINER steve.bush@neosys.com
RUN apt-get update && apt-get -y install subversion g++ postgresql-server-dev* libboost-all-dev build-essential cmake
RUN apt-get -y install unattended-upgrades && echo "APT::Periodic::Update-Package-Lists \"1\";" > /etc/apt/apt.conf.d/20auto-upgrades && echo "APT::Periodic::Unattended-Upgrade \"1\";" >> /etc/apt/apt.conf.d/20auto-upgrades

#RUN wget -O boost_1_55_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz/download && tar xzvf boost_1_55_0.tar.gz && cd boost_1_55_0/
#RUN apt-get install python-dev autotools-dev libicu-dev build-essential libbz2-dev
#./bootstrap.sh --prefix=/usr/local
#RUN n=`cat /proc/cpuinfo | grep "cpu cores" | uniq | awk '{print $NF}'` && ./b2 -j $n  --cppflags=-std=c++11 --with=date_time,filesystem,regex,thread,system install
#RUN sh -c 'echo "/usr/local/lib" >> /etc/ld.so.conf.d/usrlocal.conf' && ldconfig

RUN apt-get -y install openssh-server apache2 php5 libapache2-mod-php5 nano
RUN a=a#cachebuster2
RUN svn co HTTPS://exodusdb.googlecode.com/svn/trunk/ exodus
RUN cd ~/exodus/exodus/libexodus/exodus  && cmake . && make && make install && ldconfig
RUN cd ~/exodus/cli/src && cmake . && make && make install
RUN cd ~/exodus/service/service2 && export CPLUS_INCLUDE_PATH=../sys:../gen:../fin:../agy:../job:../tim:../med:../test && export PATH=$PATH:~/bin && cd sys && compile ../???/*.cpp
RUN mkdir /var/run/sshd
RUN sed -i -e's/without-password/yes/' /etc/ssh/sshd_config
RUN echo 'root:somesillysecret' |chpasswd
#grant access to apache and php to read and write exodus request/response files (g+s) forces all files created to be group www-exodus. add exodus/root to group www-exodus
RUN groupadd www-exodus && usermod -a -G www-exodus www-data && usermod -a -G www-exodus root && chgrp -R www-exodus /exodus/service/data/ && chmod g+sw -R /exodus/service/data/ && chmod g+x /exodus/service/data/ && find /exodus/service/data/ -type d -exec chmod g+rwxs {} +
RUN mv /var/www/html /var/www/html.orig && ln -s /exodus/www /var/www/html
RUN a2enmod ssl rewrite && a2ensite default-ssl
RUN cp /exodus/service/exodus-apache2.conf /etc/apache2/conf-enabled
EXPOSE 22
EXPOSE 80
EXPOSE 443
CMD echo "host=$PGEXODUS_PORT_5432_TCP_ADDR port=$PGEXODUS_PORT_5432_TCP_PORT dbname=exodus user=exodus password=somesillysecret connect_timeout=10" > /root/.exodus && service apache2 start && service ssh start && export EXO_HOST=$PGEXODUS_PORT_5432_TCP_ADDR && export EXO_PORT=$PGEXODUS_PORT_5432_TCP_PORT && cd ~/exodus/service/service2/sys/ && ~/bin/server

RUN apt-get -y install locales && locale-gen en_GB.UTF-8 en_US.UTF-8
#build like this
#sudo docker.io build -t exodus .

#run like this
#assuming pgexodus database is running in container named "pgexodus"
#sudo docker.io run -P --rm --link pgexodus:pgexodus exodus

