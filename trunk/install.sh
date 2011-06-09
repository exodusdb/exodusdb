#!/bin/bash

source ./config.sh INSTALL

echo "$EXO_INSTALL_CMD $EXO_INSTALL_OPT"

$EXO_INSTALL_CMD $EXO_INSTALL_OPT

test "$EXO_BATCHMODE" = "" && read -p "Press Enter:"
