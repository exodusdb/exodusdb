//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

function address_list()
{
 openwindow('EXECUTE\rGENERAL\rLISTADDRESSES')
 return true
}

function dict_ADDRESSES()
{

 //returns an array representing the currency dictionary
 
 var dict=[]//of dictrecs
 var din=-1
 
 //single valued fields
 
 di=dict[++din]=dictrec('ADDRESS_CODE','F',0)
 di.required=true
 di.printfunction='address_list()'
  
 di=dict[++din]=dictrec('COMPANY_NAME','F',1)
 neosys_dict_text(di)
 di.required=true
 di.defaultvalue="(getvalue('ADDRESS_CODE')==gparameters.key&&gparameters.name)?unescape(gparameters.name):''"
 
 di=dict[++din]=dictrec('ADDRESS1','F',4)
 neosys_dict_text(di)
 di.wordsep=vm
 di.wordno=1
 
 di=dict[++din]=dictrec('ADDRESS2','F',4)
 neosys_dict_text(di)
 di.wordsep=vm
 di.wordno=2
 
 di=dict[++din]=dictrec('ADDRESS3','F',4)
 neosys_dict_text(di)
 di.wordsep=vm
 di.wordno=3
 
 di=dict[++din]=dictrec('ADDRESS4','F',4)
 neosys_dict_text(di)
 di.wordsep=vm
 di.wordno=4

 di=dict[++din]=dictrec('COUNTRY','F',8)
 
 di=dict[++din]=dictrec('FAX','F',11)
 
 di=dict[++din]=dictrec('TELEPHONE','F',9)
 
 di=dict[++din]=dictrec('EMAIL','F',10)
 neosys_dict_text(di)
 
 di=dict[++din]=dictrec('DATE_TIME', 'F', '26')
 neosys_dict_datetimeupdated(di)
 
 return dict
 
}