#ifndef EXO_GEN_H

#include <exodus/common.h>

class gen_common : public LabelledCommon{
public:

var address;
var addresses;
var gcurrcompany;
var timesheets;
var company;
var companies;
var currency;
var currencies;
var document;
var documents;
var unit;
var units;
var popuprowbuffer;
var _security;
var glang;
var _definitions;
var accounting;

};
#define gen (*((gen_common*) mv.labelledcommon[2]))
//#define labc2 ((labc2x) mv.labelledcommon[2])

#endif//EXO_GEN_H
