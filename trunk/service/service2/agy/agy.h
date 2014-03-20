#ifndef NEO_AGY_H

#include <exodus/common.h>

class agy_common : public LabelledCommon{
public:

agy_common(){
	tsmvfns="1" _VM_ "100" _VM_ "101" _VM_ "3" _VM_ "4" _VM_ "2";
}

var schedules;
var plans;
var clients;
var vehicles;
var ratecards;
var markets;
var brands;
var agp;
var mediatypes;
var invoices;
var analysis;
var bookingorders;
var jobs;
var productioninvoices;
var productionorders;
var suppliers;
var materials;
var ratings;
var unusedxyzz;
var ads;
var surveys;
var validcodes;
var tsmvfns;

};
#define agy (*((agy_common*) mv.labelledcommon[4]))
#define agy_isdefined (mv.labelledcommon[4] != NULL)
//#define labc2 ((labc2x) mv.labelledcommon[4])

#endif //NEO_FIN_H
