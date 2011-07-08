#!/bin/bash
set -x

export EXO_MODULE=$1

echo "DESTDIR=$DESTDIR"
export DOCDIR=/usr/share/doc
test -d $DOCDIR/packages && export DOCDIR=$DOCDIR/packages
echo "DOCDIR=$DOCDIR"

dh_installexamples -Xsvn examples/* -X.pyc 2>/dev/null ||
( \
echo "dh_installexamples didnt work. doing it the manual way" && \
mkdir -p $DESTDIR$DOCDIR/libexodus-$EXO_MODULE/examples && \
cp examples/* $DESTDIR$DOCDIR/libexodus-$EXO_MODULE/examples \
)
