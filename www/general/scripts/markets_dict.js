//Copyright© 2000-2007 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**
//C++ style

function dict_MARKETS()
{

 //returns an array representing the markets dictionary

 var dict=[]//of dictrecs
 var din=-1;

 //single valued fields

 var g=0;//group number
 di=dict[++din]=dictrec("MARKET_CODE","F",0);
 di.popup="general_pop_markets(true)";
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
