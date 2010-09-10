/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
#if 0

#include <vector>
#include <exodus/mv.h>

using namespace exodus;

void xyz()
{
    var xx;
    xx++;
}

#define function Dict MARKETS::

class DictItem
{
public:
    var name;
    var type;
    var fieldno;
    var title;
    var groupno;
    var keypartno;
    var conversion;
    var functioncode;
    var align;
    var length;

    var popup;
    var maxlength;
    var required;
    var listfunction;
};

DictItem dictrec(const var& f0="", const var& f1="", const var& f2="", const var& f3="", const var& f4="", const var& f5="", const var& f6="", const var& f7="", const var& f8="", const var& f9="", const var& f10="")
{
    DictItem di;
    di.name=f0;
    di.type=f1;
    di.fieldno=f2;
    di.title=f3;
    di.groupno=f4;
    di.keypartno=f5;
    //di.=f6;
    di.conversion=f7;
    di.functioncode=f8;
    di.align=f9;
    di.length=f10;

    return di;
}

typedef std::vector<DictItem> Dict;

class MARKETS
{
    Dict dict_MARKETS();
    DictItem* di;
    Dict dict;
};

//make javascript references into pointers (FOR THE REMAINDER OF THE CODE WHICH IS CLOSE TO JAVASCRIPT)
#define di (*di)

//from here on is the pure javascript code
//////////////////////////////////////////
//only var dict=[]//of dictrecs has been commented out or changed

//Copyright (c) 2000-2007 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

function dict_MARKETS()
{

 //returns an array representing the markets dictionary

// var dict=[]//of dictrecs
 var din=0;

 //single valued fields

 var g=0;//group number
 di=dict[++din]=dictrec("MARKET_CODE","F",0);
 di.popup="general_pop_marketstrue";
 di.length=3;
 di.maxlength=3;
 di.required=true;
 di.listfunction="general_list_markets()";
 
 di=dict[++din]=dictrec("MARKET_NAME","F",1);
 neosys_dict_text(di);
 di.required=true;
 di.length=30;

 di=dict[++din]=dictrec("CURRENCY_CODE","F",5);
 general_dict_currencycode(di);

 di=dict[++din]=dictrec("DATE_TIME", "F", "6");
 neosys_dict_datetimeupdated(di);

 di=dict[++din]=dictrec("CURRENCY_NAME","S");
 general_dict_currencyname(di);

 di=dict[++din]=dictrec("SEQUENCE","F",3,"","","","","[NUMBER]","","R",5);

 di=dict[++din]=dictrec("STOPPED","F", 25);
 di=dict[++din]=dictrec("VERSION","F", 26);

 var groupn=3;
 general_dict_addlogfields(dict,groupn,30,"general_open_version()");

 din=dict.length-1;

 return dict;

}

#endif

