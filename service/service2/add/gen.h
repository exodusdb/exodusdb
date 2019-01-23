#ifndef EXO_GEN_H

#include <exodus/common.h>

class gen_common : public LabelledCommon{
public:

var address="";
var addresses="addresses";
var gcurrcompany="";
var timesheets="timesheets";
var company="";
var companies="companies";
var currency="";
var currencies="currencies";
var document="";
var documents="documents";
var unit="";
var units="units";
var popuprowbuffer="";
var _security="";
var glang="";
var accounting="";

};

#define gen_common_no 2

//works but is hard to to debug since there is no variable gen
//#define gen (*((gen_common*) mv.labelledcommon[gen_common_no]))

//cannot run conditional code during class member initialisation
//but can only create new common if not already done
//if (mv.labelledcommon[gen_common_no]==0)
//	mv.labelledcommon[gen_common_no]=new gen_common;

//works nicely but only if common already created otherwise points to nothing
//and if you try to reset it later, it tried to swap out nothing, causing segfault
gen_common&& gen=reinterpret_cast<gen_common&&> (*mv.labelledcommon[gen_common_no]);

//could use pointers but syntax is ugly gen->companies (and adecom must not recognise -> as an operator)
//gen_common*

//#define gen_isdefined (mv.labelledcommon[gen_common_no] != NULL)

#endif//EXO_GEN_H
