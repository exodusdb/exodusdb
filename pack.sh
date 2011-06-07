#!/bin/bash
set -e
source ./config.sh PACK

echo EXO_PACK_CMD=$EXO_PACK_CMD
echo EXO_PACK_OPT=$EXO_PACK_OPT

$EXO_PACK_CMD $EXO_PACK_OPT

test "$EXO_BATCHMODE" = "" && read -p "Press Enter:"
