#ifndef NEO_AGY_H

#include <exodus/common.h>

class agy_common : public LabelledCommon{
public:

agy_common(){
	tsmvfns="1" _VM_ "100" _VM_ "101" _VM_ "3" _VM_ "4" _VM_ "2";
}

var schedules="";
var plans="";
var clients="";
var vehicles="";
var ratecards="";
var markets="";
var brands="";
var agp="";
var jobtypes="";
var invoices="";
var analysis="";
var bookingorders="";
var jobs="";
var productioninvoices="";
var productionorders="";
var suppliers="";
var materials="";
var ratings="";
var unusedxyzz="";
var ads="";
var surveys="";
var validcodes="";
var tsmvfns="";

};

#define agy_common_no 4
//#define agy (*((agy_common*) mv.labelledcommon[agy_common_no]))
agy_common&& agy=reinterpret_cast<agy_common&&> (*mv.labelledcommon[agy_common_no]);
//#define agy_isdefined (mv.labelledcommon[agy_common_no] != NULL)

#endif //NEO_FIN_H
