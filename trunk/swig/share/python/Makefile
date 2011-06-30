make:
	./make.sh python
clean:
	./clean.sh python
install:
	./install.sh python
	#mkdir -p debian/libexodus-python/usr/share/doc/libexodus-python/examples
	#cp examples/* debian/libexodus-python/usr/share/doc/libexodus-python/examples
	dh_installexamples -Xsvn examples/* -X.pyc
