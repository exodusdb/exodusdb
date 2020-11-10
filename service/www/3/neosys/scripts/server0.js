// Copyright NEOSYS All Rights Reserved.

//copied in xhttp.asp and server0

//debug in ASP like this ...
function* debug(msg,end) {

 Response.Write(msg)
 if (end) Response.End()
}

db=new neosysdblink

//see also neosysdblink in client.js for file access
function neosysdblink() {
 
 this.request=''
 this.data=''
 this.response=''
 
 //setup environment for file messaging
 
 //try and get the username, password and dataset from the session
 this.username=Session("username")
 this.password=Session("password")
 this.dataset=Session("dataset")
 this.system=Session("system")
 this.timeout=Session("timeout")
 
 //default timeout is 10 minutes (NB GIVEWAY timeout is hard coded to 10 mins?)
 var defaulttimeoutmins=10
 //var defaulttimeoutmins=.25
 if (!this.timeout) this.timeout=defaulttimeoutmins*60*1000
 
 //seconds for script timeout is our timeout plus 60 seconds
 Server.ScriptTimeout=this.timeout/1000+60
 
 //used in cache
 gdataset=this.dataset
 
 this.send = neosysdblink_send_byfile
 this.start = neosysdblink_startdb
 this.documentprotocolcode='HTTP'
 
 gfso = new ActiveXObject('Scripting.FileSystemObject')
 
 this.neosysrootpath=getneosysrootpath(Server.MapPath(Request.ServerVariables("PATH_INFO")))
 
 //localhostname
 //this.localhostname=new ActiveXObject('WScript.Network').ComputerName.slice(0,8)
 this.localhostname=new ActiveXObject('WScript.Network').ComputerName.toString()
 //remove space . " ' characters
 this.localhostname=this.localhostname.replace(/[\ |\.|\"|\']/g,'')
 //take first and last four characters if longer than 8 characters
 if (this.localhostname.length>8) this.localhostname=this.localhostname.slice(0,4)+this.localhostname.slice(-4)
 this.wscriptshell=new ActiveXObject('WScript.Shell')
}

function dbwaiting(){}
function dbready(){}
function wstatus(){}