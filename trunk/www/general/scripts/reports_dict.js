
function dict_REPORTS()
{

 //returns an array representing the entry fields
 
 var dict=[]//of dictrecs
 var din=-1
 var di
 var g=0//group number

 if (typeof finance_dict_settings=='function')
 {
 
  di=dict[++din]=dictrec('SETTINGS','S')
  finance_dict_settings(di)
  
  di=dict[++din]=dictrec('SETTINGS_STRING','F',10)
 
 }
 
 di=dict[++din]=dictrec('CREATOR','F',1,'',1)
 di.nodeleterow=true
 di.noinsertrow=true
 di.readonly=true
 
 di=dict[++din]=dictrec('DESCRIPTION','F',2,'',1)
 neosys_dict_text(di)
 di.readonly=true
  
 di=dict[++din]=dictrec('UPDATE_DATE','F',3,'',1)
 neosys_dict_date(di)
 di.readonly=true

 di=dict[++din]=dictrec('UPDATE_TIME','F',4,'',1)
 neosys_dict_time(di)
 di.readonly=true
  
 di=dict[++din]=dictrec('INSTRUCTIONS','F',5,'',1)
 di.readonly=true

 di=dict[++din]=dictrec('PARAMETERS','F',6,'',1)
 di.readonly=true
 
 di=dict[++din]=dictrec('SYSTEM','F',7,'',1)
 di.readonly=true

 di=dict[++din]=dictrec('DATE_TIME','F',8,'',1)
 di.readonly=true
 
 di=dict[++din]=dictrec('DOCUMENT_NO','F',9,'',1)
 di.readonly=true
 
 di=dict[++din]=dictrec('DELETE_FLAG','F',10,'',1)
 di.checkbox='1; '
 
 return dict
 
}
