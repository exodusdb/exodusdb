var gusers_authorisation_update
function* dict_USERS() {

 //returns an array representing the client dictionary

 var dict=[]//of dictrecs
 var din=-1
 var di
 var groupn

 var istimesheetadmin=yield* exodussecurity('TIMESHEET ADMINISTRATION')
 var timesheetadminreadonlymsg=gmsg
 
 gusers_authorisation_update=yield* exodussecurity('AUTHORISATION UPDATE')
 var updateallowed=false

 di=dict[++din]=dictrec('USER_ID','F',0)
 if (gusername!='EXODUS')
  di.defaultvalue='gusername'
 di.invalidcharacters='*'
 if (yield* exodussecurity('AUTHORISATION ACCESS')) {

  updateallowed=gusers_authorisation_update
  di.popup='yield* general_pop_users()'
 }
 else {

  di.readonly=true
 }

 di=dict[++din]=dictrec('USER_NAME','F',1)
 exodus_dict_text(di)
 di.readonly=!updateallowed
 di.required=true
 
 di=dict[++din]=dictrec('PASSWORD','F',4)

 di=dict[++din]=dictrec('DEPARTMENT','F',5)
 exodus_dict_text(di)
 di.readonly=true

 di=dict[++din]=dictrec('EMAIL_ADDRESS','F',7)
 exodus_dict_emailaddress(di,';')
 di.readonly=!updateallowed

 groupn=1
 
 /*
 di=dict[++din]=dictrec('LOGIN_DATE','F',15,'',groupn)
 di.wordsep='.'
 di.wordno=1
 di.nwords=1
 exodus_dict_date(di,'MINIMAL')
 di.readonly=true
 di.noinsertrow=true
 di.nodeleterow=true
 di.align='R'

 di=dict[++din]=dictrec('LOGIN_TIME','F',15,'',groupn)
 di.wordsep='.'
 di.wordno=2
 di.nwords=1
 exodus_dict_time(di)
 di.readonly=true
 */
 
 di=dict[++din]=dictrec('LOGIN_DATETIME','F',15,'',1)
 exodus_dict_datetime(di)
 di.readonly=true
 di.noinsertrow=true
 di.nodeleterow=true
   
 di=dict[++din]=dictrec('LOGIN_LOCATION','F',16,'',groupn)
 di.readonly=true

 di=dict[++din]=dictrec('LOGIN_RESULT','F',18,'',groupn)
 di.readonly=true
 exodus_dict_text(di)

 exodus_dict_colorfontsize(dict,19)
 din=dict.length-1

 groupn=2
 
 di=dict[++din]=dictrec('HOLIDAY_FROM_DATE','F',22,'',groupn)
 di.readonly=timesheetadminreadonlymsg
 exodus_dict_date(di,'TO=HOLIDAY_UPTO_DATE')
 di.nodeleterow=!!timesheetadminreadonlymsg
 di.noinsertrow=!!timesheetadminreadonlymsg

 di=dict[++din]=dictrec('HOLIDAY_UPTO_DATE','F',23,'',groupn)
 di.readonly=timesheetadminreadonlymsg
 exodus_dict_date(di,'FROM=HOLIDAY_FROM_DATE')

 di = dict[++din] = dictrec('HOLIDAY_REASON', 'F', 38, '', groupn)
 di.readonly = timesheetadminreadonlymsg
 exodus_dict_text(di)

 di = dict[++din] = dictrec('WEEKDAYS_OFF', 'F', 24)
 di.checkbox='1;Mon:2;Tue:3;Wed:4;Thu:5;Fri:6;Sat:7;Sun'

 di=dict[++din]=dictrec('MARKET_CODE', 'F',25)
 yield* general_dict_marketcode(di)
 
 //as at last login
 di=dict[++din]=dictrec('COMPANY_CODE','F',33)
 
 //legacy menu code applies to menus without locks
 di=dict[++din]=dictrec('MENU','F',34)
 
 //from this date the login is blocked
 di=dict[++din]=dictrec('EXPIRY_DATE','F',35)
 
 //password last changed - used for auto expiry
 di=dict[++din]=dictrec('PASSWORD_DATE','F',36)
 
 //Just a way to warn user on login without a server request______
 //i.e. calculate as ={PASSWORD_DATE}+userprivs<25> every web READ return dict
 di=dict[++din]=dictrec('PASSWORD_EXPIRY_DATE','F',37)

 return dict

}
