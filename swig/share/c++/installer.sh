#!/bin/bash

dh_installexamples -Xsvn examples/* -X.pyc 2>/dev/null ||
( \
echo "dh_installexamples didnt work. doing it the manual way" && \
mkdir -p $(DESTDIR)/usr/share/doc/packages/libexodus-${EXO_MODULE}/examples && \
cp examples/* $(DESTDIR)/usr/share/doc/packages/libexodus-${EXO_MODULE}/examples \
)
