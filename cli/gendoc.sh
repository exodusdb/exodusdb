#!/bin/bash
set -euxo pipefail
:
:	$0 $*
:
:	'Syntax is ./gendoc.sh [GENDOC_BIN_PATH] [EXODUS_SRC_PATH]'
:	'e.g. cd cli && ./gendoc.sh'
:
	GENDOC_BIN_PATH=${1:-}
	EXODUS_SRC_PATH=${2:-~/exodus/exodus}
	TEST_CODE_DIR=${EXODUS_SRC_PATH}/../test

	HEADER_FILES=($EXODUS_SRC_PATH/../var/{var.h,varb.h,vars.h,vard.h,varo.h,dim.h} $EXODUS_SRC_PATH/{exoprog.h,job_manager.h,task_manager.h})
	${GENDOC_BIN_PATH}gendoc $TEST_CODE_DIR ${HEADER_FILES[@]} {m} > $EXODUS_SRC_PATH/var.1
	${GENDOC_BIN_PATH}gendoc                ${HEADER_FILES[@]} {h} > $EXODUS_SRC_PATH/var.htm
