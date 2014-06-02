FROM ubuntu
MAINTAINER steve.bush@neosys.com
RUN apt-get update && apt-get -y install subversion g++ postgresql-server-dev* build-essential cmake openssh-server apache2 php5 libapache2-mod-php5 nano locales python-dev autotools-dev libicu-dev build-essential libbz2-dev
RUN apt-get -y install unattended-upgrades && echo "APT::Periodic::Update-Package-Lists \"1\";" > /etc/apt/apt.conf.d/20auto-upgrades && echo "APT::Periodic::Unattended-Upgrade \"1\";" >> /etc/apt/apt.conf.d/20auto-upgrades
RUN locale-gen en_GB.UTF-8 en_US.UTF-8

# build boost with c++11 and install in /usr/local

#RUN apt-get -y install libboost-all-dev

RUN wget -O boost_1_55_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz/download && tar xzvf boost_1_55_0.tar.gz
RUN cd boost_1_55_0/ && ./bootstrap.sh --prefix=/usr/local
RUN cd boost_1_55_0/ && n=`cat /proc/cpuinfo | grep "cpu cores" | uniq | awk '{print $NF}'` && ./b2 -j $n cxxflags=-std=c++11 --with-filesystem --with-regex --with-system --with-thread install
RUN sh -c 'echo "/usr/local/lib" >> /etc/ld.so.conf.d/usrlocal.conf' && ldconfig

# build exodus install in /usr/local (c++11 configured in cmake config)

RUN a=b#cachebuster
RUN svn co HTTPS://exodusdb.googlecode.com/svn/trunk/ exodus
RUN mkdir ~/exodus/exodus/libexodus/exodus/dockerbuild && cd ~/exodus/exodus/libexodus/exodus/dockerbuild  && cmake .. && make && make install && ldconfig
RUN mkdir ~/exodus/cli/src/dockerbuild && cd ~/exodus/cli/src/dockerbuild && cmake .. && make && make install
#compile the service programs
RUN cd ~/exodus/service/service2 && export CPLUS_INCLUDE_PATH=../sys:../gen:../fin:../agy:../job:../tim:../med:../test && export PATH=$PATH:~/bin && cd sys && compile ../???/*.cpp
RUN mkdir /var/run/sshd
RUN sed -i -e's/without-password/yes/' /etc/ssh/sshd_config
RUN echo 'root:somesillysecret' |chpasswd

# configure exodus and apache2

#grant access to apache and php to read and write exodus request/response files (g+s) forces all files created to be group www-exodus. add exodus/root to group www-exodus
RUN groupadd www-exodus && usermod -a -G www-exodus www-data && usermod -a -G www-exodus root && chgrp -R www-exodus /exodus/service/data/ && chmod g+sw -R /exodus/service/data/ && chmod g+x /exodus/service/data/ && find /exodus/service/data/ -type d -exec chmod g+rwxs {} +
RUN mv /var/www/html /var/www/html.orig && ln -s /exodus/www /var/www/html
RUN a2enmod ssl rewrite && a2ensite default-ssl
RUN cp /exodus/service/exodus-apache2.conf /etc/apache2/conf-enabled

EXPOSE 22
EXPOSE 80
EXPOSE 443

# start up exodus server

CMD echo "host=$PGEXODUS_PORT_5432_TCP_ADDR port=$PGEXODUS_PORT_5432_TCP_PORT dbname=exodus user=exodus password=somesillysecret connect_timeout=10" > /root/.exodus && service apache2 start && service ssh start && export EXO_HOST=$PGEXODUS_PORT_5432_TCP_ADDR && export EXO_PORT=$PGEXODUS_PORT_5432_TCP_PORT && cd ~/exodus/service/service2/sys/ && ~/bin/server

#build like this
#sudo docker.io build -t exodus .

#run like this
#assuming pgexodus database is running in container named "pgexodus"
#sudo docker.io run -P --rm --link pgexodus:pgexodus exodus

