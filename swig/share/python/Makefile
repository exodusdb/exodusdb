make:
	./make.sh python
clean:
	./clean.sh python
install:
	./install.sh python
	if `which dh_installexamples` then;
		dh_installexamples -Xsvn examples/* -X.pyc
	else
		mkdir -p $(DESTDIR)/usr/share/doc/libexodus-python/examples
		cp examples/* $(DESTDIR)/usr/share/doc/libexodus-python/examples
	fi
