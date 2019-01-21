#ifndef EXO_GEN_H

#include <exodus/common.h>

class gen_common : public LabelledCommon{
public:

var address="";
var addresses="";
var gcurrcompany="";
var timesheets="";
var company="";
var companies="";
var currency="";
var currencies="";
var document="";
var documents="";
var unit="";
var units="";
var popuprowbuffer="";
var _security="";
var glang="";
var definitions="";
var accounting="";

};

#define gen_common_no 2
//#define gen (*((gen_common*) mv.labelledcommon[gen_common_no]))
gen_common&& gen=reinterpret_cast<gen_common&&> (*mv.labelledcommon[gen_common_no]);
//#define gen_isdefined (mv.labelledcommon[gen_common_no] != NULL)

#endif//EXO_GEN_H
