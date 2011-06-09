#!/bin/bash

source ./config.sh CLEAN
# > config.log

test -f Makefile && echo $EXO_CLEAN_CMD $EXO_CLEAN_OPT
test -f Makefile && $EXO_CLEAN_CMD $EXO_CLEAN_OPT

test -f Makefile && echo $EXO_DISTCLEAN_CMD $EXO_DISTCLEAN_OPT
test -f Makefile && $EXO_DISTCLEAN_CMD $EXO_DISTCLEAN_OPT

