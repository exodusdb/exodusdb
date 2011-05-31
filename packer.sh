#!/bin/bash

source ./config.sh PACK > config.log

echo $EXO_PACKER_CMD $EXO_PACKER_OPT

$EXO_PACKER_CMD $EXO_PACKER_OPT

#if [ $EXO_BATCHMODE -eq "" ] pause
