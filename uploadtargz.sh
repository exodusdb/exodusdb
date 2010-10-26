#!/bin/bash

#./googlecode_upload.py --help
#Usage: googlecode-upload.py -s SUMMARY -p PROJECT [options] FILE
#
#Options:
#  -h, --help            show this help message and exit
#  -s SUMMARY, --summary=SUMMARY
#                        Short description of the file
#  -p PROJECT, --project=PROJECT
#                        Google Code project name
#  -u USER, --user=USER  Your Google Code username
#  -w PASSWORD, --password=PASSWORD
#                        Your Google Code password
#  -l LABELS, --labels=LABELS
#                        An optional list of comma-separated labels to attach
#                        to the file

./googlecode_upload.py \
 -s "Builder for Linux etc. ./configure && make && sudo make install" \
 -p "exodusdb" \
 -u "neosys.com@gmail.com" \
 -w "${GOOGLECODE_PASS}" \
 -l "Featured,Type-Archive,OpSys-Linux" \
 $1
