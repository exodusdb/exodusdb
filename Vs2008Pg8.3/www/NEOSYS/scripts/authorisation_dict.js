// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

//partial copy betweem authorisation_dict and hourlyrates_dict
function dict_AUTHORISATION(parameters)
{

 var dict=[]
 var di
 var din=-1
  
 di=dict[++din]=dictrec('KEY','F',0)
 di.defaultvalue='"SECURITY"'
 di.required=true
 di.printfunction='authorisation_print()'
 
 ///users f1-8 are parallel (maybe partial section of mvs for some users)
 
 di=dict[++din]=dictrec('USER_ID','F',1,'',1)
 di.required=true
 di.validation='user_val_userid()'
 //need to allow to move user up/down
 //di.unique=true
 di.nodeleterow=false//!neosyssecurity('AUTHORISATION DELETE USERS')
 di.noinsertrow=!neosyssecurity('AUTHORISATION CREATE USERS')
 di.validcharacters='ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890-'
 di.allowemptyrows=true
 
 di=dict[++din]=dictrec('KEYS','F',2,'',1)
 if (!neosyssecurity('AUTHORISATION UPDATE KEYS')) di.readonly=gmsg
 neosys_dict_text(di)
 di.length=40
 di.lowercase=false
 di.validation='user_val_keys()'
 
 di=dict[++din]=dictrec('MENU','F',3,'',1)
 di.required=true
 di.popup='user_pop_menu()'
 di.validation='user_val_menu()'
 di.defaultvalue='user_def_menu()'

 di=dict[++din]=dictrec('PASSWORD','F',4,'',1)
 di.required=true

 di=dict[++din]=dictrec('HOURLY_RATE','F',5,'',1)
 di=dict[++din]=dictrec('IP_NUMBERS','F',6,'',1)
 di=dict[++din]=dictrec('EMAIL_ADDRESS','F',7,'',1)
 neosys_dict_emailaddress(di)
 
 di=dict[++din]=dictrec('USER_NAME','F',8,'',1)
 neosys_dict_text(di)
  
 //was used to buffer valid companies by user for validcode()
 //di=dict[++din]=dictrec('FIELD9','F',9)
  
 ///tasks
 
 di=dict[++din]=dictrec('TASK_ID','F',10,'',2)
 di.required=true
 di.unique=true
 di.length=60
 di.validation='task_val_taskid()'
 if (!neosyssecurity('AUTHORISATION UPDATE LOCKS'))
 {
  di.readonly=gmsg
  di.nodeleterow=true
  di.noinsertrow=true
 }
 
 di=dict[++din]=dictrec('HIDDEN_LINES','S','','',2)
 di.functioncode= function authhidelines()
 {
  var hides=['0']//dont hide first item
  var taskcodes=gds.calcx('TASK_ID',null)
  var lasttaskcode=''
  for (var taskn=0;taskn<taskcodes.length;taskn++)
  {
   var thistaskcode=taskcodes[taskn]=taskcodes[taskn].split(' ')[0]
   if (taskn>0) hides[taskn]=thistaskcode==lasttaskcode?'1':'0'
   lasttaskcode=thistaskcode
  }
  return hides
 }
 
 di=dict[++din]=dictrec('LOCKS','F',11,'',2)
 di.validation='task_val_lock()'
 if (!neosyssecurity('AUTHORISATION UPDATE LOCKS')) di.readonly=gmsg
 
 ///other
 di=dict[++din]=dictrec('TEMP_FIRST_LINE','F',20)
 di=dict[++din]=dictrec('TEMP_LAST_LINE','F',21)
 di=dict[++din]=dictrec('TEMP_MENUS','F',22,'',g=3)
 
 //other users/keys comma separated (to restriction creation of)
 di=dict[++din]=dictrec('TEMP_OTHER_USERS','F',23)
 di=dict[++din]=dictrec('TEMP_OTHER_KEYS','F',24)
 
 di=dict[++din]=dictrec('LAST_LOGIN_DATE_TIME','S','','',1)
 neosys_dict_datetime(di)
 di.functioncode='return gds.calcx("USER_ID").neosysxlate("USERS",13)'
 
 di=dict[++din]=dictrec('LAST_LOGIN_LOCATION','S','','',1)
 di.functioncode='return gds.calcx("USER_ID").neosysxlate("USERS",14)'
 
 return dict
 
}
