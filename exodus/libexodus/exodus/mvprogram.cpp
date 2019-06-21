#include <exodus/mvprogram.h>
#include <exodus/exodusmacros.h> //coding style is like application programming eg USERNAME not mv.USERNAME

// putting various member functions into all exodus programs allows access to the mv environment
// variable which is also available in all exodus programs.

//#include <unordered_map>

namespace exodus
{

// constructor with an mvenvironment
DLL_PUBLIC
ExodusProgramBase::ExodusProgramBase(MvEnvironment& inmv) : mv(inmv)
{
	cache_dictid_ = "";
	cache_perform_libid_ = "";
	dict_exodusfunctorbase_ = NULL;
}

// destructor
DLL_PUBLIC
ExodusProgramBase::~ExodusProgramBase(){};

bool ExodusProgramBase::select(const var& sortselectclause)
{
	CURSOR.r(10,"");

	if (!CURSOR.select(sortselectclause))
		return false;

	//secondary sort/select on fields that could be calculated by the database

	//any calculated fields pending secondary sort/select are stuffed in a(10)
	var calc_fields=CURSOR.a(10).raise();
	if (!calc_fields)
		return true;

	CURSOR.r(10,"");

	//ONLY TEST MATERIALS FOR NOW
	//if (!calc_fields.ucase().index("MATERIALS"))
	//	return true;

	//debug
	//calc_fields.convert(FM^VM^SM,"   ").outputl("calc=");

	var sortselectclause2=sortselectclause;

	var dictfilename=calc_fields.a(5,1);

	//debugging
	var calc_fields_file="";
	calc_fields_file.open("calc_fields");

	//prepare to create a temporary sql table
	//DROP TABLE IF EXISTS SELECT_TEMP;
	//CREATE TABLE SELECT_TEMP(
	// KEY TEXT PRIMARY KEY,
	// EXECUTIVE_CODE TEXT)
	var temptablename="SELECT_TEMP_CURSOR_" ^ CURSOR.a(1);
	var createtablesql = "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
	createtablesql ^= "CREATE TEMPORARY TABLE " ^ temptablename ^ "(\n";
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
	int nfields=calc_fields.a(1).dcount(VM);
	dim dictids(nfields);
	dim opnos(nfields);
	dim values(nfields);
	dim values2(nfields);
	for (int fieldn=1;fieldn<=nfields;++fieldn) {

		//dictids

		var dictid=calc_fields.a(1,fieldn);

		//add colons to the end of every calculated field in the sselect clause
		//so that 2nd stage select knows that these fields are available in the
		//temporary parallel file
		sortselectclause2.replacer("\\b" ^ dictid ^ "\\b",dictid ^ ":");

		dictid.converter(".","_");
		dictids(fieldn)=dictid;
		var sqlcolid=dictid^"_calc";

		//ops

		//turn ops into numbers for speed
		var op=calc_fields.a(2,fieldn);
		var opno;
		if (! op)
			opno=0;
		else if (not var("= <> > < >= <= ~ ~* !~ !~* >< >!<").locateusing(" ", op, opno))
			throw MVException(op.quote() ^ " unknown op in sql select");
		opnos(fieldn)=opno;

		//values

		var value=calc_fields.a(3,fieldn).unquote();
		if (dictid.substr(-4,4)=="DATE")
			value=iconv(value,"D");
		values(fieldn)=value;

		var value2=calc_fields.a(4,fieldn).unquote();
		if (dictid.substr(-4,4)=="DATE")
			value2=iconv(value2,"D");
		values2(fieldn)=value2;

		//sql temp table column
		createtablesql ^= " " ^ sqlcolid ^ " TEXT,";

		//sql insert column
		baseinsertsql ^= sqlcolid ^ ",";

		//debug
		//dictid.outputt();
		//op.outputt();
		//value.outputt();
		//var("").outputl();

		//debug
		if (calc_fields_file)
		{
			var key=dictfilename^"*"^dictid;
			var rec=sortselectclause^FM^op^FM^value^FM^value2;
			rec.write(calc_fields_file,key);
			key.outputl("written to calc_fields ");
		}
	}

	if (baseinsertsql[-1] == ",") {

		baseinsertsql.splicer(-1,1,")");
		baseinsertsql ^= " VALUES (";

		createtablesql.splicer(-1,1,")");
		//createtablesql.outputl();

		//create the temporary table
		CURSOR.sqlexec(createtablesql);
	}
	else
		baseinsertsql="";

	//open the dictionary
	if (dictfilename.substr(1,5).lcase()!="dict_")
		dictfilename="dict_"^dictfilename;
	if (!DICT.open(dictfilename)) {
		throw MVDBException(dictfilename.quote() ^ " cannot be opened");
	}

	int maxnrecs=calc_fields.a(6);
	int recn=0;

nextrecord:
	while(CURSOR.readnextrecord(RECORD,ID,MV)) {

		bool ok=true;

		var insertsql=baseinsertsql ^ ID.squote() ^ ",";

		for (int fieldn=1;fieldn<=nfields;++fieldn) {

			var value=calculate(dictids(fieldn));

			//debug
			//value.outputl(dictids(fieldn) ^ " value=");

			switch (int(opnos(fieldn))) {
				case 0:
					break;
				case 1:
					ok = value == values(fieldn);
					break;
				case 2:
					ok = value != values(fieldn);
					break;
				case 3:
					ok = value > values(fieldn);
					break;
				case 4:
					ok = value < values(fieldn);
					break;
				case 5:
					ok = value >= values(fieldn);
					break;
				case 6:
					ok = value <= values(fieldn);
					break;
				case 7:
					ok = value.match(values(fieldn));
					break;
				case 8:
					ok = value.match(values(fieldn),"i");
					break;
				case 9:
					ok = ! value.match(values(fieldn));
					break;
				case 10:
					ok = ! value.match(values(fieldn),"i");
					break;
				case 11:
					ok = ! value >= values(fieldn) && value <= values2(fieldn);
					break;
				case 12:
					ok = ! value < values(fieldn) || value > values2(fieldn);
					break;
			}
			if (!ok) {
				//debug
				//value.outputl("FAILED=");
				break;
			}

			//VALUES (41472, 'Practical PostgreSQL', 1212, 4);
			value.swapper("'","''").squoter();
			insertsql ^= " " ^ value ^ ",";

		}

		//skip if failed to match
		if (!ok) {
			//ID.outputl("failed ");
			continue;
		}

		//ID.outputl("passed ");

		insertsql.splicer(-1,1,")");

		//insertsql.outputl();

		CURSOR.sqlexec(insertsql);

		//limit number of records returned
		++recn;
		if (maxnrecs && recn>maxnrecs) {
			CURSOR.clearselect();
			//break;
		}

	}

	sortselectclause2.outputl();

	return CURSOR.select(sortselectclause2);

}

bool ExodusProgramBase::savelist(const var& listname)
{
	return CURSOR.savelist(listname.field(" ", 1));
}

bool ExodusProgramBase::getlist(const var& listname)
{
	return CURSOR.getlist(listname.field(" ", 1));
}

bool ExodusProgramBase::makelist(const var& listname, const var& keys)
{
	return CURSOR.makelist(listname.field(" ", 1), keys);
}

bool ExodusProgramBase::deletelist(const var& listname)
{
	return CURSOR.deletelist(listname.field(" ", 1));
}

void ExodusProgramBase::clearselect() { CURSOR.clearselect(); }

bool ExodusProgramBase::hasnext() { return CURSOR.hasnext(); }

bool ExodusProgramBase::readnext(var& key) { return CURSOR.readnext(key); }

bool ExodusProgramBase::readnext(var& key, var& valueno) { return CURSOR.readnext(key, valueno); }

bool ExodusProgramBase::selectrecord(const var& sortselectclause)
{
	return CURSOR.selectrecord(sortselectclause);
}

bool ExodusProgramBase::readnextrecord(var& record, var& key)
{
	return CURSOR.readnextrecord(record, key);
}

bool ExodusProgramBase::pushselect(const var& v1, var& v2, var& v3, var& v4)
{
	// CURSOR.quote().outputl("CURSOR=");
	// CURSOR++;
	// CURSOR has connection number hidden in it, so it cannot be used as an ordinary variable
	// ATM
	CURSOR.transfer(v2);
	CURSOR = var().date() ^ "_" ^ (var().ostime().convert(".", "_"));
	return true;

	/*
	SUBROUTINE PUSH.SELECT(V1, V2, V3, V4)
	11 NULL
	12 *STOPLINECOUNT
	13 *STARTLINECOUNT
	14 *STOPLINECOUNT
	15 *STARTLINECOUNT
	16 TRANSFER SYS109_27 TO V2
	20 V3 = SYS109_63
	24 V3 := @RM : SYS109_99
	33 V3 := @RM : SYS109_140
	42 V3 := @RM : SYS109_64
	51 V3 := @RM : SYS109_61_FILEHANDLE
	60 V3 := @RM : SYS109_87
	69 V3 := @RM : SYS109_65
	78 V3 := @RM : SYS109_66
	87 V3 := @RM : SYS109_135_MAPNAME
	96 V3 := @RM : SYS109_79
	105 V4 = SYS109_58
	109 SYS109_63 = 0
	113 SYS109_100 += 1
	119 RETURN
	*/
}

bool ExodusProgramBase::popselect(const var& v1, var& v2, var& v3, var& v4)
{
	// CURSOR.quote().outputl("CURSOR=");
	// CURSOR--;
	v2.transfer(CURSOR);
	return true;
	/*
	SUBROUTINE POP.SELECT(V1, V2, V3, V4)
	11 NULL
	12 *STOPLINECOUNT
	13 *STARTLINECOUNT
	14 *STOPLINECOUNT
	15 *STARTLINECOUNT
	16 TRANSFER V2 TO SYS109_27
	20 SYS109_63 = V3[1, @RM]
	28 V3[1, COL2()]=''
	35 SYS109_99 = V3[1, @RM]
	43 V3[1, COL2()]=''
	50 SYS109_140 = V3[1, @RM]
	58 V3[1, COL2()]=''
	65 SYS109_64 = V3[1, @RM]
	73 V3[1, COL2()]=''
	80 SYS109_61_FILEHANDLE = V3[1, @RM]
	88 V3[1, COL2()]=''
	95 SYS109_87 = V3[1, @RM]
	103 V3[1, COL2()]=''
	110 SYS109_65 = V3[1, @RM]
	118 V3[1, COL2()]=''
	125 SYS109_66 = V3[1, @RM]
	133 V3[1, COL2()]=''
	140 SYS109_135_MAPNAME = V3[1, @RM]
	148 V3[1, COL2()]=''
	155 SYS109_79 = V3[1, @RM]
	163 SYS109_58 = V4
	167 SYS109_100 -= 1
	173 RETURN
	*/
}

void ExodusProgramBase::note(const var& msg, const var& options) const
{
	var buffer = "";
	mssg(msg, options, buffer);
}
void ExodusProgramBase::note(const var& msg, const var& options, var& buffer,
			     const var& params) const
{
	mssg(msg, options, buffer, params);
}
void ExodusProgramBase::mssg(const var& msg, const var& options) const
{
	var buffer = "";
	mssg(msg, options, buffer);
}
void ExodusProgramBase::mssg(const var& msg, const var& options, var& buffer,
			     const var& params) const
{

	var interactive = !SYSTEM.a(33);
	var msg1 = msg.convert("|" ^ FM ^ VM ^ SM, "\n\n\n\n").trim("\n");

	//swap %1, %2 etc with params
	for (var ii=1;ii<=9;++ii)
		msg1.swapper("%"^ii,params.a(ii));

	std::cout << msg1;

	//R=Reply required in buffer
	if (true || options.index("R"))
	{
		buffer="";
		if (interactive)
			buffer.input();
		else
			std::cout << std::endl;

		if (buffer=="" && options.index("E"))
			buffer=0x1B;//esc
		if (options.index("C"))
			buffer.ucaser();
		return;
	}

	if (!options.index("U"))
	{
		if (USER4.length() > 8000)
		{
			var msg2 = "Aborted MSG()>8000";
			std::cout << msg2;
			USER4 ^= FM ^ msg2;
		}
		else
		{
			USER4.r(-1, msg1);
		}
	}

	std::cout << std::endl;
}

var ExodusProgramBase::authorised(const var& task0)
{
	var msg;
	return authorised(task0, msg);
}

var ExodusProgramBase::authorised(const var& task0, var& msg, const var& defaultlock,
				  const var& username0)
{

	var username;
	var msgusername;
	var taskn; // num
	var taskn2;
	var xx;
	var usern;

	var task = task0;
	if (username0.unassigned() or username0 eq "")
	{
		// allow for username like FINANCE(STEVE)
		// so security is done like FINANCE but record is kept of actual user
		// this allows for example billing module users to post as finance module users
		username = USERNAME.field("(", 1);
		msgusername = USERNAME;
	}
	else
	{
		username = username0;
		msgusername = username;
	}

	// if username='NEOSYS' or username='STEVE' then call msg(task:'');

	if (task[1] == " ")
	{
		call mssg(DQ ^ (task0 ^ DQ));
	}
	// Each task may have many "locks", each users may have many "keys"
	// A user must have keys to all the locks in order to pass

	if (not task)
	{
		return 1;
	}

	task.ucaser();
	task.converter(RM ^ FM ^ VM ^ SM, "\\\\\\");
	task.swapper(" FILE ", " ");
	task.swapper(" - ", " ");
	task.converter(".", " ");
	task.trimmer();

	msg = "";
	// **CALL note(' ':TASK)

	if (task.substr(1, 2) == "..")
	{
		// call note(task:'')
		return 1;
	}

	var noadd = task[1] == "!";
	if (noadd)
	{
		task.splicer(1, 1, "");
	}
	// if noadd else NOADD=((TASK[-1,1]='"') and (len(userprivs)<10000))
	if (not noadd)
	{
		var lenuserprivs = SECURITY.length();
		noadd = task[-1] == DQ or lenuserprivs > 48000;
	}
	var positive = task[1];
	if (positive == "#")
		task.splicer(1, 1, "");
	else
		positive = "";

	//? as first character of task (after positive) means
	// security is being used as a configuration and user neosys has no special privs
	var isneosys;
	if (task[1] == "?")
	{
		isneosys = 0;
		task.splicer(1, 1, "");
	}
	else
		isneosys = username == "NEOSYS";

	var deleting = task.substr(1, 8) == "%DELETE%";
	if (deleting)
	{
		task.splicer(1, 8, "");
	}
	var updating = task.substr(1, 8) == "%UPDATE%";
	if (updating)
	{
		task.splicer(1, 8, "");
	}
	var renaming = task.substr(1, 8) == "%RENAME%";
	if (renaming)
	{
		task.splicer(1, 8, "");
	}

	// find the task
	if (SECURITY.a(10).locate(task, taskn))
	{
		if (deleting)
		{
			// SECURITY.eraser(10, taskn);
			// SECURITY.eraser(11, taskn);
			SECURITY.remover(10, taskn);
			SECURITY.remover(11, taskn);
		updateprivs:
			gosub writeuserprivs();
			return 1;
		}
		else if (renaming)
		{
			// delete any existing rename target task
			if (SECURITY.a(10).locate(defaultlock, taskn2))
			{
				// SECURITY.eraser(10, taskn2);
				// SECURITY.eraser(11, taskn2);
				SECURITY.remover(10, taskn2);
				SECURITY.remover(11, taskn2);
				if (taskn2 < taskn)
				{
					taskn -= 1;
				}
			}
			SECURITY.r(10, taskn, defaultlock);
			if (renaming)
			{
				call note(task ^ "|TASK RENAMED|" ^ defaultlock);
			}
			goto updateprivs;
		}
		else if (updating)
		{
			var tt = defaultlock;
			if (SECURITY.a(10).locate(defaultlock, taskn2))
			{
				tt = SECURITY.a(11, taskn2);
			}
			SECURITY.r(11, taskn, tt);
			goto updateprivs;
		}
	}
	else
	{
		if (deleting)
		{
			return 1;
		}
		if (renaming)
		{
			// if the task to be renamed doesnt exist just add the target task
			call authorised(defaultlock, msg);
			return 1;
		}
		if (not noadd)
		{
			gosub readuserprivs();
			// if (SECURITY.length() < 65000) {
			if (true)
			{
				var x = var();
				if (not(SECURITY.a(10).locateby("A", task, taskn)))
				{
					var newlock = defaultlock;
					// get locks on default task if present otherwise new locks
					// are none
					if (newlock and SECURITY.a(10).locate(newlock))
					{
						newlock = SECURITY.a(11, xx);
					}
					SECURITY.inserter(10, taskn, task);
					SECURITY.inserter(11, taskn, newlock);
					gosub writeuserprivs();
					if (username == "NEOSYS")
					{
						call note(task ^ "|TASK ADDED");
					}
				}
			}
		}
	}

	// if no locks then pass ok unless positive locking required
	var locks = SECURITY.a(11, taskn);
	if (locks == "")
	{
		if (positive and not isneosys)
		{
		notallowed:
			// MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not
			// authorised to do this.|'
			if (msgusername != USERNAME)
				msg = capitalise(msgusername) ^ "is not";
			else
				msg = "Sorry, " ^ capitalise(msgusername) ^ ", you are";

			msg ^= " not";
			if (positive)
			{
				msg ^= " specifically";
			}
			msg ^= " authorised to do||" ^ capitalise(task);

			return 0;
		}
		else
			return 1;
	}
	else if (locks == "NOONE")
	{
		goto notallowed;
	}

	// if index('012',@privilege,1) then goto ok
	if (isneosys)
	{
		return 1;
	}

	// find the user (add to bottom if not found)
	// surely this is not necessary since users are in already
	if (not(SECURITY.a(1).locate(username, usern)))
	{
		if (username != "NEOSYS" and username != APPLICATION)
		{
			gosub readuserprivs();
			usern = (SECURITY.a(1)).count(VM) + (SECURITY.a(1) != "") + 1;
			if (SECURITY.length() < 65000)
			{
				var users;
				if (not(users.open("USERS")))
				{
					goto notallowed;
				}
				var USER;
				if (not(USER.read(users, username)))
				{
					goto notallowed;
				}
				SECURITY.inserter(1, usern, username);
				SECURITY.inserter(2, usern, "");
				// add in memory only
				// gosub writeuserprivs
			}
		}
	}

	// user must have all the keys for all the locks on this task
	// following users up to first blank line also have the same keys
	var keys = SECURITY.a(2).field(VM, usern, 999);
	var temp = keys.index("---", 1);
	if (temp)
	{
		keys.splicer(temp - 1, 999, "");
	}
	// convert ',' to vm in keys
	// convert ',' to vm in locks
	keys.converter("," _VM_ "", "  ");
	locks.converter(",", " ");
	// NLOCKS=COUNT(LOCKS,vm)+1
	var nlocks = locks.count(" ") + 1;

	for (var lockn = 1; lockn <= nlocks; ++lockn)
	{
		// LOCKx=FIELD(LOCKS,vm,LOCKN)
		var lockx = locks.field(" ", lockn);
		if (keys.locateusing(" ", lockx))
		{
			// call note(task:' ok')
		}
		else
			// MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not
			// authorised to do this.|' MSG='Sorry, ':capitalise(msgusername):', you are
			// not authorised to do||':capitalise(task)
			// call note(task:' ko')
			// RETURN 0
			goto notallowed;
	}; // lockn;

	// ok:
	// CALL STATUP(2,3,TASK)
	return 1;
}

void ExodusProgramBase::readuserprivs() const
{
	if (not DEFINITIONS or not(SECURITY.read(DEFINITIONS, "SECURITY")))
	{
		SECURITY = "";
	}
	return;
}

void ExodusProgramBase::writeuserprivs() const
{
	SECURITY.r(9, "");
	if (DEFINITIONS)
	{
		SECURITY.write(DEFINITIONS, "SECURITY");
	}
	return;
}

var ExodusProgramBase::capitalise(const var& str0, const var& mode0, const var& wordseps0) const
{

	var string2;

	if (mode0.unassigned() || mode0 == "CAPITALISE")
	{
		string2 = str0;
		// convert @upper.case to @lower.case in string2
		int nn = string2.length();
		var numx = var("1234567890").index(string2[1], 1);
		var cap = 1;
		var wordseps;
		var inquotes = 0;
		// wordseps=' /-.()&'
		if (wordseps0.unassigned())
			wordseps = " .()&_" _RM_ _FM_ _VM_ _SM_ _TM_ _STM_;
		else
			wordseps = wordseps0;
		for (int ii = 1; ii <= nn; ii++)
		{
			var tt = string2.substr(ii, 1);

			if (inquotes)
			{
				inquotes = tt != inquotes;
			}
			else
			{
				if (tt == DQ && (string2.count(DQ) > 1 || tt == "\'") &&
				    string2.count("\'") > 1)
				{
					inquotes = tt;
				}
				else
				{
					if (wordseps.index(tt, 1))
					{
						cap = 1;
						if (tt == " ")
							numx = var("1234567890")
								   .index(string2.substr(ii + 1, 1),
									  1);
					}
					else
					{
						if (cap || numx)
						{
							tt.converter(LOWERCASE, UPPERCASE);
							string2.splicer(ii, 1, tt);
							cap = 0;
						}
						else
						{
							tt.converter(UPPERCASE, LOWERCASE);
							string2.splicer(ii, 1, tt);
						}
					}
				}
			}

		}; // ii;

		string2.swapper("\'S ", "\'s ");
		if (string2.substr(-2, 2) == "\'S")
			string2.splicer(-2, 2, "\'s");
	}
	else if (mode0 == "QUOTE")
	{
		string2 = str0;
		if (string2 != "")
		{
			string2.converter(FM ^ VM ^ SVM ^ TM, "    ");
			string2.swapper(" ", "\" \"");
			string2 = string2.quote();
		}
	}
	else if (mode0 == "UPPERCASE")
	{
		string2 = str0;
		string2.converter(LOWERCASE, UPPERCASE);
	}
	else if (mode0 == "LOWERCASE")
	{
		string2 = str0;
		string2.converter(UPPERCASE, LOWERCASE);
	}
	else if (mode0.substr(1, 5) == "PARSE")
	{

		var uppercase = mode0.index("UPPERCASE", 1);

		string2 = str0;

		// convert to uppercase
		var quoted = "";
		for (int ii = 1; ii <= 99999; ii++)
		{
			var tt = string2.substr(ii, 1);
			// BREAK;
			if (!(tt != ""))
				break;
			;
			if (tt == quoted)
			{
				quoted = "";
			}
			else
			{
				if (!quoted)
				{
					if ((DQ ^ "\'").index(tt, 1))
					{
						quoted = tt;
					}
					else
					{
						if (tt == " ")
						{
							tt = FM;
							string2.splicer(ii, 1, tt);
						}
						else
						{
							if (uppercase)
							{
								tt.converter(LOWERCASE, UPPERCASE);
								string2.splicer(ii, 1, tt);
							}
						}
					}
				}
			}
		}; // ii;

		if (mode0.index("TRIM", 1))
		{
			string2.converter(" " _FM_, _FM_ " ");
			string2 = string2.trim();
			string2.converter(" " _FM_, _FM_ " ");
		}
	}

	return string2;
}

var ExodusProgramBase::execute(const var& sentence)
{

	var v1, v2, v3, v4;
	pushselect(v1, v2, v3, v4);

	var sentence2 = sentence.fieldstore(" ", 1, 1, sentence.field(" ", 1).lcase());

	var result = perform(sentence);

	popselect(v1, v2, v3, v4);

	return result;
}

void ExodusProgramBase::chain(const var& libraryname)
{
	CHAIN = libraryname;
	var().stop();
}

var ExodusProgramBase::perform(const var& sentence)
{
	// THISIS("var ExodusProgramBase::perform(const var& sentence)")
	// ISSTRING(sentence)

	// return ID^"*"^dictid;

	// wire up the the library linker to have the current mvenvironment
	// if (!perform_exodusfunctorbase_.mv_)
	//	perform_exodusfunctorbase_.mv_=this;

	// lowercase all library functions to aid in conversion from arev
	// TODO remove after conversion complete

	perform_exodusfunctorbase_.mv_ = (&mv);

	// save some environment
	var savesentence;
	var savecommand;
	var saveoptions;
	var saverecur0;
	var saverecur1;
	var saverecur2;
	var saverecur3;
	var saverecur4;
	SENTENCE.transfer(savesentence);
	COMMAND.transfer(savecommand);
	OPTIONS.transfer(saveoptions);
	RECUR0.transfer(saverecur0);
	RECUR0.transfer(saverecur1);
	RECUR0.transfer(saverecur2);
	RECUR0.transfer(saverecur3);
	RECUR0.transfer(saverecur4);

	SENTENCE = sentence;
	while (SENTENCE)
	{

		// set new perform environment
		COMMAND = SENTENCE;
		OPTIONS = "";
		// similar code in exodus_main() and mvprogram.cpp:perform()
		var lastchar = COMMAND[-1];
		if (lastchar == ")")
		{
			OPTIONS = "(" ^ COMMAND.field2("(", -1);
		}
		else if (lastchar == "}")
			OPTIONS = "{" ^ COMMAND.field2("{", -1);
		if (OPTIONS)
			COMMAND.splicer(-(OPTIONS.length()), OPTIONS.length(), "");

		// load the shared library file
		var libid = SENTENCE.field(" ", 1).lcase();
		std::string str_libname = libid.toString();
		if (!perform_exodusfunctorbase_.initsmf(str_libname.c_str(),
							"exodusprogrambasecreatedelete_",
							true // forcenew each perform/execute
							))
		{
			USER4 ^= "perform() Cannot find shared library \"" ^ str_libname ^
				 "\", or \"libraryexit()\" is not present in it.";
			// throw MVException(USER4);
			// return "";
			break;
		}

		// call the shared library exodus object's main function
		try
		{
			ANS = perform_exodusfunctorbase_.callsmf();
		}
		catch (const MVUndefined& e)
		{
			// if return "" is missing then default ANS to ""
			ANS = "";
		}
		catch (const MVStop& e)
		{
			// stop is normal way of stopping a perform
			// functions can call it to terminate the whole "program"
			// without needing to setup chains of returns
			// to exit from nested functions
			ANS = "";
		}
		catch (const MVAbort& e)
		{
			// similar to stop for the time being
			// maybe it should set some error flag/messages
			ANS = "";
		}
		catch (const MVAbortAll& e)
		{
			// similar to stop for the time being
			// maybe it should set some error flag/messages
			// and abort multiple levels of perform?
			ANS = "";
		}

		// chain is a kind of optional follow controlled by the library function
		// to perform another function after the first
		// go round again if anything in CHAIN
		CHAIN.transfer(SENTENCE);
	}

	// restore some environment
	savesentence.transfer(SENTENCE);
	savecommand.transfer(COMMAND);
	saveoptions.transfer(OPTIONS);
	saverecur0.transfer(RECUR0);
	saverecur1.transfer(RECUR1);
	saverecur2.transfer(RECUR2);
	saverecur3.transfer(RECUR3);
	saverecur4.transfer(RECUR4);

	return ANS;
}

var ExodusProgramBase::xlate(const var& filename, const var& key, const var& fieldno_or_name,
			     const var& mode)
{

	// TODO implement additional MV argument

	// key can be multivalued and multivalued result will be returned
	var results = "";
	var nkeys = key.dcount(VM);

	for (var keyn = 1; keyn <= nkeys; ++keyn)
	{

		var keyx = key.a(1, keyn);

		// handle non-numeric field_no ie dictionary field/column name
		if (not fieldno_or_name.isnum())
		{

			// get the whole record
			var record = keyx.xlate(filename, "", mode);

			// TODO what if key is multivalued?

			// handle record not found and mode C
			if (mode == "C" && record == keyx)
			{
				results.r(keyn, key);
				continue;
			}

			// handle record not found and mode X
			if (not record.length())
			{
				results.r(keyn, "");
				continue;
			}

			// use calculate()
			var result =
			    calculate(fieldno_or_name, "dict_" ^ filename.a(1), keyx, record);
			if (nkeys > 1)
				result.lowerer();
			results.r(keyn, result);
			continue;
		}

		// ordinary numeric or "" fieldno
		results.r(keyn, keyx.xlate(filename, fieldno_or_name, mode));
	}

	return results;
}

var ExodusProgramBase::calculate(const var& dictid, const var& dictfile, const var& id,
				 const var& record, const var& mvno)
{

	// TODO how is it possible to exchange when the variable is const?

	DICT.exchange(dictfile);
	ID.exchange(id);
	RECORD.exchange(record);
	MV.exchange(mvno);

	var result = calculate(dictid);

	DICT.exchange(dictfile);
	ID.exchange(id);
	RECORD.exchange(record);
	MV.exchange(mvno);

	return result;
}

var ExodusProgramBase::calculate(const var& dictid)
{
	// THISIS("var ExodusProgramBase::calculate(const var& dictid)")
	// ISSTRING(dictid)

	// return ID^"*"^dictid;

	// get the dictionary record so we know how to extract the correct field or call the right
	// library
	bool newlibfunc;
	bool indictmd = false;
	if (cache_dictid_ != dictid)
	{
		newlibfunc = true;
		if (not DICT)
			throw MVException("calculate(" ^ dictid ^
					  ") DICT file variable has not been set");
		if (not cache_dictrec_.reado(DICT, dictid))
		{
			// try lower case
			if (not cache_dictrec_.reado(DICT, dictid.lcase()))
			{

				// try dict_voc
				var dictmd; // TODO implement DICTMD to avoid opening
				if (not dictmd.open("dict_voc"))
				{
				baddict:
					throw MVException("calculate(" ^ dictid ^
							  ") dictionary record not in DICT " ^
							  DICT.quote());
				}
				if (not cache_dictrec_.reado(dictmd, dictid))
				{
					// try lower case
					if (not cache_dictrec_.reado(dictmd, dictid.lcase()))
					{
						goto baddict;
					}
				}
				indictmd = true;
			}
		}
		cache_dictid_ = dictid;
	}
	else
		newlibfunc = false;

	var dicttype = cache_dictrec_(1);
	bool ismv = cache_dictrec_(4)[1] == "M";

	// F type dictionaries
	if (dicttype == "F")
	{

		// check field number is numeric
		var fieldno = cache_dictrec_(2);
		if (!fieldno.isnum())
			return "";

		// field no > 0
		if (fieldno)
		{
			if (ismv)
				return RECORD(fieldno, MV);
			else
				return RECORD(fieldno);

			// field no 0
		}
		else
		{
			var keypart = cache_dictrec_(5);
			if (keypart && keypart.isnum())
				return ID.field("*", keypart);
			else
				return ID;
		}
	}
	else if (dicttype == "S")
	{
		// TODO deduplicate various exodusfunctorbase code spread around calculate mvipc*
		// etc
		if (newlibfunc)
		{

			std::string str_libname;
			if (indictmd)
				str_libname = "dict_voc";
			else
				str_libname = DICT.a(1).lcase().convert(".", "_").toString();

			// get from cache
			std::string cachekey = dictid.lcase().toString() + "_" + str_libname;
			dict_exodusfunctorbase_ = dict_function_cache[cachekey];
			// if (dict_exodusfunctorbase_) {
			//	delete dict_exodusfunctorbase_;
			//	dict_exodusfunctorbase_=NULL;
			//}

			// if not in cache then create new one
			if (!dict_exodusfunctorbase_)
			{
				// var(cachekey).outputl("cachekey=");
				dict_exodusfunctorbase_ = new ExodusFunctorBase;
				dict_function_cache[cachekey] = dict_exodusfunctorbase_;
				// wire up the the library linker to have the current mvenvironment
				// if (!dict_exodusfunctorbase_.mv_)
				dict_exodusfunctorbase_->mv_ = (&mv);

				std::string str_funcname =
				    ("exodusprogrambasecreatedelete_" ^ dictid.lcase()).toString();
				if (!dict_exodusfunctorbase_->initsmf(str_libname.c_str(),
								      str_funcname.c_str()))
					throw MVException("calculate() Cannot find Library " ^
							  str_libname ^ ", or function " ^
							  dictid.lcase() ^ " is not present");
			}
		}

		// for single valued fields, inform the called routine that MV is 0
		int savedMV;
		if (!ismv)
		{
			savedMV = MV;
			MV = 0;
		}

		// return dict_exodusfunctorbase_.calldict();
		// return ANS;

		// define a function type (pExodusProgramBaseMemberFunction)
		// that can call the shared library object member function
		// with the right arguments (none for dicts) and returning a var
		typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

		// call the shared library object main function with the right args (none for
		// dicts), returning a var std::cout<<"precal"<<std::endl;
		ANS = CALLMEMBERFUNCTION(*(dict_exodusfunctorbase_->pobject_),
					 ((pExodusProgramBaseMemberFunction)(
					     dict_exodusfunctorbase_->pmemberfunction_)))();
		// std::cout<<"postcal"<<std::endl;

		// restore the MV if necessary
		if (!ismv)
			MV = savedMV;

		return ANS;
	}

	throw MVException("calculate(" ^ dictid ^ ") " ^ DICT ^ " Invalid dictionary type " ^
			  dicttype.quote());
	return "";
}

bool ExodusProgramBase::unlockrecord(const var& filename, const var& file0, const var& key) const
{
	var file;
	if (file0.unassigned())
		file = "";
	else
		file = file0;

	if (file == "")
	{
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

	// evade warning: unused parameter
	if (filename)
	{
	}
	return 1;
}

void ExodusProgramBase::debug() const
{

	var reply;
	std::cout << "debug():";
	if (SLASH == "/")
		asm("int $3");
	// cin>>reply;
	return;
}

bool ExodusProgramBase::fsmsg(const var& msg) const
{
	mssg(msg ^ var().getlasterror());
	return false;
}

var ExodusProgramBase::sysvar(const var& var1, const var& var2, const var& var3, const var& var4)
{

	std::cout << "sysvar() do nothing:";
	//	var reply;
	//	cin>>reply;
	return "";

	// evade warning: unused parameter
	if (var1 || var2 || var3 || var4)
	{
	}
}

void ExodusProgramBase::setprivilege(const var& var1)
{

	PRIVILEGE = var1;
	std::cout << "setprivilege(" << var1 << ") do nothing" << std::endl;
	//	var reply;
	//	cin>>reply;
	return;
}

var ExodusProgramBase::decide(const var& question, const var& options) const
{
	var reply = "";
	var buffer;
	return decide(question, options, reply, buffer);
}

var ExodusProgramBase::decide(const var& questionx, const var& optionsx, var& reply,
			      const int defaultreply) const
{

	var interactive = !SYSTEM.a(33);

	var options = optionsx;
	var question = questionx;

	options.converter(VM ^ "|", FM ^ FM);
	if (!interactive)
	{
		if (defaultreply)
		{
			reply = defaultreply;
		}
		else
		{
			reply = 1;
		}
		return options.a(reply);
	}

	question.converter(VM ^ "|", FM ^ FM);
	question.swapper(FM, var().chr(13) ^ var().chr(10));
	std::cout << question << std::endl;

	//	var noptions = options.count(FM) + (options != "");
	var noptions = options.dcount(FM);
	for (int optionn = 1; optionn <= noptions; optionn++)
		std::cout << optionn << ". " << options.a(optionn) << std::endl;

inp:
	reply.input();
	if (reply == "" || reply >= 1 || reply <= noptions)
	{
		return reply;
	}
	else
	{
		goto inp;
	}

	return "";
}

void ExodusProgramBase::savescreen(var& origscrn, var& origattr) const
{
	std::cout << "ExodusProgramBase::savescreen not implemented" << std::endl;

	// evade warning: unused parameter
	if (origscrn || origattr)
	{
	}
}

// NO STANDARD C/C++ way to test for key pressed or chars in input buffer!
// SEE http://c-faq.com/osdep/kbhit.txt (but 1996) for info
// Checks keyboard buffer (stdin) and returns key
// pressed, or -1 for no key pressed
int ExodusProgramBase::keypressed(int delayusecs) const
{
	/* will not compile on mingw because unlike cygwin
	 * mingw is close to windows and windows select only works on sockets
	 * should be easy to reimplement in another way
	 *char keypressed;
	 struct timeval waittime;
	 int nummv.charsmv.read;
	 struct fdmv.set mask;
	 FDmv.SET(0, &mask);

	 waittime.tvmv.sec = 0;
	 waittime.tvmv.usec = delayusecs;
	 if (select (1, &mask, 0, 0, &waittime))
	 {
	 nummv.charsmv.read = read (0, &keypressed, 1);
	 if (nummv.charsmv.read == 1)
	 {
	 cin.putback(keypressed);
	 return keypressed;
	 }
	 }
	 */
	return 0;

	// evade warning: unused parameter
	if (delayusecs)
	{
	}
}

bool ExodusProgramBase::esctoexit() const
{
	char keypress = keypressed();
	if (keypress != 0x1B)
	{
		//		cin.putback(keypress);
		return false;
	}

	std::cout << "Paused. Press Enter to resume or Ctrl+C to cancel:" << std::endl;

	while (true)
	{
		keypress = keypressed(1000000);
		if (keypress == 0x1B)
			return true;
	}

	//	keypress=cin.peek();
	//	if (keypress==0x1B) return true;

	return false;
}

var ExodusProgramBase::otherusers(const var& param)
{
	std::cout << "ExodusProgramBase::otherusers not implemented yet";
	return var("");

	// evade warning: unused parameter
	if (param)
	{
	}
}

var ExodusProgramBase::otherdatasetusers(const var& param)
{
	std::cout << "ExodusProgramBase::otherdatausers not implemented yet";
	return var("");

	// evade warning: unused parameter
	if (param)
	{
	}
}

bool ExodusProgramBase::lockrecord(const var& filename, const var& file, const var& keyx) const
{
	var record;
	return (bool)lockrecord(filename, file, keyx, record);
}

bool ExodusProgramBase::lockrecord(const var& filename, const var& file, const var& keyx,
				   const var& recordx, const int waitsecs0,
				   const bool allowduplicate) const
{

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

lock:
	var locked = file.lock(keyx);
	if (locked || (allowduplicate && locked eq ""))
	{
		return 1;
	}
	else
	{
		if (waitsecs)
		{
			var().ossleep(1000);
			waitsecs -= 1;
			goto lock;
		}
		return false;
	}

	// evade warning: unused parameter
	(false || filename || recordx);

	return true;
}

var ExodusProgramBase::singular(const var& pluralnoun)
{

	var temp = pluralnoun;

	if (temp.substr(-2, 2) == "ES")
	{

		// companies=company
		if (temp.substr(-3, 3) == "IES")
		{
			temp.splicer(-3, 3, "Y");

			// addresses=address
		}
		else if (temp.substr(-4, 4) == "SSES")
		{
			temp.splicer(-2, 2, "");
		}
		else if (temp.substr(-4, 4) == "SHES")
		{
			temp.splicer(-2, 2, "");
		}
		else if (temp.substr(-4, 4) == "CHES")
		{
			temp.splicer(-2, 2, "");
		}
		else if (1)
		{
			temp.splicer(-1, 1, "");
		}
	}
	else
	{

		if (temp[-1] == "S")
		{
			// analysis, dos
			if (temp.substr(-2, 2) != "IS" && temp.substr(-2, 2) != "OS")
				temp.splicer(-1, 1, "");
		}
	}

	return temp;
}

void ExodusProgramBase::flushindex(const var& filename)
{
	std::cout << "ExodusProgramBase::std::flushindex not implemented yet, " << filename
		  << std::endl;
	return;
}

var ExodusProgramBase::encrypt2(const var& encrypt0) const
{

	var encrypt = encrypt0;
	var encryptkey = 1234567;

	// pass1
	while (true)
	{
		// BREAK;
		if (!(encrypt != ""))
			break;
		;
		encryptkey = (encryptkey % 390001) * (var(encrypt[1])).seq() + 1;
		encrypt.splicer(1, 1, "");
	} // loop;

	// pass2
	while (true)
	{
		encrypt ^= var().chr(65 + (encryptkey % 50));
		encryptkey = (encryptkey / 50).floor();
		// BREAK;
		if (!encryptkey)
			break;
		;
	} // loop;

	return encrypt;
}

var ExodusProgramBase::xmlquote(const var& string0) const
{

	var string1;

	if (string0.unassigned())
	{
		// de bug
		string1 = "UNASSIGNED";
	}
	else
	{
		string1 = string0;
	}

	string1.swapper("&", "&amp;");
	string1.swapper(DQ, "&quot;");
	string1.swapper("<", "&lt;");
	string1.swapper(">", "&gt;");

	string1.converter(DQ, "\'");
	string1.swapper(VM, "\" \"");
	string1.swapper(FM, "\" \"");
	return string1.quote();
}

var ExodusProgramBase::loginnet(const var& dataset, const var& username, var& cookie, var& msg)
{

	// evade warning: unused parameter
	if (false && dataset)
	{
	}

	var menuid;
	var usern;
	var menun;
	var xx;

	// this is a custom login routine called from listen2
	cookie = "";
	var menus;
	if (!menus.open("ADMENUS"))
	{
		if (!menus.open("MENUS") && username != "NEOSYS")
		{
			msg = "Error: Cannot open MENUS file";
			return false;
		}
	}

	// return allowable menus
	if (username == "NEOSYS")
	{
		menuid = "ADAGENCY";
	}
	else
	{
		if (!(SECURITY.a(1).locate(username, usern)))
		{
			msg = "Error: " ^ username.quote() ^ " user is missing";
			return false;
		}
		menuid = SECURITY.a(3, usern);
	}

	var menu = "";
	if (!menu.read(menus, menuid))
	{
		if (username == "NEOSYS")
		{
			if (!menu.read(menus, "NEOSYS"))
			{
				menu = FM ^ FM ^ FM ^ FM ^ FM ^
				       "MEDIA|ADPRODUCTION|ACCS|ANALMENU|TIMESHEETS|FILESMENU|"
				       "GENERAL|EXIT2";
				menu = menu.converter("|", VM);
			}
		}
	}
	if (!menu)
	{
		msg = "Error: " ^ menuid.quote() ^ " menu is missing";
		return false;
	}

	var menucodes = menu.a(6) ^ VM ^ "HELP";
	// remove local support menu
	if (!authorised("SUPPORT MENU ACCESS", msg, "LS"))
	{
		if (menucodes.a(1).locate("GENERA", menun))
			// menucodes.eraser(1, menun, 0);
			menucodes.remover(1, menun, 0);
	}
	menucodes.converter(VM ^ ".", ",_");

	// prepare session cookie
	cookie = "m=" ^ menucodes;

	return true;

	/* custom login per application
	 var compcode = "";

	 var temp;
	 if (!(temp.read(companies, "%RECORDS%"))) {
	 companies.select();
	 temp = "";
	 nextcomp:
	 var compcodex;
	 if (var("").readnext(compcodex)) {
	 temp.pickreplace(-1, 0, 0, compcodex);
	 goto nextcomp;
	 }
	 }

	 if (APPLICATION ne "ACCOUNTS") {
	 for (int ii = 1; ii <= 9999; ii++) {
	 compcode = temp.a(ii);
	 //until validcode('company',compcode)
	 //BREAK;
	 if (validcode2(compcode, "", "")) break;;
	 };//ii;
	 }else{
	 compcode = temp.a(1);
	 }

	 if (!compcode) {
	 msg = "Error: You are not authorised to access any companies";
	 return false;
	 }

	 var tempcompany;
	 if (!tempcompany.read(companies, compcode)) {
	 msg = "Error: " ^ compcode.quote() ^ " company code is missing";
	 return;
	 }

	 company = "";
	 initcompany(compcode);

	 force error here TODO: check trigraph following;
	 var defmarketcode = (company.a(30)) ? (company.a(30)) : (agp.a(37));
	 //if unassigned(markets) then markets=''

	 //markets is not open in finance only module
	 //readv maincurrcode from markets,defmarketcode,5 else maincurrcode=''
	 var maincurrcode = "";
	 if (FILES[0].locateusing(FM,"MARKETS", FM))
	 maincurrcode = defmarketcode.xlate("MARKETS", 5, "X");

	 if (maincurrcode.unassigned())
	 maincurrcode = "";
	 if (maincurrcode == "")
	 maincurrcode = basecurrency;

	 cookie ^= "&cc=" ^ compcode;
	 cookie ^= "&pd=" ^ currperiod ^ "/" ^ addcent(curryear);
	 cookie ^= "&bc=" ^ basecurrency;
	 cookie ^= "&bf=" ^ USER2;
	 cookie ^= "&mk=" ^ defmarketcode;
	 cookie ^= "&mc=" ^ maincurrcode;
	 temp = SYSTEM.a(23);
	 temp.swap("&", " and ");
	 cookie ^= "&db=" ^ temp;

	 backupreminder(dataset, msg);

	 changelogsubs("WHATSNEW" ^ FM ^ menucodes);
	 cookie ^= "&wn=" ^ ANS;

	 */
}

var ExodusProgramBase::AT(const int code) const
{
	// should depend on terminal type
	return var().at(code);
}

var ExodusProgramBase::AT(const int x, const int y) const
{
	// should depend on terminal type
	return var().at(x, y);
}

var ExodusProgramBase::handlefilename(const var& handle) { return handle.a(1); }

var ExodusProgramBase::memspace(const var& requiredmemory) { return 999999999; }

var ExodusProgramBase::getuserdept(const var& usercode)
{
	// locate the user in the list of users
	var usern;
	if (!(SECURITY.a(1).locate(usercode, usern)))
	{
		if (usercode == "NEOSYS")
		{
			ANS = "NEOSYS";
			return ANS;
		}
		else
		{
			ANS = "";
			return ANS;
		}
	}

	// locate divider, or usern+1
	var nusers = (SECURITY.a(1)).count(VM) + 1;
	var usernx;
	for (usernx = 1; usernx <= nusers; usernx++)
	{
		// BREAK;
		if (SECURITY.a(1, usernx) == "---")
			break;
		;
	}; // usern;

	// get the department code
	ANS = SECURITY.a(1, usernx - 1);

	return ANS;
}

var ExodusProgramBase::oconv(const var& input, const var& conversion)
{

	// call user conversion routine
	// almost identical code in var::oconv and var::iconv
	// ENSURE synchronised if you change it

	var result = input;
	var ptr = 1;
	var delimiter;
	do
	{

		// var subconversion=conversion.remove(ptr,delimiter);
		var subconversion = conversion.substr2(ptr, delimiter);

		// either call custom conversion routines
		if (subconversion[1] == "[")
		{

			// var("*").logput();
			// return 1;

			// extract any params
			var mode = subconversion.field(",", 2, 9999).field("]", 1);

			// set the function name
			ioconv_custom = subconversion.substr(2).field(",", 1).field("]", 1).lcase();

			// wire up the current environment
			ioconv_custom.mv_ = (&mv);

			// and call it
			var output;
			call ioconv_custom("OCONV", result, mode, output);
			result = output;

			// or call standard conversion methods
		}
		else
		{
			result = result.oconv(subconversion);
		}
	} while (delimiter);

	return result;
}

var ExodusProgramBase::iconv(const var& input, const var& conversion)
{

	// call user conversion routine
	// almost identical code in var::oconv and var::iconv
	// ENSURE synchronised if you change it

	var result = input;
	var ptr = 1;
	var delimiter;
	do
	{

		// var subconversion=conversion.remove(ptr,delimiter);
		var subconversion = conversion.substr2(ptr, delimiter);

		// either call custom conversion routines
		if (subconversion[1] == "[")
		{

			// extract any params
			var mode = subconversion.field(",", 2, 9999).field("]", 1);

			// set the function name
			ioconv_custom = subconversion.substr(2).field(",", 1).field("]", 1).lcase();

			// wire up the current environment
			ioconv_custom.mv_ = (&mv);

			// and call it
			var output;
			call ioconv_custom("ICONV", result, mode, output);
			result = output;

			// or call standard conversion methods
		}
		else
		{
			result = result.iconv(subconversion);
		}
	} while (delimiter);

	return result;
}

var ExodusProgramBase::invertarray(const var& input, const var& force0/*=0*/)
{
        //c sys in,=(0)

	var force=force0.unassigned() ? var(0) : force0;

        var output = "";
        var nfs = input.count(FM) + (input ne "");
        //for force to work, the first field must have full number of vns
        var maxnvs = 0;
        for (var fn = 1; fn <= nfs; ++fn) {
                var fieldx = input.field(FM, fn);
                if (fieldx.length() or force) {
                        var nvs = fieldx.count(VM) + 1;
                        if (force) {
                                if (nvs > maxnvs) {
                                        maxnvs = nvs;
                                }
                        }else{
                                maxnvs = nvs;
                        }
                        for (var vn = 1; vn <= maxnvs; ++vn) {
                                var cell = fieldx.field(VM, vn);
                                if (cell.length() or force) {
                                        output.r(vn, fn, cell);
                                }
                        };//vn;
                }
        };//fn;

        return output;
}

//automatic upto date/time
var ExodusProgramBase::elapsedtimetext(const var& fromdate, const var& fromtime)
{
	var uptodate,uptotime;
	return elapsedtimetext(fromdate,fromtime,uptodate,uptotime);
}

//given from and to
var ExodusProgramBase::elapsedtimetext(const var& fromdate, const var& fromtime, var& uptodate, var& uptotime) {
	//c sys in,in,io,io

	var text = "";

	if (uptodate.unassigned()) {
		uptodate = var().date();
	}
	if (uptotime.unassigned()) {
		uptotime = var().ostime();
	}
	//uptodate=date()
	//CALL DOSTIME(uptotime)

	//NSECS=INT(uptotime-fromTIME)
	var nsecs = uptotime - fromtime;
	//IF NSECS ELSE NSECS=1
	//uptodate=date()
	if (fromdate ne uptodate) {
		nsecs += (uptodate - fromdate) * 24 * 3600;
	}

	//cater for bug where start date isnt known and time has crossed midnight
	//so the 2nd time is less than the first
	if (nsecs < 0) {
		nsecs += 86400;
		}

	var weeks = (nsecs / 604800).floor();
	nsecs -= weeks * 604800;

	var days = (nsecs / 86400).floor();
	nsecs -= days * 86400;

	var hours = (nsecs / 3600).floor();
	nsecs -= hours * 3600;

	var minutes = (nsecs / 60).floor();
	nsecs -= minutes * 60;

	if (weeks) {
		text.r(-1, weeks ^ " week");
		if (weeks ne 1) {
			text ^= "s";
		}
	}
	if (days) {
		text.r(-1, days ^ " day");
		if (days ne 1) {
			text ^= "s";
		}
	}
	if (hours) {
		text.r(-1, hours ^ " hour");
		if (hours ne 1) {
			text ^= "s";
		}
	}
	if (minutes) {
		text.r(-1, minutes ^ " min");
		if (minutes ne 1) {
			text ^= "s";
		}
	}
	if (not(hours) and (minutes < 5)) {
		if (nsecs) {
			if (minutes or (nsecs - 10 > 0)) {
				nsecs = nsecs.oconv("MD00P");
			}else{
				nsecs = (nsecs.oconv("MD40P")) + 0;
				if (nsecs[1] == ".") {
					nsecs.splicer(1, 0, "0");
				}
			}
			if (nsecs) {
				text.r(-1, nsecs ^ " sec");
				if (nsecs ne 1) {
					text ^= "s";
				}
			} else if (not(minutes)) {
zero:
				text.r(-1, "< 1 msec");
			} else {
				text.r(-1, "exactly");
			}
		}else{
			if (not(minutes)) {
				goto zero;
			}
			text.r(3, "exactly");
		}
	}

	text.swapper(FM ^ FM ^ FM, FM);
	text.swapper(FM ^ FM, FM);
	text.swapper(FM, ", ");

	return text;
}

} // namespace exodus
