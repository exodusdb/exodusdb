#include <exodus/mvprogram.h>
#include <exodusmacros.h> //coding style is like application programming eg USERNAME not mv.USERNAME

//putting various member functions into all exodus programs allows access to the mv environment variable which is also available
//in all exodus programs.

namespace exodus {

//constructor with an mvenvironment
DLL_PUBLIC
ExodusProgramBase::ExodusProgramBase(MvEnvironment& inmv)
	:mv(inmv) {
	cache_dictid_ = L"";
	cache_perform_libid_ = L"";
}

//destructor
DLL_PUBLIC
ExodusProgramBase::~ExodusProgramBase() {
};

bool ExodusProgramBase::select(const var& sortselectclause)
{
	return CURSOR.select(sortselectclause);
}

bool ExodusProgramBase::savelist(const var& listname)
{
	return CURSOR.savelist(listname.field(L" ",1));
}

bool ExodusProgramBase::getlist(const var& listname)
{
	return CURSOR.getlist(listname.field(L" ",1));
}

bool ExodusProgramBase::makelist(const var& listname, const var& keys)
{
	return CURSOR.makelist(listname.field(L" ",1), keys);
}

bool ExodusProgramBase::deletelist(const var& listname)
{
	return CURSOR.deletelist(listname.field(L" ",1));
}

void ExodusProgramBase::clearselect()
{
	CURSOR.clearselect();
}

bool ExodusProgramBase::hasnext()
{
	return CURSOR.hasnext();
}

bool ExodusProgramBase::readnext(var& key)
{
	return CURSOR.readnext(key);
}

bool ExodusProgramBase::readnext(var& key, var& valueno)
{
	return CURSOR.readnext(key,valueno);
}

bool ExodusProgramBase::selectrecord(const var& sortselectclause)
{
	return CURSOR.selectrecord(sortselectclause);
}

bool ExodusProgramBase::readnextrecord(var& record, var& key)
{
	return CURSOR.readnextrecord(record, key);
}

bool ExodusProgramBase::pushselect(const var& v1, var& v2, var& v3, var& v4) {
	//CURSOR.quote().outputl("CURSOR=");
	//CURSOR++;
	//CURSOR has connection number hidden in it, so it cannot be used as an ordinary variable ATM
	CURSOR.transfer(v2);
	CURSOR=var().date() ^ L"_" ^ (var().ostime().convert(L".",L"_"));
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

bool ExodusProgramBase::popselect(const var& v1, var& v2, var& v3, var& v4) {
	//CURSOR.quote().outputl("CURSOR=");
	//CURSOR--;
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


void ExodusProgramBase::note(const var& msg, const var& options) const {
	var buffer = L"";
	mssg(msg, options, buffer);
}
void ExodusProgramBase::note(const var& msg, const var& options, var& buffer, const var& params) const {
	mssg(msg, options, buffer, params);
}
void ExodusProgramBase::mssg(const var& msg, const var& options) const {
	var buffer = L"";
	mssg(msg, options, buffer);
}
void ExodusProgramBase::mssg(const var& msg, const var& options, var& buffer, const var& params) const {

	//var interactive = !SYSTEM.a(33);
	var msg1=msg.convert(L"|" ^ FM ^ VM ^ SM, L"\n\n\n\n").trim(L"\n");

	std::wcout << msg1;

	if (!options.index(L"U")) {
		if (USER4.length() > 8000) {
			var msg2=L"Aborted MSG()>8000";
			std::wcout << msg2;
			USER4 ^= FM ^ msg2;
		} else {
			USER4.r(-1, msg1);
		}
	}

	std::wcout << std::endl;

}

var ExodusProgramBase::authorised(const var& task0) {
	var msg;
	return authorised(task0, msg);
}

var ExodusProgramBase::authorised(const var& task0, var& msg, const var& defaultlock, const var& username0) {

	var username;
	var msgusername;
	var taskn;//num
	var taskn2;
	var xx;
	var usern;

	var task=task0;
	if (username0.unassigned() or username0 eq "") {
		//allow for username like FINANCE(STEVE)
		//so security is done like FINANCE but record is kept of actual user
		//this allows for example billing module users to post as finance module users
		username = USERNAME.field("(", 1);
		msgusername = USERNAME;
	}else{
		username = username0;
		msgusername = username;
	}

	//if username='NEOSYS' or username='STEVE' then call msg(task:'');

	if (task[1] == " ") {
		call mssg(DQ ^ (task0 ^ DQ));
	}
	//Each task may have many "locks", each users may have many "keys"
	//A user must have keys to all the locks in order to pass

	if (not task) {
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

	if (task.substr(1, 2) == "..") {
		// call note(task:'')
		return 1;
	}

	var noadd = task[1] == "!";
	if (noadd) {
		task.splicer(1, 1, "");
	}
	//if noadd else NOADD=((TASK[-1,1]='"') and (len(userprivs)<10000))
	if (not noadd) {
		var lenuserprivs = SECURITY.length();
		noadd = task[-1] == DQ or lenuserprivs > 48000;
	}
	var positive = task[1];
	if (positive == "#")
		task.splicer(1, 1, "");
	else
		positive = "";

	//? as first character of task (after positive) means
	//security is being used as a configuration and user neosys has no special privs
	var isneosys;
	if (task[1] == "?") {
		isneosys = 0;
		task.splicer(1, 1, "");
	} else
		isneosys = username == "NEOSYS";

	var deleting = task.substr(1, 8) == "%DELETE%";
	if (deleting) {
		task.splicer(1, 8, "");
	}
	var updating = task.substr(1, 8) == "%UPDATE%";
	if (updating) {
		task.splicer(1, 8, "");
	}
	var renaming = task.substr(1, 8) == "%RENAME%";
	if (renaming) {
		task.splicer(1, 8, "");
	}

	//find the task
	if (SECURITY.a(10).locate(task, taskn)) {
		if (deleting) {
			SECURITY.eraser(10, taskn);
			SECURITY.eraser(11, taskn);
updateprivs:
			gosub writeuserprivs();
			return 1;
		} else if (renaming) {
			//delete any existing rename target task
			if (SECURITY.a(10).locate(defaultlock, taskn2)) {
				SECURITY.eraser(10, taskn2);
				SECURITY.eraser(11, taskn2);
				if (taskn2 < taskn) {
					taskn -= 1;
				}
			}
			SECURITY.r(10, taskn, defaultlock);
			if (renaming) {
				call note(task ^ "|TASK RENAMED|" ^ defaultlock);
			}
			goto updateprivs;
		} else if (updating) {
			var tt = defaultlock;
			if (SECURITY.a(10).locate(defaultlock, taskn2)) {
				tt = SECURITY.a(11, taskn2);
			}
			SECURITY.r(11, taskn, tt);
			goto updateprivs;
		}
	} else {
		if (deleting) {
			return 1;
		}
		if (renaming) {
			//if the task to be renamed doesnt exist just add the target task
			call authorised(defaultlock,msg);
			return 1;
		}
		if (not noadd) {
			gosub readuserprivs();
			//if (SECURITY.length() < 65000) {
			if (true) {
var x=var();
				if (not(SECURITY.a(10).locateby("AL", task, taskn))) {
					var newlock=defaultlock;
					//get locks on default task if present otherwise new locks are none
					if (newlock and SECURITY.a(10).locate(newlock)) {
						newlock = SECURITY.a(11, xx);
					}
					SECURITY.inserter(10, taskn, task);
					SECURITY.inserter(11, taskn, newlock);
					gosub writeuserprivs();
					if (username == "NEOSYS") {
						call note(task ^ "|TASK ADDED");
					}
				}
			}
		}
	}

	//if no locks then pass ok unless positive locking required
	var locks = SECURITY.a(11, taskn);
	if (locks == "") {
		if (positive and not isneosys) {
notallowed:
			//MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not authorised to do this.|'
			if (msgusername != USERNAME)
				msg = capitalise(msgusername) ^ "is not";
			else
				msg = "Sorry, " ^ capitalise(msgusername) ^ ", you are";

			msg ^= " not";
			if (positive) {
				msg ^= " specifically";
			}
			msg ^= " authorised to do||" ^ capitalise(task);

			return 0;

		} else
			return 1;

	} else if (locks == "NOONE") {
		goto notallowed;
	}

	//if index('012',@privilege,1) then goto ok
	if (isneosys) {
		return 1;
	}

	//find the user (add to bottom if not found)
	//surely this is not necessary since users are in already
	if (not(SECURITY.a(1).locate(username, usern))) {
		if (username != "NEOSYS" and username != APPLICATION) {
			gosub readuserprivs();
			usern = (SECURITY.a(1)).count(VM) + (SECURITY.a(1) != "") + 1;
			if (SECURITY.length() < 65000) {
				var users;
				if (not(users.open("USERS"))) {
					goto notallowed;
				}
				var USER;
				if (not(USER.read(users, username))) {
					goto notallowed;
				}
				SECURITY.inserter(1, usern, username);
				SECURITY.inserter(2, usern, "");
				//add in memory only
				//gosub writeuserprivs
			}
		}
	}

	//user must have all the keys for all the locks on this task
	//following users up to first blank line also have the same keys
	var keys = SECURITY.a(2).field(VM, usern, 999);
	var temp = keys.index("---", 1);
	if (temp) {
		keys.splicer(temp - 1, 999, "");
	}
	//convert ',' to vm in keys
	//convert ',' to vm in locks
	keys.converter("," _VM_ "", "  ");
	locks.converter(",", " ");
	//NLOCKS=COUNT(LOCKS,vm)+1
	var nlocks = locks.count(" ") + 1;

	for (var lockn = 1; lockn <= nlocks; ++lockn) {
		//LOCKx=FIELD(LOCKS,vm,LOCKN)
		var lockx = locks.field(" ", lockn);
		if (keys.locateusing(" ",lockx)) {
			//call note(task:' ok')
		} else
		//MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not authorised to do this.|'
		//MSG='Sorry, ':capitalise(msgusername):', you are not authorised to do||':capitalise(task)
	//call note(task:' ko')
		//RETURN 0
		goto notallowed;
	};//lockn;

//ok:
	//CALL STATUP(2,3,TASK)
	return 1;

}

void ExodusProgramBase::readuserprivs() const {
	if (not DEFINITIONS or not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
		SECURITY = "";
	}
	return;
}

void ExodusProgramBase::writeuserprivs() const {
	SECURITY.r(9, "");
	if (DEFINITIONS) {
		SECURITY.write(DEFINITIONS, "SECURITY");
	}
	return;
}

var ExodusProgramBase::capitalise(const var& str0, const var& mode0,
		const var& wordseps0) const {

	var string2;

	if (mode0.unassigned() || mode0 == L"CAPITALISE") {
		string2 = str0;
		//convert @upper.case to @lower.case in string2
		int nn = string2.length();
		var numx = var(L"1234567890").index(string2[1], 1);
		var cap = 1;
		var wordseps;
		var inquotes = 0;
		//wordseps=' /-.()&'
		if (wordseps0.unassigned())
			wordseps = L" .()&_" _RM_ _FM_ _VM_ _SM_ _TM_ _STM_ _SSTM_;
		else
			wordseps = wordseps0;
		for (int ii = 1; ii <= nn; ii++) {
			var tt = string2.substr(ii, 1);

			if (inquotes) {
				inquotes = tt != inquotes;
			} else {
				if (tt == DQ && (string2.count(DQ) > 1 || tt == L"\'")
						&& string2.count(L"\'") > 1) {
					inquotes = tt;
				} else {
					if (wordseps.index(tt, 1)) {
						cap = 1;
						if (tt == L" ")
							numx = var(L"1234567890").index(
									string2.substr(ii + 1, 1), 1);
					} else {
						if (cap || numx) {
							tt.converter(LOWERCASE, UPPERCASE);
							string2.splicer(ii, 1, tt);
							cap = 0;
						} else {
							tt.converter(UPPERCASE, LOWERCASE);
							string2.splicer(ii, 1, tt);
						}
					}
				}
			}

		};	//ii;

		string2.swapper(L"\'S ", L"\'s ");
		if (string2.substr(-2, 2) == L"\'S")
			string2.splicer(-2, 2, L"\'s");

	} else if (mode0 == L"QUOTE") {
		string2 = str0;
		if (string2 != L"") {
			string2.converter(FM ^ VM ^ SVM ^ TM, L"    ");
			string2.swapper(L" ", L"\" \"");
			string2 = string2.quote();
		}

	} else if (mode0 == L"UPPERCASE") {
		string2 = str0;
		string2.converter(LOWERCASE, UPPERCASE);

	} else if (mode0 == L"LOWERCASE") {
		string2 = str0;
		string2.converter(UPPERCASE, LOWERCASE);


	} else if (mode0.substr(1, 5) == L"PARSE") {

		var uppercase = mode0.index(L"UPPERCASE", 1);

		string2 = str0;

		//convert to uppercase
		var quoted = L"";
		for (int ii = 1; ii <= 99999; ii++) {
			var tt = string2.substr(ii, 1);
			//BREAK;
			if (!(tt != L""))
				break;;
			if (tt == quoted) {
				quoted = L"";
			} else {
				if (!quoted) {
					if ((DQ ^ L"\'").index(tt, 1)) {
						quoted = tt;
					} else {
						if (tt == L" ") {
							tt = FM;
							string2.splicer(ii, 1, tt);
						} else {
							if (uppercase) {
								tt.converter(LOWERCASE, UPPERCASE);
								string2.splicer(ii, 1, tt);
							}
						}
					}
				}
			}
		};	//ii;

		if (mode0.index(L"TRIM", 1)) {
			string2.converter(L" " _FM_, _FM_ L" ");
			string2 = string2.trim();
			string2.converter(L" " _FM_, _FM_ L" ");
		}

	}

	return string2;

}

var ExodusProgramBase::execute(const var& sentence) {

	var v1,v2,v3,v4;
	pushselect(v1, v2, v3, v4);

	var sentence2=sentence.fieldstore(L" ",1,1,sentence.field(L" ",1).lcase());

	var result=perform(sentence);

	popselect(v1, v2, v3, v4);

	return result;

}

void ExodusProgramBase::chain(const var& libraryname) {
	CHAIN=libraryname;
	var().stop();
}

var ExodusProgramBase::perform(const var& sentence) {
	//THISIS(L"var ExodusProgramBase::perform(const var& sentence)")
	//ISSTRING(sentence)

	//return ID^L"*"^dictid;

	//wire up the the library linker to have the current mvenvironment
	//if (!perform_exodusfunctorbase_.mv_)
	//	perform_exodusfunctorbase_.mv_=this;

	//lowercase all library functions to aid in conversion from arev
	//TODO remove after conversion complete

	perform_exodusfunctorbase_.mv_ = (&mv);

	//save some environment
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
	while (SENTENCE) {

		//set new perform environment
		COMMAND = SENTENCE;
		OPTIONS = "";
		//similar code in exodus_main() and mvprogram.cpp:perform()
		var lastchar=COMMAND[-1];
		if (lastchar==")") {
			OPTIONS=L"(" ^ COMMAND.field2(L"(",-1);
		}
		else if (lastchar=="}")
			OPTIONS=L"{" ^ COMMAND.field2(L"{",-1);
		if (OPTIONS)
			COMMAND.splicer(-(OPTIONS.length()),OPTIONS.length(), L"");

		//load the shared library file
		var libid = SENTENCE.field(L" ", 1).lcase();
		std::string str_libname = libid.toString();
		if (!perform_exodusfunctorbase_.initsmf(
			str_libname.c_str(),
			"exodusprogrambasecreatedelete_",
			true //forcenew each perform/execute
			)) {
			USER4^=L"perform() Cannot find shared library \"" ^ str_libname
				^ L"\", or \"libraryexit()\" is not present in it.";
			//throw MVException(USER4);
			//return "";
			break;
		}

		//call the shared library exodus object's main function
		try {
			ANS=perform_exodusfunctorbase_.callsmf();
		}
		catch (const MVUndefined& e) {
			//if return "" is missing then default ANS to ""
			ANS=L"";
		}
		catch (const MVStop& e) {
			//stop is normal way of stopping a perform
			//functions can call it to terminate the whole "program"
			//without needing to setup chains of returns
			//to exit from nested functions
			ANS=L"";
		}
		catch (const MVAbort& e) {
			//similar to stop for the time being
			//maybe it should set some error flag/messages
			ANS=L"";
		}
		catch (const MVAbortAll& e) {
			//similar to stop for the time being
			//maybe it should set some error flag/messages
			//and abort multiple levels of perform?
			ANS=L"";
		}

		//chain is a kind of optional follow controlled by the library function
		// to perform another function after the first
		//go round again if anything in CHAIN
		CHAIN.transfer(SENTENCE);

	}

	//restore some environment
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

var ExodusProgramBase::xlate(const var& filename, const var& key, const var& fieldno_or_name, const var& mode) {

	//TODO implement additional MV argument

	//key can be multivalued and multivalued result will be returned
	var results="";
	var nkeys=key.dcount(VM);

	for (var keyn=1;keyn<=nkeys;++keyn) {

		var keyx=key.a(1,keyn);

		//handle non-numeric field_no ie dictionary field/column name
		if (not fieldno_or_name.isnum()) {

			//get the whole record
			var record=keyx.xlate(filename, L"", mode);

			//TODO what if key is multivalued?

			//handle record not found and mode C
			if (mode == L"C" && record == keyx) {
				results.r(keyn,key);
				continue;
			}

			//handle record not found and mode X
			if (not record.length()) {
				results.r(keyn,L"");
				continue;
			}

			//use calculate()
			var result=calculate(fieldno_or_name,L"dict_" ^ filename.a(1), keyx, record);
			if (nkeys>1)
				result.lowerer();
			results.r(keyn,result);
			continue;

		}

		//ordinary numeric or "" fieldno
		results.r(keyn,keyx.xlate(filename, fieldno_or_name, mode));
	}

	return results;
}

var ExodusProgramBase::calculate(const var& dictid, const var& dictfile, const var& id, const var& record, const var& mvno) {

	//TODO how is it possible to exchange when the variable is const?

	DICT.exchange(dictfile);
	ID.exchange(id);
	RECORD.exchange(record);
	MV.exchange(mvno);

	var result=calculate(dictid);

	DICT.exchange(dictfile);
	ID.exchange(id);
	RECORD.exchange(record);
	MV.exchange(mvno);

	return result;
}

var ExodusProgramBase::calculate(const var& dictid) {
	//THISIS(L"var ExodusProgramBase::calculate(const var& dictid)")
	//ISSTRING(dictid)

	//return ID^L"*"^dictid;

	//wire up the the library linker to have the current mvenvironment
	//if (!dict_exodusfunctorbase_.mv_)
		dict_exodusfunctorbase_.mv_ = (&mv);

	//get the dictionary record so we know how to extract the correct field or call the right library
	bool newlibfunc;
	bool indictmd=false;
	if (cache_dictid_ != dictid) {
		newlibfunc = true;
		if (not DICT)
			throw MVException(
					L"calculate("
					^ dictid
					^ L") DICT file variable has not been set");
		if (not cache_dictrec_.reado(DICT, dictid)) {
			//try lower case
			if (not cache_dictrec_.reado(DICT, dictid.lcase())) {

				//try dict_voc
				var dictmd;//TODO implement DICTMD to avoid opening
				if (not dictmd.open("dict_voc")) {
baddict:
					throw MVException(
						L"calculate("
						^ dictid
						^ L") dictionary record not in DICT "
						^ DICT.quote());
				}
				if (not cache_dictrec_.reado(dictmd, dictid)) {
					//try lower case
					if (not cache_dictrec_.reado(dictmd, dictid.lcase())) {
						goto baddict;
					}
				}
				indictmd=true;
			}
		}
		cache_dictid_ = dictid;
	} else
		newlibfunc = false;

	var dicttype = cache_dictrec_(1);
	bool ismv=cache_dictrec_(4)[1]==L"M";

	//F type dictionaries
	if (dicttype == L"F") {

		//check field number is numeric
		var fieldno = cache_dictrec_(2);
		if (!fieldno.isnum())
			return L"";

		//field no > 0
		if (fieldno) {
			if (ismv)
				return RECORD(fieldno, MV);
			else
				return RECORD(fieldno);

		//field no 0
		} else {
			var keypart = cache_dictrec_(5);
			if (keypart && keypart.isnum())
				return ID.field(L"*", keypart);
			else
				return ID;

		}

	} else if (dicttype == L"S") {
		//TODO deduplicate various exodusfunctorbase code spread around calculate mvipc* etc
		if (newlibfunc) {
			std::string str_libname;
			if (indictmd)
				str_libname = "dict_voc";
			else
				str_libname = DICT.a(1).lcase().convert(L".",L"_").toString();
			std::string str_funcname = (L"exodusprogrambasecreatedelete_" ^ dictid.lcase()).toString();
			if (!dict_exodusfunctorbase_.initsmf(str_libname.c_str(),str_funcname.c_str()))
				throw MVException(
						L"calculate() Cannot find Library " ^ str_libname
								^ L", or function " ^ dictid.lcase()
								^ L" is not present");
		}

		//for single valued fields, inform the called routine that MV is 0
		int savedMV;
		if (!ismv) {
			savedMV=MV;
			MV=0;
		}

		//return dict_exodusfunctorbase_.calldict();
		//return ANS;

		//define a function type (pExodusProgramBaseMemberFunction)
		//that can call the shared library object member function
		//with the right arguments (none for dicts) and returning a var
		typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

		//call the shared library object main function with the right args (none for dicts), returning a var
		//std::cout<<"precall"<<std::endl;
		ANS =
				CALLMEMBERFUNCTION(*(dict_exodusfunctorbase_.pobject_),
						((pExodusProgramBaseMemberFunction) (dict_exodusfunctorbase_.pmemberfunction_)))();
		//std::cout<<"postcall"<<std::endl;

		//restore the MV if necessary
		if (!ismv)
			MV=savedMV;

		return ANS;

	}

	throw MVException(
			L"calculate(" ^ dictid ^ L") " ^ DICT ^ L" Invalid dictionary type "
					^ dicttype.quote());
	return L"";

}

bool ExodusProgramBase::unlockrecord(const var& filename, const var& file0, const var& key) const {
	var file;
	if (file0.unassigned())
		file = L"";
	else
		file = file0;

	if (file == L"") {
		var().unlockall();
		return 1;
	}

	//remove persistentlock
	//common /shadow.mfs/ shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3
	//done by shadow.mfs

	//remove actual lock
	file.unlock(key);

	return 1;

	//evade warning: unused parameter
	if (filename) {
	}
	return 1;
}


void ExodusProgramBase::debug() const {

	var reply;
	std::wcout << L"debug():";
	if (SLASH == L"/")
		asm("int $3");
	//cin>>reply;
	return;

}

bool ExodusProgramBase::fsmsg(const var& msg) const {
	mssg(msg ^ var().getlasterror());
	return false;
}

var ExodusProgramBase::sysvar(const var& var1, const var& var2, const var& var3,
		const var& var4) {

	std::wcout << L"sysvar() do nothing:";
//	var reply;
//	cin>>reply;
	return L"";

	//evade warning: unused parameter
	if (var1 || var2 || var3 || var4) {
	}
}

void ExodusProgramBase::setprivilege(const var& var1) {

	PRIVILEGE = var1;
	std::wcout << L"setprivilege(" << var1 << L") do nothing" << std::endl;
//	var reply;
//	cin>>reply;
	return;

}

var ExodusProgramBase::decide(const var& question, const var& options) const {
	var reply = L"";
	var buffer;
	return decide(question, options, reply, buffer);
}

var ExodusProgramBase::decide(const var& questionx, const var& optionsx, var& reply,
		const int defaultreply) const {

	var interactive = !SYSTEM.a(33);

	var options = optionsx;
	var question = questionx;

	options.converter(VM ^ L"|", FM ^ FM);
	if (!interactive) {
		if (defaultreply) {
			reply = defaultreply;
		} else {
			reply = 1;
		}
		return options.a(reply);
	}

	question.converter(VM ^ L"|", FM ^ FM);
	question.swapper(FM, var().chr(13) ^ var().chr(10));
	std::wcout << question << std::endl;

//	var noptions = options.count(FM) + (options != L"");
	var noptions = options.dcount(FM);
	for (int optionn = 1; optionn <= noptions; optionn++)
		std::wcout << optionn << L". " << options.a(optionn) << std::endl;

	inp: reply.input();
	if (reply == L"" || reply >= 1 || reply <= noptions) {
		return reply;
	} else {
		goto inp;
	}

	return L"";

}

void ExodusProgramBase::savescreen(var& origscrn, var& origattr) const {
	std::wcout << L"ExodusProgramBase::savescreen not implemented" << std::endl;

	//evade warning: unused parameter
	if (origscrn || origattr) {
	}

}

//NO STANDARD C/C++ way to test for key pressed or chars in input buffer!
//SEE http://c-faq.com/osdep/kbhit.txt (but 1996) for info
// Checks keyboard buffer (stdin) and returns key
// pressed, or -1 for no key pressed
int ExodusProgramBase::keypressed(int delayusecs) const {
	/* will not compile on mingw because unlike cygwin
	 * mingw is close to windows and windows select only works on sockets
	 * should be easy to reimplement in another way
	 *wchar_t keypressed;
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

	//evade warning: unused parameter
	if (delayusecs) {
	}

}

bool ExodusProgramBase::esctoexit() const {
	wchar_t keypress = keypressed();
	if (keypress != 0x1B) {
//		cin.putback(keypress);
		return false;
	}

	std::wcout << L"Paused. Press Enter to resume or Ctrl+C to cancel:"
			<< std::endl;

	while (true) {
		keypress = keypressed(1000000);
		if (keypress == 0x1B)
			return true;
	}

//	keypress=cin.peek();
//	if (keypress==0x1B) return true;

	return false;
}

var ExodusProgramBase::otherusers(const var& param) {
	std::wcout << L"ExodusProgramBase::otherusers not implemented yet";
	return var(L"");

	//evade warning: unused parameter
	if (param) {
	}
}

var ExodusProgramBase::otherdatasetusers(const var& param) {
	std::wcout << L"ExodusProgramBase::otherdatausers not implemented yet";
	return var(L"");

	//evade warning: unused parameter
	if (param) {
	}
}

bool ExodusProgramBase::lockrecord(const var& filename, const var& file, const var& keyx) const {
	var record;
	return lockrecord(filename, file, keyx, record);
}

bool ExodusProgramBase::lockrecord(const var& filename, const var& file, const var& keyx, const var& recordx, const int waitsecs0, const bool allowduplicate) const {

	//linemark
	//common /shadow.mfs/ shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3

	//nb case where we do not wish to wai
	//wait
	//0 fail immediately (eg looking for next batch/nextkey)
	//9 wait for 9 seconds
	//- wait for a default number of seconds
	// * wait infinite number of seconds
	//if index(file,'message',1) else de bug
	int waitsecs = waitsecs0;

	lock:
		var locked=file.lock(keyx);
		if (locked || allowduplicate && locked eq L"") {
			return 1;
		} else {
			if (waitsecs) {
				var().ossleep(1000);
				waitsecs -= 1;
				goto lock;
			}
		return false;
	}

	//evade warning: unused parameter
	(false||filename||recordx);

	return true;

}

var ExodusProgramBase::singular(const var& pluralnoun) {

	var temp = pluralnoun;

	if (temp.substr(-2, 2) == L"ES") {

		//companies=company
		if (temp.substr(-3, 3) == L"IES") {
			temp.splicer(-3, 3, L"Y");

			//addresses=address

		} else if (temp.substr(-4, 4) == L"SSES") {
			temp.splicer(-2, 2, L"");

		} else if (temp.substr(-4, 4) == L"SHES") {
			temp.splicer(-2, 2, L"");

		} else if (temp.substr(-4, 4) == L"CHES") {
			temp.splicer(-2, 2, L"");

		} else if (1) {
			temp.splicer(-1, 1, L"");
		}

	} else {

		if (temp[-1] == L"S") {
			//analysis, dos
			if (temp.substr(-2, 2) != L"IS" && temp.substr(-2, 2) != L"OS")
				temp.splicer(-1, 1, L"");
		}

	}

	return temp;

}

void ExodusProgramBase::flushindex(const var& filename) {
	std::wcout << L"ExodusProgramBase::std::flushindex not implemented yet, "
			<< filename << std::endl;
	return;
}

var ExodusProgramBase::encrypt2(const var& encrypt0) const {

	var encrypt = encrypt0;
	var encryptkey = 1234567;

	//pass1
	while (true) {
		//BREAK;
		if (!(encrypt != L""))
			break;;
		encryptkey = (encryptkey % 390001) * (encrypt[1]).seq() + 1;
		encrypt.splicer(1, 1, L"");
	}	//loop;

	//pass2
	while (true) {
		encrypt ^= var().chr(65 + (encryptkey % 50));
		encryptkey = (encryptkey / 50).floor();
		//BREAK;
		if (!encryptkey)
			break;;
	}	//loop;

	return encrypt;

}

var ExodusProgramBase::xmlquote(const var& string0) const {

	var string1;

	if (string0.unassigned()) {
		//de bug
		string1 = L"UNASSIGNED";
	} else {
		string1 = string0;
	}

	string1.swapper(L"&", L"&amp;");
	string1.swapper(DQ, L"&quot;");
	string1.swapper(L"<", L"&lt;");
	string1.swapper(L">", L"&gt;");

	string1.converter(DQ, L"\'");
	string1.swapper(VM, L"\" \"");
	string1.swapper(FM, L"\" \"");
	return string1.quote();

}

var ExodusProgramBase::loginnet(const var& dataset, const var& username,
		var& cookie, var& msg) {

	//evade warning: unused parameter
	if (false && dataset) {
	}

	var menuid;
	var usern;
	var menun;
	var xx;

	//this is a custom login routine called from listen2
	cookie = L"";
	var menus;
	if (!menus.open(L"ADMENUS")) {
		if (!menus.open(L"MENUS") && username != L"NEOSYS") {
			msg = L"Error: Cannot open MENUS file";
			return false;
		}
	}

	//return allowable menus
	if (username == L"NEOSYS") {
		menuid = L"ADAGENCY";

	} else {
		if (!(SECURITY.a(1).locate(username, usern))) {
			msg = L"Error: " ^ username.quote() ^ L" user is missing";
			return false;
		}
		menuid = SECURITY.a(3, usern);
	}

	var menu = L"";
	if (!menu.read(menus, menuid)) {
		if (username == L"NEOSYS") {
			if (!menu.read(menus, L"NEOSYS")) {
				menu =
						FM ^ FM ^ FM ^ FM ^ FM
								^ L"MEDIA|ADPRODUCTION|ACCS|ANALMENU|TIMESHEETS|FILESMENU|GENERAL|EXIT2";
				menu = menu.converter(L"|", VM);
			}
		}
	}
	if (!menu) {
		msg = L"Error: " ^ menuid.quote() ^ L" menu is missing";
		return false;
	}

	var menucodes = menu.a(6) ^ VM ^ L"HELP";
	//remove local support menu
	if (!authorised(L"SUPPORT MENU ACCESS", msg, L"LS")) {
		if (menucodes.a(1).locate(L"GENERAL", menun))
			menucodes.eraser(1, menun, 0);
	}
	menucodes.converter(VM ^ L".", L",_");

	//prepare session cookie
	cookie = L"m=" ^ menucodes;

	return true;

	/* custom login per application
	 var compcode = L"";

	 var temp;
	 if (!(temp.read(companies, L"%RECORDS%"))) {
	 companies.select();
	 temp = L"";
	 nextcomp:
	 var compcodex;
	 if (var(L"").readnext(compcodex)) {
	 temp.replace(-1, 0, 0, compcodex);
	 goto nextcomp;
	 }
	 }

	 if (APPLICATION ne L"ACCOUNTS") {
	 for (int ii = 1; ii <= 9999; ii++) {
	 compcode = temp.a(ii);
	 //until validcode('company',compcode)
	 //BREAK;
	 if (validcode2(compcode, L"", L"")) break;;
	 };//ii;
	 }else{
	 compcode = temp.a(1);
	 }

	 if (!compcode) {
	 msg = L"Error: You are not authorised to access any companies";
	 return false;
	 }

	 var tempcompany;
	 if (!tempcompany.read(companies, compcode)) {
	 msg = L"Error: " ^ compcode.quote() ^ L" company code is missing";
	 return;
	 }

	 company = L"";
	 initcompany(compcode);

	 force error here TODO: check trigraph following;
	 var defmarketcode = (company.a(30)) ? (company.a(30)) : (agp.a(37));
	 //if unassigned(markets) then markets=''

	 //markets is not open in finance only module
	 //readv maincurrcode from markets,defmarketcode,5 else maincurrcode=''
	 var maincurrcode = L"";
	 if (FILES[0].locateusing(FM,L"MARKETS", FM))
	 maincurrcode = defmarketcode.xlate(L"MARKETS", 5, L"X");

	 if (maincurrcode.unassigned())
	 maincurrcode = L"";
	 if (maincurrcode == L"")
	 maincurrcode = basecurrency;

	 cookie ^= L"&cc=" ^ compcode;
	 cookie ^= L"&pd=" ^ currperiod ^ L"/" ^ addcent(curryear);
	 cookie ^= L"&bc=" ^ basecurrency;
	 cookie ^= L"&bf=" ^ USER2;
	 cookie ^= L"&mk=" ^ defmarketcode;
	 cookie ^= L"&mc=" ^ maincurrcode;
	 temp = SYSTEM.a(23);
	 temp.swap(L"&", L" and ");
	 cookie ^= L"&db=" ^ temp;

	 backupreminder(dataset, msg);

	 changelogsubs(L"WHATSNEW" ^ FM ^ menucodes);
	 cookie ^= L"&wn=" ^ ANS;

	 */

}

var ExodusProgramBase::AT(const int code) const {
	//should depend on terminal type
	return var().at(code);
}

var ExodusProgramBase::AT(const int x, const int y) const {
	//should depend on terminal type
	return var().at(x, y);
}

var ExodusProgramBase::handlefilename(const var& handle) {
	return handle.a(1);
}

var ExodusProgramBase::memspace(const var& requiredmemory) {
	return 999999999;
}

var ExodusProgramBase::getuserdept(const var& usercode) {
	//locate the user in the list of users
	var usern;
	if (!(SECURITY.a(1).locate(usercode, usern))) {
		if (usercode == L"NEOSYS") {
			ANS = L"NEOSYS";
			return ANS;
		} else {
			ANS = L"";
			return ANS;
		}
	}

	//locate divider, or usern+1
	var nusers = (SECURITY.a(1)).count(VM) + 1;
	var usernx;
	for (usernx = 1; usernx <= nusers; usernx++) {
		//BREAK;
		if (SECURITY.a(1, usernx) == L"---")
			break;;
	};	//usern;

	//get the department code
	ANS = SECURITY.a(1, usernx - 1);

	return ANS;

}

var ExodusProgramBase::oconv(const var& input, const var& conversion) {

	//call user conversion routine
	//almost identical code in var::oconv and var::iconv
	//ENSURE synchronised if you change it

	var result=input;
	var ptr=1;
	var delimiter;
	do {

		var subconversion=conversion.remove(ptr,delimiter);

		//either call custom conversion routines
		if (subconversion[1] == L"[") {

			//extract any params
			var mode=subconversion.field(L",",2,9999).field(L"]",1);

			//set the function name
			ioconv_custom=subconversion.substr(2).field(L",",1).field(L"]",1).lcase();

			//wire up the current environment
			ioconv_custom.mv_ = (&mv);

			//and call it
			var output;
			call ioconv_custom(L"OCONV",result,mode,output);
			result=output;

		//or call standard conversion methods
		} else {
			result=result.oconv(subconversion);
		}
	} while (delimiter);

	return result;
}

var ExodusProgramBase::iconv(const var& input, const var& conversion) {

	//call user conversion routine
	//almost identical code in var::iconv and var::iconv
	//ENSURE synchronised if you change it

	var result=input;
	var ptr=1;
	var delimiter;
	do {

		var subconversion=conversion.remove(ptr,delimiter);

		//either call custom conversion routines
		if (subconversion[1] == L"[") {

			//extract any params
			var mode=subconversion.field(L",",2,9999).field(L"]",1);

			//set the function name
			ioconv_custom=subconversion.substr(2).field(L",",1).field(L"]",1).lcase();

			//wire up the current environment
			ioconv_custom.mv_ = (&mv);

			//and call it
			var output;
			call ioconv_custom(L"ICONV",result,mode,output);
			result=output;

		//or call standard conversion methods
		} else {
			result=result.iconv(subconversion);
		}
	} while (delimiter);

	return result;
}

}//of namespace exodus


