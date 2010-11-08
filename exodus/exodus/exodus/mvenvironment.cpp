/*
Copyright (c) 2009 Stephen John Bush

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#define EXO_MVENVIRONMENT_CPP
#include <exodus/mvenvironment.h>

//avoid this unless absolutely necessary then possible to move this file out of exodus var library
//(stick to throwing MVException with a suitable error message)
//#include <exodus/mvexceptions.h>

namespace exodus
{

//MUST be the same length
const var INTERNALCHARS=L"\x11\x12\x13\x14\x15\x16\x17";
const var EXTERNALCHARS=_SSTM_ _STM_ _TM_ _SM_ _VM_ _FM_ _RM_;

//TODO Should not be global otherwise cannot multithread MvEnvironment

//NB do not define default copy constructor and assignment in order to force
//derived classes to implement them since they are defined in the class header
MvEnvironment::MvEnvironment()
{

	this->USERNAME=L"";
	this->ACCOUNT=L"";
	this->AW=L"";
	this->LOWERCASE=_LOWER_CASE;
	this->UPPERCASE=_UPPER_CASE;
	this->SYSTEM=L"";
	this->SENTENCE=L"";
	this->STATION=L"";
	this->PRIVILEGE=L"";
	this->USER0=L"";
	this->USER1=L"";
	this->USER2=L"";
	this->USER3=L"";
	this->USER4=L"";
	this->FILES=L"";
	this->TCLSTACK=L"";
	this->ROLLOUTFILE=L"";
	this->INTCONST=L"";
	this->PRIORITYINT=L"";
	this->DEFINITIONS=L"";
	this->SECURITY=L"";
	this->LISTACTIVE=0;

}

//destructor
MvEnvironment::~MvEnvironment()
{

	//std::wcout<<L"MVProcess: Closing Definitions ... "<<std::flush;
	if (this->DEFINITIONS.assigned())
	{
		this->DEFINITIONS.close();
	}
	//std::wcout<<L"OK"<<std::endl;

	//std::wcout<<L"MVProcess: Disconnecting DB ... "<<std::flush;
	if (this->SESSION.assigned())
	{
		this->SESSION.close();
	}
	//std::wcout<<L"OK"<<std::endl;

	//std::wcout<<L"MvEnvironment: Closing Definitions ... "<<std::flush;
	if (this->DEFINITIONS.assigned()&&this->DEFINITIONS)
	{
		this->DEFINITIONS.close();
	}
	//std::wcout<<L"OK"<<std::endl;

	//std::wcout<<L"MvEnvironment: Disconnecting DB ... "<<std::flush;
	if (this->SESSION.assigned()&&this->SESSION)
	{
		this->SESSION.close();
	}
	//std::wcout<<L"OK"<<std::endl;

}

bool MvEnvironment::init(const var& threadno)
{
	//std::wcout<<L"MvEnvironment::init("<<threadno<<L")"<<std::endl;
	//mvprocess

	this->USERNAME=L"";
	this->ACCOUNT=L"";
	this->STATION=L"";
	this->ROLLOUTFILE=L"~"^threadno^L".$$$";
	this->THREADNO=threadno;
	this->SYSTEM.replacer(17,0,0,L"");

	cache_dictid_="";
	this->DICT="";

	return true;

}

var MvEnvironment::calculate(const var& dictid)
{
	//THISIS(L"var MvEnvironment::calculate(const var& dictid)")
	//ISSTRING(dictid)

	//return ID^L"*"^dictid;

	//wire up the the library linker to have the current mvenvironment
	if (!exodusfunctorbase_.mv_)
		exodusfunctorbase_.mv_=this;

	//get the dictionary record so we know how to extract the correct field or call the right library
	bool newlibfunc;
	if (cache_dictid_!=dictid)
	{
		newlibfunc=true;
		if (not DICT)
			throw MVException(L"calculate("^dictid^L") DICT file variable has not been set");
		if (not cache_dictrec_.read(DICT,dictid))
			throw MVException(L"calculate("^dictid^L") dictionary record not in DICT "^DICT.quote());
		cache_dictid_=dictid;
	}
	else
		newlibfunc=false;

	var dicttype=cache_dictrec_(1);

	//F type dictionaries
	if (dicttype==L"F")
	{

		//check field number is numeric
		var fieldno=cache_dictrec_(2);
		if (!fieldno.isnum())
			return L"";

		//field no > 0
		if (fieldno)
			return RECORD(fieldno,MV);

		//field no 0
		else
		{
			var keypart=cache_dictrec_(5);
			if (keypart&&keypart.isnum())
				return ID.field(L"*",keypart);
			else
				return ID;

		}
	}
	else if (dicttype==L"S")
	{
		//TODO deduplicate various exodusfunctorbase code spread around calculate mvipc* etc
		if (newlibfunc)
		{
			std::string str_libname=DICT.tostring();
			std::string str_funcname=dictid.tostring();
			if (!exodusfunctorbase_.init(str_libname.c_str(),str_funcname.c_str()))
				throw MVException(L"calculate() Cannot find Library "^str_libname^L", or function "^str_funcname^L" is not present");
		}

		exodusfunctorbase_.calldict();
		return ANS;
	}

	throw MVException(L"calculate("^dictid^L") "^DICT^L" Invalid dictionary type "^dicttype.quote());
	return L"";

}

bool MvEnvironment::lockrecord(const var& filename, const var& file, const var& keyx, const var& recordx, const var& waitsecs0) const
{

	//linemark
	//common /shadow.mfs/ shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3

	var waitsecs;
	if (waitsecs0.unassigned())
		waitsecs = 0;
	else waitsecs=waitsecs0;

	//nb case where we do not wish to wai
	//wait
	// 0 fail immediately (eg looking for next batch/nextkey)
	// 9 wait for 9 seconds
	// - wait for a default number of seconds
	// * wait infinite number of seconds
	//if index(file,'message',1) else de bug
lock:
	if (file.lock( keyx)) {
		return 1;
	}else{
		if (waitsecs) {
			var().ossleep(1000);
			waitsecs -= 1;
			goto lock;
		}
		return 0;
	}

	//should not get here
	return 0;

}

bool MvEnvironment::unlockrecord(const var& filename, const var& file0, const var& key) const
{
	var file;
	if (file0.unassigned())
		file = L"";
	else file=file0;

	if (file == L"") {
		var().unlockall();
		return 1;
	}

	//remove persistentlock
	//common /shadow.mfs/ shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3
	//done by shadow.mfs

	//remove actual lock
	file.unlock( key);

	return 1;

}


var MvEnvironment::capitalise(const var& str0, const var& mode0, const var& wordseps0) const
{

	var string2;
	var mode;
	//j b a s e
	if (mode0.unassigned())
		mode = L"CAPITALISE";
	else mode=mode0;

	if (mode == L"QUOTE") {
		string2 = str0;
		if (string2 != L"") {
			string2.converter(FM ^VM ^SVM ^TM, L"    ");
			string2.swapper(L" ", L"\" \"");
			string2 = string2.quote();
		}

	}else if (mode == L"UPPERCASE") {
		string2 = str0;
		string2.converter(this->LOWERCASE, this->UPPERCASE);

	}else if (mode == L"LOWERCASE") {
		string2 = str0;
		string2.converter(this->UPPERCASE, this->LOWERCASE);

	}else if (mode == L"CAPITALISE") {

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
		else wordseps=wordseps0;
		for (int ii = 1; ii <= nn; ii++) {
			var tt = string2.substr(ii, 1);

			if (inquotes) {
				inquotes = tt != inquotes;
			}else{
				if (tt == DQ && string2.count(DQ) > 1 || tt == L"\'" && string2.count(L"\'") > 1) {
					inquotes = tt;
				}else{
					if (wordseps.index(tt, 1)) {
						cap = 1;
						if (tt == L" ")
							numx = var(L"1234567890").index(string2.substr(ii +1, 1), 1);
					}else{
						if (cap || numx) {
							tt.converter(this->LOWERCASE, this->UPPERCASE);
							string2.splicer(ii, 1, tt);
							cap = 0;
						}else{
							tt.converter(this->UPPERCASE, this->LOWERCASE);
							string2.splicer(ii, 1, tt);
						}
					}
				}
			}

		};//ii;

		string2.swapper(L"\'S ", L"\'s ");
		if (string2.substr(-2, 2) == L"\'S")
			string2.splicer(-2, 2, L"\'s");

	}else if (mode.substr(1, 5) == L"PARSE") {

		var uppercase = mode.index(L"UPPERCASE", 1);

		string2 = str0;

		//convert to uppercase
		var quoted = L"";
		for (int ii = 1; ii <= 99999; ii++) {
			var tt = string2.substr(ii, 1);
		//BREAK;
		if (!(tt != L"")) break;;
			if (tt == quoted) {
				quoted = L"";
			}else{
				if (!quoted) {
					if ((DQ ^ L"\'").index(tt, 1)) {
						quoted = tt;
					}else{
						if (tt == L" ") {
							tt = FM;
							string2.splicer(ii, 1, tt);
						}else{
							if (uppercase) {
								tt.converter(this->LOWERCASE, this->UPPERCASE);
								string2.splicer(ii, 1, tt);
							}
						}
					}
				}
			}
		};//ii;

		if (mode.index(L"TRIM", 1)) {
			//string2.converter(L"\x20\xFE", L"\xFE\x20");
			string2.converter(L" " _FM_, _FM_ L" ");
			string2 = string2.trim();
			//string2.converter(L"\x20\xFE", L"\xFE\x20");
			string2.converter(L" " _FM_, _FM_ L" ");
		}

	}

	return string2;

}

void MvEnvironment::note(const var& msg, const var& options, var& buffer, const var& params) const
{
	note2(msg,options,buffer,params);
}

void MvEnvironment::note2(const var& msg0, const var& options, var& buffer, const var& params) const
{

	var msg=msg0.trimf('!');
	msg.converter(TM, FM);
	mssg(msg, options, buffer, params);
	return;

}

void MvEnvironment::mssg(const var& msg) const
{
	var buffer=L"";
	mssg(msg, L"", buffer, L"");
}

void MvEnvironment::mssg(const var& msg, const var& options) const
{
	var buffer=L"";
	mssg(msg, options, buffer, L"");
}

void MvEnvironment::msg2(const var& msg, const var& options, var& buffer, const var& params) const
{
	mssg(msg, options, buffer, params);
}

void MvEnvironment::mssg(const var& msg0, const var& options0, var& response, const var& params0) const
{

	var interactive;
	var xx;

	if (msg0.unassigned())
		//msg = L"";
		return;
	var msg=msg0;
	var options=options0;
	var params=params0;
	/* automatic params
	if (options.unassigned())
		options = L"";
	if (response.unassigned())
		response = L"";
	if (params.unassigned())
		params = L"";
	*/

	interactive = !this->SYSTEM.extract(33);

	//suggested problem report example

	//change the osfilename to a revelation file name
	if (options.index(L"[FS124]", 1))
		goto damaged;
	if (options.index(L"[FS125]", 1))
		goto damaged;
	if (options.index(L"[FS126]", 1))
		goto damaged;
	if (options.index(L"[FS127]", 1))
		goto damaged;
	if (options.index(L"[FS128]", 1))
		goto damaged;
	if (options.index(L"[FS129]", 1))
		goto damaged;
	if (var(L"FS124" _VM_ L"FS125" _VM_ L"FS126" _VM_ L"FS127" _VM_ L"FS128" _VM_ L"FS129").locate(msg, xx, 1)) {

		//30/6/2001
		msg = msg.extract(1, 1, 1).xlate(L"SYS.MESSAGES", 11, L"C");

		goto damaged;
	}
	if (options.index(L"DAMAGED FILE", 1)) {
damaged:
		var osfile = params.extract(1, 1);
		if (osfile.substr(-3, 1) == L".")
			osfile.splicer(-3, 3, L"");
		var filename = handlefilename(osfile);
		//filename:=' - ':params<1,1>
		if (filename)
			params.replacer(1, 1, 0, filename);
		if (!interactive) {
			response = var().chr(27);
			options.swapper(L"%1%", filename);
			options.swapper(L"%2%", params.extract(1, 2));

			//30/6/2001
			if (!(msg.index(FM, 1) || msg.index(VM, 1))) {
				var msg2 = msg.extract(1, 1, 1).xlate(L"SYS.MESSAGES", 11, L"X");
				if (!(msg2))
					msg2 = msg.extract(1, 1, 1).xlate(L"SYSMESSAGES", 11, L"X");
				if (msg2)
					msg = msg2;
			}
			msg.swapper(L"%1%", filename);
			msg.swapper(L"%2%", params.extract(1, 2));

		}
	}

	//if not interactive
	if (!interactive) {

//		//option to break out of infinite loop
//		xx.input(-1, 0);
//		if (xx == var().chr(27)) {
//			if (this->USERNAME == L"NEOSYS")
//				debug();
//			var().stop();
//		}

		//catch rev trying to get user input to correct a word
		if (var(L"W156,W159,W600,W601").locateusing(msg, L",", xx)) {
			var msg2 = msg.extract(1, 1, 1).xlate(L"SYSMESSAGES", 11, L"X");
			if (msg2)
				msg = msg2 ^ L" " ^response;
			//must be w156 space word to be handled in select2
			this->USER4 = msg ^ L" " ^response;
			var().stop();
		}

		//catch rev trying to get user input to correct a word
		//if msg='w556' then
		// *must be w156 space word to be handled in select2
		// @user4='w156 ':response:' invalid word'
		// print time()
		// return
		// end

		if (msg == L"") {
			msg = options.extract(11);
			options = options.extract(1);
		}
		var options2 = options;
		if (options2 == L"")
			options2 = msg.extract(1, 1, 1).xlate(L"SYS.MESSAGES", 1, L"X");
		var options3 = options2;
		options3.converter(L"UDWT", L"");
		if (options3 != options2) {
			goto msgrti;
		}else{
			if (!(msg.index(FM, 1) || msg.index(VM, 1))) {
				var msg2 = msg.extract(1, 1, 1).xlate(L"SYS.MESSAGES", 11, L"X");
				if (!(msg2))
					msg2 = msg.extract(1, 1, 1).xlate(L"SYSMESSAGES", 11, L"X");
				if (msg2)
					msg = msg2;
			}
			msg.swapper(L"%1%", params.extract(1));
			msg.swapper(L"%2%", params.extract(2));
			msg.swapper(L"%3%", params.extract(3));
			//force esc if too many messages
			if (this->USER4.length() > 8000) {
				msg.outputl();
				// response=wchar_t(27)
				// return
				var().stop();
			}else{
				//@user4:=@fm:msg
				this->USER4.replacer(-1, 0, 0, msg);
			}
		}
		return;
	}

msgrti:

	if (options.index(L"U", 1) || options.index(L"D", 1)) {
		if (!interactive) {
			if (options.index(L"U", 1)) {
				msg.output();
				var(L" ...").output();
			}else{
				var(L"done").outputl();
			}
			return;
		}
	}else{
		//if interactive then print wchar_t(7)
	}

	msgbase(msg, options, response, params);

	return;

}

void MvEnvironment::msgbase(const var& msg, const var& options, const var& response, const var& params) const
{

	std::wcout<<msg;
	return;

}

var MvEnvironment::handlefilename(const var& handle) const
{

	var handle2;
	var handle3;
	var files;

	handle2 = handle.field(VM, 2, 999);
	handle3 = (handle.field2(var().chr(13), -1)).substr(13, 9999).field(VM, 1, 1);
	if (handle3.substr(-3, 1) == L".")
		handle3.splicer(-3, 3, L"");
	handle3 ^= L".LK";

	if (!files.open(L"FILES")) {
		mssg(L"HANDLEFILENAME CANNOT OPEN THE \"FILES\" FILE");
//TODO:		var().stop();
        return L"";
	}

	var nfiles = (this->FILES).count(FM) + 1;
	for (int filen = 1; filen <= nfiles; filen++) {
		var filename = this->FILES.extract(filen);
		var file;
		if (file.read(files, filename)) {

			if (file.extract(5)) {

				//passed a filehandle
				if (file.extract(5) == handle2)
					return filename;

				//passed a pathname to the osfile
				if ((file.extract(5)).index(handle3, 1))
					return filename;

			}

		}
	};//filen;

	return L"";

}

void MvEnvironment::note(const var& msg, const var& options) const
{
	var buffer=L"";
	note(msg, options, buffer, L"");
}

void MvEnvironment::note(const var& msg) const
{

	//linemark
	//this should never be called with more than one parameters
	//but to avoid run time error allow four params
	var buffer;
	note2(msg,L"",buffer,L"");
	return;

}

void MvEnvironment::debug() const
{

	var reply;
	std::wcout<<L"debug():";
	//cin>>reply;
	return;

}

void MvEnvironment::fsmsg() const
{

	std::wcout<<L"fsmsg():";
//	var reply;
//	cin>>reply;
	return;

}

var MvEnvironment::sysvar(const var& var1,const var& var2,const var& mv3,const var& var4)
{

	std::wcout<<L"sysvar() do nothing:";
//	var reply;
//	cin>>reply;
	return L"";

}

void MvEnvironment::setprivilege(const var& var1) {

	this->PRIVILEGE=var1;
	std::wcout<<L"setprivilege("<<var1<<L") do nothing"<<std::endl;
//	var reply;
//	cin>>reply;
	return;

}

bool MvEnvironment::openfile(const var& filename0, var& file) const
{
	if (filename0.unassigned())
		throw MVException(L"filename0");
	var filename=filename0;
	var xx;
	
	var nomsg = filename[1] == L"*";
	if (nomsg)
		filename.splicer(1, 1, L"");
open:
	if (file.open(filename)) {

		if (!(this->FILES.locateusing(filename, FM, xx)))
			this->FILES.replacer(-1, 0, 0, filename);

		return 1;
	}else{
		if (filename == L"MD") {
			filename = L"MD";
			goto open;
		}
		if (!nomsg) {
			std::wcout << var().chr(7);
			mssg(L"The " ^ filename.quote() ^ L" file is missing");
		}
		file = L"";
		return 0;
	}

}

bool MvEnvironment::openfile2(const var& filename, var& file, const var& similarfilename, const var& autocreate) const
{

	var reply;
	//if (autocreate.unassigned())
	//	autocreate = 1;
	var firsttry = 1;
tryagain:

	if (openfile(L"*" ^ filename, file))
		return 1;

	if (firsttry) {
		//user option to create file if it does not exist
		var tt;

        if (file.createfile(filename)){};

        firsttry = 0;
		goto tryagain;
	}

	std::wcout << var().chr(7);
	var temp=L"THE " ^ filename.quote() ^ L" FILE IS MISSING";
	mssg(temp);
	file = L"";
	return 0;


}

var MvEnvironment::decide(const var& question, const var& options) const
{
	var reply=L"";
	var buffer;
	return decide(question,options,reply,buffer);
}

var MvEnvironment::decide(const var& question, const var& options, var& reply) const
{
	var buffer;
	return decide(question,options,reply,buffer);
}

var MvEnvironment::decide (const var& questionx, const var& optionsx, var& reply, var& buffer) const
{

	var interactive = !this->SYSTEM.extract(33);

	var options = optionsx;
	var question = questionx;

	options.converter(VM ^ L"|", FM ^ FM);
	if (!interactive) {
		if (buffer) {
			reply = buffer;
		}else{
			reply = 1;
		}
		return options.extract(reply);
	}

	question.converter(VM ^ L"|", FM ^ FM);
	question.swapper(FM, var().chr(13) ^ var().chr(10));
	std::wcout << question << std::endl;

//	var noptions = options.count(FM) + (options != L"");
	var noptions = options.dcount(FM);
	for (int optionn = 1; optionn <= noptions; optionn++)
		std::wcout << optionn<< L". "<< options.extract(optionn) << std::endl;

inp:
	reply.input();
	if (reply == L"" || reply >= 1 || reply <= noptions) {
		return reply;
	}else{
		goto inp;
	}

	return L"";

}

void MvEnvironment::savescreen(var& origscrn, var& origattr) const
{
	std::wcout<<L"MvEnvironment::savescreen not implemented"<<std::endl;
}

// Checks keyboard buffer (stdin) and returns key
// pressed, or -1 for no key pressed
int MvEnvironment::keypressed(int delayusecs) const
{
/* will not compile on mingw because unlike cygwin
 * mingw is close to windows and windows select only works on sockets
 * should be easy to reimplement in another way
    wchar_t keypressed;
    struct timeval waittime;
    int numthis->charsthis->read;
    struct fdthis->set mask;
    FDthis->SET(0, &mask);

    waittime.tvthis->sec = 0;
    waittime.tvthis->usec = delayusecs;
    if (select (1, &mask, 0, 0, &waittime))
    {
        numthis->charsthis->read = read (0, &keypressed, 1);
        if (numthis->charsthis->read == 1)
        {
           	cin.putback(keypressed);
            return keypressed;
        }
    }
*/
    return 0;
}

bool MvEnvironment::esctoexit() const
{
	wchar_t keypress=keypressed();
	if (keypress!=0x1B)
	{
//		cin.putback(keypress);
		return false;
	}

	std::wcout<<L"Paused. Press Enter to resume or Ctrl+C to cancel:"<<std::endl;

	while (true)
	{
		keypress=keypressed(1000000);
		if (keypress==0x1B) return true;
	}

//	keypress=cin.peek();
//	if (keypress==0x1B) return true;

	return false;
}

var MvEnvironment::otherusers(const var& param)
{
	std::wcout<<L"MvEnvironment::otherusers not implemented yet";
	return var(L"");
}

var MvEnvironment::otherdatasetusers(const var& param)
{
	std::wcout<<L"MvEnvironment::otherdatausers not implemented yet";
	return var(L"");
}

bool MvEnvironment::lockrecord(const var& filename, const var& file, const var& keyx) const
{
	var record;
	return lockrecord(filename, file, keyx, record);
}

bool MvEnvironment::lockrecord(const var& filename, const var& file, const var& keyx, const var& recordx, const int waitsecs0) const
{

	//linemark
	//common /shadow.mfs/ shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3

	//nb case where we do not wish to wai
	//wait
	//0 fail immediately (eg looking for next batch/nextkey)
	//9 wait for 9 seconds
	//- wait for a default number of seconds
	//* wait infinite number of seconds
	//if index(file,'message',1) else de bug
	int waitsecs=waitsecs0;

lock:
	if (file.lock( keyx)) {
		return 1;
	}else{
		if (waitsecs) {
			var().ossleep(1000);
			waitsecs -= 1;
			goto lock;
		}
		return false;
	}

	return true;

}

void MvEnvironment::osbreadx(var& str1, const var& filehandle, const var& filename, const int startoffset, const int length)
{
	//convert from external to internal "codepage"
	//park the high characters in the low place
	str1.osbread(filehandle,startoffset,length).converter(EXTERNALCHARS, INTERNALCHARS);
	return;
}

void MvEnvironment::oswritex(const var& str1, const var& filename) const
{

	//convert from internal to external "codepage"
	//move parked characters to high characters
	//(any field seps in the output will already have been escaped like %fe)
	str1.convert(INTERNALCHARS, EXTERNALCHARS).oswrite(filename);

	return;

}

void MvEnvironment::osbwritex(const var& str1, const var& filehandle, const var& filename, const int offset) const
{

	//convert from internal to external "codepage"
	//move parked characters to high characters
	//(any field seps in the output will already have been escaped like %fe)
	str1.convert(INTERNALCHARS, EXTERNALCHARS).osbwrite(filehandle,offset);

	return;

}

bool MvEnvironment::authorised(const var& task0)
{
	var msg;
	return authorised(task0, msg, L"");
}

bool MvEnvironment::authorised(const var& task0, var& msg, const var& defaultlock0)
{

	var task=task0;
	var defaultlock=defaultlock0;

	var dost;
	dost=var().ostime();
	//std::wcout<<dost<<std::endl;

	var noadd;
	var positive;
	var isneosys;//num
	var deleting;
	var taskn;
	var locks;
	var nlocks;
	var usern;
	var keys;
	var temp;//num

	//if @username='neosys' or @username='steve' then call msg(task:'')

	if (task[1] == L" ")
		mssg(task.quote());
	//each task may have many "locks", each users may have many "keys"
	//a user must have keys to all the locks in order to pass

	if (!task)
		goto ok;

	task.ucaser();
	task.converter(RM ^ FM ^ VM ^ SVM, L"\\\\\\");
	task.swapper(L" FILE ", L" ");
	task.swapper(L" - ", L" ");
	task.converter(L".", L" ");
	task = task.trim();

	msg = L"";
	//**call note(' ':task)

	if (task.substr(1, 2) == L"..") {
		// call note(task:'')
		return 1;
	}

	noadd = task[1] == L"!";
	if (noadd)
		task.splicer(1, 1, L"");
	//if noadd else noadd=((task[-1,1]='"') and (len(userprivs)<10000))
	if (!noadd) {
		var lenuserprivs = this->SECURITY.length();
		noadd = task[-1] == DQ || lenuserprivs > 48000;
	}
	positive = task[1] == L"#";
	if (positive)
		task.splicer(1, 1, L"");

	//? as first character of task (after positive) means
	//security is being used as a configuration and user neosys has no special privs
	if (task[1] == L"?") {
		isneosys = 0;
		task.splicer(1, 1, L"");
	}else{
		isneosys = this->USERNAME == L"NEOSYS";
	}

	deleting = task.substr(1, 8) == L"%DELETE%";
	if (deleting)
		task.splicer(1, 8, L"");

	//find the task
//	std::wcout<<this->SECURITY<<std::endl;
//	std::wcout<<task<<std::endl;
//	std::wcout<<VM<<std::endl;
	if (this->SECURITY.locate(task, taskn, 10)) {
		if (deleting) {
			this->SECURITY.eraser(10, taskn);
			this->SECURITY.eraser(11, taskn);
			writeuserprivs();
			return 1;
		}
	}else{
		if (!noadd) {
			readuserprivs();
			if (this->USERNAME == L"NEOSYS")
				note(task ^ L"|TASK ADDED");
			if (this->SECURITY.length() < 65000) {
				if (!(this->SECURITY.locateby(task, L"AL", taskn, 10)))
				{
					if (defaultlock.unassigned())
						defaultlock = L"";
					this->SECURITY.inserter(10, taskn, 0, task);
					this->SECURITY.inserter(11, taskn, 0, defaultlock);
					writeuserprivs();
				}
			}
		}
	}

	//if no locks then pass ok unless positive locking required
	locks = this->SECURITY.extract(11, taskn);
	if (locks == L"") {
		if (positive && !isneosys) {
notallowed:
			msg = capitalise(task) ^ L"||Sorry, L" ^ capitalise(this->USERNAME) ^ L", you are not authorised to do this->|";
			return 0;
		}else{
			goto ok;
		}
	}else{
		if (locks == L"NOONE")
			goto notallowed;
	}

	//if index('012',@privilege,1) then goto ok
	if (isneosys)
		goto ok;

	//find the user (add to bottom if not found)
	//surely this is not necessary since users are in already
	if (!(this->SECURITY.locate(this->USERNAME, usern, 1))) {
		if (this->USERNAME != L"NEOSYS" && this->USERNAME != this->ACCOUNT) {
			readuserprivs();
			usern = (this->SECURITY.extract(1)).dcount(VM) + 1;
			if (this->SECURITY.length() < 65000) {
				this->SECURITY.inserter(1, usern, 0, this->USERNAME);
				this->SECURITY.inserter(2, usern, 0, L"");
				writeuserprivs();
			}
		}
	}

	//user must have all the keys for all the locks on this task
	//following users up to first blank line also have the same keys
	keys = (this->SECURITY.extract(2)).field(VM, usern, 999);
	temp = keys.index(L"---", 1);
	if (temp)
		keys.splicer(temp - 1, 999, L"");
	//convert vm to ',' in keys
	keys.converter(L",", VM);
	locks.converter(L",", VM);
	nlocks = locks.count(VM) + 1;

	for (int lockn = 1; lockn <= nlocks; lockn++) {
		var lockx = locks.field(VM, lockn, 1);
		if (keys.locate(lockx, temp, 1)) {
	//call note(task:' ok')
		}else{
			msg = capitalise(task) ^ L"||Sorry, L" ^ capitalise(this->USERNAME) ^ L", you are not authorised to do this->|";
	//call note(task:' ko')
			return 0;
		}
	};//lockn;

ok:
	//call statup(2,3,task)
	return 1;

}

void MvEnvironment::readuserprivs()
{
	//if definitions then
	//put back in case called from fileman due to no datasets
	//if definitions then
	if (!this->SECURITY.read(this->DEFINITIONS, L"SECURITY"))
		this->SECURITY = L"";
	//this->SECURITY = this->SECURITY.invert();
	// end
	// end
	return;

}

void MvEnvironment::writeuserprivs()
{
	this->SECURITY.replacer(9, 0,0, L"");
	//if definitions then
	//put back in case called from fileman due to no datasets
	//if definitions then
	//this->SECURITY.invert().write(this->DEFINITIONS, L"SECURITY");
	// end
	// end
	return;

}

void MvEnvironment::logger(const var& programname0, const var& text0)
{

	var log;
	var time;
	var programname=programname0;
	var text=text0;
	//turn off interactivity. why?
	var s33 = this->SYSTEM.extract(33);
	this->SYSTEM.replacer(33, 0, 0, L"");

	if (programname.unassigned())
		programname = L"";
	if (text.unassigned())
		text = L"";

	var year = ((var().date()).oconv(L"D")).substr(-4, 4);
	if (openfile2(L"LOG" ^ year, log, L"LISTS")) {
		var entry = programname;
		var text2 = text.lower();
		text2.converter(L"|", VM);
		entry.replacer(2, 0, 0, text2);

		if (s33) {
			var station = this->SYSTEM.extract(40, 2);
		}else{
			var station = this->STATION.trim();
		}

getlogkey:
		time=var().ostime();
		var logkey = this->STATION.trim() ^ L"*" ^ this->USERNAME ^ L"*" ^ var().date() ^ L"*" ^ time;
		var xx;
		if (xx.read(log, logkey))
			goto getlogkey;

		entry.replacer(2, 0, 0, entry.extract(2).trim(VM));
		entry.write(log, logkey);
	}

	//restore interactivity
	this->SYSTEM.replacer(33, 0, 0, s33);

	return;

}

var MvEnvironment::singular(const var& pluralnoun)
{

	var temp = pluralnoun;

	if (temp.substr(-2, 2) == L"ES") {

		//companies=company
		if (temp.substr(-3, 3) == L"IES") {
			temp.splicer(-3, 3, L"Y");

			//addresses=address

		}else if (temp.substr(-4, 4) == L"SSES") {
			temp.splicer(-2, 2, L"");

		}else if (temp.substr(-4, 4) == L"SHES") {
			temp.splicer(-2, 2, L"");

			}else if (temp.substr(-4, 4) == L"CHES") {
			temp.splicer(-2, 2, L"");

		}else if (1) {
			temp.splicer(-1, 1, L"");
		}

	}else{

		if (temp[-1] == L"S") {
			//analysis, dos
			if (temp.substr(-2, 2) != L"IS" && temp.substr(-2, 2) != L"OS")
				temp.splicer(-1, 1, L"");
		}

	}

	return temp;

}

void MvEnvironment::flushindex(const var& filename)
{
	std::wcout<<L"MvEnvironment::std::flushindex not implemented yet, "<<filename<<std::endl;
	return;
}

void MvEnvironment::sysmsg(const var& msg0)
{

	var tt;
	var ver;

	//logs a message and sends it to all the technical support emails (backup)
	//do not call msg or note etc, otherwise may be recursive

	var datasetcode = this->SYSTEM.extract(17);

	//protect the msg
	var msg = msg0;

	//log the system message first in case sendmail fails
	logger(L"SYSMSG", msg);

	//get backup parameters
	var bakpars;
	if (!bakpars.read(this->DEFINITIONS, L"BACKUP"))
		bakpars = L"";
	//dos backup.cfg overrides
	if (tt.osread("BACKUP.CFG")) {
		for (int ii = 1; ii <= 99; ii++) {
			if (tt.extract(ii))
				bakpars.replacer(ii, 0, 0, tt.extract(ii));
		};//ii;
	}

	//get technical emailaddrs to send to
	//nb if any emailaddrs and neosys.com not in them
	//then neosys will not receive any message
	var emailaddrs = bakpars.extract(6);
	if (bakpars.extract(10)) {
		if (emailaddrs)
			emailaddrs ^= L"/";
		emailaddrs ^= bakpars.extract(10);
	}
	emailaddrs = emailaddrs.field(L"/", 1, 1);
	//if emailaddrs='' then readv emailaddrs from definitions,'replication',12 else emailaddrs=''
	//if dir('neosys.id') and @username='neosys' then emailaddrs='backups@neosys.com'
	//if dir('neosys.id') then emailaddrs='backups@neosys.com'
	emailaddrs.swapper(L"backups@neosys.com", L"sysmsg@neosys.com");
	if (emailaddrs == L"")
		emailaddrs = L"sysmsg@neosys.com";

	//determine subject
	var subject = L"NEOSYS System Message : Datasetcode=" ^ datasetcode;

	var body = L"Server=" ^ (this->SYSTEM.extract(44)).trim();
	body.replacer(-1, 0, 0, L"Client=" ^ this->STATION.trim());
	body.replacer(-1, 0, 0, L"User=" ^ this->USERNAME.trim());
	var temp = this->USER0;
	temp.converter(RM ^ FM ^ VM ^ SVM ^ TM ^ STM, L"`^]}\\~");
	body.replacer(-1, 0, 0, L"Request=" ^ temp);
	body.replacer(-1, 0, 0, L"Message=" ^ FM ^ msg);
	temp = this->USER1;
	temp.converter(RM ^ FM ^ VM ^ SVM ^ TM ^ STM, L"`^]}\\~");
	if (ver.osread("general\\version.dat"))
		body.replacer(-1, 0, 0, L"NEOSYS Ver:" ^ ver.extract(1));
	body.replacer(-1, 0, 0, L"@Id=" ^ this->ID);
	body.replacer(-1, 0, 0, L"Data=" ^ temp);
	body.converter(FM ^ VM ^ SVM ^ TM ^ STM ^ L"|", L"\r\r\r\r\r\r");
	body.swapper(L"\r", L"\r\n");

	var deleteaftersend = L"";

	var errormsg = L"";

	//sendmail - if it fails, there will be an entry in the log
	sendmail(emailaddrs, subject, body, L"", deleteaftersend, errormsg);

	return;

}

var MvEnvironment::sendmail(const var& toaddress0, const var& subject, const var& body0, const var& attachfilename, const var& deletex, var& errormsg)
{

	var params2;
	var bodyfilename;
	var toaddress=toaddress0;

	if (this->SENTENCE.field(L" ", 1, 1) == L"SENDMAIL") {
		toaddress = this->SENTENCE.field(L" ", 2, 1);
		if (!toaddress)
			toaddress = L"steve.bush@neosys.com";
		this->SENTENCE = L"";
		sendmail(toaddress, L"test email test subject", L"test body line 1\rtest body line2", L"", 0, errormsg);
		if (errormsg)
			mssg(errormsg);
		var().stop();
	}

	//sendmail is silent
	errormsg = L"";

	//send mail requires confirmation if user is neosys
	//cannot because backup in neosys.net would be interrupted
	//declare MvLib decide
	//if @username='neosys' then
	// s33=system<33>
	// q='you are neosys.|send mail to ':toaddress:'|':subject:'|':body0
	// convert \0d0a\ to @fm in q
	// if decide(q,'',reply) else reply=2
	// if reply=1 else return
	// system<33>=s33
	// end

	var params1 = L"";
	var definitions;
	if (definitions.open(L"DEFINITIONS")) {
		if (!(params1.read(definitions, L"SMTP.CFG")))
			{}
	}

	//global override
	params2.osread("SMTP.CFG");
	//cut off after end of file character
	params2 = params2.field(var().chr(26), 1, 1);
	params2.swapper(L"\r\n", L"\r");
	//params2.converter(L"\r\n", L"\xFE\xFE");
	params2.converter(L"\r\n", _FM_ _FM_);
	for (int ii = 1; ii <= 9; ii++) {
		if (params2.extract(ii))
			params1.replacer(ii, 0, 0, params2.extract(ii));
	};//ii;

	var params = L"";
	params.replacer(-1, 0, 0, L"fromaddress=" ^ params1.extract(1));
	params.replacer(-1, 0, 0, L"smtphostname=" ^ params1.extract(2));
	params.replacer(-1, 0, 0, L"smtpportno=" ^ params1.extract(3));
	params.replacer(-1, 0, 0, L"smtptimeoutsecs=" ^ params1.extract(4));
	params.replacer(-1, 0, 0, L"smtpusessl=" ^ params1.extract(5));
	params.replacer(-1, 0, 0, L"smtpauthtype=" ^ params1.extract(6));
	params.replacer(-1, 0, 0, L"smtpuserid=" ^ params1.extract(7));
	params.replacer(-1, 0, 0, L"smtppassword=" ^ params1.extract(8));

	var body = body0;

	if (body.index(L" ", 1) || body.length() > 10 || body.index(L"\r", 1) || body.index(L"\n", 1)) {
		bodyfilename = var(99999999).rnd() ^ L".TXT";
		body.oswrite(bodyfilename);
		bodyfilename.osclose();
		body = L"@" ^ bodyfilename;
	}else{
		body.converter(DQ, L"\'");
		bodyfilename = L"";
	}

	var paramfilename = var(99999999).rnd() ^ L".TXT";

	var errorfilename = var(99999999).rnd() ^ L".$$$";

	var cmd = L"START /w";
	//option to de-bug
	//cmd=' wscript //x'
	cmd ^= L" sendmail.js /e " ^ errorfilename ^ L" /p " ^ paramfilename;

	//params='/t ':quote(toaddress):' /s ':quote(subject):' /b ':quote(body):' /a ':quote(attachfilename)
	//if delete then params:=' /d ':delete
	params.replacer(-1, 0, 0, L"toaddress=" ^ toaddress.quote());
	params.replacer(-1, 0, 0, L"subject=" ^ subject.quote());
	params.replacer(-1, 0, 0, L"body=" ^ body.quote());
	if (attachfilename)
		params.replacer(-1, 0, 0, L"attachfilename=" ^ attachfilename.quote());
	if (deletex)
		params.replacer(-1, 0, 0, L"deleteaftersend=" ^ deletex.quote());
	params ^= FM;

	params.swapper(FM, L"\r\n");
	params.oswrite(paramfilename);

	cmd.osshell();

	if (bodyfilename)
		bodyfilename.osdelete();

	paramfilename.osdelete();

	if (errormsg.osread(errorfilename)) {
		errorfilename.osdelete();
		//errormsg.converter(L"\r\n", L"\xFE\xFE");
		errormsg.converter(L"\r\n", _FM_ _FM_);
		errormsg = errormsg.oconv(L"T#60");
	}else{
		errormsg = L"Unknown error in sendmail.js Failed to complete";
		errormsg.replacer(-1, 0, 0, cmd);
		errormsg.replacer(-1, 0, 0, params.oconv(L"T#60"));
	}
	errormsg.converter(TM, FM);

	if ((errormsg.extract(1)).trim() != L"OK") {
		logger(L"SENDMAIL", errormsg);
		return 0;
	}

	return 1;

}

var MvEnvironment::encrypt2(const var& encrypt0) const
{

	var encrypt=encrypt0;
	var encryptkey = 1234567;

	//pass1
	while (true) {
	//BREAK;
	if (!(encrypt != L"")) break;;
		encryptkey = (encryptkey % 390001) * (encrypt[1]).seq() + 1;
		encrypt.splicer(1, 1, L"");
	}//loop;

	//pass2
	while (true) {
		encrypt ^= var().chr(65 + (encryptkey % 50));
		encryptkey = (encryptkey / 50).floor();
	//BREAK;
	if (!encryptkey) break;;
	}//loop;

	return encrypt;

}

var MvEnvironment::xmlquote(const var& string0) const
{

	var string1;

	if (string0.unassigned()) {
		//de bug
		string1 = L"UNASSIGNED";
	}else{
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

var MvEnvironment::loginnet(const var& dataset, const var& username, var& cookie, var& msg)
{

	var menuid;
	var usern;
	var menun;
	var xx;

	//this is a custom login routine called from listen2
	cookie = L"";
	var menus;
	if (!menus.open(L"ADMENUS")) {
		if (!menus.open(L"MENUS")&&username != L"NEOSYS") {
			msg = L"Error: Cannot open MENUS file";
			return false;
		}
	}

	//return allowable menus
	if (username == L"NEOSYS") {
		menuid = L"ADAGENCY";

	}else{
		if (!(this->SECURITY.locate(username, usern, 1 ))) {
			msg = L"Error: " ^ username.quote() ^ L" user is missing";
			return false;
		}
		menuid = this->SECURITY.extract(3, usern);
	}

	var menu = L"";
    if (!menu.read(menus, menuid)) {
		if (username == L"NEOSYS") {
			if (!menu.read(menus, L"NEOSYS"))
			{
                    menu=FM^FM^FM^FM^FM^L"MEDIA|ADPRODUCTION|ACCS|ANALMENU|TIMESHEETS|FILESMENU|GENERAL|EXIT2";
                    menu=menu.converter(L"|",VM);
            }
		}
	}
	if (!menu) {
		msg = L"Error: " ^ menuid.quote() ^ L" menu is missing";
		return false;
	}

	var menucodes = menu.extract(6) ^ VM ^ L"HELP";
	//remove local support menu
	if (!authorised(L"SUPPORT MENU ACCESS", msg, L"LS")) {
		if (menucodes.locate(L"GENERAL", menun, 1))
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

	if (this->ACCOUNT == L"ADAGENCY") {
		for (int ii = 1; ii <= 9999; ii++) {
			compcode = temp.extract(ii);
			//until validcode('company',compcode)
		//BREAK;
		if (validcode2(compcode, L"", L"")) break;;
		};//ii;
	}else{
		compcode = temp.extract(1);
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
	var defmarketcode = (company.extract(30)) ? (company.extract(30)) : (agp.extract(37));
	//if unassigned(markets) then markets=''

	//markets is not open in finance only module
	//readv maincurrcode from markets,defmarketcode,5 else maincurrcode=''
	var maincurrcode = L"";
	if (this->FILES[0].locateusing(L"MARKETS", FM, xx))
		maincurrcode = defmarketcode.xlate(L"MARKETS", 5, L"X");

	if (maincurrcode.unassigned())
		maincurrcode = L"";
	if (maincurrcode == L"")
		maincurrcode = basecurrency;

	cookie ^= L"&cc=" ^ compcode;
	cookie ^= L"&pd=" ^ currperiod ^ L"/" ^ addcent(curryear);
	cookie ^= L"&bc=" ^ basecurrency;
	cookie ^= L"&bf=" ^ this->USER2;
	cookie ^= L"&mk=" ^ defmarketcode;
	cookie ^= L"&mc=" ^ maincurrcode;
	temp = this->SYSTEM.extract(23);
	temp.swap(L"&", L" and ");
	cookie ^= L"&db=" ^ temp;

	backupreminder(dataset, msg);

	changelogsubs(L"WHATSNEW" ^ FM ^ menucodes);
	cookie ^= L"&wn=" ^ this->ANS;

*/

}

var MvEnvironment::at(const int code) const
{
	//should depend on terminal type
	return var().at(code);
}

var MvEnvironment::at(const int x, const int y) const
{
	//should depend on terminal type
	return var().at(x, y);
}


var MvEnvironment::getuserdept(const var& usercode)
{
	//locate the user in the list of users
	var usern;
	if (!(this->SECURITY.locate(usercode, usern, 1))) {
		if (usercode == L"NEOSYS") {
			this->ANS = L"NEOSYS";
			return this->ANS;
		}else{
			this->ANS = L"";
			return this->ANS;
		}
	}

	//locate divider, or usern+1
	var nusers = (this->SECURITY.extract(1)).count(VM) + 1;
	var usernx;
	for (usernx = 1; usernx <= nusers; usernx++) {
	//BREAK;
	if (this->SECURITY.extract(1, usernx) == L"---") break;;
	};//usern;

	//get the department code
	this->ANS = this->SECURITY.extract(1, usernx - 1);

	return this->ANS;

}

}//of namespace exodus
