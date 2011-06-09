#!/bin/bash

source ./config.sh MAKE
# > config.log

test -f Makefile || echo $EXO_CONFIGURE_CMD $EXO_CONFIGURE_OPT
test -f Makefile || $EXO_CONFIGURE_CMD $EXO_CONFIGURE_OPT

echo "$EXO_MAKE_CMD $EXO_MAKE_OPT"

$EXO_MAKE_CMD $EXO_MAKE_OPT

