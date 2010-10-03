#include <exodus/exodus.h>

subroutine sortselect(in file, in sortselectfilterclause)
{

	println();
	println("sselect the data - " ^ sortselectfilterclause);

	if (!file.select(sortselectfilterclause)) {
		println("cannot select");
		stop();
	}

	println("read the data");

	var record;
	var key;
	//could also use .readnextrecord() here if we had used .selectrecord() above
	while (file.readnext(key)) {
		if (not read(record,file,key))
			continue;
		print(key ^ ": ");
		println(record.convert(FM,"|"));
		
		//for (var ii=10 ; ii<=19 ; ii++) println(record.substr(ii,1) ^ " " ^ seq(record.substr(ii,1)));
	}

	return;

}

program()
{

	//written with pick style "global function" syntax instead of OO style "method calls"

	if (not var().connect()) {
		println("Cannot connect to database. Please check configuration");
		stop();
	}

	var filename="test_clients";
	var dictfilename="dict_" ^ filename;

	//leave the test_clients and dict_test_clients around for playing with
	var cleanup=false;
	if (cleanup) {
		deletefile(filename);
		deletefile(dictfilename);
	}
	println();
	println("create test file ", filename);

	if (not createfile(filename)) {
		//println("Cannot create " ^ filename);
		//stop();
	}

	println();
	println("open the file");

	var file;
	if (not open(filename, file)) {
		println("Cannot open " ^ filename);
		stop();
	}

	println();
	println("create the test files dictionary DICT_TEST_CLIENTS");

	if (not createfile(dictfilename)) {
		//println("Cannot create " ^ dictfilename);
		//stop();
	}

	println();
	println("open the dictionary");

	var dictfile;
	if (not open(dictfilename, dictfile)) {
		println("Cannot open dictionary file");
		stop();
	}

	println();
	println("prepare some dictionary records");

	var dictrecs = "";
	dictrecs  =      "CLIENT_CODE |F|0|Client code ||||          ||L|10";
	dictrecs ^= FM ^ "CLIENT_NAME |F|1|Client name ||||          ||T|20";
	dictrecs ^= FM ^ "CLIENT_TYPE |F|2|Client type ||||          ||L|10";
	dictrecs ^= FM ^ "DATE_CREATED|F|3|Date created||||D4        ||L|12";
	dictrecs ^= FM ^ "TIME_CREATED|F|4|Time created||||MTH       ||L|12";
	dictrecs ^= FM ^ "BALANCE     |F|5|Balance     ||||MD20      ||R|15";
	dictrecs ^= FM ^ "TIMESTAMP   |F|6|Timestamp   ||||[DATETIME]||L|20";
	dictrecs ^= FM ^ "@CRT        |G| |CLIENT_CODE CLIENT_NAME CLIENT_TYPE DATE_CREATED TIME_CREATED BALANCE TIMESTAMP";

	println();
	println("write the dictionary records to the dictionary");

	var nrecs=count(dictrecs,FM)+1;
	for (var recn = 1; recn <= nrecs; recn++)
	{
		var dictrec=dictrecs.extract(recn);
		var key=field(dictrec,"|",1);
		var rec=field(dictrec,"|",2,9999);
		println(key ^ ": " ^ rec);
		key=trim(key);
		rec=trim(rec);
		rec=swap(rec," |","|");
		write(convert(rec,"|",FM), dictfile, key);
	}

	var rec;
	if (not read(rec,dictfile,"BALANCE"))
		println("Cant read BALANCE record from dictionary");

	println();
	println("clear the client file");
	clearfile(filename);

	println();
	println("prepare some data records in a readable format");

	var recs = "";
	recs ^= FM ^ "SB001|Client AAA |A |15070|76539|1000.00|15070.76539";
	recs ^= FM ^ "JB002|Client BBB |B |15000|50539|200.00|15000.50539";
	recs ^= FM ^ "JB001|Client CCC |B |15010|60539|2000.00|15010.60539";
	recs ^= FM ^ "SB1  |Client SB1 |1 |     |     |       |           ";
	recs ^= FM ^ "JB2  |Client JB2 |2 |14000|10539|0      |14000.10539";
	recs ^= FM ^ "JB10 |Client JB10|10|14010|10539|2000.00|14010.10539";
	recs.splicer(1,1,"");

	println();
	println("Write the data records to the data file");

	nrecs=count(recs,FM)+1;
	for (var recn = 1; recn <= nrecs; recn++)
	{
		var rec=recs.extract(recn);
		var key=field(rec,"|",1);
		rec=field(rec,"|",2,9999);
		println(key ^ ": " ^ rec);
		while (index(rec," |"))
			swapper(rec," |","|");
		write(convert(rec,"|",FM).trimb(), file, trim(key));
	}

	var cmd="list test_clients id-supp";
	println();
	println("list the clients using "^cmd.quote());

	osshell(cmd);
	
	sortselect(file,"BY CLIENT_CODE");

	sortselect(file,"BY BALANCE");

	sortselect(file,"BY TIMESTAMP");

	sortselect(file,"WITH CLIENT_TYPE 'B'");

	if (cleanup) {
	
	 println();
	 println("delete the files");

	 deletefile(file);
	 deletefile(dictfile);
	}
}
