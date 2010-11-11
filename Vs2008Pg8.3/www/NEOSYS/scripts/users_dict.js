var gusers_authorisation_update
function dict_USERS()
{

 //returns an array representing the client dictionary

 var dict=[]//of dictrecs
 var din=-1
 var di

 gusers_authorisation_update=neosyssecurity('AUTHORISATION UPDATE')
 var updateallowed=false

 di=dict[++din]=dictrec('USER_ID','F',0)
 di.defaultvalue='gusername'
 di.invalidcharacters='*'
 if (neosyssecurity('AUTHORISATION ACCESS'))
 {
  updateallowed=gusers_authorisation_update
  di.popup='general_pop_users()'
 }
 else
 {
  di.readonly=true
 }

 di=dict[++din]=dictrec('USER_NAME','F',1)
 neosys_dict_text(di)
 di.readonly=!updateallowed

 di=dict[++din]=dictrec('PASSWORD','F',4)

 di=dict[++din]=dictrec('DEPARTMENT','F',5)
 neosys_dict_text(di)
 di.readonly=true

 di=dict[++din]=dictrec('EMAIL_ADDRESS','F',7)
 neosys_dict_emailaddress(di)

 di=dict[++din]=dictrec('LOGIN_DATE','F',15,'',1)
 di.wordsep='.'
 di.wordno=1
 di.nwords=1
 neosys_dict_date(di,'MINIMAL')
 di.readonly=true
 di.noinsertrow=true
 di.nodeleterow=true
 di.align='R'

 di=dict[++din]=dictrec('LOGIN_TIME','F',15,'',1)
 di.wordsep='.'
 di.wordno=2
 di.nwords=1
 neosys_dict_time(di)
 di.readonly=true

 di=dict[++din]=dictrec('LOGIN_LOCATION','F',16,'',1)
 di.readonly=true

 di=dict[++din]=dictrec('LOGIN_RESULT','F',18,'',1)
 di.readonly=true
 neosys_dict_text(di)

 di=dict[++din]=dictrec('REPORT_HEAD_COLOR','F',19)
 di.wordsep=vm
 di.wordno=1
 di.nwords=1
 neosys_dict_color(di)

 di=dict[++din]=dictrec('REPORT_BODY_COLOR','F',19)
 di.wordsep=vm
 di.wordno=2
 di.nwords=1
 neosys_dict_color(di)

 di=dict[++din]=dictrec('REPORT_FONT','F',19)
 di.wordsep=vm
 di.wordno=3
 di.nwords=1
 neosys_dict_font(di)
 di.required=true

 di=dict[++din]=dictrec('SCREEN_HEAD_COLOR','F',19)
 di.wordsep=vm
 di.wordno=4
 di.nwords=1
 neosys_dict_color(di)

 di=dict[++din]=dictrec('SCREEN_BODY_COLOR','F',19)
 di.wordsep=vm
 di.wordno=5
 di.nwords=1
 neosys_dict_color(di)
 di.validation='users_val_screencolor()'

 di=dict[++din]=dictrec('SCREEN_FONT','F',19)
 di.wordsep=vm
 di.wordno=6
 di.nwords=1
 neosys_dict_font(di)
 di.validation='users_val_screenfont()'
 di.required=true

 di=dict[++din]=dictrec('SCREEN_FONT_SIZE','F',19)
 di.wordsep=vm
 di.wordno=7
 di.nwords=1
 di.conversion='50;50%:60;60%:70;70%:80;80%:90;90%:100;100%:110;110%:120;120%:130;130%:140;140%:150;150%:160;160%:170;170%:180;180%:190;190%:200;200%'
 di.validation='users_val_screenfontsize()'
 di.defaultvalue='100'
 di.allowcursor=true

 return dict

}
