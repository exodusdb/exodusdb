#!/bin/bash
set -euxo pipefail
:
:	$0 $*
:
:	Syntax is ./gendoc.sh [GENDOCPATH] [EXODUS_PATH]
:
	GENDOC_PATH=${1:-}
	EXODUS_PATH=${2:-~/exodus/exodus}
	TEST_CODE_DIR=${EXODUS_PATH}/../test

	HEADER_FILES=($EXODUS_PATH/{var.h,varb.h,vars.h,varo.h,dim.h,exoprog.h,job_manager.h,task_manager.h})
	${GENDOC_PATH}gendoc $TEST_CODE_DIR ${HEADER_FILES[@]} {m} > $EXODUS_PATH/var.1
	${GENDOC_PATH}gendoc                ${HEADER_FILES[@]} {h} > $EXODUS_PATH/var.htm
