// Copyright NEOSYS All Rights Reserved.

var gimageextensions=['gif','jpg','jpeg','png','tif','tiff','bmp']
var gvideoextensions=['mpg','mpeg','avi','mov','mp4','3gp']
var gaudioextensions=['mp3','wav','mid','midi']
var gdocumentextensions=['doc','xls','pdf','ppt','txt','zip','psjpg', 'docx', 'xlsx', 'pptx', 'htm', 'html', 'mht', 'eml', 'msg']
var gaudiovisualextensions=[gimageextensions,gvideoextensions,gaudioextensions,gdocumentextensions].join(',').split(',')
var gexistingextensions=[]
var gexistingbasefilenames=[]
var gvirtualroot

//document.onkeydown=form_onkeydown
addeventlistener(document,'keydown','form_onkeydown')

function previewimage_sync(fileInput) {

    var img=document.getElementById("thumbnail");     
    img.src=''
    var img_size=document.getElementById("thumbnail_size_span");
    img_size.innerText=''
           
    var files = fileInput.files;
    if (!files)
        return false
    for (var i = 0; i < files.length; i++) {           
        var file = files[i];
        var imageType = /image.*/;     
        if (!file.type.match(imageType)) {
            continue;
        }
        img.file = file;
        var reader = new FileReader();
        reader.onload = (function(aImg) { 
            return function(e) { 
                aImg.src = e.target.result; 
            }; 
        })(img);
        reader.readAsDataURL(file);
    }
    window.setTimeout(function(){img_size.innerText=img.width+'x'+img.height},100) 
}
    
function* form_onkeydown(event) {

 event=getevent(event)

 if (event.keyCode==27)
  //return window.close()
  return exoduswindowclose()
}

function* formfunctions_onload() {

 //global references to key elements
 $images=$$('images')
 $form1=$$('form1')
 $table_upload=$$('table_upload')

 //document.onkeydown=form_onkeydown
 addeventlistener(document,'keydown','form_onkeydown')

 $button_submit=$$('button_submit')
 addeventlistener($button_submit,'click','upload_onclick')

 $button_showupload=$$('button_showupload')
 addeventlistener($button_showupload,'click','showuploadtable')

 $audiovisualextensionselement=$$('audiovisualextensionselement')

 $button_submit=$$('button_submit')
 //addeventlistener($button_submit,'click','upload_onclick')

 //option to upload images only (eg for signatures)
 if (gparameters.allowimages)
  gaudiovisualextensions=gimageextensions

 //option to limit file extensions
 if (gparameters.allowablefileextensions)
  gaudiovisualextensions=gparameters.allowablefileextensions.split(',')

 //doesnt seem to work on ff at least
 if (gaudiovisualextensions.join('')=='jpg')
    $form1.filedata.accept='image/jpeg'
    
 if (!gparameters.key)
    gparameters.key=''

 if (!gparameters.filename)
    gparameters.filename=''

 yield* loadimages()

 if (gparameters.showupload)
     yield* showuploadtable()

 return true

}

function* showuploadtable() {

 if (!(yield* exodussecurity('UPLOAD CREATE')))
    return yield* exodusinvalid(gmsg)

 $table_upload.style.display=''
 $form1.filedata.focus()
 $button_showupload.style.display='none'
 $audiovisualextensionselement.innerText='('+gaudiovisualextensions.join(' ')+')'
 //annoyingly the submit doesnt work in ie8 after automatic popup of browsing
 $form1.filedata.click()
}

function* audiovisual_open(event) {

 event=getevent(event)

 var imagen=event.target.getAttribute('exodusimagen')
 var element=$$('audiovisuallink'+imagen)
 if (element.click)
  element.click()
}

function* audiovisual_delete(event) {
 
 event=getevent(event)

 var filename=event.target.getAttribute('exodusfilename')
 
 //confirm!
 if (!(yield* exodusyesno(filename+'\rWarning! Are you SURE that you want to irrevocably delete this file permanently?\r\rNote: This is irreversible!',2))) return yield* exodusinvalid()
 
 db.request='EXECUTE\rGENERAL\rDELETEUPLOAD\r'+filename.toLowerCase()
 if (!(yield* db.send()))
  return yield* exodusinvalid(db.response)
  
 yield* loadimages()
 
}

function* upload_onclick() {

 //check file to upload entered
 var sourcefilename=$form1.filedata.value
 if (!sourcefilename) {
  exoduscancelevent()//prevent submit
  return yield* exodusinvalid('Please browse for a file name to upload first')
 }
 
 //split off the actual file name
 var sourcefilename=sourcefilename.split('/').slice(-1)[0].split('\\').slice(-1)[0]
 var extension=$form1.filedata.value.split('.').slice(-1)[0].toLowerCase()

 // following characters are not allowed in NTFS filenames / ? < > \ : * | "
 //and the server is probably NTFS
 //so prevent such files being uploaded (from non-NTFS workstations like mac and linux)
 // /[/<>:"\?\\\*\|]/g
 if (sourcefilename.match(/[/<>:"\?\\\*\|]/g)) {
  return yield* exodusinvalid('Sorry but the following characters are not allowed in file names being uploaded\r/ ? < > \\ : * &vert; "')
 }
 
 //check file extensions
 if (!gaudiovisualextensions.exoduslocate(extension))
  return yield* exodusinvalid('Files ending .'+extension+' are not allowed to be uploaded\r\rThe allowed file extensions are:\r\r'+gaudiovisualextensions.join(', '))

 var thumbnail=$$('thumbnail')
 if ((gparameters.maxheight || gparameters.minheight) && !thumbnail.height) {
    msg='Cannot determine image height for file '+sourcefilename
    return yield* exodusinvalid(msg)
 }
 if (gparameters.maxheight && thumbnail.height>gparameters.maxheight)
   var msg='too large'
 if (gparameters.minheight && thumbnail.height<gparameters.minheight)
   var msg='too small'
 if (msg) {
    msg='The image is '+msg+'. It is '+thumbnail.width + ' wide x ' + thumbnail.height+' high in pixels'
    msg+='\n\nThe image file must be between '+gparameters.minheight+' and '+gparameters.maxheight+' PIXELS HIGH'
    msg+='\n\nThere is no restriction on width.'
    return yield* exodusinvalid(msg)
 }
 
 /*if (gparameters.originalkeyversionno.slice(-1)!='\\')
 if (gparameters.originalkeyversionno.slice(-1)!='\\')
  gparameters.originalkeyversionno+='.'
 if (gparameters.versionno||!gparameters.key)
  var targetfilename=gparameters.originalkeyversionno+sourcefilename
 else
  var targetfilename=gparameters.originalkeyversionno+extension
 */

 var targetfilename=gparameters.originalkeyversionno+''
 
 //append a dot if the last character of the path is not a \ or .
 if ('\\.'.indexOf(targetfilename.substr(-1))<0)
  targetfilename+='.'
  
 if (gparameters.versionno||!gparameters.key)
  targetfilename+=sourcefilename
 else
  targetfilename+=extension

 var basefilename=targetfilename.split('\\').slice(-1)[0].split('/').slice(-1)[0].toLowerCase()
 //var mode=(gexistingextensions.exoduslocate(extension))?'UPDATE':'CREATE'
 var mode=(gexistingbasefilenames.exoduslocate(basefilename))?'UPDATE':'CREATE'

 var question='Uploading '+sourcefilename+' to '+targetfilename+'\r\r'

 //confirm update
 if (mode=='UPDATE') {
  if (!(yield* exodusyesno(question+'Warning! Are you SURE that you want to irrevocably overwrite the existing file permanently?\rThis action cannot be undone!',2)))
    return yield* exodusinvalid()
 } else {
  //if (!(yield* exodusokcancel(question+'\rOK to upload this file now?',1))) return yield* exodusinvalid()
 }
 
 //ensure folders are made because upload may not be able to make them
 //dont create if file is being loaded without any path (has slashes or backslashes)(like media images)
 if (gparameters.originalkeyversionno.match(/[\\/]/)) {

  db.request='EXECUTE\rGENERAL\rMAKEUPLOADPATH\r'+gparameters.originalkeyversionno
  +'\r'+gparameters.filename
  +'\r'+gparameters.key
  +'\r'+gparameters.ensurenotlocked
  
  if (!(yield* db.send()))
   return yield* exodusinvalid(db.response)
 }
 
 //check allowed to update/create
 //if (!(yield* exodussecurity('MATERIAL '+mode))) return yield* exodusinvalid(gmsg)
 
 //set the target filename
 $form1.filename.value=targetfilename
 
 //set the path
 
 //"htmlfile: Access is denied." error on .submit() in MSIE seems to be solved by removing name attribute on file field
 //but firefox REQUIRES name tag on file field
 //if (isMSIE)
 //   document.getElementById('filedata').removeAttribute('name')
 
 var temp=document.location.pathname.toString()
 //ensure prefixed by / (MSIE doesnt)
 //if (temp.substr(0,1)!='/')
 //   temp='/'+temp
 temp=temp.split('/')
 //remove the upload.htm and anything following
 temp=temp.slice(0,-1)
 var upload2htm=temp.join('/')+'/upload2.htm'
 temp=temp.slice(0,-1)
 //temp[temp.length-1]='images'
 //var virtualroot=gvirtualroot
 //if (gvirtualroot.slice(0,3)=='../') {
 // virtualroot=gvirtualroot.slice(3)
 var virtualroot=gvirtualroot.replace(/\\/g,'/');
 if (virtualroot.slice(0,3)=='../') {
  virtualroot=virtualroot.slice(3)
  temp=temp.slice(0,-1)
 }
 temp[temp.length]=virtualroot
 //$form1.pathdata.value="/exodus/images"
 $form1.pathdata.value='/'+temp.join('/')

 //reduce double slash prefix to single slash
 if ($form1.pathdata.value.slice(0,2)=='//')
  $form1.pathdata.value=$form1.pathdata.value.slice(1)

 //remove trailing slash
 if ($form1.pathdata.value.slice(-1)=='/')
  $form1.pathdata.value=$form1.pathdata.value.slice(0,-1)

 //set the custom response URL otherwise default response page is generated by the upload program
 //this page will signify success and add the file to the master (job/voucher etc) record etc
 //call yield* general_postupload() in this page for internet explorer browsers
 //call yield* general_postupload() in the calling page (jobs.htm/vouchers.htm etc) for non-internet explorer browsers
 $form1.redirectpage.value='/'+upload2htm+'?sourcefilename='+escape(sourcefilename)+'&targetpath='+escape($form1.pathdata.value)+'&autofit=true'
 //$form1.redirectpage.value='../upload2.htm?sourcefilename='+escape(sourcefilename)+'&targetpath='+escape(form1.pathdata.value)+'&autofit=true'
 if (gparameters.postuploadmessage)
    $form1.redirectpage.value+='&postuploadmessage='+gparameters.postuploadmessage
 $form1.redirectpage.value+='&remainder='//solve problem that it returns with two ? in the url for the returned values

 $form1.submit()

 return true
 
}

function* loadimages() {
  
 //prevent display all images if refreshing and no parameters
 if (typeof gparameters.database == 'undefined')
    return false
    
 //clear existing images
 $images.innerHTML=''
 
 //display the primary key and versionno
 //it may be overiden below if the key uses the same versionno as a previous key
 //if changed also change VERIFYUPLOAD in UPLOAD.SUBS unless verify can be moved/implemented here from general_add_archive somehow
 //gparameters.originalkeyversionno=gparameters.database+'\\UPLOAD\\'+gparameters.filename+'\\'+gparameters.key+'\\'+gparameters.key+'.'+gparameters.versionno
 //gparameters.originalkeyversionno=gparameters.database+'\\UPLOAD\\'+gparameters.filename+'\\'+gparameters.key+'\\'+gparameters.versionno
 var uploadtarget=''
 if (gparameters.database) {
  uploadtarget+=gparameters.database
  if (gparameters.database!='upload')
    uploadtarget+='\\upload'
  uploadtarget+='\\'
 }
 if (gparameters.filename)
  uploadtarget+=gparameters.filename.toLowerCase()+'\\'
 uploadtarget+=gparameters.key.toLowerCase().replace(/[\\\/\:\*\?\<\>\|]/g,'-').replace(/\"/g,"'")
 if (gparameters.versionno)
  uploadtarget+='\\'+gparameters.versionno

 //eg \devdtest\upload\programs\
 gparameters.originalkeyversionno=uploadtarget
 
 $form1.button_submit.value='Upload '+gparameters.originalkeyversionno
 
 //return yield* openwindow('EXECUTE\rGENERAL\rOPENUPLOAD',key+'.'+versionno)
 
 gexistingextensions=[]
 gexistingbasefilenames=[]
  
 db.request='EXECUTE\rGENERAL\rOPENUPLOAD\r'+gparameters.originalkeyversionno

 //NEW triggers deletion of any files already uploaded to the server but deleted on the record (by the user and within 24 hours)
 if (gparameters.newstatus)
    db.request+='\rNEW'

 if (!(yield* db.send()))
  return yield* exodusinvalid(db.response)

 var imagedata=(fm+db.data).split(fm)
 
 gvirtualroot=imagedata[1]

 //no images so show upload form
 if (!imagedata[2]) {

  yield* showuploadtable()
  return true
 }

 //update may be allowed 
 if (gparameters.updateallowed)
  $button_showupload.style.display=''
 
 //load image(s) into the page
 var imageURLs=imagedata[2].split(vm)
 for (imagen=0;imagen<imageURLs.length;imagen++) {
  
  //rule
  $images.insertBefore(document.createElement('hr'), null)
   
  var filename=imageURLs[imagen]
  //var filename=imageURLs[imagen].split('/').slice(-1)[0]
  //var filename=filename.split('\\').slice(-1)[0]
   
  var fileextension=imageURLs[imagen].split('.').slice(-1)[0]
  gexistingextensions[gexistingextensions.length]=fileextension
   
  var basefilename=filename.split('\\').slice(-1)[0].split('/').slice(-1)[0].toLowerCase()
  gexistingbasefilenames[gexistingbasefilenames.length]=basefilename

  //show a link
  
  var link=document.createElement('a')
  $images.insertBefore(link, null)
  //link.href='../../'+gvirtualroot+imageURLs[imagen]
  //make it work with url starting with /exodus/ etc ie non-default web site
  link.href='../'+gvirtualroot+escape(imageURLs[imagen].replace(/\\/g,'/'))
  link.innerText=filename
  link.id='audiovisuallink'+imagen

  //spacer
  var span=document.createElement('span')
  $images.insertBefore(span, null)
  span.innerHTML='&nbsp;&nbsp;'

  //and an Open button
  var button=document.createElement('input')
  button.type="button"
  button.className="exodusbutton"
  $images.insertBefore(button, null)
  button.value='Open'
  addeventlistener(button,'click','audiovisual_open')
  button.setAttribute('exodusimagen',imagen)
    
  //delete may be allowed
  if (gparameters.deleteallowed)
   yield* adddeletebutton(filename)
    
  //image
  if (gimageextensions.exoduslocate(fileextension)) {
 
   //br
   var br=document.createElement('br')
   $images.insertBefore(br, null)

   //show image
   var img=document.createElement('img')
   img.style.marginTop="5px"
   $images.insertBefore(img, null)
   img.src=link.href
 
  }
  
 }
 
 //rule
 $images.insertBefore(document.createElement('hr'), null)
   
 //P
 $images.insertBefore(document.createElement('p'), null).innerHTML='&nbsp;'
    
 return true
 
}

function* adddeletebutton(filename) {

 //add a delete button
 if (yield* exodussecurity('UPLOAD DELETE')) {

  var button=document.createElement('input')
  button.type="button"
  button.className="exodusbutton"
  button.value='Delete'
  $images.insertBefore(button, null)
  button.setAttribute('exodusfilename',filename)
  addeventlistener(button,'click','audiovisual_delete')
 }
}

//this function does not appear to be used and has a duplicate name function in general.js
//delete it if no complaints about the funny message
function* general_postupload() {

 alert('general_postload in upload.js\nInform EXODUS Support')
 if (gparameters.versionno||gparameters.closeafterupload)
  //return window.close()
  return exoduswindowclose()
 else
  yield* loadimages()
}
