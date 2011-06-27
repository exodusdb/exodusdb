make:
	./make.sh perl
clean:
	rm -rf debian/files || echo "debian/files not removed"
	./clean.sh perl

all:
	mkdir debian/files || echo "debian/files already exists"
	./install.sh perl
