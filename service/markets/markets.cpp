#include <neo.h>
using namespace neo;

function(AUTHORISED)
	Mv msg;
//TODO:	env.ANS = validcode3(env.ID, "", "", "", msg);
	env.ANS=1;
	return env.ANS;
functionexit

function(CURRENCY_NAME)
	env.ANS = env.RECORD.extract(5).xlate("CURRENCIES", 1, "X");
	return env.ANS;
functionexit

function(RAWNAME)
	env.ANS = env.RECORD.extract(1);
	env.ANS.converter(env.LOWERCASE, env.UPPERCASE);
	env.ANS.converter("\'" " \".,/-", "");
	return env.ANS;
functionexit

function(PREREAD)
	return "";
functionexit

function(POSTREAD)
	return "";
functionexit

function(PREWRITE)
	return "";
functionexit

function(POSTWRITE)
	return "";
functionexit

function(PREDELETE)
	return "";
functionexit

function(POSTDELETE)
return "";
functionexit

libraryexit
