#!/bin/bash
set -e

export SWIG_SYNTAX="Syntax is ./clean.sh"
export SWIG_TARGET=$1

source ../../config.sh
if [ "$SWIG_TARGET" == "all" ]; then
	for  SWIG_TARGET in $SWIG_ALL_TARGETS; do
		push pkg/$SWIG_TARGET
		../../clean.sh $SWIG_TARGET
		popd
	done
	echo "all clean"
	exit 0
fi

echo
echo ------------------
echo $SWIG_TARGET clean
echo ------------------
test -d $SWIG_TARGET || exit 0
echo -ne "Removing: `pwd`/$SWIG_TARGET: "
rm -rf $SWIG_TARGET && echo "removed."

