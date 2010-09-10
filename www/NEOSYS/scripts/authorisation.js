// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**
///users

function form_presort(colid)
{
 if (colid=='LOCKS') form_filter('unfilter',colid)
 return true
}

var gtasks_usern=999999
var gtasks_updatehighergroups=false
var gtasks_menus=[]
var gtasks_otheruserids=[]
var gtasks_otherkeys=[]
var gtasks_newpassword

function form_preread()
{

 //used to distinguish between read security for authorisation and read security for hourlyrates
 securitymode=window.location.href.toString().indexOf('authorisation.htm')>=0?'SECURITY':'HOURLYRATES'
 gro.readenvironment=securitymode
 return true
 
}

function user_showtasks(userid)
{

 //if (!validateupdate()) return false
 
 grecn=getrecn(event.srcElement)
 var userid=gds.calcx('USER_ID',grecn)[0]
 
 db.request='EXECUTE\rGENERAL\rGETTASKS\r\r'+userid
 if (!db.send()) return neosysinvalid(db.response)

 var taskid=neosysdecide('Tasks authorised for '+userid,db.data,[[0,'Task'],[1,'Lock']],0,'','',inverted=true)
 if (!taskid) return false

 var recn=gds.calcx('TASK_ID',null).neosyslocate(taskid)-1
 if (recn<0) return false

 focuson(document.getElementsByName('LOCKS')[recn])
 
 return true
 
}

function authorisation_print()
{
 openwindow('EXECUTE\rGENERAL\rLISTAUTH')
 return true
}

//in authorisation.js and users.htm
function form_postwrite()
{

 //if change own password then login with the new one
 //otherwise cannot continue/unlock document so the lock hangs
 if (gtasks_newpassword) db.login(gusername,gtasks_newpassword)
 
 return true
 
}

function form_prewrite()
{
 return neosysokcancel('OK to save Users?',1)
}

function form_postread()
{

 gtasks_otheruserids=gds.calcx('TEMP_OTHER_USERS').split(' ')

 gtasks_otherkeys=gds.calcx('TEMP_OTHER_KEYS').split(' ')
 
 gtasks_usern=gds.calcx('USER_ID',null).neosyslocate(gusername)-1
 if (gtasks_usern<0) gtasks_usern=999999

 gtasks_updatehighergroups=neosyssecurity('AUTHORISATION UPDATE HIGHER GROUPS')

 //prepare popup for menus 
 gtasks_menus=gds.calcx('TEMP_MENUS',null)
 if (!gtasks_menus.length)
 {
  neosyswarning('Menus missing from returned data')
  gtasks_menus[1]=''
 }
 for (var i=0;i<gtasks_menus.length;i++) gtasks_menus[i]=gtasks_menus[i].split(sm)
 
 window.setTimeout('authorisation_postdisplay()',10)
 
 return true
 
}

function authorisation_postdisplay()
{
 form_filter('filter','HIDDEN_LINES',/1/)
 return true
}

function user_val_userid()
{

 //ZZZ check if user in hidden higher/lower groups!
 
 if (!task_checkrank()) return false

 //prevent duplicates/allow move
 var usernames=gds.calcx('USER_ID',null)
 var otherln1=usernames.neosyslocate(gvalue)
 if (otherln1)
 {
  var msg=gvalue+' already exists in line '+otherln1

  //prevent moving above yourself unless allowed access to higher groups
  //but this cannot really happen since you could not insert or change higher than yourself in the first place
  //if (otherln1-1<grecn&&!gtasks_updatehighergroups) return neosysinvalid(msg)
  
  //check user wants to move the account here
  if (neosysyesno(msg+'\r\rDo you want to move it here?',2)!=1) return neosysinvalid()

  //copy the other row here  
  var datarow=gds.updaterow('copy',gds.data.group1[grecn],gds.data.group1[otherln1-1])
  gds.rebind(datarow,datarow,forced=true)
  
  //delete the other row
  gds.deleterow(1,otherln1-1)
 
  return true
   
 }
 
 if (gvalue!='---'&&'ABCDEFGHIJKLMNOPQRSTUVWXYZ'.indexOf(gvalue.slice(0,1))<0) return neosysinvalid('Username must start with an alphabetic letter A-Z')
 
 if (gusername.indexOf('NEOSYS')<0&&gvalue.indexOf('NEOSYS')+1) return neosysinvalid('User name cannot include "NEOSYS"')

 //prevent hidden users
 if (gtasks_otheruserids.neosyslocate(gvalue)) return neosysinvalid(gvalue+' user already exists in another group')
 
 //have to reenter password
 if (gds.calcx('PASSWORD',grecn)[0]==''&&gds.calcx('MENU',grecn)[0]!='')
 {
  if (!neosysokcancel('You will have to enter a new password\rif you change the user name',1)) return false
 }
 
 return true
 
}

function user_val_keys()
{

 if (!task_checkrank()) return false
 
 if (!user_haskeys()) return false
  
 return true

}

function user_haskeys()
{
 if (!gvalue) return true
 var keys=gvalue.split(' ')
 for (var keyn=0;keyn<keys.length;keyn++)
 {
  var key=keys[keyn].neosystrim()
  if ((key=='NEOSYS'&&gusername!='NEOSYS')||gtasks_otherkeys.neosyslocate(key)) return neosysinvalid('You are not authorised to use key '+key.neosysquote())
  keys[keyn]=key
 }
 gvalue=keys.join(' ')
 return true
}

function user_def_menu()
{
 if (!gds.calcx('USER_ID')[0]) return ''
 return getpreviousrow('MENU',true)
}

function user_pop_menu()
{

 if (!task_checkrank()) return false
 
 var userid=gds.calcx('USER_ID',grecn)[0]
 if (!userid) return neosysinvalid('This is a group separator line\rYou can only enter data on user lines')
 
 var menuid=''
 
 //if (!(menuid=neosysdecide('','@SELECT\rADMENUS USING MENUS\rWITH MENU.TITLE BY MENU.TITLE\rMENU.TITLE ID','',1))) return false
 if (!(menuid=neosysdecide('',gtasks_menus,'',1))) return false
 
 return menuid

}

function user_val_menu()
{

 if (!task_checkrank()) return false

 for (var i=0;i<gtasks_menus.length;i++)
 {
  if (gvalue==gtasks_menus[i][1]) break
 }
 if (i>=gtasks_menus.length) return neosysinvalid(gvalue+' menu does not exist')
   
 return true
 
}

///tasks

function task_val_taskid()
{

 //prevent change of unauthorised tasks
 if (!task_authorised()) return false

 //prevent addition of unauthorised tasks
 if (!task_authorised(gvalue)) return false
 
 return true

}

function task_val_lock()
{

 //prevent change of unauthorised tasks
 if (!task_authorised()) return false

 //prevent addition of existing keys that user does not have
 if (!user_haskeys()) return false
 
 return true
 
}

function form_preinsertrow1()
{

 //double check prevent addition of users
 if (!neosyssecurity('AUTHORISATION CREATE USERS')) return neosysinvalid(gmsg)
 
 //prevent addition of users above yourself
 if (!gtasks_updatehighergroups&&grecn<=gtasks_usern) return neosysinvalid('You cannot insert users above yourself')
 
 return true
 
}

function form_predeleterow1()
{

 //double check prevent deletion of users
 if (!neosyssecurity('AUTHORISATION DELETE USERS')) return neosysinvalid(gmsg)

 //prevent deletion of yourself and higher users
 if (!gtasks_updatehighergroups&&grecn<=gtasks_usern) return neosysinvalid('You cannot delete yourself or higher users')

 if (gds.calcx('KEYS')[0]&&!neosyssecurity('AUTHORISATION UPDATE KEYS')) return neosysinvalid('You cannot delete this line because\r'+gmsg)
 
 return true
 
}

function form_predeleterow2()
{

 //prevent deletion of unauthorised tasks
 if (!task_authorised()) return false
 
 if (gds.calcx('LOCKS')[0]&&!neosyssecurity('AUTHORISATION UPDATE LOCKS')) return neosysinvalid('You cannot delete this line because\r'+gmsg)
 
 return true
 
}

function task_authorised(task)
{

 if (!task) task=gds.calcx('TASK_ID')[0]
 if (!task) return true
 
 if (!neosyssecurity(task)) return neosysinvalid('You cannot delete, change or add tasks that\ryou are not authorised to do yourself\r\r'+task)

 return true
}

function task_checkrank()
{
 
 //prevent modification of users above yourself
 if (!gtasks_updatehighergroups&&grecn<gtasks_usern) return neosysinvalid('You cannot modify users above yourself')
 
 return true
 
}