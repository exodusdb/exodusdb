/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
//#include <windows.h>
#include "Market.h"
using namespace exodus;

var Market::operator() (const var& dictid)
{
    //locate the dictionary entry by id
    Dict::const_iterator it=dict.find(dictid.tostring());

    //throw if not found
    if (it!=dict.end())
	{
	/*
		MarketFunction function=it->second;

		//note the syntax of calling a pointer to a member function requires both
		//an object in this case "*this" AND the actual member function pointer "*function" AND the parameter list "()"
		return (*this.*function)();
		return (this->*function)();
	*/
		//for speed do it in one step like this
		return (this->*(it->second))();
	
	}

	if (dictid=="PREREAD")
		return "";
	else if (dictid=="POSTREAD")
		return "";

	return "";

}

//constructor builds the dict
//Market::Market(MvEnvironment* env) : env(*env)
Market::Market() : env(*global_environments[getenvironmentn()])
{
	dict["CODE"]=&Market::code;
	dict["NAME"]=&Market::name;
	dict["POPULATION"]=&Market::population;
	dict["SEQ"]=&Market::seq;
	dict["VEHICLE_CODE"]=&Market::vehicle_code;
	dict["CURRENCY_CODE"]=&Market::currency_code;
	dict["DATE_TIME"]=&Market::date_time;
	dict["STOPPED"]=&Market::stopped;
	dict["VERSION"]=&Market::version;
	dict["USERNAME_UPDATED"]=&Market::username_updated;
	dict["DATETIME_UPDATED"]=&Market::datetime_updated;
	dict["STATION_UPDATED"]=&Market::station_updated;
	dict["VERSION_UPDATED"]=&Market::version_updated;
	dict["STATUS_UPDATED"]=&Market::status_updated;
//	dict["AUTHORISED"]=&Market::authorised;
	dict["CURRENCY_NAME"]=&Market::currency_name;
	dict["RAWNAME"]=&Market::rawname;
}

var Market::code()
{
	return env.ID;
}
var Market::name()
{
	return env.RECORD.extract(1);
}
var Market::population()
{
	return env.RECORD.extract(2);
}
var Market::seq()
{
	return env.RECORD.extract(3);
}
var Market::vehicle_code()
{
	return env.RECORD.extract(4, env.MV);
}
var Market::currency_code()
{
	return env.RECORD.extract(5);
}
var Market::date_time()
{
	return env.RECORD.extract(6);
}
var Market::stopped()
{
	return env.RECORD.extract(25);
}
var Market::version()
{
	return env.RECORD.extract(26);
}
var Market::username_updated()
{
	return env.RECORD.extract(30, env.MV);
}
var Market::datetime_updated()
{
	return env.RECORD.extract(31, env.MV);
}
var Market::station_updated()
{
	return env.RECORD.extract(32, env.MV);
}
var Market::version_updated()
{
	return env.RECORD.extract(33, env.MV);
}
var Market::status_updated()
{
	return env.RECORD.extract(34, env.MV);
}
var Market::authorised()
{	//allowpartialaccess
	var msg;
//TODO:	env.ANS = validcode3(env.ID, "", "", "", msg);
	env.ANS=1;
	return env.ANS;
}
var Market::currency_name()
{
	env.ANS = env.RECORD.extract(5).xlate("CURRENCIES", 1, "X");
	return env.ANS;
}
var Market::rawname()
{
	env.ANS = env.RECORD.extract(1);
	env.ANS.converter(env.LOWERCASE, env.UPPERCASE);
	env.ANS.converter("\'" " \".,/-", "");
	return env.ANS;
}

/*
	var msg;

dict["PREWRITE"]=&Market::prewrite;
dict["POSTWRITE"]=&Market::null;
dict["POSTDELETE"]=&Market::null;

dict["POSTINIT"]=&Market::postinit_postread_predelete;
dict["POSTREAD"]=&Market::postinit_postread_predelete;
dict["PREDELETE"]=&Market::postinit_postread_predelete;

var Market::prewrite()
{
	//update version log
	generalsubs2(mode);
}

var Market::null()
{
}

var Market::postinit_postread_predelete()
{

		//option to read previous versions
		generalsubs2(mode);
		if (!common_valid)
			return;

		//GOSUB SECURITY
		//this is done to allow LISTEN to avoid special MARKET ACCESS PARTIAL logic
		var pass = 0;
		if (mode == "POSTREAD" and not not _SYSTEM.extract(33) and common_orec) {
			//check allowed to access this ledger
			if (authorised("#MARKET ACCESS " ^ (_ID.quote()), msg, ""))
				pass = 1;
		}

		//OP='COMPANY FILE'
		//GOSUB SECURITY2
		if (!pass)
			gosub_security();

		if (mode == "PREDELETE" and _RECORD.extract(4)) {
			cout << var().chr(7);
			msg = "PLEASE DELETE THE VEHICLES FOR THIS MARKET|BEFORE DELETING THE MARKET RECORD";
			mssg(msg);
			common_valid = 0;
			return;
		}

	}else if (1) {
		msg = mode.quote() ^ " - unknown mode skipped in MARKET.SUBS";
		goto EOF_349;

	}
L386:
	return;

} // /:~
*/
