// DONT use headerguard since may required multiple times eg in dict_xxxxx.cpp files
//#ifndef SERVICE_SRC_SRV_SRV_COMMON_H_
//#define SERVICE_SRC_SRV_SRV_COMMON_H_

#include <exodus/common.h>

//#define srv_common_no 0
const int srv_common_no = 0;

commoninit(srv, srv_common_no)

	var address;
	var addresses;
	var gcurrcompcode = "";
	var alanguage;
	var company;
	var companies;
	var currency;
	var currencies;
	var document;
	var documents;
//	var unit;
//	var units;
	var markets;
//	var _security;
	var glang;
//	var _definitions;
//	var accounting = "";
	var currency_codes = "";
	var currency_convs = "";

commonexit(srv, srv_common_no)

//#endif // SERVICE_SRC_SRV_SRV_COMMON_H_
