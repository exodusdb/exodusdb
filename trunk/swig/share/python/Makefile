make:
	./make.sh python
clean:
	./clean.sh python
install:
	dh_installexamples -Xsvn examples/* -X.pyc 2>/dev/null || \
	( \
		echo "dh_installexamples didnt work. doing it the manual way" && \
		mkdir -p $(DESTDIR)/usr/share/doc/libexodus-python/examples && \
		cp examples/* $(DESTDIR)/usr/share/doc/libexodus-python/examples \
	)
	./install.sh python
	echo ------------
