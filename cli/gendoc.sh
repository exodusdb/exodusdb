#!/bin/bash
set -euxo pipefail
:
:	$0 $*
:
:	Syntax is ./gendoc.sh [GENDOCPATH] [EXODUS_PATH]
:
	GENDOC_PATH=${1:-}
	EXODUS_PATH=${2:-~/exodus/exodus}

	${GENDOC_PATH}gendoc $EXODUS_PATH/{var.h,dim.h,exoprog.h,job_manager.h,fiber_manager.h} {m} > $EXODUS_PATH/var.1
	${GENDOC_PATH}gendoc $EXODUS_PATH/{var.h,dim.h,exoprog.h,job_manager.h,fiber_manager.h} {h} > $EXODUS_PATH/var.htm