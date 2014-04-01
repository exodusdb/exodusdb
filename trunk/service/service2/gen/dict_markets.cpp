#include <exodus/library.h>

libraryinit(authorised)

#include <validcode3.h>
function main()  {
	//ALLOWPARTIALACCESS
	var xx,msg;
	return validcode3(ID, "", "", xx, msg);
}
libraryexit(authorised)


libraryinit(company_code)

function main()  {
	//markets currently dont belong to companies
	return "";
}
libraryexit(company_code)


libraryinit(currency_name)

function main()  {
	return RECORD.a(5).xlate("CURRENCIES", 1, "X");
}
libraryexit(currency_name)


libraryinit(rawname)

function main()  {
	ANS = RECORD.a(1);
	ANS.ucaser();
	ANS.converter("\'" " \".,/-", "");
	return ANS;
}
libraryexit(rawname)


libraryinit(year_period)
function main()  {
	return "";

}

libraryexit(year_period)
