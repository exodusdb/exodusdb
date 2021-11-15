#include <exodus/program.h>
programinit()

function main() {

	var db1 = "";
	var source = COMMAND.a(2);
	if ( index(source,":")) {
		db1 = field(source,":",1);
		source = field(source,":",2);
	}

	var db2 = "";
	var target = COMMAND.a(3);
	if ( index(target,":")) {
		db2 = field(target,":",1);
		target = field(target,":",2);
	}

	var options = OPTIONS;

	source = "users";
	target = "users2";

	//open file from source
	var file1;
	if ( not file1.open(source) ) {
		call fsmsg();
		stop();
	}

	//open file from source
	var file2;
	if ( not file2.open(target) ) {
		call fsmsg();
		stop();
	}
	//JE
	//if ( not file2.open(target) ) {
	//	if ( not option C )
	//		call fsmsg();
	//		stop();
	//	else
	//		createfile file2
	//		if ( not file2.open(target) )
	//		call fsmsg();
	//		stop();



	//select targetfile
	file1.select();

	//readnext key
	while(file1.readnext(ID)) {

		//read record from file, key otherwise get next ID
		if ( not RECORD.read(file1, ID) ) {
			continue;
		}

		//write record from file, key
		RECORD.write(file2, ID);

	}

	return 0;
}

programexit()

