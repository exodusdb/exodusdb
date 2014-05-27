FROM ubuntu

MAINTAINER steve.bush@neosys.com

RUN apt-get update

RUN apt-get -y install wget g++ build-essential libreadline-dev zlib1g-dev libxml2-dev libxslt-dev libssl-dev

RUN wget http://ftp.postgresql.org/pub/source/v9.4beta1/postgresql-9.4beta1.tar.bz2

RUN bunzip2 postgresql-9.4beta1.tar.bz2

RUN tar xvf postgresql-9.4beta1.tar

RUN cd postgresql-9.4beta1 && ./configure --prefix=/usr/local/ && make && make install

#get apps to download and build libpgexodus
RUN apt-get install -y subversion cmake

#get libpgexodus source
RUN svn co HTTPS://exodusdb.googlecode.com/svn/trunk/exodus/libpgexodus ~/libpgexodus

#build libpgexodus
RUN cd ~/libpgexodus/src && cmake . && make

#add user
RUN useradd postgres

#create postgres working directory
RUN mkdir -p /usr/local/pgsql/data

RUN chown postgres /usr/local/pgsql/data


#run as postgres to install and configure postgres
USER postgres

#create the postgres initial database and configuration files
RUN initdb -D /usr/local/pgsql/data -E UTF-8

#adjust PostgreSQL configuration so that remote connections to the database are possible. 
RUN echo "host all  all    0.0.0.0/0  md5" >> /usr/local/pgsql/data/pg_hba.conf

#add ``listen_addresses`` to ``/usr/local/pgsql/data/postgresql.conf``
RUN echo "listen_addresses='*'" >> /usr/local/pgsql/data/postgresql.conf


#run as root to install exodus and remove build apps
USER root

#start postgres and install exodus functions
RUN (su -c "postgres -D /usr/local/pgsql/data" postgres & ) && cd ~/libpgexodus/src && make install

#remove build apps
RUN apt-get -y autoremove wget g++ build-essential libreadline-dev zlib1g-dev libxml2-dev libxslt-dev libssl-dev subversion cmake

#remove source dirs
RUN rm -rf ~/postgresql-9.4beta1 ~/postgresql-9.4beta1.tar ~/libpgexodus


#run as postgres
USER postgres

#expose the PostgreSQL port
EXPOSE 5432

#add VOLUMEs to allow backup of config, logs and databases
VOLUME  ["/etc/postgresql", "/var/log/postgresql", "/var/lib/postgresql"]

#set the default command to run when starting the container
CMD ["postgres", "-D", "/usr/local/pgsql/data"]

#sudo docker.io run --rm -P --name pgexodus postgresql-9.4beta1 &
#sudo docker.io ps #to find exposed port number
#nano ~/.exodus #and replace port number
#psql -h localhost -p 49160 -d exodus -U exodus --password < ~/exodus.sql

#sudo docker.io ps
#sudo docker.io stop 84c85dcd8cc8

