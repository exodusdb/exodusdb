 // © 2000-2006 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**
 
 var gdatatagnames=/(^SPAN$)|(^INPUT$)|(^TEXTAREA$)|(^SELECT$)/
 var gtexttagnames=/(^SPAN$)|(^INPUT$)|(^TEXTAREA$)/
 var gradiocheckboxtypes=/(^radio$)|(^checkbox$)/
 
 //gnewimage=gimagetheme+'application_form_add.png'//'add.png'
 gnewimage=gimagetheme+'application.png'//'add.png'
 gopenimage=gimagetheme+'open.gif'
 gfindimage=gimagetheme+'zoom.png'
 gcalendarimage=gimagetheme+'calendar.gif'
 gsaveimage=gimagetheme+'tick.png'
 gcloseimage=gimagetheme+'cross.png'
 greleaseimage=gimagetheme+'key.png'//
 greopenimage=gimagetheme+'key.png'//'lock.png'
 gdeleteimage=gimagetheme+'application_form_delete.png'//'delete.png'
 glistimage=gimagetheme+'table.png'
 gprintsendimage=gimagetheme+'printer.png'//'application_form.png'
 ginsertrowimage=gimagetheme+'add.png'
 gdeleterowimage=gimagetheme+'delete.png'
 gexpandrowimage=gimagetheme+'smallexpand.gif'
 gsortimage=gimagetheme+'smallsort.gif'
 glinkimage=gimagetheme+'application_form.png'
 gfirstimage=gimagetheme+'resultset_first.png'
 glastimage=gimagetheme+'resultset_last.png'
 gnextimage=gimagetheme+'resultset_next.png'
 gpreviousimage=gimagetheme+'resultset_previous.png'
 gspacerimage=gimagetheme+'spacer.png'
 
 // document.getElementsByTagName('BODY').onload=window_onload
 window.onbeforeunload=window_onbeforeunload
 window.onunload=window_onunload
 
 //document.onmousedown=document_onmousedown
 document.onkeydown=document_onkeydown
 
 //form function global variables
 var gpagenrows=10
 var gkeycode
 var gdictfilename
 
 var gparameters
 //move any input parameters over from parent windows
 //this is done to avoid using ?param=etc in the url which prevents caching
 //and is therefore slower
 var gwindowopenerparameters=''
 try
 {
  if (window.opener&&window.opener.gwindowopenparameters)
  {
   gwindowopenerparameters=window.opener.gwindowopenparameters
   //delete the parameters so that they are only used once
   window.opener.gwindowopenparameters=null
  }
 }
 catch(e){}
 if (gwindowopenerparameters)
 {
  for (var paramname in gwindowopenerparameters)
  {
   gparameters[paramname]=gwindowopenerparameters[paramname]
  }
 }
 if (gparameters.readonlymode) greadonlymode=true
 
 var gonfocuselement
 var gpreviouselement=null
 var gnextelement=null
 var gdependents=[]
 var gKeyNodes=false//init will get an array of key nodes if any
 var gkeyexternal=''//external format of key eg STEVE*1/1/2000
 var gkey=''//internal format of key eg STEVE*12080
 var gkeys=[]
 var gkeyn=0
 var gloaded=false
 var gfirstelement
 var gfirstnonkeyelement
 var gstartelement
 var gfinalinputelement
 var gchangesmade=false//set true in validateupdate exit and delete row (not insert row)
 var gallowsavewithoutchanges=false//allows locked records (with keys) to be saved anyway
 var glocked=false//true means record is locked and available for edit
 var grelockingdoc=false
 var gwindowunloading=false
 var gclosing=false
 //var gisnewrecord replaced by gds.isnewrecord
 var grelocker
 var gro
 var gmaxgroupno=0
 var gfields=[]
 var gtables=[]
 var gsortimages=[]
 var gdoctitle=document.title
 var ginitok=false
 var grecn=null
 var gfocusonelement
 var gcell
 var gvalue
 var gorigvalue
 var gtimers
 //var gsavemode replaced by gparameters.savemode
 var greadonlymode
 var gupdateonlymode
 var gpreventcreation
 var gpreventdeletion
 var gcalendarscript//1 if needed, 2 if script inserted, 3 if script loaded
 
 //form customisation (must be in a script loaded prior to this dbform script)
 if (typeof(form_preinit)=='function')
 {
  neosysevaluate('form_preinit()','dbform.js()')
 }
 
 //add a script for the data file definition in case not included in the main html
 if (typeof(gmodule)=='undefined')
 {
  gmodule=''
  window.setTimeout('formfunctions_onload',10)
 }
 else
 {
  if (!gdictfilename) gdictfilename=gdatafilename
//  var temp='<SCR'+'IPT onload="formfunctions_onload()" id=maindict src="scripts/'+gdictfilename.toLowerCase()+'_dict.js"></SCR'+'IPT>'
  if (eval('typeof dict_'+gdictfilename+'=="undefined"'))
  {
   var temp='<SCR'+'IPT id=maindict src="scripts/'+gdictfilename.toLowerCase()+'_dict.js"></SCR'+'IPT>'
   document.writeln(temp)
  }
 }
 
 var gds
 
//'WINDOW LOAD
//''''''''''''

function formfunctions_onload()
{
 
 wstatus('Initialising, please wait ...')
 
 //has the problem that clicking on popup images "activates" the surrounding TD!
 //old method is to instate onfocus on all neosys data elements
 
 document.body.onactivate=onfocus
 
 gds=new neosysdatasource
 gds.onreadystatechange=gds_onreadystatechange
 
 //flag to later events that onload has not finished (set true at end of window_onload)
 ginitok=false
 
 //make sure not "framed"
 if (window!=window.top) top.location.href=self.location.href
 
 login('formfunctions_onload')
 
 //form customisation
 if (typeof form_onload=='function')
 {
  if (!neosysevaluate('form_onload()','form_onload()')) return
 }

 //make sure we are logged in
 if (!gusername)
 {
  db.login()
 }
 //done in client.js after db
 //gbasecurr=neosysgetcookie(glogincode,'NEOSYS2','bc')
 //gbasefmt=neosysgetcookie(glogincode,'NEOSYS2','bf')

 //check a parameter
 if (typeof gmodule=='undefined')
 {
  systemerror('formfunctions_onload()','gmodule is not defined')
  return logout('formfunctions_onload - no gmodule')
 }

 //check dictionary function exists
 var temp
 try
 {
  temp=eval('dict_'+gdictfilename)
 }
 catch(e){}
 if (typeof temp!='function')
 {
  systemerror('dict_'+gdictfilename+' dict function not defined')
  return logout('formfunctions_onload - dict function missing')
 }

 gro=new neosysrecord(neosysevaluate('dict_'+gdictfilename+'(gparameters)','formfunctions_onload()'),gdatafilename)
 gds.dict=gro.dict
 
 //calendar popup functions
 if (gcalendarscript==1)
 {
  var element=document.createElement('SCRIPT')
  element.src=NEOSYSlocation+'scripts/calendar.js'
  document.body.insertBefore(element)
  //document.writeln('<SCR'+'IPT src="'+NEOSYSlocation+'scripts/calendar.js" language=javascript></SCR'+'IPT>')
 }
  
 //greadonlymode=false

 //set the datasource of all elements
 datasrcelements=[]

// var elements=document.all
// for (var elementn=0;elementn<elements.length;elementn++)
// {
//  var element=elements[elementn]
 element=document.body
 while (true)
 {
  if (!(element=nextelement(element))) break
  if (element.tagName=='BUTTON'&&!element.tabIndex) element.tabIndex=999
  
  //NB inserting elements within the loop means that the same element
  //may be processed more than once so ensure skip on 2ndtime
//  if(typeof element.neosysgroupno=='undefined'&&element.tagName!='OPTION'&&element.dataFld!=''&&typeof element.dataFld!='undefined')
//element.dataFld ... gives error "class does not support automation"
//  if(typeof element.neosysgroupno=='undefined'&&element.dataFld&&element.tagName!='OPTION'&&element.dataFld!=''&&typeof element.dataFld!='undefined')
  if(typeof element.neosysgroupno=='undefined'&&element.tagName!='OPTION'&&element.dataFld!=''&&typeof element.dataFld!='undefined')
  {
  
   //locate the dictionary item otherwise skip the field
   var dictitem=gro.dictitem(element.dataFld)
   if (!dictitem)
   {
    systemerror('formfunctions_onload()','Form element '+neosysquote(element.dataFld)+' is not in the '+neosysquote(gdictfilename)+' dictionary\ror is not in the correct group.')
    continue
   }
   
   //check that the dataFld is not in use elsewhere (function or other field)
   var temp=''
   try
   {
    temp=eval(element.dataFld)

    //ignore previously built radio or checkboxes
    //zzzshould really check all elements
    if (temp==element) temp=''
    if (typeof temp=='object'&&typeof temp.length=='undefined') temp=[temp]
    if (temp[0].type&&(temp[0].type=='radio'||temp[0].type=='checkbox')) temp=''
   }
   catch (e){}
   if (temp)
   {
    systemerror('dataFld '+neosysquote(element.dataFld)+' is already in use elsewhere\r'+eval(temp).toString().slice(0,100)+'\r...')
    continue
   }

   //convert into DOM node
   element.id=element.dataFld
   //there is a problem with this for radio boxes as there are multiple elements for one id
   if (element.type!='radio'&&element.type!='checkbox')
   {
    element=document.getElementById(element.id)
   }
   //element=eval(element.id)

   log('formfunctions_onload '+element.dataFld)

   //dictionary modifications
   //none - currently done in dictrec builder

   //convert long text input to spans so that it can flow
   //if (element.tagName=='INPUT'&&element.neosysalign=='T')
   if (element.tagName=='INPUT'&&dictitem.align=='T')
   {

    //replace original element
    var newspan=document.createElement('SPAN')
    element.parentNode.replaceChild(newspan,element)

    //copy old attributes over to get tabIndex etc
    var oldHTML=element.outerHTML
    for (propname in element)
    {
     try
     {
      if (oldHTML.indexOf(' '+propname+'=')<0) continue
      if (element[propname]) newspan[propname]=element[propname]
     }
     catch(e){}
    }

    element=newspan
    
   }
   
   //copy the dictionary to the field
   //repeated below because cannot seem to merge expando attributes on mac!!!)
   copydictitem(dictitem,element)

   var elementtabindex=element.tabIndex
    
   //from here on there should be no need for references to the dictitem

   //build radio and check boxes
   if ((element.neosysradio&&element.type!='radio')
	   ||(element.neosyscheckbox&&element.type!='checkbox')
	  )
   {

    //radio has preference over checkbox
    if (element.neosysradio) element.neosyscheckbox=''
    
    //gdefault=getdefault(element)

    //build html for multiple inputs
    var options=(element.neosysradio?element.neosysradio:element.neosyscheckbox).neosyssplit(':;')
    var temp=''
    var elementtype=element.neosysradio?'radio':'checkbox'
    for (var i=0;i<options.length;i++)
    {

     //build an input item
     temp+='<INPUT type='+elementtype+' datafld='+element.dataFld
     if (element.neosysreadonly) temp+=' disabled=true'
     //temp+='<nobr><INPUT type='+elementtype+' datafld='+element.dataFld
     //must be done to group radio boxes
     temp+=' name='+element.dataFld
     if (typeof(options[i][0])!='undefined') temp+=' value='+options[i][0].toString().neosysquote()

     //set default but it has to be done again in cleardoc anyway
     //if (gdefault==options[i][0]) temp+=' checked=true'

     //temp+=' onfocus="return onfocus()">'
     temp+='>'

     //postfix the option title
     if (typeof(options[i][1])!='undefined') temp+='<nobr>'+options[i][1]+'</nobr>'

     //horizontal or vertical
     //if vertical then add <br> between options
     //but not after last option (so that next input can appear inline with the last option.
     //if (!element.neosyshradio) temp+='<BR>'
     //temp+=(element.neosyshorizontal?'&nbsp;':'<br>')
     if (element.neosyshorizontal) temp+='&nbsp;'
     else if (i<options.length-1) temp+='<br>'
     //temp+='</nobr>'+(element.neosyshorizontal?'&nbsp;':'<br>')
     
    }
    
    //replace original element
    var newspan=document.createElement('SPAN')
    newspan.innerHTML=temp
    //element.swapNode(temp)
    element.parentNode.replaceChild(newspan,element)
    
    //setup onfocus and onclick for all boxes
    for (var i=0;i<newspan.childNodes.length;i++)
    {
     element=newspan.childNodes[i]
     if (element.type==elementtype)
     {
      //element.onfocus=onfocus
      element.onclick=onclickradiocheckbox
      element.tabIndex=elementtabindex
     }

    }
    
    
    //having converted the element to a SPAN/array of INPUT elements ...
    element=newspan
    continue
    
   }

   if (element.neosysdropdown)
   {

    neosysassertobject(element.neosysdropdown,'formfunctions_onload','element.neosysdropdown')
    
    //convert element to a SELECT
    //var temp=document.createElement('SELECT '+element.outerHTML.slice(7)
    var temp=document.createElement('SELECT')
    copydictitem(dictitem,temp)

    //make sure tabindex is copied over
    temp.tabIndex=element.tabIndex
    
    //indicate that it is a single selection, one line drop down.
    temp.size=1
    
    //element.swapNode(t)
    element.parentNode.replaceChild(temp,element)

    request='CACHE\r'+element.neosysdropdown[0]
    colarray=element.neosysdropdown[1]

    neosyssetdropdown(temp,request,colarray)

    element=temp
//    element.innerHTML=element.innerHTML+' '
    
    element.tabIndex=elementtabindex
    
   }
   
   //convert to select element
   if (element.neosysconversion)
   {
   
    //conversion is a routine eg [NUMBER] [DATE]
    if (typeof(element.neosysconversion)=='string'&&element.neosysconversion.slice(0,1)=='[')
    {
    }
    
    //conversion is an array of options
    else
    {
    
     //convert element to a SELECT

// if (element.id=='USER_CODE') breakpoint('x '+element.id+' '+element.neosysrequired+' '+element.outerHTML)
     var temp=document.createElement('SELECT')
     copydictitem(dictitem,temp)
    
//if (element.id=='USER_CODE') breakpoint('y '+temp.id+' '+temp.neosysrequired+' '+temp.outerHTML)
     temp.size=1
     //element.swapNode(t)
     element.parentNode.replaceChild(temp,element)
     element=temp
     
     setdropdown3(element,element.neosysconversion)
     
     element.tabIndex=elementtabindex
     
     element.onchange=validateupdate
     
    }
   }

   //add some events
   //simple "element.onfocus=onfocus" does not work for data bound row elements
   if (element.tagName.match(gdatatagnames))
   {
  //  var temp='onfocus="return onfocus()"'
  //  temp=setattribute(element,temp)
  //  element=temp
    //element.onfocus=onfocus
//      element.setAttribute('onfocus','onfocus()')
   }
   
   //check for duplicate field numbers
   with (dictitem)
   {
    if (type=='F'&&!dictitem.allowduplicatefieldno)
    {
     
     var fieldandwordn=fieldno
     if (keypart) fieldandwordn+='.'+keypart
     if (dictitem.wordno) fieldandwordn+='.'+wordno

     //skip radio/checkbox elements with the same id
     if (gds.dict.fieldandwordns[fieldandwordn]!=element.id)
     {
     
      if (typeof gds.dict.fieldandwordns[fieldandwordn]!='undefined')
      {
       //throw(new Error(0,neosysquote(name)+' duplicate dictionary field and word no '+fieldandwordn+' is not allowed'))
       systemerror('formfunction_onload',neosysquote(name)+' duplicate dictionary name, field no, word no '+fieldandwordn+' is not allowed without .allowduplicatefieldno=true\r'+gds.dict.fieldandwordns[fieldandwordn])
      }
      
      //save a field pointer
      gds.dict.fieldandwordns[fieldandwordn]=element.id
      
     }
    
    }
   
   }
   
   //copy the dictionary to the field (repeated from above!)
   for (var propertyname in dictitem)
   {
//    eval('if(typeof(element.neosys'+propertyname+')=="undefined"&&typeof(dictitem.'+propertyname+')!="undefined") element.neosys'+propertyname+'=dictitem.'+propertyname)
    if (typeof(element['neosys'+propertyname])=='undefined'&&typeof(dictitem[propertyname])!='undefined') element['neosys'+propertyname]=dictitem[propertyname]
   }

   //the first element is considered to be the 'key' field
   //zzz should be field 0
   if (element.neosysfieldno=='0')
   {
    if (!gKeyNodes) gKeyNodes=[]
    gKeyNodes[gKeyNodes.length]=element
    if (gKeyNodes.length==1)
    {
     gKeyNodes[0].neosysprintfunction=element.neosysprintfunction
     gKeyNodes[0].neosyslistfunction=element.neosyslistfunction
     element.accessKey='K'
    }

   }

   //allow for data entry in SPAN elements
   if (element.neosystype=='F'&&element.tagName=='SPAN')
   {
    if (!element.neosysreadonly)
    {
     element.contentEditable='true'
     //element.className='clsNotRequired'
     //element.multiLine=true
     //if (!element.className) element.style.backgroundColor='white'
     element.style.width=element.neosyslength*7
    }
   }

   //add button before element with popups (and selects to make it clear to users that F7 is available - especially since useful when selecting multivalues)
   if ((element.tagName=='SELECT'||element.neosyspopup)&&!element.neosysreadonly&&(element.type=='text'||element.contentEditable=='true'||element.tagName=='SELECT'))
   //if (element.neosyspopup&&!element.neosysreadonly)
   {
    if (typeof element.neosyspopup=='string'||element.tagName=='SELECT')
    {
     //conversion is a routine eg [neosysfilepopup(filename,cols,coln,sortselect] [popup.clients]
     
     var element2=document.createElement('IMG')
     //add the button right before/after the field
     element2.id=element.id+'_popup'
     
//mac     element.insertAdjacentElement('afterEnd',element2)
     
     //var nobreak=document.createElement('NOBR')
     //element=element.parentNode.replaceChild(nobreak,element)
     //nobreak.insertBefore(element,null)
     //nobreak.insertBefore(element2,null)
     
     //put nowrap flag on TD in 3 levels
     //except on span fields because otherwise they dont text wrap
     if (element.tagName!='SPAN')
     {
      var td=element.parentNode
      if (td&&td.tagName!='TD') td=td.parentNode
      if (td&&td.tagName!='TD') td=td.parentNode
      if (td&&td.tagName=='TD') td.noWrap=true
     }
     
     
     //insertafter(element,element2)
     element.parentNode.insertBefore(element2,element)

     //element2.src='../neosys/images/search.gif'
//     element2.style.marginTop='3'
     element2.style.verticalAlign='top'
     //element2.style.border='solid 1px'
     if (element.dataFld.indexOf('DATE')>=0)
      element2.src=gcalendarimage
     else
      element2.src=gfindimage
     element2.title='Find a'+('aeioAEIO'.indexOf(element.neosystitle.slice(0,1))!=-1?'n':'')+' '+element.neosystitle
     element2.title+=' (F7)'
     element2.style.cursor='hand'
     
     var element2=setattribute(element2,'onclick="return neosysformpopup()"')
     //element2.setAttribute('onclick',element.neosyspopup)
     
    }
   }
   
   //add button before element for link (or after if right justified)
   if (element.neosyslink)
   {
    if (typeof(element.neosyslink)=='string')
    {
     //conversion is a routine eg [neosysfilepopup(filename,cols,coln,sortselect] [popup.clients]
     
     var element2=document.createElement('IMG')
     //add the button right after the field

     //put nowrap flag on TD in 3 levels
     var td=element.parentNode
     if (element.tagName!='SPAN')
     {
      if (td&&td.tagName!='TD') td=td.parentNode
      if (td&&td.tagName!='TD') td=td.parentNode
      if (td&&td.tagName=='TD') td.noWrap=true
     }
    
     if (element.neosysgroupno!=0&&element.neosysalign=='R')
     {
      element.parentNode.insertBefore(element2,null)
     }
     else
     {
      element.parentNode.insertBefore(element2,element)
     }
     
     //element2.src='../neosys/images/smallopen.gif'
     element2.src=glinkimage
     //element2.style.border='solid 1px'
     element2.style.verticalAlign='top'
     element2.title='Open this '+element.neosystitle+' (F6)'
     element2.isneosyslink=true
     element2.style.cursor='hand'
     
     var element2=setattribute(element2,'onclick="return neosyslink()"')

    }
   }
   
   //use the data field name as the id and name of the element
   //NB the name appears to be lost on databinding table rows
   element.id=element.dataFld
   
   //NAME attribute cannot be set at run time on elements dynamically
   // created with the createElement method
   //element.name=element.dataFld
   
   //group no
   //var groupno=parseInt(element.neosysgroupno,10)
   var groupno=element.neosysgroupno

   //function code
   if(typeof element.neosysfunctioncode=='string'&&element.neosysfunctioncode!='')
   {
    var tempfunction=null
//    try
//    {
     //cannot use neosysevaluate here unless perhaps the scope of tempfunction is global
     //neosysevaluate('new function tempfunction(){'+element.neosysfunctioncode+'}','formfunctions_onload() '+element.id)

     //eval('new function tempfunction(){'+element.neosysfunctioncode+'}')
     tempfunction=new Function(element.neosysfunctioncode)
//    }
//    catch(e){}
    //NB assign regardless of any error above otherwise fails with "return outside function" later on
    element.neosysfunctioncode=tempfunction
    
    //if (!tempfunction)
    //{
    // return logout('formfunctions_onload - error in '+element.id+'.functioncode')    
    //}
    
   }

   //align
   //right align parent TD/TH if in columns (groupn>0)
   if (!element.neosysalign&&typeof(element.neosysconversion)=='string'&&element.neosysconversion.indexOf('[NUMBER')>=0)
   {
    element.neosysalign='R'
   }
   //if (groupno>0&&element.neosysalign=='R'&&'THTD'.indexOf(element.parentNode.tagName)>=0)
   //if (element.neosysalign=='R'&&'THTD'.indexOf(element.parentNode.tagName)>=0)
   if (element.neosysalign=='R'&&(groupno>0||getancestor(element,'TFOOT')))
   {
    var cellnode=getancestor(element,' TD TH ')
    if (cellnode&&!cellnode.parentNode.align&&!cellnode.align) cellnode.align='right'
   }

   //length and maxlength
   if (element.tagName.match(gtexttagnames))
   {
    if (element.size!=1&&element.neosyslength)
    {
     if (!parseInt(element.neosyslength))
     {
      systemerror('formfunctions_onload()',element.id+'.neosyslength='+element.neosyslength+' is invalid. 10 used.')
      element.neosyslength=10
     }
     element.size=parseInt(element.neosyslength,10)
     if (element.size>2) element.size+=2
    }
    if (element.tagName=='TEXTAREA')
    {
     element.cols=element.size?element.size:30
     if (element.neosysrows) element.rows=element.neosysrows
    }
    if (element.neosysmaxlength) element.maxLength=parseInt(element.neosysmaxlength,10)
   }
   
   //lower case
   if (typeof(element.neosyslowercase)=='undefined')
   {
    if(element.tagName=='SELECT'||element.neosysalign=='T'||element.neosystype=='S')
    {
     element.neosyslowercase=true
    }
    else
    {
     element.neosyslowercase=''// not 'false' as user properties must be strings not boolean for row bound elements
    }
   }
   //force visual uppercase (actual conversion done in onbeforeupdate)
   if (!element.neosyslowercase&&element.type!='radio'&&element.type!='checkbox')
    element.style.textTransform='uppercase'
    
   //non calculated fields may be displayed as/converted to uppercase
   //if (element.neosystype=='F'&&element.tagName!='SPAN')
   if (element.neosystype=='F')
   {
   
    //as of ie5 we can only focus on elements which have tabindex
    //make them all the same and tab will work nicely
    //tabindex can also be hard coded in the form design
    //use <999 to come before defaults and >999 to come after
    if (!element.tabIndex)
    {
     if (element.neosysreadonly)
     {
      element.tabIndex=-1
     }
     else
     {
      element.tabIndex=999
     }
    }

    setfirstlastelement(element)

   }
   
   //calculated fields can display HTML
   else
   {
    try
    {
     element.dataFormatAs='HTML'
    }
    catch (e)
    {
     systemerror('formfunctions_onload()',element.dataFld+' should not be bound to an editable element '+element.tagName)
    }
   }
   
   //clear any existing values (f5 refresh does not seem to clear databinding)
   /////but now we bind an empty record so this is not necessary
  // if (element.tagName.match(gdatatagnames))
  // {
  //  setvalue(element,'')
  // }

   //highlight required fields
   //if (!element.className&&element.type!='radio'&&element.type!='checkbox')
   if (!element.className&&element.tagName.match(gdatatagnames)&&element.type!='radio'&&element.type!='checkbox')
   {
    //spans are only input if .contentEditable
    if (element.neosystype=='F'&&!element.neosysreadonly&&(element.tagName!='SPAN'||element.contentEditable=='true'))
    {
     element.className='clsNotRequired'
    }
    else
    {
     element.className='clsReadOnly'
    }
    if (element.neosysrequired&&!element.neosysdefaultvalue&&element.type!='radio'&&element.type!='checkbox')
    {
     if (element.tagName!='SELECT'||(element.tagName=='SELECT'&&element.options[element.selectedIndex].value==''))
     {
      if (element.neosysgroupno==0||element.neosysrowrequired)
      {
       element.className='clsRequired'
//      element.style.border='solid 1px'
//      element.style.backgroundColor='F0F0F0'
      }
     }
    }
   }
    
   //handle groups
  
   //build a list of datafld elements
   var screenfieldno=gfields.length
   gfields[screenfieldno]=element
   //element.neosysscreenfieldno=gfields.length-1
   //why -1 ???
   element.neosysscreenfieldno=screenfieldno
   //gfields[screenfieldno].neosysscreenfieldno=screenfieldno
   
   var rowx=getancestor(element,'TR')
   var tablex=getancestor(rowx,'TABLE')
   
   //set the datasrc of single valued elements
   if (groupno==0)
   {
   
    //but allow it in THEAD/TFOOT elements!
    //prevent group 0 element in rows of a multivalued table
    //while (tablex)
    //{
    // if (tablex.neosysgroupno) systemerror('formfunction_onload()',neosysquote(element.dataFld)+' is not multivalued and cannot be in a multivalued TABLE')
    // tablex=getancestor(tablex,'TABLE')
    //}

    //prevent group 0 element in rows of a multivalued table
    if (rowx&&rowx.neosysgroupno)systemerror('formfunction_onload()',neosysquote(element.dataFld)+' is not multivalued and cannot be in a multivalued TABLE')
    
    datasrcelements[datasrcelements.length]=element

   }
   
   //set the data source of repeating lines
   else
   {

    if (groupno>gmaxgroupno) gmaxgroupno=groupno

    //locate the table element in the parents
    if (tablex==null)
    {
     systemerror('formfunction_onload()',neosysquote(element.dataFld)+' is multivalued and must exist inside a TABLE element')
    }
    else
    {

      //add sorting button to column title
      var titleelement
      if (1&&(titleelement=document.getElementById(element.id+'_title')))
      {

       var element2=document.createElement('IMG')
       //add the button right after the field

       //put nowrap flag on TD in 3 levels
       //var td=element.parentNode
       //if (element.tagName!='SPAN')
       //{
       // if (td&&td.tagName!='TD') td=td.parentNode
       // if (td&&td.tagName!='TD') td=td.parentNode
       // if (td&&td.tagName=='TD') td.noWrap=true
       //}
    
       titleelement.insertBefore(element2,null)
       
       element2.id='sortbutton_'+element.neosysgroupno
       //element2.name=element2.id
       //element2.src='../neosys/images/smallsort.gif'
       element2.src=gsortimage
       element2.originalsrc=element2.src
       element2.title='Sort by '+element.neosystitle
       element2=setattribute(element2,'onclick="return sorttable(\''+element.id+'\')"')
       gsortimages[element.id]=element2

      }
      
      //any element can determine if emptyrows are allowed
      if (element.neosysallowemptyrows) tablex.neosysallowemptyrows=element.neosysallowemptyrows
      
      //any element can determine if a row is required
      if (element.neosysrowrequired) tablex.neosysrowrequired=element.neosysrowrequired
      
     //need to know the last data entry column
     if (!element.neosysreadonly&&element.tagName.match(gdatatagnames))
     {
      //spans are only input if .contentEditable
      if (element.tagName!='SPAN'||element.contentEditable=='true')
      {
       tablex.neosyslastinputcolumnscreenfieldno=element.neosysscreenfieldno
      }
     }
      
     //need to know the first data entry column
     if (!tablex.neosysfirstinputcolumnscreenfieldno&&!element.neosysreadonly&&element.tagName.match(gdatatagnames))
     {
      //spans are only input if .contentEditable
      if (element.tagName!='SPAN'||element.contentEditable=='true')
      {
       tablex.neosysfirstinputcolumnscreenfieldno=element.neosysscreenfieldno
       element.neosysisfirstinputcolumn=true
      }
     }
      
     //prevent use of separator characters unless multiword
     if (element.tagName!='SELECT')
     {
      if (element.neosyswordsep&&element.neosysnwords<=1)
      {
       if (typeof element.neosysinvalidcharacters=='undefined') element.neosysinvalidcharacters=''
       element.neosysinvalidcharacters+=element.neosyswordsep
      }
     }
     
     if (typeof tablex.neosysgroupno!='undefined')
     {

      //check that all fields in same table have same group no
      //zzz should also check that the table has no other datafld in subtables
      if (tablex.neosysgroupno!=groupno)
      {
       systemerror('formfunctions_onload()',element.dataFld+' is in group '+groupno+' but the table has already been bound to group '+tablex.neosysgroupno)
       return
      }
     
      //tablex.ncols++
      //element.coln=tablex.ncols
      gtables[groupno][gtables[groupno].length]=screenfieldno
      
      //hide insertrow buttons (in case first element does not have the flag)
      if (element.neosysnoinsertrow&!tablex.noinsertrow)
      {
       tablex.noinsertrow=true
       var temp=document.getElementById('insertrowbutton'+groupno)
       if (temp) temp.style.display='none'
      }
      
      //hide deleterow buttons (in case first element does not have the flag)
      if (element.neosysnodeleterow&!tablex.nodeleterow)
      {
       tablex.nodeleterow=true
       var temp=document.getElementById('deleterowbutton'+groupno)
       if (temp) temp.style.display='none'
      }

     }
     else
     {

      //first column is required
      //this should perhaps not be set since we have rowrequired and allowemptyrows
      //element.neosysrequired=true

      //check this groupno not used on other tables
      if (gtables[groupno])
      {
       systemerror('formfunctions_onload()',element.dataFld+' is in group '+groupno+' but that group is also used in another table by '+gfields[gtables[groupno][0]].id)
       return
      }
             
      //tablex.ncols=0
      gtables[groupno]=[]
      gtables[groupno][0]=screenfieldno
      gtables[groupno].tableelement=tablex
      tablex.neosysgroupno=groupno
      tablex.dataFld='group'+groupno
      tablex.id='neosysgroup'+groupno
      datasrcelements[datasrcelements.length]=tablex
      tablex.neosysdependents=''
      if (element.neosysnoinsertrow) tablex.noinsertrow=true
      if (element.neosysnodeleterow) tablex.nodeleterow=true
//      if (!tablex.className) tablex.className='NEOSYSTABLE'
//      tablex.border=1
      //capture all double clicks for potential filtering
      tablex.ondblclick=form_ondblclick
      //tablex.oncontextmenu=form_onrightclick

      tablex.onreadystatechange=tablex_onreadystatechange

      //mark group in row to allow check/prevent group0 in same row
      rowx.neosysgroupno=groupno
      
      //add insert and delete row buttons at the first column in the tbody
      
      //button shortcut keys are ctrl+ on mac and alt+ on pc
      var t2
      if (ismac)
      {
       t2='x'
       t3='x'
      }
      else
      {
       t2='(Ctrl+N)'
       t3='(Ctrl+D)'
      }
      var t=''
      t+=' <nobr>'
      if (!element.neosysnoinsertrow) t+='  <IMG id=insertrowbutton'+groupno+' alt="Insert a new row here '+t2+'" xstyle="margin-top:3" Xwidth=13 Xheight=13 id=insertrowbutton onclick="return insertrow()" src="'+ginsertrowimage+'" style="CURSOR: hand;border">'//: solid 1px">'
      if (!element.neosysnodeleterow) t+='  <IMG id=deleterowbutton'+groupno+' alt="Delete this row '+t3+'" xstyle="margin-top:3" Xwidth=13 Xheight=13 id=deleterowbutton onclick="return deleterow()" src ="'+gdeleterowimage+'" style="CURSOR: hand;border">'//: solid 1px">'
      t+=' </nobr>'
      var insertdeletebuttons=document.createElement('TD')
      insertdeletebuttons.innerHTML=t
 
      //locate the TR element in the parents
      var trx=getancestor(element,'TR')
      if (trx==null)
      {
       systemerror('formfunction_onload()',neosysquote(element.dataFld)+' is multivalued and must exist inside a TABLE element')
      }
      else
      {
       trx.insertBefore(insertdeletebuttons,trx.firstChild)
      }

      //add page up/down buttons at the first column in the thead and tfoot
      var pgupdownbuttons=document.createElement('TH')
      pgupdownbuttons.width='1%'
      
      var t='&nbsp;'
      if (tablex.dataPageSize)
      {
       t='<span id="currentpageno"></span>'
       t+='<nobr>'
       t+=' <button class="neosysbutton" id="firstpage" onclick="return changepage(0)" title="First page (Ctrl+PgUp)" '
       t+=' >&lt;&lt;</button>'
       t+=' <button class="neosysbutton" id="previouspage" onclick="return changepage(-1)" title="Previous page (PgUp)" '
       t+=' >&lt;</button>'
       t+=' <button class="neosysbutton" id="nextpage" onclick="return changepage(1)" title="Next page (PgDn)" '
       t+=' >&gt;</button>'
       t+='</nobr>'
       t+=' <button class="neosysbutton" id="lastpage" onclick="return changepage(9999)" title="Last page (Ctrl+PgDn)" '
       t+=' >&gt;&gt;</button>'
       t+='</nobr>'
      }
      
      t+='<button id=neosysgroup'+groupno+'showall'
      t+=' style=display:none onclick="form_filter(\'unfilter\','+groupno+')"'
      t+='>Show All</button>'
      
      pgupdownbuttons.innerHTML=t
      
      //locate the THEAD element in the parents
      var thx=tablex.getElementsByTagName('THEAD')[0]
      if (!thx)
      {
//taken out 30/6/2001 to allow simpler forms eg Trial Balance Printout
//       alert(element.dataFld+' Multivalued column TABLE must have a THEAD element')
      }
      else
      {

       //allow for multiple rows in thead
       //mac needs to be updated AFTER insertion of element
       //pgupdownbuttons.rowSpan=thx.children.length
       if (!thx.firstChild)
        systemerror('Missing table structure for multivalued field?')
       else
        thx.firstChild.insertBefore(pgupdownbuttons,thx.firstChild.firstChild).rowSpan=thx.children.length
      }
      
      //locate the TFOOT element in the parents
      var thx=tablex.getElementsByTagName('TFOOT')[0]
      if (!thx)
      {
      }
      else
      {
       pgupdownbuttons=pgupdownbuttons.cloneNode(true)
       
       //allow for multiple rows in thead
       pgupdownbuttons.rowSpan=thx.children.length
       thx.firstChild.insertBefore(pgupdownbuttons,thx.firstChild.firstChild)
        
      }
      
     }
    }
   }
  }//datafld element
 }

 //zero all the dependents
 for (var i=0;i<gfields.length;i++) gfields[i].neosysdependents=''

 //for each calculated field add its field number
 // to all elements whose name (in double or single quotes)
 // appear in its functioncode
 for (var fieldn=0;fieldn<gfields.length;fieldn++)
 {
  with (gfields[fieldn])
  {
   var deplist=''
//   alert(gfields[fieldn].outerHTML)
   if (neosystype=='S')
   {
    var functioncode=neosysfunctioncode.toString()
    for (var fieldn2=0;fieldn2<gfields.length;fieldn2++)
    {
     var field2=gfields[fieldn2]
     if (functioncode.indexOf('"'+field2.neosysname+'"',0)>=0||functioncode.indexOf("'"+field2.neosysname+"'",0)>=0)
     {
      if (field2.neosysdependents!='') field2.neosysdependents+=';'
      field2.neosysdependents+=fieldn
      if (field2.neosysgroupno>0&&neosysgroupno!=field2.neosysgroupno)
      {
       var tablex=getancestor(field2,'TABLE')
       if (tablex.neosysdependents!='') tablex.neosysdependents+=';'
       tablex.neosysdependents+=fieldn
      }
     }
    }
   }
  }
 }

 //get copies of the gfields in case they are deleted from the document (eg table row 1 is deleted)
 for (var fieldn=0;fieldn<gfields.length;fieldn++)
 {
  var datafld=gfields[fieldn].dataFld
  gfields[fieldn]=gfields[fieldn].cloneNode()
  if (ismac) gfields[fieldn].dataFld=datafld //because mac ie5 loses datafld when cloned
 }

 if (!gfirstnonkeyelement) gfirstnonkeyelement=gfields[0]
 if (!gfirstnonkeyelement)
 {
  systemerror('formfunctions_onload()','There are no datafld elements in the form')
  return
 }
 
 //do not do the following otherwise if a grouped checkbox is the firstnokeyelement
 //then there is a failure in getvalue because it cannot find the parentNode
 //datafld=gfirstnonkeyelement.dataFld
 //gfirstnonkeyelement=gfirstnonkeyelement.cloneNode()
 //gfirstnonkeyelement.dataFld=datafld
   
 //when user enters the key, get a record
 
 //add the open, save, close and delete buttons
 
 //button shortcut keys are ctrl+ on mac and alt+ on pc
 var t2
 if (ismac) t2='Ctrl'
 else t2='Alt'
 
 //tabindex buttons at 9999 to come after other fields at 999 
 var t=''
 t+=' <nobr>'
 //t+='    <BUTTON title="Create a new document. '+t2+'+N" accesskey="N" id=newrecord onclick="return newrecord_onclick()" class=neosysbutton tabindex=9999>New</BUTTON>'
 //t+='    <BUTTON title="Open an existing document. '+t2+'+O" accesskey="O" id=openrecord onclick="return openrecord_onclick()" class=neosysbutton tabindex=9999>Open</BUTTON>'
 
 //t+='    <BUTTON title="Save the current document. '+t2+'+S, Ctrl+Enter or F9" accesskey="S" disabled=true id=saverecord onclick="return saverecord_onclick()" class=neosysbutton tabindex=9999>Save</BUTTON>'
 //t+='    <BUTTON title="Close the current document. '+t2+'+C, Esc or F8" accesskey="C" disabled=true id=closerecord onclick="return closerecord_onclick()" class=neosysbutton tabindex=9999>Close</BUTTON>'
 //t+='    <BUTTON title="Release/Reopen the current document. '+t2+'+R," accesskey="R" disabled=true id=releasereopenrecord onclick="return releasereopenrecord_onclick()" class=neosysbutton tabindex=9999>Release</BUTTON>'
 //t+='    <BUTTON title="Delete the current document. '+t2+'+D" accesskey="D" disabled=true id=deleterecord onclick="return deleterecord_onclick()" class=neosysbutton tabindex=9999>Delete</BUTTON>'

 function addbutton(t,button)
 {
  return t+button
  //add on end if floating to the right
  //return button+t
 }
 
 if (gKeyNodes)
 {
  t=addbutton(t,menubuttonhtml('newrecord',gnewimage,'<u>N</u>ew','Create a new document. '+t2+'+N','N','newrecord_onclick()'))
  t=addbutton(t,menubuttonhtml('openrecord',gopenimage,'<u>O</u>pen','Open an existing document. '+t2+'+O','O','openrecord_onclick()'))
  t=addbutton(t,menubuttonhtml('releasereopenrecord',greleaseimage,'<u>R</u>elease','Release/Reopen the current document. '+t2+'+R','R','releasereopenrecord_onclick()'))
 }
 else
 {
  newrecord=''
  openrecord=''
  releasereopenrecord=''
 }
 t=addbutton(t,menubuttonhtml('saverecord',gsaveimage,'<u>S</u>ave','Save the current document. '+t2+'+S, Ctrl+Enter or F9','S','saverecord_onclick()'))
 t=addbutton(t,menubuttonhtml('closerecord',gcloseimage,'<u>C</u>lose','Close the current document. '+t2+'+C','C','closerecord_onclick()'))
 if (gKeyNodes&&(typeof gpreventdeletion=='undefined'||gpreventdeletion!=false))
  t=addbutton(t,menubuttonhtml('deleterecord',gdeleteimage,'<u>D</u>elete','Delete the current document. '+t2+'+D','D','deleterecord_onclick()'))
 else
  deleterecord=''
 //t+='    <IMG src="'+gnewimage+'" title="Create a new document. '+t2+'+N" accesskey="N" id=newrecord onclick="return newrecord_onclick()" class=neosysbutton tabindex=9999></IMG>'
 //t+='    <IMG src="'+gopenimage+'" title="Open an existing document. '+t2+'+O" accesskey="O" id=openrecord onclick="return openrecord_onclick()" class=neosysbutton tabindex=9999></IMG>'
 //t+='    <IMG src="'+gsaveimage+'" title="Save the current document. '+t2+'+S, Ctrl+Enter or F9" accesskey="S" disabled=true id=saverecord onclick="return saverecord_onclick()" class=neosysbutton tabindex=9999></IMG>'
 //t+='    <IMG src="'+gcloseimage+'" title="Close the current document. '+t2+'+C, Esc or F8" accesskey="C" disabled=true id=closerecord onclick="return closerecord_onclick()" class=neosysbutton tabindex=9999></IMG>'
 //t+='    <IMG src="'+greleaseimage+'" title="Release/Reopen the current document. '+t2+'+R," accesskey="R" disabled=true id=releasereopenrecord onclick="return releasereopenrecord_onclick()" class=neosysbutton tabindex=9999></IMG>'
 //t+='    <IMG src="'+gdeleteimage+'" title="Delete the current document. '+t2+'+D" accesskey="D" disabled=true id=deleterecord onclick="return deleterecord_onclick()" class=neosysbutton tabindex=9999></IMG>'

 if (gKeyNodes&&gKeyNodes[0].neosysprintfunction)
 {
  //t+='    <BUTTON title="Print/Send the current document. Alt+P" accesskey="P" disabled=true id=printsendrecord onclick="return printsendrecord_onclick()" class=neosysbutton tabindex=9999>Print/Send</BUTTON>'
  //t+='    <IMG src="'+gprintsendimage+'" title="Print/Send the current document. Alt+P" Xaccesskey="P" disabled=true id=printsendrecord onclick="return printsendrecord_onclick()" class=neosysbutton tabindex=9999></IMG>'
  t=addbutton(t,menubuttonhtml('printsendrecord',gprintsendimage,'<u>P</u>rint/Send','Print/Send the current document. '+t2+'+P','P','printsendrecord_onclick()'))
 }
 else
 {
  printsendrecord=false
 }
 if (gKeyNodes&&gKeyNodes[0].neosyslistfunction)
 {
  //t+='    <BUTTON title="List the current file. Alt+I" accesskey="I" id=listrecord onclick="return listrecord_onclick()" class=neosysbutton tabindex=9999>List</BUTTON>'
  //t+='    <IMG src="'+glistimage+'" title="List the current file. Alt+I" Xaccesskey="I" id=listrecord onclick="return listrecord_onclick()" class=neosysbutton tabindex=9999></IMG>'
  t=addbutton(t,menubuttonhtml('listrecord',glistimage,'L<u>i</u>st','List the current file. '+t2+'+I','I','listrecord_onclick()'))
 }
 else
 {
  listrecord=false
 }
 
 //first, previous, next and last record buttons
 /*
 t=addbutton(t,'<BUTTON title="Open first document. '+t2+'+{" accesskey="{" id=firstrecord onclick="return nextrecord_onclick(\'first\')" class=neosysbutton tabindex=9999 style="display:none;background-image:url('+gfirstimage+')">&lt;&lt;</BUTTON>')
 t=addbutton(t,'<BUTTON title="Open previous document. '+t2+'+[" accesskey="[" id=previousrecord onclick="return nextrecord_onclick(-1)" class=neosysbutton tabindex=9999 style=display:none>&lt;</BUTTON>')
 t=addbutton(t,'<BUTTON title="Select document. '+t2+'+^" accesskey="^" id=selectrecord onclick="return nextrecord_onclick(0)" class=neosysbutton tabindex=9999 style=display:none>^</BUTTON>')
 t=addbutton(t,'<BUTTON title="Open next document. '+t2+'+]" accesskey="]" id=nextrecord onclick="return nextrecord_onclick()" class=neosysbutton tabindex=9999 style=display:none>&gt;</BUTTON>')
 t=addbutton(t,'<BUTTON title="Open last document. '+t2+'+}" accesskey="}" id=lastrecord onclick="return nextrecord_onclick(\'last\')" class=neosysbutton tabindex=9999 style=display:none>&gt;&gt;</BUTTON>')
 */
 
 t=addbutton(t,menubuttonhtml('firstrecord',gfirstimage,' ','Open the first document. '+t2+'+{','{','nextrecord_onclick()'))
 t=addbutton(t,menubuttonhtml('previousrecord',gpreviousimage,' ','Open the previous document. '+t2+'+[','[','nextrecord_onclick(-1)'))
 t=addbutton(t,menubuttonhtml('selectrecord',gspacerimage,' ','Select document. '+t2+'+^','^','nextrecord_onclick(0)'))
 t=addbutton(t,menubuttonhtml('nextrecord',gnextimage,' ','Open the next document. '+t2+'+]',']','nextrecord_onclick()'))
 t=addbutton(t,menubuttonhtml('lastrecord',glastimage,' ','Open the last document. '+t2+'+}','}','nextrecord_onclick(\'last\')'))
 
 //login status
 var loginhtml=''
 //loginhtml+='&nbsp;&nbsp;&nbsp;'+gusername+ ' - '+gdatasetname+' - '+gdataset 
 loginhtml+='&nbsp;&nbsp;&nbsp;'+gdatasetname+' - '+gdataset+' - '+gusername
 if (gKeyNodes) t+=loginhtml
 
 t+=' </nobr>'

 //create the button rank and insert it into the form
 var formbuttons=document.createElement(gKeyNodes?'SPAN':'DIV')
 formbuttons.innerHTML=t
 if (gKeyNodes)
 {
  document.body.insertBefore(formbuttons,document.body.firstChild)
 }
 else
 {
  
  //login status
  var temp=document.createElement('DIV')
  temp.innerHTML=loginhtml
  temp.align='CENTER'
  document.body.insertBefore(temp,document.body.firstChild)
  
  formbuttons.align='CENTER'
  formbuttons.style.marginTop='5'
  document.body.insertBefore(formbuttons,null)
  
 }
 formbuttons.id='formbuttonsdiv'

 //program the various buttons to be visible when enabled
 saverecord.style.setExpression('display','saverecord.disabled?"none":"inline"')
 closerecord.style.setExpression('display','closerecord.disabled?"none":"inline"')
 if (printsendrecord) printsendrecord.style.setExpression('display','printsendrecord.disabled?"none":"inline"')
 
 if (gKeyNodes)
 {
  newrecord.style.setExpression('display','newrecord.disabled?"none":"inline"')
  releasereopenrecord.style.setExpression('display','releasereopenrecord.disabled?"none":"inline"')
  if (deleterecord) deleterecord.style.setExpression('display','deleterecord.disabled?"none":"inline"')
 }
 
 //program the first, previous, next and last record buttons to be visible when required
 if (firstrecord&&firstrecord.style.setExpression)
 {
  var tt='gkeys.length>1?"inline":"none"'
  firstrecord.style.setExpression('display',tt)
  previousrecord.style.setExpression('display',tt)
  selectrecord.style.setExpression('display',tt)
  nextrecord.style.setExpression('display',tt)
  lastrecord.style.setExpression('display',tt)
  
  firstrecord.setExpression('disabled','gkeyn==0?true:false')
  //previousrecord.setExpression('enabled','gkeyn>1?true:false')
  selectrecord_label.setExpression('innerText','(gkeyn+1)+" of "+gkeys.length')
  //nextrecord.setExpression('enabled','gkeyn.length>1?true:false')
  lastrecord.setExpression('disabled','(gkeyn==(gkeys.length-1))?true:false')
 }
 
 if (greadonlymode||gupdateonlymode||gpreventcreation)
 {
  newrecord.disabled=true
  newrecord.title='Creating new records is not allowed'
 }
 if (greadonlymode||gupdateonlymode||gpreventdeletion)
 {
  deleterecord.disabled=true
  deleterecord.title='Deleting records is not allowed'
 }
 
 //newrecord.disabled=false
 //newrecord.disabled=true
 //newrecord.style.display='none'
 
/*
 //openrecord.disabled=true
 if (gKeyNodes&&gKeyNodes.length==1&&gKeyNodes[0].neosyspopup)
 {
  openrecord.neosyspopup=gKeyNodes[0].neosyspopup
  openrecord.style.display='inline'
 }
 else
 {
  openrecord.style.display='none'
 }
*/
 var openfunction=''
 var popupfunction=''
 if (gKeyNodes)
 {
  var nvisiblekeys=0
  for (var keyn=0;keyn<gKeyNodes.length;keyn++)
  {
   //find the first openfunction (visible or not)
   if (openfunction=gKeyNodes[keyn].neosysopenfunction) break
   if (neosysenabledandvisible(gKeyNodes[keyn]))
   {
    //count the number of visible keys
    nvisiblekeys++
    //remember the first visible popupfunction
    if (!popupfunction) popupfunction=gKeyNodes[keyn].neosyspopup
   }
  }
  //if no openfunction and only one visible key with a popup function
  //then use the popup function as the open function
  if (!openfunction&&nvisiblekeys==1&&popupfunction) openfunction=popupfunction
 }
 //hide the open button if no openfunction
 if (openfunction)
 {
  openrecord.neosyspopup=openfunction
  openrecord.style.display='inline'
 }
 else
 {
  if (openrecord) openrecord.style.display='none'
 }
 
 //remove record orientated buttons if no key fields
 if (!gKeyNodes)
 {

  saverecord.tabIndex=9999-1//before menu and logout  
  closerecord.tabIndex=9999-1
  
  /*
  newrecord.style.display='none'
  openrecord.style.display='none'
  deleterecord.style.display='none'
  releasereopenrecord.style.display='none'
  //firstrecord.style.display='none'
  //nextrecord.style.display='none'
  //previousrecord.style.display='none'
  //lastrecord.style.display='none'
  */
  
  //saverecord.value='OK'
  setgraphicbutton(saverecord,'OK')
  saverecord.title='OK - Continue. '+(gfields.length>1?'Ctrl+':'')+'Enter or F9'
  saverecord.disabled=false
  saverecord.style.marginRight=5
  
  //change order of buttons ?
  //saverecord.swapNode(closerecord)
  //following does not work so comment out
  //var tempnode=saverecord.parentNode.replaceChild(closerecord,saverecord)
  //closerecord.parentNode.replaceChild(tempnode,closerecord)
  
  //no cancel button on main non-database forms
  //if (!gparameters.cancel&&typeof(form_write)=='function'||!window.dialogArguments)
  if (!gparameters.cancel&&!window.dialogArguments)
  {
   closerecord.style.display='none'
   closerecord.disabled=true
  }
  else
  {
   //closerecord.value='Cancel'
   setgraphicbutton(closerecord,'Cancel')
   closerecord.title='Cancel and exit. Esc'
   closerecord.disabled=false
  }
  
 }
 
 var temp=document.createElement('DIV')
 temp.innerHTML='<P ALIGN=CENTER STYLE="MARGIN-TOP: 5px"><SMALL>Software by <A tabindex=-1 HREF="http://www.neosys.com" tabindex=999999>NEOSYS.COM</A></SMALL></P>'
 document.body.insertBefore(temp,null)
 
 //start focused on the first key field
 if (gKeyNodes)
  gstartelement=gKeyNodes[0]
 else
  gstartelement=gfirstnonkeyelement
  
 //many routines will exit immeditately if this is not set (to avoid further error messages)
 //moved down now ... after any form_init
 //ginitok=true
 //wstatus('')
 
 //set the document titles from the parameters
 if (gparameters.documenttitle)
 {
  document.title=gparameters.documenttitle
  gdoctitle=document.title
 }
 
 //if form has a custom postinit routine
 if (typeof form_postinit=='function')
 {
  login('form_postinit before')
  neosysevaluate('form_postinit()','form_functions()')
  logout('form_postinit after')
  
  //reverse the effect of any setvalue commands in postinit
  gchangesmade=false
  
 }

 //set the title hx element
 gdoctitle=document.title
 var temp=document.getElementsByName('documenttitle')
 if (temp.length)
 {
  temp[0].innerHTML=document.title
 }
 
 //get the record from the page URL if any
/* 
 var key=window.location.toString().split('?')[1]
 if (key=='vid:1+1') key='' //'only in development
 if (key) key=unescape(key)
*/

 //many routines will exit immeditately if this is not set (to avoid further error messages)
 ginitok=true
 wstatus('')
 
 //record based forms
 if (gKeyNodes)
 {
  if (gparameters.key)
  {

   //allow multiple keys to be loaded
   if (typeof gparameters.key=='object')
   {
    gkeys=gparameters.key
    gparameters.key=gparameters.key[0]
   }
     
   //necessary to initialise gds else any expressions relying on gds.data will
   //crash after this routine returns
   cleardoc()
   
   window.setTimeout('opendoc('+neosysquote(gparameters.key.replace(/\\/g,'\\\\'))+')',10)
  }
  else
  {
   cleardoc()
  }
 }
 
 //non-record based forms can get parameters from URL or dialog arguments
 else
 {

  //initial form can come from a revstr
  if (gparameters.defaultrevstr) gro.defaultrevstr=gparameters.defaultrevstr

  cleardoc()
  
  validateall('filldefaults')
  
  grecn=null
  calcfields()
  updatedisplay()
  
 }

 if (gparameters.savebuttonvalue)
 {
  //saverecord.value=gparameters.savebuttonvalue
  setgraphicbutton(saverecord,gparameters.savebuttonvalue)
 }
 
 logout('formfunctions_onload')

}

function setfirstlastelement(element)
{
 
 //discover first non key input element
 if (element.neosysfieldno>0&&!element.neosysreadonly)
 {
  if (!gfirstnonkeyelement) gfirstnonkeyelement=element
  if (element.tabIndex<gfirstnonkeyelement.tabIndex) gfirstnonkeyelement=element
 }
    
 //discover last input element
 if (element.neosysfieldno>0&&!element.neosysreadonly)
 {
  if (!gfinalinputelement) gfinalinputelement=element
  if (element.tabIndex>=gfinalinputelement.tabIndex) gfinalinputelement=element
 }
 
 return
}
   
function setfirstlastcolumn(groupno)
{

 var tablex=document.getElementById('neosysgroup'+groupno)
 if (!tablex) neosysinvalid('setfirstlastcolumn() table'+groupno+' is missing')
 
 for (var i=0;i<gtables[groupno].length;i++)
 {
  var screenfieldno=gtables[groupno][i]
  //if (!gfields[screenfieldno].neosysreadonly&&neosysenabledandvisible(gfields[screenfieldno].id)) tablex.neosyslastinputcolumnscreenfieldno=screenfieldno-1
  if (!gfields[screenfieldno].neosysreadonly&&neosysenabledandvisible(gfields[screenfieldno].id)) tablex.neosyslastinputcolumnscreenfieldno=screenfieldno
 }
 
}

function systemerror(functionname,errormsg)
{
 if (typeof functionname=='undefined') functionname='undefined'
 if (typeof errormsg=='undefined') errormsg=''
 alert('System Error in '+functionname+'\r\r'+errormsg)
 //if (gstepping||(!ginitok&&gusername=='NEOSYS')) crashhere2
 if (gstepping||(gusername=='NEOSYS')) neosysbreak
}
 
function copydictitem(dictitem,element)
{
 
 for (var propertyname in dictitem)
 {
//    eval('if(typeof(element.neosys'+propertyname+')=="undefined"&&typeof(dictitem.'+propertyname+')!="undefined") element.neosys'+propertyname+'=dictitem.'+propertyname)
  if (typeof(element['neosys'+propertyname])=='undefined'&&typeof(dictitem[propertyname])!='undefined') element['neosys'+propertyname]=dictitem[propertyname]
//  breakpoint(propertyname)
 }
 
 element.dataFld=dictitem.name

 //will be setup later
 //element.id=dictitem.name
 ////element.name=dictitem.name
 
 //key elements default to required
 //(no longer as the next seq key can be provided by the database)
 //if (element.neosysfieldno=='0'&&(typeof(element.neosysrequired)=='undefined'||element.neosysrequired=='false'))
 //{
 // //element.neosysrequired='true'
 // element.neosysrequired=true
 //}
 
 if (!element.neosysrequired||element.neosysrequired=='false') element.neosysrequired=''
 
}

function updatedisplay(elements)
{

 //seems to be only implemented for non-bound forms
 
 //be careful not to update elements as it may be a global object
 
 //option to do all elements recursively
 if (typeof elements=='undefined') return updatedisplay(gfields)

 //do multiple elements recursively
 if (elements.length)
 {
  for (var i=0;i<elements.length;i++) updatedisplay(elements[i])
 }
 var element=elements
 
 //only do elements with a conditional display expression
 if (typeof element.neosysdisplay=='undefined') return
 
 login('updatedisplay '+element.id)

 //determine required display
 var display=gds.evaluate(element.neosysdisplay)
 if (typeof display=='undefined')
 {
  systemerror('updatedisplay('+element.id+')','.display() returned undefined')
 }
 display=display?'inline':'none'
 
 //quit if display not changed
 if (element.style.display==display) return logout('updatedisplay '+element.id+' still '+display)

 //hide all elements with the same id
 var subelements=document.getElementsByName(element.id)
 for (var subn=0;subn<subelements.length;subn++)
 {
 
  var subelement=subelements[subn]
  
  //radio elements are surrounded by DIV
  if ((subelement.type=='radio'||subelement.type=='checkbox')&&subelement.parentNode.tagName=='SPAN') subelement=subelement.parentNode
  
  //hide parent node if td or th with only one child
  var parent=subelement.parentNode
  if (parent.tagName.match(/^(TD)|(TH)$/))
  {
   //IE5.5 why does a TD enclosing a DIV have two childnodes the second being blank???
   var nchildnodes=0
   for (var i=0;i<parent.childNodes.length;i++) if (parent.childNodes[i].tagName) nchildnodes++
   if (nchildnodes==1) subelement=parent
  }
  
  subelement.style.display=display
  
 }
 
 //hide associated elements (same id plus '_title')
 var elements=document.getElementsByName(element.id+'_title')
 for (var i=0;i<elements.length;i++) elements[i].style.display=display

 logout('updatedisplay '+element.id+' became '+display)
 
 return

}

function element_neosyssetdropdown(element,request)
{
 
 assertelement(element,'element_setdropdown','element')
 
 if (!element.neosysdropdown)
 {
  systemerror('element_neosyssetdropdown()',element.id+' has no dropdown')
  return
 }
 
 var request='CACHE\r'+element.neosysdropdown[0]
 var colarray=element.neosysdropdown[1]
 
 neosyssetdropdown(element,request,colarray)
 
}

function newrecordfocus()
{

 //return
 login('newrecordfocus')
 
 if (is(gfirstelement)&&gloaded&&!gds.isnewrecord)
 {
  gnextelement=document.getElementsByName(gfirstelement)
  if (gnextelement[0]) gnextelement=gnextelement[0]
 }
 var element=gnextelement
 if (element) gnextelement=''
 else
 {
  if (gloaded||!gKeyNodes)
  {
   //element=gfirstnonkeyelement
   element=document.activeElement
   if (!element.neosysfieldno) element=gfirstnonkeyelement
   assertelement(element,'newrecordfocus','gfirstnonkeyelement')
  }
  else
  {
   element=gKeyNodes[0]
   assertelement(element,'newrecordfocus','gKeyNodes[0]')
  }
 }
  
 //check no required fields are missing in prior data
 //fill in any defaults where possible
 //do this BEFORE setting gpreviouselement as setdefault will overwrite it
 if (!gKeyNodes||glocked)
 {

  //check group 0 always
  checkrequired(gfields,element,0)
  
 }

 //required so that if still focused on an element AFTER loading the record
 //then editing the field and moving onto another element causes a proper validateupdate
//alert(element.id)
 gpreviouselement=document.getElementsByName(element.id)[0]
 //why false??? if (false&&gpreviouselement)
 //false results in repetitive batch number changing still asking discard? when no changes made
 if (gpreviouselement)
 {
  setdefault(gpreviouselement)
  gpreviouselement=document.getElementById(element.id)
  gpreviousvalue=getvalue(gpreviouselement)
 }
 
 // element.focus()
 window.scrollTo(0,0)
 window.setTimeout('focuson("'+element.id+'")',1)
 
 logout('newrecordfocus')
 
}
 
function gds_onreadystatechange()
{

 if (gds.readystate!='complete') return
 login('gds_onreadystatechange')
 grecn=null
 calcfields()
 newrecordfocus()
 logout('gds_onreadystatechange')
}

function tablex_onreadystatechange()
{
 //if need to focus on a table field then
 //do it when the table is complete
 
 if (window.event.srcElement.readyState!='complete') return
 
 login('tablex_onreadystatechange '+window.event.srcElement.neosysgroupno)

 newrecordfocus()
 
/*
 //if a key or no keys then focus on firstnonkeyfield
 if (gloaded||!gKeyNodes)
 { 
  if (window.event.srcElement.neosysgroupno==gfirstnonkeyelement.neosysgroupno)
  {
   window.setTimeout('focuson("'+gfirstnonkeyelement.id+'")',1)
  }
 }
*/
 
 logout('tablex_onreadystatechange '+window.event.srcElement.neosysgroupno)
 
}

function printsendrecord_onclick()
{
 
 if (window.event)
 {
  window.event.cancelBubble=true
  window.event.returnValue=false
 }
 
 //work out the print function else return
 var printfunction=gKeyNodes[0].neosysprintfunction
 if (!printfunction) return
 //printfunction=printfunction.replace(/%KEY%/g,gkeyexternal)
 //journal print does not work properly so ...
 printfunction=printfunction.replace(/%KEY%/gi,gkey)

 if (!saveandorcleardoc('PRINT',true,false)) return

 neosysevaluate(printfunction,'printsendrecord_onclick()')
 
 if (window.event)
 {
  window.event.cancelBubble=true
  window.event.returnValue=false
 }
 
}

function listrecord_onclick()
{
 
 if (window.event)
 {
  window.event.cancelBubble=true
  window.event.returnValue=false
 }
 
 //work out the print function else return
 var listfunction=gKeyNodes[0].neosyslistfunction
 if (!listfunction) return

 if (!saveandorcleardoc('PRINT',true,false)) return

 neosysevaluate(listfunction,'listrecord_onclick()')
 
 if (window.event)
 {
  window.event.cancelBubble=true
  window.event.returnValue=false
 }
 
}

//'WINDOW ONBEFORE UNLOAD
//'''''''''''''''''''''''
function window_onbeforeunload()
{

 if (!ginitok) return
 
 login('window_onbeforeunload' )

 //first update the current field
 //otherwise the db is updated without the last entry
 //zzz unfortunately does not seem to work!
 try{window.event.srcElement.blur()} catch(e){}
 
 gwindowunloading=true

 if (closedoc('WINDOWCLOSE',true))
 {
  //gwindowunloading=true
 }
 else
 {
  //set the following and the user can cancel the window unload event
  window.event.returnValue='WARNING: YOU HAVE NOT SAVED YOUR DATA!'
 }

 gwindowunloading=false
 
 logout('window_onbeforeunload')
 
}

//WINDOW_ONUNLOAD
/////////////////
function window_onunload()
{

 if (!ginitok) return
 
 if (glocked) unlockdoc()
 
 gwindowunloading=true
 savecache()
 gwindowunloading=false
 
 //save gdataset in case we are refreshing and the parent window isnt there to get it from
 neosyssetcookie('','NEOSYSlogincode',glogincode,'logincode')
//loginalert('onunload '+glogincode)
}

////////////////// DOCUMENT EVENTS /////////////////////

//DOCUMENT ON KEY DOWN
//////////////////////
function document_onkeydown()
{

 //no processing if not initialised
 if (!ginitok) return
 
//log(window.event.srcElement.tagName+' '+window.event.keyCode)

 //must be before custom key handlers which may use gkey
 keycode=window.event.keyCode
 gkeycode=keycode
 if (gstepping) wstatus(gkeycode)

 //custom key handlers
 //must return false to prevent further action
 if (typeof form_onkeydown=='function')
 {
  if (!form_onkeydown())
  {
   window.event.cancelBubble=true
   window.event.returnValue=false
   return false
  }
 }
 
 gwindowunloading=false
 element=window.event.srcElement
 
// if (typeof(closerecord)=='undefined') return
 
 try
 {
  if (element.tagName=='TD')
  {
   if (element.firstChild.tagName=='SPAN')
   {
    element=element.firstChild
   }
  }
 }
 catch(e){}
 
//alert(gkeycode) 
 //refresh (F5) system key refresh (do manually because mac does not seem to bubble key events)
 if (keycode==116)
 {
  if (ismac) document.location.reload(true)//true means from server, not cache
  return true
 }

 //close (F8)
 if (keycode==119)
 {
  closedoc('CLOSE',false)
  return false
 }
 
 //alt+k or alt+home is goto top (but alt+home goes to home page and cannot be cancelled)
 if ((keycode==75&&event.altKey)||(keycode==36&&event.altKey))
 {
  window.setTimeout('focuson(gstartelement.id)',1)
  window.event.cancelBubble=true
  window.event.returnValue=false
  return false
 }

 //menu bar hot keys
 if (event.altKey)
 {
  var found=true
  if (gkeycode==77) window.setTimeout('menuonmouseover(menubutton)',1)
 /* done by accesskeys on hidden buttons
  else if (gkeycode==78) window.setTimeout('newrecord_onclick()',1)
  else if (gkeycode==76) window.setTimeout('neosyslogout()',1)
  else if (gkeycode==79) window.setTimeout('openrecord_onclick()',1)
  else if (gkeycode==83) window.setTimeout('saverecord_onclick()',1)
  else if (gkeycode==67) window.setTimeout('closerecord_onclick()',1)
  else if (gkeycode==82) window.setTimeout('releasereopenrecord_onclick()',1)
  else if (gkeycode==69) window.setTimeout('deleterecord_onclick()',1)
  else if (gkeycode==73) window.setTimeout('listrecord_onclick()',1)
  else if (gkeycode==80) window.setTimeout('printsendrecord_onclick()',1)
  else if (gkeycode==88) window.setTimeout('refresh_onclick()',1)
 */
  else found=false
  if (found)
  {
   //try {gpreviouselement.focus()}
   //catch(e){}
   window.event.cancelBubble=true
   window.event.returnValue=false
   return false
  }
 }
   
 //alt+end is goto bottom
 if (keycode==35&&event.altKey)
 {
  window.setTimeout('focuson(gfinalinputelement.id)',1)
  window.event.cancelBubble=true
  window.event.returnValue=false
  return false
 }
 
 //F12 is neosys debug key
 //if (keycode==123/*&&gusername.indexOf('NEOSYS')+1*/)
 if (keycode==123&&gusername.indexOf('NEOSYS')+1)
 {
 
  //F12 - toggle stepping2
  if (!window.event.shiftKey&&!window.event.ctrlKey&&!window.event.altKey)
  {
   gstepping=!gstepping
   wstatus('stepping='+gstepping)
   return false
  }

  //shift+ctrl+F12 - break
  //force error and into breakpoint
  if (window.event.shiftKey&&window.event.ctrlKey)
  {
   gstepping=true
   breakpoint('','F12','')
   return false
  }
  
  //shift+alt+F12 - show event log
  if (window.event.shiftKey&&window.event.altKey)
  {
   glogging=true
   wstatus('glogging=true')
   if (geventlog)
   {
    windowx=windowopen()
    if (windowx) windowx.document.body.innerHTML=geventlog
   }
   return false
  }
  
  //shift+F12 - element.outerHTML
  if (window.event.shiftKey) temp=element
  
  //ctrl+F12 - element.parentNode.outerHTML
  if (window.event.ctrlKey) temp=temp.parentNode

  //alt+F12 - element.parentNode.parentNode.outerHTML
  if (window.event.altKey&&temp&&temp.parentNode) temp=temp.parentNode.parentNode

  if (temp&&temp.outerHTML)
  {
   var windowx=windowopen()
   if (windowx) windowx.document.body.innerText=decodehtmlcodes(temp.outerHTML)
  }
  
  return false
  
 }

 //F2 is now edit top be like office
 var textrange
 if (keycode==113&&(textrange=document.selection.createRange()).text!='')
 {
  textrange.collapse(false)
  textrange.select()
  window.event.cancelBubble=true
  window.event.returnValue=false
  return false
 }
 
 //F6 is now link
 if (keycode==117)
 {
  neosyslink()
  window.event.cancelBubble=true
  window.event.returnValue=false
  return false
 }
 
 //F7 is now popup (used to be F2 in arev) also replaces windows standard alt+down combination
 if (keycode==118||(event.altKey&&keycode==40&&element.tagName=='SELECT'))
 {
  neosysformpopup(element)
  window.event.cancelBubble=true
  window.event.returnValue=false
  return false
 }
 
 //get groupno otherwise return
 ggroupno=element.neosysgroupno
 
 grecn=null 
 if (ggroupno>0)
 {
 
  //get the current table and ggroupno else exit
  var rowx=getancestor(element,'TR')
  var tablex=getancestor(rowx,'TABLE')
  if (!tablex) return
  var rows=tablex.tBodies[0].childNodes
  
  //get the rown else exit
  if (ggroupno>0)
  {
   var rown=rowx.rowIndex
   if (tablex.tHead) rown-=tablex.tHead.rows.length
  }
  
  //get the recordset
  var rs=gds.data['group'+ggroupno]
  if (!rs)
  {
   window.event.cancelBubble=true
   window.event.returnValue=false
   return neosysinvalid('Group number on non-group field')
  }
  //zzz if "paging" should be offset by subtracting record number of first row
  grecn=rown
  gnrecs=rs.length
  
  var nrows=rows.length
  var pagesize=tablex.dataPagesize?tablex.dataPagesize:999999
  var pagen=neosysint(grecn/pagesize)
  var npages=Math.ceil((rs.length)/pagesize)
  
 }

 //ctrl+G is goto line
 if (keycode==71&event.ctrlKey)
 {
  if (!validateupdate()) return false
  while (true)
  {
  
   var elements=ggroupno?element:gfields[gtables[1][0]]
   elements=document.getElementsByName(elements.id)
   
   var reply=window.prompt('Go to line number ? (1-'+elements.length+')',grecn+1)
   if (!reply) break
   
   reply=(+reply)
   if (reply<1) reply=1   
   if (reply>elements.length) reply=elements.length
   
   focuson(elements[reply-1])
   break
   
  }
  
  window.event.cancelBubble=true
  return false
 }

 //alt+Y is copy previous record/column
 if (keycode==89&&event.altKey&&element.type!='button')
 {
  if (grecn>0)
  {
   setvalue(element,getpreviousrow(element.id,true))
   try{element.select()}catch(e){}
   window.event.cancelBubble=true
   window.event.returnValue=false
   return false
  }
 }
 
 //ctrl+home is goto first column
 if (tablex&&keycode==36&&event.ctrlKey)
 {
  var id=gfields[tablex.neosysfirstinputcolumnscreenfieldno]
  if (id) window.setTimeout('focuson("'+id.id+'")',1)
  window.event.cancelBubble=true
  window.event.returnValue=false
  return false
 }
 
 //ctrl+end is goto last column
 if (tablex&&keycode==35&&event.ctrlKey)
 {
  var id=gfields[tablex.neosyslastinputcolumnscreenfieldno]
  if (id) window.setTimeout('focuson("'+id.id+'")',1)
  window.event.cancelBubble=true
  window.event.returnValue=false
  return false
 }
 
 //Enter on textarea
 if (!ismac&&keycode==13&&(element.tagName=='TEXTAREA'&&!window.event.ctrlKey&&!window.event.shiftKey))
 {
  if (gKeyNodes.length&&!glocked)
  {
   keycode=9
   window.event.keyCode=keycode
  }
  if (element.value==''||element.value.slice(-4)=='\r\n\r\n')
  {
   element.value=element.value.slice(0,-4)
   keycode=9
   window.event.keyCode=keycode
  }
 }
 
 //enter key becomes tab (or ok if only one entry field)
 if (keycode==13&&(element.tagName!='TEXTAREA'||window.event.ctrlKey||window.event.shiftKey))
 {
  
  //mac ie5 event.keyCode is read only so cannot force a tab
  if (ismac&&!window.event.ctrlKey)
  {
   if (window.event.shiftKey) direction=-1
    else direction=1
   focusdirection(direction,element)
   return false
  }
   
  //ctrl+enter or enter with single field is ok/save record
  //if (window.event.ctrlKey||(gfields.length==1&&gfields[0].neosysgroupno==0))
  if (window.event.ctrlKey||(element.neosysgroupno==0&&gfinalinputelement&&gfinalinputelement.id==gstartelement.id))
  {
   if (!saverecord.disabled)
   {
    //savedoc()
    saverecord_onclick()
   }
   return false
  }
  
  //enter on last field goes to start of doc
  if (!window.event.shiftKey&&element==gfinalinputelement&&(ggroupno==0||(ggroupno>0&&grecn==gnrecs)))
  {
   focuson(gstartelement)
   return false
  }
  
  //shift+enter on 1st key field goes to end of doc
  if (window.event.shiftKey&&element==gstartelement)
  {
   focusdirection(-1,element)
   return false
  }

  
  keycode=9
  window.event.keyCode=keycode

  //dont return because enter key has some special processing on rows below
  //return
    
/*  
  if (element.tagName!='TEXTAREA')
  {
   if (window.event.shiftKey)
    focusprevious()
   else
    focusnext()
  }
  return
*/
 }

 //F9 is also save
 if (keycode==120)
 {

  //first update the current field
  //otherwise the db is updated without the last entry!!!
  if (!validateupdate()) return false
  
  //prevent document save unless the save button is enabled
  if (!saverecord.disabled)
  {
   //savedoc()
   saverecord_onclick()
  }

 return false
 
 }
 
 //esc
// if(keycode==27&&gloaded)
 if(keycode==27)
 {

  //close any "modal" popups
  if (form_closepopups())
  {
   //prevent normal esc handling
   window.event.returnValue=false
   window.event.cancelBubble=true
   return false
  }

  //if changing current field then let system use it to 'undo' changes
  if (element.dataFld&&element==gpreviouselement)
  {
  
   value=getvalue(element)

   //old value
   gdefaultvalue=getdefault(element)
   if (gpreviousvalue==''&&gdefaultvalue) gpreviousvalue=gdefaultvalue
    
   if (value!=gpreviousvalue)
   {
   
    //restore the original value
    //get from datasource
    setvalue(element,gpreviousvalue)
     
    //prevent normal esc handling
    window.event.returnValue=false
     
    //select it
    try
    {
      if (element.tagName!='TEXTAREA') element.select()
    }
    catch (e){}
     
    return
   }
  }

  closerecord_onclick()

  return false

 }
 
 //all remaining key events are related to loaded records
 if (gKeyNodes&&closerecord.disabled) return
 
 //esc when no changes means closerecord
   
 //the rest of the keys are 
 if (typeof(ggroupno)=='undefined') return
 
 //up and down keys
 if(keycode==38||keycode==40)
 {

  //on textarea leave system to process it
  if (element.tagName=='TEXTAREA') return

  //leave system to process alt+down
  //cannot do this because up/down is then used to select
  //but up/down is required to move between rows
  //if (event.altKey&&keycode==40&&element.tagName=='SELECT') return
  
  //on radio button leave system to process it
  //unless up on first element or down on last element
  if (element.type=='radio'||element.type=='checkbox')
  {
   var elements=document.getElementsByName(element.id)
   if (keycode==38&&element!=elements[0]) return
   if (keycode==40&&element!=elements[elements.length-1]) return
  }
  
  //alt+down (or alt+up!) implements popups
  if (window.event.altKey&&element.neosyspopup)
  {
   neosysformpopup()
   window.event.cancelBubble=true
   window.event.returnValue=false
   return
  }
 
 }
 
 //left arrow or right arrow and whole field is selected else return
 if (keycode==37||keycode==39)
 {

  //allow alt+left and right to do usual IE navigation
  if (event.altKey) return true
  
  //suppress left right if text not wholy selected and maxlen!=1
  if (ismac)
  {
   //mac docs say document.selection object works but it is null
   var selection=document.selection
   if (selection) selection=selection.type
   if (element.tagName.match(gtexttagnames)&&element.type!='button'&&selection!='Text') return
  }
  else
  {
   if (document.selection&&element.maxLength!=1)
   {
    var textrange=document.selection.createRange()
    //var textrange=document.forms[0].selection.createRange()
    if (element.tagName.match(gtexttagnames)&&element.type!='button'&&textrange.text!=getvalue(element)) return
   }
  }

  //left or right not in tables
  if (ggroupno==0)
  {
   if (keycode==37)
    focusprevious()
   else
    focusnext()  
   return false
  }
  
 }
 
 if (keycode==40)
 {
  //event.returnValue=false
  //event.cancelBubble=true
  //return false
 }
 
 //up down keys outside a table are like left/right
 //if((keycode==38||keycode==40)&&ggroupno==0&&element.tagName!='SELECT'&&element.type!='radio')
 if((keycode==38||keycode==40)&&ggroupno==0)
 {

  //option to revert to windows standard (eg to show rapid effect of different values)
  if (element.tagName=='SELECT'&&element.neosysallowcursor) return true
  
  if (keycode==38)
  {
   focusprevious()
  }
  else
  {
   focusnext()
  }
  
  //prevent the key from being processed as a cursor movement
  window.event.cancelBubble=true
  window.event.returnValue=false
 
  return
  
 }
 
 //tab or down on first or last col of LAST row is special
 if ((ggroupno>0)&&grecn==(rs.length-1))
 {
  if ((keycode==9||keycode==40)&&!window.event.shiftKey)
  {
   //on last column - add a row
   var firstcolelement=rowx.all(gfields[gtables[ggroupno][0]].id)
   if ((glocked||!gKeyNodes)&&element.neosysscreenfieldno==tablex.neosyslastinputcolumnscreenfieldno&&getvalue(firstcolelement)!='')
   {
    if (glocked||!gKeyNodes) neosysaddrow(ggroupno)
   }
   //on first column, if empty - go to next field after current table
   if (element.neosysisfirstinputcolumn&&getvalue(element)=='')
   {
    focusdirection(1,element,ggroupno)
    return false
   }
  }
 }
 
 //left arrow and right arrow in tables
 if (ggroupno!=0&&rown>=0&&(keycode==37||keycode==39))
 {
  var scope=rows[grecn].all
  if (keycode==37)
   focusprevious(element,scope)
  else
   focusnext(element,scope)
  return false
 }
 
 //PGUP/PGDN/UP/DOWN
 if(keycode==34||keycode==33||keycode==40||keycode==38||keycode==37||keycode==39)
 {

  if (ggroupno==0) return
  if (rown<0) return//must be in a thead row

  //if down arrow on last row then add a new row
  if(keycode==40&&grecn==(rs.length-1))
  {

   if (glocked||!gKeyNodes) neosysaddrow(ggroupno)
     
  }
  
  var nrows=rows.length
  var id=element.id
  if (!id) return//neosys like button with no id?
  
  var pagesize=tablex.dataPagesize?tablex.dataPagesize:999999
  var pagen=neosysint(grecn/pagesize)
  var npages=Math.ceil((rs.length)/pagesize)
  
  //pgdn or down arrow
  if(keycode==34||keycode==40)
  {
   
   //ctrl+pgdn or down arrow goes to last row of last page
   if (window.event.ctrlKey)
   {
    //tablex.lastPage()
    focuson(rows[rows.length-1].all(id))
   }
   
   //if not on last row of page then goto last row
   else
   {
   
    if (rown<nrows-1)
    {
     if (keycode==40)
     {
      if (!rows[rown+1].all(id))
      {
       systemerror('document_onkeydown()','Cannot locate id '+id+' in rown: '+rown+' nrows:'+rows.length+' tagname:'+rows[rown+1].all(id))
       return false
      }
      focuson(rows[rown+1].all(id))
     }
     else
     {
      var newrown=grecn+gpagenrows
      if (newrown>(nrows-1)) newrown=nrows-1
      focuson(rows[newrown].all(id))
     }
    }
    
    //if on last row go to first row of next page if any, otherwise stay put
    else
    {
    
     if (pagen<npages-1)
     {
      //tablex.nextPage()
      focuson(rows[0].all(id))
     }
    }
   }
  
  }
  
  //pgup or up arrow
  if(keycode==33||keycode==38)
  {
   //ctrl+pgup or up
   //goes to first line of first page
   if (window.event.ctrlKey)
   {
    //tablex.firstPage()
    focuson(rows[0].all(id))
   }
   else
   {
   
    //if not on first row of page go up one row (up arrow) or to first line (pgup)
    if (rown>0)
    {
     if (keycode==38)
     {
      focuson(rows[rown-1].all(id))
     }
     else
     {
      //focuson(rows[0].all(id))
      var newrown=grecn-gpagenrows
      if (newrown<0) newrown=0
      focuson(rows[newrown].all(id))
     }
    }
   
    //if on first line of page go to last line of previous page if any, or stay put
    //(same for pgup and up)
    else
    {
     if (grecn==0)
     {
      focusprevious(element)
     }
     else
     {
      //tablex.previousPage()
      focuson(rows[nrows-1].all(id))
     }
    }
   }
  }
 
 //prevent the key from being processed as a cursor movement and lose the selection
 window.event.cancelBubble=true
 window.event.returnValue=false
 
 return
 
 }


 //following are all keycodes when within tabular rows section
 //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

 //if the element has no value, the user must have selected many elements
 try
 {
  var srcvalue
  srcvalue=getvalue(element)
 }
 catch(e){}
 
 if (ggroupno>0)
 {
  
  var button
  
  //Ctrl+N is insert row
  if (keycode==78&&window.event.ctrlKey)
  {
   if ((button=rowx.all['insertrowbutton'+ggroupno])&&button.style&&button.style.display!='none')
   {
    insertrow()
   }
   
   window.event.returnValue=false //prevent new window opening
   window.event.cancelBubble=true
   return false
   
  }
  
  //Ctrl+D is delete row
  if (keycode==68&&window.event.ctrlKey)
  {
   
   if ((button=rowx.all['deleterowbutton'+ggroupno])&&button.style&&button.style.display!='none')
   {
    deleterow()
   }
   
   window.event.returnValue=false //prevent any further action
   window.event.cancelBubble=true
   return false
   
  }

 }

 //protect readonly fields
 if (!(window.event.altKey||window.event.ctrlKey)&&(element.neosysreadonly||(gKeyNodes&&!glocked&&element&&element.neosysfieldno&&element.neosysfieldno!=0)))
 {
  if (![9,16,17,18,20,35,36,112,113,114,115,116,117,118,119,120,121,122,123].neosyslocate(keycode))
  {
   if (gkeys.length&&!glocked) readonlydocmsg()
   else if (element.neosysreadonly!=true) neosysinvalid(element.neosysreadonly)
   window.event.returnValue=false
   window.event.cancelBubble=true
   return false
  }
 }

}

function focusprevious(element,scope)
{
 focusdirection(-1,element,'',scope)
}

function focusnext(element,scope)
{
 focusdirection(1,element,'',scope)
}

function focusdirection(direction,element,notgroupno,scopex)
{
   
   if (typeof notgroupno=='undefined') notgroupno=''
   
   //find next input field
   var nextelement=element
   if (typeof(nextelement)=='undefined')
   {
    if (!window.event) return
    nextelement=window.event.srcElement
   }
//   if (!element) return

   if (typeof scopex=='undefined') scope=document.all
   else scope=scopex
   
   //check scope. why?
   var sourceindex=nextelement.sourceIndex
   var scopeindex=sourceindex-scope[0].sourceIndex
   if (scope[scopeindex]!=nextelement)
   {
    //wrongscopeindex()
    focuson(nextelement)
    return
   }
   
   //test to prevent endless loop
   var passzero=scopeindex
      
   do
   {
    var scopeindex=scopeindex+direction

    //prevent infinite loop
    if (passzero==scopeindex)
    {
     //break
     return
    }
    
    //wrap/scroll down to the bottom
    if (scopeindex<0)
    {     
     if (typeof scopex=='undefined') window.scrollBy(0,999999)
     scopeindex=scope.length-1
    }
    
    //wrap/scroll up to the top
    else
    {
     if (scopeindex>scope.length-1)
     {
      if (typeof scopex=='undefined') window.scrollBy(0,-999999)
      scopeindex=0
     }
    }
    
    nextelement=scope[scopeindex]
   
    if (typeof(notgroupno)=='undefined')
    {
     scrollintoview(nextelement)
    }

//   } while((nextelement.tagName!='INPUT'&&nextelement.tagName!='SELECT')
    } while((!nextelement.tagName.match(gdatatagnames)||typeof nextelement.isTextEdit!='undefined'&&nextelement.isTextEdit==false)
           ||
           (notgroupno!==''&&nextelement.neosysgroupno==notgroupno)
           ||
           (nextelement==element)
           ||
           nextelement.neosysreadonly
           ||
           //qqq
           !nextelement.clientWidth
           ||
           nextelement.currentStyle.display=='none'
           ||
           nextelement.parentNode.currentStyle.display=='none'
          )
   
   //found it. focus on it
   focuson(nextelement)

}

function scrollintoview(element)
{
 if (!element) return
 
 //only scroll input fields
 var tagname=element.tagName
 if (!tagname.match(gdatatagnames)) return
 if (!element.dataFld) return
 
 //get total left offset
 var offsetleft=0
 var element2=element
 do
 {
  offsetleft+=element2.offsetLeft
  element2=element2.offsetParent
 }
 while (element2)

 leftextra=100
 rightextra=100
 
 if (offsetleft<leftextra)
 {
  window.scrollBy(-99999,0)
  return
 }
 
 //scroll left
 var scrollleft=document.body.scrollLeft-offsetleft 
 if (scrollleft>0)
 {
  window.scrollBy(-scrollleft-leftextra,0)
 }

 //scroll right
 else
 {
  var scrollright=offsetleft+element.offsetWidth-document.body.clientWidth-document.body.scrollLeft
  if (scrollright>0)
  {
   window.scrollBy(scrollright+rightextra,0)
  }
 }
 
 return
 
}
 
///////////////////// BUTTON EVENTS /////////////////////////

//NEW button
////////////
function newrecord_onclick()
{

 //save and close existing document otherwise cancel
 if (gkey&&!closedoc('NEW',true)) return false

 // element.focus()
 window.setTimeout('focuson("'+gfirstnonkeyelement.id+'")',10)
 
 //newrecordfocus()

}

//RELEASE BUTTON
////////////////
function releasereopenrecord_onclick()
{

 //not locked and there is a key
 if (!glocked)
 {
  opendoc(gkey)
  return true
 }
 
 if (!saveandunlockdoc()) return false
 
 //releasereopenrecord.value='Reopen'
 setgraphicbutton(releasereopenrecord,'Reopen',greopenimage)
 releasereopenrecord.disabled=false

 return true
 
}

//SAVE BUTTON
/////////////
function saverecord_onclick()
{
 
 //first update the current field
 //otherwise the db is updated without the last entry!!!
 if (!validateupdate()) return false

 //check for missing required
 if (!validateall()) return false

// saverecord.focus()
 
 //if databound form then save it
 if (gKeyNodes)
 {

  //save the record
  if (!savedoc('SAVE')) return false

  //prune the cache to reselect any edited record
  prunecache('SELECT\r'+gdatafilename+'\r')
  prunecache('READ\r'+gdatafilename+'\r'+gkey) 
  
  //custom postwrite function
  if (typeof form_postwrite=='function')
  {
   //neosysevaluate2('form_postwrite()')
   if (!neosysevaluate2('form_postwrite()')) return false
  }
  //otherwise automatic option to print if available
  else
  {
   if (printsendrecord&&!printsendrecord.disabled&&neosysyesno('Print/Send '+gkeyexternal+' ?',2)) printsendrecord_onclick()
  }
  
  return true
  
 }
 
 //from here on is NOT a databound form
 //////////////////////////////////////

 //custom prewrite function
 if (typeof(form_prewrite)=='function') if (!neosysevaluate2('form_prewrite()','saverecord_onclick()')) return false
 
 //option to confirm
 if (gparameters.confirm||gparameters.savemode&&gparameters.savemode.indexOf('CONFIRM')>=0)
 {
  if (!neosysyesno('OK to continue?',2))return false
 }
 
 ///convert screen data into a rev string
 gro.data=gds.data
 if (!gro.obj2rev()) return false
 
 //alert(gro.revstr.split(fm))
 
 //custom write and postwrite routine
 if (typeof form_write=='function')
 {
  if (!neosysevaluate2('form_write()','saverecord_onclick()')) return false
  if (typeof(form_postwrite)=='function') neosysevaluate2('form_postwrite(db)','saverecord_onclick()')
  return true
 }
 
 //option to return the form data to a calling program
 //instead of sending to the database
 if (window.dialogArguments&&(!gparameters.savemode||!gparameters.savemode.neosysswap('CONFIRM','')))
 {
  window.returnValue=gro.revstr
  window.close()
  return false
 }
 
 //standard unbound write routine
 if (!unbound_form_write()) return false
 
 //postwrite routine
 if (typeof(form_postwrite)=='function')
  neosysevaluate2('form_postwrite(db)','saverecord_onclick()')
 else
 {
  if (db.data) windowopen(db.data)
 }
 if (gparameters.savemode&&gparameters.savemode.indexOf('CLOSE')>=0) window.close()
 
 return true
 
}

function unbound_form_write()
{
 
 //send the instructions for processing and open the report
 db.request='EXECUTE\r'+gmodule+'\r'+gdatafilename
 if (!db.send(gro.revstr))
 {
  neosysinvalid(db.response)
  return false
 }
 if (db.response.slice(0,3)=='OK ') neosysnote(db.response.slice(3))
 
 return true
 
}

//CLOSE BUTTON
//////////////
function closerecord_onclick()
{

 //cancel on dialoged window closes window returns false
 if (window.dialogArguments||!gKeyNodes.length&&gparameters.savemode&&gparameters.savemode.indexOf('CONFIRM')>=0)
 //if (window.dialogArguments)
 {
 
  //user confirms or cancels
  if (gchangesmade)
  {
   var response=neosysyesnocancel('Discard data entered ?',1,'','Discard','Cancel')
   if (response!=2) return false
  }

  var returnvalue=''
  
  //clear the document so user is not asked again in onunload
  if (gKeyNodes)
  {
   gwindowunloading=true
   if (!gchangesmade) returnvalue='ACCESS '+gkey
   cleardoc()
  }
     
  if (window.dialogArguments)
  {

   //close the window
   window.returnValue=returnvalue
   window.close()
  
   //fail safe
   return false
  }
  
 }

 //close/clear the document
 //false is like f8 key pressed
 //closedoc('CLOSE',false)
 return closedoc('CLOSE',true)
 
}

//DELETE BUTTON
///////////////
function deleterecord_onclick()
{

 if (!deletedoc())
 {
  window.event.returnValue=false
 }

 //prune the cache to reselect any deleted record
 prunecache('SELECT\r'+gdatafilename)
  
 //custom postdelete function
 if (typeof form_postdelete=='function')
 {
  //neosysevaluate2('form_postdelete()')
  if (!neosysevaluate2('form_postdelete()')) return false
 }

 return true
  
}
 
//CHANGE PAGE BUTTONS
/////////////////////
function changepage(pagen)
{
 if (gKeyNodes&&closerecord.disabled) return false
 
 var tablex=getancestor(window.event.srcElement,'TABLE')
 if (tablex!=null&&typeof(tablex.neosysgroupno)!='undefined')
 {
  switch (pagen)
  {
   case 0:{tablex.firstPage();break}
   case -1:{tablex.previousPage();break}
   case 1:{tablex.nextPage();break}
   default:{tablex.lastPage();break}
  }
 }
 
}

/////////////////////// DOCUMENT FUNCTIONS //////////////////


//'''''''''
//'OPENDOC
//'''''''''

var gopening=false

function opendoc(newkey)
{

 if (newkey=='%NEW%') newkey=''
 
 //check potential key has as many parts as there are key fields
 if (newkey&&gKeyNodes.length>1)
 {
  var sepchar=gKeyNodes[0].neosyskeypart?'*':gKeyNodes[0].neosyswordsep
  if (sepchar&&newkey.split(sepchar).length!=gKeyNodes.length)
  {
   systemerror('opendoc',newkey.neosysquote()+' newkey does not return the correct number of keyparts')
   return false
  }
 }
 
 gopening=true
 var opened=opendoc2(newkey)
 gopening=false

 //add key into gkeys
 if (opened&&gKeyNodes.length)
 {
  if (!gkeys.neosyslocate(gkey))
  {
   gkeys=gkeys.neosysinsert(gkeyn+1+1,gkey)
   if (gkeys.length>1) gkeyn++
  }
 }
  
 //focus on key (last if more than one) element if opendoc2 fails
 if (!opened)
 {
  window.setTimeout('try{gKeyNodes[gKeyNodes.length-1].focus()}catch(e){}',10)
  return false
 }
 
 return true
 
}

function opendoc2(newkey0)
{
 
 login('opendoc2')
 
 //detect if a new key is present and quit if not
 var newkey=''
 if (newkey0)
 {
  newkey=newkey0
 }
 else
 {
  if (gKeyNodes)
  {
   newkey=getkey()
   if (newkey==gkey&&gloaded)
   {
    //newkey=''
    return logout('opendoc2 - no new key')
   }
  }
  if (!newkey)
  {
//qqq allow read without key to return next sequential key
//   return logout('opendoc2 - no new key')
  }
 }
 
 //unload previous doc first
 ///////////////////////////
 // if (gloaded&&!closedoc('OPEN',true))
 //have to clear even if not loaded othewise setkeyvalues fails because gds.data not yet available when key=xxx in the url
 if (!closedoc('OPEN',true))
 {
  //qqq should try and restore the previous key data?
  return logout('opendoc2 - did not close currently open doc')
 }

 setkeyvalues(newkey)//closedoc zaps them.
 gkey=newkey
 gkeyexternal=getkeyexternal()
 
 //optional preread function (returns false to abort) 
 if (typeof(form_preread)=='function')
 {

  log('preread external key='+gkeyexternal+' internalkey='+gkey)
  if (!neosysevaluate2('form_preread()','opendoc2()'))
   return logout('opendoc2 - preread false')
  
 }
 
 wstatus('Requesting data, please wait ...')
 
 //read record (with or without a lock)
 /////////////
 
 //gkey can be blank in which case a new key will be
 //returned in gro.key
 
 gro.key=gkey
 gro.read(!greadonlymode)

 loadnewkey()
 
 glocked=gro.sessionid!=''
 //ensure lock is automatically re-locked every x minutes while record is active on-screen
 if (glocked) startrelocker()
 
 //fail if no data (failure to reach backend database)
 //listen always provides data even if only an empty dataset (eg new record)
 
 if (!gro.data)
 {
  if (glocked) unlockdoc()//fail safe
  
  logout('opendoc2 - no data')
  return neosysinvalid(gro.response)
 }
 
 var lockholder='somebody else'
 var tt=db.response.indexOf('LOCKHOLDER:')
 if (tt>=0) lockholder=db.response.slice(tt+11).split('"')[1]
 
 if (db.response.toUpperCase().slice(0,16)=='ERROR: NO RECORD')
 {
  if (gupdateonlymode)
  {
   //neosysinvalid(neosysquote(gkeyexternal)+' does not exist.')
   neosysinvalid(neosysquote(getkeyexternal())+' does not exist.')
   logout('opendoc2 - cannot create new record because gupdateonlymode is true')
   return false
  }
  
  if (!glocked)
  {

   //cannot create records if cannot lock them
   if (db.response.toUpperCase().indexOf('LOCK NOT AUTHORISED')>=0)
   {
    neosysinvalid('Sorry, you are not authorised to create new records in this file.')
    return logout('opendoc2 - cannot create new record because lock not authorised')
   }

   //may not be authorised to read a record
   if (db.response.toUpperCase().indexOf('NOT AUTHORISED')>=0)
   {
    neosysinvalid(db.response)
    return logout('opendoc2 - cannot access record because not authorised')
   }

   //cannot create a record in read only mode or update only mode
   if (greadonlymode||gupdateonlymode||gpreventcreation)
   {
    //neosysinvalid('Sorry, '+gkeyexternal+' does not exist and\ryou are not authorised to create new records in this file.')
    neosysinvalid('Sorry, '+getkeyexternal()+' does not exist and\ryou are not authorised to create new records in this file.')
    return logout('opendoc2 - cannot create new record in read only mode')
   }
   
   //cannot create a record without a lock
   //if possible get another default key
   if (gKeyNodes.length==1&&gKeyNodes[0].neosysdefaultvalue)
   {
    var newkey=getdefault(gKeyNodes[0])
    if (newkey&&newkey!=gkey)
    {
     logout('opendoc2 - cannot create new record because '+lockholder+' is creating it.')
     return opendoc2(newkey)
    }
   }
 
   neosysinvalid(neosysquote(gkeyexternal)+' is being created by '+lockholder+'.\r\n\r\nYou cannot view or update it until they have finished or cancel.')
   logout('opendoc2 - cannot create new record because '+lockholder+' is creating it.')
   return false
  }

  /*
  //optionally cancel if (record does not exist
  if (neosysokcancel('Document '+neosysquote(getkeyexternal)+' does not exist. Create a new document ?',2)!=1)
  {
   if (glocked) unlockdoc()
   window.event.returnValue=false
   return logout('opendoc - user chose not to create new record')
  }
*/
  
  gds.isnewrecord=true
 
 }
 else
 {
 
  if (!glocked)
  {

   //cannot create records if cannot lock them
   if (db.response.toUpperCase().indexOf('LOCK NOT AUTHORISED')>=0)
   {
    //alert('Sorry, you are not authorised to update this file.')
   }
   else
   {
   
    if (greadonlymode)
    {
     //open in read only mode
    }
    else
    {
     if (db.response.toUpperCase().indexOf('CANNOT LOCK RECORD')>=0)
     {
      if (neosysyesnocancel(neosysquote(gkeyexternal)+' is being updated by '+lockholder+'.\rOpen for viewing only?',1,'Yes','','Cancel')!=1)
      {
       return logout('opendoc2 - because it is being updated by '+lockholder+' and the user chose not to open it in read only mode')
      }
      //releasereopenrecord.value='Reopen'
      setgraphicbutton(releasereopenrecord,'Reopen',greopenimage)
      releasereopenrecord.disabled=false
     }
     else
     {
      if (db.response.toString().slice(0,2)!='OK') return neosysinvalid(db.response)
      else neosysnote(db.response.toString().slice(2))
     }
    }
   }
  }
  
  gds.isnewrecord=false
  
 }

 //make sure gds is available in postread
 gds.data=gro.data
 gds.dict=gro.dict
 gds.dictitem=gro.dictitem

 //if record created/updated in form_postread
 //form_postread should set gchangesmade=true to allow save without user edits
 gchangesmade=false
 gpreviouselement=null
 gpreviousvalue=''
 
 //postread
 if (typeof form_postread=='function')
 {
  grecn=null
  if (!neosysevaluate2('form_postread()','opendoc2()'))
  //if (!form_postread())
  {
   if (glocked) unlockdoc()//fail safe
   cleardoc()
   return logout('opendoc2 - postread failed')
  }
  
 }

 //load the record
 if (gds.isnewrecord)
 {
  wstatus('New record')
 }
 else
 {
  wstatus('Formatting, please wait ...')
 }
 gloaded=true
 gds.load(gro)
 if (!gds.isnewrecord) wstatus('Existing record')
  
 document.title=gdoctitle+' '+gkeyexternal.neosysconvert('*',' ')

 //enable the delete and save buttons if locked
 //according to need
 if (glocked)
 {
  saverecord.disabled=false
  releasereopenrecord.disabled=false
  //releasereopenrecord.value='Release'
  setgraphicbutton(releasereopenrecord,'Release',greleaseimage)
  if (!gds.isnewrecord&&!greadonlymode&&!gupdateonlymode&&!gpreventdeletion)
  {
   deleterecord.disabled=false
  }
 }
 
 //enable the close and print buttons
 closerecord.disabled=false 
 if (printsendrecord) printsendrecord.disabled=false
 
 //hide the enter button
 //enter.disabled=true
 //enter.style.display='none'
 
 logout('opendoc2')

 return true
 
}

function loadnewkey()
{

 //load the new key
 if (gkey!=gro.key)
 {
  setkeyvalues(gro.key)
  gkey=getkey()
  //form key  x*y*z* could be a little different than the db key x*y*z
  gkeyexternal=getkeyexternal()
 }
 
}

///////////
//CLOSE DOC
///////////
function closedoc(mode,save)
{

 //WINDOWUNLOAD,CLOSE,NEW,OPEN
 
 //first update the current field
 //otherwise the db is updated without the last entry!!!
 //if (save&&!validateupdate()) return false
  
 return saveandorcleardoc(mode,save,true)
 
}

//makedoc readonly
function saveandunlockdoc()
{
 if (!validateupdate()) return false
 if (gchangesmade&&!savedoc()) return false
 if (!unlockdoc()) return false
 return true
}

//'''''''''''
//'SAVE DOC
//'''''''''''
function savedoc(mode)
{
 
 //called from saverecord_onclick()
 //('SAVE')
 
 //first update the current field
 //otherwise the db is updated without the last entry!!!
 if (!validateupdate()) return false
 
 if (gKeyNodes&&(!gchangesmade||!glocked)&&!gallowsavewithoutchanges)
 {
  neosysinvalid('Nothing to be saved.\r\rPlease enter or change some data first or just click Close')
  return false
 }
 
 var save=true
 var clear=false
 return saveandorcleardoc(mode,save,clear)

}

//'''''''''''
//'SAVE DOC
//'''''''''''
function saveandorcleardoc(mode,save,clear)
{
 
 //called from
 
 //function savedoc(mode)
 //('SAVE',true,false)
 //mode can be SAVE
   
 //function printsendrecord_onclick()
 //('PRINT',true,false)
 
 //function closedoc(mode,save)
 //(mode,save,true)
 //mode can be WINDOWCLOSE,CLOSE,NEW,OPEN
 
 login('saveandorcleardoc '+save+' '+clear)

 //first update the current field
 //otherwise the db is updated without the last entry!!!
 //if (save&&!validateupdate())
 // return logout('saveandorcleardoc - invalidateupdate failed')

 //if anything updated then option to save
 if (glocked&&(gchangesmade||(gallowsavewithoutchanges&&!clear)||(gpreviouselement&&gpreviouselement.neosysfieldno>0&&getvalue(gpreviouselement)!=gpreviousvalue)))
 {
 
  //opportunity to save or cancel
  if (typeof form_prewrite!='undefined'&&!clear)
  {

   //confirm save
   if (gparameters.savemode=='CONFIRM')
   {
    if (!neosysokcancel('OK to save '+neosysquote(gkeyexternal.neosysconvert('*',' '))+' ?',1))
    {
     //return false
     return logout('saveandorcleardoc - user cancelled')
    }
   }
  
   var response=1
  }
  else
  {
  
   var savetitle=''
   var discardtitle=''
   var action=''
   if (save) savetitle='Save'
   action=savetitle
   if (clear)
   {
    discardtitle='Discard'
    if (save) action+=' or '
    action+=discardtitle
   }
   var response=neosysyesnocancel(action+' '+neosysquote(gkeyexternal.neosysconvert('*',' '))+' ?',1,savetitle,discardtitle,'Cancel')
  }
  
  //user cancels
  if (response==0)
   return logout('saveandorcleardoc - user cancelled')

  //prepare to close modal window
  window.returnValue=''
  
  //user chooses to save
  if (response==1)
  {

   //first update the current field
   //otherwise the db is updated without the last entry!!!
   if (!validateupdate())
    return logout('saveandorcleardoc - user cancelled')

   var savekey=gkey
   var wasnewrecord=gds.isnewrecord
   
   if (!writedoc(clear))
    return logout('saveandorcleardoc - writedoc failed')
   
   //prepare to close modal window
   window.returnValue=(wasnewrecord?'NEW ':'UPDATE ')+savekey
   
  }

  //close window if modal also in deletedoc
  if (window.dialogArguments&&(mode=='SAVE'||mode=='DELETE'||mode=='CLOSE'))
  {
   gwindowunloading=true
   cleardoc()
   
   //close the window
   //window.returnValue=''
   window.close()
   
   return false
   
  }
  
  //enable deleterecord
  if (!greadonlymode&&!gupdateonlymode&&!gpreventdeletion) deleterecord.disabled=false

 }

 //clear if they choose not to save
 if (clear||response==2) cleardoc()
 else gchangesmade=false

 logout('saveandorcleardoc ok')

 return true
 
}

function cleardoc()
{

 //no processing if not initialised
 if (!ginitok) return
 
 //unconditionally closes the document and prepares a new blank record
 //but returns false if cannot unlock record
 //1. unlock
 //2. disable buttons
 //3. clear keys and record

 //called by:
 //unloaddoc if no changes or user does not cancel
 //deletedoc if users confirms

 login('cleardoc')
 
 if (gKeyNodes&&!unlockdoc())
  return logout('cleardoc - unlockdoc failed')

 //disable the buttons
 gchangesmade=false
 if (gKeyNodes)
 {
  saverecord.disabled=true
  releasereopenrecord.disabled=true
  //releasereopenrecord.value='Release'
  setgraphicbutton(releasereopenrecord,'Release',greleaseimage)
  deleterecord.disabled=true
  closerecord.disabled=true
  if (printsendrecord) printsendrecord.disabled=true
 }
 
 //set this before clear because .load will call newrecordfocus()
 gpreviouselement=null
 gpreviousvalue=''

 //close it
 if (!gwindowunloading)
 {
 
  //gkey=''
  //gkeyexternal=''
  gloaded=false
  gchangesmade=false  
  document.title=gdoctitle

  //get an empty record and load it
  wstatus('Clearing, please wait ...')
//  if (!glogging)
//  {
   gro.key=''
   gro.revstr=gro.defaultrevstr
   gro.rev2obj()
   gds.load(gro)

   //any parameters named like form by matching name
   for (var paramid in gparameters)
   {
    var paramvalue=gparameters[paramid]
    var dictitem=gro.dictitem(paramid)
    if (!dictitem) continue
    
    //update multiple values (only works for single field groups)
    if (typeof paramvalue=='string'&&dictitem.groupno>0&&paramvalue.indexOf(vm)>=0)
    {
     var element=document.getElementsByName(paramid)[0]
     insertallrows(element,paramvalue.split(vm),exceptfirst=true)
    }
    
    //update single values
    else
    {
     gds.setrecord(paramid,null,paramvalue)
    }
    
   }
  
//  }
  wstatus('')

  resetsortimages()
  
 }

 gkey=''

 if (!gwindowunloading)
 {

  //setfocus on first non-defaulted key node
  var focussed=false
  if (gKeyNodes&&gKeyNodes.length>1)
  {
   for (var i=0;i<gKeyNodes.length;i++)
   {
    setdefault(gKeyNodes[i])
    if (!getvalue(gKeyNodes[i]))
    {
     focuson(gKeyNodes[i])
     focussed=true
     break
    }
   }   
  }
  
  if (!focussed)
  {
   setdefault(gstartelement)
   focuson(gstartelement)
   if (gstartelement.tagName!='TEXTAREA') try{gstartelement.select()}catch(e){}
  }
  
  gchangesmade=false
  
 }

 //force any updates to be validated
 validateupdate()
 
 logout('cleardoc')
 
}

function resetsortimages(groupno)
{
 //reset the sort buttons
 if (!groupno)
 {
  for (groupno=1;groupno<99;groupno++)
  {
   resetsortimages(groupno)
  }
  return
 }

 var elements=document.getElementsByName('sortbutton_'+groupno)
 for (elementn=0;elementn<elements.length;elementn++) elements[elementn].src=elements[elementn].originalsrc
 
}

//not used
function copychildnodes(fromcell,tocell)
{
 for (var childn=0;childn<fromcell.childNodes.length;childn++)
 {
  var fromchildcell=fromcell.childNodes[childn]
  var tochildcell=tocell.insertBefore(fromchildcell.cloneNode(),null)
  if (tochildcell.tagName=='TD') tochildcell.innerText=''
  if (fromchildcell.childNodes.length>0) copychildnodes(fromchildcell,tochildcell)
 }
}

function deletedoc()
{ 

 //prevent delete if not locked
 if (!glocked) return neosysinvalid(readonlydocmsg())

 login('deletedoc')
 
 var question=neosysquote(gkeyexternal)+'\rAre you SURE that you want to\rPERMANENTLY and IRREVERSIBLY\rdelete this document?'
 if (neosysyesno(question,2)!=1)
 {
  window.event.returnValue=false
  neosyswarning('The document has NOT been deleted\rbecause you did not confirm.')
  return logout('deletedoc - user cancelled')
 }

 //stop relocking while trying to delete
 //not really necessary as DELETE is synchronous
 stoprelocker()
 var savekey=gkey
 
 //delete it
 db.request='DELETE\r'+gdatafilename+'\r'+gkey+'\r\r'+gro.sessionid
 if (!db.send())
 {
  neosysnote(db.response)
  
  //start the relocker again
  startrelocker()
  
  return logout('deletedoc - db returns false')
  
 }
 
 //any warnings are appended after response like 'OK xxx'
 if (db.response!='OK'&&typeof form_postdelete=='undefined')
 {
  neosyswarning(db.response.slice(2))
 }
  
 //deleting a record automatically unlocks it
 glocked=false
    
 neosyswarning(neosysquote(gkeyexternal)+' has been deleted.')
  
 //close window if modal also in writedoc
 if (window.dialogArguments)
 {
  gwindowunloading=true
  cleardoc()
  
  //close the window
  window.returnValue='DELETE '+savekey
  window.close()
  
  return false
  
 }
  
 cleardoc()

 logout('deletedoc')

 return true
  
}

function validateall(mode)
{

 login('validateall '+mode)
 
 //check all required fields are present
 //if mode is 'filldefaults' use default if possible
 for (groupno=0;groupno==0||groupno<=gmaxgroupno;groupno++)
 {

  //skip missing groups
  if (groupno>0&&!gtables[groupno]) continue
  
  //for groups, we need to know the first input column name
  //if (groupno>0) var firstcolumnname=gfields[gtables[groupno][0]].name
  if (groupno>0) var firstcolumnname=gfields[gtables[groupno][0]].id
  
  //get header row or single row of group
  var rows=(groupno==0)?[gds.data]:gds.data['group'+groupno]
   
  var allowemptyrows=(groupno==0)?false:gtables[groupno].tableelement.neosysallowemptyrows
  var rowrequired=(groupno==0)?false:gtables[groupno].tableelement.neosysrowrequired
  var nrowsfound=0
  
  for (var rown=0;rown<rows.length;rown++)
  {
   var row=rows[rown]
   
   ////only validate rows if the first input column is entered
   ////if (groupno==0||row[firstcolumnname].element.text!='')
   //if (groupno==0||row[firstcolumnname].text!='')
   
   var anydata=false
   var missingelement=''
   for (propname in row)
   {
    var cell=row[propname]
    
    //skip unbound cells
    var element=cell.element
    if (!element) continue
    
    //skip calculated cells
    if (element.neosystype!='F') continue
    
    //skip cells with data
    if (cell.text&&(typeof cell.text!='string'||cell.text.replace(/ *$/,'')))
    {
    
     //SELECT elements 
     //if (element.tagName=='SELECT'&&element.options[0].selected)
     
     anydata=true
     continue
    }
     
    //skip cells not required
    if (!element.neosysrequired) continue

    //skip cells that can be defaulted
    //do not default in rows to avoid problem of blank line and first/line line
    //filling in unwanted data (perhaps select tags also have similar problem)
    if (!groupno)
    {
     gdefault=getdefault(element)
     if (gdefault!=null&&gdefault!='')
     {
      cell.text=gdefault
      anydata=true
      continue
     }
    }
    
    //flag first element that is required, enabled and visible and missing
    if (mode!='filldefaults'&&!element.disabled&&neosysenabledandvisible(element)&&!missingelement)
    {
     missingelement=element
     if (groupno==0) break
    }
    
   }//propname in row
   
   //empty last row is always ok
   if (groupno>0&&!anydata&&rown==rows.length-1) missingelement=false
   
   //fail if any missing data
   if (missingelement&&(!allowemptyrows||anydata))
   {
    focuson(missingelement)
    neosysinvalid(missingelement.neosystitle+' is required.')
    return logout('validateall '+mode)
   }
   
   //prevent blank rows (except last one) unless allowed
   if (mode!='filldefaults'&&!anydata&&groupno!=0&&rown<(rows.length-1))
   {
    if (!allowemptyrows)
    {
     var missingelement=rows[0][firstcolumnname].element
     focuson(missingelement)
     logout('validateall '+mode+' empty row')
     return neosysinvalid('Empty rows are not allowed for '+missingelement.neosystitle)
    }
   }

   //count the number of rows with data
   if (anydata) nrowsfound++

  }//rows

  //error if not enough rows
  if (groupno>0&&mode!='filldefaults'&&!nrowsfound&&rowrequired&&neosysenabledandvisible(document.getElementById('neosysgroup'+groupno)))
  {
   var missingelement=rows[0][firstcolumnname].element
   focuson(missingelement)
   logout('validateall '+mode+' no rows')
   return neosysinvalid('At least one '+missingelement.neosystitle+' is required.')
  }
  
 }
 
 logout('validateall '+mode)
 return true
 
}

function writedoc(unlock)
{

 login('writedoc')

 //check all required fields are present
 if (!validateall())
  return logout('writedoc - validate all failed')

 //custom prewrite routine
 goldvalue=''
 gvalue=''
 if (typeof(form_prewrite)=='function')
 {
  if (!neosysevaluate2('form_prewrite()','writedoc()')) return false
  log('form_prewrite - after')
 }
 
 //stop the relocker while saving the document
 stoprelocker()

 //ok save it

 gro.request=unlock?'WRITEU':'WRITE'+'\r'+gdatafilename+'\r'+gkey
 gro.data=gds.data
 if (!gro.writex(unlock))
 {
 
  //neosysnote('Cannot save '+neosysquote(gkeyexternal)+' because: \r\r'+gro.response)
  neosysinvalid('Cannot save '+neosysquote(gkeyexternal)+' because: \r\r'+gro.response)

  return logout('writedoc - write failed')
  
 }

 //any warnings are appended after response like 'OK xxx'
 if (gro.response!='OK'&&typeof form_postwrite=='undefined')
 {
  neosyswarning(gro.response.slice(2))
 }

 //if a cached is written then remove it from the cache (could update it instead?)
 deletecacherecord(gdatafilename,gkey)
 
 //option to unlock after saving
 if (unlock)
 {
  glocked=false
 }
 else
 {
 
  //restart the relocker if failed to save
  startrelocker()
 }

 loadnewkey()
   
 //merge new data back into the screen
 if (gro.data)
 {
  wstatus('Formatting, please wait ...')
  
  //existing gds' elements are pointing to screen elements so do not copy
  //gds.data=gro.data
  
  gds.rebind(gro.data)
  
  var recn=grecn
  grecn=null
  calcfields()
  grecn=recn
  
  wstatus('')
  
  gds.isnewrecord=false
  if (!gupdateonlymode&&!greadonlymode&&!gpreventdeletion) deleterecord.disabled=false
  
  //update in case changed (eg rows resorted)
  var temp=getvalue(gpreviouselement)
  if (gpreviousvalue!=temp)
  {
   gpreviousvalue=temp
   try{gpreviouselement.select()}catch(e){}
  }
  
 }
  
 logout('writedoc ok')

 return true
 
}

function startrelocker()
{
 grelocker=window.setInterval('relockdoc()', glocktimeoutinmins/3*60*1000)
}
 
function stoprelocker()
{
 window.clearInterval(grelocker)
}

function relockdoc()
{

// if (!gloaded) return false
 
 login('relockdoc')
 
 //dont relock if (already in the process of relocking or something else)
 if (grelockingdoc) return logout('relockdoc')
 if (db.requesting) return logout('relockdoc')
 
 grelockingdoc=true

 db.request='RELOCK\r'+gdatafilename+'\r'+gkey+'\r'+gro.sessionid+'\r'+glocktimeoutinmins
 //document.bgcolor='green'
 if (db.send())
 {
  //document.bgcolor='white'
  result=true
 }
 else
 {
  response=db.response
  result=false
  //document.bgColor='RED'
  
  //lost control so prevent further editing of document
  if (response.toUpperCase().indexOf('EXPIRED')>=0)
  {
   stoprelocker()
   glocked=false
   gchangesmade=false
   saverecord.disabled=true
   releasereopenrecord.disabled=true
   //releasereopenrecord.value='Reopen'
   setgraphicbutton(releasereopenrecord,'Reopen',greopenimage)
   deleterecord.disabled=true
  }
  else
  {
   neosysnote(response)
  }
  
 }

 grelockingdoc=false
 
 logout('relockdoc')

 return result
 
}

//'''''''
//'UNLOCK
//'''''''
function unlockdoc()
{
 
 if (!gKeyNodes) return
 
 login('unlockdoc')

 //always return true even if cannot unlock
 
 //stop the relocker while trying to unlock
 stoprelocker()
 
 if (glocked)
 {
  while (true)
  {
   db.request='UNLOCK\r'+gdatafilename+'\r'+ gkey+'\r'+gro.sessionid
   if (db.send()) break
   if (!neosysyesno('Cannot release document - try again?\r'+db.response)) break
  }
 }
 
 glocked=false
 deleterecord.disabled=true
 saverecord.disabled=true
 //releasereopenrecord.disabled=true
 //releasereopenrecord.value='Reopen'
 setgraphicbutton(releasereopenrecord,'Reopen',greopenimage)

 logout('unlockdoc')
 
 return true
 
}

////////////////////// FIELD FUNCTIONS //////////////////////

function focuson(element)
{
 
 login('focuson')
 
 if (!element) 
  return logout('focuson - no element')

 //convert element name into element 
 //if (typeof(element)=='string'||!element.parentNode)
 if (typeof(element)=='string')
 {
  //element=document.getElementById(element)
  //var element2=document.getElementById(element)

  var element2=document.getElementsByName(element)
  if (grecn&&element2[grecn]) element2=element2[grecn]
  else element2=element2[0]

  if (!element2)
   return logout('focuson - cannot getElementById '+neosysquote(element))
   
  element=element2
 }
 
 //use the first if more than one
 if (typeof(element)=='object'&&!element.tagName&&element.length>0) element=element[0]
 
 log(element.id+' '+element.outerHTML)

 //if already focused on the element then .focus event will not occur so ...
 //NOW will because do .blur() beforehand
 //if (gpreviouselement==null) gpreviouselement=element
 if (false&&document.activeElement==element)
 {
  try
  {
   element.focus()
   if (element.tagName!='TEXTAREA') element.select()
  }
  catch(e){}

  gpreviouselement=element
  gpreviousvalue=getvalue(gpreviouselement)
  setdefault(element)
  log('gpreviouselement and value set to '+gpreviouselement.id+' '+neosysquote(gpreviousvalue))

  try {if (element.tagName!='TEXTAREA') element.select()}catch(e){}
  
 }
 else
 {

  //needed because delete line leaves grecn>nrecs
  grecn=getrecn(element)
  
  //taken out otherwise F7 on job number goes to market code
  // and validateupdate fails because record has not been loaded
  //setdefault(element)

  gfocusonelement=element
  window.setTimeout('focuson2()',10)
 }

 logout('focuson '+element.id)
 
 }

function focuson2()
{
 if (!gfocusonelement) return
 var focusonelement=gfocusonelement
 gfocusonelement=null
 if (!neosysenabledandvisible(focusonelement)) return focusnext(focusonelement)
 try
 {
  focusonelement.blur()
  focusonelement.focus()
  
  //focusing may cause validation that returns focus back to gpreviouselement
  if (document.activeElement!=focusonelement) return
  
  if (focusonelement.tagName!='TEXTAREA')  focusonelement.select()
  
 }
 catch(e){}
 
}

//'''''''''
//'ON FOCUS
//'''''''''
function onfocus()
{

 //this should never occur unless formfunctions_onload failed
 if (!ginitok) return
 
 //get element and elementid
 var element=window.event.srcElement
 var elementid=''
 try
 {
  elementid=window.event.srcElement.id
 }
 catch(e)
 {
  elementid=''
 }
 
 //ensure checkboxes in rows have onclick set
 if (!element.onclick&&element.type=='checkbox'||element.type=='radio')
  element.onclick=onclickradiocheckbox
 
 //ignore calendar focus
 if (element.className=='calendar') return
  
 //if arrived on a readonly field by tab (or enter as tab)
 //then skip forwards (or backwards) to the next field
 if (gkeycode==9)
 {
  //but only if normal tabindex because focusnext doesnt work otherwise
  if (element.neosysreadonly&&element.tabIndex==999)
  {
   if (window.event.shiftKey) focusprevious(element)
   else focusnext(element)
   return false
  }
 }

 //no validation/update except changing neosys elements
 if (!element.neosystype) return false
 
 //login  
 login('onfocus '+neosysquote(elementid))
 
 //quit if opening
 if (gopening)
  return logout('onfocus - gopening')
 
 //drop down any "modal" popup divs
 form_closepopups()
 
 //quit if refocussing
 if (element==gonfocuselement)
 {
//  if (setdefault(element))  if (element.tagName!='TEXTAREA') element.select()
  return logout('onfocus - same as gonfocuselement')
 }
 gonfocuselement=element

 //quit if refocussing
 if (gpreviouselement&&element==gpreviouselement)
  return logout('onfocus - same as gprevious element')

 //dont validate update if clicked popup
 //if (element&&element.id==(gpreviouselement.id+'_popup'))
 // return logout('onfocus - popup')
  
 //validate/update previous data entry
 /////////////////////////////////////
 if (element!=gpreviouselement)
 {
  if (!validateupdate()) return false
 }

 //quit if we are not focused on a proper element
 if (!elementid)
  return logout('onfocus - no element id')

 //ok previous element is valid

 //find what row we are on
 grecn=getrecn(element)
 
 //exit if not input or select
 if (!element.tagName.match(gdatatagnames))
  return logout('onfocus '+element.id+' not an INPUT or SELECT')

 //check key fields
 if (gKeyNodes&&!glocked)
 {
  if (!checkrequired(gKeyNodes,element,0))
   return logout('onfocus '+elementid+' is required but is blank (0)')
 }
  
 //check no required fields are missing in prior data
 //fill in any defaults where possible
 if (!gKeyNodes||glocked)//check keynodes as well now
 {

  //check group 0 always
  if (!checkrequired(gfields,element,0))
   return logout('onfocus '+elementid+' a prior element is visible and required but is blank (1)')
  
  //check specific group if >0
  //var elements
  //if (element.neosysgroupno>0)
  //{
  // elements=getancestor(element,'TR').all
  // if (!checkrequired(elements,element,element.neosysgroupno))
  //  return logout('onfocus '+elementid+' a prior element is visible and required but is blank (2)')
  //}
  
 }
 
 //all previous fields are valid now

 //if the key is complete and not on a key field, get the record
 //if (!gloaded&&gKeyNodes&&element.neosysfieldno!=0)
 if (gKeyNodes)
 {
  var key=getkey()
  if (key.toUpperCase()!=key)
   xxx=1
  if (key&&(gloaded&&key!=gkey)||(!gloaded&&element.neosysfieldno!=0))
  //if (key&&(gloaded&&key!=gkey)||(!gloaded))
  {
   window.setTimeout('opendoc()',100)
   return logout('onfocus '+neosysquote(elementid)+' new record')
  }
 }

 //there is no new record so setup current element
  
 //scroll to top left if the key field
 if (element==gstartelement||element.neosysfieldno==0) window.scrollTo(0,0)

 //if (element.tagName.match(gtexttagnames))  if (element.tagName!='TEXTAREA') element.select()
 
 scrollintoview(element)
 
 //remove blanks used to force formatting of spans
 if (element.tagName=='SPAN'&&element.innerText==' ') element.innerText=''
 
 gpreviouselement=element
 gpreviousvalue=getvalue(gpreviouselement)
 
 //set the default of the current element
 setdefault(element,donotvalidateupdate=true)

 //deselect previous (this does not work unless .select() is used
 //but causes a problem in focussing on the previous element and rescrolling
 if (element.tagName=='SELECT')
 {
  var textrange=document.selection.createRange()
  textrange.collapse()
//  textrange.select()
 }
 
 //select the whole text
 if (element.tagName!='TEXTAREA') try{element.select()}catch(e){}

 //this is not necessary unless .select() is used above
 //note that validation code from previous field may make current element disappear
 try {element.focus()}
 catch(e) {focusnext(element)}
 
 logout('onfocus '+elementid)
 
}

function onclickradiocheckbox()
{
//wstatus(getvalue(event.srcElement.id)+' '+gpreviousvalue)
 //space or arrow keys also simulate a click event
 
 //update immediately
 //if (element.type=='radio') element.checked=true
 //if (element.type=='checkbox') element.checked=!element.checked

 //validate or return to original
 if (!validateupdate())
 {
  setvalue(gpreviouselement,gpreviousvalue)
  return
 }
  
 gpreviousvalue=getvalue(gpreviouselement)

}

function validateupdate()
{
 
 var id=gpreviouselement?gpreviouselement.id:'undefined'
 login('validateupdate '+id)
 
 //return if no changes
 var newvalue
 if (!gpreviouselement||(newvalue=getvalue(gpreviouselement))==gpreviousvalue)
 {
  logout('validateupdate - no changes')
  return true
 }
 
 log(' *** USER/SETDEFAULT CHANGED '+id+' '+neosysquote(gpreviousvalue)+' to '+neosysquote(newvalue))
 
 //check for prior required fields if a grouped element
 var elements
 if (gpreviouselement.neosysgroupno>0)
 {
  elements=getancestor(gpreviouselement,'TR').all
  if (!checkrequired(elements,gpreviouselement,gpreviouselement.neosysgroupno))
  {
   setvalue(gpreviouselement,gpreviousvalue)
   return logout('validateupdate '+id+' a prior element is visible and required but is blank')
  }
 }
  
 //check arguments
 assertelement(gpreviouselement,'validateupdate','gpreviouselement')
 if (!gpreviouselement.tagName.match(gdatatagnames))
 {
  logout('validateupdate - not an INPUT or SELECT')
  return true
 }

 //quit ok if gpreviouselement is no longer bound
 grecn=getrecn(gpreviouselement)
 if (grecn==-1)
 {
  logout('validateupdate - gprevious no longer exists')
  return true
 }
 
 //get the old value (internal format) and grecn
 gcell=gds.getcells(gpreviouselement,grecn)[0]
 goldvalue=gcell.text
 gorigvalue=(typeof gcell.oldtext!='undefined')?gcell.oldtext:gcell.text
 
 //validate it and put back focus if it fails
 ////////////////////////////////////////////
 if (!onbeforeupdate(gpreviouselement))
 {

  //prevent loop since focus doesnt go back to radio button immediately
  //doesnt seem to work though
  //if (gpreviouselement.type=='radio')
  //{
  // gvalue=goldvalue
  // setvalue(element.id,gvalue)
  // gpreviouselement.select()
  //}
  
  log('focusing back to '+id)
  gpreviouselement.focus()
  if (ismac&&gpreviouselement.tagName!='SELECT')  if (gpreviouselement.tagName!='TEXTAREA') gpreviouselement.select()
  try{window.event.cancelBubble=true}catch(e){}

  //try and put cursor at end of gprevious text
  if (!ismac&&gpreviouselement.tagName!='SELECT')
  {
   try
   {
   gpreviouselement.select()
   var textrange=document.selection.createRange()
   textrange.collapse(false)
   textrange.select()
   }
   catch(e){}
  }

  //return false
  return logout('validateupdate - onbeforeupdate failed')
  
 }
 
 //update
 ////////
 
 //update the element multiple selections qqq
 //if reply is an array use replace all rows
 var multiplegvalue=false
 if (typeof(gvalue)=='object')
 {
  if (gvalue.length>1&&gpreviouselement.neosysgroupno>0)
  {
   multiplegvalue=true
  
   var elementid=gpreviouselement.id
   insertallrows(gpreviouselement,gvalue,exceptfirst=true)
   
   //focus on next element AFTER table
   element=document.getElementsByName(elementid)[0]
   focusdirection(1,element,gpreviouselement.neosysgroupno)
   
  }
  else
  {
   gvalue=gvalue[0]
  }  
 }
 if (!multiplegvalue) gds.setrecord(gpreviouselement,grecn,gvalue)
 
 //post update
 /////////////

 //flag record edited
 if (gpreviouselement.neosysfieldno!=0&&!gpreviouselement.neosysnochangeswarning)
 {
  gchangesmade=true
 }
 
 //calculate dependencies
 calcfields(gdependents)
 gdependents=[]
 
 //why is this necessary?
 gpreviousvalue=getvalue(gpreviouselement)
 
 logout('validateupdate - done')
 
 return true
 
}

function earlyupdate()
{
 
 //skip update on buttons
 if (!gpreviouselement||!gpreviouselement.dataFld) return
 
 gds.setrecord(gpreviouselement,grecn,gvalue)
 
}

function checkrequired(elements,element,groupno)
{

 //check the given elements with the given group number
 //and prior to the given field for required
 //and try to set default otherwise return false

 grecn=getrecn(element)
  
 var foundelement=false
 for (var i=0;i<elements.length;i++)
 {
  var element2=elements[i]

  //if given gfields then get the equivalent DOM element
  //group 0 fields are not bound to DOM initially but are in the next paragraph
  //group 1+ fields are never bound to DOM because there are many rows
  if (!element2.parentNode)
  {
   element2=document.getElementById(element2.id)
   if (!element2)
   {
   //no longer check this so that elements can be "unbound" for greater security etc
   //by changing the id to something like id_unbound
//    alert(elements[i].id+' has disappeared from the document\rPerhaps it need to be in its own HTML table')
    continue
   }
   gfields[i]=element2
  }
  
  //only check input fields
  if (!element2.tagName.match(gdatatagnames)) continue

  //don't check current but continue looking for lower tabindexed fields
  //if (element2==element) //this does not work because gfields<> bound table elements for rows
  //if (element2.neosysscreenfieldno==element.neosysscreenfieldno)
  //check ids because neosysscreenfieldno is repeated in gfields for radio 4,4,4
  //but not in the 2nd and subsequent repeated form elements 4,5,6
  if (element2.id==element.id)
  {
   foundelement=true
   continue
  }
  
  //skip elements not in desired group
  //must be after detection of foundelement above
  if (element2.neosysgroupno!=groupno) continue
  
  if (element2.neosysgroupno==groupno)
  {
   if (((!foundelement&&element2.tabIndex<=element.tabIndex)||(element2.tabIndex!=-1&&element2.tabIndex<element.tabIndex)))
   {
    //if (element&&element2.neosysrequired&&gds.getcells(element2,grecn)[0].text=='')
    //if (element&&element2.neosysrequired&&getvalue(element2)=='')
    if (element&&(element.neosysgroupno==0||element2.neosysrequired)&&getvalue(element2)=='')
    {
     
     //try to set the default
     if (!setdefault(element2)&&neosysenabledandvisible(element2))
     {
      focuson(element2)
      return false
     }

     //if still empty then fail
     //if (getvalue(element2)=='')
     if (element2.neosysrequired&&!element2.neosysreadonly&&gds.getcells(element2,grecn)[0].text=='')
     {

      //disabled or invisible elements may be blank and required (even after setdefault)
      if (element2.disabled||!neosysenabledandvisible(element2)) return true
      
	  //put up a message unless is the first column of a row
      //if (!element2.neosysisfirstinputcolumn&&neosysenabledandvisible(element2))
      if (true||!element2.neosysisfirstinputcolumn)
      {
       neosysinvalid(element2.neosystitle+' is required.')
      }
      
      focuson(element2)
      //if (!element2.neosysgroupno) focuson(element2)
      return false
      
     }
    }
   }
  }
 }
 
 return true
 
}

function getvalues(elementx,sepchar)
{

 //return a array of values, or string separated by sepchar if defined
 
 var element=elementx
 if (typeof element=='string') element=document.getElementsByName(element)
 if (typeof element.id!='undefined') element=document.all(element.id)
// else if (typeof element.id!='string') element=document.getElementsByName(element)

 var values=getvalue(element)

 if (typeof(values)=='object'&&typeof(sepchar)!='undefined') values=values.join(sepchar)
 
 return values
 
}

function getvalue(element,recn)
{

 if (element==null)
 {
  systemerror('getvalue()',' element cannot be null')
  return ''
 }
 
// if (recn==null) return getvalues(element)
 
 //return input value or select value(s) ... or undefined
 //returns a string or array of strings if element is array of elements
 
 //element is required and must be an element of type
 // INPUT text
 // INPUT radio
 // INPUT checkbox
 // SELECT
 // SPAN
 
 //get element if given element name
 if (typeof(element)=='string')
 {
  var element0=element
  //if (!(element=document.getElementById(element)))
  //{
  // systemerror('getvalue()','document.getElementById('+neosysquote(element0)+') returned nothing')
  //return
  var elements=document.getElementsByName(element0)
  if (!elements.length)
  {
   systemerror('getvalue()',neosysquote(element0)+' does not exist')
   return
  }
  if (elements[0].neosysgroupno) 
  {
   if (typeof recn=='undefined') recn=getrecn()
  }
  else recn=0
  element=elements[recn]
  if (!element)
  {
   systemerror('getvalue()',neosysquote(element0)+' does not exist')
   return
  }
 }

 //radio/checkbox buttons appear like arrays multiple elements
 //force radio/checkbox to be first element
 if (element[0]&&(element[0].type=='radio'||element[0].type=='checkbox')) element=element[0]

 if (element.length&&element[0].type!='radio')
 {
 
  if (element.tagName)
  {
   //radio buttons appear like arrays multiple elements
   if (element[0].type=='radio'||element[0].type=='checkbox') element=element[0]
  }
  else

  //if passed an array of elements, return an array of values
  //is this used anywhere?
  //NB SELECT element on Mac but not PC has length and options[]
  {
   var values=[]
   for (var i=0;i<element.length;i++)
   {
    values[i]=getvalue(element[i])
   }

  return values

  }
  
 }
 
 //ensure is an element
 assertelement(element,'getvalue','element')
 
 switch(element.tagName)
 {
 
  case 'SELECT': return getdropdown0(element)

  //crlf becomes space to prevent entry of tm characters in text fields
  case 'SPAN': return element.innerText.replace(/([\r\n]+)/g,' ').neosystrimr()

  case 'TEXTAREA': return element.value.neosystrimr()
  
  case 'INPUT':
  {
  
   switch (element.type)
   {
 
    case 'text': return element.value.neosystrimr()
    
    case 'radio':
    {
    
     var doc=element.neosysgroupno?getancestor(element,'TR').all:document.all
     //element=doc.getElementsByName(element.id)
     element=doc[element.id]
     if (!element.length) element=[element]
     
     for (var i=0;i<element.length;i++)
     {
      if (element[i].checked)
      {
       if (typeof(element[i].value)=='undefined')
        return i
       else
        return element[i].value
      }
     }
     //nothing selected
     return ''
    }
  
    case 'checkbox':
    {
    
     var doc=document
     if (element.neosysgroupno) doc=getancestor(element,'TR')
     if (!doc)
     {
      raiseerror(1000,element.id+' has no parentNode in getvalue()')
      return
     }
     //element=doc.getElementsByName(element.id)
     element=doc.all[element.id]
     if (!element.length) element=[element]
     
     var values=[]
     for (var i=0;i<element.length;i++)
     {
      if (element[i].checked)
      {
       values[values.length]=(typeof(element[i].value)=='undefined')?i:element[i].value
      }
     }
//     alert(values.join())
     //nothing selected
     //wstatus(new Date+' getvalue '+values.join(sm))
//     window.status=values.join(sm2)
     return values.join(sm2)
     
    }
  
    case 'button':
    {
     return ''
    }
    
    default:
    {
     raiseerror(1000,neosysquote(element.type)+' invalid INPUT element type in getvalue()')
     return
    }
    
 }
 
  }
  
  default:
  {
    raiseerror(1000,neosysquote(element.tagName)+' invalid tagName in getvalue()')
    return
  }
 }
 
}

function neosyssetreadonly(elements,msg,options,recn)
{

 //note: cannot set readonly off if the dictionary says that it is readonly
  
 //if msg contains something then set the element(s) to readonly and vice versa
 //tabbing and cursor keys will skip past readonly fields
 //msg will be displayed if they click and try and change the field
 
 var elementx=elements
 
 //if elements is a number then it means set all one group (for selected recn or null for all)
 if (!isNaN(+elementx))
 {
  var elements=[]
  for (var i=0;i<gfields.length;i++)
  {
   if (gfields[i].neosysgroupno==elementx&&gfields[i].neosysfieldno>0)
   {
    if (!neosyssetreadonly(gfields[i].id,msg,options,recn)) return false
   }
  }
  return true
 }
 
 //if elements is a simple string then get all the elements for it
 if (typeof elementx=='string')
 {
  var elementxstring=elementx
  elementx=document.getElementsByName(elementx)
  if (!elementx||elementx.length==0)
  {
   //only give error if it is not even in the dictionary
   //so that we can remove fields from the screen without changing the setreadonly field lists
   if (!gds.dictitem(elementxstring)) return systemerror('neosyssetreadonly()',neosysquote(elements)+' is not in the form')
   return false
  }
  
  //handle arrays of recn recursively
  //handles arrays of fieldnames and recns efficiently when items are string ids
  //since extraction of all elements for one element id is probably time consuming
  if (typeof recn=='object'&&recn.length)
  {
   for (var ii=0;ii<recn.length;ii++)
   {
    if (!neosyssetreadonly(elementx[recn[ii]],msg,options)) return false
   }
   return true
  }
 
  if (typeof recn!='undefined'&&recn!=null&&recn!='')
  {
   elementx=elementx[recn]
   if (typeof elementx=='undefined')
   {
    return systemerror('neosyssetreadonly('+elements+','+msg+','+recn+')')
   }
   elementx=[elementx]
  }
  if (elements.length==1) elements=elementx[0]
 }
 
 //handle arrays recursively
 if (typeof elementx=='object'&&elementx.length&&!elementx.dataFld)
 {
  for (var i=0;i<elementx.length;i++)
  {
   if (!neosyssetreadonly(elementx[i],msg,options,recn)) return false
  }
  return true
 }
 
 //from here on we are handling one element
 
 if (!options) options=''
 
 //skip dict items that are marked readonly
 //in chart of accounts dictitem is not available
 if (gds.dictitem)
 {
  var di=gds.dictitem(elementx.id)
  if (!di||di.readonly) return true
 }
 
 if (msg)
 {
  elementx.neosysreadonly=msg
  if (elementx.id!=document.activeElement.id)
  {
   //remove tabindex (unless we are on the element otherwise tab key doesnt work anymore!)
   elementx.oldtabindex=elementx.tabIndex
   elementx.tabIndex=-1
  }
  if (elementx.onchange) elementx.oldonchange=elementx.onchange
  elementx.onchange=readonly_onchange
  
  if (options&&options.indexOf('BGCOLOR')>=0)
  {
   if (!elementx.oldbgcolor) elementx.oldbgcolor=elementx.style.backgroundColor
   //elementx.style.backgroundColor='FFFFC0'
   elementx.style.backgroundColor=document.styleSheets[0].rules[0].style.backgroundColor
   
   //spans have no type
   if (!elementx.type||!elementx.type.match(gradiocheckboxtypes))
   {
    if (!elementx.oldborderwidth) elementx.oldborderwidth=elementx.style.borderWidth
    elementx.style.borderWidth='1px'
   
    if (!elementx.oldborderstyle) elementx.oldborderstyle=elementx.style.borderStyle
    elementx.style.borderStyle='solid'
   
    if (!elementx.oldbordercolor) elementx.oldbordercolor=elementx.style.borderColor
    elementx.style.borderColor='DDDDDD'
   }
   
  }
  
  if (options&&options.indexOf('BORDER')>=0)
  {
   if (!elementx.oldborderstyle) elementx.oldborderstyle=elementx.style.borderStyle
   elementx.style.borderStyle='none'
  }
  
 }
 else
 {
  elementx.removeAttribute('neosysreadonly')
  if (elementx.oldonchange) elementx.onchange=elementx.oldonchange
  else
  {
   if (elementx.onchange)
   {
    //elementx.removeAttribute('onchange')
    //does not seem to work on functions so ...
    elementx.onchange=null
   }
  }
  if (elementx.oldtabindex)
  {
   elementx.tabIndex=elementx.oldtabindex
   elementx.oldtabindex=''
  }
  if (typeof elementx.oldbgcolor!='undefined') elementx.style.backgroundColor=elementx.oldbgcolor
  if (typeof elementx.oldborderstyle!='undefined') elementx.style.borderStyle=elementx.oldborderstyle
 }
 
 //move onto next field if setting current focus field to readonly
 //but only if default tabindex since focusnext cant find the next tabindex properly
 if (document.activeElement.neosysreadonly&&gpreviouselement&&document.activeElement.tabIndex==999) focusnext(gpreviouselement)

 return true
  
}

function readonly_onchange()
{
 //called when user starts to change a readonly element
 
 var element=window.event.srcElement
 if (!element.neosysreadonly) return
 neosysinvalid(element.neosysreadonly)
 window.event.cancelBubble=true
 window.event.returnValue=false
 setvalue(gpreviouselement,gpreviousvalue)
 return false
 
}

function setvalue(element,valueorvalues)
{

 //given an element (array or first element for radio/checkbox buttons)
 //set the value of INPUT text/INPUT radio/INPUT checkbox/SPAN/SELECT elements

 if (!element)
 {
  systemerror('setvalue()','required argument "element" is missing')
  return
 }

 //convert element name to element
 var elementid=element.id
 if (typeof element=='string')
 {
  var elementid=element
  //element=document.getElementById(element)
  element=document.getElementsByName(element)[0]
  if (!element)
  {
   systemerror('setvalue',neosysquote(elementid)+' element does not exist')
   return
  }
 }
  
 login('setvalue '+elementid+' '+valueorvalues)
 
 setvalue2(element,valueorvalues)
 
 logout('setvalue '+elementid+' '+valueorvalues)
 
}

function setvalue2(element,value)
{

 //unprotected (faster) core of setvalue()
 value=value.toString()
 
 switch (element.tagName)
 {
  case 'INPUT':
  {
   
   switch (element.type)
   {
    
    case 'text':
    {
     setneosyslink(element,value)
     element.value=value
     break
    }
    
    case 'radio':
    {
     var doc=element.neosysgroupno?getancestor(element,'TR').all:document.all
     //element=doc.getElementsByName(element.id)
     element=doc[element.id]
     if (!element.length) element=[element]
     
     for (var i=0;i<element.length;i++)
     {
      if (value.neosyslocate(element[i].value.toString())>0)
      {
       element[i].checked=true
       value=element[i].value
       if (value=='undefined') value=i
      }
      else
      {
       element[i].checked=false
      }
     }
     break
     
    }
    
    case 'checkbox':
    {
    
     //multiple values as : or , separated string
     if (typeof value=='string'&&value.indexOf(':')>=0) value=value.split(':')
     if (typeof value=='string') value=value.split(sm2)

     //go through all the check box elements
     var doc=element.neosysgroupno?getancestor(element,'TR').all:document.all
     //element=doc.getElementsByName(element.id)
     element=doc[element.id]
     if (!element.length) element=[element]
          
     for (var i=0;i<element.length;i++)
     {
      if (value.neosyslocate(element[i].value.toString())>0)
      {
       element[i].checked=true
//       value=element[i].value
//       if (value=='undefined') value=i
      }
      else
      {
       element[i].checked=false
      }
     }
     break
     
    }
    
   }
   
   break//INPUT
  }
  
  case 'TEXTAREA':
  {
   setneosyslink(element,value)
   element.value=value
   break
  }
  
  case 'SPAN':
  {
   setneosyslink(element,value)
   if (value=='') value='&nbsp;'
   element.innerHTML=value
   break
  }
  
  case 'SELECT':
  {
   setdropdown3(element,null,'',value)
   break
  }
  
  default:
  {
    raiseerror(1000,neosysquote(element)+' '+neosysquote(element.tagName)+' invalid tagName in setvalue2('+element+','+value+')')
    return
  }
  
 }//of switch
 
}

function setneosyslink(element,value)
{
 return//always display now
 if (element.neosyslink)
 {
  if (element.previousSibling&&element.previousSibling.isneosyslink)
  {
   element.previousSibling.style.display=value?'inline':'none'
  }
  else if (element.nextSibling&&element.nextSibling.isneosyslink)
  {
   element.nextSibling.style.display=value?'inline':'none'
  }
 }
}

function getdefault(element)
{

 //return default value string or ''

 //make sure is element
 if (typeof element=='string') element=document.getElementById(element)
 assertelement(element,'getdefault','element')
 
 //default value of a select item is the selected item if not already defaulted
 //suppress otherwise cannot have "" value except as first item eg (X;XXXX:;YYYY)
 //if (!element.neosysdefaultvalue&&element.tagName=='SELECT')
 //{
 // login('getdefault '+element.id)
 // var defaultvalue=getvalue(element)
 // logout('getdefault '+element.id+' SELECT '+defaultvalue)
 // return defaultvalue
 //}
 
 //return '' if no default
 if (!element.neosysdefaultvalue) return ''
 
 login('getdefault '+element.id)
 
 //calculate default
 var defaultvalue=neosysevaluate(element.neosysdefaultvalue,'getdefault('+element.id+')')

 //select elements always have a default
 if (element.tagName=='SELECT')
 {
  if (defaultvalue==''&&element.tagName=='SELECT') defaultvalue=getvalue(element) 
 }
 
 //don't default if unique and already present
 if (defaultvalue&&element.neosysunique)
 {
  var othervalues=getvalues(element.id)
  if (othervalues.neosyslocate(defaultvalue)) defaultvalue=''
 }
  
 //convert numbers to strings and check is a string
 if (typeof defaultvalue=='number') defaultvalue=defaultvalue.toString() 
 if (typeof defaultvalue!='string')
 {
  neosysinvalid(element.id+' default returned is '+typeof defaultvalue+' - "" used')
  defaultvalue=''
 }
 
 logout('getdefault '+element.id+' '+defaultvalue)
 return defaultvalue

}

function setdefault(element,donotupdate)
{

 //used in
 //1. newrecordfocus and cleardoc to point to 1st element
 //2. focuson (why? should be setvalue?)
 //3. onfocus at end
 //4. checkrequired
 
 //if no value, get default and set value

 if (typeof element=='string') element=document.getElementById(element)
 
 //cannot update anything but key field if not locked or save button not enabled
 if (element.neosysfieldno!=0&&gKeyNodes&&(!glocked||saverecord.disabled))
 {
//  return true
 }
 
 //return true if already defaulted

 if (element.dataitem)
 {
  if (element.dataitem.defaulted) return true
 }
 else
 {
  //only the first element of radio and checkboxes are bound to data
  //should really check if the first element is already defaulted
  if ('checkbox radio'.indexOf(element.type)>=0) return true
 }

 //return true if already has a value
 //var value=gds.getcells(element,grecn)[0].text<A HREF="http://localhost/neosys7/NEOSYS/images/">http://localhost/neosys7/NEOSYS/images/</A>
 var value=getvalue(element)
 if (value) return true
 
 //signal that it has been defaulted (even if there is no default)
 if (element.dataitem) element.dataitem.defaulted=true
 
 login('setdefault '+element.id)
 
 //get the default value
 var storegrecn=grecn
 grecn=getrecn(element)
 gdefault=getdefault(element) 
 grecn=storegrecn
 
 //for select elements with no default, the first element is the default
 //is this not done in the binding routine now?
 if (gdefault==''||gdefault==null||typeof(gdefault)=='undefined')
 {
  if (element.childNodes.length) gdefault=element.childNodes[0].value
 }
 
 //if no default return
 if (!gdefault)
 {
  logout('setdefault '+element.id+' no default')
  return true
 }
 
 //save the default for use eg by custom validation routines to avoid work
 element.neosysdefault=gdefault
 
 //set the value (externally only)
 //gds.setrecord(element,grecn,gdefault)

 setvalue(element,oconvertvalue(gdefault,element.neosysconversion))
 
 //call the validate/update routine
 if (!donotupdate)
 {
 
  //changed to not mess with gpreviouselement
  savegpreviouselement=gpreviouselement
  savegpreviousvalue=gpreviousvalue

  gpreviouselement=element
  gpreviousvalue=''//gpreviousvalue=value
 
  if (!validateupdate())
  {
   logout('setdefault '+element.id+' invalid')
   return false
  }
  
  //restore
  if (savegpreviouselement!=gpreviouselement)
  {
   gpreviouselement=savegpreviouselement
   gpreviousvalue=savegpreviousvalue
  }

 }
 
 log(element.id+' defaulted to '+gdefault)
 
 logout('setdefault '+element.id+' '+neosysquote(gdefault))
 
 return true
 
}

//''''''''''''''
//'BEFORE UPDATE
//''''''''''''''
function onbeforeupdate(element)
{
 //move to validateupdate?
 
 //ignore updates while closing
 if (gclosing) return false
 
 login('onbeforeupdate')
 
// if (typeof(element)=='undefined') element=window.event.srcElement
 
 //get grecn to be safe
 grecn=getrecn(element)

 //skip if value not changed
 var value=getvalue(element)
 if (value==gpreviousvalue)
 {
  logout('onbeforeupdate')
  return true
 }

 //cannot update anything but key field if not locked or save button not enabled
 //if (element.neosysfieldno!=0&&gKeyNodes&&(!glocked||saverecord.disabled))
 if (element.neosysfieldno!=0&&gKeyNodes&&(!glocked))
 {
  setvalue(gpreviouselement,gpreviousvalue)
  readonlydocmsg()
  logout('onbeforeupdate')
  return neosysinvalid('')
 }

 //post entry processing 
 if (!validate(element))
 {
  //this should already have been done in the validate routine
  //element.focus()
  return logout('onbeforeupdate')
 }
   
 logout('onbeforeupdate')
 
 return true
 
}

function validate(element)
{

 //new value
 gvalue=getvalue(element).toString()
 
 login('validate '+element.id+' '+gvalue)
 
 //extract other infor
 var datafld=element.dataFld

 //extract global info

 //group number 
 ggroupno=element.neosysgroupno
 
 grecn=getrecn(element)

log('grecn='+grecn)

 //old value
 goldvalue=gds.getcells(element,grecn)
 if (goldvalue!=null) goldvalue=goldvalue[0].text
 
 //convert to uppercase
 if (!element.neosyslowercase&&element.type!='radio'&&element.type!='checkbox')
  gvalue=gvalue.toUpperCase()
 
 if (element.tagName!='SELECT')
 {

  //invalid character check (nearly all punctuation except - / and #)
  if (element.neosysinvalidcharacters)
  {
  
   var invalidcharacters=element.neosysinvalidcharacters+'\xF8\xF9\xFA\xFB'//\xFC\xFD\xFE\xFF'

   //ignore any valid characters
   if (element.neosysvalidcharacters) invalidcharacters=invalidcharacters.neosysconvert(element.neosysvalidcharacters,'')
  
  }
  else
  {
   var invalidcharacters='\xF8\xF9\xFA\xFB'//\xFC\xFD\xFE\xFF'
  }
  
  var temp=gvalue.neosysconvert(invalidcharacters,'')
  if (temp!=gvalue)
  {
 //   alert('Punctuation characters and spaces (except / - and #) are not allowed in key fields and have been removed\n')
    neosysinvalid('The following characters are not allowed here.\n'+invalidcharacters.neosysswap(' ',' space '))
    return logout('validate')
  }
  
  //valid character check
  if (element.neosysvalidcharacters)
  {
  
   var temp=gvalue.neosysconvert(element.neosysvalidcharacters,'')
   if (temp!='')
   {
    neosysinvalid('Only the following characters are allowed here.\n\"'+element.neosysvalidcharacters+'\"')
    return logout('validate')
   }
  }
  
 }
  
 //required check
 log('required check')
 if (element.neosysrequired=='false') element.neosysrequired=false
 if(gvalue==''&&element.neosysrequired&&neosysenabledandvisible(element))
 {
  neosysinvalid(element.neosystitle+' is required...')
  return logout('validate')
 }
 log('before file check')
 //file check (skip if has dropdown)
 if (element.neosysfilename&&gvalue!=''&&!element.neosysdropdown)
 {
 
  var filename=element.neosysfilename
  var key=gvalue

  //neosys hack (possibly not used anymore as .filename='ACCOUNTS' not used?
  if (filename=='ACCOUNTS') key='.'+gvalue
  
  db.request='CACHE\rREAD\r'+filename+'\r'+key
  if (!db.send())
  {
  
   if (db.response.indexOf('NO RECORD')>=0) db.response=neosysquote(gvalue)+' '+element.neosystitle+' is not on file.'
   
   neosysinvalid(db.response)
   
   return logout('validate - not on file '+gvalue)
   
  }
  
 }

 log('before conversion')
 
 var gvaluebeforeiconv=gvalue
 
 //if conversion is a routine. eg [NUMBER] [DATE] are standard
 var conversion=element.neosysconversion
 if (typeof(conversion)!='string'||conversion.slice(0,1)!='[') conversion=false

 //input conversion
 if (conversion&&gvalue!='')
 {
 
  //format is '[functionname,options]'
  var convarray=conversion.slice(1,-1).split(',')
  //if no options, make them ''
  if (convarray.length==1) convarray[1]=''
  
  //see also OCONV below

  var quotechar=(gvalue.indexOf('"')>=0)?"'":'"'
  var expression=convarray[0]+'('+'"ICONV",'+quotechar+gvalue.replace(/\\/g,'\\\\')+quotechar+',"'+convarray.slice(1)+'")'
  gmsg=''
  ivalue=neosysevaluate(expression,'validate('+element.id+') iconv')
  if (typeof ivalue=='undefined')
   return logout('validate - system error in input conversion')
  
  //null means failed to convert to internal value therefore invalid
  if (gvalue==null||ivalue==null)
  {
   //error message (use the conversion program name in the message)
   neosysinvalid(neosysquote(gvalue)+' is not a valid '+convarray[0].toLowerCase().replace(/_/g,' ')+'\r\r'+gmsg)
   return logout('validate - input conversion returned null')
  }

  //switch the data into internal format
  gvalue=ivalue
  
 }
 
 //custom validation - data in internal format
 log('before custom validation')
 var storegrecn=grecn
 //if (element.neosysvalidation&&!eval(element.neosysvalidation))
 if (element.neosysvalidation)
 {
  var ok
  if (typeof element.neosysvalidation=='function')
   ok=element.neosysvalidation()
  else
  {
   ok=neosysevaluate(element.neosysvalidation,'validate() functioncode')
  }
  if (gvalue==null) neosyswarning(element.id+' validation routine returned gvalue=null')
  if (!ok||gvalue==null)
  {
   grecn=storegrecn
   neosysinvalid()
   return logout('validate - validation function code returned false or gvalue as null')
  }
  
 }
 
 //get grecn again in case any prior lines have been deleted in the validation routine
 //grecn=storegrecn
 grecn=getrecn(gpreviouselement)
 
 //check for uniqueness for multivalues
 log('before unique check')
 if (gvalue&&ggroupno>0&&element.neosysunique)
 {
  var othervalues=gds.calcx(element.id,null)
  var ln
  //othervalues[grecn]='' //not needed because only validate if changed
  othervalues[grecn]='' //put back because of a validation after a multiple choice popup fails
  if (ln=othervalues.neosyslocate(gvalue))
  {
   gmsg=neosysquote(gvaluebeforeiconv)+' is already used in line '+ln+'.'
   if (element.neosysnonuniquewarning)
   {
    if (!confirm('Warning:\n\n'+gmsg,1))
    {
     logout('validate - not unique warning')
     return neosysinvalid()
    }
   }
   else
   {
    logout('validate - not unique')
    return neosysinvalid(gmsg)
   }
  }
 }
 
 //check for sequential multivalues
 //assumes that numbers will be in number format
 if (gvalue&&ggroupno>0&&element.neosyssequence)
 {
  var title=element.neosystitle
  if (element.neosyssequence=='A')
  {
   var temp
   if ((temp=getpreviousrow('',true))&&gvalue<temp)
   {
//    alert(typeof gvalue+' '+gvalue+' < '+typeof temp+' '+temp)
    logout('validate - not sequential')
    return neosysinvalid(title+' cannot be less than '+title+' in the previous row above')
   }
   var temp
   if ((temp=getnextrow('',true))&&gvalue>temp)
   {
//    alert(typeof gvalue+' '+gvalue+' > '+typeof temp+' '+temp)
    logout('validate - not sequential')
    return neosysinvalid(title+' cannot be greater than '+title+' in the next row below')
   }
  }
 }
 
 //output conversion
 log('before output conversion')
 
 ovalue=validateoconv(element,gvalue)
 if (ovalue=='undefined'||ovalue==null)
  return logout('validate - oconv failed')

 //not needed because setrecord will update the screen with oconverted data
 //gvalue=ovalue
  
 logout('validate '+element.id+' '+gvalue)
 
 return true
 
}

function validateoconv(element,ivalue)
{
  
 //returns ovalue or null if oconv fails
  
 //skip if nothing to convert
 ivalue=ivalue.toString()
 var ovalue=ivalue
 if (ovalue=='') return ovalue
 
 //if conversion is a routine. eg [NUMBER] [DATE] are standard
 var conversion=element.neosysconversion
 if (typeof conversion!='string'||conversion.slice(0,1)!='[') conversion=false
 if (!conversion) return ovalue
 
 //format is '[functionname,options]'
 var convarray=conversion.slice(1,-1).split(',')
 //if no options, make them ''
 if (convarray.length==1) convarray[1]=''
  
 //convert internal to external format
 //nb cannot cancel after updating element.value
 var quotechar=(ivalue.indexOf('"')>=0)?"'":'"'
 var expression=convarray[0]+'('+'"OCONV",'+quotechar+ivalue.replace(/\\/g,'\\\\')+quotechar+',"'+convarray[1]+'")'
 gmsg=''
 var ovalue=neosysevaluate(expression)
 if (typeof ovalue=='undefined')
 {
  return false
 }
 
 //null means failed to convert to external value therefore invalid
 if (ovalue==null)
 {
  neosysinvalid(neosysquote(ivalue)+' is not a valid '+convarray[0].toLowerCase()+'\r'+gmsg)
  return false
 }
 
 return ovalue
 
}

function calcfields(fieldns)

//given an array of field numbers calculate and set their contents

{
 login('calcfields')
 
 if (gKeyNodes&&!gloaded)
  return logout('calcfields no record')

 //if list is empty then do all calculated fields
 //exclude real fields (type=F) that have functioncode specified
 var origfieldns=fieldns
 if (fieldns==null)
 {
  fieldns=[]
  var field
  for (var fn=0;fn<gfields.length;fn++)
  {
   var field=gfields[fn]
   if (!field)
   {
    systemerror('calcfields()','gfields['+fn+'] is undefined.')
   }
   else
   {
    if (field.neosysfunctioncode&&field.neosystype!='F')
    {
     fieldns[fieldns.length]=fn
    }
   }
  }
 }
 
 //convert fieldns to an array if necessary
 if (typeof(fieldns)!='object') fieldns=[fieldns]
 
 for (var fn=0;fn<fieldns.length;fn++)
 {

  if (fieldns[fn]!=''||typeof fieldns[fn]=='number')
  {
   
   var field=gfields[fieldns[fn]]

   //only do calculated fields
   if (field.neosysfunctioncode)
   {
   
    //add dependents of dependents to the list to recalc
    if (field.neosysdependents)
    {
     var deps=field.neosysdependents.split(';')
     for (var depn=0;depn<deps.length;depn++)
     {
      //prevent recursion by limiting to 1000 fields
      if (fieldns.length<1000) fieldns[fieldns.length]=deps[depn]
     }
    }
    
    //recalculate and update record
    gds.recalcx(field.id)
    
   }
  }
 }

if (typeof origfieldns=='undefined') gdependents=[]

logout('calcfields')

}

//version to make grecn null and restore it afterwards
function neosysevaluate2(functioncode,callerfunctionname)
{
 var storegrecn=grecn
 grecn=null
 var result=neosysevaluate(functioncode,callerfunctionname)
 grecn=storegrecn
 return result
}

function neosysevaluate(functioncode,callerfunctionname)
{

 //wrapper to call custom functions or expressions (eg validation etc)
  
 //returns result of neosysevaluate
 //displays message if the function does not return something and returns ''
 //failure results in error message and returns undefined!!!
 // or if gstepping then dump to de-bugger
 
 var result

 if (typeof functioncode=='undefined')
 {
  systemerror('neosysevaluate()','The required argument "functioncode" is missing. Called from\r'+callerfunctionname)
  if (gstepping||gusername=='NEOSYS') crashhere
  return
 }
 
 //boolean just gets returned
 if (typeof functioncode=='boolean') return functioncode
 
 //special code to return today's date 
 if (typeof functioncode=='string'&&functioncode=='TODAY') functioncode='neosysdate()'
 
 //if (gstepping||(!ginitok&&gusername=='NEOSYS'))
 // if (true||gstepping||gusername=='NEOSYS')
 if (gstepping||gusername=='NEOSYS')
 {
  result=typeof functioncode=='function'?functioncode():eval(functioncode)
 }
 
 else
 {
  var e
  try
  {
   result=typeof functioncode=='function'?functioncode():eval(functioncode)
  }
  catch (e){}
  if (e)
  {
   if (typeof callerfunctionname=='undefined') callerfunctionname='"not specified"'
   systemerror('neosysevaluate()','Error No: '+e.number+'\rDescription: '+e.description+'\r\rin '+callerfunctionname+'\r\r'+functioncode.toString().substr(0,50))
   neosysinvalid()
   return
  }

 }
 
 //check that the function returned something
 if (typeof result=='undefined')
 {
  neosysinvalid('Error in'+'\r'+functioncode.toString().substr(0,500))
  systemerror('neosysevaluate()','Function returned "undefined"\rCalled from '+callerfunctionname+'\r\r'+functioncode.toString().substr(0,500))
  result=''
 }
  
 return result
 
}

function getrecord(elementname,groupn,recn)
{

 //if not used anywhere by say dec 2001 then delete
 
 //not really necessary as calcx does the same?
 
 if (typeof groupn=='undefined') groupn=0
 
 var rec
 var element
 if (!groupn)
 {
  element=gds.data[elementname]
 }
 else
 {
  if (typeof recn=='undefined')
  {
   systemerror('getrecord()','recn is undefined. (element:'+elementname+')')
   return ''
  }
  rec=gds.data['group'+groupn]
  if (!rec)
  {
   systemerror('getrecord()','group '+groupn+' not in gds.data (element:'+elementname+')')
   return ''
  }
  rec=rec[recn]
  if (!rec)
  {
   systemerror('getrecord()','recn '+recn+' is missing in gds.data[group'+groupn+' (element:'+elementname+')')
   return ''
  }
  
 }

 if (!element)
 {
  systemerror('getrecord()',elementname+' is missing from gds.data['+groupn+']['+recn+'] (element:'+elementname+')')
  return ''
 }
  
 if (typeof element.text=='undefined')
 {
  systemerror('getrecord()','.text property is missing from gds.data['+groupn+']['+recn+'].'+elementname)
  return ''
 }
  
 return element.text
  
}

function oconvertvalue(ivalue,conversion)
{
 if (!conversion) return ivalue
 if (typeof(conversion)!='string'||conversion.slice(0,1)!='[') return ivalue
 return ivalue.neosysoconv(conversion)
}

//'''''''''''
//'DELETE ROW
//'''''''''''
function deleterow(element)
{

 login('deleterow')
 
 if (!element)
 {
  element=window.event.srcElement
  //fast key repeats generates delete on the table element which can be ignored
  if (element.tagName=='TABLE') return 
 }
 var row=getancestor(element,'TR')
 var tablex=getancestor(row,'TABLE')
 var rows=tablex.tBodies[0].childNodes
 var groupno=tablex.neosysgroupno

 if (groupno==0) return logout('deleterow group 0')
 
 //does not work on mac ie5 ... var rown=row.sectionRowIndex
 var rown=row.rowIndex
 if (tablex.tHead) rown-=tablex.tHead.rows.length
 
 //make sure any previous data entered is valid EXCEPT if on the current line benig deleted
 if (gpreviouselement
 &&(gpreviouselement.neosysgroupno!=groupno||getrecn(gpreviouselement)!=rown)
 &&!validateupdate()) return logout('deleterow gprevious invalid')
 
 //cannot delete if keyed and not locked
 if (gKeyNodes&&!glocked)
 {
  readonlydocmsg()
  return logout('deleterow')
 }
 
 //alert('rown='+rown)
 var nrows=rows.length
 //var grecn=row.recordNumber-1
 
 //zzz should be offset when paging
 grecn=rown
   
 //form specific before row delete function
 var predeleterow
 try {predeleterow=eval('form_predeleterow'+groupno)}catch (e){}
 if (typeof predeleterow=='function')
 {
  if (!neosysevaluate('form_predeleterow'+groupno+'()'))
   return logout('deleterow - predelete false')
 }

 //committed from here on
  
 gpreviouselement=null
 gpreviousvalue=''
 
 gchangesmade=true
 //saverecord.disabled=false
 //releasereopenrecord.disabled=false
 //saverecord.style.display='inline'

 var pagesize=tablex.dataPagesize?tablex.dataPagesize:999999
 var pagen=neosysint(grecn/pagesize)
 
 var id
 if (window.event.srcElement.dataFld)
  id=window.event.srcElement.id
 else
  id=gfields[gtables[groupno][0]].id
 
 gds.deleterow(groupno,rown)
  
 //focus on first column of new current row
 if (rown>0&&rown==(nrows-1)) rown--//if deleting last row then focus on previous row
 rows=tablex.tBodies[0].childNodes
 var element=rows[rown].all(id)

 focuson(element)
 gpreviouselement=element
 gpreviousvalue=getvalue(gpreviouselement)
 
 //form specific after row delete function
 var postdeleterow=''
 try {postdeleterow=eval('form_postdeleterow'+groupno)}catch(e){}
 if (typeof postdeleterow=='function')
 {
  neosysevaluate('form_postdeleterow'+groupno+'()')
 }
 
 calcfields(tablex.neosysdependents.split(';'))
 
 logout('deleterow')
 
}

function deleteallrows(groupnoorelement,exceptfirst)
{

 login('deleteallrows')
 
 var groupno=groupnoorelement
 if (typeof groupno=='object')groupno=groupno.neosysgroupno
 
 //insert a blank row
 if (!exceptfirst) gds.insertrow(groupno,0)
 
 //delete all rows but the newly inserted first row
 //var recs=gds.data['group'+groupno]
 var i=0
 while (gds.data['group'+groupno].length>1)
 {
  //delete the last row
  gds.deleterow(groupno,gds.data['group'+groupno].length-1)
  log(++i)
 }
 
 logout('deleteallrows')
 
}

function insertallrows(elements,values,exceptfirst)
{

 login('insertallrows')

 //get first element if an array passed
 //otherwise convert elements to an array
 var element=elements
 if (!element.tagName&&element.length&&element[0].tagName) element=element[0]
 else elements=[elements]

 assertelement(element,'insertallrows','element')
 
 var groupno=element.neosysgroupno
 
 deleteallrows(element,exceptfirst)

 //get the group
 var rows=gds.data['group'+groupno]
  
 for (var rown=0;rown<values.length;rown++)
 {
 
  //insert a new row (except for the last)
  if (rown<(values.length-1)) gds.insertrow(groupno,rown)

  var rowvalues=values[rown]
  if (typeof rowvalues!='object') rowvalues=[rowvalues]

  //set the record and form values
  for (var coln=0;coln<elements.length&&coln<rowvalues.length;coln++)
  {
   gds.setrecord(elements[coln],rown,rowvalues[coln])
  }

 }

 gchangesmade=true
 
 //recalculate any dependents
 var dependentfieldnos=''
 for (var ii=0;ii<elements.length;ii++)
 {
  var temp=elements[ii].neosysdependents
  if (temp) dependentfieldnos=';'+temp
 }
 dependentfieldnos=dependentfieldnos.slice(1).split(';')
 grecn=null
 calcfields(dependentfieldnos)
// var tablex=getancestor(element,'TABLE')
// calcfields(tablex.neosysdependents.split(';'))

 logout('insertallrows')
 
}

function neosysaddrow(groupno)
{
 var tablex=document.getElementById('neosysgroup'+groupno)
 if (tablex&&tablex.noinsertrow) return false
 gds.addrow(groupno)
}
 
function readonlydocmsg()
{
 return neosysinvalid('This document is currently "read only"')
}

//'''''''''''
//'INSERT ROW
//'''''''''''
function insertrow()
{

 //make sure any previous stuff is validatedupdated
 if (!validateupdate()) return false
 
 login('insertrow')
 
 //insert blank row does not indicate any data entered
 //gchangesmade=true
 
 //saverecord.disabled=false
 //releasereopenrecord.disabled=false
 //saverecord.style.display='inline'
 
 var row=getancestor(window.event.srcElement,'TR')
 
 var tablex=getancestor(row,'TABLE')
 var tbodies=tablex.tBodies
 var groupno=tablex.neosysgroupno 
 if (groupno==0) return false
 
 var rown=row.rowIndex
 if (tablex.tHead) rown-=tablex.tHead.rows.length
 grecn=rown

 var trows=tbodies[0].childNodes
 var nrows=trows.length

 //if following row is hidden then expand following hidden rows INSTEAD of inserting
 if (grecn<(nrows-1)&&trows[rown+1].style.display=='none')
 {
 
  //return to insertrow image
  setinsertimage('insert',trows[grecn],groupno)
  
  for (rown=grecn+1;rown<nrows;rown++)
  {
   if (trows[rown].style.display!=='none') return true
   trows[rown].style.display='inline'
  }
  return true
 }
 
 //cannot update if (locked
 if (gKeyNodes&&!glocked)
 {
  readonlydocmsg()
  return logout('insertrow')
 }
 
 //form specific before row insert function
 var preinsertrow=''
 try {preinsertrow=eval('form_preinsertrow'+groupno)}catch(e){}
 if (typeof preinsertrow=='function')
 {
  if (!neosysevaluate('form_preinsertrow'+groupno+'()'))
   return logout('insertrow - preinsert false')
 }
 
 var nrows=tbodies[0].childNodes.length
 var pagesize=tablex.dataPagesize?tablex.dataPagesize:999999
 var pagen=neosysint(grecn/pagesize)
 
 var id
 if (window.event.srcElement.dataFld)
  id=window.event.srcElement.id
 else
  id=gfields[gtables[groupno][0]].id
 
 gchangesmade=true
 
 gds.insertrow(groupno,rown)

 var row=tbodies[0].childNodes[rown]
 
 //focus on first column of new row
 focuson(row.all(id))
 
 //ensure not copied a row with expand marker
 setinsertimage('insert',row,groupno)
 
 //form specific after row insert function
 var postinsertrow=''
 try {postinsertrow=eval('form_postinsertrow'+groupno)}catch(e){}
 if (typeof postinsertrow=='function')
 {
  neosysevaluate('form_postinsertrow'+groupno+'()')
 }
 
 logout('insertrow')
 
}

function setinsertimage(mode,row,groupno)
{

 //return to insertrow graphic zzz hardcoded should be located
 //var insertimage=row.childNodes[0].childNodes[0].childNodes[0]
 var insertimage=row.childNodes[0].all('insertrowbutton'+groupno)
 if (!insertimage) return
 
 if (mode=='expand')
 {
  //insertimage.src='../neosys/images/smallexpand.gif'
  insertimage.src=gexpandrowimage
  insertimage.alt='Expand hidden rows here (Ctrl+N)'
 }
 else
 {
  //insertimage.src='../neosys/images/smallinsert.gif'
  insertimage.src=ginsertrowimage
  insertimage.alt='Insert a new row here (Ctrl+N)'
 }
 return
}  

function openrecord_onclick()
{

 login('openrecord')

 if (!validateupdate()) return false
 
 //get a reply or return false
 if ((reply=neosysformpopup2(openrecord))==null) return logout('openrecord')
 
 //forget it if no change
 if (reply==getkey()) return logout('openrecord no change')
 
 //do not change key if user chooses not to unload an existing document
 if (!closedoc('OPEN',true))
 {
  logout('openrecord user cancelled')
  return false
 }

 opendoc(reply)
 
 logout('openrecord')
 
 return true
 
}

function nextrecord_onclick(direction)
{

 //direction is 'first', 0, '', -1, 1 or 'last' ('' means 1)
 
 var nextkeys=gkeys
 var nextkeyn=gkeyn
 
 //goto one of many ... or if select many then reduce list to those selected
 if (direction==0)
 {
  var selkeys=neosysdecide('',gkeys,[[0,'Key']],0,'',many=true)
  if (!selkeys) return false
  if (selkeys.length>1)
  {
   nextkeys=selkeys
   nextkeyn=1
  }
  else
  {
   nextkeyn=nextkeys.neosyslocate(selkeys[0])-1
  }
 }
 else
 {
  
  if (!direction) direction=1
 
  //get the next key
  if (direction=='first') nextkeyn=0
  else if (direction=='last') nextkeyn=nextkeys.length-1
  else
  {
   nextkeyn+=direction
   if (nextkeyn>=nextkeys.length) nextkeyn=0
   if (nextkeyn<0) nextkeyn=nextkeys.length-1
  }
 }
 
 var nextkey=nextkeys[nextkeyn]
 
 //switch to new key if new and user accepts to close the current one
 if (nextkey!=gkey)
 {
  //do not change key if user chooses not to unload an existing document
  if (!closedoc('OPEN',true)) return false
  gkeyn=nextkeyn
  opendoc(nextkey)
 }
 
 gkeys=nextkeys
 gkeyn=nextkeyn
 
 return true
 
}

function neosyslink()
{

 login('neosyslink')
 
 //prevent any further action
 window.event.cancelBubble=true
 window.event.returnValue=false
 
 //search next then previous siblings for popup
 //if (element.type!='text') element=element.previousSibling
 //if (element.type!='text') element=element.previousSibling
 var element=window.event.srcElement
 while (element&&!element.neosyslink)
 {
  element=element.nextSibling
 }
 if (!element||!element.neosyslink)
 {
  var element=window.event.srcElement
  while (element&&!element.neosyslink)
  {
   element=element.previousSibling
  }
 }
 
 //quit if no link defined
 if (!element||!element.neosyslink)
  return logout('neosyslink - no link')
 
 //prevent popups except on the key field unless a record is present
 if(gKeyNodes&&!gloaded&&element.neosysfieldno!=0)
 {
  focuson(gKeyNodes[0])
  return logout('neosyslink - no record')
 }
 
 grecn=getrecn(element)
 
 gvalue=getvalue(element,grecn)
 if (!element.neosyslowercase) gvalue=gvalue.toUpperCase()
 
 var reply=neosysevaluate(element.neosyslink,'neosyslink()')
 
 logout('neosyslink')
 
 return
 
}

function neosysformpopup(element)
{

 //element is only provided from f7/alt+down keyboard events
 
 login('neosysformpopup')
 
 //prevent any further action
 window.event.cancelBubble=true
 window.event.returnValue=false
 
 //search current and following siblings for popup
 //if (element.type!='text') element=element.previousSibling
 //if (element.type!='text') element=element.previousSibling
 //var element=window.event.srcElement
 if (!element)
 {
  element=window.event.srcElement
  while (element&&!element.neosyspopup&&element.tagName!='SELECT')
  {
   element=element.nextSibling
  }
 }
 
 //quit if no element
 if (!element)
  return logout('neosysformpopup - no element')
 
 //validateupdate previous field
 //if (element.id!=gpreviouselement.id)
 if (element!=gpreviouselement)
 {
  if (!validateupdate())
   return logout('neosysformpopup - validateupdate failed')
 }
  
 //cannot update anything but key field if not locked or save button not enabled
 if (element.neosysfieldno!=0&&gKeyNodes&&(!glocked||saverecord.disabled))
 {
  readonlydocmsg()
  return logout('neosysformpopup - read only document')
 }
 
 //quit if readonly
 if (element.neosysreadonly)
 {
  if (typeof element.neosysreadonly!='boolean') neosysinvalid(element.neosysreadonly)
  return logout('neosysformpopup - read only')
 }
 
 //quit if no popup defined
 if (!element.neosyspopup&&element.tagName!='SELECT')
  return logout('neosysformpopup - no popup')
 
 //prevent popups except on the key field unless a record is present
 if(gKeyNodes&&!gloaded&&element.neosysfieldno!=0)
 {
  focuson(gKeyNodes[0])
  return logout('neosysformpopup - no record')
 }
 
 grecn=getrecn(element)
 gvalue=getvalue(element)
 gpreviouselement=element
 gpreviousvalue=gvalue
 
 //get a reply or return false
 /////////////////////////////
 if ((reply=neosysformpopup2(element))==null)
 {
  focuson(element)
  return logout('neosysformpopup - no reply')
 }

 //update the element multiple selections qqq
 //if reply is an array use replace all rows
 if (reply.length>1&&element.neosysgroupno>0&&typeof(reply)=='object')
 {
  var elementid=element.id
  insertallrows(element,reply,exceptfirst=true)
  
  //focus on next element AFTER table
  element=document.getElementsByName(elementid)[0]
  focusdirection(1,element,element.neosysgroupno)
  
  logout('neosysformpopup - insert all rows')
  return true
  
 }

 //if reply is still an array use the first one
 if (typeof(reply)=='object') reply=reply[0]

 //forget it if no change (changed to allow early update on popup eg pop_target)
 //if (reply==getvalue(element))
 if (reply==gpreviousvalue)
 {
  focusnext()
  return logout('neosysformpopup - no change')
 }
 
 //do not change key if user chooses not to unload an existing document
 if (element.neosysfieldno==0&&gloaded&&!closedoc('OPEN',true))
 {
  return logout('neosysformpopup - user cancelled unloaddoc')
 }
 
 //output convert it
 if (element.neosysconversion)
 {
  reply=validateoconv(element,reply,reply)
  if (typeof reply=='undefined'||reply==null)
  {
   return logout('neosysformpopup - oconv failed')
  }
 }
 
 //setup next onfocus to validateupdate
 //done before calling popup now
 //gpreviouselement=element
 //gpreviousvalue=getvalue(element)
 
 //update the screen
 setvalue(element,reply)

 //move focus to next field (should trigger validation etc)
 //have to force validation since the focus may already be on the next field
 //if clicked on the popup
 if (validateupdate()) focusnext(element)
 //window.setTimeout('focusnext()',10)
 //validateupdate()
 
 logout('neosysformpopup')

}

function neosysformpopup2(element)
{

 //given a SELECT item or element with a popupfunction to evaluate, returns a reply or null

 //quit if no popup defined and not SELECT
 var expression=element.neosyspopup
 if (!expression&&element.tagName!='SELECT') return false

 //evaluate popup expression if provided
 if (expression)
 {
  var reply=neosysevaluate(expression,'neosysformpopup2()')
 }

 //otherwise build a list and select from the SELECT
 else
 {
 
  //build a popup from the SELECT tag
  var values=[]
  for (var i=0;i<element.childNodes.length;i++)
  {
   var option=element.childNodes[i]
   values[values.length]=[option.value,option.innerText]
  }
  
  //allow multiple selection if element is in a group with only one input column
  var multipleselection=false
  if (element.neosysgroupno)
  {
   var tableelement=gtables[element.neosysgroupno].tableelement
   var multipleselection=tableelement&&tableelement.neosyslastinputcolumnscreenfieldno==tableelement.neosysfirstinputcolumnscreenfieldno
  }
     
  //get the response(s)
  reply=neosysdecide2('',values,'1','','',multipleselection)
  
  //have to do this to cancel the standard dropdown if they press alt+down then press escape on the popup
  if (!reply) focuson(element)
  
  for (var ii=0;ii<reply.length;ii++)
  {
   reply[ii]=values[reply[ii]-1][0]
  }
  
 }
 
 //undefined, null or '' all return as null
 if
  (typeof(reply)=='undefined'
  ||(typeof(reply)=='boolean'&&reply==false)
  ||reply==null
  ||(typeof(reply)=='string'&&reply=='')
  )
 {
  reply=null
  
  //if popup returns nothing but gdependents is set then the popup must have updated
  //something itself so recalc dependents
  if (gdependents.length) calcfields(gdependents)
  
 }

 //if sole key field or the open function, setup a list of keys
 if (reply&&((openrecord&&element==openrecord)||(element.neosysfieldno==0&&gKeyNodes.length==1)))
 {
  gkeyn=0
  if (typeof reply=='object')
  {
   gkeys=reply
   //if reply is an array use the first
   reply=reply[0]
  }
  else
  {
   gkeys=[reply]
  }
 }
 
 return reply
 
}

function getkeyexternal()
{
 return getkey('oconv')
}

function debug(v)
{
 if (!confirm(v)) breakpoint()
}

function getkey(mode)
{

 login('getkey '+mode)

 //returns a string containing the key or '' if any key parts are required and ''
 var key=new Array(gKeyNodes.length)
 for (var i=0;i<gKeyNodes.length;i++)
 {
  var temp=getvalue(gKeyNodes[i])
  if (temp==''&&gKeyNodes[i].neosysrequired!=false)
  {
   logout('getkey ""')
   return ''
  }
  if (mode!='oconv')
  {
   var conversion=gKeyNodes[i].neosysconversion
   if (conversion.slice(0,1)=='[') temp=neosysiconv(temp,conversion)
  }
  key[gKeyNodes[i].neosyskeypart-1]=temp
 }
 
 var key
 if (mode!='oconv')  key=key.join('*')
 else key=key.join(' ')
 
 //debug(key) 
 
 logout('getkey '+key)

 return key
 
}

function setkeyvalues(key)
{
 //given a string updates the keyfield(s)
 for (var i=0;i<gKeyNodes.length;i++)
 {
  var temp=key.neosysfield('*',gKeyNodes[i].neosyskeypart)
  //var conversion=gKeyNodes[i].neosysconversion
  //if (conversion.slice(0,1)=='[') temp=temp.neosysoconv(conversion)
  //setvalue(gKeyNodes[i],temp)
  gds.setrecord(gKeyNodes[i].id,'',temp)
 }
}

function mergeattributes(sourceelement,targetelement)
{

 //does not work on mac despite docs
 //targetelement.mergeAttributes(sourceelement)
  
 //targetelement.id=sourceelement.id
 ////targetelement.name=sourceelement.name
 
 //merges all strin and number attributes except id, outerText and outerHTML
 for (var attribname in sourceelement)
 {
  var attribute=sourceelement[attribname]
  //alert(attribname)
  try
  {
   //if (attribname!='id'&&attribname!='name'&&attribname!='outerText'&&attribname!='outerHTML'&&(typeof(attribute)=='string'||typeof(attribute)=='number'))
   if (attribname!='innerText'&&attribname!='innerHTML'&&attribname!='outerText'&&attribname!='outerHTML'&&(typeof(attribute)=='string'||typeof(attribute)=='number'||attribute==true||attribute==false))
   {
//    alert(attribname+':'+attribute)
      targetelement.setAttribute(attribname,attribute)
//    targetelement[attribname]=attribute
   }
  }
  catch(e)
  {
  }
 }
 
}

function getrecn(element)
{

 if (typeof element=='undefined') element=event.srcElement
  
 neosysassertobject(element,'getrecn','element')
 //returns null if element not part of a table
 
 var recn
 if (element.neosysgroupno==0||!element.neosysgroupno)
 {
  recn=null
 }
 else
 {
  var rowx=getancestor(element,'TR')
  if (!rowx) return null
  var tablex=getancestor(rowx,'TABLE')
  recn=rowx.rowIndex
  if (tablex&&tablex.tHead) recn-=tablex.tHead.rows.length
 }
 
 return recn
 
}

//allows dom scan without using IE document.all[i]
function nextelement(element)
{

 //return first child
 if (element.childNodes.length) return element.firstChild

 //otherwise return next sibling
 if (element.nextSibling) return element.nextSibling
 
 //otherwise return next aunt
 while (true)
 {
  element=element.parentNode
  if (element.tagName=='BODY') return false
  if (element.nextSibling) return element.nextSibling
 }
 
}

function getnextrow(dictid,skipblanks,forward)
{
 if (typeof forward=='undefined') forward=true
 return getpreviousrow(dictid,skipblanks,forward)
}

function getpreviousrow(dictid,skipblanks,forward)
{

 //given a dictionary id (or use gpreviouselement) and an option to skip over blanks,
 //return the contents of the previous row to the current row determined by grecn
 //forward true means search forwards not backwards

 if (!dictid) dictid=gpreviouselement.id
 var value=''
 
 //quick previous search
 if (!forward)
 {
  if (grecn==0) return ''
  if (value=getvalue(dictid,grecn-1)) return value
  if (!skipblanks) return ''
 }
 
 //more comprehensive search
// var values=getvalue(dictid,null)
 var values=getvalues(dictid)
 
 if (forward)
 {
  var ln=grecn+1
  var ln2=skipblanks?values.length:ln+1
  for (ln=ln;ln<=ln2;ln++)
  {
   if ((value=values[ln])!='') return value
  }
 }
 else
 {
  var ln=grecn-1
  var ln2=skipblanks?0:ln
  for (ln=ln;ln>=ln2&&ln>=0;ln--)
  {
   if ((value=values[ln])!='') return value
  }
 }
 return ''
 
}

function form_ondblclick()
{
 return form_filter('filter')
}

function form_onrightclick()
{
 // alert('x')
 return true
}

function form_filter(mode,colidorgroupno,regexp,maxrecn)
{
 
 if (mode=='refilter')
 {
  colidorgroupno2=colidorgroupno
  form_filter('unfilter',colidorgroupno2)
  mode='filter'
 }
 
 //get colid
 var colid=colidorgroupno
 if (!colid)
 {
  colid=event.srcElement.dataFld
  if (!colid)
  {
   var element=event.srcElement.parentNode
   while (element)
   {
    colid=element.dataFld
    if (colid) break
    element=element.parentNode
   }
  }
 }
 
 //get dictitem
 if (mode=='filter'||mode=='unfilter')
 {
 }
 
 //get groupno
 var groupno
 if (typeof colidorgroupno!='number'&&(mode=='filter'||mode=='unfilter'))
 {
  var dictitem=gds.dictitem(colid)
  //if (!dictitem) return neosysinvalid(colid+' dictitem does not exist in form_filter()')
  groupno=dictitem.groupno
  if (!groupno) return neosysinvalid()//colid+' is not multivalued for sorting'
 }
 else
 {
  groupno=colidorgroupno
 }
 
 //prefilter
 if (typeof form_prefilter=='function')
 {
  //neosysevaluate('form_prefilter()','formfunctions_onload()')
  if (!form_prefilter(mode,colid)) return neosysinvalid()
 }
 
 //get the table rows
 var tablex=document.getElementById('neosysgroup'+groupno)
 if (!tablex) return neosysinvalid(colid+' is not part of a table')
 var trows=tablex.tBodies[0].rows
 
 //unfilter and exit (not tested or used anywhere)
 if (mode=='expand')
 {
  for (var rown=grecn+1;rown<trows.length;rown++)
  {
   if (trows[rown].style.display=='inline') break
   trows[rown].style.display='inline'
  }
  return true
 }
 
 //get the show all button
 var tablexshowall=document.getElementById('neosysgroup'+groupno+'showall')
 if (!tablexshowall)
 {
  //syserror('form_filter()','Cannot find showall button, are you missing a thead?')
  return true
 }
 
 //unfilter and exit
 if (mode=='unfilter')
 {
  for (var rown=0;rown<trows.length;rown++)
  {
   //change to expandrow image
   setinsertimage('insert',trows[rown],groupno)

   trows[rown].style.display='inline'
  }
  //hide the show all button
  tablexshowall.style.display='none'
  return true
 }
 
 window.status='Filtering, please wait ...'
 
 //get the value and values to be filtered
 var values=gds.recalcx(colid,null)
 if (!regexp&&!maxrecn)
 {
  grecn=getrecn()
  var value=values[grecn]
 }
 
 //hide unmatched rows
 var llastunhiddenrow=''
 var anyunhiddenrows=false
 for (var rown=0;rown<values.length;rown++)
 {
 
  //trows[rown].style.display=(values[rown]==value)?'inline':'none'
  var hiderow=false
  if (regexp)
  {
   if (values[rown].match(regexp)) hiderow=true
  }
  else if (maxrecn)
  {
   hiderow=rown>maxrecn
  }
  else if (values[rown]!=value) hiderow=true
  
  if (hiderow)
  {
   trows[rown].style.display='none'
   if (tablexshowall) tablexshowall.style.display='inline'

   //mark last unhidden row as expand image
   if (lastunhiddenrow)
   {
    //change to expandrow image
    setinsertimage('expand',lastunhiddenrow,groupno)
    lastunhiddenrow=''
   }
   
  }
  else
  {
   var lastunhiddenrow=trows[rown]
   anyunhiddenrows=true
  }
  
 }
 
 if (!anyunhiddenrows&&!maxrecn) form_filter('unfilter',colidorgroupno)
 
 window.status=''
  
 return true
 
}

var calendar_checkInDatePicker

function form_pop_calendar()
{
 //do this so that it pops up after focussing on the entry element
 window.setTimeout('form_popcalendar2()',100)
 return false
}

function form_popcalendar2()
{

 //remove any previous calendar
 //if (calendar_checkInDatePicker) calendar_checkInDatePicker.hide()

 var datevalue=gvalue.toString().neosysiconv('[DATE]')
 if (datevalue)
 {
  var msdate=new Date()
  msdate.setDate(Number(datevalue.neosysoconv('[DATE,DOM]')))
  msdate.setMonth(Number(datevalue.neosysoconv('[DATE,MONTH]')-1))
  msdate.setFullYear(Number(datevalue.neosysoconv('[DATE,YEAR]')))
  calendar_checkInDatePicker = new Calendar(msdate);
 }
 else
  calendar_checkInDatePicker = new Calendar();

/*
 calendar_checkInDatePicker.setMonthNames(new Array("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"));
 calendar_checkInDatePicker.setShortMonthNames(new Array("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"));
 calendar_checkInDatePicker.setWeekDayNames(new Array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"));
 calendar_checkInDatePicker.setShortWeekDayNames(new Array("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"));
*/

 calendar_checkInDatePicker.setFormat("d/M/yyyy");
 calendar_checkInDatePicker.setFirstDayOfWeek(6);
 calendar_checkInDatePicker.setMinimalDaysInFirstWeek(1);
 calendar_checkInDatePicker.setIncludeWeek(false);
 
 calendar_checkInDatePicker.create();
 
 calendar_checkInDatePicker.onchange = function() 
 {
 
  if (!calendar_checkInDatePicker) return true
  
  setvalue(gpreviouselement,calendar_checkInDatePicker.formatDate())

  //clear the object
  window.setTimeout('calendar_checkInDatePicker=null',1)
  
  focusnext(gpreviouselement)
  return true
 }

 calendar_checkInDatePicker.show(gpreviouselement)
 calendar_checkInDatePicker._calDiv.focus()
 //calendar_checkInDatePicker._calDiv.setActive()
 //xyz=window.setInterval('window.status=document.activeElement.outerHTML',10)
 return false
 
}

//drop down any "modal" popup divs
//return true if any closed or false if none
function form_closepopups()
{

 var anyclosed=false
 
 if (calendar_checkInDatePicker)
 {
  try
  {
   calendar_checkInDatePicker.hide()
  }
  catch (e)
  {
   if (gusername=='NEOSYS') neosysnote('couldnt drop calendar\r'+e.description)
  }
  calendar_checkInDatePicker=''
  anyclosed=true
  
 }
 
 return anyclosed
 
}

function form_val_index(filename,fieldname,allownew)
{

 //check if an existing
 
 if (!filename) filename=gdatafilename
 if (!fieldname) fieldname=gpreviouselement.dataFld

 //get the existing values
 var indexvalues=form_get_index(filename,fieldname)
 if (!indexvalues) return true
 indexvalues=indexvalues.group1

 //ok if is an existing indexvalue (Exact Case)
 for (var i=0;i<indexvalues.length;i++)
 {
  if (indexvalues[i][fieldname].text==gvalue)
  {
   return true
  }
 }
 
 //optionally change case
 for (var i=0;i<indexvalues.length;i++)
 {
  if (indexvalues[i][fieldname].text.toUpperCase()==gvalue.toUpperCase())
  {
   if (!(reply=neosysyesnocancel('Change the capitalisation of your entry?\r\rFrom: '+gvalue.neosysquote()+'\r\r  To: '+indexvalues[i][fieldname].text.neosysquote(),1))) return neosysinvalid()
   if (reply==1) gvalue=indexvalues[i][fieldname].text
   return true
  }
 }
 
 //option to allow/prevent new index values
 if (!allownew) return neosysinvalid(gvalue+' '+fieldname.toLowerCase()+' does not exist')
 
 //confirm is new index value
 if (gvalue&&(neosysyesno(' Is '+neosysquote(gvalue)+' to be a new '+fieldname.toLowerCase()+' for '+filename.toLowerCase()+' ?',1)!=1)) return false
 
 return true
 
}

function form_get_index(filename,fieldname)
{

 db.request='CACHE\rGETINDEXVALUESXML\r'+filename+'\r'+fieldname
 if (!db.send())
 {
  neosysinvalid(db.response)
  return ''
 }
 
 if (!db.data||db.data=='<records></records>') return ''
 
 return neosysxml2obj(db.data)
 
}

function form_pop_index(filename,fieldname,many)
{

 //provides a simple way to select previously entered values on an indexed field
 
 if (!filename) filename=gdatafilename
 if (!fieldname) fieldname=gpreviouselement.dataFld

 //get index values
 var indexvalues=form_get_index(filename,fieldname)
 if (!indexvalues||indexvalues.group1.length==0) return neosysinvalid('No records have been entered on '+filename.toLowerCase()+' yet')

 //user selects index value(s)
 var result=neosysdecide2('',indexvalues,[[fieldname,fieldname.neosyscapitalise()]],0,'',many)
 
 return result
 
}