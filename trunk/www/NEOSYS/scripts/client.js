 //Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**
 //undocumented functions
 //CollectGarbage() in all versions
 //"finally" block in v5.0
 
 //mozilla differences from IE
 // HTML element has a parentNode which is not usual element so be careful in "ancestor finder" routines
 // elements cannot be referred to simply as variables like idx
 // (must use getElementById or set an equivalent global variable)
 // insertBefore secondparameter is mandatory (put null)
 // document.body cannot be referred to in heading scripts until the document body is started
 // (so put document.onload function in the body tag. onload function cannot be called onload)
 // selectelement.options() should 
 //parentElement nonstandard use parentNode

gimagetheme='../neosys/images/theme2/'
gmenuimage=gimagetheme+'menu.png'//'add.png'
glogoutimage=gimagetheme+'disconnect.png'//'add.png'
grefreshimage=gimagetheme+'refresh.png'
 
window.onbeforeprint=window_onbeforeprint
function window_onbeforeprint()
{
 clientfunctions_setstyle()
}
 
 var goriginalstyles={}
 
 //prevent framing
 if (window!=top) top.location.href=location.href
 
 var gkeepalivemins=10
 //gkeepalivemins=1

 if (typeof NEOSYSlocation=='undefined') NEOSYSlocation='../neosys/'
 
 //check right browser and cookies allowed else switch to login which handled this error
 if (
//     ((!window.dialogArguments||!window.dialogArguments.cookie)&&!navigator.cookieEnabled)
//     ||
     (navigator.appVersion.indexOf('MSIE 5.5')==-1 && navigator.appVersion.indexOf('MSIE 6')==-1 && navigator.appVersion.indexOf('MSIE 7')==-1 && navigator.userAgent.indexOf('MSIE 5.23')==-1 && navigator.userAgent.indexOf('Firefox/2.0.0.1')==-1)
    )
 {
  //document.location=NEOSYSlocation+'wrongbrowser.htm'
  if (typeof glogin=='undefined')
  {
//   alert('qwe')
   document.location=NEOSYSlocation+'../default.htm'
  }
 }
 
 ismac=navigator.appVersion.indexOf('Macintosh')>=0
 
 document.protocolcode=document.location.toString().slice(0,4)
 
 var cr=String.fromCharCode(13)
 
 //global constants for revelation high end separator characters
 var rm=String.fromCharCode(255)
 var fm=String.fromCharCode(254)
 var vm=String.fromCharCode(253)
 var sm=String.fromCharCode(252)
 var sm2=sm//'ü'
 var tm=String.fromCharCode(251)
 var stm=String.fromCharCode(250)
 //regular expressions for 
 var STMre=/\xFA/g
 var TMre=/\xFB/g
 var SMre=/\xFC/g
 var VMre=/\xFD/g
 var FMre=/\xFE/g
 var RMre=/\xFF/g
 //var XMLre=/([\x25\x3C\x3E\x26])/g
 //var XXre=/([\xFF\xFE\xFD\xFC\xFB\xFA])/g
 var XMLXXre=/([\x25\x3C\x3E\x26\xFF\xFE\xFD\xFC\xFB\xFA])/g

 var dbcache

 var glogincode
 if (window.dialogArguments&&window.dialogArguments.logincode)
 {
  glogincode=window.dialogArguments.logincode
//alert('debug client.js glogincode=window.dialogArguments.logincode '+window.dialogArguments.logincode)
 }
 else if (window.opener)
 {
  try
  {
   glogincode=window.opener.glogincode
//alert('debug client.js glogincode=window.opener.glogincode '+window.opener.glogincode)
   //if (typeof gparameters=='undefined'&&window.opener.gparametersforcallee)
   // gparameters=window.opener.gparametersforcallee
  }
  catch (e){}
 }
 if (!glogincode) glogincode=''
 if (!glogincode)
 {
  glogincode=neosysgetcookie('','NEOSYSlogincode','logincode')
//alert('debug client.js glogincode=neosysgetcookie(\'\',\'NEOSYSlogincode\',\'logincode\') '+window.opener.glogincode)
 }
 
 var gusername
 var gdataset
 var gsystem
 
 var gcompanycode
 var gperiod
 var gbasecurr
 var gbasefmt
 var gmarketcode
 var gmaincurrcode
 var gdatasetname
 
 var gnpendingscripts=0

 //load gparameters from dialogarguments if present
 var gparameters
 if (typeof gparameters=='undefined') gparameters=new Object
 if (typeof dialogArguments!='undefined')
 {
  for (var param in dialogArguments) gparameters[param]=dialogArguments[param]
 }
 
 //add gparameters from URL if present eg. neosys.com/xyz.htm?param1=1&param2=2 etc.
 var temp=window.location.toString()
 if (temp.indexOf('?')>=0)
 {
  temp=unescape(temp.slice(temp.indexOf('?')+1)).split('&')
  for (i=0;i<temp.length;i++) gparameters[temp[i].split('=')[0]]=temp[i].split('=').slice(1).join('=')
 }
 if (typeof gparameters.savemode=='undefined') gparameters.savemode=gparameters.SAVEMODE

 //file access functions
 if (document.protocolcode=='file')
 {
  document.writeln('<SCR'+'IPT src="'+NEOSYSlocation+'scripts/server.js" language=javascript></SCR'+'IPT>')
 }
  
 //style sheet
 //copy any modifications to decide.htm and decide2.htm as well
 document.writeln('<link REL="stylesheet" TYPE="text/css" HREF="'+NEOSYSlocation+'global.css">')
 //be careful to always have private.css even if empty otherwise it is ALWAYS not in cache and
 //requires a server lookup all the time. therefore upgrades will destroy any neosys client's private.css
 //actually it shouldnt be ../ in the login and inital menu screen
 //cut out because slows at least modaldialog in ie7
 //document.writeln('<link REL="stylesheet" TYPE="text/css" HREF="../private.css">')

 //general functions
 if (!document.getElementById('generalfunctions'))
 {
  document.writeln('<SCR'+'IPT id=generalfunctions src="'+NEOSYSlocation+'scripts/neosys.js" language=javascript></SCR'+'IPT>')
 }
 
 //form functions
 if (typeof gdatafilename!='undefined')
 {
 
  document.writeln('<SCR'+'IPT src="'+NEOSYSlocation+'scripts/db.js" language=javascript></SCR'+'IPT>')
  document.writeln('<SCR'+'IPT src="'+NEOSYSlocation+'scripts/gds.js" language=javascript></SCR'+'IPT>')
  document.writeln('<SCR'+'IPT src="'+NEOSYSlocation+'scripts/dbform.js" language=javascript></SCR'+'IPT>')
  
 }

 //remove demo function if no other function to unload it
 if (!window.onload)
 {
  window.onload=clientfunctions_windowonload
 }
 
 //save location except if logging in
 if (typeof gnosavelocation=='undefined'&&!window.dialogArguments&&NEOSYSlocation!='./neosys/')
 {
  //if (gdataset) neosyssetcookie('','NEOSYS',escape(location),'ll',true)
 }
 
 if (false&&(gusername=='NEOSYS'||document.location.toString().indexOf('neosys4')>=0))
 {
  var glogging=1
  var gstepping=0
 }
 else
 {
  glogging=0
  gstepping=0
 }

 //ensure http session is kept alive
 if (document.protocolcode!='file')
 {
  if (gkeepalivemins) window.setInterval('sessionkeepalive()',gkeepalivemins*60*1000)
 }

 loadcache()
 
function neosyslogout()
{
 
 //cancel any automatic login
 neosyssetcookie('','NEOSYS','','a')
 
 //decide where to login again
 var newwindowlocation='../default.htm'
 var system=neosysgetcookie('','NEOSYSsystem')
 if (system&&system!='ADAGENCY') newwindowlocation+='??'+system
 
 //clear various session variable
 dblogout()

 //switch to login window 
 try
 {
  window.location=newwindowlocation
 }
 catch(e){}
 
 return false
 
}

var gmsg
var gtasks
function neosyssecurity(task)
{
 if (task.indexOf(' FILE ')>=0) neosyswarning('FILE should not be in task '+task)
 
 //make sure task list is loaded
 if (!gtasks)
 {
  db.request='EXECUTE\rGENERAL\rGETTASKS\rNOT'
  if (!db.send())
  {
   gtasks=''
   neosysinvalid(db.response)
   return false
  }
  gtasks=db.data.split(fm)[0].split(vm)
  
  //expand leading + signs
  for (var i=1;i<gtasks.length;i++)
  {
   if (gtasks[i].match(/(^\++)/))
   {
    gtasks[i]=gtasks[i].replace(/(^\++)/,gtasks[i-1].split(' ').slice(0,RegExp.$1.length).join(' ')+' ')
   }
  }
  
 }

 //fail if task not allowed
 //if (task.slice(0,3)=='!!!'||gtasks.neosyslocate(task)||(gstepping&&!confirm(task)))
 if (task.slice(0,3)=='!!!'||gtasks.neosyslocate(task))
 {
  gmsg='Sorry '+gusername.neosyscapitalise()+', you are not authorised to do\r'+task.neosyscapitalise()
  return false
 }
 
 return true

}

function sessionkeepalive()
{
 
 //last connection
 var lastconnection=neosysgetcookie('','NEOSYSlc','lc')
 if (lastconnection=='undefined') lastconnection=''
 lc=lastconnection
 lastconnection=lastconnection?new Date(lastconnection):lastconnection=new Date(0)
 
 //next connection
 var nextconnection=lastconnection
 nextconnection.setMilliseconds(lastconnection.getMilliseconds()+(gkeepalivemins*60*1000))
 
 //call server if time to keep alive
 var time=new Date
 if (time>=nextconnection&&typeof db!='undefined')
 {
  db.request='KEEPALIVE'
  db.send()
  //if (!db.send()) alert(db.response)
  window.status=time+' Keep Alive'
 }
 
}

function neosyssetexpression(elementsorelementid,attributename,expression)
{

 //check element exists
 if (!elementsorelementid) return neosysinvalid('missing element in neosyssetexpression '+attributename+' '+expression)
 
 //elements can be elementnames too
 
 //get an array of elements
 var elements
 if (typeof elementsorelementid=='string')
 {
  elements=document.getElementsByName(elementsorelementid)
  if (!elements.length) return
 }
 else
 {
  elements=elementsorelementid
 //make sure is an array
 if (!elements.length) elements=[elements]
 }
 
 //set the attribute expression for all elements
 var style=attributename.slice(0,6)=='style:'
 var attributepart=style?attributename.slice(6):attributename
 for (var ii=0;ii<elements.length;ii++)
 {
  if (typeof elements[ii]=='string') neosyssetexpression(elements[ii],attributename,expression)
  else
  {
   if (style) elements[ii].style.setExpression(attributepart,expression)
   else elements[ii].setExpression(attributepart,expression)
  }
 }
 
}

function neosysenabledandvisible(element)
{
 if (typeof element=='string') element=document.getElementsByName(element)[0]
 if (element.neosysreadonly) return false
 while (element&&element.parentNode&&element.tagName!='BODY')
 {
  if (element.style.display=='none') return false
  //if (element.runtimeStyle.display=='none') return false
  if (element.disabled) return false
  element=element.parentNode
 }
 return true
}

function showhide(element,show)
{

 var elementid=element
 
 if (typeof element=='string')
 {
  element=document.getElementsByName(element)
  if (!element.length) return neosysinvalid('element '+elementid+' does not exist in showhide()')
 }
 
 //recursive
 if (typeof element=='object'&&!element.tagName)
 {
  for (var i=0;i<element.length;i++) showhide(element[i],show)
  return
 }
 
 if (element.style) element.style.display=show?'inline':'none'
 
}

function insertafter(element,newelement)
{
 if (element.nextSibling) return element.parentNode.insertBefore(newelement,element.nextSibling)
 return element.parentNode.insertBefore(newelement,null)
}

function setinnerHTML(elementsorid,html)
{
 if (typeof elementsorid=='string') elementsorid=document.getElementsByName(elementsorid)
 for (var i=0;i<elementsorid.length;i++) elementsorid[i].innerHTML=html
}

function neosysmodalwindow(url,arguments)
{

 if (!arguments) var arguments=new Object
 
 var maxheight=window.screen.availHeight
 var maxwidth=window.screen.availWidth
 var features='DialogHeight:'+maxheight+'px; DialogWidth:'+maxwidth+'px; Center: yes; Help: yes; Resizable: yes; Status: No;'

 arguments.logincode=glogincode

 try
 {
  return window.showModalDialog(url,arguments,features)
 }
 catch(e)
 {
  alert('Please enable popups for this site (1)\r\rError:'+e.description)
  return
 }
}

//function to simplify passing a db request (with optional data)
//that returns a URL to be opened in an independent window
function openwindow(request,data)
{

 if (!data) data=''
 db.request=request
 if(!db.send(data))
 {
  neosysinvalid(db.response)
  return false
 }
 
 return windowopen(db.data)
  
}

function windowopen2(url)
{
 return windowopen(url,{'key':gvalue})
}

var gwindowopenparameters
function windowopen(url,parameters)
{

 if (!url) url=''
 if (!parameters) parameters={}
 //these parameters are picked up by the opened window (from its parent) after it opens
 //since no way to pass parameters directly to non modal windows except in the URL
 //and even then the this causes different urls perhaps defeating http caching
 gwindowopenparameters=parameters
 gwindowopenparameters.logincode=glogincode
 try
 {
  return window.open(url)
 }
 catch(e)
 {
  alert('Please enable popups for this site (2)\r\rError:'+e.description+'\r\r'+url)
  return false
 }

}

//wrapper function to replace window.open()
//to give warning if cannot open ... because of pop blockers etc.
function breakpoint(cmd,funcname,position)
{

 if (!gstepping) return
 
 window.status=funcname+' '+position+' '+cmd

 if (!cmd) cmd=' '
  
 var msg=''
 while (cmd)
 {
  if (cmd!=' ')
  {
   try
   {
    var result=this.parent.eval(cmd)
   }
   catch (e)
   {
    result=e.number+' '+e.description
   }
   msg=cmd+'=\r'+result+'\r\r'+msg
   if (!confirm(msg)) return
  }
  cmd=prompt(funcname+' '+position,cmd)
 }
}

function assertelement(element,funcname,varname)
{
 neosysassertobject(element,funcname,varname)
 if (!element.tagName)
 {
  raiseerror(1000,'In '+funcname+', '+varname+' is not an element.')
  return
 }
 return true
}

function neosysnote(msg,mode)
{
 //if (!msg) return false
 //allow return neosysnote() to be opposite of return neosysinvalid()
 if (!msg) return true

 if (!mode) mode='info'

 if (typeof msg=='string'&&msg.slice(0,6)=='Error:') msg=msg.slice(6)
   
 //msg=msg.replace(/\r/gi,'\r')
 //msg=msg.replace(/\n/gi,'\n')
 msg=msg.toString().replace(/\xFE/gi,'\r\n')
 msg=msg.replace(/\xFD/gi,'\r\n')
 msg=msg.replace(/\|/gi,'\r\n')

 neosysyesnocancel(msg,1,'OK','','',null,false,mode)
 
 return true
 
}

//''''''''
//'INVALID
//''''''''
function neosysinvalid(msg)
{
 //displays a message if provided and returns false
 //so you can use it like "return neosysinvalid(msg)" to save a line
 neosysnote(msg,'critical')
 return false
}

//''''''''
//'WARNING
//''''''''
function neosyswarning(msg)
{
 //displays a message if provided and returns true
 //so you can use it like "return neosyswarning(msg)" to save a line 
 return neosysnote(msg,'warning')
}

//duplicated in client.js, decide.htm and decide2.htm
function neosys_set_style(mode,value,value2)
{

 if (value.toUpperCase()=='DEFAULT') value=''
 
 //restore original value
 if (!value&&goriginalstyles[mode]) value=goriginalstyles[mode]
 
 //ensure display is set to inline even if not changing color
 //if (!value) return
 
 var rules=document.styleSheets[0].rules
 var oldvalue=''
 
 //screencolor
 if (mode=='screencolor')
 {
  
  //make everything visible!
  var style=rules[0].style
  style.display='block'

  //initial color is buff yellow
  if (!value) value='FFFFC0'
   
  oldvalue=style.backgroundColor
  try
  {
   style.backgroundColor=value
  }
  catch (e)
  {
   if (e.number==-2146827908) return neosysinvalid(value+' is not a recognised color')
   return systemerror('neosys_set_style("'+mode+'","'+value+'")',e.number+' '+e.description)
  }
 }
 
 //screenfont
 else if (mode=='screenfont')
 {

  //initial font is ... 8pt
  var basefontsize=8
  
  if (!value) value='verdana,arial,helvetica'
  if (!value2) value2=100
  if (!Number(value2))
  {
   alert(value2+' is not a recognised font size, using 100%')
   value2=100
  }
  if (typeof gformfontscale!='undefined'&&gformfontscale) value2*=gformfontscale
  value2=(basefontsize*Number(value2)/100)+'pt'

  for (var rulen=0;rulen<rules.length;rulen++)
  {
  
   var style=rules[rulen].style
   if (!style.fontFamily) continue
   
   oldvalue=style.fontFamily
   try
   {
    style.fontFamily=value
    style.fontSize=value2
   }
   catch (e)
   {
    if (e.number==-2146827908) return neosysinvalid(value+' is not a recognised font')
    if (e.number==-2147024809) return neosysinvalid(value2+' is not a recognised fontsize')
    return systemerror('neosys_set_style("'+mode+'","'+value+'","'+value2+'")',e.number+' '+e.description)
   }
  }
 }
 
 //save the original style
 if (!goriginalstyles[mode]&&oldvalue) goriginalstyles[mode]=oldvalue
 
}

//called early in decide and decide2
function clientfunctions_setstyle()
{
 //set font first since setting color changes style display from none to inline
 neosys_set_style('screenfont',neosysgetcookie(glogincode,'NEOSYS2','ff'),neosysgetcookie(glogincode,'NEOSYS2','fs'))
 neosys_set_style('screencolor',neosysgetcookie(glogincode,'NEOSYS2','fc'))
}

function clientfunctions_windowonload()
{
 
 //if (gnpendingscripts)
 //{
 // wstatus('Pending scripts '+gnpendingscripts)
 // window.setTimeout('clientfunctions_windowonload()',10,'javascript')
 // return
 //}
 
 login('clientfunctions_windowonload')
 
 if (!glogincode) glogincode=neosysgetcookie('','NEOSYSlogincode','logincode')
//loginalert('wol'+glogincode)
 gdataset=neosysgetcookie(glogincode,'NEOSYS2','dataset')

 db=new neosysdblink
 //gusername is set in neosysdblink
 
 gcompanycode=neosysgetcookie(glogincode,'NEOSYS2','cc')
 gncompanies=neosysgetcookie(glogincode,'NEOSYS2','nc')
 gperiod=neosysgetcookie(glogincode,'NEOSYS2','pd')
 gbasecurr=neosysgetcookie(glogincode,'NEOSYS2','bc')
 gbasefmt=neosysgetcookie(glogincode,'NEOSYS2','bf')
 gmarketcode=neosysgetcookie(glogincode,'NEOSYS2','mk')
 gmaincurrcode=neosysgetcookie(glogincode,'NEOSYS2','mc')
 gdatasetname=neosysgetcookie(glogincode,'NEOSYS2','db')
 gmenucodes=neosysgetcookie(glogincode,'NEOSYS2','m')

 clientfunctions_setstyle()

 //trigger formfunctions_onload
 if (typeof formfunctions_onload=='function') formfunctions_onload()
 
 if (!window.dialogArguments&&(typeof gshowmenu=='undefined'||gshowmenu)&&NEOSYSlocation!='./neosys/')
 {
  
  //refresh button alt+x (clear cache)
  var temp2=document.createElement('SPAN')
  //temp2.innerHTML='<button title="Logout. Alt+L" id=logoutbutton accesskey="L" class=neosysbutton onclick="return neosyslogout()" tabindex=9999 href="../default.htm&'+temp+'">Logout</button>'
  temp2.innerHTML=menubuttonhtml('refreshcache',grefreshimage,'Refresh','Refresh the Cache. '+(ismac?'Ctrl':'Alt')+'+X','X','refresh_onclick()')
  document.body.insertBefore(temp2,document.body.firstChild)

  //add logout button
  var temp2=document.createElement('SPAN')
  //temp2.innerHTML='<button title="Logout. Alt+L" id=logoutbutton accesskey="L" class=neosysbutton onclick="return neosyslogout()" tabindex=9999 href="../default.htm&'+temp+'">Logout</button>'
  temp2.innerHTML=menubuttonhtml('logoutbutton',glogoutimage,'<u>L</u>ogout','Logout. '+(ismac?'Ctrl':'Alt')+'+L','L','neosyslogout()')
  document.body.insertBefore(temp2,document.body.firstChild)
  //if (!gusername) logoutbutton.innerText='Login'
  if (!gusername)
  {
   var temp=document.getElementById('logoutbutton')
   temp.innerText='Login'
   temp.title='Login. Alt+L'
  }
  
  //add menu button
  if (gmenucodes&&gmenucodes!='EXIT2')
  {
   //t='<iframe id=menuframe ie55onload="menuonload()" src="../menu.htm" tabindex=-1 style="xposition:absolute" height=0px marginheight=0 marginwidth=0 frameborder=0 scrolling=no width=100%></iframe><p>'
   t='<iframe id=menuframe ie55onload="menuonload()" src="../menu.htm" tabindex=-1 height=0px marginheight=0 marginwidth=0 frameborder=0 scrolling=no></iframe>'
   var temp=document.createElement('SPAN')
   temp.id='menux'
   temp.innerHTML=t
   
   document.body.insertBefore(temp,document.body.firstChild)
   gmenuonloader=window.setInterval('menuonload()',1)
  }
  
 }

 logout('clientfunctions_windowonload')
 
 return

 //if demo data, reveal demo text
 if (gdataset=='DEMOADSY')
 {
  if (!ismac)
  {
   //seems to abort window_onload() in mac !
   document.styleSheets[0].addRule('.demotext','display:'+mode)
  }
  else
  {
   //does not seem to work (used to)
   var rules=document.styleSheets[0].rules
   rules[rules.length-1].style.display=mode
  }
 }
 
}

Array.prototype.neosysread=function neosysread(filename,key,fieldno)
{

 //unless returning one fieldno, always return at least n fields
 //so that accessing fields that do not exist by [] returns ''
 var minnfields=100
 
 this.neosysresponse=neosysquote(key)+' does not exist in the '+filename.toLowerCase()+' file.'
 if (key=='') return false
 
 db.request='CACHE\rREAD\r'+filename+'\r'+key
 if (!db.send())
 {
  if (db.response.indexOf('NO RECORD')>=0)
  {
   //var temp=filename.toLowerCase().neosyssingular().replace(/_/,' ')
   return false
  }
  else
  {
   if (db.response.indexOf('file is not available')>=0) systemerror('neosysread',db.response)
   this.neosysresponse=db.response
  }
  return false
 }
 
 var data=(key+fm+db.data).split(fm)
 
 //flag to extract function that fields are 1 based (key in [0])
 this.base=1
 
 if (typeof(fieldno)=='undefined')
 {
  var n=data.length
  if (n<minnfields) n=minnfields
  for (var i=0;i<n;i++)this[i]=typeof data[i]!='undefined'?data[i]:''
 }
 else
 {
  if (typeof data[fieldno]=='undefined') data[fieldno]=''
  this[0]=data[fieldno]
 }
 return true
}

function setattribute(element,text)
{
	//element must be in DOM
	//text= eg 'onfocus="return onfocus()"'

    var outerhtml='<'+element.outerHTML.slice(1,-1)+' '+text+'>'

    //create a new node from the new tag HTML with the same attributes plus the new attribute
    var newelement=document.createElement('SPAN')
    newelement.innerHTML=outerhtml
    newelement=newelement.childNodes[0]

    //copy over any innerHTML as well
    if (element.innerHTML) newelement.innerHTML=element.innerHTML

    //put the new element back into the place of the old element
    element.parentNode.replaceChild(newelement,element)

    //return the new element
    return newelement

}

//xlate method for array of keys
Array.prototype.neosysxlate=function arrayxlate(filename,fieldno,mode)
{

 var keys=this
 var results=[]
 
 //make a list of keys not in cache
 var uncachedkeys=[]
 for (var keyn=0;keyn<keys.length;keyn++)
 {

  //return '' if record cannot be read or key if blank
  results[keyn]='' 
  var key=keys[keyn]
  if (!key) continue
  
  //get a record from the cache
  var cachekey='READ\r'+filename+'\r'+key
  var rec
  if (rec=readcache(cachekey))
  {
   //and do xlate logic on cached record
   results[keyn]=neosysxlatelogic(filename,(key+fm+rec).split(fm),fieldno,mode,key)
  }
 
  //or build a unique list of keys of records to be selected
  else
  {  
   if (!uncachedkeys.neosyslocate(key)) uncachedkeys[uncachedkeys.length]=key
  }
  
 }
 
 //select all the uncached records
 if (uncachedkeys.length)
 {
  
  //WARNING !!!
  //unfortunately SELECT\r\r\rRECORD returns raw record without
  //the usual postread processing and it puts this into the cache for
  //later processing as well (perhaps it should not put it in the cache)
  //it is difficult to call the usual postread processing from SELECT2
  //on the server since postreads require AREV.COMMON (OREC etc)
  //perhaps one should have a special POSTREADX which only touches @record
  
  //select the (deduplicated) records or return systemerror
  db.request='SELECT\r'+filename+'\r\rRECORD'
  if (!db.send(uncachedkeys.join(fm)))
  {
   systemerror(db.response)
   this.neosysresponse=db.response
   return []
  }

  //if no data returned then thats it!
  if (!db.data) return results
  
  //process the multiple records separated by rm char(255) characters
  var recset=db.data.split(rm)
  for (ii=0;ii<recset.length;ii++)
  {
  
   //do xlate logic on the record
   var keyrec=recset[ii].split(fm)
   var key=keyrec[0]
   var result=neosysxlatelogic(filename,keyrec,fieldno,mode,key)

   //store the results whereever they are needed
   var keyn=0
   while (keyn=keys.neosyslocate(key,'',keyn+1))
   {
    results[keyn-1]=result
   }
   
   //save in the cache regardless
   var cachekey='READ\r'+filename+'\r'+key
   writecache(cachekey,keyrec.slice(1).join(fm))
   
  }
  
 }//any uncachedkeys

 return results
 
}

//get a (cached) database value given a filename, fieldno
//fieldno 0 means return whole record as simple array
//mode can be undefined, C (means return key if no record) and SUM means add up mvs
//zzz SHOULD return '' if no record and null if there is any error
String.prototype.neosysxlate=function stringxlate(filename,fieldno,mode)
{
 
 key=this.toString()
 if (key=='') return ''
 
 neosysassertnumeric(fieldno,'xlate',filename+' '+key)
 
 var record=[]
 record.neosysread(filename,this)
 if (db.response.indexOf('file is not available')>=0) systemerror('xlate',db.response)

 return neosysxlatelogic(filename,record,fieldno,mode,key)
 
}

function neosysxlatelogic(filename,record,fieldno,mode,key)
{
 if (record.length)
 {
  if (typeof fieldno!='undefined')
  {
   if (fieldno)
   {
    record=record[fieldno]
    if (typeof record=='undefined') record=''
   }
  }
  //option to sum the result (good for adding up multivalues)
  if (mode&&mode=='SUM')
  {
   record=record.neosyssum()
  }
 }
 else
 {

  //option to return the key if failed to get record
  //zzz ?! C mode doesnt work since record length is zero here
  //record=(mode&&mode=='C')?record[0]:''
  record=(mode&&mode=='C')?key:''
 }
 
 return record
 
}

function neosysfilepopup(filename,cols,coln,sortselectionclause,many)
{
 //filename is required
 //cols is required (array of arrays)
 //eg [['COMPANY_NAME','Company Name'],['COMPANY_CODE','Company Code']]
 
 //sortselectionclause is optional
 //eg 'WITH TYPE1 = "X" AND WITH = TYPE2 "Y" BY TYPE1 BY NAME'
 if (!sortselectionclause) sortselectionclause=''
 
 //sortselectionclause can be an array of keys
 if (typeof sortselectionclause=='object') sortselectionclause=sortselectionclause.join('" "').neosysquote()
 
 //get a list of col names sep by spaces
 var collist=''
 for (var i=0;i<cols.length;i++)
 {
  collist+=' '+cols[i][0]
 }
 collist=collist.slice(1)
 if (!collist.split(' ').neosyslocate('ID')) collist+=' ID'
 
 //get the data from the server
 db.request='CACHE\rSELECT\r'+filename.toUpperCase()+'\r'+sortselectionclause+'\r'+collist+'\rXML'
 //db.request='CACHE\rSELECT\r'+filename.toUpperCase()+'\r'+sortselectionclause+'\r'+collist+' ID'
 if (!db.send())
 {
  neosysinvalid(db.response)
  return null
 }

 //cancel if no records found
 if (db.data.indexOf('<RECORD>')<0)
 {
  neosysinvalid('Sorry, no records found')
  return null
 }
 
 return neosysdecide2('',db.data,cols,coln,'',many)
 
}

//////////////////////
// global functions //
//////////////////////

//similar function in xhttp.asp
function neosysdblink()
{
 
 this.request=''
 this.data=''
 this.response=''
 this.documentprotocolcode=document.protocolcode
 this.login=neosysdblink_login
 
 //used in cache and elsewhere
 if (!gdataset) gdataset=neosysgetcookie(glogincode,'NEOSYS2','dataset')
 gusername=neosysgetcookie(glogincode,'NEOSYS2','username')
 gsystem=neosysgetcookie('','NEOSYSsystem')
// alert(gdataset+'*'+gusername+'*'+gsystem)
 this.dataset=gdataset
 this.system=gsystem
 this.username=gusername
 
 //setup environment for http messaging
 if (this.documentprotocolcode!='file')
 {

  //netscape?
  try
  {
   this.XMLHTTP=new XMLHttpRequest()
   this.send=neosysdblink_send_byhttp_using_xmlhttp
   this.XMLHTTP.overrideMimeType("text/xml")
   return
  }
  catch(e){}
	
  try
  {
   this.XMLHTTP=new ActiveXObject('Microsoft.XMLHTTP')
   this.send=neosysdblink_send_byhttp_using_xmlhttp
//   alert('xmlhttp')
  }
  catch(e)
  {
   this.send=neosysdblink_send_byhttp_using_forms
//   alert('forms')
  }

  return
  
 }

 //otherwise setup environment for file messaging
  
 //try and get the username, password and dataset
 this.password=neosysgetcookie(glogincode,'NEOSYS2','password')
 this.timeout=neosysgetcookie(glogincode,'NEOSYS2','timeout')
 
 //default timeout is 10 minutes (NB GIVEWAY timeout is hard coded to 10 mins?)
 var defaulttimeoutmins=10
 //var defaulttimeoutmins=.25
 if (!this.timeout) this.timeout=defaulttimeoutmins*60*1000

 
 this.send=neosysdblink_send_byfile
 this.start=neosysdblink_startdb
 
 try
 { 
  gfso = new ActiveXObject('Scripting.FileSystemObject')
 }
 catch(e)
 {
  alert('Error: While creating Scripting.FileSystemObject\r'+e.number+' '+e.description)
  window.location=NEOSYSlocation+'securityhowto.htm'
  return
 }
 
 this.neosysrootpath=getneosysrootpath(document.location.toString())
 
 //localhostname
 //remove space . " ' characters
 //and convert _ to -
 this.localhostname=new ActiveXObject('WScript.Network').ComputerName.slice(0,8)
 this.wscriptshell=new ActiveXObject('WScript.Shell')
 
 this.remoteaddr=''
 this.remotehost=new ActiveXObject('WScript.Network').ComputerName.toString()
 //remove space . " ' characters
 this.remotehost=this.remotehost.replace(/[\ |\.|\"|\']/gi,'')
 this.https='off'//Request.ServerVariables("HTTPS")

}

function dblogout()
{

 //remove username etc
 //neosyssetcookie('','NEOSYS',gdataset,'dataset',true)

 //remove last page
 //neosyssetcookie('','NEOSYS','','ll',true)
 
 neosyssetcookie(glogincode,'NEOSYS2','','username')
 
 //remove settings
 neosyssetcookie(glogincode,'NEOSYS2','')
 
}

function neosysdblink_login(username,password,dataset,system)
{

 //get list of datasets from server
 var logindb=new neosysdblink
 var datasets=''
 
 if (!dataset&&gdataset) dataset=gdataset
 if (!dataset) dataset=neosysgetcookie('','NEOSYS','dataset')
 if (!system) system=neosysgetcookie('','NEOSYSsystem')
 var arguments=['','',dataset,'','',system]
 var failed=false
  
 while (true)
 {
 
  arguments[4]=datasets
  //if (!(typeof event!='undefined'&&event.shiftKey)&&!failed&&neosysgetcookie('','NEOSYS','a')=='true')
  if (!(typeof event!='undefined'&&event&&event.shiftKey)&&!failed&&(neosysgetcookie('','NEOSYS','a')=='true'||username))
  {
   arguments[0]=username?username:neosysgetcookie('','NEOSYS','u')
   arguments[1]=password?password:neosysgetcookie('','NEOSYS','p')
   arguments[2]=dataset
   arguments[5]=neosysgetcookie('','NEOSYS','a')
   arguments[6]=system
  }
  else
  {

   //if refreshing login  
   if (dataset)
   {
    datasetx=new Object
    datasetx.code=new Object
    datasetx.code.text=dataset
    datasetx.name=new Object
    datasetx.name.text=dataset
    datasets=new Object
    datasets.group1=[datasetx]
   }
   
   //otherwise get all datasets
   if (!datasets)
   {
    logindb.request='GETDATASETS'
    if (!logindb.send())
    {
     neosysinvalid(logindb.response)
     return 0
    }
    datasets=neosysxml2obj(logindb.data)
   }
   
   arguments[4]=datasets
   
   var features=''//'DialogHeight:400px; DialogWidth:800px; Center: yes; Help: yes; Resizable: yes; Status: yes;'

   try
   {
    if (ismac) arguments=window.showModalDialog('default.htm',arguments,features)
    else arguments=window.showModalDialog('../default.htm',arguments,features)
   }
   catch(e)
   {
    alert('Please enable popups for this site (3)\r\rError:'+e.description)
    return 0
   }

   //arguments=window.showModalDialog('default.htm',arguments,features)
   
  }
  
  //quit if user cancels
  if (!arguments) return 0

  logindb.request='LOGIN\r'+arguments[0]+'\r'+arguments[1]+'\r'+arguments[2]+'\r'+arguments[3]+'\r\r'+arguments[5]
  
  if (!logindb.send())
  {
   var msg=logindb.response
   if (!msg) msg='Invalid username or password'
   neosysinvalid(msg)
   failed=true
  }
  else
  {
  
   //set the username and dataset
   gusername=arguments[0]
   gdataset=arguments[2]
   gsystem=arguments[6]
   this.dataset=gdataset
   this.system=gsystem
   this.username=gusername

   //permanent cookie
   var temp='dataset='+dataset
   if (arguments[5].toString()=='true')
   {
    temp+='&u='+gusername
    temp+='&p='+arguments[1]
    temp+='&a='+arguments[5]
    temp+='&s='+arguments[6]
   }
   else
   {
    //temp+='&u='
    //temp+='&p='
    //temp+='&a='
   }
   neosyssetcookie('','NEOSYS',temp,'')

   //temporary cookie for menu
   neosyssetcookie(glogincode,'NEOSYS2',logindb.data)
   
   //temporary cookie for the dataset and username (and password for file protocol)   
   var temp='dataset='+gdataset+'&username='+gusername+'&system='+gsystem
   if (document.protocolcode=='file')
   {
    this.password=arguments[1]
    temp+='&password='+this.password
    //this.timeout=neosysgetcookie(glogincode,'NEOSYS2','timeout')
   }
   neosyssetcookie(glogincode,'NEOSYS2',temp)

   //quit success
   return 1
   
  }
 }
}

function neosysdblink_send_byhttp_using_forms(data)
{

 log(this.request)
//alert('neosysdblink_send_byhttp_using_forms\r...\r'+this.request+'\r...\r'+ data)
 this.data=data?data:''
 
 //request is required
 if (this.request=='')
 {
  this.data=''
  this.response=('ERROR: NO REQUEST')
  this.response=this.response.replace(/"/,"'")
  this.result=''
  return(0)
 }
 
 //try to use cache
 var request2=this.request
 var trycache=(request2.slice(0,6)=='CACHE\r')
 if (trycache)
 {
  request2=request2.slice(6)
  var temp
  if (temp=readcache(request2))
  {
   this.data=temp
   return true
  }
 }
 
 var gotresponse=false
 while (!gotresponse)
 {
 
  var params='dialogHeight:100px; dialogWidth:200px; center:Yes; help:No; resizable:No; status:No'
  params='dialogHeight: 201px; dialogWidth: 201px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'
  try
  {
   var reply=window.showModalDialog(NEOSYSlocation+'rs/default.htm',[this.timeout,this.request,this.data],params)
  }
  catch(e)
  {
   this.data=''
   this.response=('ERROR: Please enable popups for this site (4)\r\rError:'+e.description)
   this.result=''
   return 0
  }
  
  // alert('result '+reply[0])
  // alert('response '+reply[1])
  // alert('data '+reply[2])

  //extract the response parameters
  var result=reply[0].toString()
  this.response=reply[1].toString()
  this.data=reply[2].toString()

  if (this.response.indexOf('Please login')>=0)
  {
   if (!this.login())
   {
    this.data=''
    this.response=('ERROR: Please login')
    this.result=''
    return(0)
   }
    
  }
  else
  {
   gotresponse=true
  }

 }
 
 if (result.split(' ')[0]=='OK' || result==1)
 {
 
  //save in cache
  //perhaps only cache READO requests to avoid
  // initiating cache to speed things up
  if (this.documentprotocolcode=='file'&&(trycache||request2.slice(0,4)=='READ'))
  {
   writecache(request2,this.data)
  }
 
  return true
 }
 else
 {
  return false
 }
 
}

function neosysdblink_send_byhttp_using_xmlhttp(data)
{

 //log(this.request)
 
 //prevent reuse
 if (this.requesting)
 {
  this.data=''
  this.response=('ERROR: ALREADY REQUESTING')
  this.response=this.response.replace(/"/,"'")
  this.result=''
  return 0  
 }
 this.requesting=true
  
 this.data=data?data:''
  
 //request is required
 if (this.request=='')
 {
  this.data=''
  this.response=('ERROR: NO REQUEST')
  this.response=this.response.replace(/"/,"'")
  this.result=''
  this.requesting=false
  return 0
 }

 //try to use cache
 var request2=this.request
 var trycache=(request2.slice(0,6)=='CACHE\r')
 if (trycache)
 {
  request2=request2.slice(6)
  var temp
  if (temp=readcache(request2))
  {
   this.data=temp
   this.requesting=false
   return true
  }
 }

 var token=glogincode
 var timeout=''
 var xtext='<root>'
 xtext+='<token>' + escape(token) + '</token>'
 xtext+='<timeout>' + escape(timeout) + '</timeout>'
 xtext+='<request>' + escape(request2) + '</request>'
 var thisrequest=this.request
 this.request=''
 //xtext+='<data>' + escape(this.data) + '</data>'
 //dont escape everything otherwise unicode will arrive in the database as encoded text
 //instead of being converted to system default single byte code page in server filesystem/asp
 //convert any ascii incompatible with xml text eg < > &
 //convert the escape character "%" FIRST
 ///temp=this.data.replace(/\x25/g,'%25').replace(/\x3C/g,'%3C').replace(/\x3E/g,'%3E').replace(/\x26/g,'%26')
 //temp=temp.replace(XMLre,escape($0)) 
 //also convert revelation delimiters so that they arrive unconverted
 ///temp=temp.replace(RMre,'%FF').replace(FMre,'%FE').replace(VMre,'%FD').replace(SMre'%FC').replace(TMre,'%FB').replace(STMre,'%FA')
 //temp=temp.replace(XXre,escape($0))
//XMLre is something like [\x25\x3C\x3E\x26\xFF\xFE\xFD\xFC\xFB\xFA]
 temp=this.data.replace(XMLXXre,function($0){return escape($0)})
 
 xtext+='<data>' + temp + '</data>'
 xtext+='</root>'
//if (gusername=='NEOSYS') alert(xtext)
/*
 //decide microsoft or moz/netscape 
 var moz=false
 try
 {
  var xhttp = new ActiveXObject('Microsoft.XMLHTTP')
  //var xhttp = new ActiveXObject('Msxml2.XMLHTTP.3.0')
 }
 catch(e)
 {
  var xhttp=new XMLHttpRequest()
  moz=true
 }
*/
   
 var xdoc=xtext
 
 //package description the request parameters into an xml object
 //if (!moz)
 //{
 // var xdoc = new ActiveXObject('Microsoft.XMLDOM')
 // xdoc.loadXML(xtext)
 //}

 var dbwaitingwindow
 var gotresponse=false
 
 while (!gotresponse)
 {

  if (request2.slice(0,7)!='RELOCK\n')
  {
   dbwaitingwindow=dbwaiting()
  }

  var xhttp=this.XMLHTTP

  //send the xmldoc to the server and get the response
  try
  {
   xhttp.Open('POST', NEOSYSlocation+'scripts/xhttp.asp', false)
   //this was never required but inserted in the hope that it will
   //avoid unknown problems. Perhaps it is not necessary for active pages like .asp.
   xhttp.setrequestheader("Pragma","no-cache");
   xhttp.setrequestheader("Cache-control","no-cache");
   //consider also putting the following in asp web pages
   //<% Response.CacheControl = "no-cache" %>
   //<% Response.AddHeader "Pragma", "no-cache" %>
   //<% Response.Expires = -1 %>
  }
  catch (e)
  {
   this.data=''
   this.response='Error: Could not communicate with server. Try again later'
   this.result=''
   dbready(dbwaitingwindow)
   this.requesting=false
   return 0
  }

  try
  {
   if (thisrequest.indexOf('LOGIN')<0) window.status='Requested '+new Date()+' '+thisrequest
   var timesent=new Date
   if (this.moz) xhttp.send(xdoc)
   else
   {
    //var iii=0
    xhttp.Send(xdoc)
    //alert(xhttp.readyState)
    //while (xhttp.readyState!=4)
    //{
    //alert(xhttp.readyState)
    // wstatus((iii++)+' '+xhttp.readyState)
    // xhttp.waitForResponse(1000);
    //}
   }
   log(window.status+' '+(new Date-timesent)+'ms')
   window.status=''
  }
  catch (e)
  {
   this.data=''
   this.response=('ERROR: '+e.number+' '+e.description+' in .Send')
   this.response=this.response.replace(/"/,"'")
   this.result=''
   dbready(dbwaitingwindow)
   this.requesting=false
   return 0
  }
  
  dbready(dbwaitingwindow)
  
  //check for valid response
  if (xhttp.responseXML.firstChild!=null)
  {

   //extract the response parameters
   var responsex=xhttp.responseXML.firstChild
   if (this.moz)
   {
    this.data=unescape(responsex.childNodes[0].childNodes[0].nodeValue)
    this.response=unescape(responsex.childNodes[1].childNodes[0].nodeValue)
    var result=unescape(responsex.childNodes[2].childNodes[0].nodeValue)
   }
   else
   {
    this.data=unescape(responsex.childNodes[0].text)
    this.response=unescape(responsex.childNodes[1].text)
    var result=unescape(responsex.childNodes[2].text)
   }
 
   if (this.response.indexOf('Please login')>=0)
   {

    if (!this.login())
    {
     this.data=''
     this.response=('ERROR: Please login')
     this.result=''
     this.requesting=false
     return 0
    }
    
   }
   else
   {
    gotresponse=true
   }
   
  }
  else
  {

   //system failure, retry/abort?
   var errormsg='System failure. Do you want to retry?'
   if (xhttp.responseText.indexOf('ASP 0113')>-1)
    errormsg+='\n\nThe server failed to get a response from the NEOSYS database engine'
   errormsg+='\n\n'+striptags(xhttp.responseText)
   if (!confirm(errormsg))
   {
   //xxx
    this.data=''
    this.response=('ERROR: '+unescape(xhttp.responseText))
    this.response=this.response.replace(/"/,"'")
    this.result=''
    this.requesting=false
    return 0
   }
   
  }
 
 }

 //indicate to refresher when last activity was
 neosyssetcookie('','NEOSYSlc',new Date,'lc')

 ///if (result.split(' ')[0]=='OK' || result==1)
 if (result.split(' ')[0]=='OK' || result==1)
 {
 
  //save in cache
  //perhaps only cache READO requests to avoid
  // initiating cache to speed things up
  if (trycache||request2.slice(0,4)=='READ')
  {
   writecache(request2,this.data)
  }
 
  this.requesting=false
  return true
 }
 else
 {
  this.requesting=false
  return false
 }
 
}

function dbwaiting()
{

 document.body.style.cursor='wait'
 var dialogStyle='dialogHeight: 50px; dialogWidth: 200px; dialogTop: px; dialogLeft: px; center: Yes; help: No; resizable: Yes; status: No;'
 var windowx=''//window.showModelessDialog(NEOSYSlocation+'wait.htm','',dialogStyle)
 wstatus('Networking, please wait ...')
 return windowx
}

function dbready(windowx)
{

 document.body.style.cursor='auto'
 wstatus('')
 try{windowx.close()}catch(e){}
}

//fix a bug/feature in internet explorer where closing a window opened with window.open causes loss of all non permanent cookies
function neosysfixcookie()
{
 var cookies=document.cookie.split('; ')
 for (var ii=0;ii<cookies.length;ii++)
 {
  if (cookies[ii].slice(0,10)=='ASPSESSION')
  {
   cookies[ii]+=';expires=Sun 31 Dec 2100 23:59:59 UTC;path=/;'
   document.cookie=cookies[ii]
   //do all
   //return true
  }
 }
 return true
}

function neosyssetcookie(loginsessionid,name,value,subkey,permanent)
{

neosysfixcookie()

 //for any particular permanent cookie it must consistently be set true otherwise possible loss of following cookies including ASPSESSION
 
 /*
 var cookie0='set cookie session:'+loginsessionid+' name:'+name+' subkey:'+subkey+' value:'+value+'\r'
 alert(cookie0)
 var tt=document.cookie.split('; ')
 for (var ii=0;ii<tt.length;ii++) cookie0+='\r'+ii+' '+tt[ii]
 if (permanent&&!confirm(cookie0)) xyx
 if (permanent&&typeof permanent!='boolean') xyxxxxxxxxx
 */
 permanent=false

 //value can be multiple values and keys in cookie format x=1&y=2 etc
 //to update many without replacing all
 value=value.toString()
 if (value.indexOf('=')>=0)
 {
  var values=value.split('&')
  for (var valuen=values.length-1;valuen>=0;valuen--)
  {
   var valuex=values[valuen]
   if (valuex)
   {
    valuex=valuex.split('=')
    if (valuex.length<2) valuex[1]=''
    neosyssetcookie(loginsessionid,name,valuex.slice(1).join('='),valuex[0],permanent)
   }
  }
  return
 }
 
 if (subkey)
 {
  // crumbs are separated by ampersands
  var crumbs = neosysgetcookie(loginsessionid,name).split('&')
  var emptycrumbn
  for (var i2=0; i2 < crumbs.length; i2++)
  {
   var crumb0=crumbs[i2].split('=')[0]
   if (crumb0==subkey)
   {
    crumbs[i2]=subkey+'='+value
    subkey=''
    break
   }
   if (crumb0=='') emptycrumbn=i2
  }
  if (subkey)
  {
   if (typeof emptycrumbn!='undefined') i2=emptycrumbn
   crumbs[i2]=subkey+'='+value
  }
  value=crumbs.join('&')
 }
 //if (value.indexOf('&&')>=0) xxxxxx
 
 //document.cookie='NEOSYSsystem=ACCOUNTS;path=/;expires=Sun 31 Dec 2100 23:59:59 UTC;'
 //value+=';expires=Sun 31 Dec 2000 23:59:59 UTC;'
 var cookie=loginsessionid+name+'='+value+';path=/;'
 //if (permanent==true)
 {
  cookie+=';expires=Sun 31 Dec 2100 23:59:59 UTC;'
 }
 //else if (permanent) cookie+=permanent
  
 document.cookie=cookie
 //alert(loginsessionid+name+'\r'+value+'\r'+document.cookie)
 //document.cookie = name + '=' + value+ 'domain='+window.location.host+';path=/;'
 
}

// Retrieve the value of the cookie with the specified name
//duplicated in client.js, decide.htm and decide2.htm
function neosysgetcookie(loginsessionid,key,subkey)
{

//var cookie0='GET COOKIE session:'+loginsessionid+' key:'+key+' subkey:'+subkey
//alert(cookie0)
 //cookies are separated by semicolons
 //accessing cookies in modaldialog windows when there is port number in the URL
 //cause unspecified security error in IE6 not prior versions
 var cookies = (typeof window.dialogArguments!='undefined'&&dialogArguments.cookie)?dialogArguments.cookie:document.cookie
 cookies=cookies.split('; ')
 var fullkey=loginsessionid+key
 for (var i=0; i < cookies.length; i++)
 {
  if (cookies[i].split('=')[0]==fullkey)
  {

   if (typeof subkey=='undefined'||subkey=='')
   {
    //return unescape(cookies[i].split('=')[1])
    return unescape(cookies[i].split('=').slice(1).join('='))
   }
   
   // crumbs are separated by ampersands
   var crumbs = cookies[i].substr(fullkey.length+1).split('&')
   for (var i2=0; i2 < crumbs.length; i2++)
   {
    if (crumbs[i2].split('=')[0]==subkey)
    {
     //return unescape(crumbs[i2].split('=')[1])
     return unescape(crumbs[i2].split('=').slice(1))
    }
   }
  }
 }
 return ''
}

//from "client.js" may also be copied in some "client.js" less windows
function neosysdecide(question,data,cols,returncoln,defaultreply,many,inverted)
{
 //data and cols are [[]] or [] or revstr or a;1:b;2 string
 //data cells .text property will be used if present
 //returncoln '' means return row number(s) - 1 based
 //returncoln means column number or property name in data
 // (data columns are usually numeric and 0 based)
 //defaultreply=if returncoln then cell(s) of returncoln otherwise 1 based rown(s)
 
 //the data might be a db request prefixed with '@'
 if (typeof data=='string'&&data.slice(0,1)=='@')
 {
  db.request=data.slice(1)
  if (!db.send())
  {
   neosysinvalid(db.response)
   return null
  }
  data=db.data
  
 }

 //abort if no records found
 if (data==''||data=='<records>\r\n</records>') return neosysinvalid('No records found')

 if (defaultreply)
 {
  if (typeof defaultreply=='string') defaultreply=defaultreply.split(':')
  if (typeof defaultreply!='object') defaultreply=[defaultreply]
 }
 
 if (!inverted) inverted=false
 var dialogargs=neosyscloneobj([question,data,cols,returncoln,defaultreply,many,inverted])
 dialogargs.logincode=glogincode

 var dialogStyle='dialogHeight: 400px; dialogWidth: 600px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'

 try
 {
  var results=window.showModalDialog(NEOSYSlocation+'decide.htm',dialogargs,dialogStyle)
  if (typeof results=='undefined') results=''
 }
 catch(e)
 {
  alert('Please enable popups for this site (5)\r\rError:'+e.description)
  results=''
 }
 
 return rearray(results)
 
}

function rearray(array)
{
 //fix a bug where arrays returned from modaldialogs are missing the method .join()

 if (typeof array!='object') return array
 
 var rearray=[]
 for (var i=0;i<array.length;i++) rearray[i]=array[i]
 
 return rearray
 
}

function neosysdecide2(question,data,cols,returncoln,defaultreply,many)
{

 //called from neosysfilepopup() and a few other places
 
 //data is xml string
 //cols is array of [title,dictid]
  
 //the data might be a db request prefixed with '@'
 if (typeof data=='string'&&data.slice(0,1)=='@')
 {
  db.request=data.slice(1)
  if (!db.send())
  {
   neosysinvalid(db.response)
   return null
  }
  data=db.data
  
 }

 //abort if no records found
 if (data==''||data=='<records>\r\n</records>') return neosysinvalid('No records found')
  
 var dialogargs=neosyscloneobj([question,data,cols,returncoln,defaultreply,many])
 var dialogStyle='dialogHeight: 400px; dialogWidth: 600px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'
 
 dialogargs.logincode=glogincode

 //popup
 //dialogStyle='dialogHeight: 400px; dialogWidth: 400px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'
 //reply=window.showModalDialog(NEOSYSlocation+'decide2.htm',dialogargs,dialogStyle)
 try
 {
  var results=window.showModalDialog(NEOSYSlocation+'decide2.htm',dialogargs,dialogStyle)
  if (typeof results=='undefined') results=''
 }
 catch(e)
 {
  alert('Please enable popups for this site (6)\r\rError:'+e.description)
  results=''
 }
 
 return rearray(results)
 
}

function setdropdown2(element,dataobj,colnames,selectedvalues,requiredvalues)
{

 //1st element is automatically selected unless selectedvalues overrides it.
 
 if (!checkisdropdown(element)) return 0

 cleardropdown(element)
		
 //if (no(xmldata.text)) return(0)
 if (no(dataobj.group1)) return 0
    
 cleardropdown(element,true)
 	
 var valuecolname
 var textcolname
 
 if (typeof(colnames)=='string')
 {
  textcolname=colnames
  valuecolname=colnames
 }
 else
 {
  valuecolname=colnames[0]
  textcolname=colnames[1]
 }
 
 //var records=xmldata.firstChild.childNodes
 var records=dataobj.group1

 //var options=[]
    
 for (var i=0;i<records.length;i++)
 {
  var cell=records[i][valuecolname]
  if (typeof(cell)=='undefined')
  {
   neosysinvalid('Error: "' +valuecolname+'" not in data for setdropdown2 for "'+element.id+'"')
   return(0)
  }
        
  var value=cell.text
        
  if (no(requiredvalues) || requiredvalues.neosyslocate(value))
  {
   var option1=document.createElement('OPTION')
   option1.value=value

   var cell=records[i][textcolname]
   if (typeof(cell)=='undefined')
   {
    neosysinvalid('Error: "' +textcolname+'" not in data for setdropdown2 for "'+element.id+'"')
    return(0)
   }
   
   //start a new option        
   //var option='<OPTION value='+value

   //indicate if selected
   var selected=false
   if (selectedvalues&&selectedvalues.neosyslocate(value))
   {
    //option+=' selected=true'
    option1.selected=true
   }
         
   var description=decodehtmlcodes(cell.text)
   
   if (ismac)
   {
    element.options[i]=new Option(description,value,selected,false)
   }
   else
   {
    //close the opening tag
    //option+='>'

    //add the option description
    //must be done before insertion otherwise width on mac ie5 is v small
    option1.innerHTML=description
    //option+=cell.text

    //add into the array of options
    element.insertBefore(option1,null)
    //element.options.add(option1)
    //options[options.length]=option+'</OPTION>'+'\r\n'
   }
  }
  
 }

 //select first option if none selected
// if (element.selectedIndex==-1) element.selectedIndex=0
 
 //force element to recalculate width (needed on ie55 but not mac ie5)
 element.parentNode.replaceChild(element,element)
 
 //does not seem to do anything
 //getancestor(element,'TABLE').refresh()
 
}

function cleardropdown(element,all)
{

 login('cleardropdown')
 
 if (!checkisdropdown(element)) return(0)

 //remove existing selections (leave one but blank it to avoid "0" showing)
 while (element.childNodes.length>(all?0:1))
 {
//  element.childNodes[0].removeNode(true)
  element.removeChild(element.childNodes[0])
 }
 if (element.childNodes.length)
 {
  element.childNodes[0].value=""
  element.childNodes[0].text=""
 }
 logout('cleardropdown')
}

function cleardropdownselections(element)
{

	if (!checkisdropdown(element)) return(0)

//    element.selectedIndex=-1
    if (element.childNodes.length) element.childNodes[0].selected=true
    
    //remove existing selections
	for (var i=0;i<element.childNodes.length;i++)
	{
	 element.childNodes[i].selected=false
	}
}

function checkisdropdown(element)
{

 //login('checkisdropdown')

 assertelement(element,'checkisdropdown','element')
    
 if (typeof(element)!='object' ||element.tagName!='SELECT')
 {
  neosysinvalid('Error: The target is not a SELECT tag')
  //logout('checkisdropdown fail')
  return false
 }
 //logout('checkisdropdown')
 return true
 
}

function setdropdown3(element,dropdowndata,colns,selectedvalues,requiredvalues)
{

 //NB cannot update elements options if currently bound to data because IE busts?
 //try clonenode+replacenode?
	
 //parameters
 ////////////

 //dropdowndata can be a string (even '') with character seps :; or fm vm (used if vm or fm found)
 //if null will not set new options, only select the selectedoptions
 //neosysrequired false means add a blank option at the beginning
 
 //element must be a select element
 if (!checkisdropdown(element))
 {
  log('setdropdown3 - not a select element')
  return false
 }

 //login('setdropdown3 '+element.id+' '+neosysquote(dropdowndata))
 login('setdropdown3 '+element.id)

 //dropdowndata may be an array of strings or a string with ;: seps for cols/rows respectively
 //if [] then dropdowndata are not updated otherwise existing dropdowndata are deleted
 if (typeof(dropdowndata)=='undefined'||dropdowndata==null) dropdowndata=[]
	
 //convert conversion string to an array
 if (typeof(dropdowndata)=='string')
 {
     
  //make sure we get at least one conversion
  if (dropdowndata=='') dropdowndata=';'

  //convert into an array
  var sepchars=(dropdowndata.indexOf(vm)>=0||dropdowndata.indexOf(fm)>=0)?fm+vm:':;'
  dropdowndata=dropdowndata.neosyssplit(sepchars)

 }

 //if not required make sure that the first dropdown option is ""
 if (dropdowndata.length&&!element.neosysrequired)
 {
  if (dropdowndata[0][0]!=''&&dropdowndata[dropdowndata.length-1][0]!='')
  {
   //ripple up, then replace first element
   for (var i=dropdowndata.length;i>0;i--) dropdowndata[i]=dropdowndata[i-1]
   dropdowndata[0]=['','']
  }
 }

// if (dropdowndata.length) breakpoint(element.id+' '+element.neosysrequired+' '+dropdowndata.tostatement())
  
 //colns may be array(2) being pointers into dropdowndata for option value and text
 //otherwise [0,1]
 if (!is(colns)) colns=[0,1]
 valuecoln=colns[0]
 textcoln=colns[1]

 //selectvalues is optionally the value(s) that are to be "selected"
 //optional, can be array or single
 if (typeof(selectedvalues)=='undefined') selectedvalues=[]
 if (typeof(selectedvalues)!='object') selectedvalues=[selectedvalues]
	
 //requiredvalues is optionally the value(s) that are to be included in the dropdown
 //optional, can be array or single
 if (typeof(requiredvalues)=='undefined') requiredvalues=[]
 if (typeof(requiredvalues)!='object') requiredvalues=[requiredvalues]

 //neosysinvalid(selectedvalues.join())
 //method
 ////////
    
 if (dropdowndata.length)
 {
 
  cleardropdown(element,true)
  
  for (var i=0;i<dropdowndata.length;i++)
  {
   var value=dropdowndata[i][valuecoln]
   if (no(requiredvalues) || requiredvalues.neosyslocate(value))
    {
     
     var text=dropdowndata[i][textcoln]
     if (typeof(text)=='undefined'||text=='')
     {
      text=value
      if (typeof text=='undefined') text=''
      else text=text.toString().neosysconvert('_',' ').neosyscapitalise()
     }

     addoption(element,value,text)
     
    }
   }
  }
	    
 //mark selected options
 if (selectedvalues.length)
 {

  //clear existing selections
  if (!dropdowndata.length) cleardropdownselections(element)
		
  for (var j=0;j<selectedvalues.length;j++)
  {
   var selectedvalue=selectedvalues[j]
   for (var i=0;i<element.childNodes.length;i++)
   {
    var option=element.childNodes[i]
    //if (selectedvalues.neosyslocate(option.value)) option.selected=true
    //if (selectedvalues.neosyslocate(option.text)) option.selected=true
    if (option.value==selectedvalue||option.text==selectedvalue)
    {
     option.selected=true
     break
    }
   }

   //add option if setting to a value not in the dropdown
   //(or error)
   if (i>=element.childNodes.length)
   {
    //except setting to '' means reset to first option

    if (selectedvalue=='')
    {
     // (also dataitem.text! why???)
     // but this has the result that
     //all first row drop downs are reset to first option
     //in the binding blankrowx so do not do it
     //ok do it only if bound and make sure not bound while clearing line 1
     if (element.dataitem) element.dataitem.text=element.value
    }
    else
    {
//     systemerror('setdropdown3()','"'+selectedvalue+'" is not in the dropdown')
     option=addoption(element,selectedvalue,selectedvalue)
     option.selected=true
    }
   }
   
  }
  
 }

 //force option 1 if no options
// if (element.neosysfieldno!='0'&&element.selectedIndex==-1) element.selectedIndex=0
    
 logout('setdropdown3')

 return true
 
}

function addoption(element,value,text)
{

 //build an option
 var option1=document.createElement('OPTION')
 option1.value=value
 option1.text=text

 //insert it into the dropdown
 //element.add(option1)//does not work on the mac for option 1 after clearing
 element[element.length]=option1
 
 return element[element.length-1]
 
}

//pls keep this routine synchronised in decide2.htm and scripts/client.htm
function neosysxml2obj(xmltext)
{

 var dataobj=new Object
 dataobj.group1=[]
 var recn=-1
 var currentrow
 xml=xmltext.split('<')
 for (var fragn=1;fragn<xml.length;fragn++)
 {
  var frag=xml[fragn].split('>')
  if (frag[0].toLowerCase()!='records'&&frag[0].toLowerCase()!='/records')
  {
   if (frag[0].toLowerCase()=='record')
   {
    dataobj.group1[++recn]=currentrow=new Object
   }
   else
   {
    if (frag[0].slice(0,1)!='/')
    {
     //currentrow[frag[0]]=(new Object).text=frag[1]
     var cell=new Object
     cell.text=frag[1]
     currentrow[frag[0]]=cell
    }
   }
  }
 }

 return dataobj
 
}


//copy any modifications to decide2.htm as well
function neosysobj2xml(obj)
{

 var xml='<record>'+cr
 for (var propname in obj)
 {
  if (propname.slice(0,5)=='group')
  {
  
   xml+='<'+propname+'>'+cr

   var group=obj[propname]
   for (var rown=0;rown<group.length;rown++)
   {
    xml+='<'+'row'+rown+'>'+cr
    var row=group[rown]
    for (var propname2 in row)
    {
//     if (row[propname2].element)
     {
      xml+='['+row[propname2].element.id+']'
      xml+='<'+propname2+'>'
      xml+=row[propname2].text
      xml+='</'+propname2+'>'+cr
     }
    }
    xml+='</'+'row'+rown+'>'+cr
   }
   
   xml+='</'+propname+'>'+cr
  }
  else
  {
   if (obj[propname].element)
   {
    xml+='['+obj[propname].element.id+']'
    xml+='<'+propname+'>'
    xml+=obj[propname].text
    xml+='</'+propname+'>'+cr
   }
  }
 }
 
 xml+='</record>'
 return xml
 
}

function neosyssetdropdown(element,request,colarray,selectedvalues,xyz)
{

 if (!checkisdropdown(element)) return(0)

 db.request=request
 
 if (db.send())
 {

  dataobj=neosysxml2obj(db.data)
  
  //xmltemp=new ActiveXObject('Microsoft.XMLDOM')
  //xmltemp.loadXML(db.data.replace(/\&/gi,'+'))
  ////xmltemp.loadXML(db.data)
  setdropdown2(element,dataobj,colarray,selectedvalues,null)
 }
 else
 {
  neosysinvalid(db.response)
 }
 return true
}

function getdropdown0(element)
{
 var index=element.selectedIndex
// breakpoint(index)

 //ie5 on mac appears to use index=length sometimes (when only one option?)
 if (index>=element.length) index=0

 //ie5 on pc uses -1 to indicate not selected
 if (index<0) return ''
 
 return element[index].value
 
}

function neosysgetdropdown(element,mode)
{
 if (!is(mode))mode='selected'//otherwise 'all'
 var selectedvalues=[] 
 for (var i=0;i<element.options.length;i++)
 {
  if (mode=='all' || element.options[i].selected)
  {
   selectedvalues[selectedvalues.length]=element.options[i].value
   if (mode!='all' && !element.multiple) return (selectedvalues.join())
  }
 }
if (mode!='all' && !element.multiple) selectedvalues=selectedvalues.join()
return(selectedvalues)
}


//this function provides a place to debug into asp client code
//stepping out of this function allows you to step through code in client for asp
function aspbreak()
{
 var x=''
}

function neosysinput(question,text,texthidden)
{
 if (!text) text=''
 return neosysyesnocancel(question,'','OK','','Cancel',text,texthidden)
}

function neosysyesno(question,defaultbutton)
{
 return (neosysyesnocancel(question,defaultbutton,'Yes','No')==1)
}

function neosysokcancel(question,defaultbutton)
{
 return neosysyesnocancel(question,defaultbutton,'OK','','Cancel')
}

function neosysyesnocancel(question,defaultbutton,yesbuttontitle,nobuttontitle,cancelbuttontitle,text,texthidden,image)
{
 question=question.toString()
 if (question.slice(0,6)=='Error:') question=question.slice(6)
 var dialogargs=[question,defaultbutton,yesbuttontitle,nobuttontitle,cancelbuttontitle,text,texthidden,image]
 var dimensions=(question.indexOf('\r')>=2)?'dialogHeight: 300px; dialogWidth: 600px;':'dialogHeight: 220px; dialogWidth: 500px;'
 try
 {
  var response=window.showModalDialog(NEOSYSlocation+'confirm.htm',dialogargs,dimensions+' center: Yes; help: No; resizable: No; status: No;')
  if (response+''=='undefined') response=0
  return response
 }
 catch(e)
 {
  alert('Please enable popups for this site (7)\r\rError:'+e.description)
  return 0
 }
 
}

function striptags(string)
{
 var temp=document.createElement('DIV')
 temp.innerHTML=string
 return temp.innerText
}

/*
function makeXMLisland(xmlelement,cmd)
{
 db.request=cmd+'\rXML'
 if(!db.send())
 {
  neosysinvalid(db.response)
  return false
 }
 xmlelement.loadXML(db.data)
 //document.writeln('<XML id='+id+'>')
 //document.writeln(db.data)
 //document.writeln('</XML>')
}
*/

function getancestor(startelement,ancestorTag)
{

 if (startelement==null) return(null)
 
 var ancestor=startelement.parentNode
 //while (ancestor!=null && ancestor.tagName!=ancestorTag)
 while (ancestor!=null && ancestorTag.indexOf(ancestor.tagName)==-1)
 {
  ancestor=ancestor.parentNode
 }
 return (ancestor)
}

/////////////////
//DEBUG FUNCTIONS
/////////////////

function wstatus(msg)
{
 window.status=msg
}

window.geventlog=''
window.geventstack=['','']
window.geventdepth=1

function login(eventdesc)
{
 if (!glogging) return
 
 //if (window_location.hostname!='sb_compaq') return
 
// geventdepth=geventdepth+1
 geventstack=geventstack.slice(0,geventdepth+1)
 geventstack[geventdepth]='}'+eventdesc
 log(geventstack[geventdepth])
 geventdepth=geventdepth+1
}

function logout(eventdesc)
{

 //returns false to return logout('xxx')
 
 if (!glogging) return false
 
 geventdepth=geventdepth-1
 if (geventdepth<0) geventdepth=0
// window.one
 if (geventstack[geventdepth].split(' ')[0]!='}'+eventdesc.split(' ')[0])
 {
  alert('logout does not match login\r\r'+eventdesc+' '+geventdepth+'\r\r'+geventstack.join('~'))
  windowx=windowopen()
  windowx.document.body.innerText=geventlog
 }
 
 geventstack[geventdepth]='{'+eventdesc

 log(geventstack[geventdepth])

 return false
  
}

var gfirstlogtime
var glastlogtime

function log(entrytext)
{
 
 if (!glogging) return

 //if (glogging>1) alert(entrytext)

 //limit the log size
 if (geventlog.length>20000)
 {
  //geventlog=geventlog.slice(geventlog.indexOf('\r',16000)+1)
  geventlog=geventlog.slice(5000)
 }

 //work out the delay
 var logtime=new Date()
 if (typeof glastlogtime=='undefined')
 {
  glastlogtime=logtime
  gfirstlogtime=logtime
 }
 var totlogtime='0000000000000000'+(logtime-gfirstlogtime)
 totlogtime=totlogtime.slice(totlogtime.length-9)
 var delayms='0000000000000000'+(logtime-glastlogtime)
 delayms=delayms.slice(delayms.length-9)
 
 //log it
 geventlog+=totlogtime+' '+delayms+neosysstr(' .',geventdepth)+entrytext+'<br>\r\n'
 
 glastlogtime=logtime
 
 //display it
 if (entrytext==geventstack[geventdepth]) entrytext=''
 window.status=totlogtime+geventstack.join('').slice(0,1000)+' '+entrytext

// if (!geventlogwindow) geventlogwindow=windowopen()
// geventlogwindow.document.body.innerHTML=geventlog.toString.slice(-1000)

// if (gstepping)
// {
//  //alert (entrytext)
//  neosysoswrite(geventlog,'c:\\debug.txt')
// }

}
var geventlogwindow

/////////////////
//CACHE FUNCTIONS
/////////////////

function loadcache()
{

 if (typeof(dbcache)=='object') return true

 login('loadcache')
 
 //cannot cache if called before document is loaded unless mac - behaviours not supported

 if (!document.body&&!ismac)
 {
  logout('loadcache - no document.body')
  return false
 }

 //try and get cache from parent window
 //if (window.parent&&(dbcache=window.parent.document.getElementById('dbcache')
 //{
 // if (typeof dbcache=='object')
 // {
 //  alert('ok cash')
 //  logout('loadcache ok - from parent window')
 //  return true
 // }
 //}
   
 //create a global variable span to hold the userdata
 dbcache=document.createElement('SPAN')
 dbcache.values=new Object
 try
 {
  dbcache.addBehavior ('#default#userData')
 }
 catch (e)
 {
  //return false
  //amazingly we can return true and still use the disconnected variable
  //as a cache ... although the .save .load methods are not supported
  logout('loadcache ok - no behavours - cannot .save .load etc')
  return true
 }
 
 var insertdocument=document
// if (window.dialogArguments) insertdocument=window.parent.document
 
 dbcache.id='dbcache'
 insertdocument.body.insertBefore(dbcache,document.body.firstChild)
//  document.body.insertBefore(dbcache,document.body.lastChild)
 
 //load the cache  
 var cacheloadstarted=new Date()
 window.status='Loading Cache'
 dbcache.load('\neosyscache')
 window.status=''
 dbcache.cacheupdated=false

 //warning if cache takes too long to load  
 var loadsecs=(new Date()-cacheloadstarted)/1000
 if (loadsecs>3)
 {
  clearcache()
  alert('Cache cleared')
 }
//alert(dbcache.XMLDocument.documentElement.xml)

 //clear the cache if in an old format
 if (dbcache.getAttribute('CACHEVERSION')!='1.00')
 {
  dbcache.load('emptycache')
  dbcache.setAttribute('CACHEVERSION','1.00')
 }
 else
 {
 
  //clear the cache if dataset and/or username is different
  if (dbcache.getAttribute('dataset_username')!=gdataset+'_'+gusername)
  {
   dbcache.load('emptycache')
   dbcache.setAttribute('dataset_username',gdataset+'_'+gusername)
  }
 
 }
  
 //try and ensure the the cache is saved eventually
 if (!window.onunload) window.onunload=savecache
 
 //fail if cannot open cache
 if (typeof(dbcache)!='object')
 {
  logout('loadcache - dbcache is not an object')
  return false
 }
 
 logout('loadcache - ok loaded')
 
 return true
 
}

function savecache()
{

 login('savecache')

 logout('savecache - skipped')
 return
  
 if (typeof(dbcache)!='object'||dbcache.id!='dbcache')
 {
  logout('savecache - no dbcache')
  return false
 }
 
 if (!dbcache.cacheupdated)
 {
  logout('savecache - not saved because not update')
  return true
 }
 
 //set the cache expiry date
 var ndaystocache=3
 var expirydate=new Date()
 expirydate.setUTCDate(expirydate.getUTCDate()+ndaystocache)
 dbcache.expires=expirydate.toUTCString()

 var result=''
 while (result=='')
 {
  try
  {
   window.status='Saving Cache'
   dbcache.save('\neosyscache')
   window.status=''
   result='OK'
  }
  catch (e)
  {
  
   //if cannot save one attribute then forget it
   var attributes=dbcache.XMLDocument.documentElement.attributes
   if (attributes.length<2) result='ko'

   //find the largest item
   var maxlen=0
   var attributename
   for (var i=0;i<attributes.length;i++)
   {
    if (attributes[i].value.length>maxlen)
    {
     maxlen=attributes[i].value.length
     attributename=attributes[i].name
    }
   }
   
   //remove the largest attribute and try again
   window.status='Reducing Cache'
   attributes.removeNamedItem(attributename)
   
  }
 }
 
 logout('savecache ok')

 return true
 
}

function showcache()
{

 if (!dbcache) return
 
 var values=dbcache.values

 //work out the total cache size (values only)
 var cachesize=0
 var cachen=0
 var html=''
 for (var key in values)
 {
  if (key.slice(0,6)!='neosys')
  {
   html+='<TR><TD>'+key+'</TD><TD>'+values[key]+'</TD></TR>'
  }
 }
 if (html)
 {
  var win=window.open()
  win.document.body.innerHTML='<TABLE>'+html+'</TABLE>'
 }
 
}

//prune cache every x seconds
var gcachepruneintervalsecs=100
var gmaxcachesize=1024000
var gmaxcachen=100
window.setInterval('trimcache()',gcachepruneintervalsecs*1000)
function trimcache()
{

 if (!dbcache) return
 
 var values=dbcache.values

 //work out the total cache size (values only)
 var cachesize=0
 var cachen=0
 for (var key in values)
 {
  if (key.slice(0,6)!='neosys')
  {
   cachesize+=values[key].length
   cachen++
  }
 }
 
 var result=[cachesize,cachen]
 if (gusername=='NEOSYS') wstatus('trimcache() cachesize:'+cachesize+' cacheitems:'+cachen)
 
 //delete initial keys until the size is below the desired limit 
 //hopefully the initial keys are the oldest
 for (var key in values)
 {
  if (cachesize<=gmaxcachesize&&cachen<=gmaxcachen) break
  if (key.slice(0,6)!='neosys')
  {
   cachesize-=values[key].length
   cachen--
   delete values[key]
  }
 }
 
 return result
 
}

function prunecache(request)
{

 //prunes all entries starting with request and returns the number of entries pruned

 //also prunes in window.opener and window.parent (and up chain if any)
 if (window.opener)
 {
  try
  {
   window.opener.prunecache(request)
  }
  catch(e){}
 }
 else if (window.parent&&window.parent!=window)
 {
  try
  {
   window.parent.prunecache(request)
  }
  catch(e){}
 }
 
 if (!dbcache) return
 
 var values=dbcache.values
 var npruned=0
 var prunekey=cachekey(request)
 
 for (var key in values)
 {
  if (key.slice(0,prunekey.length)==prunekey)
  {
   delete values[key]
   npruned+=1
  }
 }
 
 return npruned
 
}

function readcache(request)
{

 login('readcache')
 
 if (!loadcache())
 {
  logout('readcache - loadcache failed')
  return null
 }
 
 var key=cachekey(request)
 var result
 if (dbcache.values)
 {
  result=dbcache.values[key]
  
  //delete and restore to implement simple LRU cache
  //delete dbcache.values[key]
  //dbcache.values[key]=result
  
 }
 else
 {
  result=dbcache.getAttribute(key)

  //delete and restore to implement simple LRU cache
  //dbcache.removeAttribute(key)
  //dbcache.addAttribute(key,result)
  
 }
 
 logout('readcache ok')
 
 return result
 
}

function cachekey(request)
{
 
 login('cachekey')
 
 //return escape(this.dataset+'\r'+this.username+'\r'+request).replace(/%/gi,'_')
// var temp=escape(gdataset+'_'+request).replace(/%/g,'_').replace(/@/g,'_AT_').replace(/\//g,'_SLASH_').replace(/\*/g,'_STAR_')
 var temp=escape(request).replace(/%/g,'_').replace(/@/g,'_AT_').replace(/\+/g,'_PLUS_').replace(/\//g,'_SLASH_').replace(/\*/g,'_STAR_')
 
 //alert(temp)
 
 logout('cachekey '+temp)
 
 return temp
 
}

function writecacherecord(data,filename,key)
{
 //if data is an array then assume data[0] is the key and remove it
 //default key from data[0]
 if (typeof data=='object')
 {
  if (!key) key=data[0]
  data=data.slice(1).join(fm)
 }
 writecache('READ\r'+filename+'\r'+key,data)
}
  
function deletecacherecord(filename,key)
{
 deletecache('READ\r'+filename+'\r'+key)
}

function writecache(request,data)
{
 login('writecache')

 if (!loadcache())
 {
  logout('writecache loadcache failed')
  return false
 }
 
 if (dbcache.values) dbcache.values[cachekey(request)]=data
 else dbcache.setAttribute(cachekey(request),data)
  
 dbcache.cacheupdated=true
 
 logout('writecache')
 
 return true
 
}

function deletecache(request)
{
 login('deletecache')

 if (!loadcache())
 {
  logout('deletecache loadcache failed')
  return false
 }
 
 if (dbcache.values) delete dbcache.values[cachekey(request)]
 else dbcache.removeAttribute(cachekey(request))
 
 dbcache.cacheupdated=true
 
 logout('deletecache')
 
 return true
 
}

function clearcache()
{

 login('clearcache')

 gtasks=null
 
 if (!loadcache())
 {
  logout('clearcache loadcache failed')
  return false
 }
 
 try
 {
  //will fail on mac
  dbcache.load('emptycache')
  dbcache.values=new Object
  dbcache.save('\neosyscache')
 }
 catch(e)
 {
  if (ismac)
  {
   //does not work on mac despite being documented
   //dbcache.clearAttributes()
   dbcache=document.createElement('SPAN')
  }
  else
  {
   logout('clearcache fail '+e.description)
   return false
  }
 }
 
 dbcache.cacheupdated=false
 logout('clearcache ok')

 return true
   
}

function sorttable(colid)
{

 if (typeof form_presort=='function')
 {
  //neosysevaluate('form_presort()','formfunctions_onload()')
  if (!form_presort(colid)) return neosysinvalid()
 }
 
 //determine the groupno
 var dictitem=gds.dictitem(colid)
 if (!dictitem) return neosysinvalid()
 var groupno=dictitem.groupno
 if (!groupno) return neosysinvalid(colid+' is not multivalued for sorting')
 
 window.status='Sorting, please wait ...'
 
 //get the data rows
 if (!groupno) groupno=1
 var datarows=gds.data['group'+groupno]
 var fromrown=0
 var uptorown=datarows.length-1
 
 //get the data
 var data=[]
 var dictitemlength
 if (dictitem.align=='R')
 {
  dictitemlength=dictitem.length
  if (!dictitemlength) dictitemlength=20
 }
 var sortdatalevels=[]
 for (var rown=0;rown<datarows.length;rown++)
 {
  var sortdata=datarows[rown][colid].text
  if (dictitemlength)
  {
   if (sortdata)
   {
    var temp=new neosysamount(sortdata)
    sortdata=temp.amount.split('.')
    if (!sortdata[1]) sortdata[1]=''
    sortdata=temp.unit+neosysspace(dictitemlength-sortdata[0].length)+sortdata[0]+temp.amount
   }
  }
  else
  {
   //if indented then work out prefix from higher levels
   if (sortdata.match(/(^\s+)/))
   {
    var prefix=''
    var indent=RegExp.$1.length
    for (var i=0;i<indent;i++)
    {
     if (sortdatalevels[i]) prefix+=sortdatalevels[i]
    }
    //save the current level
    sortdatalevels=sortdatalevels.slice(0,indent)
    sortdatalevels[indent]=sortdata
    //add the prefix
    if (prefix) sortdata=prefix+sortdata
   }
   else
   {
    sortdatalevels=[sortdata]
   }
   
  }

  //right justify the rown number assume max 999999  
  var temp=rown.toString()
  temp=neosysspace(6-temp.length)+temp
  
  //save the two column array for sorting
  //need the +temp so that indented fields sort correctly
  data[rown]=[sortdata+temp,temp]
  
 }
 
 //get the table rows
 var tablex=document.getElementById('neosysgroup'+groupno)
 if (tablex) var tablerows=tablex.tBodies[0].rows
 
 //sort the data and table rows
 //////////////////////////////
 //NB .sort() will sort arrays of arrays using all elements of the subarray in turn
 //in our case we make subarray 2nd element
 // the original rown number (right justified) so .sort() is STABLE!!!
 //ie original row order is preserved for duplicate sorted items
 //this means that you can sort on one column and then another
 //and the first column sorted *remains sorted* within duplicate values of
 //of the second column sorted.
 //eg to sort by company AND name .. sort on name first then company
 data=data.sort()
 //QuickSort(data,fromrown,uptorown)
 
 //set the order image
 var order=(event.srcElement.src.indexOf('down')>=0)?'up':'down'
 resetsortimages(groupno)
 event.srcElement.src='../neosys/images/smallsort'+order+'.gif'
 
 //reverse if sorting descending
 if (order=='up') data.reverse()
 
 //get an array of the tablerows
 var oldrows=[]
 for (var oldrown=0;oldrown<tablerows.length;oldrown++) oldrows[oldrown]=tablerows[oldrown]
 
 //reorder data and table rows
 var newdatarows=[]
 for (var newrown=0;newrown<data.length;newrown++)
 {
  
  var oldrown=(+Math.floor(data[newrown][1]))
  
  //build new ordered data
  newdatarows[newrown]=datarows[oldrown]
  
  //rearrange table rows
  if (newrown!=oldrown) tablerows[newrown].swapNode(oldrows[oldrown])
  
 }
 
 //replace ordered data
 gds.data['group'+groupno]=newdatarows
 
 if (typeof form_postsort=='function')
 {
  //neosysevaluate('form_postsort()','formfunctions_onload()')
  if (!form_postsort(colid)) return neosysinvalid()
 }
 
 window.status=''
 
}

//******
//*MENU*
//******

//var mmm=0
gmenuloaded=false
function menuonload()
{
//window.status=++mmm
 menuframe=document.getElementById('menuframe')

 //iframe.onload supported in ie55
 //if (typeof menuframe.menudiv=='undefined'||menuframe.menucompleted.readyState!='complete') return
 if (!menuframe.menucompleted) return

 window.clearInterval(gmenuonloader)
  
 login('menuonload')
 
 //menux.innerHTML=menuframe.document.body.innerHTML
 //for some reason the iframe contains the span and iframe
 //so the above does not work!!!
 //so loop through the elements of the iframe skipping menux and menuframe
 var temp=''
 var childNodes=menuframe.document.body.childNodes
 for (var i=0;i<childNodes.length;i++)
 {
  if (childNodes[i].outerHTML&&childNodes[i].id!='menux'&&childNodes[i].id!='menuframe')
  {
   temp+=childNodes[i].outerHTML
  }
 }
 menux.innerHTML=temp
 //menux.zIndex=999 //done in stylesheet but does not work over select elements (see MS docs)
 
 gmenuloaded=true
 
 logout('menuonload')

}

function menuhide(element)
{

 var okmenus=false

 if (!element) element=menudiv.parentNode
  
 if (!element.menuok) okmenus=neosysgetcookie(glogincode,'NEOSYS2','m').split(',')
 
 if (!element.neosysmenuaccesskeys) element.neosysmenuaccesskeys=[]
 
 var childnodes=element.childNodes

nextchild:
 for (var i=element.childNodes.length-1;i>=0;i--)
 {
  var child=childnodes[i]
  
  //delete some menus
  if (okmenus&&okmenus.join()!=''&&child.id&&child.id.slice(0,5)=='menu_')
  {
   if (!okmenus.neosyslocate(child.id.slice(5)))
   {
    //ignore failure due to too early key response
    try
    {
     child=element.removeChild(child)
     continue nextchild
    }
    catch(e){}
   }
  }
  
  if (child.className=='menu'||child.parentNode.className=='menu')
  {
   if (child.tagName=='DIV')
   {
    child.className='menu'
    child.style.display='none'
    menuhide(child)
   }
   if (child.tagName=='A')
   {
    child.onmouseover=menuonmouseover
    child.onmouseout=menuonmouseout
    //child.onfocus=menuonmouseover
    //child.onblur=menuonmouseout
    child.className='menuitem'
    
    if (typeof child.neosyssubmenuoption=='undefined')
    {
    
     //save an array of elements by accesskey in the menu
     var underlineelement
     if ((underlineelement=child.getElementsByTagName('U')).length)
     {
      var menuaccesskey=underlineelement[0].innerText.neosystrim().slice(0,1).toUpperCase()
      var temp=element.neosysmenuaccesskeys[menuaccesskey]
      if (gusername=='NEOSYS'&&temp) neosysnote('Duplicate menu access key '+menuaccesskey.neosysquote()+' for\r'+child.innerText+'\rand\r'+temp.innerText)
      element.neosysmenuaccesskeys[menuaccesskey]=child
     }
    }
     
    //add submenu indicators
    if (!child.href&&!child.onclick&&typeof child.neosyssubmenuoption=='undefined')
    {
     child.neosyssubmenuoption=true
     //child.style.width=child.parentNode.runtimeStyle.width
     //child.style.border='1px solid'
     if (element.id!='menudiv') child.innerHTML+=' ...'

    }
    else
    {
     child.neosyssubmenuoption=false
    }
    
   }
  }
 }

 element.menuok=true
 
}

var gnmenus=0
var gmenutimeout=''

function menuonmouseover(menuoption)
{

 if (!gmenuloaded) return false
 
 //like esc
 if (menuoption=='click')
 {
  menuoption=''
  if (gnmenus)
  {
   gnmenus=0
   menuclose()
   window.event.cancelBubble=true
   return false
  }
 }
 
 var keyboarding=menuoption
 gnmenus=1
 window.clearTimeout(gmenutimeout)

 //hide select elements as they show show through the drop down menus (dont respect z-order)
 if (document.styleSheets[0].rules[0].selectorText!='SELECT') document.styleSheets[0].addRule('SELECT','visibility:hidden',0)

 //get the menuoption
 if (!menuoption)
 {
  menuoption=window.event.srcElement
  if (!menuoption) return
 }
 //click detected on childnode
 if (menuoption.id=='menubutton_label') menuoption=menuoption.parentNode
 
 var menu=menuoption.parentNode

 //window.status=new Date+' '+menu.clientLeft+' '+menu.clientWidth
 var leftoffset
 var topoffset
 if (menuoption.id=='menubutton'||menuoption.tagName=='BUTTON'||menuoption.id.slice(0,5)=='menu_')
 {
//  leftoffset=menuoption.offsetWidth+menuoption.offsetLeft-4
//  topoffset=0
  leftoffset=menuoption.offsetLeft+1
  topoffset=menuoption.offsetHeight+menuoption.offsetTop-1
 }
 else
 {
  leftoffset=menu.offsetWidth-menu.clientLeft-8
  topoffset=-menu.clientTop
  topoffset=0
 }
 
 menuchangeoption(menu,menuoption)
 
 //menu.focus()
 //menu.onkeydown=menuonkeydown

 //looks for a following menudiv to open
 //otherwise returns
 var submenu=menuoption
 while (submenu=submenu.nextSibling)
 { 
  if (submenu.tagName) break
 }
 //submenu=document.all[menuoption.sourceIndex+1]
 if (!submenu||submenu.className!='menu')
 {
  if (!submenu) submenu=window.event.srcElement
  if (!submenu) return
 }

 //close any other submenus
 menuhide(menu)

 //open the submenu
 
 // submenu.style.left=window.event.srcElement.clientLeft+window.event.srcElement.clientWidth
 submenu.style.left=leftoffset
 if (topoffset) submenu.style.top=topoffset
 submenu.style.display='inline'
 submenu.noWrap=true
//alert(submenu.outerHTML)  
 //highlight first menu item if none highlighted
 if (keyboarding||submenu.id=='menudiv')
 {
  if (!submenu.highlightedelement)
  {
   //look for nextsibling
   var subelements=submenu.childNodes
   for (var subelementn=0;subelementn<subelements.length;subelementn++)
   {
    if (subelements[subelementn].tagName=='A')
    {
     menuchangeoption(submenu,subelements[subelementn])
     break
    }
   }
  }

  //simple submenu.focus doesnt work as usual ... blanks the menudiv!
  menufocus(submenu)  
  submenu.onkeydown=menuonkeydown
  
 }

}

//sets a timeout to focus on an element
//OR executes the timeout function
var gmenuelement;
function menufocus(menu)
{

 //if given an element save it global and set timeout callback
 if (menu)
 {
  gmenuelement=menu
  window.setTimeout('menufocus()',1)
  return
 } 
 
 //focus on the saved element
 try{gmenuelement.focus()} catch(e){}
 
}

function menuonkeydown(menu,key)
{
 
 //usually called as an event with no parameters
 //doesnt seem to work well except called as an event
 if (!menu||!key)
 {
  menu=window.event.srcElement
  key=window.event.keyCode
 }
 
 var menuoption=menu.highlightedelement
 if (!menuoption) return
 
 var horizontal=(menu.id=='menudiv')

 //35 End 36 Home 37 Left 38 Up 39 Right 40 Down
 //65-90 A-Z
 var alphakey=(key>=65&&key<=90)
 var endkey=key==35
 var homekey=key==36
 var leftkey=key==37
 var upkey=key==38
 var rightkey=key==39
 var downkey=key==40
 var esckey=key==27
 var enterkey=key==13
 
 //wstatus(new Date+' '+key)

 //esc ... close menu and return focus to where it was when menu was opened
 if (esckey||(!horizontal&&leftkey))
 {
  if (menu.parentNode.parentNode.highlightedelement)
  {
   menuonmouseover(menu.parentNode.parentNode.highlightedelement)
   //window.setTimeout('menuonkeydown(13)',10)
   //attempt to press Enter on it
   //menuonkeydown(menu.parentNode.parentNode,13)
   if (window.event) window.event.cancelBubble=true
   return false
  }
  else
  {
   gnmenus=0
   menuclose()
   if (window.event) window.event.cancelBubble=true
   return false
  }
 }

 //menu access key?
 if (alphakey&&menu.neosysmenuaccesskeys)
 {
  var newmenuoption=menu.neosysmenuaccesskeys[String.fromCharCode(key)]
  if (newmenuoption)
  {
   menuoption=newmenuoption
   //will be done below menuchangeoption(menu,menuoption)
   enterkey=true
   alphakey=false
  }
 }
 
 //enter key opens a menu or click a link (shift+enter to open a new window)
 if (enterkey||(horizontal&&downkey)||(!horizontal&&rightkey))
 {
  if (enterkey&&menuoption.href)
  {
   try
   {
    if (menuoption.onclick) menuoption.onclick()
    else
    {
     if (window.event&&window.event.shiftKey) windowopen(menuoption.href)
     else window.location=menuoption.href
    }
   }
   catch(e){}
  }
  else if (enterkey&&menuoption.onclick)
  {
   try
   {
    menuoption.click()
   }
   catch(e){}
  }
  else
  {
   if (!menuoption.href&&!menuoption.onclick) menuonmouseover(menuoption)
  }
  if (window.event) window.event.cancelBubble=true
  return false
 }
 
 //right/left/up/down arrows and home/end
 //or letter keys to search for that item
 if ((endkey||homekey)||(horizontal&&(leftkey||rightkey))||(!horizontal&&(upkey||downkey))||alphakey)
 {
 
  var newmenuoption
  
  //search forward from current element to the end
  //(or backwards to beginning if left/up key)
  if ((key>=37&&key<=40)||alphakey)
  {
   newmenuoption=menuoption
   while (newmenuoption&&(newmenuoption=((leftkey||upkey)?newmenuoption.previousSibling:newmenuoption.nextSibling)))
   {
    if (newmenuoption.tagName=='A')
    {
    
     //if cursor key then break at first available menu option 
     if (!alphakey) break
     
     //or break if the first letter matches the alpha code entered
     if (newmenuoption.innerText.neosystrim().slice(0,1).toUpperCase()==String.fromCharCode(key)) break
     
    }
   }
  }
  
  //resume search from the beginning (or end if left/up key)
  //search from the beginning if pressed Home
  //search from the end if pressed End
  if (!newmenuoption)
  {
   newmenuoption=((endkey||leftkey||upkey)?menu.lastChild:menu.firstChild)
   while (newmenuoption)
   {
    if (newmenuoption.tagName=='A')
    {
    
     //if cursor key then break at first available menu option 
     if (!alphakey) break
     
     //ir break if the first letter matches matches the alpha code entered
     if (newmenuoption.innerText.neosystrim().slice(0,1).toUpperCase()==String.fromCharCode(key)) break
     
    }
    newmenuoption=((endkey||leftkey||upkey)?newmenuoption.previousSibling:newmenuoption.nextSibling)
   }
  }
 }
  
 //if found a new option then move highlight to it
 if (newmenuoption&&newmenuoption.tagName=='A'&&newmenuoption!=menuoption)
 {

  menuchangeoption(menu,newmenuoption)

  if (window.event) window.event.cancelBubble=true
  return false
    
 }
 
 //window.event.cancelBubble=true
 //window.event.returnValue=false
 
 return true
 
}

function menuchangeoption(menu,newmenuoption)
{
 
 //skip if over underlined element (bubble will still highlight the parent)
 if (newmenuoption.tagName=='U') return
 
 //clear old menu item to default colors
 var menuoption=menu.highlightedelement
 if (menuoption)
 {
  menuoption.style.backgroundColor=''
  menuoption.style.color=''
 }
 
 if (!newmenuoption) return
 
 //highlight new menu item
 //newmenuoption.style.backgroundColor='D0D0D0'
 
 newmenuoption.style.backgroundColor='highlight'
 newmenuoption.style.color='highlighttext'

 //record new highlighted item in the menu
 menu.highlightedelement=newmenuoption
   
 //focus and onkeydown should remain on the menu div not the item
 if (!true)
 {
  //menu.focus()
  gmenuelement=menu
  window.setTimeout('menufocus()',10)
 }
 menu.onkeydown=menuonkeydown
  
}

function menuonmouseout()
{
 gnmenus=0
 gmenutimeout=window.setTimeout('menuclose()',1000)
 //window.event.srcElement.style.color='black'
}

function menuclose()
{
 
 if (gnmenus==0)
 {
  menuhide()
  menubutton.style.backgroundColor=''
  menubutton.style.color=''
  menubutton.tabIndex=9999
 }
 
 //redisplay select elements
 if (document.styleSheets[0].rules[0].selectorText=='SELECT') document.styleSheets[0].removeRule(0)
 
}

function menubuttonhtml(id,imagesrc,name,title,accesskey,onclickexpression)
{

 var tx='<span'
 if (id) tx+=' id="'+id+'"'
 tx+=' border=1 class=graphicbutton'
 tx+=' onmousedown=this.style.borderStyle="inset"'
 tx+=' onmouseup=this.style.borderStyle="outset"'
 tx+=' onmouseout=this.style.borderStyle="outset"'
 if (title) tx+=' title="'+title+'"'
// tx+=' style="float:left"'
 if (accesskey) tx+=' accesskey="'+accesskey+'"'
 if (onclickexpression) tx+=' onclick="return '+onclickexpression+'"'
 tx+='>'
 
 if (imagesrc)
 { 
  tx+='<IMG src='+imagesrc
  tx+='>'
 }
 
 //tx+='<br>'
 
 tx+='<span id='+id+'_label style="font-size:100%; vertical-align:super">'
 tx+=' '+name
 tx+='</span>'
 
 tx+='</span>'

 //create a tiny button to capture the accesskey to prevent it from being used by the browser
 if (accesskey)
 {
  tx+='<button tabindex=-1 style="background-color:white; height:1;width:1"'
  tx+=' accesskey="'+accesskey+'"'
  tx+=' onclick="return '+onclickexpression+'"'
  tx+='></button>'
 }
 
 return tx

}

function setgraphicbutton(button,labeltext,src)
{
 var label=document.getElementById(button.id+'_label')
 label.innerHTML=labeltext
 if (src) button.getElementsByTagName('IMG')[0].src=src
}

function refresh_onclick()
{
 if(clearcache())
  neosysnote('All NEOSYS data cached in this window has been cleared\rand will be retrieved from the server again as and when required.')
 // \r\rN.B. NEOSYS forms and scripts will remain cached and may\rbe updated when you close and reopen all browser\rwindows - depending on the cache settings in your browser.')
 else
  neosysnote('Cannot clear cache.')
 return true
}
