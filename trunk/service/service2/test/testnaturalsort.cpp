#include <exodus/program.h>
#include <assert.h>
programinit()

function main() {
	printl("testqqqnaturalsort says 'Hello World!'");

	var tempsortfile="temptestqqqnaturalsortfile";
	deletefile(tempsortfile);

	//let everything be one transaction
	//with automatic rollback at close of program ^_^
	begintrans();

	createfile(tempsortfile);
	clearfile(tempsortfile);

	var ndigits=9;
	var nzeros=25;

	//create records backwards to testqqq sort
	var ii,jj;
	for (ii=ndigits;ii>=1;--ii)
		for (jj=nzeros;jj>=0;--jj) {
			write("x",tempsortfile,"x"^ii^str("0",jj));
		}
	//read sorted records forwards to check numbers
	tempsortfile.select("by @ID");
	for (jj=0;jj<=nzeros;++jj) {
		for (ii=1;ii<=ndigits;++ii) {
			var key;
			var key2;
			tempsortfile.readnext(key);
			key2="x"^ii^str("0",jj);
			if (key ne key2) {
				printt(key,key2, "out of order");
				printl();
			}
		}
	}

	//rely on automatic rollback
	//deletefile(tempsortfile);
	//committrans();

	return 0;
}

programexit()

