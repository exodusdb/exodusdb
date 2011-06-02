#!/bin/bash

source ./config.sh UPLOAD
# > config.log

echo EXO_UPLOAD_CMD="$EXO_UPLOAD_CMD"
echo EXO_UPLOAD_OPT="$EXO_UPLOAD_OPT"

$EXO_UPLOAD_CMD $EXO_UPLOAD_OPT
