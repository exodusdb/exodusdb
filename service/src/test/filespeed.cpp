#include <exodus/program.h>
programinit()

function main() {

	printl("filespeed says 'Hello World!'");
	createfile("TEMP");
	var file;
	open("TEMP",file);
	var rec=str("x",1000);

	begintrans();
	for (var ii=0;ii<1000;++ii){
//		print("+");
		write(rec,file,ii);
	}
	printl("written 1000 1000byte records");
	for (int ii=0;ii<1000;++ii){
//		print("-");
		deleterecord(file,ii);
	}
	committrans();
	printl("deleted 1000 1000byte records");

	deletefile("TEMP");
	return 0;
}

programexit()

