EXO_MODULE:=php
make:
	./make.sh ${EXO_MODULE}
test:
	cp php/exo.php .
	export LD_LIBRARY_PATH="php:${LD_LIBRARY_PATH}"
	php examples/test.php
	php -r 'include("./exo.php");$$exo=new mvar("D");print $$exo->date()->oconv($$exo)."\n";'
	php -r 'include("./exo.php");$$exo=new mvar("D");print $$exo->date()->oconv("D")."\n";'
clean:
	./clean.sh ${EXO_MODULE}
install:
	./install.sh ${EXO_MODULE}

