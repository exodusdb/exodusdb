//Copyright NEOSYS All Rights Reserved.
//undocumented functions
//CollectGarbage() in all versions
//"finally" block in v5.0

//mozilla differences from IE (out of date?)
// HTML element has a parentNode which is not usual element so be careful in "ancestor finder" routines
// elements cannot be referred to simply as variables like idx
// (must use getElementById or set an equivalent global variable)
// insertBefore secondparameter is mandatory (put null)
// document.body cannot be referred to in heading scripts until the document body is started
// (so put document.onload function in the body tag. onload function cannot be called onload)
// selectelement.options() should
//parentElement nonstandard use parentNode

//default location of exodus scripts etc
if (typeof EXODUSlocation == 'undefined')
    EXODUSlocation = '../exodus/'

//define global variables and initialise some of them
//many global variables are defined throughout the code outside functions ... ususally just preceeding them
//some global variable might be defined inside function by assignment without a defining var
//most global variables start with g except some very commonly used ones like fm, vm, sm
//a more complete set of global variables may be found using a debugger

//var gautofitwindow = true
var gisdialog
var glocked//mainly required in dbform but used in logout

var gdomainname = window.location.toString().split('/')[2]
//if (document.cookie.indexOf('username') < 0)
//    alert('qno username')

var geventlog = ''
var geventstack = ['', '']
var geventdepth = 1
var gonunload = false
var gonbeforeunload = false

var glogging, gstepping

var glogevents//related to yield
var glogcookie
var glogsettimeout

//various images
var gimagetheme = '../../exodus/images/theme2/'
//var gmenuimage=gimagetheme+'menu.png'//'add.png'
var gmenuimage = gimagetheme + 'menu.gif'//'add.gif'
var glogoutimage = gimagetheme + 'disconnect.png'//'add.png'
var grefreshimage = gimagetheme + 'refresh.png'

var gcache

var glogincode
var gDialogArguments//similar to window.dialogArguments

var gusername
var gdataset
var gsystem

var gcompanycode
var gperiod
var gbasecurr
var gbasefmt
var gthousands_regex
var gmarketcode
var gmaincurrcode
var gdatasetname
var gtz = [0, 0]
gtz[0] = new Date().getTimezoneOffset() * -60

var gnpendingscripts = 0

//load gparameters from gDialogArguments if present
var gparameters

//var grecn should only be used in dbform.js. used here in ICONV special case

//Revelation/Pick field separator characters
var rm, fm, vm, sm, tm, stm
var STMre, TMre, SMre, VMre, FMre, RMre, ALLFMre
var XMLXXre
var FMs

isMSIE = typeof ActiveXObject != 'undefined' || ('ActiveXObject' in window)//|| is for MSIE11
isMac = navigator.appVersion.indexOf('Macintosh') >= 0

var e//mac safari 3.1.2 cannot tolerate "catch(e)" without this

var goriginalstyles = {}

//prevent framing?
//if (window != top)
//    top.location.href = location.href

var gkeepalivemins = 10
//gkeepalivemins=1

//check browser capabilities

var gunsupported = ''
var gswitchingbrowser = false

//support "innerText" or fail
if (typeof document.createElement('div').innerText == 'undefined') {
    if (typeof HTMLElement == 'undefined' || !HTMLElement.prototype || !HTMLElement.prototype.__defineGetter__)
        gunsupported += ' innerText()'
    else {
        //for more see http://dean.edwards.name/moz-behaviors/src/
        //for moz/safari
        HTMLElement.prototype.__defineGetter__("innerText", function () { return this.textContent; });
        HTMLElement.prototype.__defineSetter__("innerText", function ($value) { this.textContent = $value; });
        /*
        if(typeof HTMLElement!="undefined"){
        HTMLElement.prototype.innerText getter = function(){
        var tmp = this.innerHTML.replace(/<br />/gi,"\n");
        return tmp.replace(/<[^>]+>/g,"");
        }

        HTMLElement.prototype.innerText setter = function(txtStr){
        var parsedText = document.createTextNode(txtStr);
        this.innerHTML = "";
        this.appendChild( parsedText );
        }
        }
        */
    }
}
//define if our source contains function * and yield * statements
//HARD CODED DEPENDING ON PRESENCE OR NOT OF YIELD STATEMENTS IN SOURCE CODE
//AND THEREFORE CANNOT BE CHANGED
/* yield */ var guseyield = true
// noyield // var guseyield=false
var gyieldregex = /yield ?\*/g

//determine if yield * supported by browser (not used anywhere atm)
var gcan_yield
try { eval('function * gcan_yield(x){return yield * true;}'); gcan_yield = true } catch (e) { gcan_yield = false }

var gcan_showmodaldialog = window.showModalDialog

//this forces pages going to /2/ to switch to /3/ which might be important for backend generated links to /2/ eg exodus/dblink.htm
//simulate no modal dialog in firefox as will happen in mid 2015-2016
//apparently it is not actually blocked in firefox yet (2016) but, without special configuration, user is given annoying options to suppress the popup
if (gcan_yield)
    gcan_showmodaldialog = false

//this code only executes in /2/ ie javascript without yield option
//cannot continue in non-yielding code if browser does not support showmodaldialog
//maybe could be used to inform user/support team
//that the user could use alternative web site supporting yield (if detected gcan_yield above)
if (!guseyield) {
    if (!gcan_showmodaldialog) {
        //switch to exodus/3 if browser hasnt showModalDialog but can yield
        if (gcan_yield) {

            var url = window.location.toString()
            window.location = url.replace(/\/[12]\//, '/3/')
            gswitchingbrowser = true
            //return // no return in global code
        } else
            gunsupported += ' showModalDialog() or yield'
    }
}

//call remainder of global code only if not switching browser
if (!gswitchingbrowser) {
    exodus_client_init()
}

//this is only called if not switching browser
//any global variable defined in this function must not of course be declare var here otherwise would be local function variables
function exodus_client_init() {

    //actually this is only needed if exodusforms are used
    if (!document.getElementsByClassName && !document.all)
        gunsupported += ' getElementsByClassName or .all'

    //check right browser and cookies allowed else switch to login which handled this error
    if (gunsupported && !gswitchingbrowser) {
        alert('Sorry, your web browser does not support EXODUS.\nUse Internet Explorer 6+, Safari 3+, Firefox 3+ or Chrome 8.0+\n\nIt doesnt support' + gunsupported)
        //window.location.assign(EXODUSlocation+'wrongbrowser.htm')
        if (typeof glogin == 'undefined') {
            window.location.assign(EXODUSlocation + '../index.html')
            return
        }
    }

    //implement swapNode if not native
    if (!document.swapNode) {
        Node.prototype.swapNode = function (node) {
            var p = node.parentNode;
            var s = node.nextSibling;
            this.parentNode.replaceChild(node, this);
            p.insertBefore(this, s);
            return this;
        }
    }

    //polyfill
    if (typeof console == 'undefined')
        window.console = {}
    if (typeof console.log == 'undefined')
        window.console.log = log

    //setup function $$() NOT $() since that is used by JQuery
    if (document.getElementsByClassName)
        $$ = $class
    else
        $$ = $all

    document.protocolcode = document.location.toString().slice(0, 4)

    //global constants for revelation high end separator characters

    //new style flexible but chose unusual characters not likely to be required by clients and yet visible for debugging
    //characters can be changed if required by require a change in the http server message handler too

    //c0,c1, f5-ff do not occur in utf8 at all.
    //see http://www.ifi.unizh.ch/mml/mduerst/papers/PDF/IUC11-UTF-8.pdf for graphical layout of utf8
    //00-7f means one ascii byte
    //80-BF means is a trailing byte
    //c0-c1 is illegal
    //c2-df is a two byte sequence first byte
    //e0-ef is a three byte sequence first byte
    //f0-f4 is a four byte sequence first byte
    //c0,c1, f5-ff do not occur in utf8 at all.
    //0xxxxxxx (US-ASCII)
    //110xxxxx 10xxxxxx
    //1110xxxx 10xxxxxx 10xxxxxx
    //11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    //111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
    //1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

    //using NKO code page 07FA-07FF for our 6 field separator characters

    //prestored global regular expressions for speed
    //STMre = /\u07FA/g
    //TMre = /\u07FB/g
    //SMre = /\u07FC/g
    //VMre = /\u07FD/g
    //FMre = /\u07FE/g
    //RMre = /\u07FF/g
    //ALLFMre = /[\u07FA\u07FB\u07FC\u07FD\u07FE\u07FF]/g
    STMre = /\x1A/g
    TMre = /\x1B/g
    SMre = /\x1C/g
    VMre = /\x1D/g
    FMre = /\x1E/g
    RMre = /\x1F/g
    ALLFMre = /[\x1A\x1B\x1C\x1D\x1E\x1F]/g

    //\x0A-\x0F in the back end
    //(no longer) also block character F8 and F9
    //F9 is used as a field mark in DOS eg PRINT ("XXX":\F9\:"YYY") 'L#20' ... formats both to 20 characters
    //also block character F8 since to make logical block of eight field mark characters
    //XMLXXre = /([\x25\x3C\x3E\x26\u07FF\u07FE\u07FD\u07FC\u07FB\u07FA\u07F9\u07F8])/g
    //FMs = '\u07FF\u07FE\u07FD\u07FC\u07FB\u07FA\u07F9\u07F8'
    //XMLXXre = /([\x25\x3C\x3E\x26\u07FF\u07FE\u07FD\u07FC\u07FB\u07FA])/g
    //FMs = '\u07FF\u07FE\u07FD\u07FC\u07FB\u07FA'
    XMLXXre = /([\x25\x3C\x3E\x26\x1F\x1E\x1D\x1C\x1B\x1A])/g
    FMs = '\x1F\x1E\x1D\x1C\x1B\x1A'

    //calculate all the global field separator character variables
    rm = FMs[0]
    fm = FMs[1]
    vm = FMs[2]
    sm = FMs[3]
    tm = FMs[4]
    stm = FMs[5]

    //get alternative global dialog arguments - see dialogArgumentsForChild in function exodusshowmodaldialog
    gDialogArguments = window.dialogArguments
    if (!gDialogArguments && window.opener) {
        try {
            //window.opener may have been closed and refreshing the page might cause an error
            gDialogArguments = window.opener.dialogArgumentsForChild
        }
        catch (e) { }
    }

    gisdialog = typeof window.dialogArguments != 'undefined'

    if (window.dialogArguments && window.dialogArguments.logincode) {
        glogincode = window.dialogArguments.logincode
        //alert('debug client.js glogincode=window.dialogArguments.logincode '+window.dialogArguments.logincode)
    }

    //can extract cookied immediately
    if (!glogincode) glogincode = ''
    if (!glogincode) {
        glogincode = exodusgetcookie2('logincode', 'EXODUSlogincode')
    }
    if (!gdataset) gdataset = ''
    if (!gdataset) {
        gdataset = exodusgetcookie2('dataset')
    }
    setdateformat()

    //gexodus_server = gdataset.toLowerCase() == gdataset
    gexodus_server = typeof exodusread != 'undefined'

    if (typeof gparameters == 'undefined')
        gparameters = new Object
    if (typeof window.dialogArguments != 'undefined') {
        for (var param in window.dialogArguments)
            gparameters[param] = window.dialogArguments[param]
    }
    if (gparameters.gtasks && !gtasks)
        gtasks = gparameters.gtasks

    //add gparameters from URL if present eg. neosys.com/xyz.htm?param1=1&param2=2 etc.
    var temp = window.location.toString()
    if (temp.indexOf('?') >= 0) {
        //temp = unescape(temp.slice(temp.indexOf('?') + 1)).split('&')
        temp = unescape(temp.slice(temp.indexOf('?') + 1)).replace(/\?/g, '&').split('&')
        for (i = 0; i < temp.length; i++)
            gparameters[temp[i].split('=')[0]] = temp[i].split('=').slice(1).join('=')
    }
    if (typeof gparameters.savemode == 'undefined')
        gparameters.savemode = gparameters.SAVEMODE

    //file access functions
    if (document.protocolcode == 'file') {
        document.writeln('<scr' + 'ipt type="text/javascript" src="' + EXODUSlocation + 'scripts/server.js"></scr' + 'ipt>')
    }

    //style sheet
    document.writeln('<link id="exodus_global_css" rel="stylesheet" type="text/css" href="' + EXODUSlocation + 'global.css">')

    //jquery (not used)
    //document.writeln('<scr' + 'ipt type="text/javascript" id=jquery src="' + EXODUSlocation + 'scripts/jquery-1.4.4-min.js"></scr' + 'ipt>')

    //general functions
    if (!($$('generalfunctions'))) {
        document.writeln('<scr' + 'ipt type="text/javascript" id=generalfunctions src="' + EXODUSlocation + 'scripts/exodus.js"></scr' + 'ipt>')
    }

    //form functions
    if (typeof gdatafilename != 'undefined' || typeof gdictfilename != 'undefined') {
        document.writeln('<scr' + 'ipt type="text/javascript" src="' + EXODUSlocation + 'scripts/db.js"></scr' + 'ipt>')
        document.writeln('<scr' + 'ipt type="text/javascript" src="' + EXODUSlocation + 'scripts/gds.js"></scr' + 'ipt>')
        document.writeln('<scr' + 'ipt type="text/javascript" src="' + EXODUSlocation + 'scripts/dbform.js"></scr' + 'ipt>')
    }

    //if (!window.onload)
    //save any custom onload function to be executed by clientfunctions_windowonload when it finishes
    gwindowonload = window.onload
    //window.onload = clientfunctions_windowonload
    window.onload = null
    addeventlistener(window, 'load', 'clientfunctions_windowonload')
    //addeventlistener(window,'unload','clientfunctions_windowonunload_sync')

    //ensure print preview styles are set during print/preview
    //window.onbeforeprint = window_onbeforeprint
    addeventlistener(window, 'beforeprint', 'window_onbeforeprint')

    //save location except if logging in
    if (typeof gnosavelocation == 'undefined' && !window.dialogArguments && EXODUSlocation != './exodus/') {
        //if (gdataset) exodussetcookie('','EXODUS',escape(location),'ll',true)
    }

    glogging = false
    gstepping = 0

    //ensure http session is kept alive
    if (document.protocolcode != 'file') {
        if (gkeepalivemins)
            //exodussetinterval('yield* sessionkeepalive()', gkeepalivemins * 60 * 1000)
            exodussetinterval('yield* sessionkeepalive()', gkeepalivemins * 60 * 1000)
    }

    loadcache()

}

//end of main initialisation.
//what follows should be functions and their global variables only

function* window_onbeforeprint() {
    yield* clientfunctions_setstyle()
}

function* exoduslogout_onclick() {

    //cancel any automatic login
    exodussetcookie('', 'EXODUS', '', 'a')

    //decide where to login again
    var newwindowlocation = '../index.html'
    var system = exodusgetcookie2('', 'EXODUSsystem', null)
    if (system && system != 'ADAGENCY')
        newwindowlocation += '??' + system

    //clear various session variable
    yield* dblogout()

    //switch to login window
    try {
        window.location.assign(newwindowlocation)
    }
    catch (e) { }

    return false

}

var gmsg
var gtasks
function* exodussecurity(task) {

    //return empty gmsg if authorised
    gmsg = ''

    //look for ancient source code
    if (task.indexOf(' FILE ') >= 0)
        yield* exoduswarning('FILE should not be in task ' + task)

    //make sure task list is loaded (clearing cache also clears gtasks for convenience)
    if (!gtasks) {
        db.request = 'CACHE\rEXECUTE\rGENERAL\rGETTASKS\rNOT'
        if (!(yield* db.send())) {
            gtasks = ''
            gmsg = db.response
            yield* exodusinvalid(gmsg)
            return false
        }
        gtasks = db.data.split(fm)[0].split(vm)

        //expand leading + signs
        for (var i = 1; i < gtasks.length; i++) {
            if (gtasks[i].match(/(^\++)/)) {
                gtasks[i] = gtasks[i].replace(/(^\++)/, gtasks[i - 1].split(' ').slice(0, RegExp.$1.length).join(' ') + ' ')
            }
        }

    }

    //fail if task not allowed
    //if (task.slice(0,3)=='!!!'||gtasks.exoduslocate(task)||(gstepping&&!(confirm(task))))
    if (task.slice(0, 3) == '!!!' || gtasks.exoduslocate(task)) {
        gmsg = 'Sorry ' + gusername.exoduscapitalise() + ', you are not authorised to do\r' + task.exoduscapitalise()
        return false
    }

    return true

}

function* sessionkeepalive() {

    //last connection
    var lastconnection = exodusgetcookie2('lc', 'EXODUSlc', '')
    if (lastconnection == 'undefined')
        lastconnection = ''
    lc = lastconnection
    lastconnection = lastconnection ? new Date(lastconnection) : lastconnection = new Date(0)

    //next connection
    var nextconnection = lastconnection
    nextconnection.setMilliseconds(lastconnection.getMilliseconds() + (gkeepalivemins * 60 * 1000))

    //call server if time to keep alive
    var time = new Date
    if (time >= nextconnection && typeof db != 'undefined') {
        var tempdb = new exodusdblink()
        tempdb.request = 'KEEPALIVE'
        yield* tempdb.send()
        //if (!(yield* db.send())) alert(db.response)
        window.status = time + ' Keep Alive'
    }

}

function exodussetexpression(elementsorelementid, attributename, expression) {

    //check element exists
    if (!elementsorelementid)
        return yield * exodusinvalid('missing element in exodussetexpression ' + attributename + ' ' + expression)

    //elements can be elementnames too

    //get an array of elements
    var elements
    if (typeof elementsorelementid == 'string') {
        elements = document.getElementsByName(elementsorelementid)
        if (!elements.length) {
            elements = ''
            elements = $$(elementsorelementid)
            if (!elements)
                try {
                    elements = eval(elementsorelementid)
                }
                catch (e) { }
            if (!elements)
                return
            //TODO cater for msie returning an array of many
            elements = [elements]
        }
    }
    else {
        elements = elementsorelementid
        //make sure is an array
        if (!elements.length) elements = [elements]
    }

    //set the attribute expression for all elements
    var style = attributename.slice(0, 6) == 'style:'
    var attributepart = style ? attributename.slice(6) : attributename
    for (var ii = 0; ii < elements.length; ii++) {
        if (typeof elements[ii] == 'string')
            exodussetexpression(elements[ii], attributename, expression)
        else {

            //ie8 leaves the function in but throws an error when used?!
            try {
                //force error if doesnt exist or is ie8 (exists and doesnt work?!)
                if (elements[ii].setExpression) {
                    var element = elements[ii]
                    if (style)
                        element = element.style
                    element.setExpression(attributepart, expression)
                    continue;
                }
            }
            catch (e) {
                //TODO replace setexpression with some kind of callback events
            }

        }
    }

}

var gsetexpressioninterval = []
function exodussetexpression2(elementids, attributename, expression) {

    var elementids2
    if (typeof elementids == 'string')
        elementids2 = elementids.split(',')
    else
        elementids2 = elementids

    var allelements = []
    for (var ii = 0; ii < elementids2.length; ++ii) {
        var elementid = elementids2[ii]

        var elements = document.getElementsByName(elementid)
        if (!elements.length) {
            elements = $$(elementid)
            if (!elements)
                try {
                    elements = eval(elementid)
                }
                catch (e) { }
            if (!elements)
                continue
            //$$() like "MSIE global id variables" returns array only if more than one
            if (!elements.length)
                elements = [elements]
        }
        //rearray to make sure is an array otherwise ie6 $$() seems to return a collection and concat appends all as one element on the end
        allelements = allelements.concat(rearray(elements))
    }

    var style = attributename.slice(0, 6) == 'style:'
    var attributepart = style ? attributename.slice(6) : attributename

    exodussetexpression2b(elementid, allelements, style, attributepart, expression)

}

function exodussetexpression2b(expressionid, elements, style, attributename, expression) {

    gsetexpressioninterval[expressionid] = exodussetinterval(

        //build a closure containing all the elements to be updated
        //and to be called at intervals
        function anon_from_exodussetexpression2b() {
            //NB LEAVE A SPACE BEFORE THE LEFT BRACKET to prevent converter adding yield * in front of it
            //NB next line in yielding code is a method call and is NOT ONLY a comment
            exodussetexpression2c(elements, style, attributename, expression)
            /* yield */.next()

            //In yielding code, the above only creates a generator function which has to be
            //spurred into action by calling its next() method - below
            //We cannot simply prefix yield * in front of it to cause it to automatically execute
            //because it is in an anonymous function that will be called at intervals by window
            //and if the anonymous function were to be marked function * (ie to be a yielding function
            //known as a generator) the window call at intervals would only create a generator
            // and not actually start the function by calling its next() method
        }
        , 250)//every quarter second
}

//this is called at intervals
function* exodussetexpression2c(elements, style, attributename, expression) {
    //set the attribute expression for all elements
    var result = yield* exodusevaluate(expression)
    //console.log(expression+' is '+result+')
    if (attributename == 'disabled')
        1 == 1
    for (var ii = 0; ii < elements.length; ii++) {
        var element = elements[ii]
        if (style)
            element = element.style
        element[attributename] = result
    }

}

function exodusenabledandvisible(element0, allowreadonly) {
    var element = element0
    if (typeof element == 'string') {
        element = document.getElementsByName(element0)[0]
        if (!element) {
            element = document.getElementById(element0)
            if (!element) {
                element = $$(element0)
                if (!element) {
                    systemerror('exodusenabledandvisible() cannot getElementsByName ' + element0)
                }
            }
        }
    }
    if (!allowreadonly && element.getAttribute('exodusreadonly'))
        return false

    if (!element.offsetWidth || element.style.display == 'none')
        return false

    //have to check parents in msie TODO only required for older MSIE?)
    while (isMSIE && element.parentNode && element.parentNode.tagName != 'BODY') {
        element = element.parentNode
        if (element.style.display == 'none')
            return false
        if (element.getAttribute('disabled'))
            return false
    }

    return true
}

function showhide(element, show) {

    var elementid = element

    if (typeof element == 'string') {
        element = $$(element)
        if (!element)
            //return yield* exodusinvalid('element ' + elementid + ' does not exist in showhide()')
            systemerror('showhide("' + elementid + '")', ' window element does not exist')
    }

    //recursive
    //if (typeof element=='object'&&!element.tagName)
    if ((typeof element == 'object' || typeof element == 'function') && !element.tagName) {
        for (var ii = 0; ii < element.length; ii++)
            showhide(element[ii], show)
        return
    }

    if (element.style) {
        if (!show)
            element.style.display = 'none'
        else {
            //use '' because 'inline' doesnt line up columns in mozilla and table-row etc is only accepted by msie8+
            element.style.display = ''
        }
    }
    return true
}

function insertafter(element, newelement) {
    if (element.nextSibling)
        return element.parentNode.insertBefore(newelement, element.nextSibling)
    return element.parentNode.insertBefore(newelement, null)
}

function setinnerHTML(elementsorid, html) {
    if (typeof elementsorid == 'string') {
        elementsorid = $$(elementsorid)
        if (elementsorid.tagName) {
            elementsorid.innerHTML = html
            return
        }
    }
    for (var i = 0; i < elementsorid.length; i++)
        elementsorid[i].innerHTML = html
}

function blockmodalui_sync() {

    unblockmodalui_sync()

    //YIELD//console.log('BLOCKING UI')

    blocker = document.createElement('div')
    blocker.style.width = '100%'
    blocker.style.height = '100%'
    blocker.style.background = 'rgba(255,255,255,0.25)'//white overlay with only 25% opacity
    blocker.style.position = 'fixed'
    blocker.style.top = '0'
    blocker.style.left = '0'
    blocker.style.zIndex = '100'
    blocker.id = 'uiblockerdiv'

    document.body.insertBefore(blocker, null)

    //keep focus off parent window and on child window or exodusdiv
    blocker.onclick = function uiblockerdiv_onclick() {

        if ($$('exodusconfirmdiv')) {
            window.setTimeout('exodus_confirm_function3()', 10)
            //alert('Please wait for server response')
        }
        else if (gchildwin) {
            var actualwin
            if (gchildwin.lazy) {
                actualwin = gchildwin.actual
                //open a 'please wait' window the first time that they click the blockerdiv or they close the 'please wait' window
                if (!actualwin || actualwin.closed) {
                    var dialogstyle = getdialogstyle_sync(dialogstyle)

                    var question = 'Processing. Please wait.'
                    var defaultbutton = 1
                    var dialogargs = [question, defaultbutton, 'Wait', 'Cancel']
                    //pass the xmlhttprequestobject so it can be aborted if user clicks Cancel
                    if (gchildwin.xhttp)
                        dialogargs.xhttp = gchildwin.xhttp

                    //similar code in blockmodalui_sync and exodusconfirm
                    var dialogstyle
                    var newwidth = 200
                    var newheight = 100
                    var max=getmaxwindow_sync()
                    var newleft = 0 + (max.width - newwidth) / 2
                    var newtop = 0 + (max.height - newheight) / 2
                    var dialogstyle = 'top='+newtop+', left='+newleft+', width='+newwidth+', height='+newheight
                    //alert(dialogstyle)

                    //cant call exodusshowmodaldialog because that requires a global gcurrentevent variable/generator function
                    //and that global/generator is already in use handling some current event that is yielded for async xmlhttprequest
                    actualwin = window.open(EXODUSlocation + 'confirm.htm', '', dialogstyle)
                    gchildwin.actual = actualwin

                    if (!actualwin) {
                        alert('Unable to show popup window - please enable popups; disable your popup blocker.')
                        return
                    }

                    //pass arguments and callback/resume function to child window
                    actualwin.dialogArguments = dialogargs

                }
            } else
                actualwin = gchildwin

            //focus on child window
            window.setTimeout(function () { try { actualwin.focus() } catch (e) { } }, 10)

            //also focus on child's child recursively
            var winuiblocker = actualwin.document.getElementById('uiblockerdiv')
            if (winuiblocker)
                winuiblocker.click()

        }
    }

}

var gchildwin

function unblockmodalui_sync() {
    //close 'please wait' window if present
    if (gchildwin && !gchildwin.closed && gchildwin.actual && !gchildwin.actual.closed)
        gchildwin.actual.close()
    var blocker = $$('uiblockerdiv')
    if (blocker) {
        //YIELD//console.log('UNBLOCKING UI')
        exodusremovenode(blocker)
        //console.log('parent window ui unblocked')
    }
}

function getdialogstyle_sync(dialogstyle) {

    var standardstyle = 'Center:yes'
    standardstyle += ',Help:no'
    standardstyle += ',Resizable:yes'
    standardstyle += ',Scroll:yes'
    standardstyle += ',Status:no'
    standardstyle += ',scrollbars=1'//mozilla?
    //standardstyle+=',modal=yes'//no longer supported in chrome
    standardstyle += ',alwaysRaised=yes'

    var max = getmaxwindow_sync()
    //var maxwidth = window.outerWidth//window.innerWidth//screen.availWidth
    //var maxheight = window.outerHeight//window.innerHeight//screen.availHeight
    //var maxwidth = max.width
    //var maxheight = max.height

    //on ie6 seems to minimise
    //maxwidth=0
    //maxheight=0
    if (!dialogstyle) {
        //var maxwidth=window.screen.availWidth
        //var maxheight=window.screen.availHeight
        //var dialogstyle = 'DialogHeight:' + maxheight + 'px;DialogWidth:' + maxwidth + 'px'
        //var dialogstyle = 'Center:yes,Help:yes,Resizable:yes;Status:No'
        var dialogstyle = standardstyle

        //IE
        //if (maxwidth)
        //    dialogstyle += ',DialogWidth:' + maxwidth + 'px'
        //if (maxheight)
        //    dialogstyle += ',DialogHeight:' + maxheight + 'px'

        //mozilla
        if (max.width)
            dialogstyle += ',width=' + max.width
        if (max.height)
            dialogstyle += ',height=' + max.height

        if (typeof window.screenX != 'undefined')
            //dialogstyle+=', left='+window.screenX
            dialogstyle += ',left=' + max.left
        if (typeof window.screenY != 'undefined')
            //dialogstyle+=', top='+window.screenY
            dialogstyle += ',top=' + max.top

        //prevents centering in ff3
        //dialogstyle+='; DialogHeight: 100px; DialogWidth: 500px'
        //manual centering (TODO calculate it from maxheight and maxwidth)
        //dialogstyle+='; DialogTop: 100px; DialogLeft: 100px'
    }
    else if (dialogstyle == 'max') {
        dialogstyle = standardstyle + ',height:' + max.height + 'px,width:' + max.width + 'px;'
    }

    //return with comma AND semicolon separators (MSIE requires semicolon and Firefox/Webkit require commas)
    //dialogstyle = dialogstyle + ';' + dialogstyle.replace(/,/g, ';')

    return dialogstyle
}

function* exodusshowmodaldialog(url, arguments, dialogstyle) {

    if (!arguments)
        var arguments = new Object
    if (!arguments.gtasks)
        arguments.gtasks = gtasks

    dialogstyle = getdialogstyle_sync(dialogstyle)

    //always send login code
    arguments.logincode = glogincode
    try {

        //provide an alternative location for the child dialogWindow to get dialogArguments
        //gDialogArguments=window.opener.dialogArgumentsForChild
        //to avoid bug? in firefox where dialogArguments is always set to null when calling a modal dialog immediately on opening firefox
        //currently only used in index.html when being used as a modal dialog to do a login on the fly
        //eg when accessing pages via favourites without going through index.html first

        dialogArgumentsForChild = arguments

        //non-yielding code with showModalDialog
        var result
        if (!guseyield)
            result = window.showModalDialog(url, arguments, dialogstyle)

        //yielding code
        else {

            //example
            //../media/schedulefind.htm?FILENAME=SCHEDULES
            /*
            Center: yes, Help: no, Resizable: yes, Scroll: yes, Status: no, scrollbars=1,
            alwaysRaised=yes, DialogWidth:1870px, DialogHeight:1053px, width=1870, height=1053, left=50, top=27
            ;
            Center: yes; Help: no; Resizable: yes; Scroll: yes; Status: no; scrollbars=1; 
            alwaysRaised=yes; DialogWidth:1870px; DialogHeight:1053px; width=1870; height=1053; left=50; top=27
            */
            dialogstyle = '' // now always show in a tab

            //open the child window async
            gchildwin = window.open(url, '', dialogstyle)

            if (!gchildwin) {
                alert('Unable to show popup window - please enable popups; disable your popup blocker.')
                return
            }

            //pass arguments and callback/resume function to child window
            gchildwin.dialogArguments = arguments

            //auto resume if the child window disappears - every poll every n ms
            //this will stop when it goes out of scope when this function terminates
            window.setTimeout('exodus_autoresume()', 100)

            //wait here until exodus_autoresume detects that the child window is closed
            // and passes its return value here
            result = yield* exodus_yield('exodusshowmodaldialog ' + url)
            console.log('exodusshowmodaldialog result is ' + result)
        }

        //Safari doesnt return an error and looks like a Window [x] close unfortunately
        if (typeof result == 'undefined' && window.navigator.appVersion.indexOf('Safari') >= 0)
            throw (url)

        return result

    }
    catch (e) {
        console.log('caught error in exodusshowmodaldialog: ' + e)
        //alert('Please enable popups for this site (1)\n\nError:'+(e.description?e.description:e))
        //alert('Please enable popups for this site (1)\n\nError:'+(e.description?e.description:e)+'\n\n'+url+'\n\n'+arguments)
        return
    }

    //return nothing if cannot popup
    return

}

function* exodus_yield(source) {

    blockmodalui_sync()

    logevent('          >> BEFORE YIELD ' + source)
    logevent('          ... yielding ... pause this code and resume parent code')

    ////////////////////////////////////////////////
    //PAUSE HERE until something provides a result
    //by executing geventhandler  .next(result)
    ////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////
    //NOTE IN YIELDING CODE THE FOLLOWING LINE IS NOT ONLY A COMMENT
    //NOTE IN NON-YIELDING CODE THE FOLLOWING LINE IS ONLY A COMMENT
    ////////////////////////////////////////////////////////////////

    /* yield */   var result = yield source

    logevent('          << AFTER YIELD ' + source)

    return result

}

//called by child windows to return result to the parent before closing
function exoduswindowclose(returnvalues) {

    //window.opener.gchildwin_returnvalue = returnvalues
    if (window.opener) {
        window.opener.focus()//for MSEDGE
        if (window.opener.exodus_setchildwin_returnvalue) {
            if (typeof returnvalues == 'undefined')
                returnvalues = ''
            returnvalues.exodusisarray = true
            window.opener.exodus_setchildwin_returnvalue(returnvalues)
        }
    }
    window.returnValue = returnvalues
    window.close()
}

//parent window function called by childwindow to return result to parent window
var gchildwin_returnvalue
function exodus_setchildwin_returnvalue(returnvalue) {
    //gchildwin_returnvalue=returnvalue
    //shallow copy array to avoid permissions issue when child windows closes in edge
    if (returnvalue.exodusisarray) {
        gchildwin_returnvalue = []
        for (var ii = 0; ii < returnvalue.length; ++ii) {
            //gchildwin_returnvalue[ii]=returnvalue[ii].toString()
            gchildwin_returnvalue[ii] = returnvalue[ii]
        }
    }
    else {
        gchildwin_returnvalue = returnvalue
    }
}

function exodus_autoresume() {

    //if child window still active then schedule another check later
    if (gchildwin && !gchildwin.closed) {
        //console.log('exodus_autoresume - window still present and not closed')
        window.setTimeout('exodus_autoresume()', 100)
        return
    }

    console.log('EXODUS_AUTORESUME since gchildwin is closed or missing')

    //childwin doesnt exist or has closed
    //get the returnvalue from it or return an empty string
    try {
        var returnvalue = gchildwin.returnValue
    } catch (e) {
        gchildwin = false
        returnvalue = ''
    }

    //alert('2 gchildwin_returnvalue='+gchildwin_returnvalue)
    if (gchildwin_returnvalue) {
        returnvalue = gchildwin_returnvalue
        //        alert('gchildwin_returnvalue='+gchildwin_returnvalue)
        gchildwin_returnvalue = undefined
    }

    exodus_resume(returnvalue, 'exodus_autoresume')

}

//given a result from some event, resume a yielded geventhandler function, passing it a result
// after closing the modaluiblocker
//1. a child window is closed
//2. an xmlhttp action completes (ok/error/timeout/abort)
//3. user clicks various keys while a exodusconfirmdiv is present
function exodus_resume(value, source) {

    logevent(' ')
    logevent('exodus_resume  <in ' + value + ' from ' + source + ')')

    //clear ui and stop resumer before resuming
    //because .next() will run to the next async/ui occurrence in the current event process
    unblockmodalui_sync()

    //give the result (value) to the yielded and now resuming code
    //result = yield 'xxxxxxxxxxxxx'
    var next = exodus_next(value, 'exodus_resume from ' + source)

    logevent('exodus_resumed out> ' + next.value + ') from ' + source + ')')

    //probably no purpose in returning this since we are not using generator functions to acquire values,
    //only to act as suspended functions until some data they need is acquired asynchronously from exodusdiv, child window, xmlhttp
    return next.value

}

function exodus_next(value, source) {

    logevent('  ==> BEFORE NEXT EVENTHANDLER from ' + source + ' - value in:' + value + ' ===')

    ///////////////////////////////////////////////////////////////
    //very important code either a) starts a "suspendable" function
    //or b) resumes a suspended function passing a value into it
    ///////////////////////////////////////////////////////////////

    //all new events are blocked while our event handler (generator function) is not "done"
    //"done" means that the function has returned (not yielded)
    //this is because it is a global variable so our exodus_resume function knows what to resume
    if (!gblockevents)
        form_blockevents(true, source)

    var next = geventhandler.next(value)

    logevent('  <== AFTER NEXT EVENTHANDLER from ' + source + ' ===')
    logevent('      done:' + next.done + ' value out:' + next.value)

    if (next.done) {
        form_blockevents(false, source)
        //following should not be necessary since
        //if our event handler does not yield anywhere then it will not be created
        //and if our event handle DOES yield then it will be cleared in exodus_resume
        unblockmodalui_sync()
    }

    return next
}

function displayresponsedata_sync(request, data) {
    //yield* displayresponsedata(request, data).next()
    //hide function from converter to yield *
    var temp = window['displayresponsedata'](request, data)
    if (temp.next)
        exodusneweventhandler(temp, 'displayresponsedata')
}

function* displayresponsedata(request, data) {

    if (!data)
        data = ''

    db.request = request
    if (!(yield* db.send(data))) {
        yield* exodusinvalid(db.response)
        return false
    }
    if (db.data)
        yield* exodusnote(db.data)
    else
        yield* exodusnote(db.response.slice(2))

    return true

}

function openwindow_sync(request, data) {
    //LEAVE SPACE AFTER FUNCTION NAME TO PREVENT CONVERSION TO YIELD
    var result = openwindow(request, data)
    if (result.next) {
        result = exodusneweventhandler(result, 'openwindow_sync ' + request).value
    }
    return result
}

//function to simplify passing a db request (with optional data)
//that returns a URL to be opened in an independent window
function* openwindow(request, data) {

    if (!data)
        data = ''
    db.request = request
    if (!(yield* db.send(data))) {
        yield* exodusinvalid(db.response)
        return false
    }

    if (db.response != 'OK')
        yield* exodusnote(db.response.slice(3))

    if (db.data) {
        var urls = db.data.split(fm)
        var result
        for (ii = 0; ii < urls.length; ++ii)
            result = yield* windowopen(urls[ii])
        return result
    }
}

function* windowopenkey(url, key) {
    if (!key)
        key = gvalue
    return yield* windowopen(url, { 'key': key })
}

function windowopen_sync(url, parameters, style) {
    //LEAVE SPACE AFTER FUNCTION NAME TO PREVENT CONVERSION TO YIELD
    var result = windowopen(url, parameters, style)
    if (result.next)
        return result.next()
    return result
}

var gwindowopenparameters
function* windowopen(url, parameters, style) {

    url = url || ''
    parameters = parameters || ''
    style = style || ''

    //url may arrive like ..\DATA\DEVDTEST\~3499268.htm or ../data/DEVDTEST/~3499268.htm
    // and will be converted here to ../../DATA/DEVDTEST/~3499268.htm
    //which browsers will interpret as as the full URL http://hostname/DATA/DEVDTEST/~3499268.htm
    //if (url.replace(/\\/g, '/').toUpperCase().substr(0, 8) == '../DATA/')
    //    url = '../' + url.replace(/\\/g, '/')

    // replace backslashes with slashes and anything before /data/ with ../..
    url = url.replace(/\\/g, '/').replace(/^.*[\/\\]data[\/\\]/,'../../data/')

    //if no style or "tab" parameter, windows opened from dialogs are opened in new maximised windows
    if (style == '' && gisdialog) {
        var max = getmaxwindow_sync()

        style += ';DialogHeight:' + max.height + 'px; DialogWidth:' + max.width + 'px; Resizable: yes'
        //mozilla
        style += ',toolbar=yes,menubar=yes,resizable=yes,scrollbars=yes,status=yes'
        style += ',width=' + max.width
        style += ',height=' + max.height
        style += ',left=' + max.left
        style += ',top=' + max.top
    }
    //"tab" used to force opening in a tab despite being in a dialog (because the dialog is closing)
    if (style == 'tab')
        style = ''

    //normalise windows style urls with backslashes to slashes.
    //firefox wont normalise window style relative urls like ..\ but will if they are like ../
    // and this may cause problems for any other relative urls in the page like images
    url = url.replace(/\\/g, '/')

    //these parameters are picked up by the opened window (from its parent) after it opens
    //since no way to pass parameters directly to non modal windows except in the URL
    //and even then the this causes different urls perhaps defeating http caching
    gwindowopenparameters = parameters
    gwindowopenparameters.logincode = glogincode
    try {

        var result = window.open(url, '', style)

        //Safari doesnt return an error and looks like a Window [x] close unfortunately
        if (typeof result == 'undefined' && window.navigator.appVersion.indexOf('Safari') >= 0)
            throw (url)

        return result

    }
    catch (e) {
        //alert('Please enable popups for this site (2)\n\nError:'+(e.description?e.description:e))
        return false
    }

}

//wrapper function to replace window.open()
//to give warning if cannot open ... because of pop blockers etc.
function* exodusbreak(cmd, funcname, position) {

    if (!gstepping) return

    window.status = funcname + ' ' + position + ' ' + cmd

    if (!cmd) cmd = ' '

    var msg = ''
    while (cmd) {
        if (cmd != ' ') {

            var result
            try {
                result = this.parent.eval(cmd)
            }
            catch (e) {
                result = e.number + ' ' + e.description
            }
            msg = cmd + '=\n' + result + '\n\n' + msg

            //display result and optionally quit
            if (!(confirm(msg))) {
                //FF3 about:config
                //extensions.firebug-service.breakOnErrors
                //breakrighthere
                debugger
                return
            }

        }
        cmd = prompt(funcname + ' ' + position, cmd)
    }
}

function assertelement(element, funcname, varname) {
    exodusassertobject(element, funcname, varname)
    if (!element.tagName) {
        return systemerror('assertelement()', 'In ' + funcname + ', ' + varname + ' is not an element.')
    }
    return true
}

function* exodusnote(msg, mode) {

    //if (!msg) return false
    //allow return yield* exodusnote() to be opposite of return yield* exodusinvalid()
    if (!msg)
        return true

    //depends on number of buttons now 1=info >1=question
    //if (!mode) mode='info'

    if (typeof msg == 'string' && msg.slice(0, 6) == 'Error:')
        msg = msg.slice(6)

    //msg=msg.replace(/\r/,'\r')
    //msg=msg.replace(/\n/,'\n')
    msg = msg.toString().replace(FMre, '\r\n').replace(VMre, '\r\n')
    msg = msg.replace(/\|/, '\r\n')

    yield* exodusconfirm(msg, 1, 'OK', '', '', null, false, mode)

    return true

}

//''''''''
//'INVALID
//''''''''
function* exodusinvalid(msg) {
    //displays a message if provided and returns false
    //so you can use it like "return yield* exodusinvalid(msg)" to save a line
    yield* exodusnote(msg, 'critical')
    return false
}

//''''''''
//'WARNING
//''''''''
function* exoduswarning(msg) {
    //displays a message if provided and returns true
    //so you can use it like "return yield* exoduswarning(msg)" to save a line
    return yield* exodusnote(msg, 'warning')
}

function exodus_set_style(mode, value, value2) {

    if (value.toUpperCase() == 'DEFAULT') value = ''

    //restore original value
    if (!value && goriginalstyles[mode]) value = goriginalstyles[mode]

    //ensure display is set to inline even if not changing color
    //if (!value) return

    //var rules = document.styleSheets[0].cssRules||document.styleSheets[0].rules
    //var ss = document.getElementById('exodus_global_css');
    var link=document.querySelector("link[href='../exodus/global.css']")
    if (!link)
        return
    var ss = link.sheet
    var rules = ss.cssRules || ss.rules
    var oldvalue = ''

    //screencolor
    if (mode == 'screencolor'
        && rules) {

        //make everything visible!
        var style = rules[0].style
        //style.display='block'//ie6/7 ok ff3 shows 100% width
        //following is not supported in IE5.5 and FF2
        //but ok in IE6 IE7 IE8b1 FF3 Saf3.0Win Saf3.1Win Opera9.5b Konqueror3.5.7
        //IE 6/7 only where there is a natural display: inline.
        //http://www.quirksmode.org/css/display.html
        //style.display = 'inline-block'
        style.display = ''

        //initial color is buff yellow
        if (!value)
            value = '#ffffc0'

        oldvalue = style.backgroundColor
        try {
            style.backgroundColor = value
        }
        catch (e) {
            if (e.number == -2146827908) return yield * exodusinvalid(value + ' is not a recognised color')
            return systemerror('exodus_set_style("' + mode + '","' + value + '")', e.number + ' ' + e.description)
        }
    }

    //screenfont
    else if (mode == 'screenfont' && rules) {

        //initial font is ... 8pt
        var basefontsize = 8

        if (!value) value = 'verdana,sans-serif,arial,helvetica'
        if (!value2) value2 = 100
        if (!(Number(value2))) {
            alert(value2 + ' is not a recognised font size, using 100%')
            value2 = 100
        }
        if (typeof gformfontscale != 'undefined' && gformfontscale) value2 *= gformfontscale
        value2 = (basefontsize * Number(value2) / 100) + 'pt'

        for (var rulen = 0; rulen < rules.length; rulen++) {

            var style = rules[rulen].style
            if (!style || !style.fontFamily) continue

            oldvalue = style.fontFamily
            try {
                style.fontFamily = value
                style.fontSize = value2
            }
            catch (e) {
                if (e.number == -2146827908) return yield * exodusinvalid(value + ' is not a recognised font')
                if (e.number == -2147024809) return yield * exodusinvalid(value2 + ' is not a recognised fontsize')
                return systemerror('exodus_set_style("' + mode + '","' + value + '","' + value2 + '")', e.number + ' ' + e.description)
            }
        }
    }

    //save the original style
    if (!goriginalstyles[mode] && oldvalue) goriginalstyles[mode] = oldvalue

}

//called early in decide and decide2
function* clientfunctions_setstyle() {
    //set font first since setting color changes style display from none to inline
    exodus_set_style('screenfont', exodusgetcookie2('ff'), exodusgetcookie2('fs'))
    exodus_set_style('screencolor', exodusgetcookie2('fc'))
}

function* clientfunctions_getglobals() {
    gcompanycode = exodusgetcookie2('cc')
    gncompanies = exodusgetcookie2('nc')
    gperiod = exodusgetcookie2('pd')
    gbasecurr = exodusgetcookie2('bc')
    gshowzeros = exodusgetcookie2('bc')
    gbasefmt = exodusgetcookie2('bf')
    gmarketcode = exodusgetcookie2('mk')
    gmaincurrcode = exodusgetcookie2('mc')
    gdatasetname = exodusgetcookie2('db')
    gmenucodes = exodusgetcookie2('m')
    gmaxstrsize = Number(exodusgetcookie2('ms'))
    gtz = exodusgetcookie2('tz').split(fm)
    if (gtz.join('')) {
        gtz[0] = Number(gtz[0])
        if (gtz[1])
            gtz[1] = Number(gtz[1])
        else
            gtz[1] = 0
    }
    else {
        //gtz = [0, 0]
        gtz = [new Date().getTimezoneOffset() * -60, 0]
    }

    //support input of numbers like 999,999,999.99 or 999.999.999,99
    //gbasefmt is MDn0, or MCn0, where n=2 or 3 for number of decimals and D/C means , or . for thousand sep
    if (gbasefmt.substr(0, 2) == 'MC')
        gthousands_regex = /\./gi
    else
        gthousands_regex = /,/gi
}

function add_exodus_menubar() {

    //if no exodus_menubar element
    //create a exodus_menu span
    // at the beginning of the body
    // or after the first navbar element

    gexodus_menubar = document.getElementById('exodus_menu')
    if (!gexodus_menubar) {
        var span = document.createElement('SPAN')
        span.id = 'exodus_menu'
        var navbar1 = document.getElementsByClassName('navbar')[0];
        if (navbar1)
            navbar1.parentNode.insertBefore(span, navbar1.nextSibling)
        else
            document.body.insertBefore(span, document.body.firstChild)
        gexodus_menubar = document.getElementById('exodus_menu')
    }

}

function* clientfunctions_windowonload() {

    ////login('clientfunctions_windowonload')

    if (document.getElementsByClassName)
        id2classname()

    //already done in global script
    //    if (!glogincode)
    //        glogincode = exodusgetcookie2('logincode', 'EXODUSlogincode', null)

    //loginalert('wol'+glogincode)
    //    gdataset = exodusgetcookie2('dataset')

    yield* clientfunctions_getglobals()

    db = new exodusdblink
    //gusername is set in exodusdblink

    //In case previous window didnt successfully unlock its record in onbeforeunload
    //Could be turned off hoping synchronous unlock in window_onunload is sufficiently reliable
    // to avoid need for double unlocking
    var pending = exodusgetcookie(glogincode, 'EXODUSpending')
    if (pending) {
        exodussetcookie(glogincode, 'EXODUSpending', '')
        console.log(pending)
        db.request = pending
        yield* db.send()
    }

    yield* clientfunctions_setstyle()

    //trigger formfunctions_onload
    if (typeof formfunctions_onload == 'function')
        yield* formfunctions_onload()

    //if (gautofitwindow && document.getElementById('autofitwindowelement'))
    //    exodussettimeout('exodusautofitwindow()', 10)
    //exodussetinterval('exodusautofitwindow()', 10)

    add_exodus_menubar()

    //add menu, logout and refresh buttons if not a popup, depending on gshowmenu, not /exodus/ location and no navbar elements
    //if no exodus_menu span (even if no menu, it is a holder for EXODUS form buttons New/Save etc.)
    if (!window.dialogArguments && (typeof gshowmenu == 'undefined' || gshowmenu) && EXODUSlocation != './exodus/' && document.getElementsByClassName('navbar').length == 0) {

        var ctrlalt = isMac ? 'Ctrl' : 'Alt'

        //insert buttons in reverse order

        //button to refresh (clear cache)
        if (typeof gshowrefreshcachebutton == 'undefined' || gshowrefreshcachebutton) {
            var temp2 = document.createElement('span')
            temp2.innerHTML = menubuttonhtml('refreshcache', grefreshimage, '<u>R</u>efresh', 'Refresh the Database Cache. ' + ctrlalt + '+R', 'X')
            //document.body.insertBefore(temp2, document.body.firstChild)
            gexodus_menubar.insertBefore(temp2, gexodus_menubar.firstChild)
            temp2.style.float = 'left'
            //if no dbform
            if (typeof gdictfilename == 'undefined')
                addeventlistener(temp2, 'click', 'refreshcache_onclick')
        }

        //button to logout
        var temp2 = document.createElement('span')
        temp2.innerHTML = menubuttonhtml('exoduslogout', glogoutimage, 'Lo<u>g</u>out', 'Logout. ' + ctrlalt + '+G', 'G')
        //document.body.insertBefore(temp2, document.body.firstChild)
        gexodus_menubar.insertBefore(temp2, gexodus_menubar.firstChild)
        temp2.style.float = 'left'
        if (!gusername) {
            var temp = $$('exoduslogoutbutton')
            temp.innerText = 'Login'
            temp.title = 'Login. Alt+L'
        }
        //if no dbform
        if (typeof gdictfilename == 'undefined')
            addeventlistener(temp2, 'click', 'exoduslogout_onclick')

        //button for menu
        if (gmenucodes && gmenucodes != 'EXIT2') {
            var menu_span = document.createElement('span')
            menu_span.style.float = 'left'
            menu_span.style.maxWidth = '65px'//stop first button flashing very wide initially
            menu_span.innerHTML = menubuttonhtml('menu', gmenuimage, '<u>M</u>enu', 'Menu. ' + ctrlalt + '+M', 'M')
            //document.body.insertBefore(menu_span, document.body.firstChild)
            gexodus_menubar.insertBefore(menu_span, gexodus_menubar.firstChild)

            //div to retrieve menu structure (insert into end of Menu button span)
            var dropdown_menu_span = document.createElement('span')
            dropdown_menu_span.id = 'menux'
            //dropdown_menu_span.style.display = 'none'
            dropdown_menu_span.innerHTML = '<iframe id=menuframe src="../../data/menu.htm" tabindex=-1 height=1px marginheight=1 marginwidth=1 frameborder=0 scrolling=no></iframe>'
            var menuelement = $$('menubutton')
            //why insert it into the menu button?
            menuelement.insertBefore(dropdown_menu_span, null)
            //gexodus_menubar.insertBefore(dropdown_menu_span, gexodus_menubar.firstChild)

            //dont rely on onload event ... poll every 100ms to see if iframe is loaded
            gmenuonloader = exodussetinterval('menuonload()', 100)
        }

    }

    //execute any custom window onload function
    //avoid error "Can't execute code from a freed script"
    //gwindowonload comes from any window.onload ... not used atm
    if (gwindowonload)
        try { gwindowonload() }
        catch (e) { }

    //logout('clientfunctions_windowonload')

    return

}

//var mmm=0
gmenuloaded = false
function menuonload() {

    //$$('menuframe').contentDocument.getElementById('menucompleted')
    var iframe = $$('menuframe')
    idocument = iframe.contentDocument
    var menuhtml
    if (idocument) {
        if (!(idocument.getElementById('menucompleted')))
            return

        //enable exodus support menu items
        if (gusername=='EXODUS') {
            var elements = idocument.getElementsByClassName('exodussupportmenuitem')
            for (var ii=0;ii<elements.length;++ii)
                elements[ii].style.display = ''
        }

        menuhtml = idocument.body.innerHTML

    }
    else {
        //menuframe=$$('menuframe')
        //iframe.onload supported in ie55
        //if (typeof menuframe.menudiv=='undefined'||menuframe.menucompleted.readyState!='complete') return
        if (!menuframe.menucompleted)
            return

        //menux.innerHTML=menuframe.document.body.innerHTML
        //for some reason the iframe contains the span and iframe
        //so the above does not work!!!
        //so loop through the elements of the iframe skipping menux and menuframe
        var menuhtml = ''
        var childNodes = menuframe.document.body.childNodes
        for (var i = 0; i < childNodes.length; i++) {
            if (childNodes[i].outerHTML && childNodes[i].id != 'menux' && childNodes[i].id != 'menuframe')
                menuhtml += childNodes[i].outerHTML
        }

    }

    //login('menuonload')

    window.clearInterval(gmenuonloader)

    //menux.zIndex=999 //done in stylesheet but does not work over select elements (see MS docs)

    gmenuloaded = true

    //store the menu in the menu button
    $$('menux').innerHTML = menuhtml
    var menudiv = $$('menudiv')
    var target = menudiv.parentNode.parentNode
    insertafter(target, menudiv)

    //enable the menu button events
    var menubuttonx = $$('menubutton')
    menubuttonx.onmouseover = menuonmouseover
    menubuttonx.onmouseoutmenuonmouseout

    //do both?!
    //addeventlistener(menubuttonx, 'onclick', 'menuonclick')
    //addeventlistener(menubuttonx, 'onmouseover', 'menuonmouseover')
    //addeventlistener(menubuttonx, 'onmouseout', 'menuonmouseout')

    //logout('menuonload')

}

Array.prototype.exodusread = function* array_exodusread(filename, key, fieldno, cache) {

    //unless returning one fieldno, always return at least n fields
    //so that accessing fields that do not exist by [] returns ''
    var minnfields = 100

    this.exodusresponse = exodusquote(key) + ' does not exist in the ' + filename.toLowerCase() + ' file.'
    if (key == '')
        return false

    if (typeof cache == 'undefined' || cache)
        db.request = 'CACHE\r'
    else
        db.request = ''
    db.request += 'READ\r' + filename + '\r' + key
    if (!(yield* db.send())) {
        if (db.response.indexOf('NO RECORD') >= 0) {
            //var temp=filename.toLowerCase().exodussingular().replace(/_/,' ')
            return false
        }
        else {
            if (db.response.indexOf('file is not available') >= 0) systemerror('exodusread', db.response)
            this.exodusresponse = db.response
        }
        return false
    }

    var data = (key + fm + db.data).split(fm)

    //flag to extract function that fields are 1 based (key in [0])
    this.base = 1

    if (typeof (fieldno) == 'undefined' || fieldno == '' || fieldno == null) {
        var n = data.length
        if (n < minnfields) n = minnfields
        for (var i = 0; i < n; i++) this[i] = typeof data[i] != 'undefined' ? data[i] : ''
    }
    else {
        if (typeof data[fieldno] == 'undefined') data[fieldno] = ''
        this[0] = data[fieldno]
    }
    return true
}

//xlate method for array of keys
Array.prototype.exodusxlate = function* arrayxlate(filename, fieldno, mode) {

    var keys = this
    var results = []

    //make a list of keys not in cache
    var uncachedkeys = []
    for (var keyn = 0; keyn < keys.length; keyn++) {

        //return '' if record cannot be read or key if blank
        results[keyn] = ''
        var key = keys[keyn]
        if (!key) continue

        //get a record from the cache
        var cachekey = 'READ\r' + filename + '\r' + key
        var rec
        if (rec = readcache(cachekey)) {
            //and do xlate logic on cached record
            results[keyn] = yield* exodusxlatelogic(filename, (key + fm + rec).split(fm), fieldno, mode, key)
        }

        //or build a unique list of keys of records to be selected
        else {
            if (!uncachedkeys.exoduslocate(key)) uncachedkeys[uncachedkeys.length] = key
        }

    }

    //select all the uncached records
    if (uncachedkeys.length) {

        //WARNING !!!
        //unfortunately SELECT\r\r\rRECORD returns raw record without
        //the usual postread processing and it puts this into the cache for
        //later processing as well (perhaps it should not put it in the cache)
        //it is difficult to call the usual postread processing from SELECT2
        //on the server since postreads require DOS.COMMON (OREC etc)
        //perhaps one should have a special POSTREADX which only touches @record

        //select the (deduplicated) records or return systemerror
        db.request = 'SELECT\r' + filename + '\r\rRECORD'
        if (!(yield* db.send(uncachedkeys.join(fm)))) {
            systemerror(db.response)
            this.exodusresponse = db.response
            return []
        }

        //if no data returned then thats it!
        if (!db.data) return results

        //process the multiple records separated by rm char characters
        var recset = db.data.split(rm)
        for (ii = 0; ii < recset.length; ii++) {

            //do xlate logic on the record
            var keyrec = recset[ii].split(fm)
            var key = keyrec[0]
            var result = yield* exodusxlatelogic(filename, keyrec, fieldno, mode, key)

            //store the results whereever they are needed
            var keyn = 0
            while (keyn = keys.exoduslocate(key, '', keyn + 1)) {
                results[keyn - 1] = result
            }

            //save in the cache regardless
            var cachekey = 'READ\r' + filename + '\r' + key
            writecache(cachekey, keyrec.slice(1).join(fm))

        }

    } //any uncachedkeys

    return results

}

//get a (cached) database value given a filename, fieldno
//fieldno 0 means return whole record as simple array
//mode can be undefined, C (means return key if no record) and SUM means add up mvs
//zzz SHOULD return '' if no record and null if there is any error
String.prototype.exodusxlate = function* stringxlate(filename, fieldno, mode) {

    key = this.toString()
    if (key == '') return ''

    exodusassertnumeric(fieldno, 'xlate', filename + ' ' + key)

    var record = []
    yield* record.exodusread(filename, this)
    if (db.response.indexOf('file is not available') >= 0) systemerror('xlate', db.response)

    return yield* exodusxlatelogic(filename, record, fieldno, mode, key)

}

function* exodusxlatelogic(filename, record, fieldno, mode, key) {
    if (record.length) {
        if (typeof fieldno != 'undefined') {
            if (fieldno) {
                record = record[fieldno]
                if (typeof record == 'undefined') record = ''
            }
        }
        //option to sum the result (good for adding up multivalues)
        if (mode && mode == 'SUM') {
            record = record.exodussum()
        }
    }
    else {

        //option to return the key if failed to get record
        //zzz ?! C mode doesnt work since record length is zero here
        //record=(mode&&mode=='C')?record[0]:''
        record = (mode && mode == 'C') ? key : ''
    }

    return record

}

function* exodusfilepopup(filename, cols, coln, sortselectionclause, many, filtertitle, maxnrecs) {
    //filename is required
    //cols is required (array of arrays)
    //eg [['COMPANY_NAME','Company Name'],['COMPANY_CODE','Company Code']]

    //sortselectionclause is optional
    //eg 'WITH TYPE1 = "X" AND WITH = TYPE2 "Y" BY TYPE1 BY NAME'
    if (!sortselectionclause)
        sortselectionclause = ''

    if (!filtertitle)
        filtertitle = ''

    //sortselectionclause can be an array of keys
    if (typeof sortselectionclause == 'object')
        sortselectionclause = sortselectionclause.join('" "').exodusquote()

    //get a list of col names sep by spaces
    var collist = ''
    for (var i = 0; i < cols.length; i++)
        collist += ' ' + cols[i][0]
    collist = collist.slice(1)
    if (!collist.split(' ').exoduslocate('ID'))
        collist += ' ID'

    //increased from 1000 since changelog records>1000 and no way to
    if (typeof maxnrecs == 'undefined')
        maxnrecs = 2000

    //get the data from the server
    db.request = 'CACHE\rSELECT\r' + filename.toUpperCase() + '\r' + sortselectionclause + '\r' + collist + '\rXML\r' + maxnrecs
    //db.request='CACHE\rSELECT\r'+filename.toUpperCase()+'\r'+sortselectionclause+'\r'+collist+' ID'
    if (!(yield* db.send())) {
        yield* exodusinvalid(db.response)
        return null
    }

    //cancel if no records found
    if (db.data.indexOf('<RECORD>') < 0) {
        var msg = 'Sorry, no records found'
        if (filtertitle)
            msg += '\nfor ' + filtertitle
        else if (sortselectionclause.indexOf('WITH COMPANY_CODE') >= 0)
            msg += '\nfor the chosen company'
        yield* exodusinvalid(msg)
        return null
    }

    var question
    if (filtertitle)
        question = 'Which do you want?' + filtertitle

    return yield* exodusdecide2(question, db.data, cols, coln, '', many)

}

//////////////////////
// global functions //
//////////////////////

//similar function in xhttp.asp
function exodusdblink() {

    this.request = ''
    this.data = ''
    this.response = ''
    this.documentprotocolcode = document.protocolcode
    this.login = exodusdblink_login

    //maybe preset by php in some heading script

    //used in cache and elsewhere
    if (!gdataset) gdataset = exodusgetcookie2('dataset')
    //gusername = exodusgetcookie2('username')
    gusername = exodusgetcookie2('username') || gusername
    //alert('xyz2 EXODUS2/username='+exodusgetcookie2('username'))
    //alert(document.cookie)
    gsystem = exodusgetcookie2('', 'EXODUSsystem', null) || gsystem

    // alert(gdataset+'*'+gusername+'*'+gsystem)
    this.dataset = gdataset
    this.system = gsystem
    this.username = gusername

    //setup environment for http messaging
    if (this.documentprotocolcode != 'file') {

        //netscape or IE7 xmlhttp
        try {
            this.XMLHTTP = new XMLHttpRequest()
            this.send = exodusdblink_send_byhttp_using_xmlhttp
            return
        }
        catch (e) { }

        //IE6 xmlhttp
        try {
            this.XMLHTTP = new ActiveXObject('Microsoft.XMLHTTP')
            this.send = exodusdblink_send_byhttp_using_xmlhttp
        }

        //asp forms
        catch (e) {
            this.send = exodusdblink_send_byhttp_using_forms
        }

        return

    }

    //otherwise setup environment for file messaging

    //try and get the username, password and dataset
    this.password = exodusgetcookie2('password')
    this.timeout = exodusgetcookie2('timeout')

    //default timeout is 10 minutes (NB GIVEWAY timeout is hard coded to 10 mins?)
    var defaulttimeoutmins = 10
    //var defaulttimeoutmins=.25//15 seconds
    if (!this.timeout)
        this.timeout = defaulttimeoutmins * 60 * 1000


    this.send = exodusdblink_send_byfile
    this.start = exodusdblink_startdb

    try {
        gfso = new ActiveXObject('Scripting.FileSystemObject')
    }
    catch (e) {
        alert('Error: While creating Scripting.FileSystemObject\n' + e.number + ' ' + e.description)
        window.location.assign(EXODUSlocation + 'securityhowto.htm')
        return
    }

    this.exodusrootpath = getexodusrootpath(document.location.toString())

    //localhostname
    //remove space . " ' characters
    //and convert _ to -
    this.localhostname = new ActiveXObject('WScript.Network').ComputerName.slice(0, 8)
    this.wscriptshell = new ActiveXObject('WScript.Shell')

    this.remoteaddr = ''
    this.remotehost = new ActiveXObject('WScript.Network').ComputerName.toString()
    //remove space . " ' characters
    this.remotehost = this.remotehost.replace(/[\ |\.|\"|\']/g, '')
    this.https = 'off'//Request.ServerVariables("HTTPS")

}

function* dblogout() {

    //remove username etc
    //exodussetcookie('','EXODUS',gdataset,'dataset',true)

    //remove last page
    //exodussetcookie('','EXODUS','','ll',true)

    exodussetcookie(glogincode, 'EXODUS2', '', 'username')
    //alert('xyz EXODUS2/username='+exodusgetcookie2('username'))

    //remove settings
    //alert('before clear '+document.cookie)
    exodussetcookie(glogincode, 'EXODUS2', '')
    exodussetcookie(glogincode, 'EXODUS', '')
    //alert('after clear '+document.cookie)

}

function* exodusdblink_login(username, password, dataset, system) {

    //get list of datasets from server
    var logindb = new exodusdblink
    var datasets = ''

    if (!dataset && gdataset)
        dataset = gdataset
    if (!dataset)
        dataset = exodusgetcookie2('dataset', 'EXODUS', '')
    if (!system)
        system = exodusgetcookie2('', 'EXODUSsystem', null)
    var arguments = ['', '', dataset, '', '', '', system]
    var failed = false

    //maybe relogging after request to relogin (eg expired session on server)
    if (gusername) {
        var question = 'Your session has timed out\nor been lost to another login or another computer or browser\nor the server has been restarted.'
        //current work is cleared without option to recover if detect login on another computer or browser
        if (glocked && gchangesmade)
            question += '\n\nWarning! Your current work on ' + gkey + ' will be lost if you quit.'
        question += '\n\nLogin again?'
        if (!(yield* exodusyesno(question, 1))) {

            //switch to login window
            exodussettimeout('window.location.assign("../index.html")', 1)

            //try to avoid unlocking on exit
            glocked = false
            setchangesmade(false)
            gkey = ''
            this.requesting = false
            db.requesting = false

            yield* exodusinvalid()
            failed = true
            //pity there is no way to abort script without generating an error
            //TODO avoid showing error message in catch clause if switching to index.html
            force_an_exit___please_ignore_this_message()

            return false

        }

        arguments[0] = gusername
    }

    while (true) {

        arguments[4] = datasets
        if (!(typeof event != 'undefined' && event && event.shiftKey) && !failed && (exodusgetcookie2('a', 'EXODUS', '') == 'true' || username)) {
            arguments[0] = username ? username : exodusgetcookie2('u', 'EXODUS', '')
            arguments[1] = password ? password : exodusgetcookie2('p', 'EXODUS', '')
            arguments[2] = dataset
            arguments[5] = exodusgetcookie2('a', 'EXODUS', '')
            arguments[6] = system
        }
        else {

            //if refreshing login
            if (dataset) {
                datasetx = new Object
                datasetx.code = new Object
                datasetx.code.text = dataset
                datasetx.name = new Object
                datasetx.name.text = dataset
                datasets = new Object
                datasets.group1 = [datasetx]
            }

            //otherwise get all datasets
            if (!datasets) {
                logindb.request = 'GETDATASETS'
                if (!(yield* logindb.send())) {
                    yield* exodusinvalid(logindb.response)
                    return 0
                }
                datasets = exodusxml2obj(logindb.data)
            }

            arguments[4] = datasets

            url = '../index.html'
            arguments = yield* exodusshowmodaldialog(url, arguments)

        }

        //quit if user cancels
        if (!arguments) return 0

        glogincode = (arguments[2] + '*' + arguments[0] + '*').replace(/ /g, '')
        exodussetcookie('', 'EXODUSlogincode', glogincode, 'logincode')

        logindb.request = 'LOGIN\r' + arguments[0] + '\r' + arguments[1] + '\r' + arguments[2] + '\r' + arguments[3] + '\r\r' + arguments[5]

        if (!(yield* logindb.send())) {
            var msg = logindb.response
            if (!msg)
                msg = 'Invalid username or password'
            var response = yield* exodusinvalid(msg)
            failed = true
        }
        else {

            //set the username and dataset
            gusername = arguments[0]
            gdataset = arguments[2]
            gsystem = arguments[6]
            this.dataset = gdataset
            this.system = gsystem
            this.username = gusername

            //permanent cookie
            var temp = 'dataset=' + dataset
            if (arguments[5].toString() == 'true') {
                temp += '&u=' + gusername
                temp += '&p=' + arguments[1]
                temp += '&a=' + arguments[5]
                temp += '&s=' + arguments[6]
            }
            else {
                //temp+='&u='
                //temp+='&p='
                //temp+='&a='
            }
            exodussetcookie('', 'EXODUS', temp, '')

            //temporary cookie for menu
            exodussetcookie(glogincode, 'EXODUS2', logindb.data)

            yield* clientfunctions_getglobals()

            //temporary cookie for the dataset and username (and password for file protocol)
            var temp = 'dataset=' + gdataset + '&username=' + gusername + '&system=' + gsystem
            if (document.protocolcode == 'file') {
                this.password = arguments[1]
                temp += '&password=' + this.password
                //this.timeout=exodusgetcookie2('timeout')
            }
            exodussetcookie(glogincode, 'EXODUS2', temp)

            //quit success
            return 1

        }
    }
}

function* exodusdblink_send_byhttp_using_forms(data) {

    //log(this.request)
    //alert('exodusdblink_send_byhttp_using_forms\n...\n'+this.request+'\n...\n'+ data)
    this.data = data ? data : ''

    //request is required
    if (this.request == '') {
        this.data = ''
        this.response = ('ERROR: NO REQUEST')
        this.response = this.response.replace(/"/, "'")
        this.result = ''
        return (0)
    }

    //try to use cache
    var request2 = this.request
    var trycache = (request2.slice(0, 6) == 'CACHE\r')
    if (trycache) {
        request2 = request2.slice(6)
        var temp
        if (temp = readcache(request2)) {
            this.data = temp
            return true
        }
    }

    var gotresponse = false
    while (!gotresponse) {

        //var params='dialogHeight:100px; dialogWidth:200px; center:Yes; help:No; resizable:No; status:No'
        //params='dialogHeight: 201px; dialogWidth: 201px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'
        var reply = yield* exodusshowmodaldialog(EXODUSlocation + 'rs/index.html', [this.timeout, this.request, this.data])
        if (!reply) {
            this.data = ''
            this.response = ('ERROR: Request to server failed')
            this.result = ''
            return 0
        }

        //extract the response parameters
        var result = reply[0].toString()
        this.response = reply[1].toString()
        this.data = reply[2].toString()

        var lcresponse = this.response.toLowerCase()
        if (lcresponse.indexOf('Please login') >= 0) {
            //if (lcresponse.indexOf('automatic') >= 0) {
            //    glocked=false
            //    gchangesmade=false
            //}
            if (!(/**/ yield* this.login())) {
                this.data = ''
                this.response = ('ERROR: Please login')
                this.result = ''
                return (0)
            }

        }
        else {
            gotresponse = true
        }

    }

    if (result.split(' ')[0] == 'OK' || result == 1) {

        //save in cache
        //perhaps only cache READO requests to avoid
        // initiating cache to speed things up
        if (this.documentprotocolcode == 'file' && (trycache || request2.slice(0, 4) == 'READ')) {
            writecache(request2, this.data)
        }

        return true
    }
    else {
        return false
    }

}

var gxhttp

function* exodusdblink_send_byhttp_using_xmlhttp(data) {

    //log(this.request)

    var ignoreresult = (typeof this.request == 'string') && (this.request.slice(0, 6) == 'RELOCK' || this.request.slice(0, 9) == 'KEEPALIVE')
    //indicate to refresher when last activity was
    if (ignoreresult)
        exodussetcookie('', 'EXODUSlc', new Date(), 'lc')

    //prevent reuse
    if (this.requesting) {
        this.data = ''
        this.response = ('ERROR: ALREADY REQUESTING:\r' + this.requesting + '\r --- \rNEW REQUEST:\r' + this.request + this.data)
        this.response = this.response.replace(/"/, "'")
        this.result = ''
        return 0
    }
    //this.requesting=true
    this.requesting = this.request

    this.data = data ? data : ''

    //request is required
    if (this.request == '') {
        this.data = ''
        this.response = ('ERROR: NO REQUEST')
        this.response = this.response.replace(/"/, "'")
        this.result = ''
        this.requesting = false
        return 0
    }

    //try to use cache
    var request2 = this.request
    var trycache = (request2.slice(0, 6) == 'CACHE\r')
    if (trycache) {
        request2 = request2.slice(6)
        var temp = readcache(request2)
        //returns undefined or null if not in cache
        if (temp || temp == '') {
            this.data = temp
            this.requesting = false
            return true
        }
    }

    var dbwaitingwindow
    var gotresponse = false
    var remoteprocedurecallfailed = false
    var rpcserverunavailable = false

    var origrequest = this.request

    while (!gotresponse) {

        var token = glogincode
        var timeout = ''
        var xtext = '<root>'
        xtext += '<token>' + escape(token) + '</token>'
        xtext += '<timeout>' + escape(timeout) + '</timeout>'
        xtext += '<request>' + escape(request2) + '</request>'
        var thisrequest = this.request
        this.request = ''
        //xtext+='<data>' + escape(this.data) + '</data>'
        //dont escape everything otherwise unicode will arrive in the database as encoded text
        //instead of being converted to system default single byte code page in server filesystem/asp
        //convert any ascii incompatible with xml text eg < > &
        //convert the escape character "%" FIRST
        //also convert revelation delimiters so that they arrive unconverted
        //XMLre is something like [\x25\x3C\x3E\x26\ plus the field and value marks etc
        temp = this.data.replace(XMLXXre, function ($0) { return escape($0) })

        xtext += '<data>' + temp + '</data>'
        xtext += '</root>'
        //if (gusername=='EXODUS') alert(xtext)
        /*
        //decide microsoft or moz/netscape
        var moz=false
        try {

        var xhttp = new ActiveXObject('Microsoft.XMLHTTP')
        //var xhttp = new ActiveXObject('Msxml2.XMLHTTP.3.0')
        }
        catch(e) {

        var xhttp=new XMLHttpRequest()
        moz=true
        }
        */

        var xdoc = xtext

        //package description the request parameters into an xml object
        //if (!moz)
        //{
        // var xdoc = new ActiveXObject('Microsoft.XMLDOM')
        // xdoc.loadXML(xtext)
        //}

        var dbwaitingwindow
        var gotresponse = false
        var remoteprocedurecallfailed = false
        var rpcserverunavailable = false

        var xhttp = this.XMLHTTP
        //var servererrormsg='ERROR: Could not communicate with server. Try again later\r\r'
        var servererrormsg = 'Network or server failure. (No response)\r\rPlease try again later\rand/or get technical assistance\r\r'
        var xhttpaborted = false

        //g because perhaps will be a global variable
        //var gasynchronous=guseyield && !isMSIE && (gusername=='EXODUS' || (typeof gdataset != 'undefined' && gdataset.slice(-4) == 'TEST'))
        var gasynchronous = guseyield && !(gonunload || gonbeforeunload)
        if (gasynchronous) {

            gchildwin = { lazy: true }
            //a reference to xhttp so we can call abort on it if user chooses Cancel in the popup from blockui
            gchildwin.xhttp = xhttp

            //a reference to xhttp so we can call abort on it if user chooses to close the window
            gxhttp = xhttp

            xhttp.onload = function (e) {
                if (xhttp.readyState === 4) {
                    if (xhttp.status === 200) {
                        //console.log(xhttp.responseText);
                        exodus_resume('ok', 'OK exodusdblink_send_byhttp_using_xmlhttp');
                    } else {
                        console.error(xhttp.status + ' ' + xhttp.statusText);
                        exodus_resume('ok', 'OK exodusdblink_send_byhttp_using_xmlhttp ' + xhttp.status + ' ' + xhttp.statusText);
                    }
                }
            };
            xhttp.onerror = function (e) {
                console.error(xhttp.status + ' ' + xhttp.statusText);
                exodus_resume('error', 'ERROR exodusdblink_send_byhttp_using_xmlhttp' + xhttp.status + ' ' + xhttp.statusText);
            };
            xhttp.ontimeout = function () {
                console.error("The request for " + url + " timed out.");
                exodus_resume('timeout', 'TIMEOUT exodusdblink_send_byhttp_using_xmlhttp');
            };
            xhttp.onabort = function (e) {
                console.error('XMLHTTPREQEST ABORTED --- ' + xhttp.statusText);
                xhttpaborted = true
                if (!gonunload)
                    exodus_resume('abort', 'ABORT exodusdblink_send_byhttp_using_xmlhttp');
            };
        }

        //open
        //console.log('gasynchronous='+gasynchronous)

        //send the xmldoc to the server and get the response
        try {
            //false for synchronous (wait for response before continuing to next statement)
            //xhttp.open('POST', EXODUSlocation + 'scripts/xhttp.asp', async = gasynchronous || ignoreresult)
            xhttp.open('POST', EXODUSlocation + 'scripts/xhttp.php', async = gasynchronous || ignoreresult)
            //this was never required but inserted in the hope that it will
            //avoid unknown problems. Perhaps it is not necessary for active pages like .asp.
            xhttp.setRequestHeader("Pragma", "no-cache");
            xhttp.setRequestHeader("Cache-control", "no-cache");
            //xhttp.setRequestHeader('Content-type','application/x-www-form-urlencoded');
            //consider also putting the following in asp web pages
            //<% Response.CacheControl = "no-cache" %>
            //<% Response.AddHeader "Pragma", "no-cache" %>
            //<% Response.Expires = -1 %>

            //following will asynchronously handle relock failed due to login where
            //by preventing further relocking
            /* actually relocking on duplicate sessions is allowed for the time being TODO block it
            if (ignoreresult) {
                xhttp.onreadystatechange=
                //TODO consider what happens in cases other than 200 and no response at all
                function() {
                    if (xmlhttp.readyState==4 && xmlhttp.status==200) {
                        if (http.responseText.indexOf('Please login')>=0) {
                            stoprelocker()
                        }
                    }
                }
            }
            */

        }
        catch (e) {
            this.data = ''
            this.response = servererrormsg + e.number + ' ' + e.description + ' in .open()'
            this.result = ''
            dbready(dbwaitingwindow)
            this.requesting = false
            return 0
        }

        //prevent error "object does not support property or method '.loadXML'" in MSIE 10 plus and warnings in other browsers
        //https://blogs.msdn.microsoft.com/ie/2012/07/19/xmlhttprequest-responsexml-in-ie10-release-preview/
        //fails in IE6?
        if (isMSIE)
            try {
                xhttp.responseType = 'msxml-document'
            } catch (e) { }

        //send

        try {

            //needs Tools, Internet Options, Security, Trusted Sites, Sites, Add, Close, OK
            var action = 'Requested ' + new Date() + ' ' + thisrequest
            //dont show pass!
            if (!glogging && thisrequest.indexOf('LOGIN') < 0)
                window.status = action

            var timesent = new Date
            //alert('DEBUG: pre:'+thisrequest)
            xhttp.send(xdoc)
            //alert('DEBUG: post:'+thisrequest)

            //log(action + ' ' + (new Date() - timesent) + 'ms')
            window.status = ''

            //force an error if ignoreresult since ignoreresult ignores all errors or success
            if (ignoreresult)
                forceanerrorhere = forceanerrorhere

        }
        catch (e) {

            //ignore any errors while ignoreresult
            if (ignoreresult) {
                this.data = ''
                this.response = 'OK'
                this.result = ''
                this.requesting = false
                return true
            }

            //if (e.number==-2146697211) is "The system cannot locate the resource specified"
            this.data = ''
            this.response = servererrormsg + ' in .send()\n\n' + (e.number ? (e.number + ' ' + e.description) : e.toString())
            this.response = this.response.replace(/"/, "'")
            this.result = ''
            dbready(dbwaitingwindow)
            this.requesting = false
            return 0
        }

        //yield here and wait for .send to complete
        if (gasynchronous) {

            ///////////////////////////////////////////////////////////////
            //PAUSE HERE until child window closes and our autoresume
            //function calls .next(childwin.returnValue) to put into result
            ///////////////////////////////////////////////////////////////
            var result = yield* exodus_yield('exodusdblink_send_byhttp_using_xmlhttp : ' + thisrequest)

            //ignoring result since xhttp contains error codes
            //maybe could process differently
            //if (result=='error') {}
            //else if (result=='timeout') {}
            //else if (result=='abort') {}
            //else if (result=='ok') {}
            //else {/*unexpected result*/}

        }

        dbready(dbwaitingwindow)

        //response

        //check for valid response
        if (xhttp.responseXML && xhttp.responseXML.firstChild != null) {

            //extract the response parameters
            var responsex = xhttp.responseXML.firstChild
            if (true || responsex.childNodes[0].childNodes) {
                //moz
                //this.data=unescape(responsex.childNodes[0].childNodes[0].nodeValue)
                //this.response=unescape(dbgetnodevalues(responsex.childNodes[1]))
                //var result=unescape(dbgetnodevalues(responsex.childNodes[2]))
                this.data = unescape(dbgetnodesvalue(xhttp.responseXML.getElementsByTagName('data')[0].childNodes))
                this.response = unescape(dbgetnodesvalue(xhttp.responseXML.getElementsByTagName('response')[0].childNodes))
                var result = xhttp.responseXML.getElementsByTagName('result')
                if (result.length)
                    result = unescape(dbgetnodesvalue(result[0].childNodes))
                else
                    result = ''
                //alert('result '+result+' typeof result'+typeof result)
                if (typeof result == 'undefined') {
                    debugger
                }
            }
            else {
                //iexplorer
                this.data = unescape(responsex.childNodes[0].text)
                this.response = unescape(responsex.childNodes[1].text)
                var result = unescape(responsex.childNodes[2].text)
            }

            if (this.response.indexOf('Please login') >= 0) {
                //                var tt=origrequest.slice(0,5)!='LOGIN'?gusername:''
                //                tt=gusername
                //alert(origrequest+' gusername='+tt)
                tt = ''
                if (!(/**/ yield* this.login(tt))) {
                    this.data = ''
                    this.response = ('ERROR: Please login')
                    this.result = ''
                    this.requesting = false
                    return 0
                }

            }
            else {
                gotresponse = true
            }

        }
        else {

            //if iis process is killed (dllhost.exe/IWAM_xxx) then it doesnt work on the first call
            if (!remoteprocedurecallfailed && xhttp.responseText.indexOf('remote procedure') >= 0) {
                remoteprocedurecallfailed = true
                continue
            }

            //if iis process is killed then it doesnt work on the first call
            if (!rpcserverunavailable && xhttp.responseText.indexOf('RPC server') >= 0) {
                rpcserverunavailable = true
                continue
            }

            //aborting/unloading. try to avoid error message in caller
            if (gonunload || gonbeforeunload || xhttpaborted) {
                this.data = ''
                if (xhttpaborted)
                    this.response = 'Cancelled'
                else
                    this.response = ''
                this.result = ''
                this.requesting = false
                return false
            }

            //system failure, retry/abort?
            //Could be failure to convert server response to xml ... check browser debugger console for errors
            //sometimes caused by unusual data encoding in back end from old data entry systems. Latin versus UTF8.
            var tt = origrequest.split('\r')
            if (tt[0] == 'LOGIN')
                //remove password from message
                tt[2] = ''
            var errormsg = 'System failure. Do you want to retry?\r' + tt.join('\r')
            if (xhttp.responseText.indexOf('ASP 0113') > -1)
                errormsg += '\n\nThe server failed to get a response from the EXODUS database engine'
            errormsg += '\n\n' + unescape(striptags(xhttp.responseText))
            console.log(errormsg)
            if (!(confirm(errormsg))) {
                //xxx
                this.data = ''
                this.response = ('ERROR: ' + unescape(xhttp.responseText))
                this.response = this.response.replace(/"/, "'")
                this.result = ''
                this.requesting = false
                return 0
            }

        }

    }

    //indicate to refresher when last activity was
    exodussetcookie('', 'EXODUSlc', new Date(), 'lc')

    ///if (result.split(' ')[0]=='OK' || result==1)
    if (result.split(' ')[0] == 'OK' || result == 1) {

        //save in cache
        //perhaps only cache READO requests to avoid
        // initiating cache to speed things up
        if (trycache || request2.slice(0, 4) == 'READ')
            writecache(request2, this.data)

        this.requesting = false
        return true
    }
    else {
        this.requesting = false
        return false
    }

}

function dbgetnodesvalue(nodes) {
    var text = ''
    var nn = nodes.length
    for (var ii = 0; ii < nn; ii++)
        text = text + nodes[ii].nodeValue
    return text
}

function dbwaiting() {

    document.body.style.cursor = 'wait'
    var dialogstyle = 'dialogHeight: 50px; dialogWidth: 200px; dialogTop: px; dialogLeft: px; center: Yes; help: No; resizable: Yes; status: No;'
    var windowx = ''//window.showModelessDialog(EXODUSlocation+'wait.htm','',dialogstyle)
    wstatus('Networking, please wait ...')
    return windowx
}

function dbready(windowx) {

    document.body.style.cursor = 'auto'
    wstatus('')
    if (windowx) try { windowx.close() } catch (e) { }
}

//fix a bug/feature in internet explorer where closing a window opened with window.open causes loss of all non permanent cookies
function exodusfixcookie() {
    var cookies = document.cookie.split('; ')
    var npreservedcookies = 0
    var ntopreserve = 10
    //preserve first ntopreserve ASPSESSION type cookies
    for (var ii = 0; ii < cookies.length && npreservedcookies < ntopreserve; ii++) {
        npreservedcookies++
        if (cookies[ii].slice(0, 10) == 'ASPSESSION') {
            cookies[ii] += '; expires=Sun, 31 Dec 2100 23:59:59 GMT'//; path=/'
            document.cookie = cookies[ii]
            //do all
            //return true
        }
    }
    return true
}

function exodussetcookie(loginsessionid, name, value, subkey, permanent) {

    if (glogcookie)
        console.log('EXODUSSETCOOKIE(' + loginsessionid + ', ' + name + ', ' + value + ', ' + subkey + ', ' + permanent + ')')

    //exodusfixcookie()

    //for any particular permanent cookie it must consistently be set true otherwise possible loss of following cookies including ASPSESSION

    /*
    var cookie0='set cookie session:'+loginsessionid+' name:'+name+' subkey:'+subkey+' value:'+value+'\r'
    alert(cookie0)
    var tt=document.cookie.split('; ')
    for (var ii=0;ii<tt.length;ii++) cookie0+='\r'+ii+' '+tt[ii]
    if (permanent&&!(confirm(cookie0))) xyx
    if (permanent&&typeof permanent!='boolean') xyxxxxxxxxx
    */
    if (!permanent)
        permanent = false

    //value can be multiple values and keys in cookie format x=1&y=2 etc
    //to update many without replacing all
    value = value.toString()
    if (value.indexOf('=') >= 0) {
        var values = value.split('&')
        for (var valuen = values.length - 1; valuen >= 0; valuen--) {
            var valuex = values[valuen]
            if (valuex) {
                valuex = valuex.split('=')
                if (valuex.length < 2) valuex[1] = ''
                exodussetcookie(loginsessionid, name, valuex.slice(1).join('='), valuex[0], permanent)
            }
        }
        return
    }

    if (subkey) {
        // crumbs are separated by ampersands
        var crumbs = exodusgetcookie2('', name, loginsessionid).split('&')
        var emptycrumbn
        for (var i2 = 0; i2 < crumbs.length; i2++) {
            var crumb0 = crumbs[i2].split('=')[0]
            if (crumb0 == subkey) {
                crumbs[i2] = subkey + '=' + value
                subkey = ''
                break
            }
            if (crumb0 == '') emptycrumbn = i2
        }
        if (subkey) {
            if (typeof emptycrumbn != 'undefined') i2 = emptycrumbn
            crumbs[i2] = subkey + '=' + value
        }
        value = crumbs.join('&')
    }
    //if (value.indexOf('&&')>=0) xxxxxx

    //document.cookie='EXODUSsystem=ACCOUNTS'//; expires=Fri 31 Dec 2100 23:59:59 GMT; path=/'
    //value+=';expires=Fri 31 Dec 2000 23:59:59 GMT;'
    var cookie = loginsessionid + name + '=' + escape(value)
    //if (permanent==true)
    if (value.length)
        //x = 1
        cookie += '; expires=Fri, 31 Dec 2100 23:59:59 GMT; SameSite=Strict'
    else
        cookie += '; expires=Fri, 31 Dec 2000 23:59:59 GMT; SameSite=Strict'
    //else if (permanent) cookie+=permanent

    //problem on safari5 on mac - seems to cause multiple paths if path=/ without domain name (only if ipno?)
    //and isnt necessary since without path then cookie is available to all pages
    //but IS necessary because the default path on login page and other pages is DIFFERENT!
    //summary of issues
    //1. PC and Mac (all browsers) can login but NOT logout with no path=/ or no domain
    //2. PC and Mac (all browsers) can login and out with path=/ and no domain - EXCEPT safari cannot login
    //3. Mac (all browsers) can login and out with path=/ and domain
    //doesnt seem to make any difference if domain or ip number
    //cookie=cookie.replace(/&/g,'#')
    //if (cookie.indexOf(';')>=0)
    // alert(cookie)
    //cookie=cookie.replace(/;/g,'?')
    if (isMac) {
        //cookie+='; path=/'
        //var urlbit=document.location.href.toString().split('/')
        //urlbit=urlbit[0]+'//'+urlbit[2]+'/'
        cookie += '; path=/'
        //cookie='xyz=steve&abc=123; path=/'
        //alert(cookie)
    }
    else
        cookie += '; path=/'
    //cookie+='; domain='+gdomainname

    //cookie=cookie.replace(/=/,'!')
    document.cookie = cookie
    //alert(loginsessionid+name+'\n'+value+'\n'+document.cookie)
    //document.cookie = name + '=' + value+ 'domain='+window.location.host//+'; path=/;'

    //if (name=='EXODUS2')
    // alert(cookie+'\r\r'+document.cookie)

}

function exodusgetcookie2(subkey, key, loginsessionid) {
    if (!loginsessionid && loginsessionid !== null)
        loginsessionid = glogincode
    else if (loginsessionid === null)
        loginsessionid = ''
    if (!key)
        key = 'EXODUS2'
    var result = exodusgetcookie(loginsessionid, key, subkey)
    ///console.log('exodusgetcookie2('+loginsessionid+', '+key+', '+subkey+')='+result)
    return result
}

// Retrieve the value of the cookie with the specified name
function exodusgetcookie(loginsessionid, key, subkey) {

    if (glogcookie)
        var log = 'exodusgetcookie(' + loginsessionid + ', ' + key + ', ' + subkey + ')'

    //var cookie0='GET COOKIE session:'+loginsessionid+' key:'+key+' subkey:'+subkey
    //alert(cookie0)
    //cookies are separated by semicolons
    //accessing cookies in modaldialog windows when there is port number in the URL
    //cause unspecified security error in IE6 not prior versions
    var cookies = (window.dialogArguments && window.dialogArguments.cookie) ? window.dialogArguments.cookie : document.cookie
    //if (window.dialogArguments&&window.dialogArguments.cookie)
    //alert('W='+window.dialogArguments&&window.dialogArguments.cookie)
    //else
    //alert('C='+document.cookie)
    //cookies=cookies.replace(/#/g,'&')
    //cookies=cookies.replace(/!/,'=')
    cookies = unescape(cookies)
    cookies = cookies.split('; ')
    var fullkey = loginsessionid + key
    for (var i = 0; i < cookies.length; i++) {
        if (cookies[i].split('=')[0] == fullkey) {

            if (typeof subkey == 'undefined' || subkey == '') {
                //return unescape(cookies[i].split('=')[1])
                //return unescape(cookies[i].split('=').slice(1).join('='))
                var cookie = cookies[i].split('=').slice(1).join('=')
                if (glogcookie)
                    console.log(log + ' = "' + cookie + '"')
                return cookie
            }

            // crumbs are separated by ampersands
            var crumbs = cookies[i].substr(fullkey.length + 1).split('&')
            for (var i2 = 0; i2 < crumbs.length; i2++) {
                if (crumbs[i2].split('=')[0] == subkey) {
                    //return unescape(crumbs[i2].split('=')[1])
                    var cookie = unescape(crumbs[i2].split('=').slice(1))
                    if (glogcookie)
                        console.log(log + ' = "' + cookie + '"')
                    return cookie
                }
            }
        }
    }
    if (glogcookie)
        console.log(log + ' = ""')
    return ''
}

//from "client.js" may also be copied in some "client.js" less windows
function* exodusdecide(question, data, cols, returncoln, defaultreply, many, inverted) {
    //data and cols are [[]] or [] or revstr or a;1:b;2 string
    //data cells .text property will be used if present
    //returncoln '' means return row number(s) - 1 based
    //returncoln means column number or property name in data
    // (data columns are usually numeric and 0 based)
    //defaultreply=if returncoln then cell(s) of returncoln otherwise 1 based rown(s)

    if (typeof data == 'undefined')
        data = 'Yes:No'

    //the data might be a db request prefixed with '@'
    if (typeof data == 'string' && data.slice(0, 1) == '@') {
        db.request = data.slice(1)
        if (!(yield* db.send())) {
            yield* exodusinvalid(db.response)
            return null
        }
        data = db.data

    }

    //abort if no records found
    if (data == '' || data == '<records>\r\n</records>')
        return yield* exodusinvalid('No records found')

    if (typeof data == 'string' && data.slice(0, 8) == '<records')
        data = exodusxml2obj(data)

    //xml2obj returns records in .group1
    if (data.group1)
        data = data.group1

    if (defaultreply) {
        if (typeof defaultreply == 'string')
            defaultreply = defaultreply.split(':')
        if (typeof defaultreply != 'object')
            defaultreply = [defaultreply]
    }

    if (!inverted)
        inverted = false
    var dialogargs = exoduscloneobj([question, data, cols, returncoln, defaultreply, many, inverted])
    dialogargs.logincode = glogincode

    //var dialogstyle='dialogHeight: 400px; dialogWidth: 600px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'

    //if (guseyield && gdataset.slice(-4)=='TEST')
    //if (guseyield)
    var results = yield* exodusconfirm2(dialogargs)
    if (typeof results == 'undefined')
        results = ''

    return rearray(results)

}

function rearray(array) {
    //fix a bug where arrays returned from modaldialogs are missing the method .join()

    if (!array || typeof array != 'object')
        return array

    var rearray = []
    for (var i = 0; i < array.length; i++)
        rearray[i] = array[i]

    return rearray

}

function* exodusdecide2(question, data, cols, returncoln, defaultreply, many) {

    //new in-window style popup
    //if (gdataset.slice(-4)=='TEST') {
    if (guseyield) {

        //if row columns are not numeric then convert numeric return column number into named column in data rows
        //also done in decide_onload() after splitting string into columns (could remove from here)
        //alert('data:'+data+'\ncols:'+cols+'\nreturncoln:'+returncoln)
        if (typeof cols != 'string') {
            var tt = cols[returncoln]
            if (tt && tt[0])
                returncoln = tt[0]
            //empty returncoln means return row number(s) of option(s) selected
            //if (!returncoln)
            //    returncoln=0
        }
        var results = yield* exodusdecide(question, data, cols, returncoln, defaultreply, many)

        //callers of decide2 expect reply in array
        if (results && (typeof results == 'string' || typeof results == 'number'))
            results = [results]

        return results
    }

}

function* setdropdown2(element, dataobj, colnames, selectedvalues, requiredvalues, noautoselection) {

    //1st element is automatically selected unless noautoselection (or selectedvalues overrides it)

    if (!(checkisdropdown(element)))
        return 0

    cleardropdown(element)

    //if (no(xmldata.text)) return(0)
    if (no(dataobj.group1))
        return 0

    cleardropdown(element, true)

    var valuecolname
    var textcolname

    if (typeof colnames == 'string' && colnames.indexOf('\r'))
        colnames = colnames.split('\r')

    if (typeof (colnames) == 'string') {
        textcolname = colnames
        valuecolname = colnames
    }
    else {
        valuecolname = colnames[0]
        textcolname = colnames[1]
        if (colnames[2])
            noautoselection = colnames[2]
    }

    //var records=xmldata.firstChild.childNodes
    var records = dataobj.group1

    //var options=[]
    var anyselected = false

    for (var i = 0; i < records.length; i++) {
        var cell = records[i][valuecolname]
        if (typeof (cell) == 'undefined') {
            //yield* exodusinvalid('Error: "' + valuecolname + '" not in data line ' + i + ' for setdropdown2 for "' + element.id + '" (1)')
            systemerror('Error: "' + valuecolname + '" not in data line ' + i + ' "' + records[i] + '" for setdropdown2 for "' + element.id + '" (1)')
            return (0)
        }

        var value = cell.text

        if (no(requiredvalues) || requiredvalues.exoduslocate(value)) {
            var option1 = document.createElement('option')
            option1.value = value

            var cell = records[i][textcolname]
            if (typeof (cell) == 'undefined') {
                //yield* exodusinvalid('Error: "' + textcolname + '" not in data line ' + i + ' for setdropdown2 for "' + element.id + '" (2)')
                systemerror('Error: "' + textcolname + '" not in data line ' + i + ' for setdropdown2 for "' + element.id + '" (2)')
                return (0)
            }

            //start a new option
            //var option='<OPTION value='+value

            //indicate if selected
            var selected = false
            if (selectedvalues && selectedvalues.exoduslocate(value)) {
                //option+=' selected=true'
                option1.selected = true
                anyselected = true
            }

            var description = decodehtmlcodes(cell.text)

            //add the option description
            //must be done before insertion otherwise width on mac ie5 is v small
            option1.innerHTML = description
            //option+=cell.text

            //add into the array of options
            element.insertBefore(option1, null)
            //element.options.add(option1)
            //options[options.length]=option+'</OPTION>'+'\r\n'

        }

    }

    if (noautoselection && !anyselected) {
        var blankoption = document.createElement('option')
        blankoption.value = ''
        blankoption.innerHTML = ''
        element.insertBefore(blankoption, element.childNodes[0])
    }

    //select first option if none selected
    // if (element.selectedIndex==-1) element.selectedIndex=0

    //force element to recalculate width (needed on msie55 but not mac ie5)
    //this work around probably no longer needed
    //element.parentNode.replaceChild(element,element)

    //does not seem to do anything
    //getancestor(element,'TABLE').refresh()

}

function cleardropdown(element, all) {

    //login('cleardropdown')

    if (!(checkisdropdown(element))) return (0)

    //remove existing selections (leave one but blank it to avoid "0" showing)
    while (element.childNodes.length > (all ? 0 : 1)) {
        //  element.childNodes[0].removeNode(true)
        //element.removeChild(element.childNodes[0])
        exodusremovenode(element.childNodes[0])
    }
    if (element.childNodes.length) {
        element.childNodes[0].value = ""
        element.childNodes[0].text = ""
    }
    //logout('cleardropdown')
}

function cleardropdownselections(element) {

    if (!(checkisdropdown(element)))
        return (0)

    //    element.selectedIndex=-1
    if (element.childNodes.length)
        element.childNodes[0].selected = true

    //remove existing selections
    for (var i = 0; i < element.childNodes.length; i++) {
        element.childNodes[i].selected = false
    }
}

function checkisdropdown(element) {
    assertelement(element, 'checkisdropdown', 'element')

    if (typeof (element) != 'object' || element.tagName != 'SELECT') {
        yield * exodusinvalid('Error: The target is not a SELECT tag')
        return false
    }
    return true
}

function setdropdown3(element, dropdowndata, colns, selectedvalues, requiredvalues) {

    //NB cannot update elements options if currently bound to data because IE busts?
    //try clonenode+replacenode?

    //parameters
    ////////////

    //dropdowndata can be a string (even '') with character seps :; or fm vm (used if vm or fm found)
    //if null will not set new options, only select the selectedoptions
    //exodusrequired false means add a blank option at the beginning

    //element must be a select element
    if (!(checkisdropdown(element))) {
        //log('setdropdown3 - not a select element')
        return false
    }

    ////login('setdropdown3 '+element.id+' '+exodusquote(dropdowndata))
    //login('setdropdown3 ' + element.id)

    //dropdowndata may be an array of strings or a string with ;: seps for cols/rows respectively
    //if [] then dropdowndata are not updated otherwise existing dropdowndata are deleted
    if (typeof (dropdowndata) == 'undefined' || dropdowndata == null) dropdowndata = []

    //convert conversion string to an array
    if (typeof (dropdowndata) == 'string') {

        //make sure we get at least one conversion
        if (dropdowndata == '') dropdowndata = ';'

        //convert into an array
        var sepchars = (dropdowndata.indexOf(vm) >= 0 || dropdowndata.indexOf(fm) >= 0) ? fm + vm : ':;'
        dropdowndata = dropdowndata.exodussplit(sepchars)

    }

    //if not required make sure that the first dropdown option is ""
    if (dropdowndata.length && !element.getAttribute('exodusrequired')) {
        if (dropdowndata[0][0] != '' && dropdowndata[dropdowndata.length - 1][0] != '') {
            //ripple up, then replace first element
            for (var i = dropdowndata.length; i > 0; i--) dropdowndata[i] = dropdowndata[i - 1]
            dropdowndata[0] = ['', '']
        }
    }

    // if (dropdowndata.length) yield* exodusbreak(element.id+' '+element.getAttribute('exodusrequired')+' '+dropdowndata.tostatement())

    //colns may be array(2) being pointers into dropdowndata for option value and text
    //otherwise [0,1]
    if (!(is(colns))) colns = [0, 1]
    valuecoln = colns[0]
    textcoln = colns[1]

    //selectvalues is optionally the value(s) that are to be "selected"
    //optional, can be array or single
    if (typeof (selectedvalues) == 'undefined') selectedvalues = []
    if (typeof (selectedvalues) != 'object') selectedvalues = [selectedvalues]

    //requiredvalues is optionally the value(s) that are to be included in the dropdown
    //optional, can be array or single
    if (typeof (requiredvalues) == 'undefined') requiredvalues = []
    if (typeof (requiredvalues) != 'object') requiredvalues = [requiredvalues]

    //yield* exodusinvalid(selectedvalues.join())
    //method
    ////////

    if (dropdowndata.length) {

        cleardropdown(element, true)

        for (var i = 0; i < dropdowndata.length; i++) {
            var value = dropdowndata[i][valuecoln]
            if (!value)
                value = ''
            if (no(requiredvalues) || requiredvalues.exoduslocate(value)) {

                var text = dropdowndata[i][textcoln]
                if (typeof (text) == 'undefined' || text == '') {
                    text = value
                    if (typeof text == 'undefined') text = ''
                    else text = text.toString().exodusconvert('_', ' ').exoduscapitalise()
                }

                addoption(element, value, text)

            }
        }
    }

    //mark selected options
    if (selectedvalues.length) {

        //clear existing selections
        if (!dropdowndata.length) cleardropdownselections(element)

        for (var j = 0; j < selectedvalues.length; j++) {
            var selectedvalue = selectedvalues[j]
            for (var i = 0; i < element.childNodes.length; i++) {
                var option = element.childNodes[i]
                //if (selectedvalues.exoduslocate(option.value)) option.selected=true
                //if (selectedvalues.exoduslocate(option.text)) option.selected=true
                if (option.value == selectedvalue || option.text == selectedvalue) {
                    try {
                        option.selected = true
                    } catch (e) { }//error in ie6 sometimes
                    break
                }
            }

            //add option if setting to a value not in the dropdown
            //(or error)
            if (i >= element.childNodes.length) {
                //except setting to '' means reset to first option

                if (selectedvalue == '') {
                    // (also dataitem.text! why???)
                    // but this has the result that
                    //all first row drop downs are reset to first option
                    //in the binding blankrowx so do not do it
                    //ok do it only if bound and make sure not bound while clearing line 1
                    if (element.dataitem) element.dataitem.text = element.value
                }
                else {
                    //     systemerror('setdropdown3()','"'+selectedvalue+'" is not in the dropdown')
                    option = addoption(element, selectedvalue, selectedvalue)
                    option.selected = true
                }
            }

        }

    }

    //force option 1 if no options
    // if (element.getAttribute('exodusfieldno')!='0'&&element.selectedIndex==-1) element.selectedIndex=0

    //logout('setdropdown3')

    return true

}

function addoption(element, value, text) {

    //build an option
    var option1 = document.createElement('option')
    option1.value = value
    option1.text = text

    //insert it into the dropdown
    //element.add(option1)//does not work on the mac for option 1 after clearing
    element[element.length] = option1

    return element[element.length - 1]

}

function exodusxml2obj(xmltext) {

    var dataobj = new Object
    dataobj.group1 = []
    var recn = -1
    var currentrow
    xml = xmltext.split('<')
    for (var fragn = 1; fragn < xml.length; fragn++) {
        var frag = xml[fragn].split('>')
        if (frag[0].toLowerCase() != 'records' && frag[0].toLowerCase() != '/records') {
            if (frag[0].toLowerCase() == 'record') {
                dataobj.group1[++recn] = currentrow = new Object
            }
            else {
                if (frag[0].slice(0, 1) != '/') {
                    //currentrow[frag[0]]=(new Object).text=frag[1]
                    var cell = new Object
                    var fragdecoded = HTMLDecode(frag[1]).replace(/%25/g, '%')
                    cell.text = fragdecoded
                    currentrow[frag[0]] = cell
                }
            }
        }
    }

    return dataobj

}

function* exodussetdropdown(element, request, colarray, selectedvalues, noautoselection) {

    if (!(checkisdropdown(element)))
        return (0)

    db.request = request
    if (yield* db.send()) {

        //console.log('exodussetdropdown:'+db.data)

        dataobj = exodusxml2obj(db.data)

        //xmltemp=new ActiveXObject('Microsoft.XMLDOM')
        //xmltemp.loadXML(db.data.replace(/\&/g,'+'))
        ////xmltemp.loadXML(db.data)
        yield* setdropdown2(element, dataobj, colarray, selectedvalues, null, noautoselection)
        if (db.data == '' || !dataobj.group1.length)
            element.setAttribute('exodusdropdown', '')
    }
    else {
        yield* exodusinvalid(db.response)
    }
    return true
}

function getdropdown0(element) {

    var index = element.selectedIndex
    // yield* exodusbreak(index)

    //ie5 on mac appears to use index=length sometimes (when only one option?)
    if (index >= element.length) index = 0

    //ie5 on pc uses -1 to indicate not selected
    if (index < 0) return ''

    return element[index].value

}

function exodusgetdropdown(element, mode) {
    if (!(is(mode)))
        mode = 'selected'//otherwise 'all'
    var selectedvalues = []
    for (var i = 0; i < element.options.length; i++) {
        if (mode == 'all' || element.options[i].selected) {
            selectedvalues[selectedvalues.length] = element.options[i].value
            if (mode != 'all' && !element.multiple)
                return selectedvalues.join()
        }
    }
    if (mode != 'all' && !element.multiple)
        selectedvalues = selectedvalues.join()
    return selectedvalues
}

function* exodusinput(question, text, texthidden) {
    if (!text) text = ''
    return yield* exodusconfirm(question, '', 'OK', '', 'Cancel', text, texthidden)
}

function* exodusyesno(question, defaultbutton) {
    return ((yield* exodusconfirm(question, defaultbutton, 'Yes', 'No')) == 1)
}

function* exodusokcancel(question, defaultbutton) {
    return yield* exodusconfirm(question, defaultbutton, 'OK', '', 'Cancel')
}

function* exodusconfirm(question, defaultbutton, yesbuttontitle, nobuttontitle, cancelbuttontitle, text, texthidden, image) {

    //clean up question
    if (!question)
        question = 'OK to continue?'
    question = question.toString()
    if (question.slice(0, 6) == 'Error:')
        question = question.slice(6)

    console.log(question)

    var istextinput = typeof text != 'undefined' && text !== null

    //use div to avoid opening a new window if possible
    //if ((!gusername || gusername=='EXODUS') && guseyield && !istextinput) {
    if (guseyield && !istextinput) {

        return yield* exodusconfirm2(question, defaultbutton, yesbuttontitle, nobuttontitle, cancelbuttontitle, text, texthidden, image)

        //use separate window for popup if yield is not available eg internet explorer
    } else {

        var dialogargs = [question, defaultbutton, yesbuttontitle, nobuttontitle, cancelbuttontitle, text, texthidden, image]

        //var dialogstyle
        //dialogstyle=(question.indexOf('\r')>=2)
        //?'dialogHeight: 300px; dialogWidth: 600px;'
        //:'dialogHeight: 220px; dialogWidth: 500px;'
        //dialogstyle+=' center: Yes; help: No; resizable: No; status: No;'

        //similar code in blockmodalui_sync and exodusconfirm
        var newwidth = 200
        var newheight = 150
        var max=getmaxwindow_sync()
        var newleft = 0 + (max.width - newwidth) / 2
        var newtop = 0 + (max.height - newheight) / 2
        var dialogstyle = 'top='+newtop+', left='+newleft+', width='+newwidth+', height='+newheight
        //sadly has no effect
        dialogstyle +', menubar=no, scrollbars=no, status=no, titlebar=no, toolbar=no'

        var response = yield* exodusshowmodaldialog(EXODUSlocation + 'confirm.htm', dialogargs, dialogstyle)
    }

    //text input returns a string (may be zero length) or false if clicked cancel
    if (istextinput) {
        if (typeof response == 'string')
            return response
    }

    //no response treated same as cancel button (0)
    if (!response)
        response = 0

    return response

}

function striptags(string) {
    var temp = document.createElement('div')
    temp.innerHTML = string
    return temp.innerText
}

/*
function* makeXMLisland(xmlelement,cmd) {

db.request=cmd+'\rXML'
if(!yield* db.send()) {

yield* exodusinvalid(db.response)
return false
}
xmlelement.loadXML(db.data)
//document.writeln('<XML id='+id+'>')
//document.writeln(db.data)
//document.writeln('</XML>')
}
*/

function getancestor(startelement, ancestorTag) {

    if (startelement == null) return (null)

    var ancestor = startelement.parentNode
    //while (ancestor!=null && ancestor.tagName!=ancestorTag)
    ancestorTag = ancestorTag.toUpperCase()
    while (ancestor != null && ancestorTag.indexOf(ancestor.tagName) == -1) {
        ancestor = ancestor.parentNode
    }
    return (ancestor)
}

/////////////////
//DEBUG FUNCTIONS
/////////////////

function wstatus(msg) {
    //needs Tools, Internet Options, Security, Trusted Sites, Sites, Add, Close, OK
    //or "Allow scripting to update status bar"
    window.status = msg
}

function loadcache() {

    //if gcache available already
    try {
        //ensure we can still access gcache.values since permission can be denied in MSIE if parent window has been closed
        if (typeof gcache == 'object' && gcache != null && gcache.values)
            return true
    } catch (e) {
        gcache = null
    }

    //login('loadcache')
    //sometimes error on accessing window.opener.gcache
    //Error: Permission denied to access property "gcache"
    try {
        //if (window.opener && window.opener.gcache) {
        if (window.opener && window.opener.gcache && gdataset && window.opener.gdataset == gdataset) {
            gcache = window.opener.gcache
            //ensure we can access gcache.values since permission can be denied in MSIE if parent window has been closed
            if (gcache.values)
                temp = true
        }
    } catch (e) {
        gcache = null
    }

    if (!gcache) {
        //create a global variable span to hold the userdata
        gcache = document.createElement('span')
        gcache.values = new Object
    }

    //logout('loadcache - ok loaded')

    return true

}

function showcache() {

    if (!gcache) return

    try {
        var values = gcache.values
    }
    catch (e) {
        return
    }

    //work out the total cache size (values only)
    var cachesize = 0
    var cachen = 0
    var html = ''
    for (var key in values) {
        if (key.slice(0, 6) != 'exodus') {
            html += '<tr><td>' + key + '</td><td>' + values[key] + '</td></tr>'
        }
    }
    if (html) {
        var win = window.open()
        win.document.body.innerHTML = '<table>' + html + '</table>'
    }

}

//prune cache every x seconds
var gcachepruneintervalsecs = 100
var gmaxcachesize = 1024000
var gmaxcachen = 100
exodussetinterval('trimcache()', gcachepruneintervalsecs * 1000)
function trimcache() {

    if (!gcache) return

    try {
        var values = gcache.values
    }
    catch (e) {
        return
    }

    //work out the total cache size (values only)
    var cachesize = 0
    var cachen = 0
    for (var key in values) {
        if (key.slice(0, 6) != 'exodus') {
            cachesize += values[key].length
            cachen++
        }
    }

    var result = [cachesize, cachen]
    if (gusername == 'EXODUS') wstatus('trimcache() cachesize:' + cachesize + ' cacheitems:' + cachen)

    //delete initial keys until the size is below the desired limit
    //hopefully the initial keys are the oldest
    for (var key in values) {
        if (cachesize <= gmaxcachesize && cachen <= gmaxcachen) break
        if (key.slice(0, 6) != 'exodus') {
            cachesize -= values[key].length
            cachen--
            delete values[key]
        }
    }

    return result

}

function prunecache(request) {

    //prunes all entries starting with request and returns the number of entries pruned

    //also prunes in window.opener and window.parent (and up chain if any)
    if (window.opener) {
        try {
            window.opener.prunecache(request)
        }
        catch (e) { }
    }
    else if (window.parent && window.parent != window) {
        try {
            window.parent.prunecache(request)
        }
        catch (e) { }
    }

    if (!gcache) return

    try {
        var values = gcache.values
    }
    catch (e) {
        return
    }

    var npruned = 0
    var prunekey = cachekey(request)

    for (var key in values) {
        if (key.slice(0, prunekey.length) == prunekey) {
            delete values[key]
            npruned += 1
        }
    }

    return npruned

}

function readcache(request) {

    //login('readcache')

    if (!(loadcache())) {
        //logout('readcache - loadcache failed')
        return null
    }

    var key = cachekey(request)
    var result
    if (gcache.values) {
        result = gcache.values[key]

        //delete and restore to implement simple LRU cache
        //delete gcache.values[key]
        //gcache.values[key]=result

    }
    else {
        result = gcache.getAttribute(key)

        //delete and restore to implement simple LRU cache
        //gcache.removeAttribute(key)
        //gcache.addAttribute(key,result)

    }

    //logout('readcache ok')

    return result

}

function cachekey(request) {

    //login('cachekey')

    //return escape(this.dataset+'\r'+this.username+'\r'+request).replace(/%/g,'_')
    // var temp=escape(gdataset+'_'+request).replace(/%/g,'_').replace(/@/g,'_AT_').replace(/\//g,'_SLASH_').replace(/\*/g,'_STAR_')
    var temp = escape(request).replace(/%/g, '_').replace(/@/g, '_AT_').replace(/\+/g, '_PLUS_').replace(/\//g, '_SLASH_').replace(/\*/g, '_STAR_')

    //alert(temp)

    //logout('cachekey ' + temp)

    return temp

}

function writecacherecord(data, filename, key) {
    //if data is an array then assume data[0] is the key and remove it
    //default key from data[0]
    if (typeof data == 'object') {
        if (!key) key = data[0]
        data = data.slice(1).join(fm)
    }
    writecache('READ\r' + filename + '\r' + key, data)
}

function deletecacherecord(filename, key) {
    deletecache('READ\r' + filename + '\r' + key)
}

function writecache(request, data) {
    //login('writecache')

    if (!(loadcache())) {
        //logout('writecache loadcache failed')
        return false
    }

    if (gcache.values)
        gcache.values[cachekey(request)] = data
    else
        gcache.setAttribute(cachekey(request), data)

    gcache.cacheupdated = true

    //logout('writecache')

    return true

}

function deletecache(request) {
    //login('deletecache')

    if (!(loadcache())) {
        //logout('deletecache loadcache failed')
        return false
    }

    if (gcache.values) {
        var cacheitem = gcache.values[cachekey(request)]
        if (typeof cacheitem != 'undefined')
            delete gcache.values[cachekey(request)]
    } else
        gcache.removeAttribute(cachekey(request))

    gcache.cacheupdated = true

    //logout('deletecache')

    return true

}

function clearcache() {

    //login('clearcache')

    //force refresh security table
    gtasks = null

    if (!(loadcache())) {
        //logout('clearcache loadcache failed')
        return false
    }

    try {
        //will fail on mac
        if (typeof gcache.load != 'undefined')
            gcache.load('emptycache')

        gcache.values = new Object

        if (typeof gcache.save != 'undefined')
            gcache.save('\exoduscache')
    }
    catch (e) {
        //logout('clearcache fail ' + e.description)
        return false
    }

    gcache.cacheupdated = false
    //logout('clearcache ok')

    return true

}

function* sorttable(event, order) {

    event = getevent(event)

    var colid = event.target.sorttableelementid

    if (typeof form_presort == 'function') {
        //yield* exodusevaluate('yield* form_presort()','yield* formfunctions_onload()')
        if (!(yield* form_presort(colid)))
            return yield* exodusinvalid()
    }

    //determine the groupno
    var dictitem = gds.dictitem(colid)
    if (!dictitem) return yield* exodusinvalid()
    var groupno = dictitem.groupno
    if (!groupno)
        return yield* exodusinvalid(colid + ' is not multivalued for sorting')

    window.status = 'Sorting, please wait ...'

    //get the data rows
    if (!groupno) groupno = 1
    var datarows = gds.data['group' + groupno]
    var fromrown = 0
    var uptorown = datarows.length - 1

    var up2down = false
    try {

        var clickedelement = event.target || event.srcElement

        //var clickedelement = document.getElementsByName('sortbutton_' + groupno)[0]

        //check if reverting from reverse to normal
        up2down = clickedelement.src.indexOf('up') >= 0

        //decide the order image
        var order = (clickedelement.src.indexOf('down') >= 0) ? 'up' : 'down'
    }
    catch (e) {
        if (typeof order == 'unassigned')
            order = 'down'
    }

    //get the data
    var data = []
    var dictitemlength
    if (dictitem.align == 'R') {
        dictitemlength = dictitem.length
        if (!dictitemlength) dictitemlength = 20
    }
    var sortdatalevels = []
    //for (var rown=0;rown<datarows.length;rown++)
    rown1 = 0
    rown2 = uptorown
    rowinc = 1
    if (up2down) {
        rown1 = uptorown
        rown2 = 0
        rowinc = -1
    }
    for (var rown = rown1; rown >= 0 && rown < datarows.length; rown += rowinc) {
        var sortdata = datarows[rown][colid].text
        if (dictitemlength) {
            if (sortdata) {
                var temp = new exodusamount(sortdata)
                sortdata = temp.amount.split('.')
                if (!sortdata[1]) sortdata[1] = ''
                sortdata = temp.unit + exodusspace(dictitemlength - sortdata[0].length) + sortdata[0] + temp.amount
            }
        }
        else {
            //if indented then work out prefix from higher levels
            if (sortdata.match(/(^\s+)/)) {

                //refuse to sort in reverse if indented
                if (order == 'up') {
                    return yield* exodusinvalid('Cannot reverse sort when any data is indented')
                }

                var prefix = ''
                var indent = RegExp.$1.length
                for (var i = 0; i < indent; i++) {
                    if (sortdatalevels[i]) prefix += sortdatalevels[i]
                }
                //save the current level
                sortdatalevels = sortdatalevels.slice(0, indent)
                sortdatalevels[indent] = sortdata
                //add the prefix
                if (prefix) sortdata = prefix + sortdata
            }
            else {
                sortdatalevels = [sortdata]
            }

        }

        //right justify the rown number assume max 999999
        var temp = rown.toString()
        //temp = exodusspace(6 - temp.length) + temp
        temp = ('000000' + temp).slice(-6)

        //save the two column array for sorting
        //need the +temp so that indented fields sort correctly
        data[rown] = [sortdata + ' ' + temp, temp]

    }

    //get the table rows
    var tablex = $$('exodusgroup' + groupno)
    if (tablex)
        var tablerows = tablex.tBodies[0].rows

    //sort the data and table rows
    //////////////////////////////
    //NB .sort() will sort arrays of arrays using all elements of the subarray in turn
    //in our case we make subarray 2nd element
    // the original rown number (right justified) so .sort() is STABLE!!!
    //ie original row order is preserved for duplicate sorted items
    //this means that you can sort on one column and then another
    //and the first column sorted *remains sorted* within duplicate values of
    //of the second column sorted.
    //eg to sort by company AND name .. sort on name first then company
    data = data.sort()
    //QuickSort(data,fromrown,uptorown)

    //reverse if sorting descending
    if (order == 'up')
        data.reverse()

    //get an array of the tablerows
    var oldrows = []
    for (var oldrown = 0; oldrown < tablerows.length; oldrown++) oldrows[oldrown] = tablerows[oldrown]

    //change the sort image now confirmed
    try {
        yield* resetsortimages(groupno)
        clickedelement.src = gimagetheme + 'smallsort' + order + '.gif'
    } catch (e) { }

    //reorder data and table rows
    var newdatarows = []
    for (var newrown = 0; newrown < data.length; newrown++) {

        var oldrown = (+Math.floor(data[newrown][1]))

        //build new ordered data
        newdatarows[newrown] = datarows[oldrown]

        //rearrange table rows
        if (newrown != oldrown) tablerows[newrown].swapNode(oldrows[oldrown])

    }

    //replace ordered data
    gds.data['group' + groupno] = newdatarows

    if (typeof form_postsort == 'function') {
        //yield* exodusevaluate('yield* form_postsort()','yield* formfunctions_onload()')
        if (!(yield* form_postsort(colid)))
            return yield* exodusinvalid()
    }

    window.status = ''

}

//******
//*MENU*
//******

function menuhide(element) {

    var okmenus = false

    if (!element)
        element = $$('menudiv').parentNode

    //gmenucodes FINANCE,SUPPORT,MEDIAANALYSIS,MEDIA,JOBS,TIMESHEETS,HELP
    if (!element.menuok) {
        okmenus = exodusgetcookie2('m')
        //if (gusername!='EXODUS')
        //    okmenus=okmenus.replace('FINANCE,','')
        okmenus = okmenus.split(',')
    }

    if (!element.exodusmenuaccesskeys)
        element.exodusmenuaccesskeys = []

    var childnodes = element.childNodes

    nextchild:
    for (var i = element.childNodes.length - 1; i >= 0; i--) {
        var child = childnodes[i]

        //delete some menus
        if (okmenus && okmenus.join() != '' && child.id && child.id.slice(0, 5) == 'menu_') {
            if (!okmenus.exoduslocate(child.id.slice(5))) {
                //ignore failure due to too early key response
                try {
                    child = element.removeChild(child)
                    continue nextchild
                }
                catch (e) { }
            }
        }

        if (child.className == 'menu' || child.parentNode.className == 'menu') {
            if (child.tagName == 'DIV') {
                child.className = 'menu'
                child.style.display = 'none'
                menuhide(child)
            }
            if (child.tagName == 'A') {
                child.onmouseover = menuonmouseover
                //stops keys working properly
                //child.onclick = menuonmouseover
                child.onmouseout = menuonmouseout
                //do both ?!
                //addeventlistener(child,'mouseover','menuonmouseover')
                //addeventlistener(child,'mouseout','menuonmouseout')
                //child.onfocus=menuonmouseover
                //child.onblur=menuonmouseout
                child.className = 'menuitem'
                child.style.paddingLeft = '0px'

                if (typeof child.exodussubmenuoption == 'undefined') {

                    //save an array of elements by accesskey in the menu
                    var underlineelement
                    if ((underlineelement = child.getElementsByTagName('U')).length) {
                        var menuaccesskey = underlineelement[0].innerText.exodustrim().slice(0, 1).toUpperCase()
                        var temp = element.exodusmenuaccesskeys[menuaccesskey]
                        if (gusername == 'EXODUS' && temp)
                            //yield* exodusnote('Duplicate menu access key ' + menuaccesskey.exodusquote() + ' for\r' + child.innerText + '\rand\r' + temp.innerText)
                            alert('Duplicate menu access key ' + menuaccesskey.exodusquote() + ' for \r' + child.innerText + ' \rand \r' + temp.innerText)
                        element.exodusmenuaccesskeys[menuaccesskey] = child
                    }
                }

                //add submenu indicators
                if (!child.href && !child.onclick && typeof child.exodussubmenuoption == 'undefined') {
                    child.exodussubmenuoption = true
                    //child.style.width=child.parentNode.runtimeStyle.width
                    //child.style.border='1px solid'
                    if (element.id != 'menudiv')
                        child.innerHTML += ' ...'

                }
                else {
                    child.exodussubmenuoption = false
                }

            }
        }
    }

    element.menuok = true

}

var gnmenus = 0
var gmenutimeout = ''

//menu_onclick=menuonclick
function* menu_onclick(event) {

    //disabled to get menu working better on mobile
    //onclick cannot call "onmouseover" event on mobile since
    // "mouseover" event is called after onclick if not focussed (strange but true)
    //and double calling would have the effect of opening and closingopen+close)
    //if (typeof Touch == 'undefined')
    //    menuonmouseover(event, 'click')

    //menudiv.style.display = menudiv.style.display == '' ? 'none' : ''
    return
}

function menuonmouseover(event, menuoption) {

    //also called from onkeydown for alt+M keyboard shortcut

    //quit if the menu is not loaded from the web site yet
    if (!gmenuloaded)
        return false

    //like esc if any menus are open
    if (menuoption == 'click') {
        menuoption = ''
        if (gnmenus) {
            gnmenus = 0
            menuclose()
            return exoduscancelevent(event)
        }
    }

    //indicate menuing and cancel any request to close menus
    var keyboarding = menuoption
    gnmenus = 1
    window.clearTimeout(gmenutimeout)

    //get the menuoption
    if (!menuoption) {
        menuoption = event.target
        if (!menuoption || typeof menuoption != 'object')
            return
    }
    //click detected on childnode
    //if (menuoption.id == 'menubutton_label')
    if (menuoption.id == 'menubutton_label' || menuoption.tagName == 'IMG' || menuoption.tagName == 'U')
        menuoption = menuoption.parentNode

    var menu = menuoption.parentNode

    //window.status=new Date()+' '+menu.clientLeft+' '+menu.clientWidth
    var leftoffset
    var topoffset
    if (menuoption.id && (menuoption.id == 'menubutton' || menuoption.tagName == 'BUTTON' || menuoption.id.slice(0, 5) == 'menu_')) {
        //    console.log("menu_onmouseover 5a " + menuoption)

        //  leftoffset=menuoption.offsetWidth+menuoption.offsetLeft-4
        //  topoffset=0
        //make the menu visible just beneath the menu button
        leftoffset = menuoption.offsetLeft + 1
        topoffset = menuoption.offsetHeight + menuoption.offsetTop - 1
    }
    else {
        //    console.log("menu_onmouseover 5b " + menuoption)

        //make the menu visible just to the right of the menu option
        leftoffset = menu.offsetWidth - menu.clientLeft - 8
        //topoffset = -menu.clientTop
        //topoffset = 0
        topoffset = menuoption.offsetTop + 5
    }

    menuchangeoption(menu, menuoption)

    //menu.focus()
    //menu.onkeydown=menuonkeydown

    //looks for a following menudiv to open
    //otherwise returns
    var submenu
    submenu = menuoption
    while (submenu = submenu.nextSibling) {
        if (submenu.tagName)
            break
    }
    if (!submenu || submenu.className != 'menu') {
        if (!submenu)
            submenu = event.target
        if (!submenu)
            return
    }

    //close any other submenus
    menuhide(menu)

    //open the submenu
    // submenu.style.left=window.event.srcElement.clientLeft+window.event.srcElement.clientWidth
    submenu.style.left = leftoffset + 'px'
    if (topoffset)
        submenu.style.top = topoffset + 'px'
    submenu.style.display = ''
    submenu.noWrap = true
    //alert(submenu.outerHTML)
    //highlight first menu item if none highlighted
    if (keyboarding || submenu.id == 'menudiv') {
        if (!submenu.highlightedelement) {
            //look for nextsibling
            var subelements = submenu.childNodes
            for (var subelementn = 0; subelementn < subelements.length; subelementn++) {
                if (subelements[subelementn].tagName == 'A') {
                    menuchangeoption(submenu, subelements[subelementn])
                    break
                }
            }
        }

        //simple submenu.focus doesnt work as usual ... blanks the menudiv!
        menufocus(submenu)
        submenu.onkeydown = menuonkeydown
        //do both ?!
        //addeventlistener(submenu,'keydown','menuonkeydown')

    }

}

//sets a timeout to focus on an element
//OR executes the timeout function
var gmenuelement;
function menufocus(menu) {

    //if given an element save it global and set timeout callback
    if (menu) {
        gmenuelement = menu
        exodussettimeout('menufocus()', 1)
        return
    }

    //focus on the saved element (so that keyboard events get send to menu elements)
    //-1 to allow scripting focus without forming part of normal keyboard tab sequence
    gmenuelement.tabIndex = -1
    try { gmenuelement.focus() } catch (e) { }

}

function menuonmouseout(event) {
    event = getevent(event)

    gnmenus = 0
    gmenutimeout = exodussettimeout('menuclose()', 1000)
    //window.event.srcElement.style.color='black'
}

function menuclose() {

    if (gnmenus == 0) {
        menuhide()
        var xmenubutton = $$('menubutton')
        xmenubutton.style.backgroundColor = ''
        xmenubutton.style.color = ''
        xmenubutton.tabIndex = 9999
    }

}

function menuonkeydown(event, menu, key) {

    event = getevent(event)

    //usually called as an event with no parameters
    //doesnt seem to work well except called as an event
    if (!menu || !key) {
        menu = event.target
        key = event.keyCode
    }
    //console.log('menuonkeydown '+key+' '+menu+' '+key)

    var menuoption = menu.highlightedelement
    if (!menuoption)
        return

    var horizontal = (menu.id == 'menudiv')

    //35 End 36 Home 37 Left 38 Up 39 Right 40 Down
    //65-90 A-Z
    var alphakey = (key >= 65 && key <= 90)
    var endkey = key == 35
    var homekey = key == 36
    var leftkey = key == 37
    var upkey = key == 38
    var rightkey = key == 39
    var downkey = key == 40
    var esckey = key == 27
    var enterkey = key == 13

    //wstatus(new Date()+' '+key)

    //esc ... close menu and return focus to where it was when menu was opened
    if (esckey || (!horizontal && leftkey)) {
        if (menu.parentNode.parentNode.highlightedelement) {
            menuonmouseover(event, menu.parentNode.parentNode.highlightedelement)
            //exodussettimeout('menuonkeydown(13)',10)
            //attempt to press Enter on it
            //menuonkeydown(menu.parentNode.parentNode,13)
            return exoduscancelevent(event)
        }
        else {
            gnmenus = 0
            menuclose()
            return exoduscancelevent(event)
        }
    }

    //menu access key?
    if (alphakey && menu.exodusmenuaccesskeys) {
        var newmenuoption = menu.exodusmenuaccesskeys[String.fromCharCode(key)]
        if (newmenuoption) {
            menuoption = newmenuoption
            //will be done below menuchangeoption(menu,menuoption)
            enterkey = true
            alphakey = false
        }
    }

    //enter key opens a menu or click a link (shift+enter to open a new window)
    if (enterkey || (horizontal && downkey) || (!horizontal && rightkey)) {
        if (enterkey && menuoption.href) {
            try {
                if (menuoption.onclick)
                    menuoption.onclick()
                else {
                    if (event && event.shiftKey)
                        windowopen_sync(menuoption.href)
                    else
                        window.location.assign(menuoption.href)
                }
            }
            catch (e) { }
        }
        else if (enterkey && menuoption.onclick) {
            try {
                menuoption.click()
            }
            catch (e) { }
        }
        else {
            if (!menuoption.href && !menuoption.onclick)
                menuonmouseover(event, menuoption)
        }
        return exoduscancelevent(event)
    }

    //right/left/up/down arrows and home/end
    //or letter keys to search for that item
    if ((endkey || homekey) || (horizontal && (leftkey || rightkey)) || (!horizontal && (upkey || downkey)) || alphakey) {

        var newmenuoption

        //search forward from current element to the end
        //(or backwards to beginning if left/up key)
        if ((key >= 37 && key <= 40) || alphakey) {
            newmenuoption = menuoption
            while (newmenuoption && (newmenuoption = ((leftkey || upkey) ? newmenuoption.previousSibling : newmenuoption.nextSibling))) {
                if (newmenuoption.tagName == 'A') {

                    //if cursor key then break at first available menu option
                    if (!alphakey)
                        break

                    //or break if the first letter matches the alpha code entered
                    if (newmenuoption.innerText.exodustrim().slice(0, 1).toUpperCase() == String.fromCharCode(key))
                        break

                }
            }
        }

        //resume search from the beginning (or end if left/up key)
        //search from the beginning if pressed Home
        //search from the end if pressed End
        if (!newmenuoption) {
            newmenuoption = ((endkey || leftkey || upkey) ? menu.lastChild : menu.firstChild)
            while (newmenuoption) {
                if (newmenuoption.tagName == 'A') {

                    //if cursor key then break at first available menu option
                    if (!alphakey)
                        break

                    //ir break if the first letter matches matches the alpha code entered
                    if (newmenuoption.innerText.exodustrim().slice(0, 1).toUpperCase() == String.fromCharCode(key))
                        break

                }
                newmenuoption = ((endkey || leftkey || upkey) ? newmenuoption.previousSibling : newmenuoption.nextSibling)
            }
        }
    }

    //if found a new option then move highlight to it
    if (newmenuoption && newmenuoption.tagName == 'A' && newmenuoption != menuoption) {

        menuchangeoption(menu, newmenuoption)

        return exoduscancelevent(event)

    }

    return true

}

function menuchangeoption(menu, newmenuoption) {

    //skip if over underlined element (bubble will still highlight the parent)
    if (newmenuoption.tagName == 'U') return

    //clear old menu item to default colors
    var menuoption = menu.highlightedelement
    if (menuoption) {
        menuoption.style.backgroundColor = ''
        menuoption.style.color = ''
    }

    if (!newmenuoption) return

    //highlight new menu item
    //newmenuoption.style.backgroundColor='#d0d0d0'

    newmenuoption.style.backgroundColor = 'highlight'
    newmenuoption.style.color = 'highlighttext'

    //record new highlighted item in the menu
    menu.highlightedelement = newmenuoption

    //focus and onkeydown should remain on the menu div not the item
    if (!true) {
        //menu.focus()
        gmenuelement = menu
        exodussettimeout('menufocus()', 10)
    }
    menu.onkeydown = menuonkeydown
    //menu.addEventListener('keydown',menuonkeydown)

}

function menubuttonhtml(id, imagesrc, name, title, accesskey, align) {

    var tagname = 'span'
    //tagname='div'
    var tx = '<' + tagname

    if (id)
        tx += ' id="' + id + 'button' + '"'

    tx += ' class=graphicbutton'
    tx += ' onmousedown=this.style.borderStyle="inset"'
    tx += ' onmouseup=this.style.borderStyle="outset"'
    tx += ' onmouseout=this.style.borderStyle="outset"'

    if (title)
        tx += ' title="' + title + '"'

    //there is no float:center?!
    var style = 'white-space:nowrap'
    if (align == 'center')
        //tx+=' style="float:'+align+'"'
        //tx+=' style="margin:0 auto"'
        //switch off the default from graphicbutton class?
        style += ';float:none;clear:both;overflow:hidden'
    else if (align)
        style += ''//';float:' + align
    //style=''
    if (style)
        tx += ' style="' + style + '"'

    if (accesskey)
        tx += ' accesskey="' + accesskey + '"'

    //tx += ' exodusonclick=' + id + '_onclick(event)"'
    tx += ' exodusonclick="'
    /* yield */ tx += 'yield * '
    tx += id + '_onclick(event)"'

    tx += '>'

    //alert(id+' '+tx)
    //image inside span button (any styling should also be edited into menu.htm menu <img>
    if (imagesrc) {
        //style="vertical-align:text-top"
        tx += '<IMG src=' + imagesrc + ' style="vertical-align:top"'
        tx += '>'
    }

    //tx+='<br />'

    tx += '<span id=' + id + 'button' + '_label>'
    tx += ' ' + name
    tx += '</span>'

    tx += '</' + tagname + '>'

    //create a tiny button to capture the accesskey to prevent it from being used by the browser
    //firefox access key is shift+alt unless reconfigured in about:config
    //http://kb.mozillazine.org/Ui.key.contentAccess
    //so ALSO implemented in onkeydown
    if (accesskey) {
        tx += '<button xtabindex=-1 style="background-color:white; height:1px; width:1px; border-style:none; margin:0px ;padding:0px"'
        tx += ' accesskey="' + accesskey + '"'
        /* yield */ tx += 'yield * '
        tx += ' exodusonclick="' + id + '_onclick(event)"'
        tx += '></button>'
    }

    return tx

}

function setgraphicbutton(button, labeltext, src) {

    if (labeltext) {
        var label = $$(button.id + '_label')
        label.innerHTML = labeltext
    }
    if (src)
        button.getElementsByTagName('IMG')[0].src = src
}

function* refreshcache_onclick() {
    if (clearcache())
        yield* exodusnote('All EXODUS data cached in this window has been cleared\rand will be retrieved from the server again as and when required.')
    // \r\rN.B. EXODUS forms and scripts will remain cached and may\rbe updated when you close and reopen all browser\rwindows - depending on the cache settings in your browser.')
    else
        yield* exodusnote('Cannot clear cache.')
    return true
}

function exoduscancelevent(event) {

    //should error maybe
    event = getevent(event)
    if (!event) {
        //console.log('exoduscancelevent QUITTING - no event!')
        return false
    }

    event.cancelBubble = true
    event.returnValue = false

    if (event.stopPropagation) {
        //console.log('event.stopPropagation()')
        event.stopPropagation()
    }

    if (event.preventDefault) {
        //console.log('event.preventDefault()')
        event.preventDefault()
    }

    event.cancel = true

    return false

}

//not really a great idea to use global when async functions could be running
//used in getrecn and setreadonly at the moment
var gevent

function getevent(event) {

    //if not passed event then try MSIE's window.event
    if (!event) {
        if (window.event)
            event = window.event
        else
            event = gevent
    }

    //if not given, and cannot get event then return empty object (with no target property)
    //perhaps should throw an error
    if (!event)
        return {}

    //if no target then try MSIE's window.srcElement
    if (!event.target && event.srcElement)
        event.target = event.srcElement

    //if cannot find target then return empty object with target property of
    //so if passed an element then element.target is itself. usually the caller is looking for .target
    if (!event.target) {
        var origevent = event
        event = {}
        event.target = origevent
    }

    //defeat Safari 3 bug
    if (event.target && event.target.nodeType == 3)
        event.target = event.target.parentNode;

    //return event (with .target)
    gevent = event
    return event

}

//allows dom scan without using IE document.all(ii)
function nextelement(element) {

    //return first child
    if (element.childNodes.length)
        return element.firstChild

    //otherwise return next sibling
    if (element.nextSibling)
        return element.nextSibling

    //otherwise return next aunt
    while (true) {
        element = element.parentNode
        if (!element.tagName)
            return false
        if (element.nextSibling)
            return element.nextSibling
    }

}

function id2classname(element) {

    //scan all elements adding id to classname as exodus_idname
    if (!element)
        element = document.body
    while (element = nextelement(element)) {
        if (element.id) {
            if (element.className) {
                if (element.className.indexOf('exodusid_') < 0) {
                    element.className += ' exodusid_' + element.id
                }
            }
            else
                element.className = 'exodusid_' + element.id
        }
    }

}

function getmaxwindow_sync() {

    //maximum size is the size of the parent window
    var parentwindow = window.opener
    //popups within popups are limited by the original window size not the immediate parent
    while (parentwindow && parentwindow.opener)
        parentwindow = parentwindow.opener
    //if .opener not supported
    if (!parentwindow) {
        parentwindow = window.parent
        while (parentwindow && parentwindow.parent && parentwindow != parentwindow.parent)
            parentwindow = parentwindow.parent
    }
    if (!parentwindow)
        systemerror('cannot find window.opener or window.parent', 'getmaxwindow_sync')

    var max = {}
    try {
        //internet explorer gives permission denied when uploading files
        max.width = parentwindow.outerWidth
        max.height = parentwindow.outerHeight
        //max.height=parentwindow.innerHeight
        max.left = parentwindow.screenLeft || parentwindow.screenX
        max.top = parentwindow.screenTop || parentwindow.screenY
        //max.top=max.top+(parentwindow.outerHeight-parentwindow.innerHeight)
        //alert('getmaxwindow_sync() max='+max.width+' '+max.height+' '+max.left+' '+max.top)
    } catch (e) {
    }
    if (!max.width) {
        max.width = window.screen.availWidth
        max.height = window.screen.availHeight
        max.left = 0
        max.top = 0
    }
    max.style = ';DialogHeight:' + max.height + 'px; DialogWidth:' + max.width + 'px;,'
    max.style += ',width=' + max.width
    max.style += ',height=' + max.height
    max.style += ',left=' + max.left//window.screenX
    max.style += ',top=' + max.top//window.screenY

    return max
}


var gblockevents//stops onclick event at the same time as onfocus event
function form_blockevents(truefalse, callinfo) {

    var callername = ''
    if (form_blockevents.caller) {
        callername = form_blockevents.caller.name
    }

    if (!gblockevents)
        gblockevents = 0

    if (truefalse) {
        ++gblockevents
        logevent('-------->' + gblockevents + '         block ' + callername + ' ' + callinfo)

    } else {
        --gblockevents
        logevent('         ' + gblockevents + '<--------unblock ' + callername + ' ' + callinfo)
    }

    if (gblockevents < 0)
        gblockevents = 0

}

function starteventhandler(eventfunctionname, functionx) {

    //event handler function
    //2. creates a generator (suspendable/resumable function)
    //   of the right type for the event
    //   (knows event generator function from closure)
    //   and passes the event into it
    //3. calls the generator function to start executing
    //4. the generator function will either return (with event return value?)
    //   or suspend and wait for some window or xmlhttp to resume it
    //5. any window/xmlhttp that wishes the function to resume
    //   can call geventhandler .next(data) where data is the value to be used
    //   as the expression to the right of the yield statement
    return function exodus_anon_sync_event_handler(event) {

        event = getevent(event)
        //YIELD//console.log('\n--- Event '+event.type+' ---')

        var eventdescription = event.type + ' ' + event.target.tagName + ' id:' + event.target.id + ' gblockevents:' + gblockevents

        //prevent events like onfocus being followed by onclick while
        //onfocus is yielding to async events like window.open etc
        //var uiblockerdiv=$$('uiblockerdiv')
        //if (gblockevents||uiblockerdiv) {
        if (gblockevents) {

            if (event.type == 'unload' || event.type == 'beforeunload') {
                //always call unloadevents
                //TODO create a new gcurrentevent?

                //special treatment of events while exodusconfirm is up
            } else if ($$('exodusconfirmdiv')) {

                //allow mouse right click, copy of error message text etc.
                if (event.type == 'copy')
                    return true

                if (event.type == 'keydown') {

                    //allow keyboard Ctrl+c to copy error message text etc.
                    if (event.ctrlKey && event.which == 67)
                        return true

                    var keycode = event.keyCode ? event.keyCode : event.which
                    var keyletter = String.fromCharCode(keycode).toUpperCase()

                    logevent('exodus_anon_sync_event_handler+exodusconfirmdiv ' + event.target.id + ' ctrlKey:' + event.ctrlKey + ' key:' + keycode + ' letter:' + keyletter)

                    //FIX detection of specific keys on buttons like Y N etc

                    //POSITIVE = F9 or Ctrl+Enter or SPACE some initial
                    if (keycode == 120 || (keycode == 13) || (keycode == 32) || keyletter == gexodusconfirmletters[1]) {
                        window.setTimeout('exodus_confirm_function1()', 1)
                        return exoduscancelevent(event)
                    }

                    //CANCEL = Esc or some initial
                    else if (keycode == 27 || keyletter == gexodusconfirmletters[3]) {
                        window.setTimeout('exodus_confirm_function3()', 1)
                        return exoduscancelevent(event)
                    }

                    //NEGATIVE = F8 or some initial
                    else if (keycode == 119 || keyletter == gexodusconfirmletters[2]) {
                        window.setTimeout('exodus_confirm_function2()', 1)
                        return exoduscancelevent(event)
                    }
                }
            }

            logevent('!!!SKIPPING event!!! ' + eventdescription + ' because gblockevents is set, and not keydown related to exodusconfirmdiv')

            return exoduscancelevent(event)

        }//end of event blocking

        //events are not blocked - create a new event handler

        //make the global generator function (that can yield) and can be resumed by calling .next()
        var eventhandler = functionx(event)
        if (!eventhandler) {
            var msg1 = 'exodus_anon_sync_event_handler ' + eventfunctionname + ' ' + event.target
            var msg2 = 'cant create functionx ' + functionx.name
            logevent(msg1 + ' : ' + msg2)
            //systemerror(msg1,msg2)

        } else {

            //start the event, result will be that provided by the FIRST yield
            //we cant run the event to completion if there is more than one yield
            //var next=exodus_next('exodus_anon_sync_event_handler for '+eventdescription)
            var next = exodusneweventhandler(eventhandler, eventdescription + ' in exodus_anon_sync_event_handler')
            var result = next.value

            //onbeforeunload may return text immediately so return that
            if (result)
                event.returnValue = result//what exactly does this do?
            else
                result = false

            //            logevent('<<<<<<<<<< '+eventdescription + ' NEW EVENT HANDLER in exodus_anon_sync_event_handler')
            //            logevent('           done:'+next.done+ ' result:'+result)

            return result

        }
    }
}

var geventhandler
var geventn = 0

function exodusneweventhandler(eventhandler, location) {

    ++geventn
    logevent(' ')
    logevent('=== NEW EVENT HANDLER ' + geventn + ' for ' + location + '===')
    geventhandler = eventhandler

    //run the generator function to first yield or completion if no yielding
    var next = exodus_next('', 'exodusneweventhandler from ' + location)

    //code will continue immediately here after completion or yielding of the function
    //IF the function yielded to window.open for example
    //temp.value will be 1 and temp.done will be false
    return next
}

function addeventlistener(element, eventname, functionx) {

    if (!element)
        systemerror('addeventlistener ' + eventname + ' ' + functionx, 'element is missing')

    //logevent('addeventlistener:'+(element.id||element.name||element.tagName||window)+','+eventname+'->'+functionx)

    if (typeof functionx == 'string') {
        //element.setAttribute('on'+eventname,functionx+'(event)')
        //return
        functionxname = functionx
        functionx = window[functionx]
        if (!functionx)
            systemerror('error: in addeventlistener ' + functionxname + ' function does not exist')
        if (guseyield)
            functionx = starteventhandler(eventname, functionx)
    }

    //normally using bubbling style but focus event doesnt bubble so we use capture style for that
    var capture = eventname == 'focus'

    if (element.addEventListener)
        element.addEventListener(eventname, functionx, capture)//FF/Safari/IE9+
    else if (element.attachEvent)//pre IE9
        //if cant attachEvent then possibly it isnt a DOM element
        element.attachEvent('on' + eventname, functionx)
    //else
    //run function in the environment of the element
    //element.attachEvent('on'+eventname,function(){functionx.call(element)})

    return true
}

/*
function exodusgetattribute(element,attributename) {

var attribute=element.getAttribute(attributename)
if (!attribute)
return attribute
if (typeof attribute=='string') {

if (attribute=='true')
return true
else if (attribute=='false')
return false
return attribute
}
return attribute
}
*/

//used anywhere yet?
function SetAttribute(element, attName, attValue) {

    if (attValue == null || attValue.length == 0)
        element.removeAttribute(attName, 0);			// 0 : Case Insensitive
    else
        element.setAttribute(attName, attValue, 0);	// 0 : Case Insensitive
}
function GetAttribute(element, attName, valueIfNull) {

    var oAtt = element.attributes[attName];

    if (oAtt == null || !oAtt.specified)
        return valueIfNull ? valueIfNull : '';

    var oValue = element.getAttribute(attName, 2);

    if (oValue == null)
        oValue = oAtt.nodeValue;

    return (oValue == null ? valueIfNull : oValue);
}

function exodusremoveelementsbyid(id) {
    while (true) {
        var tt = document.getElementById(id)
        if (!tt)
            break
        tt.parentNode.removeChild(tt)
    }
}

function exodusremovenode(element) {
    element.parentNode.removeChild(element)
}

function HTMLEncode(text) {
    if (!text)
        return ''
    //&amp first
    return text.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
}

function HTMLDecode(text) {
    if (!text)
        return ''
    //&amp; last
    return text.replace(/&gt;/g, '>').replace(/&lt;/g, '<').replace(/&amp;/g, '&')
}

function exodusint2date(exodusdate) {
    //problem unless automatic daylight saving time change is switched off
    //both the following return the same date!!!
    //zzz need to find a more reliable algorithm
    //alert(new Date(1967,11,31+14361))
    //alert(new Date(1967,11,31+14362))
    //return new Date(1967,11,31+(+exodusdate))

    //var date=new Date(0)
    //date.setUTCFullYear(1967)
    //date.setUTCMonth(11)
    //date.setUTCDate(31+(+exodusdate))
    //return date

    return new Date(Date.UTC(1967, 11, 31 + parseInt(exodusdate, 10)))

}

//thin wrapper to handle timeouts with/without yielding code
//command should be in text format so that functions will have yield * prefixed by converter to yielding code
function exodussettimeout(command, milliseconds) {
    if (glogsettimeout)
        console.log('exodussetimeout(' + command + ')')
    //if (typeof command=='string' && command.slice(0,7)=='yield * ') {
    //    command=command.slice(7).replace(/"/g,"'")
    if (typeof command == 'string' && command.match(gyieldregex)) {
        command = command.replace(gyieldregex, '').replace(/"/g, "'")
        return window.setTimeout('exodustimeout_sync("' + command + '")', milliseconds)
    } else
        //eval directly if not prefixed by "yield * '
        return window.setTimeout(command, milliseconds)
}

//sync function called on timeout in yielding code
//spins off an async/yielding function
function exodustimeout_sync(command) {

    logevent('exodustimeout_sync:geventhandler.done:' + geventhandler.done)

    //if another event handler is already running then defer execution for 100ms
    if (gblockevents) {
        window.setTimeout('exodustimeout_sync("' + command + '")', 100)
        return
    }

    //the async function should run to completion
    // even if it pauses for multiple async operations on the way.
    //command MUST be prefixed with "yield *" and return a generator
    var generator = eval(command);
    exodusneweventhandler(generator, 'exodustimeout_sync() ' + command)//yielding code
    //not interested in result
}

//thin wrapper to handle timeouts with/without yielding code
function exodussetinterval(command, milliseconds) {
    //if (typeof command=='string' && command.slice(0,7)=='yield * ') {
    //    command=command.slice(7).replace(/"/g,"'")
    if (typeof command == 'string' && command.match(gyieldregex)) {
        command = command.replace(gyieldregex, '').replace(/"/g, "'")
        return window.setInterval('exodusinterval_sync("' + command + '")', milliseconds)
    } else
        return window.setInterval(command, milliseconds)
}

//sync function called at intervals in yielding code
//spins off an async/yielding function
function exodusinterval_sync(command) {

    //if another event handler is already running then defer execution for 100ms
    if (gblockevents) {
        return
    }

    //the async function should run to completion
    // even if it pauses for multiple async operations on the way.
    //We create a new generator object each interval
    //We do not create one generator and call its .next() each interval
    //command MUST be prefixed with "yield *" and return a generator
    var generator = eval(command);
    exodusneweventhandler(generator, 'exodusinterval_sync() ' + command)//yielding code
    //not interested in result
}

function systemerror(functionname, e) {
    if (typeof functionname == 'undefined')
        functionname = 'undefined'
    //if error generated by user cancelling a server request then try to ignore the error
    if (functionname == 'Cancelled')
        return
    if (typeof e == 'undefined')
        e = ''
    var msg = e.toString()
    //if (e.name)
    //    msg+='\n'+e.name
    //if (e.message)
    //    msg+='\nError '+e.message
    if (e.fileName)
        msg += '\nFilename ' + e.fileName
    if (!e.stack && e.lineNumber)
        msg += ' Line No. ' + e.lineNumber
    if (!e.stack && e.columnNumber)
        msg += ' Column No. ' + e.columnNumber
    if (e.stack)
        msg += '\n' + e.stack
    else {
        var stack = new Error().stack
        if (stack)
            msg += '\n\n' + stack.toString()//.exodusconvert('\n\r','||')
        else {
            var caller
            if (arguments && arguments.callee)
                caller = arguments.callee
            else if (systemerror && systemerror.caller)
                caller = systemerror.caller
            if (caller)
                msg += '\n'
            while (caller) {
                msg += '\n' + caller.toString().split('\n')[0]
                caller = caller.caller
            }
        }
    }
    if (!gonunload)
        alert('System Error in ' + functionname + '\n' + msg)
    //if (gstepping||(!ginitok&&gusername=='EXODUS')) crashhere2
    if (gstepping || (gusername == 'EXODUS') || (gdataset && gdataset.slice(-4) == 'TEST'))
        exodus_javascript_system_error
}

//works like msie
function $all(elementid, element) {
    if (!element) {
        var id = elementid.id ? elementid.id : elementid
        /*
        var result
        if (document.querySelectorAll) {
        var result = document.querySelectorAll('.exodusid_' + id)
        if (!result.length)
        result = false
        }
        if (!result)
        */
        //return document.all(id) //this returns only one element in ie8 unless in compatibility mode
        return document.all[id]

        /* doesnt solve difference in document.all in ie8 between normal and compatibility mode
        var id = elementid.id ? elementid.id : elementid
        var temp = window[id]
        if (temp)
        return temp
        else
        return document.all(elementid)

        */
    }
    else
        //should this be changed to [elementid] similar to above
        // or is it supposed to return element it only 1 and array if more than one
        return element.all(elementid)
}

function $class(elementid, element) {
    var temp
    if (element)
        temp = element.getElementsByClassName('exodusid_' + elementid)
    else
        temp = document.getElementsByClassName('exodusid_' + elementid)
    //return the element if one(
    if (temp.length == 1)
        return temp[0]
    //return an array if many
    else if (temp.length > 1)
        return temp

    //last ditch attempt to find by id
    //NB but this doesnt respect the element argument
    temp = document.getElementById(elementid)
    if (temp)
        return temp

    //return undefined if not found
    return
}

function setdisabledandhidden(element, truefalse) {
    if (!element)
        return
    if (truefalse) {
        element.disabled = true//this seems to have the effect of setting attribute disabled to "" in modern browsers!
        element.setAttribute('disabled', 'disabled')
        element.style.display = 'none'
    }
    else {
        element.disabled = false
        if (element.removeAttribute)
            element.removeAttribute('disabled')
        element.style.display = ''
    }
}


//cross browser
function* getcurrentstyle(element) {
    if (window.getComputedStyle)
        return document.defaultView.getComputedStyle(element, null)
    if (element.currentStyle)
        return element.currentStyle
}

//firefox innertext doesnt work like internet explorer and webkit (chrome/safari)
//need BR to show as \n
function exodus_getinnertext(element) {
    if (element.tagName == 'BR')
        return '\n'
    if (typeof element.nodeValue == 'string')
        return element.nodeValue
    var nodes = element.childNodes
    if (!nodes)
        return ''
    var text = ''
    for (var noden = 0; noden < nodes.length; ++noden) {
        //for (var noden=0;noden<nodes.length;++noden) {
        //alert(nodes[node].tagName+' '+nodes[node].textContent)
        text += exodus_getinnertext(nodes[noden])//recursive
        //alert(text)
    }
    return text
}

function* exodusconfirm2(questionx, defaultbuttonn, positivebuttonx, negativebuttonx, cancelbuttonx, text, texthidden, imagesrc) {

    //performs "in-window" questions, selections and inputs
    //replaces (or called by)
    //exodusconfirm: questions (yes/no/cancel) and one line inputs
    //exodusdecide/exodusdecide2: selections

    var decide_args//holds popup list args if any

    if (typeof questionx == 'object') {

        //[question, data, cols, returncoln, defaultreply, many, inverted]
        decide_args = questionx
        questionx = decide_args[0]
        var decide_returnmany = decide_args[5]

        //question
        if (!questionx)
            questionx = 'Which do you want?'
        questionx = questionx.replace(/[\|\r\n]/g, '<br />')
        //if (decide_returnmany)
        //    questionx += '&nbsp;&nbsp;<span style="font-size:66%">(A=All)</span>'

    } else
        var decide_args = undefined

    var nbuttons = 0
    var buttons = []

    gexodusconfirmletters = []

    //check buttons
    if (positivebuttonx)
        nbuttons++
    if (negativebuttonx)
        nbuttons++
    if (cancelbuttonx)
        nbuttons++
    if (nbuttons == 0) {
        positivebuttonx = '<u>Y</u>es'
        negativebuttonx = '<u>N</u>o'
        cancelbuttonx = '<u>C</u>ancel'
        nbuttons = 3
    }

    var istextinput = typeof text != 'undefined' && text != null

    //create a centralised div with the appropriate buttons or input box
    var div = document.createElement('div')
    div.id = 'exodusconfirmdiv'
    div.style.zIndex = 200
    div.style.position = 'fixed'
    div.style.textAlign = 'center'
    div.style.background = 'lightgrey'
    div.style.border = '1px solid grey'
    div.style.padding = '10px'
    div.style.boxShadow = '0px 0px 7px #666666'
    //div.style.fontSize=25//had no effect
    div.style.maxHeight = (window.innerHeight - 50) + 'px'
    div.style.maxWidth = window.innerWidth + 'px'
    //div.style.maxHeight=(window.outerHeight-50)+'px'
    //div.style.maxWidth=window.outerWidth+'px'
    div.style.overflow = 'auto'

    //image

    var html = ''
    if (!imagesrc && (questionx.indexOf('!') + 1 || questionx.toLowerCase().indexOf('are you sure ') + 1 || questionx.toLowerCase().indexOf('warning') + 1))
        imagesrc = 'warning'
    else if (!imagesrc) {
        if (nbuttons == 1)
            imagesrc = 'info'
        else
            imagesrc = 'question1'
    }
    if (imagesrc) {
        if (imagesrc == 'critical') {
            imagesrc = 'xpcritical.gif'
            div.style.backgroundColor = '#ffdddd'//reddish
        }
        if (imagesrc == 'warning') {
            imagesrc = 'xpwarning.gif'
            div.style.backgroundColor = '#ffff99'//yellowish
            //darker than usual messages to distinguish from the usual buff document background color
        }
        if (imagesrc == 'info') {
            imagesrc = 'xpinfo.gif'
            div.style.backgroundColor = '#ddffdd'//greenish
        }
        if (imagesrc == 'question1') {
            imagesrc = 'xpquestion.gif'
            div.style.backgroundColor = '#ddddff'//blueish
        }
        if (!(imagesrc.indexOf('/') + 1 + imagesrc.indexOf('\\') + 1)) {
            imagesrc = gimagetheme + imagesrc
            if (window.location.href.toString().indexOf('index.html') >= 0)
                imagesrc = imagesrc.slice(3)
        }
        html += '<img src="' + imagesrc + '" alt="" xstyle="display: none" height="32" width="32" />'
    }
    var imagehtml = html

    //question

    var html = ''
    questionx = questionx.replace(/\r\n/g, '\r')
    questionx = questionx.replace(/\r\r/g, '<p />')
    questionx = questionx.replace(/\r/g, '<br />')
    questionx = questionx.replace(/\n/g, '<br />')
    questionx = questionx.replace(FMre, '<br />')
    questionx = questionx.replace(VMre, '<br />')
    questionx = questionx.replace(SMre, '<br />')
    questionx = questionx.replace(TMre, '<br />')
    questionx = questionx.replace(/\|/g, '<br />')
    var questionhtml = questionx

    //buttons
    var html = ''
    if (!decide_args) {
        var nbuttons = 0
        if (positivebuttonx)
            yield* addbutton('positive', 1, positivebuttonx, 'F9')
        if (negativebuttonx)
            yield* addbutton('negative', 2, negativebuttonx, 'F8')
        if (cancelbuttonx)
            yield* addbutton('cancel', 3, cancelbuttonx, 'Esc')

        //make sure no button is default unless specified
        if (!defaultbuttonn) {
            if (istextinput) {
                exodussettimeout('focusontext()', 10)
            }
            else {
                //xxtry{gbuttons[0].blur()}catch(e){}
            }
        }
    }
    var buttonshtml = html
    html = ''

    function* addbutton(buttonid, buttonn, buttontext, buttonfunckey) {

        nbuttons++

        html += '<span id="' + buttonid + 'button"'

        html += ' tabindex="0"'
        html += ' class="graphicbutton"'

        //mouse events down/up/out/click
        html += ' onmousedown="this.style.borderStyle=\'inset\'"'
        html += ' onmouseup="this.style.borderStyle=\'outset\'"'
        html += ' onmouseout="this.style.borderStyle=\'outset\'"'
        html += ' onclick="exodus_confirm_function' + buttonn + '()"'

        //letter
        var letter = buttontext.match(/(<[uU]>)(.)/)
        if (letter)
            letter = letter[2]
        else if (!istextinput)
            letter = buttontext.substr(0, 1)
        if (letter) {
            letter = letter.toUpperCase()
            html += ' exodusletter="' + letter + '"'
        }
        gexodusconfirmletters[buttonn] = letter

        //title
        html += 'title="Press '
        if (letter)
            html += letter + ' or '
        html += buttonfunckey + '"'

        //set the button number
        html += ' exodusbuttonnumber="' + nbuttons + '"'
        html += ' exodusyesnocancel="' + (buttonn % 3) + '"'

        html += '>' + buttontext + '</span>'

        //array of visible buttons
        //buttons[buttons.length] = button

        //set the default button
        //if (buttons.length == defaultbuttonn) {
        //    gdefaultbutton = button
        //    window.setTimeout('gdefaultbutton.focus()', 10)
        //}

    }//end of addbutton

    //template for html framework
    var html = '\
            <table cellspacing="1" cellpadding="1">\
            <tr align="left" width="5%">\
                <td style="vertical-align: middle;padding: 20px">\
                    '+ imagehtml + '\
                </td>\
                <td>\
                    <div class="statementclass" id="question1">\
                    '+ questionhtml + '\
                    </div>\
                </td>\
            </tr>'

    //add an empty table for any decide options
    if (decide_args) {
        html += '\
            <tr>\
            <td colspan=2 align="center">\
            <table id="decide_table1" xwidth=100% xclass="exodusform" bordercolor="#d0d0d0" cellspacing="0" xcellpadding="0">\
                <thead onclick="decide_sorttable2(event)" style="cursor: pointer">\
                    <tr id="decide_table1head1row1">\
                    </tr>\
                </thead>\
                <tbody id="decide_table1body1">\
                </tbody>\
            </table>\
            </td>\
            </tr>\
            <tr>\
            <td></td>\
            <td>\
            <button id="decide_okbutton" tabindex="0" class="graphicbutton"\
                onmousedown="this.style.borderStyle="inset"\
                onmouseup="this.style.borderStyle="outset"\
                onmouseout="this.style.borderStyle="outset"\
                title="Press Ctrl+Enter or F9">\
                OK</button>\
            <button id="decide_cancelbutton" tabindex="0" class="graphicbutton"\
                onmousedown="this.style.borderStyle="inset"\
                onmouseup="this.style.borderStyle="outset"\
                onmouseout="this.style.borderStyle="outset"\
                title="Press Esc">\
                Cancel</button>\
            </td>\
            </tr>'
    }

    //OK/Cancel/Print buttons at the bottom
    else {
        html += '\
                <tr>\
                    <td>\
                        &nbsp;\
                    </td>\
                    <td>\
                        <p id="textinputp" style="display: none">\
                            <input size="60" id="textinput" />\
                        </p>\
                        <p id="textinputphidden" style="display: none">\
                            <input type="password" size="60" id="textinputhidden" id="textinputhidden" />\
                        </p>\
                        <span id="yesnocancelbuttons">\
                            '+ buttonshtml + '\
                        </span>\
                    </td>\
                </tr>\
                </table>'
    }

    //finally create the div body
    div.innerHTML = html

    //insert and centralise the div after it has autosized itself
    document.body.insertBefore(div, null)

    //build rows of decide popup
    if (decide_args) {

        //build rows, or if only one option, then obtain the response
        /////////////////////////////////////////////////////////////
        var response = decide_onload(decide_args)

        //return only option
        // quit if false ie failed to load eg no options
        if (typeof response != 'undefined') {
            exodusremovenode(div)
            return response
        }

    }

    var top = window.innerHeight / 2 - div.offsetHeight / 2
    var left = window.innerWidth / 2 - div.offsetWidth / 2
    div.style.top = top + 'px'
    div.style.left = left + 'px'

    //if case too much to fit vertically on the screen, use scrollbars
    //for messages show the bottom of the message
    //for popup lists, show the top of the list
    if (!decide_args)
        div.scrollTop = div.scrollHeight

    //div.onkeydown=function exodusconfirm_onkeydown(event) {
    //    exoduscancelevent(event)
    //}

    //YIELD RIGHT HERE!
    //1. hang here until something like a button click function calls geventhandler .next(response)
    //2. keyword "yield" causes crash in internet explorer so it will be commented out in /2/ version
    //////////////////////////////////////////////////////////////////////////////////////////////////

    var response = yield* exodus_yield('exodusconfirm2')

    exodusremovenode(div)

    //text input returns a string (may be zero length) or false if clicked cancel
    if (typeof text != 'undefined' || text === null) {
        if (typeof response == 'string')
            return response
    }

    //no response treated same as cancel button (0) or '' if popup list
    if (!response) {
        if (decide_args)
            response = ''
        else
            response = 0
    }

    return response
}

//return 1
function exodus_confirm_function1(event) {
    return exodus_confirm_function(1, event)
}

//return 2
function exodus_confirm_function2(event) {
    return exodus_confirm_function(2, event)
}

//return 0
function exodus_confirm_function3(event) {
    return exodus_confirm_function(0, event)
}

function exodus_confirm_function(buttonno, event) {

    console.log('exodus_confirm_function buttonno:' + buttonno)
    event = getevent(event)
    exoduscancelevent(event)
    exodus_resume(buttonno, 'exodus_confirm_function')
}

//backpage when popup is up should remove the popup and NOT backpage
//especially important on mobile where no escape or backspace key exists
window.addEventListener('popstate', cancel_backpage_event, false);
function cancel_backpage_event(event) {
    // The popstate event is fired each time when the current history entry changes.
    // there must be some history present (see pushState below)

    //execute the normal action if no popup is present
    if (!document.getElementById('exodusconfirmdiv')) {
        history.back()
        return
    }

    //remove the popup and its controlling generator/coroutine
    //exodusremovenode(exodusconfirmdiv)
    //geventhandler = false
    exodus_resume(false, 'cancel_backpage_event')

    //make sure there is some history remains
    history.pushState(null, null, window.location.pathname);

    return
}

function decide_onload(decide_args) {

    //var question = decide_args[0]
    var data = decide_args[1]
    var cols = decide_args[2]
    var decide_returncolid = decide_args[3]
    var defaultreply = decide_args[4]//must be an array with method exoduslocate
    var decide_returnmany = decide_args[5]
    var decide_inverted = decide_args[6]

    //data
    //[[,,,],[,,,],[,,,]] or
    //col1.1 vm col1.2 fm col2.1 vm col2.2 etc
    if (!data && !cols)
        data = [['Yes'], ['No']]
    if (data && typeof data == 'string') {
        data = data.split(data.indexOf(fm) + 1 ? fm : ':')
    }
    for (ii = 0; ii < data.length; ii++) {
        if (typeof data[ii] == 'string') {
            data[ii] = data[ii].split(data[ii].indexOf(vm) + 1 ? vm : ';')
        }
        else if (typeof data[ii] == 'number')
            data[ii] = [data[ii]]
    }

    //quit if no data
    if (!data.length) {
        alert('No data, no options available')
        return false
    }

    //columns
    //[[dictid,title],etc. or
    //colid vm coltitle fm ... etc one per column
    if (!cols) {
        cols = []
        for (var ii = 0; ii < data[0].length; ii++)
            cols[ii] = [ii, '']
    }
    if (typeof cols == 'string') {
        if (cols.indexOf(fm) < 0 && cols.indexOf(vm) < 0) {
            cols = cols.replace(/:/g, fm)
            cols = cols.replace(/;/g, vm)
        }
        cols = cols.split(fm)
    }
    for (ii = 0; ii < cols.length; ii++) {
        if (typeof cols[ii] == 'string')
            cols[ii] = cols[ii].split(vm)
    }
    var ncols = cols.length

    //decide_returncolid
    if (typeof decide_returncolid == 'undefined')
        decide_returncolid = ''

    //solve ptcy pop account list without ledger present
    //convert numerical column number to colid
    if (typeof decide_returncolid == 'number') {
        var tt = cols[decide_returncolid]
        //dont convert if column ids are numeric since presumably the returncolid is an id not a columnno
        if (tt && tt[0] && !exodusnum(tt[0]))
            decide_returncolid = tt[0]
    }

    //defaultreply

    //returnmany
    //returnmany=

    //build the table

    //build the column headings row
    var firstselection

    //get a handle on the existing heading row
    var oRow = $$('decide_table1head1row1')

    //add two columns for the check box or radio button column to the column heading
    var oCell = document.createElement('th')
    oCell.colSpan = 2
    oCell.align = 'center'
    if (decide_returnmany)
        var tt = '<button'
            + ' title="Press A for All"'
            //+ ' onclick="decide_all_onclick()"'
            + ' style="font-size:80%" class="exodusbutton"'
            + '>All</button>'
    else
        var tt = '&nbsp;'
    oCell.innerHTML = tt
    oRow.appendChild(oCell)

    if (decide_returnmany)
        oCell.getElementsByTagName('button')[0].onclick = decide_all_onclick

    //add a column to show the order of selections
    if (decide_returnmany) {
        var oCell = document.createElement('th')
        oCell.align = 'center'
        if (decide_returnmany)
            oRow.appendChild(oCell)
    }

    //add other column headings
    for (ii = 0; ii < ncols; ii++) {
        var oCell = document.createElement('th')
        oCell.style.fontWeight = 'bold'
        var title = cols[ii][1]
        if (typeof title == 'undefined') {
            title = cols[ii][0]
            if (!title)
                title = ''
            if (parseInt(title))
                title = ''
            title = title.exoduscapitalise().replace(/_/gi, ' ')
        }
        oCell.innerHTML = title
        oRow.appendChild(oCell)

        if (!cols[ii][3] && cols[ii][2] && (cols[ii][2] == 'DATE' || cols[ii][2] == 'TIME'))
            cols[ii][3] = 'right'

        //check colid is in data
        //if ((!inverted&&typeof (data[0][cols[ii][0]])=='undefined')||(inverted&&typeof (data[cols[ii][0]][0])=='undefined'))
        if ((!decide_inverted && typeof (data[0][cols[ii][0]]) == 'undefined') || (decide_inverted && typeof (data[cols[ii][0]]) == 'undefined')) {
            //  alert(cols[ii])
            alert('popup column "' + cols[ii][0] + '" not in popup data in decide_onload')
            return false
        }

    }

    //column heading row built

    //build the table rows
    var starttime = new Date
    var maxsecs = 10

    var table1body1x = $$('decide_table1body1')

    var optionno = 0

    //rows
    var nrows = decide_inverted ? data[cols[0][0]].length : data.length
    for (rown = 0; rown < nrows; rown++) {

        //break if too many rows
        if ((new Date() - starttime) > (maxsecs * 1000)) {
            if (!(confirm('A large popup window is taking time to prepare.\r\rClick [OK] to wait or [Cancel] to see part.')))
                break
            starttime = new Date
        }

        if (decide_returncolid === '') {
            returnvalue = rown + 1
        }
        else {
            returnvalue = decide_inverted ? data[decide_returncolid][rown] : data[rown][decide_returncolid]
            if (typeof returnvalue == 'object')
                returnvalue = returnvalue.text
        }

        //hide checkboxes on rows with nothing to return
        var visibilityhidden = ''
        if (decide_returncolid) {
            if (decide_returnmany)
                //visibilityhidden = returnvalue ? '' : 'onclick="decide_checkbox_select()"'
                visibilityhidden = ''
            else
                visibilityhidden = returnvalue ? '' : 'style="visibility:hidden"'
        }

        //create a new row
        var oRow = document.createElement('tr')
        oRow.setAttribute('decide_row', rown)

        table1body1x.appendChild(oRow)

        //add line number column
        var oCell = document.createElement('td')
        oRow.appendChild(oCell)
        //if (decide_returnmany)

        oCell.id = 'decide_optionno'
        oCell.name = 'decide_optionno'
        oCell.setAttribute('name', 'decide_optionno')
        if (visibilityhidden) {
            if (decide_returnmany)
                optionno++
            oCell.style.visibility = 'hidden'
        }
        else {
            optionno++
            oCell.setAttribute('decide_optionno', optionno)
            oCell.innerHTML = optionno + "."
        }

        //add a check box or radio button column
        var oCellx = document.createElement('td')
        oRow.appendChild(oCellx)

        var event_handler = undefined
        if (decide_returnmany) {
            oCellx.innerHTML = '<input ' + visibilityhidden + ' id="decide_selection" name="decide_selection" type="checkbox" />'
            event_handler = decide_checkbox_select
        }
        else {
            var checked = ''
            if (!firstselection) {
                if (!visibilityhidden) {
                    checked = 'checked=true'
                    firstselection = oCellx
                }
            }
            //oCellx.innerHTML = '<input ' + checked + ' ' + visibilityhidden + ' id=decide_selection name=decide_selection type=radio onmousedown="decide_radio_onmousedown_sync(this)" />'
            oCellx.innerHTML = '<input ' + checked + ' ' + visibilityhidden + ' id="decide_selection" name="decide_selection" type="radio" />'
            event_handler = decide_radio_select
        }

        var oCellxInput = oCellx.getElementsByTagName('input')[0]
        if (event_handler)
            //oCellxInput.onclick=event_handler
            oCellxInput.onmousedown = event_handler
        //addeventlistener(oCellxInput,'click',event_handler)
        oCellxInput.setAttribute('decide_optionno', optionno)

        if (defaultreply && defaultreply.exoduslocate(returnvalue))
            oCellx.firstChild.checked = true

        oCellx.firstChild.setAttribute('decide_returnvalue', returnvalue)

        var singlereturnvalue
        if (returnvalue) {
            if (typeof singlereturnvalue == 'undefined')
                singlereturnvalue = returnvalue
            else
                singlereturnvalue = false
        }

        //add a column to show the order of selections
        if (decide_returnmany) {
            var oCell = document.createElement('td')
            oCell.setAttribute('name', 'decide_rank')
            oCell.align = 'center'

            //set defaultreply's row, to rank 1 (GB)
            if (typeof defaultreply[0] == 'number') {
                if (rown == defaultreply[0] - 1)
                    oCell.innerHTML = 1
            }

            oRow.appendChild(oCell)
        }

        //add the data columns for this row
        for (coln = 0; coln < ncols; coln++) {
            var oCell = document.createElement('td')
            oRow.appendChild(oCell)

            var colinfo = cols[coln]

            if (decide_inverted) {
                value = data[colinfo[0]][rown]
            }
            else {
                value = data[rown][colinfo[0]]
            }
            if (typeof value == 'undefined')
                value = ''
            else if (typeof value == 'object')
                value = value.text

            //date conversion
            if (colinfo[2] == 'DATE' && exodusnum(value))
                value = DATE(value)

            //time conversion
            if (colinfo[2] == 'TIME' && exodusnum(value))
                value = TIME(value)

            //oCell.innerHTML=value
            //use innerText otherwise things like <> in the data do not show
            //oCell.innerText = value
            //oCell.innerHTML = value.exodusconvert(vm+sm+tm,'   ')
            value = value.replace(ALLFMre, '<br/>')
            oCell.innerHTML = value

            //align
            if (colinfo[3])
                oCell.align = colinfo[3]
            else
                oCell.align = 'left'

            if (defaultreply) {
                if (typeof decide_returncolid === '') {
                    if (defaultreply.exoduslocate(rown + 1))
                        oCellx.firstChild.checked = true
                }
                else {
                    //returncolid was being treated as onscreen col preventing proper defaulting
                    //if (value && coln === returncolid) {
                    if (value && (colinfo[0] == decide_returncolid || Number(colinfo[0]) == decide_returncolid)) {
                        if (defaultreply.exoduslocate(value))
                            oCellx.firstChild.checked = true
                    }
                }
            }

        }

        //table rows built

        //table built

    }

    var okbutton = $$('decide_okbutton')
    okbutton.onclick = decide_ok_onclick

    var cancelbutton = $$('decide_cancelbutton')
    cancelbutton.onclick = decide_cancel_onclick

    //autoselect only one option
    if (singlereturnvalue) {
        if (decide_returnmany)
            singlereturnvalue = [singlereturnvalue]
        return singlereturnvalue
    } else if (typeof singlereturnvalue == 'undefined') {
        alert('No data, no options available')
        return false
    }

    //exodussettimeout('exodusautofitwindow()', 10)

    var selections = document.getElementsByName('decide_selection')

    //focus on the first checked item or the first rown
    for (var ii = 0; ii < selections.length; ++ii)
        if (selections[ii].checked)
            break
    if (ii >= selections.length)
        ii = 0
    //selections[ii].focus()
    client_focuson(selections[ii])

    //ensure some history exists so that the "backpage/backbutton"
    //"popstate" event occurs. See above
    if (typeof ghistorypushed == 'undefined') {
        ghistorypushed = true
        history.pushState(null, null, window.location.pathname);
    }

    addeventlistener(exodusconfirmdiv, 'keydown', decide_document_onkeydown)
    addeventlistener(exodusconfirmdiv, 'keyup', decide_document_onkeyup)
    addeventlistener(exodusconfirmdiv, 'click', decide_document_onclick)
    addeventlistener(exodusconfirmdiv, 'dblclick', decide_document_ondblclick)
    addeventlistener(exodusconfirmdiv, 'mouseover', decide_document_onmouseover)
    addeventlistener(exodusconfirmdiv, 'mouseout', decide_document_onmouseout)

    //returning undefined indicates that we need to yield and wait for decide_ok_onclick etc to resume
    //returning false indicates some problem
    //returning anything else indicates that there is only one option
    return undefined

    //remainder of functions is event handlers

    function decide_all_onclick(event) {

        selections = document.getElementsByName('decide_selection')
        var truefalse = !selections[0].checked

        //have to clear all existing ranking first
        decide_all_clear()

        var lastrank = 0
        if (truefalse) {
            for (var ii = 0; ii < selections.length; ii++)
                //selections[ii].checked = truefalse
                lastrank = decide_checkbox_select(event, selections[ii], truefalse, ii, lastrank)
        }
    }

    function decide_all_clear() {
        selections = document.getElementsByName('decide_selection')
        ranks = document.getElementsByName('decide_rank')
        for (var ii = 0; ii < selections.length; ii++) {
            var element = selections[ii]
            element.checked = false
            var rowtag = getancestor(element, 'tr')
            rowtag.style.fontWeight = 'normal'
            ranks[ii].innerText = ''
        }
    }

    function decide_checkbox_selectone(element, checking, checkrown, ranks, lastrank) {

        element.checked = checking

        //var rowtag=getancestor(element,'tr')
        rowtag = element.parentNode.parentNode
        rowtag.style.fontWeight = checking ? 'bold' : 'normal'

        if (checking) {
            lastrank++
            ranks[checkrown].innerText = lastrank
        } else
            ranks[checkrown].innerText = ''

        return lastrank
    }

    function decide_checkbox_select(event, element, checking, checkrown, lastrank) {

        event = getevent(event)
        if (!element)
            element = event.target

        //dont click on checkbox since mousedown will have already done the job
        if (event.type == 'click' && event.target.type == 'checkbox')
            return false

        var doingall = true
        if (typeof checkrown == 'undefined') {
            checking = !element.checked
            doingall = false
        }

        if (typeof checking == 'undefined')
            checking = !element.checked

        //find current rown and level
        if (typeof checkrown == 'undefined') {
            var indentregex = /^[\. -]*/
            var selection2 = document.getElementsByName('decide_selection')
            var level
            for (ii = 0; ii < selection2.length; ii++) {
                if (selection2[ii] == element) {
                    checkrown = ii
                    level = selection2[ii].parentNode.nextSibling.nextSibling.innerText.match(indentregex)
                    break
                }
            }
        }

        var ranks = document.getElementsByName('decide_rank')

        //checking=determine lastrank unless provided
        if (checking) {
            //count selections
            if (!doingall) {
                lastrank = 0
                var checkupto = doingall ? checkrown : ranks.length
                for (var ii = 0; ii < checkupto; ++ii) {
                    var rank = Number(ranks[ii].innerText)
                    if (rank)
                        lastrank++
                }
            }
        }

        //unchecking=reduce higher ranks
        else {
            var removerank = Number(ranks[checkrown].innerText)
            for (var ii = 0; ii < ranks.length; ++ii) {
                var rank = Number(ranks[ii].innerText)
                if (rank > removerank)
                    ranks[ii].innerText = rank - 1
            }
        }

        lastrank = decide_checkbox_selectone(element, checking, checkrown, ranks, lastrank)

        //check/uncheck any following lower level rows
        if (!doingall) {
            for (ii = checkrown + 1; ii < selection2.length; ii++) {
                //quit once get back to the same level
                if (selection2[ii].parentNode.nextSibling.nextSibling.innerText.match(indentregex) <= level)
                    break
                lastrank = decide_checkbox_selectone(selection2[ii], checking, ii, ranks, lastrank)
                //lastrank=decide_checkbox_select(event,selection2[ii],checking,ii,lastrank)
            }
        }

        //necessary to prevent unchecking by something unknown higher up
        //event.cancelBubble=true
        exoduscancelevent(event)

        if (doingall)
            return lastrank
        else
            return false
    }

    function decide_radio_select(event, element) {
        event = getevent(event)
        if (!element)
            element = event.target
        element.checked = true
        decide_ok_onclick()
        return exoduscancelevent(event)
    }

    function decide_document_onmouseover(event) {
        return decide_document_onmouse(event, 'over')
    }

    function decide_document_onmouseout(event) {
        return decide_document_onmouse(event, 'out')
    }

    function decide_document_onmouse(event, mode) {

        event = getevent(event)
        exoduscancelevent(event)

        var trtag = getancestor(event.target, 'tr')
        if (!trtag)
            return false
        if (!trtag.getAttribute('decide_row'))
            return false

        var element = trtag.getElementsByTagName('input')[0]
        if (!element)
            return

        if (mode == 'over') {
            trtag.setAttribute('mousesave', trtag.style.backgroundColor)
            trtag.style.backgroundColor = 'lightgrey'
        } else {
            trtag.style.backgroundColor = trtag.getAttribute('mousesave')
            trtag.removeAttribute('mousesave')
        }

        return
    }

    function decide_document_onclick(event, dblclick) {
        event = getevent(event)

        var trtag = getancestor(event.target, 'tr')
        var element
        if (!trtag || !(element = trtag.getElementsByTagName('input')[0])) {
            element = document.getElementsByName('decide_selection')[0]
            client_focuson(element)
            return
        }

        if (decide_returnmany)
            decide_checkbox_select(event, element, dblclick)
        else
            decide_radio_select(event, element)

        client_focuson(element)

        return exoduscancelevent(event)
    }

    function decide_document_ondblclick(event) {
        decide_document_onclick(event, true)
        decide_ok_onclick()
        return exoduscancelevent(event)
    }

    function decide_getreturnvalues() {

        //NB returns row numbers 1 based not 0 based!
        //0 means cancelled

        var returnvalues = []
        var selection2 = document.getElementsByName('decide_selection')
        var ranks = document.getElementsByName('decide_rank')
        for (ii = 0; ii < selection2.length; ii++) {
            if (selection2[ii].checked) {
                var returnvalue = selection2[ii].getAttribute('decide_returnvalue')
                if (!decide_returnmany) {
                    returnvalues = returnvalue
                    break
                }

                if (returnvalue) {
                    var rank = Number(ranks[ii].innerText) - 1
                    returnvalues[rank] = returnvalue
                }
            }
        }

        if (!returnvalues.length)
            returnvalues = ''

        //remove empty array values
        returnvalues = returnvalues.exodustrim('')

        return returnvalues

    }

    function decide_ok_onclick() {

        var returnvalues = decide_getreturnvalues()

        //return exoduswindowclose(returnvalues)
        exodus_resume(returnvalues, 'decide_ok_onclick')

    }

    function decide_cancel_onclick() {
        //return exoduswindowclose('')
        exodus_resume('', 'decide_ok_onclick')
    }

    //purely to suppress any automatic checkbox ticking by the browser
    //so we can control it in onkeydown
    function decide_document_onkeyup(event) {
        return exoduscancelevent(event)
    }
    function decide_document_onmouseup(event) {
        return exoduscancelevent(event)
    }

    function decide_document_onkeydown(event) {

        event = getevent(event)

        if (typeof decide_returnmany == 'undefined')
            decide_returnmany = ''

        var keycode = event.keyCode

        console.log('decide_document_onkeydown ' + keycode)

        //ctrl+Enter or single select
        if (keycode == 13 && event.ctrlKey) {
            decide_ok_onclick()
            return exoduscancelevent(event)
        }

        //F9 is old save
        if (keycode == 120) {
            decide_ok_onclick()
            return exoduscancelevent(event)
        }

        //Esc is cancel
        if (keycode == 27) {
            decide_cancel_onclick()
            return exoduscancelevent(event)
        }

        //home goto top if scrollbar
        if (keycode == 36) {
            exodusconfirmdiv.scrollTop = 0
            return exoduscancelevent(event)
        }

        //end goto bottom if scrollbar
        if (keycode == 35) {
            exodusconfirmdiv.scrollTop = exodusconfirmdiv.scrollHeight
            return exoduscancelevent(event)
        }

        var selections = document.getElementsByName('decide_selection')
        var options = document.getElementsByName('decide_optionno')

        var element = event.target

        //ctrl+enter and f9 is ok ... so is space if not !returnmany
        if (keycode == 120 || (keycode == 13 && event.ctrlKey) || (keycode == 32 && !decide_returnmany)) {
            decide_ok_onclick()
            return exoduscancelevent(event)
        }

        //digits 0-9 select options 1-10
        if (keycode >= 49 && keycode <= 57) {

            var optionn = keycode - 48
            if (optionn == 0)
                optionn = 10
            for (var rown = 0; rown < options.length; ++rown) {
                if (options[rown].getAttribute('decide_optionno') == optionn) {
                    if (!decide_returnmany || selections.length == 1) {
                        //decide_ok_onclick()
                        decide_radio_select(event, selections[rown])
                        break
                    }
                    //selections[rown].checked = !selections[rown].checked
                    //break;
                    var element = selections[rown]
                    decide_checkbox_select(event, element)
                    client_focuson(element)
                    break
                }
            }
            return exoduscancelevent(event)
        }

        //enter or space - check or clear a checkbox and move down (or up if shift)
        if (decide_returnmany && (keycode == 32 || keycode == 13)) {
            decide_checkbox_select(event)
            if (event.shiftKey)
                keycode = 38//fake up
            else
                keycode = 40//fake down
        }

        var n
        if (element.rowIndex)
            n = element.rowIndex
        else if (element.parentNode && element.parentNode.rowIndex)
            n = element.parentNode.rowIndex
        else if (element.parentNode && element.parentNode.parentNode && element.parentNode.parentNode.rowIndex)
            n = element.parentNode.parentNode.rowIndex
        else
            return

        n -= 1

        //pgup 33/pgdn 34/down 40/up 38/tab 9/backspace 8 keys
        if (keycode == 33 || keycode == 34 || keycode == 40 || keycode == 38 || keycode == 9 || keycode == 8) {

            var direction
            if (keycode == 34 || keycode == 40 || (keycode == 9 && !event.shiftKey)) direction = 1
            if (keycode == 33 || keycode == 38 || (keycode == 9 && event.shiftKey || keycode == 8)) direction = -1
            //pgdn
            if (keycode == 34) {
                if (event.ctrlKey) {
                    n = selections.length - 1
                }
                else {
                    if (n == (selections.length - 1))
                        n = 0
                    else {
                        n += 10
                        if (n >= (selections.length - 1))
                            n = selections.length - 1
                    }
                }
                n--
            }
            //pgup
            if (keycode == 33) {
                if (event.ctrlKey) {
                    n = 0
                }
                else {
                    if (n == 0)
                        n = selections.length - 1
                    else {
                        n -= 10
                        if (n <= 0)
                            n = 0
                    }
                }
                n++
            }
            var newelement = element
            while (true) {

                n += direction
                if (n < 0) n = selections.length - 1
                if (n >= selections.length) n = 0

                newelement = selections[n]

                //skip if no other suitable elements
                if (newelement == element)
                    return

                //skip if not visible
                if (!newelement)
                    return false//may not be on an element
                if (newelement.style.visibility == 'hidden')
                    continue

                //scroll to the top or bottom if on the first or last option
                var newoptionno = newelement.getAttribute('decide_optionno')
                if (n == 0 || newoptionno == 1)
                    exodusconfirmdiv.scrollTop = 0
                else if (n == (selections.length - 1))
                    exodusconfirmdiv.scrollTop = exodusconfirmdiv.scrollHeight
                break

            }

            newelement.focus()
            newelement.select()

            if (keycode == 8 && decide_returnmany) {
                decide_checkbox_select(event, newelement)
                return exoduscancelevent(event)
            }

            //select new radio button if pressing up/down (but not pgup/pgdn so they can scan multi-page options without changing the currently selected option)
            if (!decide_returnmany && keycode != 33 && keycode != 34)
                newelement.checked = true

            return exoduscancelevent(event)

        }

        //all following refers to many selections
        if (!decide_returnmany) {
            if (keycode == 13) {
                decide_ok_onclick()
                return exoduscancelevent(event)
            }
            return
        }

        //A=all or none
        if (keycode == 65) {
            decide_all_onclick()
            return exoduscancelevent(event)
        }

        //del or F8 none/delete
        if (keycode == 46 || keycode == 119) {
            decide_all_clear()
            return exoduscancelevent(event)
        }

    }

} //of decide_onload()

var gsorttable2offset = 1//decide
//var gsorttable2offset=0//decide2
function decide_sorttable2(event) {

    //locate the current element
    event = getevent(event)

    var th = event.target
    if (th.tagName != "TH")
        return (0)

    var tableelement = th.parentElement.parentElement.parentElement
    var tablerows = tableelement.tBodies[0].getElementsByTagName('tr')
    var coln = th.cellIndex + gsorttable2offset

    var reverse = event.target.getAttribute('sorttable2_issorted')
    event.target.setAttribute('sorttable2_issorted', reverse ? '' : 1)

    var rown = th.parentElement.rowIndex
    var nrows = tablerows.length
    fromrown = 0
    uptorown = nrows - 1

    if (gdateformat == 'M/d/yyyy')
        var dateformat = [2, 0, 1]
    else if (gdateformat == 'yyyy/M/d')
        var dateformat = [0, 1, 2]
    else //gdateformat = 'd/M/yyyy'
        var dateformat = [2, 1, 0]
    var yy = dateformat[0] + 1
    var mm = dateformat[1] + 1
    var dd = dateformat[2] + 1

    var sortdata = []
    var dateregex = / ?(\d{4}|\d{1,2})\/ ?(\d{1,2})\/(\d{4}|\d{2})/
    var periodregex = / ?(\d{1,2})\/(\d{4})/g
    for (var ii = fromrown; ii <= uptorown; ++ii) {
        var cell = tablerows[ii].cells[coln]
        var value = (cell.textContent || cell.innerText || "").toUpperCase()
        var match
        while (match = value.match(dateregex)) {
            //convert dates like n/n/yy or n/n/yyyy to sortable yyyy|mm|dd format
            value = value.replace(dateregex, ('0000' + match[yy]).slice(-4) + '|' + ('00' + match[mm]).slice(-2) + '|' + ('00' + match[dd]).slice(-2))
            //console.log(value)
        }

        value = value.replace(periodregex, '$2|$1')

        //natural sort
        value = value.replace(/[-+]?[1234567890.,]+/g, function (x) { if (x.slice(0, 1) == '-') { y = '-'; x = x.slice(1) } else y = ''; return y + ('0000000000000000000000' + x).slice(-20) })
        //value+=('000000000000'+ii).slice(-10)//stable sort
        //cell.setAttribute('sortvalue',value)
        sortdata.push([value, ii])
    }

    sortdata.sort(function (a, b) { if (a[0] < b[0]) return -1; if (a[0] > b[0]) return 1; return a[1] - b[1] })
    if (reverse)
        sortdata.reverse()

    //get an array of the tablerows
    var oldrows = []
    for (var oldrown = 0; oldrown < tablerows.length; oldrown++)
        oldrows[oldrown] = tablerows[oldrown]

    //reorder table rows
    var newdatarows = []
    for (var newrown = 0; newrown < sortdata.length; ++newrown) {
        var oldrown = sortdata[newrown][1]
        if (newrown != oldrown)
            tablerows[newrown].swapNode(oldrows[oldrown])
    }

}//decide_sorttable2

//these functions should be removed after a while if never called
function login() {
    if (gusername && gusername == 'EXODUS')
        alert(login.caller.name + ' called login')
}
function log() {
    //if (gusername&&gusername=='EXODUS')
    //    alert(log.caller.name+' called log')
}
function logout() {
    if (gusername && gusername == 'EXODUS')
        alert(logout.caller.name + ' called logout')
}

function setdateformat() {

    gfirstdayofweek = exodusgetcookie2('fd')
    if (!gfirstdayofweek) gfirstdayofweek = 1
    gfirstdayofweek = Number(gfirstdayofweek)

    var dateformat = exodusgetcookie2('df')
    //international and default
    if (!dateformat || dateformat.slice(0, 2) == '31') {
        gdatedaypos = 0
        gdatemonthpos = 1
        gdateyearpos = 2
        gdateformat = 'd/M/yyyy'
        //american
    } else if (dateformat.slice(0, 2) == '01') {
        gdatedaypos = 1
        gdatemonthpos = 0
        gdateyearpos = 2
        gdateformat = 'M/d/yyyy'
    }
    //modern sortable date
    else {
        gdatedaypos = 2
        gdatemonthpos = 1
        gdateyearpos = 0
        gdateformat = 'yyyy/M/d'
    }
    //console.log('setdateformat() '+gfirstdayofweek)
    return
}

function DATE(mode, value, params) {

    //gmsg='e.g. 31, 31/1, 31/1/01 or 31/1/2001'

    //can handle an array of values
    if (typeof value == 'object')
        return exodusconvarray(DATE, mode, value, params)

    //not == which would disallow 0 which is 31/12/1967
    if (value === '')
        return ''

    if (typeof value == 'undefined') {
        value = mode
        mode = 'OCONV'
    }

    var result
    var result2

    //input conversion
    if (mode == 'ICONV') {

        //four digits are assumed to be in ddmm or mmdd format
        if (value.match(/^\d{4}$/))
            value = value.substr(0, 2) + '/' + value.substr(2, 2)

        //five digits are assumed to be already in internal format
        if (value.match(/^\d{5}$/))
            return value

        //six/eight digits are assumed to be in DDMMYY/MMDDYY/DDMMYYYY/MMDDYYYY format
        if (value.match(/^(\d{6})|(\d{8})$/))
            value = value.substr(0, 2) + '/' + value.substr(2, 2) + '/' + value.substr(4)

        //assume in the format 31x1x2000
        value = value.replace(/\W/g, ' ').split(' ').slice(0, 3)

        //check day is one or two digits and force to three values (blanks if necessary)
        if (value.length == 1) {
            value[gdatedaypos] = value[0]
            value[gdatemonthpos] = ''
            value[gdateyearpos] = ''
        }
        else if (value.length == 2)
            value[2] = ''

        //default params
        if (typeof params == 'undefined') params = ''

        //option to not iconv (re
        //if (params=='NOICONV')
        //{
        // if (value.length==1)
        // return value[0]
        //}

        var fromto = ''
        if (params.indexOf('=') + 1) {
            params = params.split('=')
            fromto = params[0]
            var otherdateid = params[1]
            //otherdate = yield* gds.getx(otherdateid,grecn)
            //otherdate = getvalue(otherdateid,grecn)
            //direct access to gds.data to get the internal date (getvalue gets the external format)
            //should NOT be accessing gds directly but cannot use yielding .getx from non-yielding oconv/iconv
            //TODO doesnt handle other dates in rows very well
            var otherdate = gds.data[otherdateid]
            if (!otherdate)
                otherdate = gds.data['group' + ggroupno][grecn][otherdateid]
            otherdate = otherdate.text
            if (typeof otherdate == 'object')
                otherdate = otherdate[0]

            //params=otherdate.exodusoconv('[DATE]').exodusfield('/',2,2)
            //get month/year
            params = otherdate.exodusoconv('[DATE,MONTH]') + '/' + otherdate.exodusoconv('[DATE,YEAR]')

        }

        //split the params
        params = params.replace(/\//g, ' ').split(' ')

        //default to month parameter
        //if (!value[1]&&!!params[0]) value[1]=params[0]
        if (!value[gdatemonthpos] && !!params[0])
            value[gdatemonthpos] = params[0]

        //otherwise default to current month
        if (!value[gdatemonthpos])
            value[gdatemonthpos] = new Date().getMonth() + 1

        //convert month names to month numbers
        var monthno = parseInt(value[gdatemonthpos], 10)
        if (isNaN(monthno)) {
            var mthname = value[gdatemonthpos].toUpperCase()
            var mthnamelen = mthname.length
            for (var ii = 0; ii < 12; ++ii) {
                if (gmonthnames[ii].slice(0, mthnamelen).toUpperCase() == mthname) {
                    monthno = ii + 1
                    value[gdatemonthpos] = monthno
                    break
                }
            }
        }

        //default to year parameter
        if (!value[gdateyearpos] && !!params[1])
            value[gdateyearpos] = params[1]

        //otherwise default to current year
        if (value[gdateyearpos])
            value[gdateyearpos] = ADDCENT(value[gdateyearpos])
        else
            value[gdateyearpos] = new Date().getFullYear()

        //check integers
        if (isNaN(parseInt(value[gdatedaypos], 10)) || isNaN(monthno) || isNaN(parseInt(value[gdateyearpos], 10))) {
            gmsg = invaliddatemsg()
            return null
        }

        //check day is one or two digits
        if (!value[gdatedaypos].match(/^\d{1,2}$/)) {
            gmsg = invaliddatemsg()
            return null
        }

        /* convert to ms date
        //try to convert to internal date format
        result=new Date(value[2],value[1]-1,value[0])
        if (isNaN(result)) {

        gmsg=invaliddatemsg()
        return null
        }
        */

        //convert dates from MS base to base zero=31/12/67
        result = (Date.UTC(value[gdateyearpos], value[gdatemonthpos] - 1, value[gdatedaypos]) - Date.UTC(1967, 11, 31)) / 24 / 60 / 60 / 1000

        //prevent 6 digit years ie ge 15 oct 2241
        //causes problem with -10000
        //if (result.toString().length > 5) {
        //    gmsg = invaliddatemsg()
        //    return null
        //}

        //limit 1/1/1900 to 31/12/2099
        if (result < -24835 || result > 48213) {
            gmsg = invaliddatemsg()
            return null
        }

        //check that the oconv matches the iconv
        //the above algorithm allows dom <=99 and is calculated into next months
        result2 = DATE('OCONV', result).split('/')
        if (parseInt(result2[gdatedaypos], 10) != parseInt(value[gdatedaypos], 10) || parseInt(result2[gdatemonthpos], 10) != parseInt(value[gdatemonthpos], 10)) {
            gmsg = invaliddatemsg()
            return null
        }

        //check versus/amend other date

        if (fromto) {

            var otherdate0 = otherdate

            //<= >= and !== allow for date 0 31/12/1967

            //prevent todate less than fromdate
            if (fromto == 'TO' && result >= otherdate)
                otherdate = result
            if (fromto == 'FROM' && result <= otherdate)
                otherdate = result

            //if change fromdate and todate blank or same as from goldvalue
            if (fromto == 'UPTO' && (otherdate == '' || otherdate == goldvalue))
                otherdate = result

            //update the otherdate
            if (otherdate !== otherdate0) {

                //cant call async gds.setx while oconv is not async so do it by timeout
                //cant call setvalue either since that only updates the screen and not gds
                //yield* gds.setx(otherdateid, grecn, otherdate)
                exodussettimeout('yield* gds.setx("' + otherdateid + '", ' + grecn + ', ' + otherdate + ')', 1)
            }

        }
    }

    //output conversion
    else {

        //  if (value=='"') alert(value)
        //if not digits then return unconverted
        //allow for negative dates before 1968
        if (typeof value == 'string' && !value.match(/^-?\d*$/))
            return value
        //  if (value=='"') alert('x'+value)

        /* convert to ms date
        value=new Date(value)
        if (isNaN(value)) {

        gmsg=invaliddatemsg()
        return null
        }
        //zzz should format it with params?
        result=value.getDate()+'/'+(value.getMonth()+1)+'/'+value.getFullYear()
        */

        //convert from 1=1/1/67 to text DD/MM/YYYY format
        //result=new Date(1967,11,31+parseInt(value,10))
        result = new Date
        result.setTime(Date.UTC(1967, 11, 31 + parseInt(value, 10)))

        switch (params) {
            case 'DOW': {

                result = ((value - 1) % 7) + 1
                break
            }
            case 'DAYNAME': {

                result = ((value - 1) % 7) + 1
                result = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday'][result - 1]
                break
            }
            case 'DAYNAME3': {

                result = ((value - 1) % 7) + 1
                result = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'][result - 1]
                break
            }
            case 'DOM': {

                result = result.getUTCDate()
                break
            }
            case 'MONTH': {

                result = result.getUTCMonth() + 1
                break
            }
            case 'DOM2': {

                result = ('0' + result.getUTCDate()).slice(-2)
                break
            }
            case 'MONTH2': {

                result = ('0' + (result.getUTCMonth() + 1)).slice(-2)
                break
            }
            case 'YEAR': {

                result = result.getUTCFullYear()
                break
            }
            case 'YEAR2': {

                result = parseInt(result.getUTCFullYear().toString().slice(-2), 10)
                break
            }
            case 'PERIOD': {

                //returns the current month/current year eg 1/2000
                result = (result.getUTCMonth() + 1) + '/' + result.getUTCFullYear()
                break
            }
            case 'PERIOD2': {

                //returns the current month/current year eg 1/00
                result = (result.getUTCMonth() + 1) + '/' + result.getUTCFullYear().toString().slice(-2)
                break
            }
            case 'YEARPERIOD': {

                //returns the current current year.current month eg 2001.01
                result = result.getUTCFullYear() + '.' + ('00' + (result.getUTCMonth() + 1)).slice(-2)
                break
            }
            case 'MINIMAL': {

                result2 = []
                result2[gdatedaypos] = result.getUTCDate()
                result2[gdatemonthpos] = result.getUTCMonth() + 1
                result2[gdateyearpos] = result.getUTCFullYear()
                result = result2

                //trim same year and month
                var curryear = (new Date().getFullYear()).toString()
                if (result[gdateyearpos] == curryear) {
                    result[gdateyearpos] = ''
                    var currmonth = (new Date().getMonth() + 1).toString()
                    if (result[gdatemonthpos] == currmonth) {
                        result[gdatemonthpos] = ''
                    }
                }
                result = result.exodusjoin('/').exodustrim('/')

                //add day of week
                var dow = ((value - 1) % 7) + 1
                result = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'][dow - 1] + ' ' + result

                break
            }
            case 'D': {
                //alert('"'+result+'"')
                //result = result.getUTCDate()// + '/' + gmonthnames[result.getUTCMonth() + 1] + '/' + result.getUTCFullYear().toString()
                result = result.getUTCDate() + ' ' + gmonthnames[result.getUTCMonth()].substr(0, 3).toUpperCase() + ' ' + result.getUTCFullYear();
                break;
            }
            default: {

                result2 = []
                result2[gdatedaypos] = result.getUTCDate()
                result2[gdatemonthpos] = result.getUTCMonth() + 1
                result2[gdateyearpos] = result.getUTCFullYear()
                result = result2.join('/')
            }
        }

    }

    return result

}

function TIME(mode, value, params) {

    gmsg = 'e.g. 12:45, 1245 12.75, 00:00, 23:59, 11:59:59, 24:00, 25:00 etc.'

    var result

    //can handle an array of values
    if (typeof value == 'object')
        return exodusconvarray(TIME, mode, value, params)
    /*
    //can handle an array of values
    if (typeof(value)=='object') {

    result=[]
    for (var i=0;i<value.length;i++) {

    result[i]=TIME(mode,value[i],params)

    //if any conversion fails return complete failure
    if (result[i]==null) return null

    }
    return result
    }
    */

    //blank in .. blank out
    if (value == '')
        return ''

    if (typeof value == 'undefined') {
        value = mode
        mode = 'OCONV'
    }

    if (mode == 'OCONV') {

        //times by themselves for now are considered to be "local time" of data entry person tz could be added info
        //DATE_TIME uses gtz
        //value=Number(value)+gtz[0]

        var secs = value % 60
        value -= secs
        var mins = (value / 60) % 60
        value -= 60 * mins
        //var hours=(value/60/60)%24
        //allow 24:00 and 25:00 etc
        var hours = (value / 60 / 60)
        result = ('0' + hours).slice(-2) + ':' + ('0' + mins).slice(-2)
        //        if (params.indexOf('S'))
        //         result+= ':' + ('0' + secs).slice(-2)

        //result+=':'+('0'+secs).slice(-2)
    }
    else {

        //format must be HH:MM or HH:MM:SS (":" may also be space or missing)
        var temp = value
        if (temp.match(/^\d{4}$/))
            temp = temp.substr(0, 2) + ':' + temp.substr(2, 2)
        if (temp.match(/^\d{6}$/))
            temp = temp.substr(0, 2) + ':' + temp.substr(2, 2) + ':' + temp.substr(4, 2)
        temp = temp.exodusconvert('. ', '::').split(':')
        if (!temp[1]) temp[1] = '00'
        if (!temp.join(':').match(/(^\d{1,2}:\d{1,2}$)|(^\d{1,2}:\d{1,2}:\d{1,2}$)/)) return null

        //if (!temp[1]) temp[1]=0
        if (!temp[2]) temp[2] = 0
        temp[0] = +temp[0]
        temp[1] = +temp[1]
        temp[2] = +temp[2]
        //if (temp[0]>23) return null
        //allow up to two days eg 47:59
        if (temp[0] > 47) return null
        if (temp[1] > 59) return null
        if (temp[2] > 59) return null

        result = temp[0] * 60 * 60 + temp[1] * 60 + temp[2]
        //times by themselves for now are considered to be "local time" of data entry person tz could be added info
        //DATE_TIME uses gtz
        //result-=gtz[0]

    }

    return result

}

function DATE_TIME(mode, value, params) {
    if (value == '') return ''
    if (mode == 'OCONV') {
        if (!params) params = ''
        params = (params + '!').split('!')
        value = value.toString().split('.')
        var datebit = Number(value[0])
        var timebit
        if (value.length < 1)
            timebit = ''
        else {
            timebit = Number(value[1]) + gtz[0]
            //assume -86400<gtz[0]<86400
            if (timebit < 0) {
                datebit -= 1
                timebit += 86400
            }
            else if (timebit > 86400) {
                datebit -= 1
                timebit += 86400
            }
        }
        return datebit.exodusoconv('[DATE,' + params[0] + ']') + ' ' + timebit.exodusoconv('[TIME,' + params[1] + ']')
    }
    else {
        //iconv not implemented yet
        return value
    }

}

function PERIOD_OF_TIME(mode, value, params) {

    if (value == '')
        return ''

    var result

    //can handle an array of values
    if (typeof value == 'object')
        return exodusconvarray(PERIOD_OF_TIME, mode, value, params)
    /*
    //can handle an array of values
    if (typeof(value)=='object') {

    result=[]
    for (var i=0;i<value.length;i++) {

    result[i]=PERIOD_OF_TIME(mode,value[i],params)

    //if any conversion fails return complete failure
    if (result[i]==null) return null

    }
    return result
    }
    */

    if (mode == 'ICONV') {

        //if already numeric then simply return it
        if (exodusnum(value)) return value

        //allow slash, dash, space and comma as well as ":" for separator
        value = value.exodusconvert('/- ,', '::::')

        value = value.split(':')
        if (value.length < 2) value[1] = 0

        hours = value[0]
        mins = value[1]

        //check numeric otherwise return undefined
        if (!(exodusnum(hours) || !exodusnum(mins))) {
            return null
        }

        result = +hours + (+mins) / 60

    }
    else {

        if (!(exodusnum(value))) {
            //out=''
            //status()=2
            return null
        }

        //nothing in nothing out
        if (value == '') return ''

        value = value.toString()
        value = value.split('.')
        if (value[0] == '') value[0] = '0'
        if (value.length < 2) value[1] = '0'

        var result = value[0] + ':'
        temp = value[1]
        if (temp) temp = parseFloat('.' + temp)
        temp = exodusround(temp * 60, 0)
        temp = '00' + temp
        result += temp.slice(temp.length - 2)

    }

    return result

}

var gclient_focuson_element
function client_focuson(element) {
    gclient_focuson_element = element
    window.setTimeout(client_focuson2, 1)
}

function client_focuson2(element) {
    if (gclient_focuson_element && gclient_focuson_element.focus) {
        gclient_focuson_element.focus()
        gclient_focuson_element = undefined
    }
}

function logevent(msg) {
    if (!console)
        return
    if (glogevents)
        console.log(msg)
}

//end of client.js
