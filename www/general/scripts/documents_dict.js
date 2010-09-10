// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

function dict_DOCUMENTS(parameters,dict)
{

 //adds to or returns an array representing the entry fields
 
 if (!dict) var dict=[]//of dictrecs
 var din=-1
 var di
 var g=0//group number

 di=dict[++din]=dictrec('ID','F',0)
 di.validcharacters='\\DOCSPUBLIC0123456789.htm'
 di.lowercase=true
 
 di=dict[++din]=dictrec('CREATOR','F',1)
 di.defaultvalue='gusername'
 
 di=dict[++din]=dictrec('DESCRIPTION','F',2)
 neosys_dict_text(di)
 di.length=60
 di.required=true
  
 di=dict[++din]=dictrec('CREATION_DATE','F',3)
 neosys_dict_date(di)
 
 di=dict[++din]=dictrec('CREATION_TIME','F',4)
 neosys_dict_time(di)
 
 di=dict[++din]=dictrec('INSTRUCTIONS','F',5)
 di.defaultvalue=parameters.COMMAND.neosysquote()
 
 di=dict[++din]=dictrec('PARAMETERS','F',6)
 
 di=dict[++din]=dictrec('SYSTEM','F',7)
 
 di=dict[++din]=dictrec('DATE_TIME','F',8)
 neosys_dict_datetimeupdated(di)
 
 di=dict[++din]=dictrec('DOCUMENT_NO','F',9)

 return dict
 
}
