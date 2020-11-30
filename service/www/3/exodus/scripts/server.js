// Copyright NEOSYS All Rights Reserved.
//copied in xhttp.asp and server.htm

//talk to application
//implemented as direct file messaging //
/////////////////////////////////////////

var gautostartdatabase = true//unless ..\exodus\NET.CFG first line is AUTOSTART=NO
var gsecondstowaitforreceipt = 10
var gmillisecondstowaitforstart = 30 * 1000

function getexodusrootpath(documentlocation) {

    //scan the parent folders for exodus and data folders
    //otherwise null

    documentlocation = unescape(documentlocation)

    //handle file:///c:/etc/etc
    if (documentlocation.slice(0, 8) == 'file:///') documentlocation = documentlocation.slice(8)
    //handle file://servername/etc/etc
    if (documentlocation.slice(0, 7) == 'file://') {
        documentlocation = documentlocation.slice(5)
        documentlocation = documentlocation.replace(/\//g, '\\')
    }

    if (documentlocation.indexOf('?') >= 0) documentlocation = documentlocation.slice(0, documentlocation.indexOf('?'))
    try {
        var folder = gfso.GetFile(documentlocation).ParentFolder
    }
    catch (e) {
        Response.Write(e.description)
        Response.End()
    }

    while (folder) {
        //check if a 'data' folder is present
        try {
            if (gfso.GetFolder(folder.Path + '\\data')) {

                //check that exodus is also available in the same folder
                if (gfso.GetFolder(folder.Path + '\\exodus')) {
                    exodusrootpath = folder.Path
                    //append a trailing backslash if necessary
                    if (exodusrootpath.slice(exodusrootpath.length - 1) != ":" && exodusrootpath.slice(exodusrootpath.length - 1) != "\\")
                        exodusrootpath = exodusrootpath + "\\"
                    return exodusrootpath
                }
            }
        } catch (e) { }

        //check if a 'exodus\data' folder is present
        try {
            if (gfso.GetFolder(folder.Path + '\\exodus\\data')) {

                //check that exodus is also available in the same folder
                if (gfso.GetFolder(folder.Path + '\\exodus\\exodus')) {
                    exodusrootpath = folder.Path
                    //append a trailing backslash if necessary
                    if (exodusrootpath.slice(exodusrootpath.length - 1) != ":" && exodusrootpath.slice(exodusrootpath.length - 1) != "\\")
                        exodusrootpath = exodusrootpath + "\\"
                    return exodusrootpath + 'exodus\\'
                }
            }
        } catch (e) { }

        //otherwise continue looking in higher folders
        folder = folder.ParentFolder

    }

    //cannot find database location
    return null

}

function xlog() {
}

function exodusdblink_send_byfile(data) {

    if (typeof gfso == 'undefined' || !gfso) return

    xlog(this.request)

    //try to use cache
    var request2 = this.request
    var trycache = (this.documentprotocolcode == 'file' && request2.slice(0, 6) == 'CACHE\r')
    if (trycache) {
        request2 = request2.slice(6)
        var temp
        if (temp = readcache(request2)) {
            this.data = temp
            return true
        }
    }

    var linkfile3exists = false

    this.data = data

    var dbwaitingwindow
    //dbwaitingwindow=dbwaiting()

    var cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER'.toUpperCase()
    var databasestoppedresponse = 'The EXODUS Service is stopped\r\rPlease try again later or ask your\rtechnical support staff to start it.'
    var norequestresponse = 'ERROR: NO REQUEST'.toUpperCase()
    var nodataresponse = 'ERROR: NO DATA'.toUpperCase()
    var invaliddatapathresponse = 'ERROR: INVALID DATA PATH'.toUpperCase()

    //fail if could not locate the database
    if (this.exodusrootpath == null) {
        this.response = cannotfinddatabaseresponse
        dbready(dbwaitingwindow)
        this.request = ''
        return 0
    }

    try {
        //var waiter = new ActiveXObject("Dynu.Wait")
        //the below line will cause a wait of 10 seconds
    }
    catch (e) { }

    var loginlocation = '../index.html'

    // try
    {
        //failure means that there was no response within the timeout period
        result = 0
        this.response = ''

        //fail if no request
        if (this.request == '') {
            this.response = norequestresponse
            dbready(dbwaitingwindow)
            this.request = ''
            return (0)
        }

        //special request without username or password
        //gets info without accessing exodus engine
        if (this.request.split('\r')[0] == 'GETDATASETS') {
            //  Response.Write(this.request)
            //  Response.End()
            var datasets = getdatasets(this.exodusrootpath, this.request.split('\r')[1])

            //error if no datasets
            if (datasets.length == 0) {
                this.data = ''
                this.response = 'Error: Cannot locate EXODUS engine or no dataset found'
                dbready(dbwaitingwindow)
                this.request = ''
                return (0)
            }

            //convert to xml 
            xmltext = '<records>\r'
            for (var i = 0; i < datasets.length; i++) {
                xmltext += '<record>\r'
                xmltext += '<name>' + datasets[i][0] + '</name>\r'
                xmltext += '<code>' + datasets[i][1] + '</code>\r'
                xmltext += '</record>\r'
            }
            xmltext += '</records>\r'

            this.data = xmltext
            this.response = 'OK'
            dbready(dbwaitingwindow)
            this.request = ''
            return (1)

        }

        //fail if GLOBAL.END file is present
        //doesnt examine ..\\..\\global.end so one can temporary stop/restart an installation
        //with nothing but possibly a slight pause to the end users.
        //if the restart is not within the timeout period 30 seconds? then they will
        //get a database not available message or depending on config the system will try to start another db process
        //TODO probably starting a db process should respect the higher global.end too
        try {
            proglocation = this.exodusrootpath + 'exodus\\'
            tf = gfso.GetFile(proglocation + 'GLOBAL.END')
            this.response = databasestoppedresponse
            dbready(dbwaitingwindow)
            this.request = ''
            return 0
        }
        catch (e) { }

        //maybe get new username, password and dataset from the request
        var requests = (this.request + '\r\r\r\r\r\r\r').split('\r')
        if (requests[0] == 'LOGIN') {
            this.username = requests[1]
            this.password = requests[2]
            this.dataset = requests[3]
            this.authno = requests[4]
            this.system = requests[5]
            this.request = requests[0]//remove the password from the request string

            //save the username, password and dataset for following sessions
            if (this.documentprotocolcode == 'file') {
                exodussetcookie(glogincode, 'EXODUS2', 'username', this.username)
                exodussetcookie(glogincode, 'EXODUS2', 'password', this.password)
                exodussetcookie(glogincode, 'EXODUS2', 'dataset', this.dataset)
                exodussetcookie('', 'EXODUSsystem', this.system)
            }
            else {
                Session('username') = this.username
                Session('password') = this.password
                Session('dataset') = this.dataset
                Session('system') = this.system
                //to be restored as follows in the 'on server' version of exodusdblink
            }
        }

        //check username is present - session may have timed out
        if (typeof (this.username) == 'undefined' || this.username == '') {
            this.response = 'Error: Please login - Session not established or timed out'
            dbready(dbwaitingwindow)
            this.request = ''
            return (0)
        }

        //check password is present
        if (typeof (this.password) == 'undefined' || this.password == '') {
            this.response = 'Error: Password parameter is missing'
            dbready(dbwaitingwindow)
            this.request = ''
            return (0)
        }

        //check dataset is present
        if (typeof (this.dataset) == 'undefined' || this.dataset == '') {
            this.response = 'Error: Dataset parameter is missing'
            dbready(dbwaitingwindow)
            this.request = ''
            return (0)
        }

        //append trailing '\' to dataset if necessary
        var datasetdir = this.dataset
        if (datasetdir.substr(datasetdir.length - 1) != ':' && datasetdir.substr(datasetdir.length - 1) != '\\') {
            datasetdir = datasetdir + '\\'
        }

        datalocation = (this.exodusrootpath + 'data\\').toUpperCase()
        //var flag_filename = datalocation + datasetdir + 'GLOBAL.SVR'
        var flag_filename = datalocation + datasetdir + this.dataset + '.SVR'

        //nodatabaseconnectionresponse='ERROR: Trying to start the EXODUS engine - please try again.\\n\\n"'+this.dataset+'"\\n\\n'+flag_filename

        /*
        //check that engine is active and start if not
        //////////////////////////////////////////////
  
        //wait for response
        var date1 = new Date()
        waituntil = date1.getTime() + (gmillisecondstowaitforstart / 1000 / 24 / 60 / 60)
        var loopcount=0

        thisloop:
        while (1) {
  
        //fail if no response within x seconds
        if (date1.getTime() > waituntil) {

        this.response = 'Error: Cannot start EXODUS engine for ' + this.dataset
        dbready(dbwaitingwindow)
        this.request=''
        return(0)
        }

        //
        //   //ok if db flag file exists and has been updated in the last 60secs
        //   try
        //   {
        //    var file=gfso.GetFile(flag_filename)
        //    var datelastmodified=file.DateLastModified
        //    //file.close
        //    file.Close()
        //    if (datelastmodified>=(date1.getTime()-60000))
        //    {
        //     break
        //    }
        //   }
        //   catch(e)
        //   {
        //    //suppress 'file not found' error
        //    if (e.number!=-2146828235) throw(e)
        //   }
        // 

        //ok if any db flag file exists and has been updated in the last 60secs
        var folder = gfso.GetFolder(datalocation+datasetdir)
        var files = new Enumerator(folder.files)
        for (;!files.atEnd();files.moveNext()) {

        var file=files.item()
    
        //skip if not a server flag file
        if (file.name.substr(file.name.length-4,4)!='.SVR') continue
    
        //break if file time is in last 60 seconds
        if (file.DateLastModified>=(date1.getTime()-60000)) break thisloop
    
        }

        //fail if not allowed to start database
        if (!autostartdatabase) {

        this.response='EXODUS.NET Service not started\rDataset: '+datasetdir.slice(0,-1)
        dbready(dbwaitingwindow)
        this.request=''
        return false
        }
    
        loopcount++
        if(loopcount==1) {

        //start database
        if (!(this.start()))
        //if (startresult.toString().split(' ')[0]!='OK')
        {
        //this.response='ERROR: '+startresult
        dbready(dbwaitingwindow)
        this.request=''
        return 0
        } 
        else {

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

        while (1) {

            nnorequestresponses++
            if (nnorequestresponses > 2) break

            if (this.response.toUpperCase() != norequestresponse && this.response.toUpperCase() != nodataresponse) {
                break
            }

            //make a random file name
            do {
                //~*.htm files are not backed up in FILEMAN
                linkfilename = '~' + exodusrnd(9999999, 1000000)
                linkfilename = datalocation + datasetdir + linkfilename
            } while (gfso.FileExists(linkfilename + '.*'))

            //write data if any
            //(before request so that there is not sharing violation in REV)
            if (this.data != '' && this.data != null) {
                try {
                    //,1 for unicode codepage conversion
                    var tf = gfso.CreateTextFile(linkfilename + '.2', 1)
                }
                catch (e) {
                    this.response = invaliddatapathresponse + ' "' + linkfilename + '.2" ' + e.description
                    dbready(dbwaitingwindow)
                    this.request = ''
                    return (0)
                }
                //tf.Write(escape(this.data))
                temp = this.data;
                //allow field marks to pass through unicode conversion untouched
                if (!unicode) temp = temp.replace(/\u00FE/g, '%FE').replace(/\u00FD/g, '%FD').replace(/\u00FC/g, '%FC').replace(/\u00FB/g, '%FB')

                try {
                    //unicode codepage conversion happens here
                    tf.Write(temp)
                }
                catch (e) {
                    var invalidchars = 'ERROR: Unacceptable characters in data'
                    if (e.number == -2146828283) this.response = invalidchars
                    else this.response = e.number + ' ' + e.description
                    return 0
                }

                tf.Close()
            }

            //write cmd (not as a .1 file otherwise the listener may
            //try to read it before it is ready
            try {
                var tf = gfso.CreateTextFile(linkfilename + '.1$', 1)
            }
            catch (e) {
                this.response = invaliddatapathresponse + ' "' + linkfilename + '.1$" ' + e.description
                dbready(dbwaitingwindow)
                this.request = ''
                return (0)
            }

            tf.WriteLine(linkfilename + '.1')

            //more info in request			
            if (!this.remoteaddr) this.remoteaddr = ''
            if (!this.remotehost) this.remotehost = ''
            if (!this.https) this.https = ''
            tf.WriteLine('VERSION 2')
            tf.WriteLine(this.remoteaddr)
            tf.WriteLine(this.remotehost)
            tf.WriteLine(this.https)

            tf.WriteLine(this.dataset)
            tf.WriteLine(this.username)
            tf.WriteLine(this.password)
            //tf.WriteLine(this.request)
            //allow field marks to pass through unicode conversion untouched
            temp = this.request.replace(/\u00FE/g, '%FE').replace(/\u00FD/g, '%FD').replace(/\u00FC/g, '%FC').replace(/\u00FB/g, '%FB')
            tf.WriteLine(temp)
            tf.Close()

            //show time and request on status
            if (this.request.indexOf('LOGIN') < 0) wstatus('Requested ' + new Date() + ' ' + this.request)

            //rename the command file to end in .1 so that the listener
            //pick it up
            tf = gfso.GetFile(linkfilename + '.1$')
            try {
                tf.Move(linkfilename + '.1')
            }
            catch (e) {
                this.response = 'Cannot rename "' + linkfilename + '.1$"\nto ' + linkfilename + '.1"\r' + e.description
                dbready(dbwaitingwindow)
                this.request = ''
                return (0)
            }
            tf.Close
            //tf.Close() !!!! why not???

            //qqq wait for the request to dissappear otherwise start the database
            while (gautostartdatabase && true) {

                //Sleep while the request file still exists every 10 ms for x seconds
                try {
                    var cmd = this.exodusrootpath + 'exodus\\waiting2.exe ' + linkfilename + '.1 ' + gsecondstowaitforreceipt + ' 10'
                    this.wscriptshell.Run(cmd, 0, true)
                }
                catch (e) {
                    this.response = e.number + ' ' + e.description + ' cannot run waiting2.exe'
                    dbready(dbwaitingwindow)
                    this.request = ''
                    return 0
                }

                try {
                    tf = gfso.GetFile(linkfilename + '.1')
                }
                catch (e) {
                    break
                }

                //start database otherwise fail
                if (!this.start()) {

                    //delete request file
                    try { gfso.DeleteFile(linkfilename + '.1') } catch (e) { }

                    //this.response='ERROR: '+startresult
                    dbready(dbwaitingwindow)
                    this.request = ''
                    return 0
                }

                break

            }

            //wait for response
            ///////////////////
            var date1 = new Date()
            //waituntil = date1.getTime() + (this.timeout / 1000 / 24 / 60 / 60)
            waituntil = date1.getTime() + this.timeout
            while (1) {

                //exit if no response within x seconds
                if ((new Date()).getTime() > waituntil) {
                    this.response = 'Error: No response in ' + (this.timeout / 1000) + ' seconds from database server at ' + linkfilename
                    dbready(dbwaitingwindow)
                    this.request = ''
                    return (0)
                }

                //looking for the response file every 10 ms for 5 seconds (uses sleep to save processor cycles)
                try {
                    var cmd = this.exodusrootpath + 'exodus\\waiting.exe ' + linkfilename + '.3' + ' 5 10'
                    this.wscriptshell.Run(cmd, 0, true)
                }
                catch (e) {
                    this.response = 'Error: ' + e.number + ' ' + e.description + '\rCannot run waiting.exe'
                    this.request = ''
                    return 0
                }

                var error = null
                try {
                    tf = gfso.OpenTextFile(linkfilename + '.3', 1)
                    linkfile3exists = true
                    //read the response
                    this.response = unescape(tf.ReadAll())
                    tf.Close() //this works
                    //tf.Close //this does not work
                }
                catch (e) {
                    error = e
                }

                if (error == null) {

                    //response determines true or false

                    //if response is not OK then do not look for or return any data
                    this.data = ''

                    //read the data if any
                    //(moved from below so can return data even if not "OK")
                    try {
                        tf = gfso.OpenTextFile(linkfilename + '.2', 1)
                        this.data = unescape(tf.ReadAll())
                        tf.Close()
                        //tf.Close
                    }
                    catch (e) {
                    }

                    if (this.response.split(' ')[0] == 'OK') {
                        result = 1
                        /*
                        //read the data if any
                        try {

                        tf = gfso.OpenTextFile(linkfilename + '.2', 1)
                        this.data = unescape(tf.ReadAll())
                        tf.Close()
                        //tf.Close
                        }
                        catch(e) {

                        }
                        */

                        //save in cache
                        //perhaps only cache READO requests to avoid
                        // initiating cache to speed things up
                        if (this.documentprotocolcode == 'file' && (trycache || request2.slice(0, 4) == 'READ')) {
                            writecache(request2, this.data)
                        }

                    }
                    else {
                        result = 0
                    }

                    break

                } //end of try to open response file

                //allow other processes to work\
                //like vbscript DoEvents
                //var ws=new ActiveXObject('WScript.???')
                //ws.Sleep(50)
                //if (waiter) waiter.Wait(.10)

            } //end of loop to wait for a response

        } //end of loop to wait for a non null response

    }

    //if direct access then convert relative reference to fullpath
    if (this.documentprotocolcode == 'file') {
        if (this.request.slice(0, 7) == 'EXECUTE' && this.data.slice(0, 3).toUpperCase() == '..\\') {
            this.data = this.exodusrootpath + this.data.slice(3)
        }
    }

    //clean up
    try { gfso.DeleteFile(linkfilename + '.1') } catch (e) { }
    try { gfso.DeleteFile(linkfilename + '.2') } catch (e) { }
    try { gfso.DeleteFile(linkfilename + '.3') } catch (e) {
        //perhaps write a file to indicate to database program to delete the .2 and .3
        //files in case the web browser is not able to delete them
        try {
            var tf = gfso.CreateTextFile(linkfilename + '.4', 1)
            tf.Close()
        }
        catch (e) { }
    }

    dbready(dbwaitingwindow)
    this.request = ''
    return result

}


///////////////////////////////////////////////////////////////
//routine to start database
//waits up to 60 seconds to get a positive or negative response
//then returns 'OK' or someother message to indicate failure
//negative response would be bad username, password or dataset
///////////////////////////////////////////////////////////////
function exodusdblink_startdb() {

    var cannotfinddatabaseresponse = 'ERROR: SERVER CONFIGURATION ERROR - CANNOT FIND DATABASE ON SERVER'.toUpperCase()
    if (this.exodusrootpath == null) {
        this.response = cannotfinddatabaseresponse
        return false
    }

    // var proglocation=getexodusrootpath()+'exodus'
    //proglocation='C:\\exodus\\'
    //var exodusrootpath=getexodusrootpath()
    proglocation = this.exodusrootpath + 'exodus\\'

    // var x=Session.CodePage
    // Session.CodePage=1252
    // this.localhostname.replace(/_/gi,'-')
    // this.localhostname.replace(/"|'| |./,'')
    //Session.CodePage=x

    //not needed as connectstring is passed in .RUN file
    //connectfile = proglocation + this.localhostname + '.$1'

    //build the connection string and filename
    connectstring = this.dataset + '\r\n' + this.username + '\r\n' + this.password + '\r\n' + this.authno + '\r\n' + this.system + '\r\n'

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

    //move to exodus program directory
    var currentdirectory = this.wscriptshell.CurrentDirectory
    if (currentdirectory != proglocation) {

        //check scripting engine version
        //scriptengineversion=ScriptEngineMajorVersion()+'.'+ScriptEngineMinorVersion()
        //if ((+scriptengineversion)<5.6)
        //{
        // this.response='Error: Scripting Version 5.6 is needed to change CurrentDirectory'
        // return 0
        //}

        try {
            this.wscriptshell.CurrentDirectory = proglocation
        }
        catch (e) {
            this.response = 'Error: Scripting Version 5.6 (included with IE6 or http://www.microsoft.com/scripting) is needed to change CurrentDirectory\rOR Cannot change Current Directory to ' + proglocation
            return 0
        }
    }

    //quit if autostart suppressed
    try {
        var file = gfso.OpenTextFile('NET.CFG', 1)
        var text = file.ReadAll()
        file.Close()
        if (text.split('\r')[0].toUpperCase() == 'AUTOSTART=NO') return true
    }
    catch (e) {
    }

    /*
    //kick off exodus
    //cmd=proglocation+'exodus.js /system '+this.system+' /dataset '+this.dataset+' /username '+this.username+' /password '+this.password
    cmd='AREV.EXE ADAGENCY,EXODUS /X'
    // cmd='WAITING.EXE AREV.EXE 3 3'
    taskid = this.wscriptshell.Run(cmd)
    Response.Write('error: '+currentdirectory)
    Response.End()
    */
    var runfilename = proglocation + this.system + '.RUN'
    var tf = gfso.CreateTextFile(runfilename, 1)
    tf.Write(connectstring)
    tf.Close()

    //restore current directory
    if (currentdirectory != proglocation) {
        this.wscriptshell.CurrentDirectory = currentdirectory
    }

    //prepare a response file
    var responsefilename = proglocation + this.localhostname + '.$2'

    //delete the response file
    if (!(osdelete(responsefilename, 'exodusdblink_startdb init'))) {
        //throw(e)
        this.response = gmsg
        return false
    }

    //wait for a response
    var date1 = new Date()
    waituntil = date1.getTime() + (gmillisecondstowaitforstart / 1000 / 24 / 60 / 60)

    while (1) {

        //fail if no response within x seconds
        if (date1.getTime() > waituntil) {

            //delete the .run request file
            try { gfso.DeleteFile(runfilename, 1) } catch (e) { }

            //not needed as connectstring is passed in .RUN file
            //delete the .$1 connection file
            //try{gfso.DeleteFile(connectfile,1)}catch(e){}

            this.response = 'Error: Cannot start EXODUS engine for ' + this.dataset
            return 0
        }

        //Sleep while looking for the response file every 500 ms for 10 seconds
        //var cmd=this.exodusrootpath+'exodus\\waiting.exe '+responsefilename + '.3'+' 10 100'
        try {
            var cmd = this.exodusrootpath + 'exodus\\waiting.exe ' + responsefilename + ' 10 500'
            this.wscriptshell.Run(cmd, 0, true)
        }
        catch (e) {
            this.response = 'Error: ' + e.number + ' ' + e.description + '\rCannot run waiting.exe'
            return 0
        }

        //try and get the response file text
        try {
            var file = gfso.OpenTextFile(responsefilename, 1)
            this.response = file.ReadAll()
            //file.Close //this does not work ... as you would expect
            file.Close()

            //delete the response file
            if (!(osdelete(responsefilename, 'exodusdblink_startdb exit'))) {
                //throw(e)
                this.response = gmsg
                return false
            }

            if (this.response.split('\r')[0] != 'OK') return false
            return true
        }
        catch (e) {
            //suppress 'file not found' error
            //if (e.number!=-2146828235) throw(e)

            //suppress 'file not found' and 'read past end of file' error
            if (e.number != -2146828235 && e.number != -2146828226) throw (e)
        }

    }

}

function exodusrnd(max, min) {
    //return a random integer between 0 (or min) and max-1
    if (!min) min = 0
    max = max - 1
    return (Math.floor(min + Math.random() * (max - min + 1)))
}

function exodusoswrite(string, filename) {
    tf = gfso.CreateTextFile(filename, 1)
    tf.Write(string)
    tf.Close()
}

function exodusosread(filename) {
    var text = ''
    try {
        var tf = gfso.OpenTextFile(filename)
        var text = tf.ReadAll()
        //tf.Close()
        tf.Close
    }
    catch (e) { }
    return (text)
}

var gmsg
function osdelete(filename, caller) {
    gmsg = ''
    try {
        gfso.DeleteFile(filename, 1)
    }
    catch (e) {
        //suppress 'file not found' and 'permission denied' error
        //if (e.number!=-2146828235&&e.number!=-2146828218)
        if (e.number != -2146828235) {
            //throw(e)
            gmsg = caller + ' cannot delete filename ' + filename + '\r' + e.description + ' ' + e.number
            return false
        }
    }
    return true
}

function getdatasets(exodusrootpath, systemcode) {

    //return an array of available dataset codes and names
    //or an empty array

    if (!systemcode) systemcode = 'adagency'

    //get an array of datasets
    //location of \exodus folder
    var proglocation = (exodusrootpath + 'exodus\\').toUpperCase()
    var datasets = exodusosread(proglocation + systemcode + '.vol')
    //.split('\r')[0]
    //backward compatible with old text format
    var fm = String.fromCharCode(254)
    var vm = String.fromCharCode(253)
    var sm = String.fromCharCode(252)
    datasets = datasets.replace(/\r/g, fm)
    datasets = datasets.replace(/\*/g, vm)
    datasets = datasets.replace(/,/g, sm)

    //strip off the firstline
    datasets = datasets.substr(0, datasets.indexOf(fm))

    //strip off the firstword
    datasets = datasets.substr(datasets.indexOf(' ') + 1)

    //split by vm
    datasets = datasets.split(vm)
    //subsplit by sm
    for (var i = 0; i < datasets.length; i++) {
        datasets[i] = (datasets[i] + sm + sm + sm + sm + sm + sm).split(sm)
        datasets[i][0] = datasets[i][0] + ' (' + datasets[i][1] + ')'
    }

    //strip out the datasets with no data
    var datalocation = (exodusrootpath + 'data\\').toUpperCase()
    var existingdatasets = []
    for (var i = 0; i < datasets.length; i++) {
        if (gfso.FileExists(datalocation + datasets[i][1] + '\\general\\revmedia.lk')) {
            existingdatasets[existingdatasets.length] = datasets[i]
        }
    }

    return existingdatasets


}
