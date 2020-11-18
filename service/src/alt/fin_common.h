#include <exodus/common.h>

//only needed in sys/initcompany until fin.base_currency_code, fin,maxperiod etc. can be removed

#define fin_common_no 3

commoninit(fin,fin_common_no)

	var accounts;
	var balances;
	var vouchers;
	var voucherindex;
	var curryear;
	var currperiod;
	var currfmt;
	var credit;
	var unused_currcompany;
	var currfromyear;
	var currfromperiod;
	var currtoyear;
	var currtoperiod;
	var fromyear;
	var toyear;
	var fromperiod;
	var toperiod;
	var unused_abp;
	var ledgers;
	var charts;
	var batches;
	var basecurrency;
	var basefmtx;
	var currcurrency;
	var converted;
	var gendesc;
	var taxes;
	var statmtypes;
	var maxperiod;
	var account;
	var definition;
	var unused_alanguage;
	var settings;
	var currcompanycodes;
	var accparams;
	var unused_1;
	var unused_2;
	var unused_3;
	var hexdatesize;

commonexit(fin,fin_common_no)
