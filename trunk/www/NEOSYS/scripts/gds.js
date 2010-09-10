//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

//////////////////
function neosysdatasource()
{
 
 this.data=null
 this.onreadystatechange=null
 this.readystate='initialised'

 //methods
 this.evaluate=gds_evaluate
 this.recalcx=gds_recalcx
 this.calcx=gds_calcx
 this.getcells=gds_getcells
 this.setrecord=gds_setrecord
 this.setrecord2=gds_setrecord2
 this.setdefaulted=gds_setdefaulted
 this.load=gds_load
 this.updaterow=gds_updaterow
 this.appendrow=gds_appendrow
 this.createrow=gds_createrow
 this.addrow=gds_addrow
 this.insertrow=gds_insertrow
 this.blankrow=gds_blankrow
 this.blankrowx=gds_blankrowx
 this.deleterow=gds_deleterow
 this.bind=gds_bind
 this.rebind=gds_rebind
 
}

function gds_evaluate(functionobject)
{
 if (typeof functionobject=='boolean') return functionobject
 if (!functionobject) return true
 this.tempfunction=functionobject
 return this.tempfunction()
}

function gds_recalcx(dictitemorid,recn)
{

 //actually is screenelement not dictitem!
 
 //convert id into dictitem
 var dictitem=dictitemorid
 if (typeof dictitem=='string')
 {
  dictitem=this.dictitem(dictitem)
  if (!dictitem)
  {
   systemerror('gds_recalcx()',neosysquote(dictitemorid)+' is not in the '+gdatafilename+' dictionary.')
   return ''
  }
 }

 this.tempfunction=dictitem.functioncode?dictitem.functioncode:dictitem.neosysfunctioncode
 if (!this.tempfunction)
 {

  //recalc doesnt need to be called for F items but if it is should work like calcx
  if (dictitem.type=='F') return this.calcx(dictitem.name,recn)

  return ''
 }
 
 login('recalcx '+dictitem.name+' '+grecn)
 //login('recalcx '+dictitem.id+' '+grecn)
 
 var storegrecn=grecn
 if (!dictitem.groupno) grecn=null

 //calculate the answer (possibly multivalued)
 var results=this.tempfunction()
 if (typeof results=='undefined') systemerror('gds_recalx',(dictitem.name?dictitem.name:dictitem.id)+' function returned undefined')

 //and update the record and display
 /*if (dictitem.type!='S') */
 this.setrecord(dictitem,!dictitem.groupno?0:grecn,results)
 
 grecn=storegrecn
 
 logout('recalcx '+dictitem.name+' '+grecn)
 //logout('recalcx '+dictitem.id+' '+grecn)
 
 return results
 
}

//CALCX
function gds_calcx(datafld,recn,oldtext)
{

 //if no data section then return nothing
 //eg in postinit setting expressions based on gds before there is any data section
 //neosyssetexpression('currencycode_span','innerText','gds.calcx("CURRENCY_CODE")')
 //(not necessary now that cleardoc is *always* called in forminit
 //if (!this.data) return ''

 if (!ginitok) return ''
 
 if (typeof recn=='undefined') recn=grecn
 
 var results=[]
 
 elements=this.getcells(datafld,recn)
 
 if (elements.length==0)
 {
  systemerror('gds.calcx()',datafld+' is not in the dictionary\ror is not in the correct group.')
  results[0]=''
  return results
 }
 
 for (var i=0;i<elements.length;i++)
 {
  var element=elements[i]
  results[i]=(oldtext&&(typeof element.oldtext!='undefined'))?element.oldtext:element.text
 }

 //if not multivalued then return single value
 if (elements.groupno==0) results=results[0]
 
 return results
 
}

//GETCELLS
//////////
function gds_getcells(dbelementordbelementid,recn)
{

 if (!this.data)
 {
  systemerror('gds_getcells','this.data is unassigned')
  return []
 }
 
 //return an array of objects that have text properties containing the data
 //for a specific field (all or one row - array with one element)
 //return emptyarray if dbelementid does not exist
 
 //given a dbelement, get target cell, or cells if recn=null


 //this seems to solve a problem (at least 
 // where onfocus events are happening at the same time as
 //client_onload thread ... who knows why you can put anything instead of return
 //alert('x') puts up an alert but you cannot seem force into debugger eg xxx() is allowed!!!
 if (!ginitok) return

 login('getcells '+dbelementordbelementid+' '+recn)
 
 var dictid
 var dbelement
 var dbelement2
 var groupno
 
 //if passed an object assume that it is a neosys HTML data element
 //or a dictitem
 if (typeof(dbelementordbelementid)=='object')
 {
  dictid=dbelementordbelementid.dataFld
  if (dictid) groupno=dbelementordbelementid.neosysgroupno
  else
  {
   dictid=dbelementordbelementid.name
   if (!dictid)
   {
    return systemerror('gds_getcells','dbelement does not have a datafld or name\r')
   }
   groupno=dbelementordbelementid.groupno
  }
 }
 //if given a dbelementid, convert to (first) matching dbelement
 else
 {
 
  //gets all elements if multivalued
  //gets first element only (on PC (mac?) dbelement2[0]==dbelement is true)
  //dbelement2=document.all[dbelementordbelementid]
  
  //find dbelement in dom
  dbelement=document.getElementById(dbelementordbelementid)
  if (dbelement)
  {
   dictid=dbelementordbelementid
   groupno=dbelement.neosysgroupno
  }
  
  //otherwise try to find it in the dictionary
  else
  {
   var dictitem=this.dictitem(dbelementordbelementid)
   if (!dictitem)
   {
    systemerror('getcells',neosysquote(dbelementordbelementid)+' db element is missing')
    logout('getcells - no db element - returning []')
    return []
   }
   
   dictid=dictitem.name
   groupno=dictitem.groupno
   
  }
  
  //NB mac SELECT tags are arrays of options as well
  //if (dbelement.length>1) dbelement=dbelement[0]
  
 }

 //get the SELECT node of OPTION elements
 //if (dbelement.tagName=='OPTION') dbelement=dbelement.parentNode
 
 if (groupno==0)
 {
  cells=[this.data[dictid]]
 }
 else
 {
  var datasource
  datasource=this.data['group'+groupno]
  var startrown
  var endrown
  if (recn==null)
  {
   startrown=0
   endrown=datasource.length-1
  }
  else
  {
   startrown=recn
   endrown=recn
  }
  var cells=[]
  for (var rown=startrown;rown<=endrown;rown++)
  {
   //some cases like calling calcx for one grouped dictid from a dictid belonging to a different group number
   //cause errors when the line numbers dont agree. this should be treated as a bug in the application code
   try
   {
    cells[cells.length]=datasource[rown][dictid]
   }
   catch(e)
   {
    var errmsg=dictid+'  has different group'
    if (gpreviouselement) errmsg+=' from '+gpreviouselement.id
    systemerror('getcells()',errmsg)
   }
  }
 }
 
 if (!cells[0])
 {
  systemerror('getcells()',dictid+'  has no cells')
 }

 //save the groupno and dictid
 cells.groupno=groupno
 cells.dictid=dictid
 
 logout('getcells')

 return cells
 
}

//SETRECORD
///////////
function gds_setrecord(element,recn,values)
{
 
 //given a screen element or dbelement or element name
 //and values, update the screen and record

 //called from:
 //validateupdate
 //setdefault
 //gds_recalcx
 //earlyupdate
 //insertallrows
 //general user code
 
 //done in getcells now
 //if (typeof element=='string') element=document.getElementById(element)
 //assertelement(element,'setrecord','element')

 //nb select looks like an array
 //if (element.length>1) element=element[0]

 //check values is defined
 if (typeof values=='undefined')
 {
  systemerror('setrecord()','values is undefined')
  return
 }
 
 //check recn is null or numeric
 if (recn!=null) neosysassertnumeric(recn,'setrecord','recn')
 
 //get the cells to be updated 
 var cells=this.getcells(element,recn)
 if (!cells[0]) return
 
 if (values==null) values=''

 //convert single value into an array
 if (typeof(values)!='object') values=[values]
 
 return this.setrecord2(cells,values)
 
}

//SETDEFAULTED
//////////////
function gds_setdefaulted(element,recn,trueorfalse)
{
 
 //nb setting a value to '' also clears the "defaulted" property
 
 //check recn is null or numeric
 if (recn!=null) neosysassertnumeric(recn,'setrecord','recn')
 
 //get the cells to be updated 
 var cells=this.getcells(element,recn)
 if (!cells[0]) return
 
 //set the cells to defaulted trueorfalse
 for (var rown=0;rown<cells.length;rown++)
 {
  cells[rown].defaulted=trueorfalse
 }
 
 return
 
}

function gds_setrecord2(cells,values)
{

 var conversion=false
 
 // for (var rown=0;rown<values.length;rown++)
 //sometimes more values than cells!
 for (var rown=0;rown<values.length&&rown<cells.length;rown++)
 {

//this is cut out because
//could enter 999.999 when the old data is 1000.00 and
//although the internal data does not need to be updated,
//the screen value MIGHT need updating (in this case from 999.999 to 1000.00
//  if (cells[rown].text!=values[rown])

  {

   //prevent NaN or undefined
   var value=values[rown]
   if (typeof value!='string'&&(isNaN(value)||typeof value=='undefined'))
   {
    systemerror('setrecord()','Attempting to set '+cells.dictid+' rown '+(rown+1)+' to '+(typeof value=='undefined'?'undefined':'NaN')+' is disallowed')
    continue
   }

   var rowcell=cells[rown]
   
   //prevent very small numbers
   if (typeof value=='number'&&Math.abs(value)<0.0000000001) value=0

   //save original value if not present
   if (typeof rowcell.oldtext=='undefined') rowcell.oldtext=rowcell.text
   
   //get the screen element
   var screenelement=rowcell.element
  
   //update the recs
   rowcell.text=value
//if (gro.data.group1[0].BILL.text=='0') xxxx
   //reset the defaulted property
   if (!value&&rowcell.defaulted)
   {
    //clearing a checkbox does not allow redefaulting
    if (!screenelement||'checkbox radio'.indexOf(screenelement.type)<0)
    {
     cells[rown].defaulted=false
    }
   }
   //update the screen if possible
  
   //if no screen element then
   if (!screenelement||typeof screenelement!='object')
   {
  
    //if null then maybe same group number in different tables
    //or trying to set a field before it is bound

    //if no screen element, display an error message
    //get the id
    //if (gds.readystate!='loading')
    //{    
    // if (typeof element=='string')
    //  var elementid=element
    // else
    //  var elementid=element.id
    // alert('setrecord cannot set "'+elementid+'" because it has not been bound.\rMaybe dictitem has same group number in different tables.')
    //}
   
   }
  
   //if there is a screen element then update the screen
   else
   {
  
    //work out if conversion required and do any conversion
    if (rown==0) conversion=screenelement.neosysconversion
    if (conversion)
    {
     var ivalue=value
     value=oconvertvalue(value,conversion)
     if (value==null)
     {
      systemerror('gds_setrecord("'+screenelement.id+'") '+neosysquote(ivalue)+' is not a '+conversion)
      return
     }
    }
    
    //update the screen value (fast unprotected)
    setvalue2(screenelement,value)
  
    //add new dependents to list if not already there
    if (rown==0)
    {
     var newdependents=screenelement.neosysdependents
     if (newdependents)
     {
      newdependents=newdependents.split(';')
      for (var depn=0;depn<newdependents.length;depn++)
      {
       if (!gdependents.neosyslocate(newdependents[depn])) gdependents[gdependents.length]=newdependents[depn]
      }
     }
    }
//alert(screenelement.dataFld+' : '+screenelement.name+' : '+newdependents.join()+' : '+gdependents.join())
   }
   
  }
  
 }//next rown
 
 return true
 
}


//LOAD
//////
function gds_load(neosysrecordobject1)
{
 
 login('load')
 
 //trigger loading event
 this.readystate='loading'
 if (typeof(this.onreadystatechange)=='function') this.onreadystatechange()

 //grab the dict and dictitem method
 this.dict=neosysrecordobject1.dict
 this.dictitem=neosysrecordobject1.dictitem
 
 //bind newdatasource to the HTML screen
 this.data=neosysrecordobject1.data
 this.bind(this.data,'',0)
 
// alert(neosysobj2xml(gro.revstr))
//gstepping=true
//breakpoint('x')

 //trigger completion event
 this.readystate='complete'
 if (typeof(this.onreadystatechange)=='function') this.onreadystatechange()
 
 logout('load')
 
}

//UPDATEROW
///////////
function gds_updaterow(mode,row,copyrow)
{

 for (dictid in copyrow)
 {

  //only copy non-key fields
  var dictitem=this.dictitem(dictid)
  if (dictitem&&dictitem.fieldno.toString()!='0')
  {
   var element=row[dictid].element
   //new data cell
   var cell=row[dictid]=new Object
   //but restore the binding
   cell.element=element 
   if (mode!='copy'||dictitem.copyable!=false)
   {
    cell.text=copyrow[dictid].text
//if (gro.data.group1[0].BILL.text=='0') xxxx
   }
   else
   {
    cell.text=''
   }
  }
 }

 return row
 
}

//APPENDROW
///////////
function gds_appendrow(mode,group,copyrow)
{

 //if mode='copy' then do not copy fields with .copyable==false

 //add a new row
 var row=group[group.length]=this.createrow(group)

 //update the row
 this.updaterow(mode,row,copyrow)
 
 return row
 
}

//CREATEROW
///////////
function gds_createrow(group)
{
 var newrow=new Object
 var oldrow=group[0]
 for (var propname in oldrow)
 {
  var cell=newrow[propname]=new Object
  cell.text=''
 }
 return newrow
}
 
//ADDROW
////////
function gds_addrow(groupno,newdatarow)
{

 var tablex=document.getElementById('neosysgroup'+groupno)
 //if (!tablex) return false
 
 //get the group
 var group=this.data['group'+groupno]
 
 //append a new row (copy first row)
 if (tablex)
 {
  var tbody=tablex.tBodies[0]
  var firstrow=tbody.childNodes[0]
  
  if (!ismac)
  {
   var newrow=firstrow.cloneNode(true)
   tbody.insertBefore(newrow,null)
  }
  else
  {
   //so do the following
   var newrow=tbody.insertBefore(document.createElement('TR'))
   //for (var coln=0;coln<firstrow.childNodes.length;coln++)
   for (var coln=firstrow.childNodes.length-1;coln>=0;coln--)
   {
    //var newnode=firstrow.childNodes[coln].cloneNode(true)
    var newnode=document.createElement('TD')
    newnode.innerHTML=firstrow.childNodes[coln].innerHTML
    //newnode.style.width=10
    if (newrow.childNodes.length)  newrow.insertBefore(newnode,newrow.childNodes[0])
    else newrow.insertBefore(newnode)
   }

  }
  setupnewrow(newrow,firstrow)
 }
 
 //create a new row
 //formrow=tablex.tBodies[0].childNodes[rown]
 group[group.length]=this.createrow(group)
 
 //and blank it/bind it
 var datarow=group[group.length-1]
 if (newdatarow)
 {
  //not tested/debugged yet
  this.rebind(datarow,newdatarow)
 }
 else
 {
  this.blankrowx(datarow,newrow,true)
 }

 var rown=group.length-1
 if (ismac) window.setTimeout('fixmacinsertdeleteevent(1,'+rown+')',1)

 return datarow=group[rown]
  
}

function setonfocus(element)
{
 return
 setonfocus2('INPUT',element)
 setonfocus2('SELECT',element)
 setonfocus2('TEXTAREA',element)
}

function setonfocus2(tagname,element)
{
 elements=element.getElementsByTagName(tagname)
 for (var i=0;i<elements.length;i++)
 {
  //qwertyelements[i].onfocus=onfocus
//  elements[i].ondblclick=form_ondblclick
 }
}

function setupnewrow(newrow,firstrow)
{

 if (false&&ismac)
 {
  for (var coln=0;coln<newrow.childNodes.length;coln++)
  {
   //newrow.replaceChild(newrow.childNodes[coln],newrow.childNodes[coln])
   var firstcell=firstrow.childNodes[coln]
   var cell=newrow.childNodes[coln]
   var isfirstrow=(cell==firstcell)
   cell.innerHTML=firstcell.innerHTML+'\r'

  }
 }
 
 //add events
 setonfocus(newrow)

}

//INSERTROW
///////////
function gds_insertrow(groupno,rown)
{

 var tablex=document.getElementById('neosysgroup'+groupno)
 if (!tablex)return
 
 //get the group
 var group=this.data['group'+groupno]

 //insert a new row (copy first row)
 var tbody=tablex.tBodies[0]
 var firstrow=tbody.childNodes[0]
 
 if (!ismac)
 {
  var newrow=firstrow.cloneNode(true)
  tbody.insertBefore(newrow,tbody.childNodes[rown])
 }
 else
 {
  //so do the following
  var newrow=tbody.insertBefore(document.createElement('TR'),tbody.childNodes[rown])
  //for (var coln=0;coln<firstrow.childNodes.length;coln++)
  for (var coln=firstrow.childNodes.length-1;coln>=0;coln--)
  {
   //var newnode=firstrow.childNodes[coln].cloneNode(true)
   var newnode=document.createElement('TD')
   newnode.innerHTML=firstrow.childNodes[coln].innerHTML
   //newnode.style.width=10
   if (newrow.childNodes.length)  newrow.insertBefore(newnode,newrow.childNodes[0])
   else newrow.insertBefore(newnode)
  }

 }
 
 setupnewrow(newrow,firstrow)

 //ripple up the following data rows (backwards from last)
 for (var rown2=group.length-1;rown2>=rown;rown2--)
 {
  group[rown2+1]=group[rown2]
 }
 
 //create a new row
 group[rown]=this.createrow(group)
 
 //and blank it/bind it
 this.blankrowx(group[rown],newrow,true)

 if (ismac) window.setTimeout('fixmacinsertdeleteevent(1,'+rown+')',1)
 
 return group[rown]
 
}

function fixmacinsertdeleteevent(chapterno,rown)
{
 if (ismac)
 {
//alert('1')
  var newrow=document.getElementById('neosyschapter'+chapterno)
  newrow=newrow.tBodies[0].childNodes[rown]
  var imgs=newrow.childNodes[0].getElementsByTagName('IMG')
  for (var ii=0;ii<imgs.length;ii++)
  {
   if (imgs[ii].id=='insertrowbutton') imgs[ii].onclick=insertrow
   else
   {
    if (imgs[ii].id=='deleterowbutton')
    {
     imgs[ii].onclick=deleterow
//     alert(imgs[ii].onclick)
    }
   }
//alert('2')
  }
 }
}

function gds_blankrow(groupno,rown)
{

 //get the group
 var group=this.data['group'+groupno]
 
 //get the table and row
 var tablex=document.getElementById('neosysgroup'+groupno)
 if (!tablex)return false
 var formrow=tablex.tBodies[0].childNodes[rown]
 if (!formrow) return false

 //blank it
 this.blankrowx(group[rown],formrow,true)

 return group[rown]
 
}

//BLANKROWX
///////////
function gds_blankrowx(datarow,formrow,initdata)
{

 //initdata=false is only used in bind
 //to clear form and NOT do any binding (actually it unbinds)
 
 //blank all the columns found in the data and form rows
 for (var propname in datarow)
 {
 
  //blank the data element
  if (initdata)
  {
   var dataitem=datarow[propname]=new Object
   dataitem.text=''
   dataitem.oldtext=''
   dataitem.defaulted=false
  }
  
  //blank the form element
  try
  {
   var element=null
   element=formrow.all[propname]
   
   if (initdata)
   {
    datarow[propname].element=element
    if (element) element.dataitem=datarow[propname]
   }
   else
   {
    datarow[propname].element=null
    if (element) element.dataitem=null
   }
  }
  catch(e)
  {
   continue
  }
  
  if (element&&(element.dataFld==propname||element.id==propname))
  {
  
   if (ismac) element.dataFld=element.id//fix mac bug
   
   setvalue(element,'')
   
  }
 }
 
}

//DELETEROW
///////////
function gds_deleterow(groupno,rown1,rown2)
{
 //get the group
 var group=this.data['group'+groupno]
 
 if (typeof rown2=='undefined') rown2=rown1
  
 //get the table and row
 var tablex=document.getElementById('neosysgroup'+groupno)
 if (!tablex)return
 var formrow=tablex.tBodies[0].childNodes[rown1]
 if (!formrow) return
 //do not delete first and only row, blank it instead
 if (rown1==0&&group.length==1)
 {
  this.blankrowx(group[rown1],formrow,true)
  return
 }

 //Delete the datasource row

 //zzz?how to avoid deleting all rows?
 
 //copy nondeleted rows to a new group
 var newgroup=[]
 for (var rown=0;rown<group.length;rown++)
 {
  if (rown<rown1||rown>rown2) newgroup[newgroup.length]=group[rown]
 }
 //set the group
 this.data['group'+groupno]=newgroup
 
 //Delete the form row
 var formrowparentnode=formrow.parentNode
 //remove following rows if any
 for (rown=rown1+1;rown<=rown2;rown++) formrowparentnode.removeChild(formrow.nextSibling)
 //remove firstrow
 formrowparentnode.removeChild(formrow)
 
}

//BIND
//////
function gds_bind(datasource,elements,rownx)
{
 
 login('bind')
 
 for (var propname in datasource)
 {
 
  //many rows
  if (propname.slice(0,5)=='group')
  {
  
   //locate the table
   var tablex=document.getElementById('neosys'+propname)
   if (!tablex) continue//table is not on form
   
   log('deleting existing rows')
   //delete existing rows except first (backwards from last)
   //var rows=tablex.getElementsByTagName('TBODY')[0].childNodes
   //!!!watch out the above will include any tables included in the header of the table
   var rows=tablex.tBodies[0].childNodes
   for (var rown=rows.length-1;rown>0;rown--)
   {
    rows[rown].parentNode.removeChild(rows[rown])
   }
   log('deleted existing rows')
   
   //insert and bind new rows   
   var rows=datasource[propname]
   var tbody=tablex.tBodies[0]
   
   //blank the first row and get a clone of it
   var firstrow=tbody.childNodes[0]
   this.blankrowx(rows[0],firstrow,false)
   if (!ismac) var emptyrow=firstrow.cloneNode(true)

//gtimers=new neosystimers(10)

   for (var rown=0;rows[rown];rown++)
   {
//gtimers.start(1)
    //for second row onwards: append a new row (copy first row)
    var rowx
    if (rown==0)
    {
     rowx=firstrow
    }
    else
    {

	 if (!ismac)
	 {
      var newrow=emptyrow.cloneNode(true)
      tbody.insertBefore(newrow,null)
     }
     else
     {
      //so do the following
      var newrow=tbody.insertBefore(document.createElement('TR'))
      //for (var coln=0;coln<firstrow.childNodes.length;coln++)
      for (var coln=firstrow.childNodes.length-1;coln>=0;coln--)
      {
       //var newnode=firstrow.childNodes[coln].cloneNode(true)
       var newnode=document.createElement('TD')
       newnode.innerHTML=firstrow.childNodes[coln].innerHTML
       //alert(firstrow.childNodes[coln].innerHTML+'\r\r\r'+newnode.innerHTML)
       //newnode.style.width=10
       if (newrow.childNodes.length)  newrow.insertBefore(newnode,newrow.childNodes[0])
       else newrow.insertBefore(newnode)
      }

     }
     
     rowx=newrow
     
    }
//gtimers.stop(1)

//gtimers.start(2)
    //setup even the first row to avoid a bug in mac ie5 where column widths are excessive
    setupnewrow(rowx,firstrow)
//gtimers.stop(2)

    if (ismac) window.setTimeout('fixmacinsertdeleteevent(1,'+rown+')',10)
    
//gtimers.start(3)
    //call bind data recursively to the new row
    this.bind(rows[rown],rowx.all,rown)
//gtimers.stop(3)

//    this.bind(rows[rown],rowx)
    
   }

//gtimers.display()
     
  }
  //one row or the header
  else
  {
  
   //locate the first element with the same datafld
   var element
   if (!elements)
   {
    element=document.getElementById(propname)
   }
   else
   {
    element=elements[propname]
   }
   if (!element) continue
//log('start')
   //for checkbox and radio buttons, bind to the first child
   if (element.length>0&&(element[0].type=='radio'||element[0].type=='checkbox')) element=element[0]

   //if the field is used in the form then
   //bind the element to the datasource and set the default
   if (element.id!=propname) continue
   
//    element.dataFld=element.id//fix mac bug?

   //bind the element to the datasource
   //and viceversa
   var dataitem=datasource[propname]
   dataitem.element=element
   element.dataitem=dataitem

   //default immediately for radio/checkbox and dropdown
   //or if literal default (starts with " or ')
   //do not default immediately for lines to avoid problem described in validateall()
   if (!element.neosysgroupno&&element.neosysdefaultvalue)
   {
   
    if (dataitem.text==''
        &&
        (element.type=='radio'
         ||
         element.type=='checkbox'
         ||
         element.tagName=='SELECT'
         ||
         (
          typeof element.neosysdefaultvalue=='string'
          &&
          '\'\"'.indexOf(element.neosysdefaultvalue.slice(0,1))>=0
         )
        )
       )
    {
     //if element.neosysdefaultvalue
     dataitem.text=getdefault(element)
//if (gro.data.group1[0].BILL.text=='0') xxxx
     dataitem.defaulted=true
    }
   
    //initialise not defaulted
    else
    {
     dataitem.defaulted=false
    }
    
   }
   else
   {
    //1st select option is the data
    if (!dataitem.text&&element.tagName=='SELECT')
     if (dataitem.text=element.childNodes.length) dataitem.text=element.childNodes[0].value
   }
   
//log('started')

   //save the original value
   var value=dataitem.text
   dataitem.oldtext=value
       
   //put the value into the screen
//log('value=')
    
   //conversions
   var conversion=element.neosysconversion
   if (value!=''&&typeof(conversion)=='string'&&conversion.substr(0,1)=='[')
   {
    value2=value.neosysoconv(conversion)
    if (value2==null)
    {
     raiseerror(1000,'in bind("'+element.id+'") '+neosysquote(value)+' is not a valid internal '+conversion.slice(1,-1)+' value')
     return
    }
    value=value2
   }
//log('converted')

   //update the screen
   //increase speed by not setting values emptyvalues
   if (value||rownx<=1) setvalue2(element,value)
  
  }
  
 }//next propname

 logout('bind')
 
}

//REBIND
////////
function gds_rebind(newdatasource,olddatasource,forced)
{

 //will fail a) if newdatasource has more group rows than the olddatasource

 login('rebind')
 
 if (!olddatasource)
 {
  olddatasource=this.data
  for (var groupno=1;groupno<100;groupno++)
  {
   var newrows=newdatasource['group'+groupno]
   var oldrows=olddatasource['group'+groupno]
   if (newrows&&oldrows)
   {
    for (var rown=0;newrows[rown];rown++)
    {
     //append a new row if necessary
     if (!oldrows[rown]) this.addrow(groupno)

     gds.rebind(newrows[rown],oldrows[rown])
     
    }
    
    //delete any excess rows
    //for (var rown=oldrows.length-1;rown>=newrows.length;rown--) this.deleterow(groupno,rown)
    if (oldrows.length>newrows.length) this.deleterow(groupno,newrows.length,oldrows.length-1)
    
   }
  }
 }
  
 for (var propname in newdatasource)
 {
 
  //recursive for groups
  if (propname.slice(0,5)=='group') return
  
  //update any changed values
  var newvalue=newdatasource[propname].text
  var oldcell=olddatasource[propname]
  if (forced||newvalue!=oldcell.text)
  {
//qqqq
//alert(propname+' '+newvalue+' '+oldcell.text)
   //if (oldcell.element&&oldcell.element.neosysfieldno)
    this.setrecord2([oldcell],[newvalue])
  }
  //initialise "oldtext"
  oldcell.oldtext=newvalue
  
 }//next propname
 
 logout('rebind')
  
}