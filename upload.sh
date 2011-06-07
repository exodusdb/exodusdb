#!/bin/bash

source ./config.sh UPLOAD

echo EXO_UPLOAD_CMD="$EXO_UPLOAD_CMD"
echo EXO_UPLOAD_OPT="$EXO_UPLOAD_OPT"

test "UPLOADPASS_EXO" = "" && "UPLOADPASS_EXO must be configured first"
test "UPLOADPASS_EXO" = "" && exit

$EXO_UPLOAD_CMD $UPLOAD_OPT_EXO

test "$EXO_BATCHMODE" = "" && read -p "Press Enter:"
