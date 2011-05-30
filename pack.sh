#!/bin/bash

source ./config.sh PACK > config.log

$EXO_PACK_CMD $EXO_PACK_OPT

if [ $EXO_BATCHMODE -eq "" ] pause
