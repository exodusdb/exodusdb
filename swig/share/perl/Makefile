EXO_MODULE:=perl
make:
	./make.sh ${EXO_MODULE}
clean:
	./clean.sh ${EXO_MODULE}
install:
	./install.sh ${EXO_MODULE}
	dh_installexamples -Xsvn examples/* -X.pyc 2>/dev/null || \
	( \
		echo "dh_installexamples didnt work. doing it the manual way" && \
		mkdir -p $(DESTDIR)/usr/share/doc/libexodus-${EXO_MODULE}/examples && \
		cp examples/* $(DESTDIR)/usr/share/doc/libexodus-${EXO_MODULE}/examples \
	)
