// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

var gimageextensions=['gif','jpg','jpeg','tif','tiff','bmp']
var gvideoextensions=['mpg','mpeg','avi','mov']
var gaudioextensions=['mp3','wav','mid','midi']
var gdocumentextensions=['doc','xls','pdf','ppt','txt']
var gaudiovisualextensions=[gimageextensions,gvideoextensions,gaudioextensions,gdocumentextensions].join(',').split(',')
var gexistingextensions=[]

function formfunctions_onload()
{
 loadimages()
 return true
}

function showuploadtable()
{
 table_upload.style.display='inline'
 form1.filedata.focus()
 button_showupload.style.display='none'
 audiovisualextensionselement.innerText='('+gaudiovisualextensions.join(',')+')'
}

function audiovisual_open()
{
 var imagen=event.srcElement.neosysimagen
 document.getElementById('audiovisuallink'+imagen).click()
}

function audiovisual_delete()
{
 
 var filename=event.srcElement.neosysfilename
 
 //confirm!
 if (!neosysyesno(filename+'\rAre you SURE that you want to delete this file permanently?\rThis action cannot be undone!',2)) return neosysinvalid()
 
 db.request='EXECUTE\r\MEDIA\r\DELETEMATERIAL'
 if (!db.send(filename)) return neosysinvalid(db.response)
 loadimages()
 
}

function upload_onclick()
{

 //check file to upload entered
 var filename=form1.filedata.value
 if (!filename) return neosysinvalid('Please enter a file name to upload first')

 //split off the actual file name
 var filename=filename.split('/').slice(-1)[0].split('\\').slice(-1)[0]
 var extension=form1.filedata.value.split('.').slice(-1)[0].toLowerCase()

 //check file extensions
 if (!gaudiovisualextensions.neosyslocate(extension))
 {
  return neosysinvalid('Files ending .'+extension+' are not allowed to be uploaded\r\rThe allowed file extensions are:\r\r'+gaudiovisualextensions.join(', '))
 }

 var mode=(gexistingextensions.neosyslocate(extension))?'UPDATE':'CREATE'
 var targetfilename=gparameters.originalschedulematerial+'.'+extension
 var question='Uploading '+filename+' -> '+targetfilename+'\r\r'

 //confirm update
 if (mode=='UPDATE')
 {
  if (!neosysyesno(question+'Are you SURE that you want to overwrite the existing file permanently?\rThis action cannot be undone!',2)) return neosysinvalid()
 }
 else
 {
  //if (!neosysokcancel(question+'\rOK to upload this file now?',1)) return neosysinvalid()
 }
 
 //check allowed to update/create
 if (!neosyssecurity('MATERIAL '+mode)) return neosysinvalid(gmsg)
 
 //set the target filename
 form1.filename.value=targetfilename
 
 //set the path
 temp=document.location.pathname.toString().split('/')
 temp=temp.slice(0,-1)
 temp[temp.length-1]='images'
 //form1.pathdata.value="/neosys/images"
 form1.pathdata.value='/'+temp.join('/')
 
 //set the response URL
 form1.redirectpage.value="../upload2.htm"
 
 return true
 
}

function loadimages()
{

 //clear existing images
 images.innerHTML=''
 
 //display the primary schedule and material
 //it may be overiden below if the schedule uses the same material as a previous schedule
 gparameters.originalschedulematerial=gparameters.scheduleno+'.'+gparameters.materialcode
 form1.submit.value='Upload '+gparameters.originalschedulematerial
  
 //return openwindow('EXECUTE\r\MEDIA\r\OPENMATERIAL',scheduleno+'.'+materialcode)

 gexistingextensions=[]
  
 db.request='EXECUTE\r\MEDIA\r\OPENMATERIAL'
 if (db.send(gparameters.scheduleno+'.'+gparameters.materialcode))
 {

  //load image(s) into the page
  var imageURLs=db.data.split('\r')
  for (imagen=0;imagen<imageURLs.length;imagen++)
  {
  
   //rule
   images.insertBefore(document.createElement('HR'))
   
   var filename=imageURLs[imagen].split('/').slice(-1)[0]
   var filename=filename.split('\\').slice(-1)[0]
   var fileextension=imageURLs[imagen].split('.').slice(-1)[0]
   gexistingextensions[gexistingextensions.length]=fileextension
   
   //show a link
   {
    var link=document.createElement('A')
    link.href=imageURLs[imagen]
    link.innerText=filename
    link.id='audiovisuallink'+imagen
    images.insertBefore(link)
    
    //spacer
    var span=document.createElement('SPAN')
    span.innerHTML='&nbsp;&nbsp;'
    images.insertBefore(span)

    //and an Open button
    var button=document.createElement('BUTTON')
    button.value='Open'
    images.insertBefore(button)
    button.onclick=audiovisual_open
    button.neosysimagen=imagen
    
    //and a Delete button
    adddeletebutton(filename)
    
    //image
    if (gimageextensions.neosyslocate(fileextension))
    {
 
     //br
     var br=document.createElement('BR')
     images.insertBefore(br)

     //show image
     var img=document.createElement('IMG')
     img.src=imageURLs[imagen]
     images.insertBefore(img)
  
    }
       
   }

  }
  
  if (imageURLs.length)
  {
  
   //rule
   images.insertBefore(document.createElement('HR'))
   
   //P
   images.insertBefore(document.createElement('P')).innerHTML='&nbsp;'
   
  }
     
  return true
  
 }

 //return general error
 var temp=db.response.indexOf('IMAGE_NOT_FOUND')
 if (temp<0) return neosysinvalid(db.response)
 
 //image not found error ... allow upload
 
 //display the original material
 gparameters.originalschedulematerial=db.response.slice(temp+16)
 showuploadtable()
  
 return true
 
}

function adddeletebutton(filename)
{
 //add a delete button
 if (neosyssecurity('MATERIAL DELETE'))
 {
  var button=document.createElement('BUTTON')
  button.value='Delete'
  images.insertBefore(button)
  button.neosysfilename=filename
  button.onclick=audiovisual_delete
 }
}