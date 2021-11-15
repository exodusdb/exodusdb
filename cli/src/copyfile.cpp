#include <exodus/program.h>
programinit()

function main() {

	if (not COMMAND.a(2) or not COMMAND.a(3))
		abort("Syntax is copyfile [<DB1>:][<FILE1>,...] [<DB2>:[<FILE2] {OPTIONS}");

	var dbname1 = "";
	var source = COMMAND.a(2);
	if (index(source,":")) {
		dbname1 = field(source,":",1);
		source = field(source,":",2);
	}

	var dbname2 = "";
	var target = COMMAND.a(3);
	if (index(target,":")) {
		dbname2 = field(target,":",1);
		target = field(target,":",2);
	}

	var db1;
	if (not db1.connect(dbname1))
		abort(dbname1.quote() ^ " Cannot connect");

	var db2;
	if (not db2.connect(dbname2))
		abort(dbname2.quote() ^ " Cannot connect");

	if (not source) {
		source = db1.listfiles();
		target = "";
	}

	source.converter(",", FM);

	for (var filename1 : source) {

		if (filename1 eq "dict.all")
			continue;

		var filename2 = target;
		if (not filename2)
			filename2 = filename1;

		//open file from source
		var file1;
		if (not file1.open(filename1, db1) )
			abort(filename1.quote() ^ " cannot be opened in db " ^ dbname1.quote());

		//open file from target
		var file2;
		if (not file2.open(filename2, db2)) {
			if (not OPTIONS.index("C") or not db2.createfile(filename2) or not file2.open(filename2, db2))
				abort(filename2.quote() ^ " cannot be opened in db " ^ dbname2.quote());
		}

		db2.begintrans();

		//select targetfile
		file1.select(filename1 ^ " (R)");

		//readnext key
		var recn = 0;
		while(file1.readnext(RECORD, ID, MV)) {

			recn++;
			if (not mod(recn,1000)) {
				print(at(-40), filename1, recn ^ ".", ID);
				if (esctoexit())
					abort("");
			}

			RECORD.write(file2, ID);

		}

		db2.committrans();

		if (recn)
			printl(at(-40), filename1, recn);

	}

	return 0;
}

programexit()
