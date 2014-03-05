// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

//partial copy between authorisation_dict and hourlyrates_dict
//cannot insert/delete rows here so no need for full dictionary to coordinate row insert/deletion
function dict_HOURLYRATES(parameters)
{

 var dict=[]
 var di
 var din=-1
  
 di=dict[++din]=dictrec('KEY','F',0)
 di.defaultvalue='"SECURITY"'
 di.required=true
 
 ///users f1-8 are parallel (maybe partial section of mvs for some users)
 
 di=dict[++din]=dictrec('USER_ID','F',1,'',1)
 di.readonly=true
 di.nodeleterow=true
 di.noinsertrow=true
 di.allowemptyrows=true
 
 di=dict[++din]=dictrec('HOURLY_RATE','F',5,'',1)
 neosys_dict_number(di,2,0,999999)
 
 di=dict[++din]=dictrec('TEMP_MENUS','F',22,'',g=3)
 di=dict[++din]=dictrec('TEMP_OTHER_USERS','F',23)
 di=dict[++din]=dictrec('TEMP_OTHER_KEYS','F',24)
 
 return dict
 
}