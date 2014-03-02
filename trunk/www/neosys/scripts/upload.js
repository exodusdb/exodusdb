// © 2007 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

var gimageextensions=['gif','jpg','jpeg','png','tif','tiff','bmp']
var gvideoextensions=['mpg','mpeg','avi','mov','mp4','3gp']
var gaudioextensions=['mp3','wav','mid','midi']
var gdocumentextensions=['doc','xls','pdf','ppt','txt','zip','psjpg', 'docx', 'xlsx', 'pptx', 'htm', 'html', 'mht', 'eml']
var gaudiovisualextensions=[gimageextensions,gvideoextensions,gaudioextensions,gdocumentextensions].join(',').split(',')
var gexistingextensions=[]
var gexistingbasefilenames=[]
var gvirtualroot

document.onkeydown=form_onkeydown

function form_onkeydown(event)
{
 if (!event)
  event=window.event
 eventtarget=geteventtarget(event)

 if (event.keyCode==27)
  window.close()
}

function formfunctions_onload()
{

 //option to upload images only (eg for signatures)
 if (gparameters.allowimages)
  gaudiovisualextensions=gimageextensions

 //option to limit file extensions
 if (gparameters.allowablefileextensions)
  gaudiovisualextensions=gparameters.allowablefileextensions.split(',')

 if (!gparameters.key)
    gparameters.key=''

 if (!gparameters.filename)
    gparameters.filename=''

 //global references to key elements
 $images=$$('images')
 $form1=$$('form1')
 $table_upload=$$('table_upload')
 $button_showupload=$$('button_showupload')
 $audiovisualextensionselement=$$('audiovisualextensionselement')

 loadimages()

 if (gparameters.showupload)
     showuploadtable()

 return true

}

function showuploadtable()
{

 if (!neosyssecurity('UPLOAD CREATE'))
    return neosysinvalid(gmsg)

 $table_upload.style.display=''
 $form1.filedata.focus()
 $button_showupload.style.display='none'
 $audiovisualextensionselement.innerText='('+gaudiovisualextensions.join(' ')+')'
 //annoyingly the submit doesnt work in ie8 after automatic popup of browsing
 //$form1.filedata.click()
}

function audiovisual_open(event)
{
 if (!event)
  event=window.event
 eventtarget=geteventtarget(event)

 var imagen=eventtarget.getAttribute('neosysimagen')
 var element=$$('audiovisuallink'+imagen)
 if (element.click)
  element.click()
}

function audiovisual_delete(event)
{
 
 if (!event)
  event=window.event
 eventtarget=geteventtarget(event)

 var filename=eventtarget.getAttribute('neosysfilename')
 
 //confirm!
 if (!neosysyesno(filename+'\rAre you SURE that you want to delete this file permanently?\r\rNote: This is irreversible!',2)) return neosysinvalid()
 
 db.request='EXECUTE\r\GENERAL\r\DELETEUPLOAD\r'+filename
 if (!db.send())
  return neosysinvalid(db.response)
  
 loadimages()
 
}

function upload_onclick()
{

 //check file to upload entered
 var sourcefilename=$form1.filedata.value
 if (!sourcefilename)
  return neosysinvalid('Please browse for a file name to upload first')

 //split off the actual file name
 var sourcefilename=sourcefilename.split('/').slice(-1)[0].split('\\').slice(-1)[0]
 var extension=$form1.filedata.value.split('.').slice(-1)[0].toLowerCase()

 //check file extensions
 if (!gaudiovisualextensions.neosyslocate(extension))
  return neosysinvalid('Files ending .'+extension+' are not allowed to be uploaded\r\rThe allowed file extensions are:\r\r'+gaudiovisualextensions.join(', '))

 /*if (gparameters.originalkeyversionno.slice(-1)!='\\')
 if (gparameters.originalkeyversionno.slice(-1)!='\\')
  gparameters.originalkeyversionno+='.'
 if (gparameters.versionno||!gparameters.key)
  var targetfilename=gparameters.originalkeyversionno+sourcefilename
 else
  var targetfilename=gparameters.originalkeyversionno+extension
 */

 var targetfilename=gparameters.originalkeyversionno+''
 if ('\\.'.indexOf(targetfilename.slice(-1))<=0)
  targetfilename+='.'
 if (gparameters.versionno||!gparameters.key)
  targetfilename+=sourcefilename
 else
  targetfilename+=extension

 var basefilename=targetfilename.split('\\').slice(-1)[0].split('/').slice(-1)[0].toLowerCase()
 //var mode=(gexistingextensions.neosyslocate(extension))?'UPDATE':'CREATE'
 var mode=(gexistingbasefilenames.neosyslocate(basefilename))?'UPDATE':'CREATE'

 var question='Uploading '+sourcefilename+' to '+targetfilename+'\r\r'

 //confirm update
 if (mode=='UPDATE')
  if (!neosysyesno(question+'Are you SURE that you want to overwrite the existing file permanently?\rThis action cannot be undone!',2)) return neosysinvalid()
 else
 {
  //if (!neosysokcancel(question+'\rOK to upload this file now?',1)) return neosysinvalid()
 }
 
 //ensure folders are made because upload may not be able to make them
 //dont create if file is being loaded without any path (has slashes or backslashes)(like media images)
 if (gparameters.originalkeyversionno.match(/[\\/]/))
 {
  db.request='EXECUTE\rGENERAL\r\MAKEUPLOADPATH\r'+gparameters.originalkeyversionno
  +'\r'+gparameters.filename
  +'\r'+gparameters.key
  +'\r'+gparameters.ensurenotlocked
  
  if (!db.send())
   return neosysinvalid(db.response)
 }
 
 //check allowed to update/create
 //if (!neosyssecurity('MATERIAL '+mode)) return neosysinvalid(gmsg)
 
 //set the target filename
 $form1.filename.value=targetfilename
 
 //set the path
 temp=document.location.pathname.toString().split('/')
 //remove the upload.htm and anything following
 temp=temp.slice(0,-1)
 //temp[temp.length-1]='images'
 var virtualroot=gvirtualroot
 if (gvirtualroot.slice(0,3)=='../')
 {
  virtualroot=gvirtualroot.slice(3)
  temp=temp.slice(0,-1)
 }
 temp[temp.length]=virtualroot
 //$form1.pathdata.value="/neosys/images"
 $form1.pathdata.value='/'+temp.join('/')
 if ($form1.pathdata.value.slice(0,2)=='//')
  $form1.pathdata.value=$form1.pathdata.value.slice(1)
 if ($form1.pathdata.value.slice(-1)=='/')
  $form1.pathdata.value=$form1.pathdata.value.slice(0,-1)

 //set the custom response URL otherwise default response page is generated by the upload program
 //this page will signify success and add the file to the master (job/voucher etc) record etc
 //call general_postupload() in this page for internet explorer browsers
 //call general_postupload() in the calling page (jobs.htm/vouchers.htm etc) for non-internet explorer browsers
 $form1.redirectpage.value='../upload2.htm?sourcefilename='+escape(sourcefilename)+'&targetpath='+escape($form1.pathdata.value)+'&autofit=true'
 if (gparameters.postuploadmessage)
    $form1.redirectpage.value+='&postuploadmessage='+gparameters.postuploadmessage
 form1.redirectpage.value+='&remainder='//solve problem that it returns with two ? in the url for the returned values

 return true
 
}

function loadimages()
{
  
 //clear existing images
 $images.innerHTML=''
 
 //display the primary key and versionno
 //it may be overiden below if the key uses the same versionno as a previous key
 //if changed also change VERIFYUPLOAD in UPLOAD.SUBS unless verify can be moved/implemented here from general_add_archive somehow
 //gparameters.originalkeyversionno=gparameters.database+'\\UPLOAD\\'+gparameters.filename+'\\'+gparameters.key+'\\'+gparameters.key+'.'+gparameters.versionno
 //gparameters.originalkeyversionno=gparameters.database+'\\UPLOAD\\'+gparameters.filename+'\\'+gparameters.key+'\\'+gparameters.versionno
 var uploadtarget=''
 if (gparameters.database) {
  uploadtarget+=gparameters.database
  if (gparameters.database!='UPLOAD')
    uploadtarget+='\\UPLOAD'
  uploadtarget+='\\'
 }
 if (gparameters.filename)
  uploadtarget+=gparameters.filename+'\\'
 uploadtarget+=gparameters.key.replace(/[\\\/\:\*\?\<\>\|]/g,'-').replace(/\"/g,"'")
 if (gparameters.versionno)
  uploadtarget+='\\'+gparameters.versionno
 
 gparameters.originalkeyversionno=uploadtarget
 
 $form1.submit.value='Upload '+gparameters.originalkeyversionno
 
 //return openwindow('EXECUTE\r\GENERAL\r\OPENUPLOAD',key+'.'+versionno)
 
 gexistingextensions=[]
 gexistingbasefilenames=[]
  
 db.request='EXECUTE\r\GENERAL\r\OPENUPLOAD\r'+gparameters.originalkeyversionno
 if (!db.send())
  return neosysinvalid(db.response)

 var imagedata=(fm+db.data).split(fm)
 
 gvirtualroot=imagedata[1]

 //no images so show upload form
 if (!imagedata[2])
 {
  showuploadtable()
  return true
 }

 //update may be allowed 
 if (gparameters.updateallowed)
  $button_showupload.style.display=''
 
 //load image(s) into the page
 var imageURLs=imagedata[2].split(vm)
 for (imagen=0;imagen<imageURLs.length;imagen++)
 {
  
  //rule
  $images.insertBefore(document.createElement('HR'),null)
   
  var filename=imageURLs[imagen]
  //var filename=imageURLs[imagen].split('/').slice(-1)[0]
  //var filename=filename.split('\\').slice(-1)[0]
   
  var fileextension=imageURLs[imagen].split('.').slice(-1)[0]
  gexistingextensions[gexistingextensions.length]=fileextension
   
  var basefilename=filename.split('\\').slice(-1)[0].split('/').slice(-1)[0].toLowerCase()
  gexistingbasefilenames[gexistingbasefilenames.length]=basefilename

  //show a link
  
  var link=document.createElement('A')
  $images.insertBefore(link,null)
  link.href=gvirtualroot+imageURLs[imagen]
  link.innerText=filename
  link.id='audiovisuallink'+imagen
    
  //spacer
  var span=document.createElement('SPAN')
  $images.insertBefore(span,null)
  span.innerHTML='&nbsp;&nbsp;'

  //and an Open button
  var button=document.createElement('BUTTON')
  $images.insertBefore(button,null)
  button.value='Open'
  button.onclick=audiovisual_open
  button.setAttribute('neosysimagen',imagen)
    
  //delete may be allowed
  if (gparameters.deleteallowed)
   adddeletebutton(filename)
    
  //image
  if (gimageextensions.neosyslocate(fileextension))
  {
 
   //br
   var br=document.createElement('BR')
   $images.insertBefore(br,null)

   //show image
   var img=document.createElement('IMG')
   $images.insertBefore(img,null)
   img.src=gvirtualroot+imageURLs[imagen]
 
  }
  
 }
 
 //rule
 $images.insertBefore(document.createElement('HR'),null)
   
 //P
 $images.insertBefore(document.createElement('P'),null).innerHTML='&nbsp;'
    
 return true
 
}

function adddeletebutton(filename)
{

 //add a delete button
 if (neosyssecurity('UPLOAD DELETE'))
 {
  var button=document.createElement('BUTTON')
  button.value='Delete'
  $images.insertBefore(button,null)
  button.setAttribute('neosysfilename',filename)
  button.onclick=audiovisual_delete
 }
}

function general_postupload()
{
 if (gparameters.versionno||gparameters.closeafterupload)
  window.close()
 else
  loadimages()
}
