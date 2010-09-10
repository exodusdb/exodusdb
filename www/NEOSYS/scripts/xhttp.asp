<%@ LANGUAGE = javascript %>
<%
//<SCRIPT RUNAT=Server SRC=server0.js LANGUAGE=javascript></SCRIPT>
//<SCRIPT RUNAT=Server SRC=server.js LANGUAGE=javascript></SCRIPT>
%>
<%

// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

	//737 oldibmdosgreek
	//1253 windows greek
	//65001 unicode utf-8
//	Session.CodePage=1253
//	Response.CodePage=1253
//copied in xhttp.asp and server0

//debug in ASP like this ...
function debug(msg,resume)
{
	Response.Write(msg)
	if (!resume) Response.End()
}
//debug(Session.CodePage+' '+Response.CodePage)
//db=new neosysdblink

//see also neosysdblink in client.js for file access
function neosysdblink(token)
{
	this.request=''
	this.data=''
	this.response=''
	this.token=token
	
	//setup environment for file messaging
	
	//try and get the username, password and dataset from the session
	this.username=Session(this.token+'_username')
	this.password=Session(this.token+'_password')
	this.dataset=Session(this.token+'_dataset')
	this.system=Session(this.token+'_system')
	this.timeout=Session(this.token+'_timeout')
	this.portno=Session(this.token+'_portno')
	
	//default timeout is 10 minutes (NB BACKEND GIVEWAY timeout is hard coded to 10 mins?)
	var defaulttimeoutmins=10
	//if (this.username=='NEOSYS') defaulttimeoutmins=5
	if (!this.timeout) this.timeout=defaulttimeoutmins*60*1000

	//seconds for script timeout is our timeout plus 60 seconds
	Server.ScriptTimeout=this.timeout/1000+60

	//used in cache
	gdataset=this.dataset
    
	this.send=neosysdblink_send_byfile
	this.start=neosysdblink_startdb
	this.documentprotocolcode='HTTP'
	
	gfso = new ActiveXObject('Scripting.FileSystemObject')
	
	this.neosysrootpath=getneosysrootpath(Server.MapPath(Request.ServerVariables("PATH_INFO")))
	
	//localhostname
	//this.localhostname=new ActiveXObject('WScript.Network').ComputerName.slice(0,8)
	this.localhostname=new ActiveXObject('WScript.Network').ComputerName.toString()
	//remove space . " ' characters
	this.localhostname=this.localhostname.replace(/[\ |\.|\"|\']/gi,'')
	//take first and last four characters if longer than 8 characters
	if (this.localhostname.length>8) this.localhostname=this.localhostname.slice(0,4)+this.localhostname.slice(-4)
	this.wscriptshell=new ActiveXObject('WScript.Shell')
	
	this.remoteaddr=Request.ServerVariables("REMOTE_ADDR")
	this.remotehost=Request.ServerVariables("REMOTE_HOST")
	this.https=Request.ServerVariables("HTTPS")
	
}

function dbwaiting(){}
function dbready(){}
function wstatus(){}
%>
<%

//copied in xhttp.asp and server.htm

//talk to application
//implemented as direct file messaging //
/////////////////////////////////////////

var gautostartdatabase=true//unless ..\neosys\NET.CFG first line is AUTOSTART=NO
var gsecondstowaitforreceipt=10
var gmillisecondstowaitforstart=30*1000

function getneosysrootpath(documentlocation)
{
	
	//scan the parent folders for neosys and data folders
	//otherwise null
	
	documentlocation=unescape(documentlocation)

	//handle file:///c:/etc/etc
	if (documentlocation.slice(0,8)=='file:///') documentlocation=documentlocation.slice(8)
	//handle file://servername/etc/etc
	if (documentlocation.slice(0,7)=='file://')
	{
		documentlocation=documentlocation.slice(5)
		documentlocation=documentlocation.replace(/\//g,'\\')
	}
	
	if (documentlocation.indexOf('?')>=0)documentlocation=documentlocation.slice(0,documentlocation.indexOf('?'))
	try
	{
		var folder=gfso.GetFile(documentlocation).ParentFolder
	}
	catch(e)
	{
		Response.Write(e.description)
		Response.End()
	}
	
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
		
		//check if a 'neosys\data' folder is present
		try
		{
			if (gfso.GetFolder(folder.Path+'\\neosys\\data'))
			{
			
				//check that neosys is also available in the same folder
				if (gfso.GetFolder(folder.Path+'\\neosys\\neosys'))
				{
					neosysrootpath=folder.Path
					//append a trailing backslash if necessary
					if (neosysrootpath.slice(neosysrootpath.length-1)!=":" && neosysrootpath.slice(neosysrootpath.length-1)!="\\")
						neosysrootpath = neosysrootpath + "\\"
					return neosysrootpath+'neosys\\'
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

function neosysdblink_send_byfile(data)
{

	if (!gfso) return
	
	log(this.request)

	//try to use cache
	var request2=this.request
	var trycache=(this.documentprotocolcode=='file'&&request2.slice(0,6)=='CACHE\r')
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

	var linkfile3exists=false
		
	this.data=data
	
	var dbwaitingwindow
	//dbwaitingwindow=dbwaiting()
	
	var cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER'.toUpperCase()
	var databasestoppedresponse='The NEOSYS Service is stopped\r\rPlease try again later or ask your\rtechnical support staff to start it.'
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

	try
	{
		//var waiter = new ActiveXObject("Dynu.Wait")
		//the below line will cause a wait of 10 seconds
	}
	catch(e){}
				
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

		//get datasets for LOGIN screen
		//even for LOGIN so that we can determine the port number
		//as TCP port 5700+the dataset number (base zero)
		//this strategy has the defect that adding deleting datasets messes the port number
		//***port number connection not used at the moment due to slower speed under IIS
		var requests=(this.request+'\r\r\r\r\r\r\r\r').split('\r')
		if (requests[0]=='LOGIN'||requests[0]=='GETDATASETS')
		{
			var datasets=getdatasets(this.neosysrootpath,requests[requests[0]=='LOGIN'?5:1])
			if (!datasets) return 0
		}
		
		//special request without username or password
		//gets info without accessing neosys engine
		if (requests[0]=='GETDATASETS')
		{
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
			return 1

		}
		
		//fail if GLOBAL.END file is present
		try
		{
			proglocation=this.neosysrootpath+'neosys\\'
			tf=gfso.GetFile(proglocation+'GLOBAL.END')
			this.response = databasestoppedresponse
			dbready(dbwaitingwindow)
			this.request=''
			return 0
		}
		catch(e){}
			
		//maybe get new username, password and dataset from the request
		if (requests[0]=='LOGIN')
		{
		
			this.username=requests[1]
			this.password=requests[2]
			this.dataset=requests[3]
			this.authno=requests[4]
			this.system=requests[5]
			this.request=requests[0]//remove the password from the request string
			
			//work out the port number
			for(var ii=0;ii<datasets.length;ii++)
			{
				if (datasets[ii][1]==requests[3]) break
			}
			if (ii>=datasets.length)
			{
				this.data=''
				this.response='Error: Dataset "'+requests[3]+'"not found in dataset codes'
				dbready(dbwaitingwindow)
				this.request=''
				return(0)			 
			}
			this.portno=5700+ii
			
			//save the username, password and dataset for following sessions
			if (this.documentprotocolcode=='file')
			{
			    //should all be per token
				neosyssetcookie(glogincode,'NEOSYS2','username',this.username)
				neosyssetcookie(glogincode,'NEOSYS2','password',this.password)
				neosyssetcookie(glogincode,'NEOSYS2','dataset',this.dataset)
				neosyssetcookie('','NEOSYSsystem',this.system)
				neosyssetcookie(glogincode,'NEOSYS2','portno',this.portno)
			}
			else
			{
			
			    //verify!
			    if (this.token&&this.token!=this.dataset+'*'+this.username+'*') debug('token is '+this.token+' but should be '+this.dataset+'*'+this.username+'*')
			    
				Session(this.token+'_username')=this.username
				Session(this.token+'_password')=this.password
				Session(this.token+'_dataset')=this.dataset
				Session(this.token+'_system')=this.system
				Session(this.token+'_portno')=this.portno
				//to be restored as follows in the 'on server' version of neosysdblink
			}
		}

		//check username is present - session may have timed out
		if (typeof(this.username)=='undefined'||this.username=='')
		{
			this.response='Error: Please login - Session not established or timed out'
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
	
		//check system is present
		/*
		if (typeof(this.system)=='undefined'||this.system=='')
		{
			this.response='Error: System parameter is missing'
			dbready(dbwaitingwindow)
			this.request=''
			return(0)
		}
	*/
		//check port no is present
		if (typeof(this.portno)=='undefined'||this.portno=='')
		{
			this.response='Error: Port No parameter is missing'
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
		var flag_filename=datalocation+datasetdir+'GLOBAL.SVR'
		
		//nodatabaseconnectionresponse='ERROR: Trying to start the NEOSYS engine - please try again.\\n\\n"'+this.dataset+'"\\n\\n'+flag_filename

/*
		//check that engine is active and start if not
		//////////////////////////////////////////////
		
		//wait for response
		var date1 = new Date()
		waituntil = date1.getTime() + (gmillisecondstowaitforstart / 1000 / 24 / 60 / 60)
		var loopcount=0

thisloop:
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

			////ok if db flag file exists and has been updated in the last 60secs
			//try
			//{
			// var file=gfso.GetFile(flag_filename)
			// var datelastmodified=file.DateLastModified
			// //file.close
			// file.Close()
			// if (datelastmodified>=(date1.getTime()-60000))
			// {
			//  break
			// }
			//}
			//catch(e)
			//{
			// //suppress 'file not found' error
			// if (e.number!=-2146828235) throw(e)
			//}

			//ok if any db flag file exists and has been updated in the last 60secs
			var folder = gfso.GetFolder(datalocation+datasetdir)
			var files = new Enumerator(folder.files)
			for (;!files.atEnd();files.moveNext())
			{
				var file=files.item()
				
				//skip if not a server flag file
				if (file.name.substr(file.name.length-4,4)!='.SVR') continue
				
				//break if file time is in last 60 seconds
				if (file.DateLastModified>=(date1.getTime()-60000)) break thisloop
				
			}

			//fail if not allowed to start database
			if (!autostartdatabase)
			{
				this.response='NEOSYS.NET Service not started\rDataset: '+datasetdir.slice(0,-1)
				dbready(dbwaitingwindow)
				this.request=''
				return false
			}
				
			loopcount++
			if(loopcount==1)
			{
				//start database
				if (!this.start())
				//if (startresult.toString().split(' ')[0]!='OK')
				{
					//this.response='ERROR: '+startresult
					dbready(dbwaitingwindow)
					this.request=''
					return 0
				} 
				else
				{
					//do not break, go round loop waiting for the 'listening' flag file to appear
					//break
				}
			}

		}
*/

		//send the request to the database and wait for response
		//if receive 'ERROR: NO REQUEST' then send the request again
		////////////////////////////////////////////////////////////
		nnorequestresponses = -1
		this.response = norequestresponse

		//Response.Write(this.timeout)
		//Response.End()
			
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
                //~*.htm files are not backed up in FILEMAN
				linkfilename = '~' + neosysrnd(9999999,1000000)
				linkfilename0=linkfilename
				linkfilename = datalocation + datasetdir + linkfilename
			} while (gfso.FileExists(linkfilename + '.*'))
		
			//write data if any
			//(before request so that there is not sharing violation in REV)
			if (this.data!='' && this.data!=null)
			{
				try
				{
					//,1 for unicode codepage conversion
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
				temp=this.data.replace(/\u0025/g,'%25').replace(/\u00FE/g,'%FE').replace(/\u00FD/g,'%FD').replace(/\u00FC/g,'%FC').replace(/\u00FB/g,'%FB')
				try
				{
					//unicode codepage conversion happens here
					tf.Write(temp)
				}
				catch (e)
				{
				
					//convert unicode characters to %u9999 except current codepage
					//to avoid failure to convert to current codepage
					var unicodefrom
					var unicodeto
					switch (Session.CodePage)
					{
					case 1253: {unicodefrom=767;unicodeto=767+256;break}//0300
					case 1256: {unicodefrom=1535;unicodeto=1535+256;break}//0600
					}
					
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
				this.response = invaliddatapathresponse + ' "' + linkfilename  + '.1$" ' + e.description
				dbready(dbwaitingwindow)
				this.request=''
				return (0)
			}

			tf.WriteLine(linkfilename + '.1')

			//more info in request			
			if (!this.remoteaddr) this.remoteaddr=''
			if (!this.remotehost) this.remotehost=''
			if (!this.https) this.https=''
			tf.WriteLine('VERSION 2')
			tf.WriteLine(this.remoteaddr)
			tf.WriteLine(this.remotehost)
			tf.WriteLine(this.https)
			
			tf.WriteLine(this.dataset)
			tf.WriteLine(this.username)
			tf.WriteLine(this.password)
			//tf.WriteLine(this.request)
			//allow field marks to pass through unicode conversion untouched
			temp=this.request.replace(/\u0025/g,'%25').replace(/\u00FE/g,'%FE').replace(/\u00FD/g,'%FD').replace(/\u00FC/g,'%FC').replace(/\u00FB/g,'%FB')
			tf.WriteLine(temp)
			tf.Close()
			
			//show time and request on status
			if (this.request.indexOf('LOGIN')<0) wstatus('Requested '+new Date()+' '+this.request)
			
			//rename the command file to end in .1 so that the listener
			//pick it up
			tf=gfso.GetFile(linkfilename + '.1$')
			try
			{
				tf.Move(linkfilename + '.1')
			}
			catch(e)
			{
				this.response = 'Cannot rename "' + linkfilename  + '.1$"\nto ' + linkfilename  + '.1"\r' + e.description
				dbready(dbwaitingwindow)
				this.request=''
				return (0)    
			}
			tf.Close
			//tf.Close() !!!! why not???
			
			var TCP1
			//var hostname="localhost"
			var hostname="127.0.0.1"
			var portno=this.portno//5700
			var message=linkfilename0+'.1'
			var reply
			var buffer

			timex=new Date

			/*
			//SockLite object
			//WScript.echo('Creating Object')
			TCP1=Application('TCP1B')
			if (!TCP1)
			{
				//TCP1=new ActiveXObject('Socket.TCP')
				TCP1=new ActiveXObject('SockLite.SocketLite')
				Application('TCP1B')=TCP1
			// Response.Write('NEW')
			// Response.End()
			}
			
			//WScript.echo('Connecting to Host '+hostname+':'+portno)
			hSocket = TCP1.ConnectSock(hostname, portno)
			if (hSocket==-1)
			{
					this.response = 'Cannot connect to '+hostname+':'+portno
					dbready(dbwaitingwindow)
					this.request=''
					return 0
			}
			
			//WScript.echo('Sending message "'+message+'"')
			bytessent=TCP1.SendData(hSocket,message)
			if (bytessent==0)
			{
					this.response = 'Cannot connect to '+hostname+':'+portno
					dbready(dbwaitingwindow)
					this.request=''
					return 0
			}
			//Response.Write(message+' sent')
			//Response.End()
			
			//WScript.echo('Closing Socket')
			TCP1.CloseSock(hSocket)
			
			//WScript.echo('Destroying object')
			TCP1=null
			//Response.Write(new Date-timex)
			//Response.End()
			*/
			
			/*
			
			// Socket.TCP object
			
			//WScript.echo('Creating Object')
			TCP1=Application('TCP2')
			if (!TCP1)
			{
				TCP1=new ActiveXObject('Socket.TCP')
			// Application('TCP2')=TCP1
			}
			
			//WScript.echo('Connecting to Host '+hostname+':'+portno)
			TCP1.Host=hostname+':'+portno
			TCP1.Open()
			
			//WScript.echo('Sending message "'+message+'"')
			TCP1.SendLine(message)
			
			//WScript.echo('Receiving reply')
			//reply=TCP1.GetLine()
			//WScript.echo('Reply was '+reply)
			
			//wait for disconnection method
			//TCP1.WaitForDisconnect()
			
			//buffer=TCP1.Buffer
			//WScript.echo('Buffer is '+buffer)
			
			//WScript.echo('Closing Socket')
			TCP1.Close()
			
			//WScript.echo('Destroying object')
			TCP1=null

*/

			//qqq wait for the request to dissappear otherwise start the database
			//Sleep while the request file still exists every 10 ms for x seconds
			try
			{
//				var cmd=this.neosysrootpath+'neosys\\waiting4.exe '+linkfilename+'.1 '+gsecondstowaitforreceipt+' 10'
				var cmd=this.neosysrootpath+'neosys\\waiting2.exe '+linkfilename+'.1 '+gsecondstowaitforreceipt+' 10'
				cmd+=' '+hostname+' '+portno+' '+message
				cmd+=' '+this.neosysrootpath+' '+' '+this.system+' '+this.dataset+' '+this.username+' '+this.password+' '+gautostartdatabase
//				cmd="d:\\neosys\\neosys\\monitor.exe"
				this.wscriptshell.Run(cmd,0,true)
//Response.Write("monitor run")
//Response.End()
			}
			catch (e)
			{
				this.response=e.number+' '+e.description+' cannot run waiting2.exe'
				dbready(dbwaitingwindow)
				this.request=''
				return 0
			}

//Response.Write(cmd)
//Response.End()

			//zzz
			//really should break if cannot connect on TCP/IP otherwise will hang without hope of file being renamed
			//or perhaps the seconds to wait for receipt should be as long as this.timeout
			
			//option to start new database process if request not taken within x seconds
			while (gautostartdatabase&&true)
			{

				try
				{
					tf=gfso.GetFile(linkfilename + '.1')
				}
				catch(e)
				{
					break
				}

				//start database otherwise fail
				if (!this.start())
				{

					//delete request file
					try{gfso.DeleteFile(linkfilename + '.1')}catch(e){}
					
					//this.response='ERROR: '+startresult
					dbready(dbwaitingwindow)
					this.request=''
					return 0
				}

				break
				
			}
//Response.Write(new Date-timex)
//Response.End()
			
			//wait for response
			///////////////////
			var date1 = new Date()
			//waituntil = date1.getTime() + (this.timeout / 1000 / 24 / 60 / 60)
			waituntil = date1.getTime() + this.timeout
			while (1)
			{

				//exit if no response within x seconds
				if ((new Date).getTime() > waituntil)
				{
					this.response = 'Error: No response in '+(this.timeout/1000)+' seconds from database server at ' + linkfilename
					dbready(dbwaitingwindow)
					this.request=''
					return(0)
				}

				//looking for the response file every 10 ms for 5 seconds (uses sleep to save processor cycles)
				try
				{
					var cmd=this.neosysrootpath+'neosys\\waiting.exe '+linkfilename + '.3'+' 5 10'
					this.wscriptshell.Run(cmd,0,true)
				}
				catch (e)
				{
					this.response='Error: '+e.number+' '+e.description+'\rCannot run waiting.exe'
					this.request=''
					return 0
				}
				
				var error=null
				try
				{
					tf = gfso.OpenTextFile(linkfilename + '.3', 1)
					linkfile3exists=true
					//read the response
					this.response = unescape(tf.ReadAll())
					tf.Close() //this works
					//tf.Close //this does not work
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
						//strangely it doesnt seem to matter what format (unicode/ascii/sysdefault)
						//you open it as (third parameter)
						tf = gfso.OpenTextFile(linkfilename + '.2', 1)
						this.data = unescape(tf.ReadAll())
						tf.Close()
						//tf.Close
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
							this.data = unescape(tf.ReadAll())
							tf.Close()
							//tf.Close
						}
						catch(e)
						{
						}
*/      

						//save in cache
						//perhaps only cache READO requests to avoid
						// initiating cache to speed things up
						if (this.documentprotocolcode=='file'&&(trycache||request2.slice(0,4)=='READ'))
						{
							writecache(request2,this.data)
						}
		
					}
					else
					{
						result = 0
					}

					break 

				}//end of try to open response file

				//allow other processes to work\
				//like vbscript DoEvents
				//var ws=new ActiveXObject('WScript.???')
				//ws.Sleep(50)
				//if (waiter) waiter.Wait(.10)
				
			}//end of loop to wait for a response
			
		}//end of loop to wait for a non null response
	
	}//end of loop to try

	//if direct access then convert relative reference to fullpath
	if (this.documentprotocolcode=='file')
	{
		if (this.request.slice(0,7)=='EXECUTE'&&this.data.slice(0,3).toUpperCase()=='..\\')
		{
			this.data=this.neosysrootpath+this.data.slice(3)
		}
	}
	
	//clean up
	try{gfso.DeleteFile(linkfilename + '.1')}catch(e){}
	try{gfso.DeleteFile(linkfilename + '.2')}catch(e){}
	try{gfso.DeleteFile(linkfilename + '.3')}catch(e)
	{
		//perhaps write a file to indicate to database program to delete the .2 and .3
		//files in case the web browser is not able to delete them
		try
		{
			var tf = gfso.CreateTextFile(linkfilename + '.4', 1)
			tf.Close()
		}
		catch (e){}
	}
	
	dbready(dbwaitingwindow)
	this.request=''
	return result
	
}


///////////////////////////////////////////////////////////////
//routine to start database
//waits up to 60 seconds to get a positive or negative response
//then returns 'OK' or someother message to indicate failure
//negative response would be bad username, password or dataset
///////////////////////////////////////////////////////////////
function neosysdblink_startdb()
{

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

	//not needed as connectstring is passed in .RUN file
	//connectfile = proglocation + this.localhostname + '.$1'

	//build the connection string and filename
	connectstring=this.dataset+'\r\n'+this.username+'\r\n'+this.password+'\r\n'+this.authno+'\r\n'+this.system+'\r\n'
	
	//not needed as connectstring is passed in .RUN file
	//write connection string on connection file
	//var tf = gfso.CreateTextFile(connectfile, 1)
	//tf.Write(connectstring)
	//tf.Close()
	
/* 
	batfilename = proglocation + this.localhostname + '.BAT'

	//write connection string on connection file
	var tf = gfso.CreateTextFile(batfilename, 1)
	tf.WriteLine('@echo off')
	tf.WriteLine(proglocation.slice(0,2))
	tf.WriteLine('CD '+proglocation)
	tf.WriteLine('adagency')
	tf.Close()
	
	var cmd=batfilename
	
	gfso.ChDrive(proglocation.slice(0,1))
	gfso.ChDir(proglocation).slice(2)

*/

	//move to neosys program directory
	var currentdirectory=this.wscriptshell.CurrentDirectory
	if (currentdirectory!=proglocation)
	{

		//check scripting engine version
		//scriptengineversion=ScriptEngineMajorVersion()+'.'+ScriptEngineMinorVersion()
		//if ((+scriptengineversion)<5.6)
		//{
		// this.response='Error: Scripting Version 5.6 is needed to change CurrentDirectory'
		// return 0
		//}
		
		try
		{
			this.wscriptshell.CurrentDirectory=proglocation
		}
		catch(e)
		{
			this.response='Error: Scripting Version 5.6 (included with IE6 or http://www.microsoft.com/scripting) is needed to change CurrentDirectory\rOR Cannot change Current Directory to '+proglocation
			return 0
		}
	}
	
	//quit if autostart suppressed
	try
	{
		var file=gfso.OpenTextFile('NET.CFG',1)
		var text=file.ReadAll()
		file.Close()
		//if (text.split('\r')[0].toUpperCase()=='AUTOSTART=NO') return true
		if (text.toUpperCase().indexOf('AUTOSTART=NO')>=0) return true
	}
	catch(e)
	{
	}
			
/*
	//kick off neosys
	//cmd=proglocation+'neosys.js /system '+this.system+' /dataset '+this.dataset+' /username '+this.username+' /password '+this.password
	cmd='AREV.EXE ADAGENCY,NEOSYS /X'
// cmd='WAITING.EXE AREV.EXE 3 3'
	taskid = this.wscriptshell.Run(cmd)
	Response.Write('error: '+currentdirectory)
	Response.End()
*/
	var runfilename=proglocation+this.system+'.RUN'
	var tf = gfso.CreateTextFile(runfilename,1)
	tf.Write(connectstring)
	tf.Close()

	//restore current directory
	if (currentdirectory!=proglocation)
	{
		this.wscriptshell.CurrentDirectory=currentdirectory
	}
	
	//prepare a response file
	var responsefilename = proglocation+this.localhostname+'.$2'

	//delete the response file
	if (!osdelete(responsefilename,'neosysdblink_startdb init'))
	{
		//throw(e)
		if (gmsg.indexOf('2146828218')>=0) gmsg='Not authorised to start database service'
		this.response=gmsg
		return false
	}
	
	//wait for a response
	var date1 = new Date()
	waituntil = date1.getTime() + (gmillisecondstowaitforstart / 1000 / 24 / 60 / 60)

	while (1)
	{
		
			//fail if no response within x seconds
		if (date1.getTime() > waituntil)
		{
		
			//delete the .run request file
			try{gfso.DeleteFile(runfilename,1)}catch(e){}
			
			//not needed as connectstring is passed in .RUN file
			//delete the .$1 connection file
			//try{gfso.DeleteFile(connectfile,1)}catch(e){}
			
			this.response = 'Error: Cannot start NEOSYS engine for ' + this.dataset
			return 0
		}

		//Sleep while looking for the response file every 500 ms for 10 seconds
		//var cmd=this.neosysrootpath+'neosys\\waiting.exe '+responsefilename + '.3'+' 10 100'
		try
		{
			var cmd=this.neosysrootpath+'neosys\\waiting.exe '+responsefilename+' 10 500'
			this.wscriptshell.Run(cmd,0,true)
		}
		catch (e)
		{
			this.response='Error: '+e.number+' '+e.description+'\rCannot run waiting.exe'
			return 0
		}

		//try and get the response file text
		try
		{
			var file=gfso.OpenTextFile(responsefilename,1)
			this.response=file.ReadAll()
			//file.Close //this does not work ... as you would expect
			file.Close()
			
			//delete the response file
			if (!osdelete(responsefilename,'neosysdblink_startdb exit'))
			{
				//throw(e)
				this.response=gmsg
				return false
			}
	
			if (this.response.split('\r')[0]!='OK') return false
			return true
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

function neosysrnd(max,min)
{
	//return a random integer between 0 (or min) and max-1
	if (!min) min=0
	max=max-1
	return(Math.floor(min+Math.random()*(max-min+1)))
}

function neosysoswrite(string,filename)
{
	tf = gfso.CreateTextFile(filename, 1)
	tf.Write(string)
	tf.Close()
}

function neosysosread(filename)
{
	var text=''
	try
	{
		var tf = gfso.OpenTextFile(filename)
		var text=tf.ReadAll()
		//tf.Close()
		tf.Close
	}
	catch (e){}
	return(text)
}

var gmsg
function osdelete(filename,caller)
{
	gmsg=''
	try
	{
		gfso.DeleteFile(filename,1)
	}
	catch(e)
	{
		//suppress 'file not found' and 'permission denied' error
		//if (e.number!=-2146828235&&e.number!=-2146828218)
		if (e.number!=-2146828235)
		{
			//throw(e)
			gmsg=caller+' cannot delete filename '+filename+'\r'+e.description+' '+e.number
			return false
		}
	}
	return true
}
	
function getdatasets(neosysrootpath,systemcode)
{

	//return an array of available dataset codes and names
	//or an empty array

	if (!systemcode) systemcode='adagency'
		
	//get an array of datasets
	//location of \neosys folder
	var proglocation = (neosysrootpath + 'neosys\\').toUpperCase()
	//get the first line
	var datasets=neosysosread(proglocation+systemcode+'.vol').split('\r')[0]
	//strip off the firstword
	datasets=datasets.substr(datasets.indexOf(' ')+1)
	//split by '*'
	datasets=datasets.split('*')
	//subsplit by ','
	for(var i=0;i<datasets.length;i++)
	{
		datasets[i]=(datasets[i]+',,,,').split(',')
		datasets[i][0]=datasets[i][0]+' ('+datasets[i][1]+')'
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
	
	return existingdatasets
	
}
%>
<%

	//load the posted XML document
	var requestx = Server.CreateObject("Microsoft.XMLDOM");
	requestx.load(Request)
	if (requestx.parseError.errorCode!=0)
	{
		var myErr = requestx.parseError;
		 debug("You have error " + myErr.reason,true);
	}
//if (requestx.xml.indexOf('VAL.CONTROL')>=0) debug(requestx.xml,true)
	//extract the request parameters
	var token=unescape(requestx.firstChild.childNodes[0].text)
	var currenttimeout=unescape(requestx.firstChild.childNodes[1].text)
	var request=unescape(requestx.firstChild.childNodes[2].text)
	var data=unescape(requestx.firstChild.childNodes[3].text)
	
	//use default timeout if necessary
	if (currenttimeout!="") timeout=currenttimeout
	
	//use this for testing or keeping session alive
	if (this.request.slice(0,9)=='KEEPALIVE')
	{
		db=new Object
		db.data='KEEPALIVE'
		db.response='OK'
		dbresult=true
	}
	else
	{
		//send the request and data to the database
		var db=new neosysdblink(token)
		db.request=request
		var error=null 
		try
		{
			var dbresult=db.send(data)
		}
		catch(e)
		{
			db.response=e.description
			db.data=""
		}
	}
		
	//package the response in xml text format
	//zzz maybe faster to send as xml object?
	xmltext="<root>"
	xmltext+="<data>" + escape(db.data) + "</data>"
	xmltext+="<response>" + escape(db.response) + "</response>"
	xmltext+="<result>" + escape(dbresult) + "</result>"
	xmltext+="</root>"

	Response.ContentType="text/xml"
	Response.Expires=-1000
	Response.Buffer=0
	//Response.Write("<?xml version='1.0' encoding='ISO-8859-1'?>")
	Response.Write(xmltext)
	
%>
