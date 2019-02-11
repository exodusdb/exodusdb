#ifndef NEO_FIN_H

#include <exodus/common.h>

class fin_common : public LabelledCommon{
public:

//default contructor
fin_common(){
 chartmvfns = "3" _VM_ "4" _VM_ "7" _VM_ "8" _VM_ "10" _VM_ "11" _VM_ "12" _VM_ "14" _VM_ "21" _VM_ "22" _VM_ "24" _VM_ "25" _VM_ "27" _VM_ "29" _VM_ "32" _VM_ "33";
 vouchermvfns = "3" _VM_ "5" _VM_ "8" _VM_ "10" _VM_ "11" _VM_ "17" _VM_ "19" _VM_ "20" _VM_ "23" _VM_ "24" _VM_ "30" _VM_ "34" _VM_ "37";
}

var accounts="";
var balances="";
var vouchers="";
var voucherindex="";
var curryear="";
var currperiod="";
var currfmt="";
var credit="";
var currcompany="";
var currfromyear="";
var currfromperiod="";
var currtoyear="";
var currtoperiod="";
var fromyear="";
var toyear="";
var fromperiod="";
var toperiod="";
var unusedxyz="";
var ledgers="";
var charts="";
var batches="";
var basecurrency="";
var basefmt="";
var currcurrency="";
var converted="";
var gendesc="";
var taxes="";
var statmtypes="";
var maxperiod="";
var account="";
var definition="";
var alanguage="";
var settings="";
var currcompanycodes="";
var accparams="";
var collections="";
var zx3="";
var zx4="";
var hexdatesize="";
var chartmvfns="";
var vouchermvfns="";

};

#define fin_common_no 3
//#define fin (*((fin_common*) mv.labelledcommon[fin_common_no]))
fin_common&& fin=reinterpret_cast<fin_common&&> (*mv.labelledcommon[fin_common_no]);
//#define fin_isdefined (mv.labelledcommon[fin_common_no] != NULL)

#endif//NEO_FIN_H
