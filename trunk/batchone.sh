#!/bin/bash

echo ---------------------------batchone.sh---------------------------- >> batch.log
date >> batch.log

echo EXO_BATCH_MAJOR_VER  = $EXO_BATCH_MAJOR_VER >>batch.log
echo EXO_BATCH_MINOR_VER  = $EXO_BATCH_MINOR_VER >>batch.log
echo EXO_BATCH_MICRO_VER  = $EXO_BATCH_MICRO_VER >>batch.log
echo EXO_BATCH_BUILD_VER  = $EXO_BATCH_BUILD_VER >>batch.log
echo BATCH_TARGET_CPU     = $BATCH_TARGET_CPU    >> batch.log
echo BATCH_CONFIGURATION  = $BATCH_CONFIGURATION >> batch.log

export EXO_BATCHMODE=YES

test "$EXO_BATCH_CLEAN"  = "YES" && ./clean.sh  1>> batch.log 2>>batch.err
test "$EXO_BATCH_MAKE"   = "NO"  || ./make.sh   1>> batch.log 2>>batch.err
test "$EXO_BATCH_PACK"   = "NO"  || ./pack.sh   1>> batch.log 2>>batch.err
test "$EXO_BATCH_UPLOAD" = "YES" && ./upload.sh 1>> batch.log 2>>batch.err

date >> batch.log

