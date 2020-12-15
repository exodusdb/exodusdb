FROM ubuntu:bionic
MAINTAINER steve.bush@neosys.com
RUN apt-get update; apt-get -y install curl sudo apt-utils
#RUN curl https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | bash

EXPOSE 22
EXPOSE 80
EXPOSE 443

#build like this
#sudo docker.io build -t exodus .

#run like this
#assuming pgexodus database is running in container named "pgexodus"
#sudo docker.io run -P --rm --link pgexodus:pgexodus exodus

