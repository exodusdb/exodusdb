#!/bin/bash

source ./config.sh PACK

echo $EXO_PACKER_CMD $EXO_PACKER_OPT

$EXO_PACKER_CMD $EXO_PACKER_OPT

test "$EXO_BATCHMODE" = "" && read -p "Press Enter:"
