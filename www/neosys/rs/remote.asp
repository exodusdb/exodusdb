<%@ Language=JavaScript %>

<%

//copy from/to _serverfunctions.js
//manually included because Microsoft SUS objects to INCLUDING a file with dots in its path

function dblink()
{

 this.request=''
 this.data=''
 this.response=''

 //setup environment for file messaging
 
 //try and get the username, password and dataset from the session
 this.username=Session("username")
 this.password=Session("password")
 this.dataset=Session("dataset")
 this.timeout=Session("timeout")

 //default timeout is 120 seconds
 if (!this.timeout)this.timeout=120*1000
 
 //used in cache
 gdataset=this.dataset
 
 this.send=dblink_send_byfile
 this.start=dblink_startdb
 this.documentprotocol='HTTP'
 
 gfso = new ActiveXObject('Scripting.FileSystemObject')
 
 this.neosysrootpath=getneosysrootpath(Server.MapPath(Request.ServerVariables("PATH_INFO")))
 
 //localhostname
 //remove space . " ' characters
 //and convert _ to -
 this.localhostname=new ActiveXObject('WScript.Network').ComputerName.slice(0,8)
 this.wscriptshell=new ActiveXObject('WScript.Shell')
 
}

function dbwaiting(){}
function dbready(){}















//copy this code to _clientfunctions.htm and _serverfunctions.htm

//talk to application
//implemented as direct file messaging //
/////////////////////////////////////////

function getneosysrootpath(documentlocation)
{
 
 //scan the parent folders for neosys and data folders
 //otherwise null
 
 documentlocation=unescape(documentlocation)
 if (documentlocation.slice(0,8)=='file:///') documentlocation=documentlocation.slice(8)
 if (documentlocation.indexOf('?')>=0)documentlocation=documentlocation.slice(0,documentlocation.indexOf('?'))
 var folder=gfso.GetFile(documentlocation).ParentFolder
 
 while(folder)
 {
  //check if a 'data' folder is present
  try
  {
   if (gfso.GetFolder(folder.Path+'\\data'))
   {
   
    //check that neosys is also available in the same folder
    if (gfso.GetFolder(folder.Path+'\\neosys'))
    {
     neosysrootpath=folder.Path
     //append a trailing backslash if necessary
     if (neosysrootpath.slice(neosysrootpath.length-1)!=":" && neosysrootpath.slice(neosysrootpath.length-1)!="\\")
      neosysrootpath = neosysrootpath + "\\"
     return neosysrootpath
    }
   }
  }catch(e){}
  
  //otherwise continue looking in higher folders
  folder=folder.ParentFolder
  
 }

 //cannot find database location
 return null
 
}

function log()
{
}

function dblink_send_byfile(data)
{

 log(this.request)
 
 this.data=data
 
 var dbwaitingwindow
 //dbwaitingwindow=dbwaiting()
 
 var cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER'.toUpperCase()
 var norequestresponse = 'ERROR: NO REQUEST'.toUpperCase()
 var nodataresponse = 'ERROR: NO DATA'.toUpperCase()
 var invaliddatapathresponse = 'ERROR: INVALID DATA PATH'.toUpperCase()
 
 //fail if could not locate the database
 if (this.neosysrootpath==null)
 {
  this.response = cannotfinddatabaseresponse
  dbready(dbwaitingwindow)
  this.request=''
  return 0
 }
 
 var loginlocation='../default.htm'
 
// try
 { 
  //failure means that there was no response within the timeout period
  result=0
  this.response=''
  
  //fail if no request
  if (this.request=='')
  {
   this.response = norequestresponse
   dbready(dbwaitingwindow)
   this.request=''
   return(0)
  }

  //special request without username or password
  //gets info without accessing neosys engine
  if (this.request.split('\r')[0]=='GETDATASETS')
  {
   var datasets=getdatasets(this.neosysrootpath)

   //error if no datasets
   if (datasets.length==0)
   {
    this.data=''
    this.response='Error: Cannot locate NEOSYS engine or no dataset found'
    dbready(dbwaitingwindow)
    this.request=''
    return(0)
   }
   
   //convert to xml 
   xmltext='<records>\r'
   for (var i=0;i<datasets.length;i++)
   {
    xmltext+='<record>\r'
    xmltext+='<name>'+datasets[i][0]+'</name>\r'
    xmltext+='<code>'+datasets[i][1]+'</code>\r'
    xmltext+='</record>\r'
   }
   xmltext+='</records>\r'
   
   this.data=xmltext
   this.response='OK'
   dbready(dbwaitingwindow)
   this.request=''
   return(1)

  }

  //maybe get new username, password and dataset from the request
  var requests=(this.request+'\r\r\r\r\r\r\r').split('\r')
  if (requests[0]=='LOGIN')
  {
   this.username=requests[1]
   this.password=requests[2]
   this.dataset=requests[3]
   this.authno=requests[4]
   this.request=requests[0]//remove the password from the request string
   
   //save the username, password and dataset for following sessions
   if (this.documentprotocol=='File Protocol')
   {
    setcookie(glogincode,'NEOSYS2','username',this.username)
    setcookie(glogincode,'NEOSYS2','password',this.password)
    setcookie(glogincode,'NEOSYS2','dataset',this.dataset)
   }
   else
   {
    Session('username')=this.username
    Session('password')=this.password
    Session('dataset')=this.dataset
    //to be restored as follows in the 'on server' version of dblink
   }
  }

  //check username is present - session may have timed out
  if (typeof(this.username)=='undefined'||this.username=='')
  {
   this.response='Error: Please login - Session not established or timed out.'
   dbready(dbwaitingwindow)
   this.request=''
   return(0)
  }
 
  //check password is present
  if (typeof(this.password)=='undefined'||this.password=='')
  {
   this.response='Error: Password parameter is missing'
   dbready(dbwaitingwindow)
   this.request=''
   return(0)
  }
 
  //check dataset is present
  if (typeof(this.dataset)=='undefined'||this.dataset=='')
  {
   this.response='Error: Dataset parameter is missing'
   dbready(dbwaitingwindow)
   this.request=''
   return(0)
  }
 
  //append trailing '\' to dataset if necessary
  var datasetdir=this.dataset
  if (datasetdir.substr(datasetdir.length-1)!=':' && datasetdir.substr(datasetdir.length-1)!='\\')
  {
   datasetdir = datasetdir + '\\'
  }

  datalocation = (this.neosysrootpath + 'data\\').toUpperCase()  
  var flagfilename=datalocation+datasetdir+'GLOBAL.SVR'
  
  //nodatabaseconnectionresponse='ERROR: Trying to start the NEOSYS engine - please try again.\\n\\n"'+this.dataset+'"\\n\\n'+flagfilename

  //check that engine is active and start if not
  //////////////////////////////////////////////
  
  //wait for response
  var date1 = new Date()
  var millisecondstowaitforstart=15*1000
  waituntil = date1.getTime() + (millisecondstowaitforstart / 1000 / 24 / 60 / 60)
  var loopcount=0
    
  while (1)
  {
  
    //fail if no response within x seconds
   if (date1.getTime() > waituntil)
   {
    this.response = 'Error: Cannot start NEOSYS engine for ' + this.dataset
    dbready(dbwaitingwindow)
    this.request=''
    return(0)
   }

   //ok if db flag file exists and has been updated in the last 30secs
   try
   {
    var file=gfso.GetFile(flagfilename)
    var datelastmodified=file.DateLastModified
    file.close
    if (datelastmodified>=(date1.getTime()-60000))
    {
     break
    }
   }
   catch(e)
   {
    //suppress 'file not found' error
    if (e.number!=-2146828235) throw(e)
   }

   db.response='NEOSYS.NET Service not started\rDataset: '+datasetdir.slice(0,-1)
   this.response=db.response
   dbready(dbwaitingwindow)
   this.request=''
   return (0)

   loopcount++
   if(loopcount==1)
   {
    var startresult=this.start()
    if (startresult.split(' ')[0]!='OK')
    {
     this.response='ERROR: '+startresult
     dbready(dbwaitingwindow)
     this.request=''
     return(0)
    } 
    else
    {
     //do not break, go round loop waiting for the 'listening' flag file to appear
     //break
    }
   }
   
  }

  //send the request to the database and wait for response
  //if receive 'ERROR: NO REQUEST' then send the request again
  ////////////////////////////////////////////////////////////
  nnorequestresponses = -1
  this.response = norequestresponse

  while (1)
  {

   nnorequestresponses++
   if (nnorequestresponses>2) break

   if(this.response.toUpperCase()!=norequestresponse && this.response.toUpperCase()!=nodataresponse)
   {
    break
   }
  
   //make a random file name
   do
   {
    linkfilename = '~' + randomInt(1000000,9999999)
    linkfilename = datalocation + datasetdir + linkfilename
   } while (gfso.FileExists(linkfilename + '.*'))
  
   //write data if any
   //(before request so that there is not sharing violation in REV)
   if (this.data!='' && this.data!=null)
   {
    try
    {
     var tf = gfso.CreateTextFile(linkfilename + '.2', 1)
    }
    catch (e)
    {
     this.response = invaliddatapathresponse + ' "' + linkfilename  + '.2" ' + e.description
     dbready(dbwaitingwindow)
     this.request=''
     return (0)
    }
	//tf.Write(escape(this.data))
	//allow field marks to pass through unicode conversion untouched
	temp=this.data.replace(/\u00FE/g,'%FE').replace(/\u00FD/g,'%FD').replace(/\u00FC/g,'%FC').replace(/\u00FB/g,'%FB')
				
				try
				{
					//unicode codepage conversion happens here
					tf.Write(temp)
				}
				catch (e)
				{
					var invalidchars='ERROR: Unacceptable characters in data'
					if (e.number==-2146828283) this.response=invalidchars
					else this.response=e.number+' '+e.description
					return 0
				}
				
    tf.Close()
   }
  
   //write cmd (not as a .1 file otherwise the listener may
   //try to read it before it is ready
   try
   {
    var tf = gfso.CreateTextFile(linkfilename + '.1$', 1)
   }
   catch (e)
   {
    this.response = invaliddatapathresponse + ' "' + linkfilename  + '.1" ' + e.description
    dbready(dbwaitingwindow)
    this.request=''
    return (0)
   }
   tf.WriteLine(linkfilename + '.1')
   tf.WriteLine(this.dataset)
   tf.WriteLine(this.username)
   tf.WriteLine(this.password)
   //tf.WriteLine(this.request)
   //allow field marks to pass through unicode conversion untouched
   temp=this.request.replace(/\u00FE/g,'%FE').replace(/\u00FD/g,'%FD').replace(/\u00FC/g,'%FC').replace(/\u00FB/g,'%FB')
   tf.WriteLine(temp)
   tf.Close()
   
   //rename the command file to end in .1 so that the listener
   //pick it up
   tf=gfso.GetFile(linkfilename + '.1$')
   tf.Move(linkfilename + '.1')
   tf.Close

   //wait for response
   var date1 = new Date()
   waituntil = date1.getTime() + (this.timeout / 1000 / 24 / 60 / 60)
   while (1)
   {

    //exit if no response within x seconds
    if (date1.getTime() > waituntil)
    {
     this.response = 'Error: No response from database server at ' + linkfilename
     dbready(dbwaitingwindow)
     this.request=''
     return(0)
    }
   
    var error=null

    try
    {
     tf = gfso.OpenTextFile(linkfilename + '.3', 1)
     //read the response
     this.response = tf.ReadAll()
     tf.Close()
    }
    catch(e)
    {
     error=e
    }
    
    if (error==null)
    {
   
     //response determines true or false
     
     //if response is not OK then do not look for or return any data
     this.data=''
     
     //read the data if any
     //(moved from below so can return data even if not "OK")
     try
     {
      tf = gfso.OpenTextFile(linkfilename + '.2', 1)
      this.data = unescape(tf.ReadAll())
      tf.Close()
     }
     catch(e)
     {
     }
     
     if (this.response.split(' ')[0]=='OK')
     {
      result = 1
/*
      //read the data if any
      try
      {
       tf = gfso.OpenTextFile(linkfilename + '.2', 1)
       this.data = tf.ReadAll()
       tf.Close()
      }
      catch(e)
      {
      }
*/      
     }
     else
     {
      result = 0
     }

     break 

    }//end of try to open response file

    //allow other processes to work\
    //vbscript DoEvents

   }//end of loop to wait for a response
   
  }//end of loop to wait for a non null response
 
 } 

 //if direct access then convert relative reference to fullpath
 if (this.documentprotocol=='File Protocol')
 {
  if (this.request.slice(0,7)=='EXECUTE'&&this.data.slice(0,8).toUpperCase()=='..\\DATA\\')
  {
   this.data=this.neosysrootpath+this.data.slice(3)
  }
 }
 
 //clean up
 try{gfso.DeleteFile(linkfilename + '.1')}catch(e){}
 try{gfso.DeleteFile(linkfilename + '.2')}catch(e){}
 try{gfso.DeleteFile(linkfilename + '.3')}catch(e){}
 
 dbready(dbwaitingwindow)
 this.request=''
 return(result)
 
}

///////////////////////////////////////////////////////////////
//routine to start database
//waits up to 60 seconds to get a positive or negative response
//then returns 'OK' or someother message to indicate failure
//negative response would be bad username, password or dataset
///////////////////////////////////////////////////////////////
function dblink_startdb()
{

 var millisecondstowaitforstart=15*1000

 var cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER'.toUpperCase()
 if (this.neosysrootpath==null)
 {
  this.response = cannotfinddatabaseresponse
  return false
 }
 
 // var proglocation=getneosysrootpath()+'neosys'
 //proglocation='C:\\neosys\\'
 //var neosysrootpath=getneosysrootpath()
 proglocation=this.neosysrootpath+'neosys\\'
 
// var x=Session.CodePage
// Session.CodePage=1252
// this.localhostname.replace(/_/gi,'-')
// this.localhostname.replace(/"|'| |./,'')
//Session.CodePage=x

 connectfile = proglocation + this.localhostname + '.$1'

 //build the connection string and filename
 connectstring=this.dataset+'\r\n'+this.username+'\r\n'+this.password+'\r\n'+this.authno
 //write connection string on connection file
 var tf = gfso.CreateTextFile(connectfile, 1)
 tf.Write(connectstring)
 tf.Close()
 
// gfso.ChDrive(proglocation.slice(0,1))
// gfso.ChDir(proglocation)
 
 batfilename = proglocation + this.localhostname + '.BAT'

 //write connection string on connection file
 var tf = gfso.CreateTextFile(batfilename, 1)
 tf.WriteLine('@echo off')
 tf.WriteLine(proglocation.slice(0,2))
 tf.WriteLine('CD '+proglocation)
 tf.WriteLine('adagency')
 tf.Close()

 taskid = this.wscriptshell.Run(batfilename)  
  
 //prepare a response file
 var responsefilename = proglocation + this.localhostname + '.$2'
 try{gfso.DeleteFile(responsefilename,1)}catch(e){}
 
 //wait for a response
 var date1 = new Date()
 waituntil = date1.getTime() + (millisecondstowaitforstart / 1000 / 24 / 60 / 60)

 while (1)
 {
  
   //fail if no response within x seconds
  if (date1.getTime() > waituntil)
  {
   this.response = 'Error: Cannot start NEOSYS engine for ' + this.dataset
   return(0)
  }

  //try and get the response file text
  try
  {
   var file=gfso.OpenTextFile(responsefilename,1)
   var result=file.ReadAll()
   file.Close()
   return(result)
  }
  catch(e)
  {
   //suppress 'file not found' error
   //if (e.number!=-2146828235) throw(e)
   
   //suppress 'file not found' and 'read past end of file' error
   if (e.number!=-2146828235&&e.number!=-2146828226) throw(e)
  }
   
 }
 
}

function randomInt(min,max)
{
     return(Math.floor(min+Math.random()*(max-min+1)))
}

function oswrite(string,filename)
{
 tf = gfso.CreateTextFile(filename, 1)
 tf.Write(string)
 tf.Close
}

function osread(filename)
{
 var text=''
 try
 {
  var tf = gfso.OpenTextFile(filename)
  var text=tf.ReadAll()
  tf.Close
 }
 catch (e){}
 return(text)
}

function getdatasets(neosysrootpath)
{

 //return an array of available dataset codes and names
 //or an empty array
 
 //get an array of datasets
 //location of \neosys folder
 var proglocation = (neosysrootpath + 'neosys\\').toUpperCase()
 //get the first line
 var datasets=osread(proglocation+'adagency.vol').split('\r')[0]
 //strip off the firstword
 datasets=datasets.substr(datasets.indexOf(' ')+1)
 //split by '*'
 datasets=datasets.split('*')
 //subsplit by ','
 for(var i=0;i<datasets.length;i++)
 {
  datasets[i]=(datasets[i]+',,,,').split(',')
 }
 
 //strip out the datasets with no data
 var datalocation = (neosysrootpath + 'data\\').toUpperCase()
 var existingdatasets=[]
 for(var i=0;i<datasets.length;i++)
 {
  if (gfso.FileExists(datalocation+datasets[i][1]+'\\general\\revmedia.lk'))
  {
   existingdatasets[existingdatasets.length]=datasets[i]
  }
 }
 
 return(existingdatasets)
 
}

%>














<%

 Response.Expires=0

 var db=new dblink()
  
 //extract the request parameters
 var currenttimeout=unescape(Request.Form('timeout'))
 var request=unescape(Request.Form('request'))
 var data=unescape(Request.Form('data'))
 
 //use default timeout if necessary
 if (currenttimeout!="") timeout=currenttimeout
 
 //send the request and data to the database
 var dbresult
 db.request=request
 try
 {
  dbresult=db.send(data)
 }
 catch(e)
 {
  dbresult=0
  db.response=e.description
  db.data=""
 }

%>
<HTML>
<HEAD>
</HEAD>
<BODY onload='window.parent.closeandreturn(unescape(form1.result.value),unescape(form1.response.value),unescape(form1.data.value))'>
<FORM id=form1>
<input name=result value="<%=escape(dbresult)%>">
<input name=response value="<%=escape(db.response)%>">
<input name=data value="<%=escape(db.data)%>">
</FORM>
</BODY>
</HTML>