// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

greportwindow=''

function reports_run(parameters,docid)
{
 
 if (!parameters)
 {
  grecn=getrecn(event.srcElement)
  parameters=gds.calcx('PARAMETERS',grecn)[0]
 }

 var data=reports_run_parameters(parameters)
 if (typeof data=='boolean') return false

 if (!docid) docid=gds.calcx('DOCUMENT_NO',grecn)[0]
  
 db.request=['EXECUTE','GENERAL','GETREPORT',docid].join('\r')
 if (!db.send(data)) return neosysinvalid(db.response)

 greportwindow=windowopen(db.data)
 
 return true

}
 
function reports_edit(key)
{
 
 if (!key)
 {
  grecn=getrecn(event.srcElement)
  key=gds.calcx('DOCUMENT_NO')[0]
 }
 if (key=='new') key=''

 //var action=neosysmodalwindow('../'+gparameters.module+'/'+gparameters.mode+'.htm',{'key':escape(key)})
 var action=neosysmodalwindow('../'+gparameters.module+'/'+gparameters.mode+'.htm',{'key':key})
 
 if (action)
 {

  //UPDATE, NEW, ACCESS, DELETE, undefined if they close the window by X)
  action=action.split(' ')
  
  //if (action[0]!='DELETE')
  if (action[0]=='UPDATE'||action[0]=='NEW')
  {

   var docid=action[1]
      
   if (1==neosysyesno('Run the report?\r',1))
   {
    var parameters=[]
    if (parameters.neosysread('DOCUMENTS',docid))
    {
     parameters=parameters[6].split(vm).join(rm)
     reports_run(parameters,docid)
    }
    else
    {
     neosysinvalid(parameters.neosysresponse)
    }
   }
  }
  
  if (action[0]!='ACCESS')
  {
   reports_refresh()

   //focus back on the report window after refreshing the screen  
   if (greportwindow)
   {
    window.setTimeout('greportwindow.focus();greportwindow=""',100)
   }
  }
  
 }
 
 return true

}

function reports_copy()
{

 grecn=getrecn(event.srcElement)

 db.request=['EXECUTE','GENERAL','COPYREPORT',gds.calcx('DOCUMENT_NO',grecn)[0]].join('\r')
 if (!db.send()) return neosysinvalid(db.response)

 reports_edit(db.data.split(fm)[0])
 
 reports_refresh()
 
 return true

}

function reports_delete()
{

 var alldocnos=gds.calcx('DOCUMENT_NO',null)
 
 var deleteflags=gds.calcx('DELETE_FLAG',null)
 var docnos=[]
 for (var i=0;i<alldocnos.length;i++)
 {
  if (deleteflags[i]) docnos[docnos.length]=alldocnos[i]+vm
 }

 if (!docnos.length) return neosysinvalid('Please check some reports to be deleted first')
 
 if (!neosysyesno('Are you SURE that you want to delete '+docnos.length+' '+'report'.neosysplural(docnos.length)+'?\r\rNote: This action is permanent and irreversible!')) return false
 
 db.request=['EXECUTE','GENERAL','DELETEREPORT',docnos.join(vm)].join('\r')
 if (!db.send()) return neosysinvalid(db.response)

 reports_refresh()
 
 return true

}

function reports_refresh()
{
 
 db.request=['EXECUTE','GENERAL','GETREPORTS',gparameters.COMMAND].join('\r')
 if (!db.send()) return neosysinvalid(db.response)
 
 gro.defaultrevstr=db.data
 closerecord_onclick()

 //save/ok button has no function
 saverecord.style.display='none'
 
 return true
 
}

function form_postinit()
{

 gparameters.savebuttonvalue=''
 
 reports_refresh()
 
 return true

}

