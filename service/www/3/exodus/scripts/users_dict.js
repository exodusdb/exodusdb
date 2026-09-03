var gusers_authorisation_update
async function dict_USERS() {

 //returns an array representing the client dictionary

 var dict=[]//of dictrecs
 var din=-1
 var di
 var groupn

 var istimesheetadmin=await exoui_security('TIMESHEET ADMINISTRATION')
 var timesheetadminreadonlymsg=gmsg
 
 gusers_authorisation_update=await exoui_security('AUTHORISATION UPDATE')
 var updateallowed=false

 di=dict[++din]=dictrec('USER_ID','F',0)
 // Key field — code host (same as JOB_NO / authorisation USER_ID), not free-text
 exo_dict_code(di)
 di.required=true
 // Default / open own user for everyone (including EXODUS).
 // gparameters.key is the normal form-init path to opendoc (see dbform after form_postinit).
 di.defaultvalue='gusername'
 if (typeof gparameters != 'undefined' && gparameters && !gparameters.key
  && typeof gusername == 'string' && gusername)
  gparameters.key = gusername
 di.invalidcharacters='*'
 if (await exoui_security('AUTHORISATION ACCESS')) {
  updateallowed=gusers_authorisation_update
  // F7 popup + typeahead (same as emailusers); expired skipped via LIVE_USER
  await system_dict_usercode(di)
 }
 else {
  di.readonly=true
 }

 di=dict[++din]=dictrec('USER_NAME','F',1)
 exo_dict_text(di)
 di.readonly=!updateallowed
 di.required=true
 
 di=dict[++din]=dictrec('PASSWORD','F',4)

 di=dict[++din]=dictrec('DEPARTMENT','F',5)
 exo_dict_text(di)
 di.readonly=true

 di=dict[++din]=dictrec('EMAIL_ADDRESS','F',7)
 exo_dict_emailaddress(di,';')
 di.readonly=!updateallowed

 groupn=1
 
 /*
 di=dict[++din]=dictrec('LOGIN_DATE','F',15,'',groupn)
 di.wordsep='.'
 di.wordno=1
 di.nwords=1
 exo_dict_date(di,'MINIMAL')
 di.readonly=true
 di.noinsertrow=true
 di.nodeleterow=true
 di.align='R'

 di=dict[++din]=dictrec('LOGIN_TIME','F',15,'',groupn)
 di.wordsep='.'
 di.wordno=2
 di.nwords=1
 exo_dict_time(di)
 di.readonly=true
 */
 
 di=dict[++din]=dictrec('LOGIN_DATETIME','F',15,'',1)
 exo_dict_datetime(di)
 // Display-only: code SPAN hugs content (INPUT default size wasted fixed space)
 exo_dict_code(di)
 di.align='R'
 di.readonly=true
 di.noinsertrow=true
 di.nodeleterow=true
   
 di=dict[++din]=dictrec('LOGIN_LOCATION','F',16,'',groupn)
 exo_dict_code(di)
 di.readonly=true

 di=dict[++din]=dictrec('LOGIN_RESULT','F',18,'',groupn)
 di.readonly=true
 exo_dict_text(di)

 exo_dict_colorfontsize(dict,19)
 din=dict.length-1

 groupn=2
 
 di=dict[++din]=dictrec('HOLIDAY_FROM_DATE','F',22,'',groupn)
 di.readonly=timesheetadminreadonlymsg
 exo_dict_date(di,'TO=HOLIDAY_UPTO_DATE')
 di.nodeleterow=!!timesheetadminreadonlymsg
 di.noinsertrow=!!timesheetadminreadonlymsg

 di=dict[++din]=dictrec('HOLIDAY_UPTO_DATE','F',23,'',groupn)
 di.readonly=timesheetadminreadonlymsg
 exo_dict_date(di,'FROM=HOLIDAY_FROM_DATE')

 di = dict[++din] = dictrec('HOLIDAY_REASON', 'F', 38, '', groupn)
 di.readonly = timesheetadminreadonlymsg
 exo_dict_text(di)

 di = dict[++din] = dictrec('WEEKDAYS_OFF', 'F', 24)
 di.checkbox='1;Mon:2;Tue:3;Wed:4;Thu:5;Fri:6;Sat:7;Sun'
 di.horizontal = true

 di=dict[++din]=dictrec('MARKET_CODE', 'F',25)
 if (typeof general_dict_marketcode !== 'undefined')
     await general_dict_marketcode(di)
 else {
     di.filename = 'MARKETS'
 }
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
