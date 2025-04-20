#if EXO_MODULE
#else
#	include <exodus/dim.h>
#	include <exodus/rex.h>
#endif

#include <exodus/exoprog.h>
#include <exodus/exocallable.h>

#include <exodus/exoimpl.h>

// Allows and *requires* coding style like exodus application programming
// e.g. must use USERNAME not mv.USERNAME
#include <exodus/exomacros.h>

// Putting various member functions into all exodus programs allows access to the mv environment
// variable which is also available in all exodus programs.

namespace exo {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"

///////////////////////////
// Special Member Functions
///////////////////////////

// 2. Destructor
ExoProgram::~ExoProgram(){}

var ExoProgram::libinfo(in libname) {
	// Return dir size/date/time of /home/user/lib/libxxxxxxxx.so
	return var(perform_callable_.libfilepath(libname)).osfile();
}

/////////////////////
// Other Constructors
/////////////////////

// Constructor from an ExoEnv
// mv is a reference to allow mv.xxxxxx syntax instead of mv->xxxxxx
// However, being a reference, it must be provided at ExodusProgram construction time
// and cannot be changed thereafter.
ExoProgram::ExoProgram(ExoEnv& inmv)
	:
	perform_callable_(inmv),
	mv(inmv)
{
	cached_dictid_ = "";
	dict_callable_ = nullptr;
}

#pragma GCC diagnostic pop

//////////////////
// Other functions
//////////////////

// select
bool ExoProgram::select(in sortselectclause_or_filehandle) {

	//TRACE(sortselectclause_or_filehandle)

	//simple select on filehandle
	if (sortselectclause_or_filehandle.contains(FM)) {
		CURSOR = sortselectclause_or_filehandle;
		return CURSOR.select();
	}

	var sortselectclause = sortselectclause_or_filehandle;

	// Stage 1
	/////////

	// Force default connection
	CURSOR.updater(2, "");

	// Indicate there are no calculated fields
	CURSOR.updater(10, "");

	//perform the select (stage 1 of possibly two stages)
	//any fields requiring calculation that cannot be done by the database
	//will be put skipped and put aside in CURSOR.updater(10) for stage 2

	// Sselect or fail
	if (!CURSOR.select(sortselectclause)) {
		return false;
		/////////////
	}

	// We are done if there are no calculated fields!
	var calc_fields = CURSOR.f(10);
	if (!calc_fields) {
		return true;
		////////////
	}

	// Stage 2 "2stage"
	//////////////////

	if (not TERMINAL) {
		CURSOR.convert(_ALL_FMS, _VISIBLE_FMS).logputl("2 Stage Select:");
		sortselectclause.logputl();
	}

	//secondary sort/select on fields that could not be calculated by the database

	//calc_fields.oswrite("calc_fields=");

	//clear the list of calculated fields
	CURSOR.updater(10, "");

	//vms to fms etc in calculated fields
	calc_fields.raiser();

	//ONLY TEST MATERIALS FOR NOW
	//if (!calc_fields.ucase().contains("MATERIALS"))
	//	return true;

	//debug
	//calc_fields.convert(FM^VM^SM,"   ").logputl("calc=");

	var dictfilename = calc_fields.f(5, 1);

	//debugging
	var calc_fields_file;
	if (!calc_fields_file.open("calc_fields"))
		calc_fields_file = "";

	//open the dictionary
	if (not dictfilename.lcase().starts("dict."))
		dictfilename = "dict." ^ dictfilename;
	if (!DICT.open(dictfilename)) {
		dictfilename = "dict.voc";
		if (!DICT.open(dictfilename)) {
			//throw VarDBException(dictfilename.quote() ^ " cannot be opened");
			UNLIKELY
			throw VarError(dictfilename.quote() ^ " cannot be opened");
		}
	}

	//prepare to create a temporary sql table
	//DROP TABLE IF EXISTS SELECT_STAGE2;
	//CREATE TABLE SELECT_STAGE2(
	// KEY TEXT PRIMARY KEY,
	// EXECUTIVE_CODE TEXT)
	let temptablename = "SELECT_CURSOR_STAGE2_" ^ CURSOR.f(1);
	var createtablesql = "";
	createtablesql ^= "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
	//createtablesql ^= "CREATE TABLE " ^ temptablename ^ "(\n";
	createtablesql ^= "CREATE TEMPORARY TABLE IF NOT EXISTS " ^ temptablename ^ "(\n";
	createtablesql ^= " KEY TEXT PRIMARY KEY,\n";

	//prepare to insert sql records
	// INSERT INTO books (id, title, author_id, subject_id)
	// VALUES (41472, 'Practical PostgreSQL', 1212, 4);
	var baseinsertsql = "INSERT INTO " ^ temptablename ^ "(KEY,";

	//parse the fields for speed
	//FIRST_APPEARANCE_DATE>='9/6/2019'
	//UPLOADS=''
	//AUTHORISED<>''
	//DEADLINE<='10/6/2019'
	int nfields = calc_fields.f(1).fcount(VM);
	dim dictids(nfields);
	dim opnos(nfields);
	dim reqivalues(nfields);
	dim reqivalues2(nfields);
	dim ioconvs(nfields);
	dim sqltypes(nfields);

	for (int fieldn = 1; fieldn <= nfields; ++fieldn) {

		//dictids

		var dictid = calc_fields.f(1, fieldn);

		var dictrec;
		if (not dictrec.readc(DICT,dictid))
			dictrec="";
		ioconvs[fieldn] = dictrec.f(7);

		//add colons to the end of every calculated field in the sselect clause
		//so that 2nd stage select knows that these fields are available in the
		//temporary parallel file
		sortselectclause.replacer(rex("\\b" ^ dictid ^ "\\b"), dictid ^ ":");

		dictid.converter(".", "_");
		dictids[fieldn] = dictid;
		let sqlcolid = dictid ^ "_calc";

		//ops

		var ovalue = calc_fields.f(3, fieldn).convert(SM, VM).unquote();

		var op = calc_fields.f(2, fieldn);

		//multivalued selections are not well supported. handle the obvious cases"
		if (ovalue.contains(VM)) {
			if (op == "=")
				op = "in";
			else if (op == "<>")
				op = "not_in";
		}

		//turn ops into numbers for speed. see c++ switch statement below
		var opno;
		if (!op)
			opno = 0;
		else if (not var("= <> > < >= <= ~ ~* !~ !~* >< >!< in not_in !! ! ] [ []").locateusing(" ", op.convert(" ", "_"), opno))
			UNLIKELY
			throw VarError(op.quote() ^ " unknown op in sql select");
		opnos[fieldn] = opno;

		//reqivalues
		if (op == "in" and ovalue.starts("(") and ovalue.ends(")")) {
			ovalue.cutter(1);
			ovalue.popper();
			ovalue.replacer("', '", VM);
			ovalue.trimmerboth();
			ovalue.unquoter();
			//ovalue.convert(VM,"]").logputl("ovalue=");
		}

		let ovalue2 = calc_fields.f(4, fieldn).unquote();

		//iconv
		var ivalue;
		var ivalue2;
		let ioconv = ioconvs[fieldn];
		if (ioconv) {
			ivalue = iconv(ovalue, ioconv);
			ivalue2 = iconv(ovalue2, ioconv);
		}
		else {
			ivalue = ovalue;
			ivalue2 = ovalue2;
		}

		//save reqivalues
		reqivalues[fieldn] = ivalue;
		reqivalues2[fieldn] = ivalue2;

		//sqltype
		var sqltype;
		//if (ioconv.starts("[DATETIME")) {
		//	sqltype = "DATE";//TODO
		//}
		if (ioconv.starts("[DATE")) {
			sqltype = "DATE";
            ioconvs[fieldn] = "D";
		}
		else if (ioconv.starts("[TIME")) {
			sqltype = "TIME";//TODO check if works
		}
		// Note that we aslo assume numeric if a filter value is present and is numeric (without quotes)
		else if (ioconv.starts("[NUMBER") or (not ovalue.empty() and ovalue.isnum())) {
			sqltype = "NUMERIC";
		}
		else {
			sqltype = "TEXT";
		}
		sqltypes[fieldn] = sqltype;
		//TRACE(dictid^" "^ioconv^" "^sqltype)

		//sql temp table column
		createtablesql ^= " " ^ sqlcolid ^ " "^ sqltype ^ ",";

		//sql insert column
		baseinsertsql ^= sqlcolid ^ ",";

		//debug
		//dictid.outputt();
		//op.outputt();
		//ovalue.outputt();
		//var("").logputl();

		//debug
		if (calc_fields_file && dictid != "AUTHORISED") {
			let key = dictfilename ^ "*" ^ dictid;
			let rec = sortselectclause ^ FM ^ op ^ FM ^ ovalue ^ FM ^ ovalue2;
			rec.write(calc_fields_file, key);
			key.logput("written to calc_fields ");
		}
	}

	if (baseinsertsql.ends(",")) {

		baseinsertsql.popper();
		baseinsertsql ^= ")";
		baseinsertsql ^= " VALUES (";

		createtablesql.popper();
		createtablesql ^= ")";
		//createtablesql.logputl();

		//clear the table in case we are reusing it
		createtablesql ^= ";DELETE FROM " ^ temptablename ^ ";\n";

		//create the temporary table
		if (!CURSOR.sqlexec(createtablesql))
			UNLIKELY
			throw VarError("ExoProgram::select " ^ CURSOR.lasterror());

	} else
		baseinsertsql = "";

	int maxnrecs = calc_fields.f(6);
	int recn = 0;

	//nextrecord:
	while (CURSOR.readnext(RECORD, ID, MV)) {

		bool allok = true;

		//var id2 = MV ? (ID ^ "*" ^ MV) : ID;
		var insertsql = baseinsertsql ^ ID.replace("'", "''").squote() ^ ",";

		//some or all fields may have filters on them
		//some or all fields may not have filters, if 'order by' calculated fields
		for (int fieldn = 1; fieldn <= nfields; ++fieldn) {

			let ivalues = calculate(dictids[fieldn]);
//			TRACE(ivalues)
//			TRACE(opnos.join())
//			TRACE(fieldn)
//			TRACE(opnos[fieldn])
			if (opnos[fieldn]) {
				//debug
				//ivalues.logputl(dictids[fieldn] ^ " ivalues=");

				var nextsep;
				var pos = 1;
				bool ok = false;

				while (true) {

					let ivalue = ivalues.substr2(pos,nextsep);
//					TRACE(ivalue)

					switch (int(opnos[fieldn])) {
						case 0://cannot occur - due to if statement above
							break;
						case 1:	 // =
							ok = ivalue == reqivalues[fieldn];
							//ok = ivalue.locate(reqivalues[fieldn]);
							//if (ok) {
							//	TRACE(ivalue)
							//	TRACE(reqivalues[fieldn])
							//	TRACE(calculate("COMPANY_CODE"))
							//}
							break;
						case 2:	 // <>
							ok = ivalue != reqivalues[fieldn];
							//ok = !ivalue.locate(reqivalues[fieldn]);
							break;
						case 3:	 // >
							ok = ivalue > reqivalues[fieldn];
							break;
						case 4:	 // <
							ok = ivalue < reqivalues[fieldn];
							break;
						case 5:	 // >=
							ok = ivalue >= reqivalues[fieldn];
							//TRACE(ivalue ^ " " ^ reqivalues[fieldn] ^ " " ^ ok)
							break;
						case 6:	 // <=
							ok = ivalue <= reqivalues[fieldn];
							break;
						case 7:	 // ~ regex
							ok = ivalue.match(reqivalues[fieldn]);
							break;
						case 8:	 // ~* regex case insensitive
							ok = ivalue.match(reqivalues[fieldn], "i");
							break;
						case 9:	 // !~ not regex
							ok = !(ivalue.match(reqivalues[fieldn]));
							break;
						case 10:  // !~* not regex case insensitive
							ok = !(ivalue.match(reqivalues[fieldn], "i"));
							break;
						case 11:  // between x and y, from x to
							ok = (ivalue >= reqivalues[fieldn] && ivalue <= reqivalues2[fieldn]);
							break;
						case 12:  // not between x and y, not from x to y
							ok = (ivalue < reqivalues[fieldn] || ivalue > reqivalues2[fieldn]);
							break;
						case 13:  // in list
							ok = reqivalues[fieldn].locate(ivalue);
							break;
						case 14:  // not in list
							ok = !reqivalues[fieldn].locate(ivalue);
							break;
						case 15:  // is true (not "" 0 "0" "00" "0.0" etc).
							ok = ivalue;
							//ok = !!ivalue;
//							TRACE(!!ivalue)
							break;
						case 16:  // is false (isnt true)
							ok = !ivalue;
							break;
						case 17:  // STARTING ]
							ok = reqivalues[fieldn].starts(ivalue);
							break;
						case 18:  // ENDING [
							ok = reqivalues[fieldn].ends(ivalue);
							ok = !ivalue;
							break;
						case 19:  //  CONTAINING []
							ok = reqivalues[fieldn].contains(ivalue);
							break;
						default:
						    // TODO should this be a special system error since it cannot be caused by application programmer?
						    throw VarError("EXODUS: Error in " ^ var(__PRETTY_FUNCTION__) ^ " Invalid opmo " ^ opnos[fieldn]);
					}//switch

					//quit searching data values if found or no more
//					TRACE(ok)
					if (ok || !nextsep)
						break;

				}//for each ivalue

				//if a field fails test then skip and remaining fields and readnext candidate record
				if (!ok) {
					//debug
					//ivalue.logputl("FAILED=");
					allok = false;
					break;// out of fields loop. assuming that all filters are AND. fail one = skip record
				}

			}//if opno

			let sqltype = sqltypes[fieldn];
			var ovalues;
			if (sqltype == "DATE" || sqltype == "TIME" || sqltype == "TIMESTAMP") {
				if (ivalues) {
					ovalues = oconv(ivalues,ioconvs[fieldn]).squote();
				}
				else
					//ovalues = "nullptr";
					ovalues = "null";
			}
			else {
				//ivalueS (41472, 'Practical PostgreSQL', 1212, 4);
				ovalues = ivalues.replace("'", "''").squote();
			}

			insertsql ^= " " ^ ovalues ^ ",";

		}//fieldn

		//skip if failed to match
		if (!allok) {
//			ID.logputl("stage 1/2 failed ");
			continue;
		}

//		ID.logputl("stage 1/2 ok ");

		// Replace final comma with a closing bracket and additional SQL
		// Ignore any duplicates due to multivalues
		// TODO insert stage2 records with MV as par of the key?
		insertsql.popper();
		insertsql ^= ") ON CONFLICT (key) DO NOTHING";

		//insertsql.logputl();

		if (not CURSOR.sqlexec(insertsql)) {
			//tolerate failure to write in case multiple records returned (due to mv selection?)
			if (not CURSOR.lasterror().contains("duplicate key"))
				CURSOR.lasterror().errputl("Error inserting pass2 record:");
		}

		//option to limit number of records returned
		++recn;
		if (maxnrecs && recn > maxnrecs) {
			//break out of readnext loop
			CURSOR.clearselect();
			//break;
		}

	}//readnext

	if (not TERMINAL)
		sortselectclause.errputl("\nstage2=");

	bool result = CURSOR.select(sortselectclause);

	return result;
}

// savelist
bool ExoProgram::savelist(SV listname) {
	return CURSOR.savelist(var(listname).field(" ", 1));
}

// getlist
bool ExoProgram::getlist(SV listname) {
	return CURSOR.getlist(var(listname).field(" ", 1));
}

// formlist
bool ExoProgram::formlist(SV filename_or_command, in keys /*=""*/, const int fieldno /*=0*/) {

	//remove any options from the filename or command
	var filename2 = filename_or_command;
	if (filename2.ends(")")) {
		let options = filename2.field(" ", -1);
		filename2.cutter(-options.len());
		filename2.trimmerlast();
	}

	//optionally get keys from filename or command
	let keys2 = (keys == "") ? filename2.field(" ", 2, 999999999) : keys;

	//remove any keys from the filename
	filename2 = filename2.field(" ", 1);

	//open the file
	clearselect();
	if (not CURSOR.open(filename2))
		UNLIKELY
		throw VarDBException(filename2.quote() ^ " file cannot be opened in formlist(" ^ keys ^ ")");

// No longer available in vardb
//	return CURSOR.formlist(keys2, fieldno);

	var record;
	if (not record.read(CURSOR, keys2)) UNLIKELY {
//		throw VarError("formlist() cannot read on handle(" ^ CURSOR ^ ") ");
		keys2.errputl("formlist() cannot read on handle(" ^ CURSOR ^ ") ");
		return false;
	}

	//optional field extract
	if (fieldno)
		record = record.f(fieldno).convert(VM, FM);

	if (not this->selectkeys(record)) UNLIKELY
		throw VarDBException(CURSOR.lasterror());

	return true;
}

// makelist
bool ExoProgram::makelist(SV listname, in keys) {
	if (not listname.empty())
		throw VarDBException("makelist() with listname is not longer supported. Write to the lists file directly instead.");
	return CURSOR.selectkeys(keys);
}

// makelist
bool ExoProgram::selectkeys(in keys) {
	return CURSOR.selectkeys(keys);
}

// deletelist
bool ExoProgram::deletelist(SV listname) {
	return CURSOR.deletelist(var(listname).field(" ", 1));
}

// clearselect
void ExoProgram::clearselect() {
	CURSOR.clearselect();
}

// hasnext
bool ExoProgram::hasnext() {
	return CURSOR.hasnext();
}

// readnext 1
bool ExoProgram::readnext(out key) {
	return CURSOR.readnext(key);
}

// readnext 2
bool ExoProgram::readnext(out key, out valueno) {
	return CURSOR.readnext(key, valueno);
}

// readnext 3
bool ExoProgram::readnext(out record, out key, out valueno) {
	return CURSOR.readnext(record, key, valueno);
}

// deleterecord. ONE argument. no key. Delete MANY records.
// Uses CURSOR active select list to delete records.
bool ExoProgram::deleterecord(in filename) {

	let filename2 = filename.f(1).field(" ", 1);
	bool any_failures = false;
	var key;
	while (CURSOR.readnext(key)) {
		if (not filename2.deleterecord(key))
			any_failures = true;
	}
	return any_failures;
}

// deleterecord. TWO arguments. Delete ONE record.
bool ExoProgram::deleterecord(in dbfile, in key) {
	return dbfile.deleterecord(key);
}

// pushselect
void ExoProgram::pushselect(out saved_cursor) {
	// CURSOR has connection number hidden in it, and maybe other info, so it cannot be used as an ordinary variable
	// saved_cursor = CURSOR.move();
	saved_cursor = CURSOR;
	CURSOR = var().date() ^ "_" ^ (var().ostime().convert(".", "_"));
	return;
}

// popselect
void ExoProgram::popselect(in saved_cursor) {
	// CURSOR = saved_cursor.move();
	CURSOR = saved_cursor;
	return;
}

// note 2
//void ExoProgram::note(in msg, in options) const {
void ExoProgram::note(in msg) const {
	var response = "";
	note(msg, "", response);
}

// note 3
void ExoProgram::note(in msg, in options, io response) const {

	let params = "";
	let interactive = !SYSTEM.f(33);
	if (interactive)
		std::cout << var("----------------------------------------") << std::endl;

	//skip if just "downing" a previous "upped" message
	if (options.contains("D")) {
		return;
	}

	//we must pass the message unmodified into USER4 below
	//e.g. to pass ACCOUNTLIST back to client with FM/VM etc
	//var msg1 = msg.convert("|" ^ FM ^ VM ^ SM, "\n\n\n\n").trim("\n");
	var msg1 = msg;

	//replace %1, %2 etc with params
	for (var ii = 1; ii <= 9; ++ii)
		msg1.replacer("%" ^ ii, params.f(ii));

	msg1.converter(_FM _VM "|", "\n\n\n");
	msg1.trimmer("\n");

	std::cout << msg1 << std::endl;

	let origresponse = response.assigned() ? response : "";

	//R=Reply required in response
	if (options.contains("R")) {
		if (interactive) {

			//one space after the prompt
			//std::cout << " ";

			if (response.unassigned())
				response = "";

			//input with empty prompt allows defaulting and editing
			if (not response.input("? ")) {}

			//default
			//if (response == "")
			//	response=origresponse;

			//escape anywhere in the input returned as a single ESC character
			//or empty input with ESC option means ESC
			if (options.contains("E") and (response == "" or response.contains("\x1B")))
				response = "\x1B";  //esc

			std::cout << std::endl;
		} else {

			//input=output if not interactive
			response = origresponse;
		}

		//force upper case
		if (options.contains("C"))
			response.ucaser();

		return;
	}

	if (!options.contains("U")) {
		if (USER4.len() > 8000) {
			let msg2 = "Aborted MSG()>8000";
			if (not USER4.contains(msg2)) {
				std::cout << msg2 << std::endl;
				//std::cout << USER4 << std::endl;
				USER4 ^= FM ^ msg2;
			}
		} else {
			USER4.updater(-1, msg1);
		}
	}
}

// execute
var ExoProgram::execute(in command_line) {

	var saved_cursor;
	pushselect(saved_cursor);

	let nrvo = perform(command_line);

	popselect(saved_cursor);

	return nrvo;
}

// chain
[[noreturn]]
void ExoProgram::chain(in command_line) {

	// Pass an environment variable that the performing/executing program will run on termination of this program.
	CHAIN = command_line;

	this->stop();
}

// perform
var ExoProgram::perform(in command_line) {

	// lowercase all library functions to aid in conversion from pickos
	// TODO remove after conversion complete

	// TODO connect many "command" to system calls using the VOC file to enable easy command line interfaces to exodus features.

	var nrvo = "";

	// Hard coded to avoid needing a library. Currently only supporting upper case perform SELECT
	// TODO replace all perform SELECT with simple select()
	if (command_line.starts("SELECT ")) {
		select(command_line);
		nrvo = "";
		return nrvo;
	}
	if (command_line.starts("DELETEFILE ")) {
		if (not var().deletefile(command_line.replace(" DATA ", " ").replace(" DICT ", "dict.").field(" ", 2))) {};
		nrvo = "";
		return nrvo;
	}

	// Save some environment variables
	//////////////////////////////////

	// Must take a *COPY* since command_line might be a reference to SENTENCE if called like 'perform(SENTENCE)'
	//var savesentence = move(SENTENCE);
	let saved_sentence = SENTENCE;

	// Note that move sets the moved variable to an empty string "".
	let saved_command = COMMAND.move();
	let saved_options = OPTIONS.move();

	let saved_recur0 = RECUR0.move();
	let saved_recur1 = RECUR1.move();
	let saved_recur2 = RECUR2.move();
	let saved_recur3 = RECUR3.move();
	let saved_recur4 = RECUR4.move();

	let saved_id     = ID.move();
	let saved_record = RECORD.move();
	let saved_mv     = MV.move();
	let saved_dict   = DICT.move();

	const int saved_precision = var::getprecision();

	// Increase the perform/execute "level"
	LEVEL++;

	// A function to restore the environment
	////////////////////////////////////////
	auto restore_environment = [&]() {

		// Restore the saved environment variables

		saved_sentence.swap(SENTENCE);
		saved_command.swap(COMMAND);
		saved_options.swap(OPTIONS);

		saved_recur0.swap(RECUR0);
		saved_recur1.swap(RECUR1);
		saved_recur2.swap(RECUR2);
		saved_recur3.swap(RECUR3);
		saved_recur4.swap(RECUR4);

		saved_id.swap(ID);
		saved_record.swap(RECORD);
		saved_mv.swap(MV);
		saved_dict.swap(DICT);

		var::setprecision(saved_precision);

		// Decrease the perform/execute "level"
		LEVEL--;
	};

	// Do the perform
	/////////////////

	// CHAIN may be set by the performed library to execute a follow on SENTENCE/command_line
	CHAIN = "";

	SENTENCE = command_line;
	while (SENTENCE) {

		// Parse words using spaces into an FM delimited string leaving quoted phrases intact.
		// Spaces are used to parse fields.
		// Spaces and single quotes are preserved inside double quotes.
		// Spaces are double quotes preserved inside single quotes.
		// Backslashes and any character following a backslash (particularly spaces, double and single quotes and backslashes) are treated as non-special characters.
		COMMAND = SENTENCE.parse(' ');

		// Cut off OPTIONS from end of COMMAND if present
		// *** SIMILAR code in
		// 1. exofuncs.cpp exodus_main()
		// 2. exoprog.cpp  perform()
		// OPTIONS are in either (AbC) or {AbC} WITHOUT SPACES in the last field of COMMAND
		OPTIONS = COMMAND.field(FM, -1);
		if ((OPTIONS.starts("{") and OPTIONS.ends("}")) or (OPTIONS.starts("(") and OPTIONS.ends(")"))) {
			// Remove last field of COMMAND. TODO fpopper command or remover(-1)?
			COMMAND.cutter(-OPTIONS.len() - 1);
			// Remove first { or ( and last ) } chars of OPTIONS
			OPTIONS.cutter(1);
			OPTIONS.popper();
		} else {
			OPTIONS = "";
		}

		// Load the shared library file (always lowercase) TODO allow mixed case
		// creating a new object so that its member variables all start out unassigned.
		let libid = SENTENCE.field(" ", 1).lcase();
		std::string libname = libid.toString();
		if (!perform_callable_.initsmf(
											mv,
											libname.c_str(),
											/* Use the non-adorned factory function for library() not library(xxx)*/
											"exoprogram_createdelete_",
											// Force new each perform/execute
											true
											)) {
			USER4 ^= "perform() Cannot find shared library \"" + libname +
					 "\", or \"libraryinit()\" is not present in it.";
			// UNLIKELY throw VarError(USER4);
			// return "";
			break;
		}

		// Call the shared library exodus object's main function
		try {

			//ANS = perform_callable_.callsmf();
			var* retval = new var;
			*retval = perform_callable_.callsmf();

			// Detect corrupt return if we are lucky
			// If we are unlucky the corrupt data will look good
			// OK: var_typ will have only bottom 5 bits set
			// KO: var_typ will have other bits set
			// By throwing without deleting the new head var, we will avoid
			// segfault: free(): invalid pointer" when the var's std:string is "destructed"
			// but the memory will leak.
			if (retval->unassigned()) {
				retval->dump().errputl();
				restore_environment();
				UNLIKELY
				throw VarError("exoprog::perform corrupt result. perform only functions, not subroutines.");
			}

			// Since the returned var appears OK, use it.
			nrvo = std::move(*retval);

			// Since the returned var appears OK, delete it.
			// If retval was not actually returned by callsmf then we may get a segfault: free(): invalid pointer
			// from the fake var's std::string destructor
			delete retval;

			var::setlasterror("");
		}

		// TODO reimplement this
//		} catch (const VarUnconstructed&) {
//			// if return "" is missing then default ANS to ""
//			ANS = "";

		// Stop
		catch (const ExoStop& e) {
			// stop is normal way of stopping a perform
			// functions can call it to terminate the whole "program"
			// without needing to setup chains of returns
			// to exit from nested functions
			var::setlasterror("");
			nrvo = e.message;
		}

		// Abort
		catch (const ExoAbort& e) {
			var::setlasterror("ExoAbort:" ^ e.message);
			nrvo = "";
		}

		// AbortAll
		catch (const ExoAbortAll& e) {
			// similar to abort for the time being
			// maybe it should abort multiple levels of perform/execute?
			var::setlasterror("ExoAbortAll:" ^ e.message);
			nrvo = "";
		}

		//TODO Create a second version of this whole try/catch block
		//that omits catch (VarError) if EXO_DEBUG is set
		//so that gdb will catch the original error and allow backtracing there
		//Until then, use gdb "catch throw" as mentioned below.
		catch (const VarError& e) {
			//restore environment in case VarError is caught
			//in caller and the program resumes processing
			restore_environment();

			var::setlasterror(e.message);

			// Use gdb command "catch throw" to break at error line to get back traces there
			UNLIKELY
			throw;
		}

		// CHAIN is a kind of optional follow-on controlled by the library function
		// to perform another function after the first
		// Go round again if anything in CHAIN
		SENTENCE = CHAIN.move();

	} //

	// restore various environment variables
	restore_environment();

	return nrvo;
}

// xlate
var ExoProgram::xlate(in filename, in key, in fieldno_or_name, const char* mode) {

	// TODO implement additional MV argument

	// Key can be multivalued and will return a multivalued result
	var results = "";
	let nkeys = key.fcount(VM);

	let is_fieldno = fieldno_or_name.isnum();

	var dictfile;
	if (not is_fieldno) {
		if (not dictfile.open("dict." ^ filename.f(1))) {
			UNLIKELY
			throw VarError("ExoProgram::xlate(filename:" ^ filename ^ ", key:" ^ key ^ ",field:" ^ fieldno_or_name ^ ") - dict." ^ filename ^ " does not exist.");
		}
	}

	for (var keyn = 1; keyn <= nkeys; ++keyn) {

		let keyx = key.f(1, keyn);

		// If ordinary numeric or "" fieldno
		if (is_fieldno) {
			results.updater(keyn, keyx.xlate(filename, fieldno_or_name, mode));
		}

		// Otherwise handle non-numeric field_no ie dictionary field/column name
		else {

			// Get the whole record
			let record = keyx.xlate(filename, "", mode);

			// TODO what if key is multivalued?

			// Handle record not found and mode C
			if (*mode == 'C' && record == keyx) {
				results.updater(keyn, key);
				continue;
			}

			// Handle record not found and mode X
			if (record.empty()) {
				results.updater(keyn, "");
				continue;
			}

			// Use calculate() with four parameters
			var result =
				calculate(fieldno_or_name, dictfile, keyx, record);
			if (nkeys > 1)
				result.lowerer();
			results.updater(keyn, result);
		}

	}

	if (nkeys > 1)
		results.converter(_FM, _VM);
	//else
	//    sep = _RM;

	return results;
}

// calculate 4
var ExoProgram::calculate(in dictid, in dictfile, in id, in record, in mvno) {

	//dictid @ID/@id is hard coded to return ID
	//to avoid incessant lookup in main file dictionary and then defaulting to dict.voc
	//sadly this means that @ID cannot be customised per file
	//possibly amend the read cache to cache the dict.voc version for the main file
	//if (dictid == "@ID" || dictid == "@id")
	//	return ID;

	// How is it possible to swap when the argument variable is const?
	// Because libexodus provides a const swap that isnt const just for calculate()
	// to be able to work rapidly swapping globals in and out of place TEMPORARILY.
	// and all it to work with const arguments.
	// TODO replace with const_cast?

	// Save and reconfigure the relevent environment variables
	DICT.swap(dictfile);
	ID.swap(id);
	RECORD.swap(record);
	MV.swap(mvno);

	// Call the normal calculate function
	let result = calculate(dictid);

	// Restore the environment variables
	DICT.swap(dictfile);
	ID.swap(id);
	RECORD.swap(record);
	MV.swap(mvno);

	return result;
}

// calculate 1
var ExoProgram::calculate(in dictid) {
	// THISIS("var ExoProgram::calculate(in dictid)")
	// ISSTRING(dictid)

	// return ID^"*"^dictid;

	// Get the dictionary record so we know how to extract the correct field
	// or call the right library
	bool indictvoc = false;
	bool newlibfunc = false;
	std::string curr_dictid = std::string(dictid) + _RM + DICT.f(1).toString();
	if (cached_dictid_ != curr_dictid) {
		cached_dictid_ = "";
		newlibfunc = true;

		if (not DICT)
			UNLIKELY
			throw VarError("ExoProgram::calculate(" ^ dictid ^
						  ") DICT file variable has not been set");

		if (not cached_dictrec_.readc(DICT, dictid)) {

			// try other case
			var othercase_dictid = dictid.lcase();
			if (othercase_dictid == dictid)
				othercase_dictid = dictid.ucase();
			if (not cached_dictrec_.readc(DICT, othercase_dictid)) {

				// try dict.voc
				var dictvoc;  // TODO implement mv.DICTVOC to avoid opening
				if (not dictvoc.open("dict.voc")) {
baddict:
					UNLIKELY
					throw VarError("ExoProgram::calculate(" ^ dictid ^
								  ") dictionary record not in DICT " ^
								  DICT.f(1).quote() ^ " nor in dict.voc");
				}
				if (not cached_dictrec_.readc(dictvoc, dictid)) {
					// try other case
					if (not cached_dictrec_.readc(dictvoc, othercase_dictid)) {
						goto baddict;
					}
				}
				indictvoc = true;
			}
			// cache whichever dict record was found
			// regardless of file xxxxxxxx/voc and upper/lower case key
			// as if it was found in the initial file,key requested
			// this will save repeated drill down searching on every access.
			cached_dictrec_.updater(16, indictvoc);
			cached_dictrec_.writec(DICT, dictid);
		}
		cached_dictid_ = curr_dictid;

		//detect from the cached record if it came from dict.voc
		//so we can choose the libdict_voc if so
		indictvoc = cached_dictrec_.f(16);

	}

	let dicttype = cached_dictrec_.f(1);
	bool ismv = cached_dictrec_.f(4).starts("M");

	// F type dictionaries
	if (dicttype == "F") {

		// check field number is numeric
		let fieldno = cached_dictrec_.f(2);
		if (!fieldno.isnum())
			return "";

		// field no > 0
		if (fieldno) {
			if (ismv)
				return RECORD.f(fieldno, MV);
			else
				return RECORD.f(fieldno);

			// field no 0
		} else {
			let keypart = cached_dictrec_.f(5);
			if (keypart && keypart.isnum())
				return ID.field("*", keypart);
			else
				return ID;
		}
	} else if (dicttype == "S") {
		if (newlibfunc) {

			std::string libname;
			if (indictvoc)
				libname = "dict_voc";
			else
				libname = DICT.f(1).lcase().convert(".", "_").toString();

			// Try to get dict_callable from cache
			std::string callable_cachekey = dictid.lcase().toString() + "_" + libname;
			dict_callable_ = cached_dict_functions[callable_cachekey];

			// If not in cache
			if (!dict_callable_) {
				// var(cachekey).logputl("cachekey=");

				// Create a new dict_callable
				dict_callable_ = new Callable;

//				// Patch in the current exoenv
//				if (!dict_callable_->mv_)
//					dict_callable_->mv_ = (&mv);

				// Wire it up with the necessary ExoEnv mv
				// and locate the correct function object factory function in the library
				std::string factory_funcname = "exoprogram_createdelete_" + dictid.lcase().toString();
				if (!dict_callable_->initsmf(
												mv,
												libname.c_str(),
												factory_funcname.c_str(),
												// dont force new each call
												false
											)
				)
					UNLIKELY
					throw VarError("ExoProgram::calculate() Cannot find Library " +
								  libname + ", or function " +
								  dictid.lcase() + " is not present");

				// Cache it
				cached_dict_functions[callable_cachekey] = dict_callable_;

			}
		}

		//AREV does not do this.
		//in AREV, a multivalued dict may call a single valued dict that calls a multivalued dict.
		//and MV needs to be preserved over the set of calls.
		//Therefore all the following is cancelled.
		// for single valued fields, inform the called routine that MV is 0
		//int savedMV;
		//if (!ismv) {
		//	savedMV = MV;
		//	MV = 0;
		//}

		// return dict_callable_.calldict();
		// return ANS;

		const int saved_precision = var::getprecision();

		ANS = dict_callable_->callsmf();

		// restore the MV if necessary
		//if (!ismv)
		//	MV = savedMV;

        var::setprecision(saved_precision);

		return ANS;
	}

	UNLIKELY
	throw VarError("ExoProgram::calculate(" ^ dictid ^ ") " ^ DICT ^ " Invalid dictionary type " ^
				  dicttype.quote());
}

//// debug
//void ExoProgram::debug() const {
//
//	let reply;
//	std::clog << "debug():";
//	if (OSSLASH == "/")
//		asm("int $3");	//use gdb n commands to resume
//	else
//		var().debug();
//	return;
//}

// fsmg
//bool ExoProgram::fsmsg(in msg) const {
//	note(msg ^ var().lasterror());
//	return false;
//}

// decide 2
var ExoProgram::decide(in question, in options) const {
	var reply = "";
	return decide(question, options, reply);
}

// decide 4
var ExoProgram::decide(in questionx, in optionsx, out reply, const int defaultreply) const {

	// If default reply is 0 then there is no default
	// and pressing Enter returns "" and reply is set to 0

	// question can be multiline
	var question = questionx;
	question.converter(_ALL_FMS "|", "\n\n\n\n\n\n\n");

	let interactive = !SYSTEM.f(33);
	if (interactive)
		std::cout << var("========================================") << std::endl;

	std::cout << question << std::endl;

	//	let noptions = options.fcount(FM);
	var options = optionsx;
	if (not options)
		options = "Yes" _VM "No";
	options.converter(VM ^ "|", FM ^ FM);
	let noptions = options.fcount(FM);
	for (int optionn = 1; optionn <= noptions; optionn++) {
		if (optionn == defaultreply)
			std::cout << "*";
		else
			std::cout << " ";
		std::cout << optionn << ". " << options.f(optionn) << std::endl;
	}

inp:
	if (defaultreply)
		std::cout << " Please enter 1 - " << noptions << " or Enter to default or 0 to cancel.";
	else
		std::cout << " Please enter 1 - " << noptions;
	std::cout << std::endl;

	reply = defaultreply;

	if (interactive) {
		if (not reply.input("? ")) {}

		//entering ESC anywhere in the input causes "no response"
		if (reply.contains("\x1B"))
			reply = 0;

		//reply must be numeric in range
		if (!reply.isnum())
			goto inp;
	}

	//no input means use default which might be zero
	if (reply == "")
		reply = defaultreply;

	//reply must be integer
	reply = int(reply);

	//zero or empty response
	if (!reply)
		return "";

	//reply must be 0 to noptions
	if (reply < 0 || reply > noptions) {
		if (interactive)
			goto inp;
		UNLIKELY
		throw VarError(questionx ^ " Default reply must be 0-" ^ noptions);
	}

	return options.f(reply);
}

// esctoexzit
bool ExoProgram::esctoexit() const {

//	if (not var().keypressed())
	if (not var().hasinput())
		return false;

	//std::cout << "Press Esc again to cancel or any other key to continue " << std::endl;
	std::cout << " Paused. Continue? (Y/n) ";
	std::cout << std::flush;

	//wait for ever
	var key;
	if (not key.input()) {}

	//key.logputl("key=");
	//key.oconv("HEX").logputl("key=");

	return key.last().ucase() == "N";
}

//// otherusers
//var ExoProgram::otherusers(in /*param*/) {
//	std::cout << "ExoProgram::otherusers not implemented yet";
//	return var("");
//}
//
//// otherdatasetusers
//var ExoProgram::otherdatasetusers(in /*param*/) {
//	std::cout << "ExoProgram::otherdatausers not implemented yet";
//	return var("");
//}

// lockrecord 3
bool ExoProgram::lockrecord(in filename, io file, in keyx) const {
	let record;
	return lockrecord(filename, file, keyx, record);
}

// lockrecord 6
bool ExoProgram::lockrecord(in filename, io file, in keyx, in /*recordx*/, const int waitsecs0, const bool allowduplicate) const {

	// linemark
	// common /shadow.mfs/
	// shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3

	// nb case where we do not wish to wai
	// wait
	// 0 fail immediately (eg looking for next batch/nextkey)
	// 9 wait for 9 seconds
	//- wait for a default number of seconds
	// * wait infinite number of seconds
	// if index(file,'message',1) else de bug
	int waitsecs = waitsecs0;

	if (file.unassigned()) {
		if (not file.open(filename)) {
			call note(filename.quote() ^ " cannot be opened in LOCKRECORD " ^ keyx);
			this->abort();
		}
	}

lock:

	// Fail if global terminate or reload requested
	if (TERMINATE_req or RELOAD_req) {
		var("").errputl("Lock request for " ^ file.f(1).quote() ^ ", " ^ keyx.quote() ^ " cancelled because 'terminate' or 'reload' requested.");
		return false;
	}

	let locked = file.lock(keyx);
	if (locked || (allowduplicate && locked == "")) {

		// fail if unexpired persistent lock exists in LOCKS file
		// on the same connection as file

		var leaselocks;
		if (leaselocks.open("LOCKS", file)) {

			let filename_for_locks = (filename.assigned() && filename) ? filename : file.f(1);
			let lockfilekey = filename_for_locks ^ "*" ^ keyx;

			var lockrec;
			if (lockrec.read(leaselocks, lockfilekey)) {

				//current dos time
				//convert to Windows based date/time (ndays since 1/1/1900)
				//31/12/67 in rev date() format equals 24837 in windows date format
				var dostime = var().ostime();
				dostime = 24837 + var().date() + dostime / 24 / 3600;

				//remove lock if expired
				if (lockrec.f(1) <= dostime) {
					leaselocks.deleterecord(lockfilekey);
					lockrec = "";
				}

				//or release any absolute lock
				//and return indication of lock failure
				else {
					if (locked)
						file.unlock(keyx);

					// failure due to leaselock fails immediately regardless of waitsecs
					return 0;
				}
			}
		}

		return 1;
	} else {
		if (waitsecs) {

			if ((waitsecs0 - waitsecs) >  30)
	            var("").errputl("Locking file : " ^ file.f(1).quote() ^ " key : " ^ keyx.quote() ^ " secs : " ^ waitsecs ^ "/" ^ waitsecs0);

			var().ossleep(1000);
			waitsecs -= 1;
			goto lock;
		}
		return false;
	}

//#if __cpp_lib_unreachable >= 202202L 
//	std::unreachable();
//#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
	// TODO should this be a special system error since it cannot be caused by application programmer?
	throw VarError("EXODUS: Error in " ^ var(__PRETTY_FUNCTION__) ^ " Executing unreachable code.");
#pragma clang diagnostic push
//#endif
}

// unlockrecord 0 = unlock all
bool ExoProgram::unlockrecord() const {
	var xx;
	return unlockrecord("", xx, "");
}

// unlockrecord
bool ExoProgram::unlockrecord(in /*filename*/, io file0, in key) const {
	var file;
	if (file0.unassigned())
		file = "";
	else
		file = file0;

	if (file == "") {
		var().unlockall();
		return 1;
	}

	// remove persistentlock
	// common /shadow.mfs/
	// shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3 done by
	// shadow.mfs

	// remove actual lock
	file.unlock(key);

	return 1;
}

var ExoProgram::AT(const int columnno, const int rowno) const {

	// Hard coded for xterm at the moment
	// http://www.xfree86.org/current/ctlseqs.html

	// WARNING: Pick OS column and row numbering is 0 based
	// BUT in exodus we move to 1 based numbering to be consistent with
	// General terminal standards.

	// Columns and rows are 1 based.

	return "\x1B[" ^ var(rowno) ^ ';' ^ var(columnno) ^ 'H';
}

var ExoProgram::AT(const int columnno) const {

	// Columns are 1 based.

	// Move to column n - ESC[nG
	if (columnno > 0)
		return "\x1B[" ^ var(columnno) ^ 'G';

	// Move to the first column "\r"
	if (columnno == 0)
		// return "\x1b[G";
		return "\r";  // works on more terminals

	// Clear the screen and home the cursor - ESC[H ESC[J
	if (columnno == -1)
		return "\x1B[H\x1B[J";

	// Move the cursor to top left home - ESC[H
	if (columnno == -2)
		return "\x1B[H";

	// Clear from cursor to end of screen - ESC[J
	if (columnno == -3)
		return "\x1B[J";

	// Clear from cursor to end of line - ESC[K
	if (columnno == -4)
		return "\x1B[K";

	// Move cursor to the first column and clear to end of line
	if (columnno == -40)
		return "\r\x1B[K";

	return "";
}

// oconv
var ExoProgram::oconv(in input0, in conversion) {

	// call user conversion routine
	// almost identical code in var::oconv and var::iconv
	// ENSURE synchronised if you change it

	var result = input0;

	//nothing in, nothing out
	//unfortutely conversions like L# do convert ""
	//if (result.empty())
	//	return result;

	var ptr = 1;
	var delimiter;
	do {

		// var subconversion=conversion.remove(ptr,delimiter);
		var subconversion = conversion.substr2(ptr, delimiter);

		// EITHER 1. Call standard conversion methods
		//if (subconversion[1] != "[") {
		if (!subconversion.starts("[")) {
			result = result.oconv(subconversion);
		}

		else
		// OR 2. Call custom conversion routines
		// unlike pickos, custom routines are reponsible
		// for processing and returning multiple values
		{

			// var("*").logput();
			// return 1;

			//for speed dont call custom conversions on empty strings
			//nothing in, nothing out
			//Unfortunately [taghtml,td] produces output from nothing
			//if (result.empty())
			//	continue;

			//remove brackets
			subconversion.substrer(2);
			if (subconversion.ends("]"))
				subconversion.popper();

			//determine the function name
			let functionname = subconversion.field(",").lcase();

			// extract any params
			let mode = subconversion.b(functionname.len() + 2);

			var outx;

			//custom function "[NUMBER]" actually has a built in function
			if (functionname == "number") {
				gosub exoprog_number("OCONV", result, mode, outx);
			}

			//custom function "[DATE]" will use the standard exodus var oconv
			else if (functionname == "date") {
				gosub exoprog_date("OCONV", result, mode, outx);
			}

			// Custom function "[CAPITALISE]" will use the standard exodus var oconv
			// DEPRECATE and change to use oconv "MRT" available since 2025-03-05
			else if (functionname == "capitalise") {
//				outx = capitalise(result, mode, "");
				outx = result.tcase();

			} else {

				// set the function name
				ioconv_custom = functionname;

				// wire in the current environment
				//ioconv_custom.mv_ = &mv;

				// and call it
				//call ioconv_custom("OCONV", result, mode, output);

				//call it once per field (any field mark RM-ST are preserved)
				var posn = 1;
				var ifield, ofield, delim;
				outx = "";
				while (true) {

					ifield = result.substr2(posn, delim);

					call ioconv_custom("OCONV", ifield, mode, ofield);

					outx ^= ofield;

					if (not delim)
						break;

					outx ^= var().chr(RM.ord() + 1 - delim);
				}
			}
			result = outx;
		}
	} while (delimiter);

	return result;
}

// iconv
var ExoProgram::iconv(in input, in conversion) {

// TODO
//	Value	Description
//	0	Successful conversion.
//	1	The data in string cannot be converted using conversion.
//	2	conversion is not a recognized conversion specification.

	// call user conversion routine
	// almost identical code in var::oconv and var::iconv
	// ENSURE synchronised if you change it

	var result = input;
	var ptr = 1;
	var delimiter;
	do {

		// var subconversion=conversion.remove(ptr,delimiter);
		let subconversion = conversion.substr2(ptr, delimiter);

		// or call standard conversion methods
		if (not subconversion.starts("[")) {

			result = result.iconv(subconversion);

		// or call custom conversion routines
		} else {

			//determine the function name
			let functionname = subconversion.cut(1).field(",", 1).field("]", 1).lcase();

			// extract any params
			let mode = subconversion.field(",", 2, 9999).field("]", 1);

			var outx;

			//custom function "[NUMBER]" actually has a built in function
            if (functionname == "number") {
                gosub exoprog_number("ICONV", result, mode, outx);
			}
			//custom function "[DATE]" actually has a built in function
            else if (functionname == "date") {
                gosub exoprog_date("ICONV", result, mode, outx);
			}
			//custom function "[CAPITALISE]" DEPRECATE and replace with 
            else if (functionname == "capitalise") {
                //gosub capitalise(result, mode, outx);
				//result = result;
				outx = result;
            } else {

				// set the function name
				ioconv_custom = functionname;

				// wire up the current environment
				//ioconv_custom.mv_ = (&mv);

				// and call it
				call ioconv_custom("ICONV", result, mode, outx);
			}

			result = outx;
		}
	} while (delimiter);

	return result;
}


void ExoProgram::getdatetime(out user_date, out user_time, out system_date, out system_time, out UTC_date, out UTC_time) {

	//see CHANGETZ which seems to go through all times (dates?) and changes them
	//this should be done ONCE to standardise on gmt/utc really
	//really need a DBTZ to determine datetime for storage in the database

	var tt;//num

	//ensure time and date are taken in the same day
	//by ensuring time is not less than time1
	//which could happen over midnight
	while (true) {
		let system_time0 = var().time();
		system_date = var().date();
		system_time = var().time();
		///BREAK;
		if (system_time ge system_time0) break;
	}//loop;

	//no timezone info
	if (not TZ) {
		user_date = system_date;
		user_time = system_time;
		//assume system is on gmt/utc or database contains non-gmt datetimes
		UTC_date = system_date;
		UTC_time = system_time;
		goto exit;
	}

	//@sw<1> is the ADJUSTMENT to get user time from server time, therefore add
	user_time = system_time;
	user_date = system_date;
	tt = TZ.f(1);
	if (tt) {
		user_time += tt;
		if (user_time ge 86400) {
			//assume offset cannot be more than 24 hours!
			user_date = system_date + 1;
			user_time -= 86400;
		} else if (user_time lt 0) {
			//assume offset cannot be less than 24 hours!
			user_date = system_date - 1;
			user_time += 86400;
		}
	}

	//@sw<2> is the difference from gmt/utc to server time, therefore subtract
	UTC_time = system_time;
	UTC_date = system_date;

	//following is irrelevent until we support user tz when server tz ISNT gmt/utc
	//
	//remove server tz to get gmt/utc. if server is ahead of gmt then serv tz is +
	tt = TZ.f(2);
	if (tt) {
		UTC_time -= tt;
		if (UTC_time ge 86400) {
			//assume tz cannot be more than 24 hours!
			UTC_date = system_date + 1;
			UTC_time -= 86400;
		} else if (UTC_time lt 0) {
			//assume tz cannot be less than 24 hours!
			UTC_date = system_date - 1;
			UTC_time += 86400;
		}
	}

/////
exit:
/////
	if (SENTENCE eq "GETDATETIME") {
		var msg = "User:   " ^ user_date.oconv("D");
		msg ^= " " ^ user_time.oconv("MTH");
		msg ^= FM ^ "Server: " ^ system_date.oconv("D");
		msg ^= " " ^ system_time.oconv("MTH");
		msg ^= FM ^ "GMT/UTC:" ^ UTC_date.oconv("D");
		msg ^= " " ^ UTC_time.oconv("MTH");
		call note(msg);
	}

	return;
}

//var ExoProgram::timedate2(in user_date0, in user_time0, in glang) {
var ExoProgram::timedate2() {

	//caserevised*

	var user_date;
	var user_time;
	var x3;
	var x4;
	var x5;
	var x6;

	//use parameters only if both are provided
//	if (user_time0.unassigned()) {
		call getdatetime(user_date, user_time, x3, x4, x5, x6);
//	} else {
//		user_date = user_date0;
//		user_time = user_time0;
//	}

	var temp = "";
	if (not temp) {
		temp = "MTH";
	}
	temp = oconv(user_time, temp);
	if (temp.starts("0")) {
		temp.cutter(1);
	}
//	if (glang) {
//		if (temp.ends("AM")) {
//			temp.paster(-2, 2, glang.f(16));
//		}
//		if (temp.ends("PM")) {
//			temp.paster(-2, 2, glang.f(17));
//		}
//	}

	if (DATEFMT eq "") {
		DATEFMT = "D/E";
	}
	temp.prefixer(oconv(user_date, "[DATE,*4]") ^ " ");

	return temp;
}


// elapsedtimetext 1 - from program start/TIMESTAMP
var ExoProgram::elapsedtimetext() const {
	return elapsedtimetext(TIMESTAMP, var().ostimestamp());
}

// elapsedtimetext 2 - given two timestamps
var ExoProgram::elapsedtimetext(in timestamp1, in timestamp2) const {

	var text = "";

	var secs = (timestamp2 - timestamp1) * 86'400;

	let weeks = (secs / 604800).floor();
	secs -= weeks * 604800;

	let days = (secs / 86400).floor();
	secs -= days * 86400;

	let hours = (secs / 3600).floor();
	secs -= hours * 3600;

	let minutes = (secs / 60).floor();
	secs -= minutes * 60;

	if (weeks) {
		text.appender(weeks ^ " week");
		if (weeks ne 1) {
			text ^= "s";
		}
	}
	if (days) {
		text.appender(FM_, days, " day");
		if (days ne 1) {
			text ^= "s";
		}
	}
	if (hours) {
		text.appender(FM_, hours, " hour");
		if (hours ne 1) {
			text ^= "s";
		}
	}
	if (minutes) {
		text.appender(FM_, minutes, " min");
		if (minutes ne 1) {
			text ^= "s";
		}
	}
	if (not(hours) and (minutes < 5)) {
		if (secs) {
			if (minutes or (secs - 10 > 0)) {
				secs = secs.round(0);
			} else {
				//secs = (secs.oconv("MD40P")) + 0;
				secs = secs.round(6) + 0;
				if (secs.starts(".")) {
					secs.prefixer("0");
				}
			}
			if (secs >= 0.1) {
				text.appender(FM_, secs, " sec");
				if (secs ne 1) {
					text ^= "s";
				}
			} else if (not minutes and not hours and not days and not weeks) {
zero:
				if (not text) {
					//text.appender(FM_, "< 1 ms");
					var ms = secs * 1000;
					if (ms < 0.1) {
						var μs = ms * 1000;
						if (timestamp1.toDouble() == timestamp2.toDouble()) {
							text = "0 secs";
						} else if (μs < 0.1) {
								var ns = ms * 1'000'000;
								text = ns.round(3) ^ " ns";
						} else
							text = μs.round(3) ^ " μs";
					} else
						text = ms.round(3) ^ " ms";
				}
			} else {
				//text.appender(FM_, "exactly");
			}
		} else {
			if (not minutes and not hours and not days and not weeks) {
				goto zero;
			}
			//text.appender(3, "exactly");
		}
	}

//	text.replacer(FM ^ FM ^ FM, FM);
//	text.replacer(FM ^ FM, FM);
	text.trimmer(_FM);
	text.replacer(_FM, ", ");

	return text;
}

// number
	//function main(in type, in input0, in ndecs0, out outx) {
	//c xxx in,in,in,out

var ExoProgram::exoprog_date(in type, in in0, in mode0, out outx) {

	// Should really be sensitive to timezone in @TZ but that requires time
	// "time" is basically users time without a timezone
	// "date" is basically users date without a timezone
	// Use DATETIME for TZ sensitive conversion.

	// The job of "[DATE,arg]" is to append options to DATEFMT or
	// if arg starts "D" then arg is used to replace DATEFMT but any E option from DATEFMT is retained.

	var inx = in0;
	var mode = mode0;

	// NINO
	if (inx eq "") {
		outx = "";
		return 0;
	}

	// "[DATE" has a special option "*" to remove spaces
	// Equivalent to new "D" oconv option "Z"
//	let nospaces = mode.contains("*");
//	if (nospaces) {
//		mode.converter("*", "");
//	}
//	mode.converter("*", "Z");

	bool raw_D = false;

	if (mode) {

		if (mode.starts("D")) {

			// Dont use DATEFMT except for its E if present
			raw_D = true;

			if (DATEFMT.contains("E"))
				// Copy DATEFMT's E if present
				mode ^= "E";

		} else {

			// Append to DATEFMT

			// Suppress any 2 digit year in DATEFMT
			// Can still get 2 digit year by using "DATE,2]"
//			mode = DATEFMT.replace("2", "") ^ mode;
			mode = DATEFMT ^ mode;

//			// 4 means force 4 digit year regardless of DATEFMT
//			if (mode eq "4") {
//				mode = DATEFMT;
//				// The intent here is to replace a "2" in DATEFMT D2/E
//				// and the web service initialises DATEFMT to D2xxx
//				// srv/initcompany.cpp:    DATEFMT            = "D2/E";
//				// srv/initcompany.cpp:            DATEFMT = "D2/E";
//				// srv/initcompany.cpp:            DATEFMT = "D2/E";
//				// srv/initcompany.cpp:            DATEFMT = "D2-E";
//				// srv/initcompany.cpp:            DATEFMT = "D2E";
//				// srv/initcompany.cpp:            DATEFMT = "D2";
//				// srv/initcompany.cpp:            DATEFMT = "D2/";
//				// srv/initcompany.cpp:            DATEFMT = "D2-";
//				// srv/initcompany.cpp:            DATEFMT = "D2E";
//				// srv/initcompany.cpp:            DATEFMT = "D2";
//				// BUT the default initialisation of DATEFMT in exoprogram is to "D/E"
//				// but when DATEFMT changed to D/E it ended up
//				// overwriting the "/" and reverting to a alphabetic month
//				// resulting in [DATE,4*] [DATE,*4] and [DATE,4]
//				// outputing dates like "9JAN2020" which are at least unambiguous.
//				// Solution is to remove "2" if it is present since 4 is the default
//				// The alternative is to change DATEFMT initialisation to D4/E
//	//			mode.paster(2, 1, "4");
//				if (mode.contains("2"))
//					mode.replacer("2", "");
//			}

		}

	} else if (DATEFMT) {
		mode = DATEFMT;

	} else {
//		// Default to 2 digit year ?
//		mode = "D2/E";

		// Default to alpha month for output conversion
		// and international year for input conversion of numeric dates
		mode ="DE";    // "09 JAN 2020" 11 chars
//		mode ="DE!";   //   "09JAN2020"  9 chars
//		mode ="DEZ!";  //   " 9JAN2020"  9 chars
//		mode ="DEZZ!"; //    "9JAN2020"  8 or 9 chars
	}

	//status=0
	if (type eq "OCONV") {

		// [DATE always replaced leading zeros with spaces
		// but we will not include it for hard coded D conversions
		if (not raw_D)
			mode ^= "Z";

		// * is a [DATE option that means Z option
		mode.converter("*", "Z");

		// 1. if 3 or more digits
		//dont oconv 1 or 2 digits as they are probably day of month being converted
		// to proper dates
		//IF len(inx) gt 2 and inx MATCHES '0N' OR inx MATCHES '"-"0N' OR inx MATCHES '0N.0N' THEN
		if (inx.len() gt 2 and inx.match("^\\d*$")) {
			goto ok;
		}

		// 2. "-" or "-digits"
		// 3. "digits.digits" or ".digits" or "digits."
		if (inx.match("^-\\d*$") or inx.match("^\\d*\\.\\d*$")) {
ok:
			//language specific (date format could be a pattern in lang?)
			if (mode eq "L") {

				throw VarNotImplemented("oconv(" ^ mode0 ^ ") and \"L\"");

////				let tt = inx.oconv("D4/E");
////				let mth = glang.f(2).field("|", tt.field("/", 2));
////				let day = tt.field("/", 1);
////				let year = tt.field("/", 3);
////				//if 1 then
////				outx = day ^ " " ^ mth ^ " " ^ year;
////				//end else
////				// outx=mth:' ':day:' ':year
////				// end
//				outx = inx.oconv("D");

			} else {

				// Do a basic date conversion
				// Assuming 00/00/0000?
				outx = oconv(inx, mode);

//				// Always replace leading zeros with spaces
//				// (Equivalent to Z option)
//				// leading 0 becomes a space
//				if (outx.starts("0")) {
//					outx.paster(1, 1, " ");
//				}
//
//				// 4th char 0 becomes a space
//				// This is buggy since 4th digit of leading year 2000 will be removed
//				if (outx.at(4) eq "0") {
//					outx.paster(4, 1, " ");
//				}

//				// This option is no longer the default
//				// Always remove sepchar around alpha month
//				// (equivalent to ! option prefixed and over)
//				// dd/XXX/yyyy -> ddAAAyyyy
//				if (outx.b(4, 3).match("^[A-Za-z]{3}$")) {
//					// remove 3rd and 7th chars
//					outx.paster(7, 1, "");
//					outx.paster(3, 1, "");
//				}

//				// Option to remove spaces
//				// Equivalent to ZZ option
//				if (nospaces) {
//					outx.converter(" ", "");
//				}

			}

		} else {
			// Cant convert if not three or more digits or digits with a decimal point somewhere
			outx = inx;
		}

	} else if (type eq "ICONV") {

		// 0-31 is converted to dd ^ MMM YYYY
		if (inx.match("^\\d*$") and inx le 31) {
			inx ^= var().date().oconv("D").b(4, 9);
		}

		// Ordinary D conversion which may or may not fail
		outx = iconv(inx, mode);

	}

	return 0;
}

// number
var ExoProgram::exoprog_number(in type, in input0, in ndecs0, out outx) {

	outx = "";

	// oconv only
	var zz = "";
	var ndecs = ndecs0;
	if (ndecs.contains("Z")) {
		ndecs.converter("Z", "");
		zz = "Z";
	}

	// Remove all thousands separators (comma or dot)
	// depending on BASEFMT
	// Also remove all spaces
	var input;
	if (BASEFMT.starts("MC")) {
		input = input0.convert(". ", "");
	} else {
		input = input0.convert(", ", "");
	}

	if (type == "ICONV") {

		var reciprocal = 0;
		if (input.starts("/")) {
			reciprocal = 1;
			input.cutter(1);
		} else {
			if (input.starts("1/")) {
				reciprocal = 1;
				input.cutter(2);
			}
		}

		// NINO
		if (input.empty()) {
			STATUS = "";
			outx = "";
			return 0;
		}

		// Split off any trailing currency code/unit
		var unitx;
		outx = amountunit(input, unitx);

		// Only handle numerics
		if (not outx.isnum()) {
			outx = "";
			STATUS = 2;
			return 0;
		}

		// Determine ndecs

		// a. ndecs = ""
		if (ndecs == "") {

			if (DICT) {
				// Get from RECORD via dictionary
				// Could be anything but normally 1, 2 or 3 for currency decimal places
				ndecs = calculate("NDECS");
			} else {
				// Use BASEFMT
				// Cannot be negative
				ndecs = BASEFMT.at(3);
			}
			// Fall back to 2 decimals
			if (ndecs == "") {
				ndecs = 2;
			}
		}

		// b. ndecs "*" or "X"
		// [NUMBER,X] means no decimal place conversion to be done
		else if (ndecs == "*" or ndecs == "X") {
			// Cannot be negative.
			// Can be > 9
			ndecs = outx.field(".", 2).len();
		}

		// c. ndecs = "BASE"
		else if (ndecs == "BASE") {
			// Cannot be negative
			ndecs = BASEFMT.at(3);
			// Fall back to 2 decimals
			if (ndecs == "") {
				ndecs = 2;
			}
		}

		// d. ndecs 0 1 2 3 etc.
		else {
			// Check numeric below
			// Could be negative.
		}

		// ndecs must be >= 0
		// Only dict NDECS could return negative so we will not check for that since it still has some meaning
		if (!ndecs.isnum())
			throw VarNonNumeric("In iconv([NUMBER," ^ ndecs0 ^ "]. ndecs: " ^ ndecs.quote() ^ " must be numeric.");

		// Perform the reciprocal if requested
		if (reciprocal and outx) {
//			outx = ((1 / outx).oconv("MD90P")) + 0;
			outx = 1 / outx;
		}

		// Round to the requested ndecs
//		let fmtx = "MD" ^ ndecs ^ "0P";
//		outx = outx.oconv(fmtx);
		outx = outx.round(ndecs);

		// Restore the trailing currency code/unit
		outx ^= unitx;
		STATUS = "";

		return 0;
	}

	//oconv
	//////

	// ndecs can be "ndecs_out,ndecs_shift__left" like MD20
	let point_shift_left = ndecs.field(",", 2);
	ndecs = ndecs.field(",", 1);

	// Set to 2 if any cannot be converted because non-numeric
	STATUS = "";

	// Loop through fields, values, subvalues etc. are found in the input
	var posn = 1;
	while (true) {

		// Get one field, value, subvalue etc.
		var delim;
		var input1 = input.substr2(posn, delim);

		if (not input1.empty()) {

			// Leading plus may be reinstated on output
			let plus = input1.first() == "+" ? "+" : "";
			if (plus)
				input1.cutter(1);

//			let temp = input1;
//			temp.converter("0123456789-.", "            ");
//			let numlen = input1.len() - temp.trimfirst().len();
//			let unitx = input1.b(numlen + 1, 99);
//			let numx = input1.first(numlen);
			var unitx;
			var numx = amountunit(input1, unitx);

			if (not numx.isnum()) {
				// Cant convert so output cleaned up number
				outx ^= plus ^ input1;
				STATUS = 2;
			}
			else {

				// Empty numx with unitx is treated as 0
				if (numx.empty())
					numx = 0;

				// Work out required fmt code

				if (point_shift_left) {
					numx = input1 / var(10).pwr(point_shift_left);
				}

				if (numx.contains("E-")) {
					if (numx.isnum()) {
						numx = numx.oconv("MD90P");
					}
				}

				var fmtx;
				if (ndecs.len() == 1 and ndecs.isnum()) {
					// Format to a given number of decimal places.
					fmtx = BASEFMT.first(2) ^ ndecs ^ "0,";
				}
				else if (ndecs == "" or ndecs == "X" or ndecs == "*") {
					// Retain previous number of decimal places.
					fmtx = BASEFMT.first(2) ^ numx.field(".", 2).len() ^ "0,";
				} else {
					// Default to BASEFMT
//				else if (ndecs == "BASE") {
					fmtx = BASEFMT;
				}

				// Do the required formatting
				var output1 = oconv(numx, fmtx ^ zz);

				// Restore trailing currency code/unit
				output1 ^= unitx;

				// Add leading 0 to prevent leading . or ,
				if (not output1.empty() and ".,"_var.contains(output1.first())) {
					output1.prefixer("0");
				}

				// Accumulate in output
				// Restore any leading + prefix
				outx ^= plus;
				outx ^= output1;
			}

		} // input1 not empty

		// if no trailing delimiter then we are done
		if (not delim)
			break;

		// Append whatever trailing delimiter was found when extracting the current amount
		outx ^= var().chr(RM.ord() + 1 - delim);

	}  //loop next field, subvalue etc.

//	STATUS = "";
	return 0;
}

// sortarray
void ExoProgram::sortarray(io array, in fns, in orderby0) {
	//function main(io array, in fns=0, in orderby0="") {
	//c sys io,0,""

	//sorts one or more parallel mved fields in a record

	//array is the record which the field(s) to be sorted are present
	//fns is a mv array of parallel field numbers to be sorted
	//order is D or A (can be repeated to sort for multiple sorts per fn)
	//eg AA would sort the first two fns ascending
	//justification is L or R for left (text) or right (numberic)

	//MULTIPLE SORTS NOT SUPPORTED CURRENTLY
	//MULTIPLE FIELDS WILL BE SORTED IN PARALLEL ON THE FIRST FIELD ONLY

	var orderby;
	var fn;
	let success;
	var newvn;

	if (orderby0.unassigned()) {
		orderby = "AL";
	} else {
		orderby = orderby0;
	}

	let nfns = fns.count(VM) + 1;

	let sortfn = fns.f(1, 1);
	let unsorted = array.f(sortfn);
	var sorted = "";

	//count values in sorted and parallel fields
	var nv = unsorted.fcount(VM);
	for (var fnn = 2; fnn <= nfns; ++fnn) {
		fn = fns.f(1, fnn);
		let othervalues = array.f(fn);
		let nv2 = array.f(fn).fcount(VM);
		if (nv2 > nv)
			nv = nv2;
	} //fnn;

	//insert into a new array without other fields for speed
	var newarray = "";
	for (var vn = 1; vn <= nv; ++vn) {
		let value = unsorted.f(1, vn);
		if (not(sorted.locateby(orderby, value, newvn))) {
			{
			}
		}
		sorted.inserter(1, newvn, value);

		//insert any parallel fields
		for (var fnn = 2; fnn <= nfns; ++fnn) {
			fn = fns.f(1, fnn);
			let othervalue = array.f(fn, vn);
			newarray.inserter(fn, newvn, othervalue);
		}	//fnn;

	}	//vn;

	array.updater(sortfn, sorted);

	//put any parallel fields back into the original array
	for (var fnn = 2; fnn <= nfns; ++fnn) {
		fn = fns.f(1, fnn);
		array.updater(fn, newarray.f(fn));
	}  //fnn;

	return;
}

// invertarray
var ExoProgram::invertarray(in input, bool padded /*=false*/) {

	// not forced
	// TRACE: inp = "a^1]2]3]4^x"
	// TRACE: out = "a]1]x^]2^]3^]4"

	// force
	// TRACE: inp = "a^1]2]3]4^x"
	// TRACE: out = "a]1]x^]2]^]3]^]4]"

	var result = "";
	int maxvn = 0;
	int fn = 0;
	for (var fx : input) {
		fn++;
		int vn = 0;
		dim vs = fx.split(_VM);
		for (var vx : vs) {
			vn++;
			if (not vx.empty()) {
				result(vn, fn) = vx;
			}
		}
		if (vn > maxvn)
			maxvn = vn;
	}

	// Optionally pad out so all fields have the same number of values
	if (padded) {
		for (int vn = 1; vn <= maxvn; vn++) {
			result(vn, fn) = result(vn, fn);
		}
	}

	return result;
}

// amountunit 1 arg
var ExoProgram::amountunit(in input0) {
	var unitcode;
	return amountunit(input0, unitcode);
}

// amountunit 2 arg
var ExoProgram::amountunit(in input0, out unitx) {

	// TODO Consider runtime Non-numeric or other error if amount or unit is missing
	// TODO Consider runtime error (Non-numeric?) if amount is not numeric

	// New method based on finding the last digit. Doesnt allow digits in unit codes.

	// Input            Amount        Unit
	// -----            ------        ----
	// 1.23456E-6USD -> 1.23456E-6    USD
	// USD              ""            USD
	// 1                1             ""

	// 123.45e7USD
	// 123.45e+07USD
	// 123.45e-07USD
	// 123.45M3 cubic meters TODO
	// 123.45e10M3 cubic meters TODO
	// 123e10 means e+10 and no unit TODO

	std::string_view sv = input0;

    for (auto it = sv.rbegin(); it != sv.rend(); ++it) {
        if (*it <= '9' and *it >= '0') {

			// it - s.rbegin()
			// 123456 -> 0
			// 123456XXX -> 3

			auto pos = sv.size() - std::size_t(it - sv.rbegin());
			unitx = sv.substr(pos);
			return sv.substr(0, pos);
        }
    }

	// All unit no digits
	unitx = sv;
	return "";

}

// simd implementation is 3-4x slower
//#include <cstring>
//#include <immintrin.h>
//#include <string_view>
//#include <iostream>
//#include <cstdint>
//
//using SV = std::string_view;
//
//inline SV split_simd(SV s, SV& unit) {
//    constexpr size_t SSE_SIZE = 16;
//
//    if (s.empty()) {
//        unit = s;
//        return "";
//    }
//
//    if (s.size() <= SSE_SIZE) {
//        alignas(16) char buffer[SSE_SIZE] = {0};
//        std::memcpy(buffer, s.data(), s.size());
//        __m128i chars = _mm_load_si128(reinterpret_cast<const __m128i*>(buffer));
//
//        __m128i min_digit = _mm_set1_epi8('0');
//        __m128i max_digit = _mm_set1_epi8('9');
//        __m128i lt_0 = _mm_cmplt_epi8(chars, min_digit);
//        __m128i gt_9 = _mm_cmpgt_epi8(chars, max_digit);
//        __m128i not_digit = _mm_or_si128(lt_0, gt_9);
//        __m128i is_digit = _mm_andnot_si128(not_digit, _mm_set1_epi8(-1));
//
//        uint16_t mask = _mm_movemask_epi8(is_digit);
//
//        if (mask == 0) {
//            unit = s;
//            return "";
//        }
//
//        size_t pos = 31 - __builtin_clz(static_cast<uint32_t>(mask));
//        if (pos >= s.size()) {
//            unit = s;
//            return "";
//        }
//
//        unit = s.substr(pos + 1);
//        return s.substr(0, pos + 1);
//    }
//
//    for (auto it = s.rbegin(); it != s.rend(); ++it) {
//        if (*it <= '9' && *it >= '0') {
//            auto pos = s.size() - std::size_t(it - s.rbegin());
//            unit = s.substr(pos);
//            return s.substr(0, pos);
//        }
//    }
//    unit = s;
//    return "";
//}
//
//int main() {
//    auto test = [](SV s) {
//        SV unit;
//        SV amount = split_simd(s, unit);
//        std::cout << "Input: \"" << s << "\" -> Amount: \"" << amount << "\", Unit: \"" << unit << "\"\n";
//    };
//
//    test("123abc");
//    test("abc");
//    test("12345");
//    test("");
//    test("12abc34de");
//    return 0;
//}

// clang-format off

ExoExit     ::ExoExit(in errmsg)     : message(errmsg) {}
ExoStop     ::ExoStop(in errmsg)     : message(errmsg) {}
ExoAbort    ::ExoAbort(in errmsg)    : message(errmsg) {}
ExoAbortAll ::ExoAbortAll(in errmsg) : message(errmsg) {}

[[noreturn]] void ExoProgram::stop(in errmsg)     const {throw ExoStop(errmsg);}
[[noreturn]] void ExoProgram::abort(in errmsg)    const {throw ExoAbort(errmsg);}
[[noreturn]] void ExoProgram::abortall(in errmsg) const {throw ExoAbortAll(errmsg);}

// clang-format on

}  // namespace exo
