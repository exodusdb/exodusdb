//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**
//undocumented functions
//CollectGarbage() in all versions 
//"finally" block in v5.0

//mozilla differences from IE
// HTML element has a parentNode which is not usual element so be careful in "ancestor finder" routines
// elements cannot be referred to simply as variables like idx
// (must use getElementById or set an equivalent global variable)
// insertBefore secondparameter is mandatory (put null)
// document.body cannot be referred to in heading scripts until the document body is started
// (so put document.onload function in the body tag. onload function cannot be called onload)
// selectelement.options() should 
//parentElement nonstandard use parentNode

//default location of neosys scripts etc
if (typeof NEOSYSlocation == 'undefined')
    NEOSYSlocation = '../neosys/'

gdomainname = window.location.toString().split('/')[2]
//if (document.cookie.indexOf('username') < 0)
//    alert('qno username')

//various images
var gimagetheme = '../neosys/images/theme2/'
//var gmenuimage=gimagetheme+'menu.png'//'add.png'
var gmenuimage = gimagetheme + 'menu.gif'//'add.gif'
var glogoutimage = gimagetheme + 'disconnect.png'//'add.png'
var grefreshimage = gimagetheme + 'refresh.png'

var e//mac safari 3.1.2 cannot tolerate "catch(e)"

//ensure print preview styles are set during print/preview
window.onbeforeprint = window_onbeforeprint
function window_onbeforeprint() {
    clientfunctions_setstyle()
}

var goriginalstyles = {}

//prevent framing
if (window != top)
    top.location.href = location.href

var gkeepalivemins = 10
//gkeepalivemins=1

//check browser capabilities

var unsupported = ''

//support "innerText" or fail
if (typeof document.createElement('DIV').innerText == 'undefined') {
    if (typeof HTMLElement == 'undefined' || !HTMLElement.prototype || !HTMLElement.prototype.__defineGetter__)
        unsupported += ' innerText()'
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
if (!window.showModalDialog)
    unsupported += ' showModalDialog()'

//actually this is only needed if neosysforms are used
if (!document.getElementsByClassName && !document.all)
    unsupported += ' getElementsByClassName or .all'

//check right browser and cookies allowed else switch to login which handled this error
if (unsupported) {
    alert('Sorry, your web browser does not support NEOSYS.\nUse Internet Explorer 6+, Safari 3+, Firefox 3+ or Chrome 8.0+\n\nIt doesnt support' + unsupported)
    //document.location=NEOSYSlocation+'wrongbrowser.htm'
    if (typeof glogin == 'undefined')
        document.location = NEOSYSlocation + '../default.htm'
}

//implement swapNode if not native
if (self.Node) {
    Node.prototype.swapNode = function (n) {
        var p = n.parentNode;
        var s = n.nextSibling;
        this.parentNode.replaceChild(n, this);
        p.insertBefore(this, s);
        return this;
    }
}

//works like msie
function $all(elementid, element) {
    if (!element) {
        var id = elementid.id ? elementid.id : elementid
        /*
        var result
        if (document.querySelectorAll) {
        result = document.querySelectorAll('.neosysid_' + id)
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
        temp = element.getElementsByClassName('neosysid_' + elementid)
    else
        temp = document.getElementsByClassName('neosysid_' + elementid)
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

//setup function $$() NOT $() since that is used by JQuery
if (document.getElementsByClassName)
    $$ = $class
else
    $$ = $all

isMSIE = typeof ActiveXObject != 'undefined'

isMac = navigator.appVersion.indexOf('Macintosh') >= 0

document.protocolcode = document.location.toString().slice(0, 4)

var cr = String.fromCharCode(13)

//global constants for revelation high end separator characters
/*
var fm2='þ'
var vm2='ý'
var sm2='ü'
//regular expressions
var STMre=/\xFA/g
var TMre=/\xFB/g
var SMre=/\xFC/g
var VMre=/\xFD/g
var FMre=/\xFE/g
var RMre=/\xFF/g
var XMLXXre=/([\x25\x3C\x3E\x26\xFF\xFE\xFD\xFC\xFB\xFA\xF9\xF8])/g
//also block character F9 as is used as a field mark in AREV eg PRINT ("XXX":\F9\:"YYY") 'L#20' ... formats both to 20 characters
//also block character F8 since to make logical block of eight field mark characters
var FMs='\xFF\xFE\xFD\xFC\xFB\xFA\xF9\xF8'
rmcharcode=255
*/

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

//UTF8 web page containing all main characters (not CJK)
//internet explorer 7.0 on XP takes a long time to open this and slow paging
//firefox 2.0.0.6 on the same machine has no problem pages much faster and shows main more characters as non-box
//win32 http://www.columbia.edu/kermit/utf8-t1.html

//linux http://dejavu.sourceforge.net/samples/DejaVuSansMono.pdf
//http://www.unicode.org/charts/PDF/U2500.pdf
//http://www.eki.ee/letter/chardata.cgi?ucode=00FA-00FF
//choose symbol that exist in both environments in to order to read
// but ones that we can acceptably say that we cannot handle
//choose ending in hex F to be easy to compare to original 00FF
//could also choose and 2 byte private use unicode code point in the range E000-F8FF
/*
box drawing character show in firefox and explorer test page above but not system boxes unfortunately.
[╘]  U+2558   &amp;#9560;  BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
[╙]  U+2559   &amp;#9561;  BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
[╚]  U+255A   &amp;#9562;  BOX DRAWINGS DOUBLE UP AND RIGHT
[╛]  U+255B   &amp;#9563;  BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
[╜]  U+255C   &amp;#9564;  BOX DRAWINGS UP DOUBLE AND LEFT SINGLE
[╝]  U+255D   &amp;#9565;  BOX DRAWINGS DOUBLE UP AND LEFT
[╞]  U+255E   &amp;#9566;  BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
[╟]  U+255F   &amp;#9567;  BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
*/
//IN THE THAT THE UNICODE CODE POINTS CHOSEN TO REPRESENT FIELD MARKS IN THE FRONT END NEEDS TO BE CHANGES
//YOU ONLY NEED TO CHANGE THEM IN THIS PROJECT client.js (here) and xhttp.asp
//1. change here and in xhttp.asp
//2. search whole project for something like u255F to verify
//3. be aware that ntvdm backend sends and receives field marks characters F8-FF as escaped characters to the asp file
//4. the asp file converts them to the chosen unicode characters here before sending them to the front end HTML.
//5. the process works in reverse when sending data from the front end to (or back to) the backend
//6. be aware that the ntvdm backend converts any real F8-FF characters from the front end to to hex 16-1F to make way for unescaping %F8-FF
//7. ie in the unlikely event that the front end sends any real 16-1F characters to the ntvdm backend they will appear to be converted
// to characters F8-FF on return.

//these should be replaced by fm/vm/sm
var fm2 = '╞'
var vm2 = '╝'
var sm2 = '╜'

//prestored regular expressions for speed
var STMre = /\u255A/g
var TMre = /\u255B/g
var SMre = /\u255C/g
var VMre = /\u255D/g
var FMre = /\u255E/g
var RMre = /\u255F/g

//also block character F8 and F9
//F9 is used as a field mark in AREV eg PRINT ("XXX":\F9\:"YYY") 'L#20' ... formats both to 20 characters
//also block character F8 since to make logical block of eight field mark characters
var XMLXXre = /([\x25\x3C\x3E\x26\u255F\u255E\u255D\u255C\u255B\u255A\u2559\u2558])/g
var FMs = '\u255F\u255E\u255D\u255C\u255B\u255A\u2559\u2558'
rmcharcode = 9567//U+255F ╟ (box drawings vertical double and right single)

//calculate all the field mark variables
var rm = String.fromCharCode(rmcharcode)
var fm = String.fromCharCode(rmcharcode - 1)
var vm = String.fromCharCode(rmcharcode - 2)
var sm = String.fromCharCode(rmcharcode - 3)
var tm = String.fromCharCode(rmcharcode - 4)
var stm = String.fromCharCode(rmcharcode - 5)


var dbcache

var glogincode

//get alternative dialog arguments - see dialogArgumentsForChild in function neosysshowmodaldialog
gDialogArguments = window.dialogArguments
if (!gDialogArguments && window.opener) {
    try {
        //window.opener may have been closed and refreshing the page might cause an error
        gDialogArguments = window.opener.dialogArgumentsForChild
    }
    catch (e)
  { }
}

if (window.dialogArguments && window.dialogArguments.logincode) {
    glogincode = window.dialogArguments.logincode
    //alert('debug client.js glogincode=window.dialogArguments.logincode '+window.dialogArguments.logincode)
}
/* sometimes seems to get the wrong window opener resulting in wrong glogincode
else if (window.opener)
{
try
{
glogincode=window.opener.glogincode
//alert('debug client.js glogincode=window.opener.glogincode '+window.opener.glogincode)
//if (typeof gparameters=='undefined'&&window.opener.gparametersforcallee)
// gparameters=window.opener.gparametersforcallee
}
catch (e){}
}
*/
if (!glogincode) glogincode = ''
if (!glogincode) {
    glogincode = neosysgetcookie('', 'NEOSYSlogincode', 'logincode')
    //alert('debug client.js glogincode=neosysgetcookie(\'\',\'NEOSYSlogincode\',\'logincode\') '+window.opener.glogincode)
    //alert('debug client.js glogincode= '+glogincode)
    //alert('xyz3'+neosysgetcookie(glogincode,'NEOSYS2','username'))
    //alert(document.cookie)
}

var gusername
var gdataset
var gsystem

var gcompanycode
var gperiod
var gbasecurr
var gbasefmt
var gmarketcode
var gmaincurrcode
var gdatasetname
var gtz = [0, 0]
//gtz[0]=new Date().getTimezoneOffset()*-60

var gnpendingscripts = 0

//load gparameters from window.dialogArguments if present
var gparameters
if (typeof gparameters == 'undefined') gparameters = new Object
if (typeof window.dialogArguments != 'undefined') {
    for (var param in window.dialogArguments) gparameters[param] = window.dialogArguments[param]
}
if (gparameters.gtasks && !gtasks)
    gtasks = gparameters.gtasks

//add gparameters from URL if present eg. neosys.com/xyz.htm?param1=1&param2=2 etc.
var temp = window.location.toString()
if (temp.indexOf('?') >= 0) {
    temp = unescape(temp.slice(temp.indexOf('?') + 1)).split('&')
    for (i = 0; i < temp.length; i++) gparameters[temp[i].split('=')[0]] = temp[i].split('=').slice(1).join('=')
}
if (typeof gparameters.savemode == 'undefined') gparameters.savemode = gparameters.SAVEMODE

//file access functions
if (document.protocolcode == 'file') {
    document.writeln('<SCR' + 'IPT type="text/javascript" src="' + NEOSYSlocation + 'scripts/server.js"></SCR' + 'IPT>')
}

//style sheet
//copy any modifications to decide.htm and decide2.htm as well
document.writeln('<link REL="stylesheet" TYPE="text/css" HREF="' + NEOSYSlocation + 'global.css">')
//be careful to always have private.css even if empty otherwise it is ALWAYS not in cache and
//requires a server lookup all the time. therefore upgrades will destroy any neosys client's private.css
//actually it shouldnt be ../ in the login and inital menu screen
//cut out because slows at least modaldialog in ie7
//document.writeln('<link REL="stylesheet" TYPE="text/css" HREF="../private.css">')

//jquery
document.writeln('<SCR' + 'IPT type="text/javascript" id=jquery src="' + NEOSYSlocation + 'scripts/jquery-1.4.4-min.js"></SCR' + 'IPT>')

//general functions
if (!$$('generalfunctions')) {
    document.writeln('<SCR' + 'IPT type="text/javascript" id=generalfunctions src="' + NEOSYSlocation + 'scripts/neosys.js"></SCR' + 'IPT>')
}

//form functions
if (typeof gdatafilename != 'undefined' || typeof gdictfilename != 'undefined') {
    document.writeln('<SCR' + 'IPT type="text/javascript" src="' + NEOSYSlocation + 'scripts/db.js"></SCR' + 'IPT>')
    document.writeln('<SCR' + 'IPT type="text/javascript" src="' + NEOSYSlocation + 'scripts/gds.js"></SCR' + 'IPT>')
    document.writeln('<SCR' + 'IPT type="text/javascript" src="' + NEOSYSlocation + 'scripts/dbform.js"></SCR' + 'IPT>')
}

//if (!window.onload)
//save any custom onload function to be executed by clientfunctions_windowonload when it finishes
gwindowonload = window.onload
window.onload = clientfunctions_windowonload

//save location except if logging in
if (typeof gnosavelocation == 'undefined' && !window.dialogArguments && NEOSYSlocation != './neosys/') {
    //if (gdataset) neosyssetcookie('','NEOSYS',escape(location),'ll',true)
}

glogging = 0
gstepping = 0

//ensure http session is kept alive
if (document.protocolcode != 'file') {
    if (gkeepalivemins) window.setInterval('sessionkeepalive()', gkeepalivemins * 60 * 1000)
}

loadcache()

function neosyslogout_onclick() {

    //cancel any automatic login
    neosyssetcookie('', 'NEOSYS', '', 'a')

    //decide where to login again
    var newwindowlocation = '../default.htm'
    var system = neosysgetcookie('', 'NEOSYSsystem')
    if (system && system != 'ADAGENCY') newwindowlocation += '??' + system

    //clear various session variable
    dblogout()

    //switch to login window 
    try {
        window.location = newwindowlocation
    }
    catch (e) { }

    return false

}

var gmsg
var gtasks
function neosyssecurity(task) {

    //return empty gmsg if authorised
    gmsg = ''

    //look for ancient source code
    if (task.indexOf(' FILE ') >= 0)
        neosyswarning('FILE should not be in task ' + task)

    //make sure task list is loaded (clearing cache also clears gtasks for convenience)
    if (!gtasks) {
        db.request = 'EXECUTE\rGENERAL\rGETTASKS\rNOT'
        if (!db.send()) {
            gtasks = ''
            gmsg = db.response
            neosysinvalid(gmsg)
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
    //if (task.slice(0,3)=='!!!'||gtasks.neosyslocate(task)||(gstepping&&!confirm(task)))
    if (task.slice(0, 3) == '!!!' || gtasks.neosyslocate(task)) {
        gmsg = 'Sorry ' + gusername.neosyscapitalise() + ', you are not authorised to do\r' + task.neosyscapitalise()
        return false
    }

    return true

}

function sessionkeepalive() {

    //last connection
    var lastconnection = neosysgetcookie('', 'NEOSYSlc', 'lc')
    if (lastconnection == 'undefined') lastconnection = ''
    lc = lastconnection
    lastconnection = lastconnection ? new Date(lastconnection) : lastconnection = new Date(0)

    //next connection
    var nextconnection = lastconnection
    nextconnection.setMilliseconds(lastconnection.getMilliseconds() + (gkeepalivemins * 60 * 1000))

    //call server if time to keep alive
    var time = new Date
    if (time >= nextconnection && typeof db != 'undefined') {
        var tempdb = new neosysdblink()
        tempdb.request = 'KEEPALIVE'
        tempdb.send()
        //if (!db.send()) alert(db.response)
        window.status = time + ' Keep Alive'
    }

}

function neosyssetexpression(elementsorelementid, attributename, expression) {

    //check element exists
    if (!elementsorelementid)
        return neosysinvalid('missing element in neosyssetexpression ' + attributename + ' ' + expression)

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
            neosyssetexpression(elements[ii], attributename, expression)
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

function neosysenabledandvisible(element) {
    if (typeof element == 'string') element = document.getElementsByName(element)[0]
    if (element.getAttribute('neosysreadonly'))
        return false
    while (element && element.parentNode && element.tagName != 'BODY') {
        if (element.style.display == 'none')
            return false
        //if (element.runtimeStyle.display=='none') return false
        if (element.getAttribute('disabled')) return false
        element = element.parentNode
    }
    return true
}

function showhide(element, show) {

    var elementid = element

    if (typeof element == 'string') {
        element = $$(element)
        if (!element)
            return neosysinvalid('element ' + elementid + ' does not exist in showhide()')
    }

    //recursive
    //if (typeof element=='object'&&!element.tagName)
    if ((typeof element == 'object' || typeof element == 'function') && !element.tagName) {
        for (var ii = 0; ii < element.length; ii++)
            showhide(element[ii], show)
        return
    }

    if (element.style) {
        if (show) {
            //inline doesnt line up columns in mozilla and table-row is not accepted by msie5/6/7
            if (document.all)
                displayvalue = 'inline'
            else if (element.tagName == 'TR')
                displayvalue = 'table-row'
            else if (element.tagName == 'TD' || element.tagName == 'TH')
                displayvalue = 'table-cell'
            else if (element.tagName == 'TBODY')
                displayvalue = 'table-row-group'
            else
                displayvalue = 'inline'
        }
        else
            displayvalue = 'none'

        element.style.display = displayvalue
    }

}

function insertafter(element, newelement) {
    if (element.nextSibling)
        return element.parentNode.insertBefore(newelement, element.nextSibling)
    return element.parentNode.insertBefore(newelement, null)
}

function setinnerHTML(elementsorid, html) {
    if (typeof elementsorid == 'string')
        elementsorid = document.getElementsByName(elementsorid)
    for (var i = 0; i < elementsorid.length; i++)
        elementsorid[i].innerHTML = html
}

function neosysshowmodaldialog(url, arguments, dialogstyle) {

    //http://developer.mozilla.org/en/docs/DOM:window.showModalDialog
    //http://msdn.microsoft.com/en-us/library/ms536759.aspx

    if (!arguments)
        var arguments = new Object
    if (!arguments.gtasks)
        arguments.gtasks = gtasks
    //http://localhost/neosys2/jobs/timesheets.htm
    var standardstyle = 'Center: yes; Help: no; Resizable: yes; Scroll: yes; Status: no'
    var maxheight = window.screen.availHeight
    var maxwidth = window.screen.availWidth
    if (!dialogstyle) {
        //var maxheight=window.screen.availHeight
        //var maxwidth=window.screen.availWidth
        //var dialogstyle='DialogHeight:'+maxheight+'px; DialogWidth:'+maxwidth+'px; Center: yes; Help: yes; Resizable: yes; Status: No;'
        ////var dialogstyle='dialogHeight: 400px; dialogWidth: 600px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'
        dialogstyle = standardstyle
        //prevents centering in ff3
        //dialogstyle+='; DialogHeight: 100px; DialogWidth: 500px'
        //manual centering (TODO calculate it from maxheight and maxwidth)
        //dialogstyle+='; DialogTop: 100px; DialogLeft: 100px'
    }
    else if (dialogstyle == 'max') {
        dialogstyle = standardstyle + '; DialogHeight:' + maxheight + 'px; DialogWidth:' + maxwidth + 'px;'
    }

    //always send login code
    arguments.logincode = glogincode
    try {

        //provide an alternative location for the child dialogWindow to get dialogArguments
        //gDialogArguments=window.opener.dialogArgumentsForChild
        //to avoid bug? in firefox where dialogArguments is always set to null when calling a modal dialog immediately on opening firefox
        //currently only used in default.htm when being used as a modal dialog to do a login on the fly
        //eg when accessing pages via favourites without going through default.htm first

        dialogArgumentsForChild = arguments

        var result = window.showModalDialog(url, arguments, dialogstyle)

        //Safari doesnt return an error and looks like a Window [x] close unfortunately
        if (typeof result == 'undefined' && window.navigator.appVersion.indexOf('Safari') >= 0)
            throw (url)

        return result

    }
    catch (e) {
        //alert('Please enable popups for this site (1)\n\nError:'+(e.description?e.description:e))
        //alert('Please enable popups for this site (1)\n\nError:'+(e.description?e.description:e)+'\n\n'+url+'\n\n'+arguments)
        return
    }

    //return nothing if cannot popup
    return

}

function displayresponsedata(request, data) {

    if (!data)
        data = ''
    db.request = request
    if (!db.send(data)) {
        neosysinvalid(db.response)
        return false
    }

    neosysnote(db.data)
    return true

}

//function to simplify passing a db request (with optional data)
//that returns a URL to be opened in an independent window
function openwindow(request, data) {

    if (!data)
        data = ''
    db.request = request
    if (!db.send(data)) {
        neosysinvalid(db.response)
        return false
    }

    if (db.response != 'OK')
        neosysnote(db.response.slice(3))

    if (db.data)
        return windowopen(db.data)

}

function windowopen2(url) {
    return windowopen(url, { 'key': gvalue })
}

var gwindowopenparameters
function windowopen(url, parameters) {

    if (!url) url = ''
    if (!parameters) parameters = {}

    //switch to normal url from development url which doesnt have data folder (otherwise too many files)
    if (window.location.href.toString().toLowerCase().indexOf('//localhost/neosys7/') >= 0 && url.toLowerCase().slice(0, 8) == '..\\data\\')
        url = window.location.href.toString().split('/').slice(0, 3).join('/') + '/neosys/' + url.slice(3)

    //these parameters are picked up by the opened window (from its parent) after it opens
    //since no way to pass parameters directly to non modal windows except in the URL
    //and even then the this causes different urls perhaps defeating http caching
    gwindowopenparameters = parameters
    gwindowopenparameters.logincode = glogincode
    try {

        var result = window.open(url)

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
function neosysbreak(cmd, funcname, position) {

    if (!gstepping) return

    window.status = funcname + ' ' + position + ' ' + cmd

    if (!cmd) cmd = ' '

    var msg = ''
    while (cmd) {
        if (cmd != ' ') {

            try {
                var result = this.parent.eval(cmd)
            }
            catch (e) {
                result = e.number + ' ' + e.description
            }
            msg = cmd + '=\n' + result + '\n\n' + msg

            //display result and optionally quit
            if (!confirm(msg)) {
                //FF3 about:config
                //extensions.firebug-service.breakOnErrors
                breakrighthere
                return
            }

        }
        cmd = prompt(funcname + ' ' + position, cmd)
    }
}

function assertelement(element, funcname, varname) {
    neosysassertobject(element, funcname, varname)
    if (!element.tagName) {
        raiseerror(1000, 'In ' + funcname + ', ' + varname + ' is not an element.')
        return
    }
    return true
}

function neosysnote(msg, mode) {
    //if (!msg) return false
    //allow return neosysnote() to be opposite of return neosysinvalid()
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

    neosysyesnocancel(msg, 1, 'OK', '', '', null, false, mode)

    return true

}

//''''''''
//'INVALID
//''''''''
function neosysinvalid(msg) {
    //displays a message if provided and returns false
    //so you can use it like "return neosysinvalid(msg)" to save a line
    neosysnote(msg, 'critical')
    return false
}

//''''''''
//'WARNING
//''''''''
function neosyswarning(msg) {
    //displays a message if provided and returns true
    //so you can use it like "return neosyswarning(msg)" to save a line 
    return neosysnote(msg, 'warning')
}

//duplicated in client.js, decide.htm and decide2.htm
function neosys_set_style(mode, value, value2) {

    if (value.toUpperCase() == 'DEFAULT') value = ''

    //restore original value
    if (!value && goriginalstyles[mode]) value = goriginalstyles[mode]

    //ensure display is set to inline even if not changing color
    //if (!value) return

    var rules = document.styleSheets[0].cssRules
    if (!rules)
        rules = document.styleSheets[0].rules
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
        style.display = 'inline-block'

        //initial color is buff yellow
        if (!value)
            value = '#ffffc0'

        oldvalue = style.backgroundColor
        try {
            style.backgroundColor = value
        }
        catch (e) {
            if (e.number == -2146827908) return neosysinvalid(value + ' is not a recognised color')
            return systemerror('neosys_set_style("' + mode + '","' + value + '")', e.number + ' ' + e.description)
        }
    }

    //screenfont
    else if (mode == 'screenfont' && rules) {

        //initial font is ... 8pt
        var basefontsize = 8

        if (!value) value = 'verdana,arial,helvetica'
        if (!value2) value2 = 100
        if (!Number(value2)) {
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
                if (e.number == -2146827908) return neosysinvalid(value + ' is not a recognised font')
                if (e.number == -2147024809) return neosysinvalid(value2 + ' is not a recognised fontsize')
                return systemerror('neosys_set_style("' + mode + '","' + value + '","' + value2 + '")', e.number + ' ' + e.description)
            }
        }
    }

    //save the original style
    if (!goriginalstyles[mode] && oldvalue) goriginalstyles[mode] = oldvalue

}

//called early in decide and decide2
function clientfunctions_setstyle() {
    //set font first since setting color changes style display from none to inline
    neosys_set_style('screenfont', neosysgetcookie(glogincode, 'NEOSYS2', 'ff'), neosysgetcookie(glogincode, 'NEOSYS2', 'fs'))
    neosys_set_style('screencolor', neosysgetcookie(glogincode, 'NEOSYS2', 'fc'))
}

function clientfunctions_getglobals() {
    gcompanycode = neosysgetcookie(glogincode, 'NEOSYS2', 'cc')
    gncompanies = neosysgetcookie(glogincode, 'NEOSYS2', 'nc')
    gperiod = neosysgetcookie(glogincode, 'NEOSYS2', 'pd')
    gbasecurr = neosysgetcookie(glogincode, 'NEOSYS2', 'bc')
    gshowzeros = neosysgetcookie(glogincode, 'NEOSYS2', 'bc')
    gbasefmt = neosysgetcookie(glogincode, 'NEOSYS2', 'bf')
    gmarketcode = neosysgetcookie(glogincode, 'NEOSYS2', 'mk')
    gmaincurrcode = neosysgetcookie(glogincode, 'NEOSYS2', 'mc')
    gdatasetname = neosysgetcookie(glogincode, 'NEOSYS2', 'db')
    gmenucodes = neosysgetcookie(glogincode, 'NEOSYS2', 'm')
    gtz = neosysgetcookie(glogincode, 'NEOSYS2', 'tz').split(fm)
    if (gtz) {
        gtz[0] = Number(gtz[0])
        if (gtz[1])
            gtz[1] = Number(gtz[1])
        else
            gtz[1] = 0
    }
    else
        gtz = [0, 0]
}

function clientfunctions_windowonload() {

    login('clientfunctions_windowonload')

    if (document.getElementsByClassName)
        id2classname()

    if (!glogincode) glogincode = neosysgetcookie('', 'NEOSYSlogincode', 'logincode')
    //loginalert('wol'+glogincode)
    gdataset = neosysgetcookie(glogincode, 'NEOSYS2', 'dataset')

    clientfunctions_getglobals()

    db = new neosysdblink
    //gusername is set in neosysdblink

    clientfunctions_setstyle()
    //trigger formfunctions_onload
    if (typeof formfunctions_onload == 'function')
        formfunctions_onload()

    if (document.getElementById('autofitelement')) {
        window.setTimeout('neosysautofitwindow()', 0)
    }

    if (!window.dialogArguments && (typeof gshowmenu == 'undefined' || gshowmenu) && NEOSYSlocation != './neosys/') {

        var ctrlalt = isMac ? 'Ctrl' : 'Alt'

        //button to refresh (clear cache)
        var temp2 = document.createElement('DIV')
        temp2.innerHTML = menubuttonhtml('refreshcache', grefreshimage, 'Refresh', 'Refresh the Cache. ' + ctrlalt + '+X', 'X')
        document.body.insertBefore(temp2, document.body.firstChild)

        //button to logout
        var temp2 = document.createElement('DIV')
        temp2.innerHTML = menubuttonhtml('neosyslogout', glogoutimage, '<u>L</u>ogout', 'Logout. ' + ctrlalt + '+L', 'L')
        document.body.insertBefore(temp2, document.body.firstChild)
        if (!gusername) {
            var temp = $$('neosyslogoutbutton')
            temp.innerText = 'Login'
            temp.title = 'Login. Alt+L'
        }

        //button for menu
        if (gmenucodes && gmenucodes != 'EXIT2') {
            var temp2 = document.createElement('DIV')
            temp2.innerHTML = menubuttonhtml('menu', gmenuimage, '<u>M</u>enu', 'Menu. ' + ctrlalt + '+M', 'M')
            document.body.insertBefore(temp2, document.body.firstChild)

            //div to retrieve menu structure
            temp = document.createElement('DIV')
            temp.id = 'menux'
            temp.style.display = 'none'
            temp.innerHTML = '<iframe id=menuframe ie55onload="menuonload()" src="../menu.htm" tabindex=-1 height=1px marginheight=1 marginwidth=1 frameborder=0 scrolling=no></iframe>'
            var menuelement = $$('menubutton')
            menuelement.insertBefore(temp, null)

            //dont rely on onload event ... poll every 100ms to see if iframe is loaded
            gmenuonloader = window.setInterval('menuonload()', 100)
        }

    }

    //execute any custom window onload function
    //avoid error "Can't execute code from a freed script"
    if (gwindowonload)
        try { gwindowonload() }
        catch (e) { }

    logout('clientfunctions_windowonload')

    return

}

//var mmm=0
gmenuloaded = false
function menuonload() {

    //$$('menuframe').contentDocument.getElementById('menucompleted')
    var iframe = $$('menuframe')
    idocument = iframe.contentDocument
    if (idocument) {
        if (!idocument.getElementById('menucompleted'))
            return

        temp = idocument.body.innerHTML
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
        var temp = ''
        var childNodes = menuframe.document.body.childNodes
        for (var i = 0; i < childNodes.length; i++) {
            if (childNodes[i].outerHTML && childNodes[i].id != 'menux' && childNodes[i].id != 'menuframe')
                temp += childNodes[i].outerHTML
        }

    }

    login('menuonload')

    window.clearInterval(gmenuonloader)

    //menux.zIndex=999 //done in stylesheet but does not work over select elements (see MS docs)

    gmenuloaded = true

    //store the menu in the menu button
    $$('menux').innerHTML = temp
    //menux.innerHTML=temp
    var menudiv = $$('menudiv')
    var target = menudiv.parentNode.parentNode
    insertafter(target, menudiv)

    //enable the menu button events
    var menubuttonx = $$('menubutton')
    menubuttonx.onclick = menuonclick
    menubuttonx.onmouseover = menuonmouseover
    menubuttonx.onmouseoutmenuonmouseout
    addeventlistener(menubuttonx, 'onclick', menuonclick)
    addeventlistener(menubuttonx, 'onmouseover', menuonmouseover)
    addeventlistener(menubuttonx, 'onmouseout', menuonmouseout)

    logout('menuonload')

}

Array.prototype.neosysread = function neosysread(filename, key, fieldno, cache) {

    //unless returning one fieldno, always return at least n fields
    //so that accessing fields that do not exist by [] returns ''
    var minnfields = 100

    this.neosysresponse = neosysquote(key) + ' does not exist in the ' + filename.toLowerCase() + ' file.'
    if (key == '') return false

    if (typeof cache == 'undefined' || cache)
        db.request = 'CACHE\r'
    else
        db.request = ''
    db.request += 'READ\r' + filename + '\r' + key
    if (!db.send()) {
        if (db.response.indexOf('NO RECORD') >= 0) {
            //var temp=filename.toLowerCase().neosyssingular().replace(/_/,' ')
            return false
        }
        else {
            if (db.response.indexOf('file is not available') >= 0) systemerror('neosysread', db.response)
            this.neosysresponse = db.response
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
Array.prototype.neosysxlate = function arrayxlate(filename, fieldno, mode) {

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
            results[keyn] = neosysxlatelogic(filename, (key + fm + rec).split(fm), fieldno, mode, key)
        }

        //or build a unique list of keys of records to be selected
        else {
            if (!uncachedkeys.neosyslocate(key)) uncachedkeys[uncachedkeys.length] = key
        }

    }

    //select all the uncached records
    if (uncachedkeys.length) {

        //WARNING !!!
        //unfortunately SELECT\r\r\rRECORD returns raw record without
        //the usual postread processing and it puts this into the cache for
        //later processing as well (perhaps it should not put it in the cache)
        //it is difficult to call the usual postread processing from SELECT2
        //on the server since postreads require AREV.COMMON (OREC etc)
        //perhaps one should have a special POSTREADX which only touches @record

        //select the (deduplicated) records or return systemerror
        db.request = 'SELECT\r' + filename + '\r\rRECORD'
        if (!db.send(uncachedkeys.join(fm))) {
            systemerror(db.response)
            this.neosysresponse = db.response
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
            var result = neosysxlatelogic(filename, keyrec, fieldno, mode, key)

            //store the results whereever they are needed
            var keyn = 0
            while (keyn = keys.neosyslocate(key, '', keyn + 1)) {
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
String.prototype.neosysxlate = function stringxlate(filename, fieldno, mode) {

    key = this.toString()
    if (key == '') return ''

    neosysassertnumeric(fieldno, 'xlate', filename + ' ' + key)

    var record = []
    record.neosysread(filename, this)
    if (db.response.indexOf('file is not available') >= 0) systemerror('xlate', db.response)

    return neosysxlatelogic(filename, record, fieldno, mode, key)

}

function neosysxlatelogic(filename, record, fieldno, mode, key) {
    if (record.length) {
        if (typeof fieldno != 'undefined') {
            if (fieldno) {
                record = record[fieldno]
                if (typeof record == 'undefined') record = ''
            }
        }
        //option to sum the result (good for adding up multivalues)
        if (mode && mode == 'SUM') {
            record = record.neosyssum()
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

function neosysfilepopup(filename, cols, coln, sortselectionclause, many) {
    //filename is required
    //cols is required (array of arrays)
    //eg [['COMPANY_NAME','Company Name'],['COMPANY_CODE','Company Code']]

    //sortselectionclause is optional
    //eg 'WITH TYPE1 = "X" AND WITH = TYPE2 "Y" BY TYPE1 BY NAME'
    if (!sortselectionclause) sortselectionclause = ''

    //sortselectionclause can be an array of keys
    if (typeof sortselectionclause == 'object') sortselectionclause = sortselectionclause.join('" "').neosysquote()

    //get a list of col names sep by spaces
    var collist = ''
    for (var i = 0; i < cols.length; i++) {
        collist += ' ' + cols[i][0]
    }
    collist = collist.slice(1)
    if (!collist.split(' ').neosyslocate('ID')) collist += ' ID'

    maxnrecs = 1000

    //get the data from the server
    db.request = 'CACHE\rSELECT\r' + filename.toUpperCase() + '\r' + sortselectionclause + '\r' + collist + '\rXML\r' + maxnrecs
    //db.request='CACHE\rSELECT\r'+filename.toUpperCase()+'\r'+sortselectionclause+'\r'+collist+' ID'
    if (!db.send()) {
        neosysinvalid(db.response)
        return null
    }

    //cancel if no records found
    if (db.data.indexOf('<RECORD>') < 0) {
        neosysinvalid('Sorry, no records found')
        return null
    }

    return neosysdecide2('', db.data, cols, coln, '', many)

}

//////////////////////
// global functions //
//////////////////////

//similar function in xhttp.asp
function neosysdblink() {

    this.request = ''
    this.data = ''
    this.response = ''
    this.documentprotocolcode = document.protocolcode
    this.login = neosysdblink_login

    //used in cache and elsewhere
    if (!gdataset) gdataset = neosysgetcookie(glogincode, 'NEOSYS2', 'dataset')
    gusername = neosysgetcookie(glogincode, 'NEOSYS2', 'username')
    //alert('xyz2 NEOSYS2/username='+neosysgetcookie(glogincode,'NEOSYS2','username'))
    //alert(document.cookie)
    gsystem = neosysgetcookie('', 'NEOSYSsystem')
    // alert(gdataset+'*'+gusername+'*'+gsystem)
    this.dataset = gdataset
    this.system = gsystem
    this.username = gusername

    //setup environment for http messaging
    if (this.documentprotocolcode != 'file') {

        //netscape or IE7 xmlhttp
        try {
            this.XMLHTTP = new XMLHttpRequest()
            this.send = neosysdblink_send_byhttp_using_xmlhttp
            return
        }
        catch (e) { }

        //IE6 xmlhttp
        try {
            this.XMLHTTP = new ActiveXObject('Microsoft.XMLHTTP')
            this.send = neosysdblink_send_byhttp_using_xmlhttp
        }

        //asp forms
        catch (e) {
            this.send = neosysdblink_send_byhttp_using_forms
        }

        return

    }

    //otherwise setup environment for file messaging

    //try and get the username, password and dataset
    this.password = neosysgetcookie(glogincode, 'NEOSYS2', 'password')
    this.timeout = neosysgetcookie(glogincode, 'NEOSYS2', 'timeout')

    //default timeout is 10 minutes (NB GIVEWAY timeout is hard coded to 10 mins?)
    var defaulttimeoutmins = 10
    //var defaulttimeoutmins=.25
    if (!this.timeout) this.timeout = defaulttimeoutmins * 60 * 1000


    this.send = neosysdblink_send_byfile
    this.start = neosysdblink_startdb

    try {
        gfso = new ActiveXObject('Scripting.FileSystemObject')
    }
    catch (e) {
        alert('Error: While creating Scripting.FileSystemObject\n' + e.number + ' ' + e.description)
        window.location = NEOSYSlocation + 'securityhowto.htm'
        return
    }

    this.neosysrootpath = getneosysrootpath(document.location.toString())

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

function dblogout() {

    //remove username etc
    //neosyssetcookie('','NEOSYS',gdataset,'dataset',true)

    //remove last page
    //neosyssetcookie('','NEOSYS','','ll',true)

    neosyssetcookie(glogincode, 'NEOSYS2', '', 'username')
    //alert('xyz NEOSYS2/username='+neosysgetcookie(glogincode,'NEOSYS2','username'))

    //remove settings
    //alert('before clear '+document.cookie)
    neosyssetcookie(glogincode, 'NEOSYS2', '')
    neosyssetcookie(glogincode, 'NEOSYS', '')
    //alert('after clear '+document.cookie)

}

function neosysdblink_login(username, password, dataset, system) {

    //get list of datasets from server
    var logindb = new neosysdblink
    var datasets = ''

    if (!dataset && gdataset) dataset = gdataset
    if (!dataset) dataset = neosysgetcookie('', 'NEOSYS', 'dataset')
    if (!system) system = neosysgetcookie('', 'NEOSYSsystem')
    var arguments = ['', '', dataset, '', '', '', system]
    var failed = false

    while (true) {

        arguments[4] = datasets
        //if (!(typeof event!='undefined'&&event.shiftKey)&&!failed&&neosysgetcookie('','NEOSYS','a')=='true')
        if (!(typeof event != 'undefined' && event && event.shiftKey) && !failed && (neosysgetcookie('', 'NEOSYS', 'a') == 'true' || username)) {
            arguments[0] = username ? username : neosysgetcookie('', 'NEOSYS', 'u')
            arguments[1] = password ? password : neosysgetcookie('', 'NEOSYS', 'p')
            arguments[2] = dataset
            arguments[5] = neosysgetcookie('', 'NEOSYS', 'a')
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
                if (!logindb.send()) {
                    neosysinvalid(logindb.response)
                    return 0
                }
                datasets = neosysxml2obj(logindb.data)
            }

            arguments[4] = datasets

            url = '../default.htm'
            //alert(arguments)
            arguments = neosysshowmodaldialog(url, arguments)

        }

        //quit if user cancels
        if (!arguments) return 0

        glogincode = arguments[2] + '*' + arguments[0] + '*'
        neosyssetcookie('', 'NEOSYSlogincode', glogincode, 'logincode')

        logindb.request = 'LOGIN\r' + arguments[0] + '\r' + arguments[1] + '\r' + arguments[2] + '\r' + arguments[3] + '\r\r' + arguments[5]

        if (!logindb.send()) {
            var msg = logindb.response
            if (!msg) msg = 'Invalid username or password'
            neosysinvalid(msg)
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
            neosyssetcookie('', 'NEOSYS', temp, '')

            //temporary cookie for menu
            neosyssetcookie(glogincode, 'NEOSYS2', logindb.data)

            clientfunctions_getglobals()

            //temporary cookie for the dataset and username (and password for file protocol)   
            var temp = 'dataset=' + gdataset + '&username=' + gusername + '&system=' + gsystem
            if (document.protocolcode == 'file') {
                this.password = arguments[1]
                temp += '&password=' + this.password
                //this.timeout=neosysgetcookie(glogincode,'NEOSYS2','timeout')
            }
            neosyssetcookie(glogincode, 'NEOSYS2', temp)

            //quit success
            return 1

        }
    }
}

function neosysdblink_send_byhttp_using_forms(data) {

    log(this.request)
    //alert('neosysdblink_send_byhttp_using_forms\n...\n'+this.request+'\n...\n'+ data)
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
        var reply = neosysshowmodaldialog(NEOSYSlocation + 'rs/default.htm', [this.timeout, this.request, this.data])
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

        if (this.response.indexOf('Please login') >= 0) {
            if (!this.login()) {
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

function neosysdblink_send_byhttp_using_xmlhttp(data) {

    //log(this.request)

    var ignoreresult = (typeof this.request == 'string') && (this.request.slice(0, 6) == 'RELOCK' || this.request.slice(0, 9) == 'KEEPALIVE')
    //indicate to refresher when last activity was
    if (ignoreresult)
        neosyssetcookie('', 'NEOSYSlc', new Date, 'lc')

    //prevent reuse
    if (this.requesting) {
        this.data = ''
        this.response = ('ERROR: ALREADY REQUESTING\r' + this.requesting + '\rxxxxxxxxxxx\r' + this.request)
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
        //if (gusername=='NEOSYS') alert(xtext)
        /*
        //decide microsoft or moz/netscape 
        var moz=false
        try
        {
        var xhttp = new ActiveXObject('Microsoft.XMLHTTP')
        //var xhttp = new ActiveXObject('Msxml2.XMLHTTP.3.0')
        }
        catch(e)
        {
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

        /* moved up to before xtext creation
        to cater for fact that username (part of glogincode and token) may not be known at point of requesting
        while (!gotresponse)
        {

        if (request2.slice(0,7)!='RELOCK\n')
        {
        dbwaitingwindow=dbwaiting()
        }
        */

        var xhttp = this.XMLHTTP
        //var servererrormsg='ERROR: Could not communicate with server. Try again later\r\r'
        var servererrormsg = 'Network or server failure. (No response)\r\rPlease try again later\rand/or get technical assistance\r\r'

        //open

        //send the xmldoc to the server and get the response
        try {
            //false for synchronous (wait for response before continuing to next statement)
            xhttp.open('POST', NEOSYSlocation + 'scripts/xhttp.asp', async = ignoreresult)
            //this was never required but inserted in the hope that it will
            //avoid unknown problems. Perhaps it is not necessary for active pages like .asp.
            xhttp.setRequestHeader("Pragma", "no-cache");
            xhttp.setRequestHeader("Cache-control", "no-cache");
            //xhttp.setRequestHeader('Content-type','application/x-www-form-urlencoded');
            //consider also putting the following in asp web pages
            //<% Response.CacheControl = "no-cache" %>
            //<% Response.AddHeader "Pragma", "no-cache" %>
            //<% Response.Expires = -1 %>
        }
        catch (e) {
            this.data = ''
            this.response = servererrormsg + e.number + ' ' + e.description + ' in .open()'
            this.result = ''
            dbready(dbwaitingwindow)
            this.requesting = false
            return 0
        }

        //prevent error "object does not support property or method '.loadXML'" in MSIE 10 plus
        //fails in IE6?
        try {
            xhttp.responseType='msxml-document'
        } catch (e) {}
        
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

            log(action + ' ' + (new Date - timesent) + 'ms')
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

        dbready(dbwaitingwindow)

        //response

        //check for valid response
        if (xhttp.responseXML.firstChild != null) {

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
            }
            else {
                //iexplorer
                this.data = unescape(responsex.childNodes[0].text)
                this.response = unescape(responsex.childNodes[1].text)
                var result = unescape(responsex.childNodes[2].text)
            }

            if (this.response.indexOf('Please login') >= 0) {

                if (!this.login()) {
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

            //system failure, retry/abort?
            var errormsg = 'System failure. Do you want to retry?'
            if (xhttp.responseText.indexOf('ASP 0113') > -1)
                errormsg += '\n\nThe server failed to get a response from the NEOSYS database engine'
            errormsg += '\n\n' + striptags(xhttp.responseText)
            if (!confirm(errormsg)) {
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
    neosyssetcookie('', 'NEOSYSlc', new Date, 'lc')

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
    var windowx = ''//window.showModelessDialog(NEOSYSlocation+'wait.htm','',dialogstyle)
    wstatus('Networking, please wait ...')
    return windowx
}

function dbready(windowx) {

    document.body.style.cursor = 'auto'
    wstatus('')
    if (windowx) try { windowx.close() } catch (e) { }
}

//fix a bug/feature in internet explorer where closing a window opened with window.open causes loss of all non permanent cookies
function neosysfixcookie() {
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

function neosyssetcookie(loginsessionid, name, value, subkey, permanent) {

    //neosysfixcookie()

    //for any particular permanent cookie it must consistently be set true otherwise possible loss of following cookies including ASPSESSION

    /*
    var cookie0='set cookie session:'+loginsessionid+' name:'+name+' subkey:'+subkey+' value:'+value+'\r'
    alert(cookie0)
    var tt=document.cookie.split('; ')
    for (var ii=0;ii<tt.length;ii++) cookie0+='\r'+ii+' '+tt[ii]
    if (permanent&&!confirm(cookie0)) xyx
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
                neosyssetcookie(loginsessionid, name, valuex.slice(1).join('='), valuex[0], permanent)
            }
        }
        return
    }

    if (subkey) {
        // crumbs are separated by ampersands
        var crumbs = neosysgetcookie(loginsessionid, name).split('&')
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

    //document.cookie='NEOSYSsystem=ACCOUNTS'//; expires=Fri 31 Dec 2100 23:59:59 GMT; path=/'
    //value+=';expires=Fri 31 Dec 2000 23:59:59 GMT;'
    var cookie = loginsessionid + name + '=' + escape(value)
    //if (permanent==true)
    if (value.length)
    //x = 1
        cookie += '; expires=Fri, 31 Dec 2100 23:59:59 GMT'
    else
        cookie += '; expires=Fri, 31 Dec 2000 23:59:59 GMT'
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

    //if (name=='NEOSYS2')
    // alert(cookie+'\r\r'+document.cookie)

}

// Retrieve the value of the cookie with the specified name
//duplicated in client.js, decide.htm and decide2.htm
function neosysgetcookie(loginsessionid, key, subkey) {

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
                return cookies[i].split('=').slice(1).join('=')
            }

            // crumbs are separated by ampersands
            var crumbs = cookies[i].substr(fullkey.length + 1).split('&')
            for (var i2 = 0; i2 < crumbs.length; i2++) {
                if (crumbs[i2].split('=')[0] == subkey) {
                    //return unescape(crumbs[i2].split('=')[1])
                    return unescape(crumbs[i2].split('=').slice(1))
                }
            }
        }
    }
    return ''
}

//from "client.js" may also be copied in some "client.js" less windows
function neosysdecide(question, data, cols, returncoln, defaultreply, many, inverted) {
    //data and cols are [[]] or [] or revstr or a;1:b;2 string
    //data cells .text property will be used if present
    //returncoln '' means return row number(s) - 1 based
    //returncoln means column number or property name in data
    // (data columns are usually numeric and 0 based)
    //defaultreply=if returncoln then cell(s) of returncoln otherwise 1 based rown(s)

    //the data might be a db request prefixed with '@'
    if (typeof data == 'string' && data.slice(0, 1) == '@') {
        db.request = data.slice(1)
        if (!db.send()) {
            neosysinvalid(db.response)
            return null
        }
        data = db.data

    }

    //abort if no records found
    if (data == '' || data == '<records>\r\n</records>') return neosysinvalid('No records found')

    if (defaultreply) {
        if (typeof defaultreply == 'string') defaultreply = defaultreply.split(':')
        if (typeof defaultreply != 'object') defaultreply = [defaultreply]
    }

    if (!inverted) inverted = false
    var dialogargs = neosyscloneobj([question, data, cols, returncoln, defaultreply, many, inverted])
    dialogargs.logincode = glogincode

    //var dialogstyle='dialogHeight: 400px; dialogWidth: 600px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'

    var results = neosysshowmodaldialog(NEOSYSlocation + 'decide.htm', dialogargs)
    if (typeof results == 'undefined')
        results = ''

    return rearray(results)

}

function rearray(array) {
    //fix a bug where arrays returned from modaldialogs are missing the method .join()

    if (!array || typeof array != 'object') return array

    var rearray = []
    for (var i = 0; i < array.length; i++) rearray[i] = array[i]

    return rearray

}

function neosysdecide2(question, data, cols, returncoln, defaultreply, many) {

    //called from neosysfilepopup() and a few other places

    //data is xml string
    //cols is array of [title,dictid]

    //the data might be a db request prefixed with '@'
    if (typeof data == 'string' && data.slice(0, 1) == '@') {
        db.request = data.slice(1)
        if (!db.send()) {
            neosysinvalid(db.response)
            return null
        }
        data = db.data

    }

    //abort if no records found
    if (data == '' || data == '<records>\r\n</records>') return neosysinvalid('No records found')

    var dialogargs = neosyscloneobj([question, data, cols, returncoln, defaultreply, many])
    //var dialogstyle='dialogHeight: 400px; dialogWidth: 600px; dialogTop: px; dialogLeft: px; center: Yes; help: Yes; resizable: Yes; status: Yes;'

    dialogargs.logincode = glogincode

    var results = neosysshowmodaldialog(NEOSYSlocation + 'decide2.htm', dialogargs)
    if (!results)
        results = ''

    return rearray(results)

}

function setdropdown2(element, dataobj, colnames, selectedvalues, requiredvalues) {

    //1st element is automatically selected unless selectedvalues overrides it.

    if (!checkisdropdown(element)) return 0

    cleardropdown(element)

    //if (no(xmldata.text)) return(0)
    if (no(dataobj.group1)) return 0

    cleardropdown(element, true)

    var valuecolname
    var textcolname

    if (typeof (colnames) == 'string') {
        textcolname = colnames
        valuecolname = colnames
    }
    else {
        valuecolname = colnames[0]
        textcolname = colnames[1]
    }

    //var records=xmldata.firstChild.childNodes
    var records = dataobj.group1

    //var options=[]

    for (var i = 0; i < records.length; i++) {
        var cell = records[i][valuecolname]
        if (typeof (cell) == 'undefined') {
            neosysinvalid('Error: "' + valuecolname + '" not in data line ' + i + ' for setdropdown2 for "' + element.id + '" (1)')
            return (0)
        }

        var value = cell.text

        if (no(requiredvalues) || requiredvalues.neosyslocate(value)) {
            var option1 = document.createElement('OPTION')
            option1.value = value

            var cell = records[i][textcolname]
            if (typeof (cell) == 'undefined') {
                neosysinvalid('Error: "' + textcolname + '" not in data line ' + i + ' for setdropdown2 for "' + element.id + '" (2)')
                return (0)
            }

            //start a new option        
            //var option='<OPTION value='+value

            //indicate if selected
            var selected = false
            if (selectedvalues && selectedvalues.neosyslocate(value)) {
                //option+=' selected=true'
                option1.selected = true
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

    //select first option if none selected
    // if (element.selectedIndex==-1) element.selectedIndex=0

    //force element to recalculate width (needed on msie55 but not mac ie5)
    //this work around probably no longer needed
    //element.parentNode.replaceChild(element,element)

    //does not seem to do anything
    //getancestor(element,'TABLE').refresh()

}

function cleardropdown(element, all) {

    login('cleardropdown')

    if (!checkisdropdown(element)) return (0)

    //remove existing selections (leave one but blank it to avoid "0" showing)
    while (element.childNodes.length > (all ? 0 : 1)) {
        //  element.childNodes[0].removeNode(true)
        //element.removeChild(element.childNodes[0])
        neosysremovenode(element.childNodes[0])
    }
    if (element.childNodes.length) {
        element.childNodes[0].value = ""
        element.childNodes[0].text = ""
    }
    logout('cleardropdown')
}

function cleardropdownselections(element) {

    if (!checkisdropdown(element)) return (0)

    //    element.selectedIndex=-1
    if (element.childNodes.length) element.childNodes[0].selected = true

    //remove existing selections
    for (var i = 0; i < element.childNodes.length; i++) {
        element.childNodes[i].selected = false
    }
}

function checkisdropdown(element) {
    assertelement(element, 'checkisdropdown', 'element')

    if (typeof (element) != 'object' || element.tagName != 'SELECT') {
        neosysinvalid('Error: The target is not a SELECT tag')
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
    //neosysrequired false means add a blank option at the beginning

    //element must be a select element
    if (!checkisdropdown(element)) {
        log('setdropdown3 - not a select element')
        return false
    }

    //login('setdropdown3 '+element.id+' '+neosysquote(dropdowndata))
    login('setdropdown3 ' + element.id)

    //dropdowndata may be an array of strings or a string with ;: seps for cols/rows respectively
    //if [] then dropdowndata are not updated otherwise existing dropdowndata are deleted
    if (typeof (dropdowndata) == 'undefined' || dropdowndata == null) dropdowndata = []

    //convert conversion string to an array
    if (typeof (dropdowndata) == 'string') {

        //make sure we get at least one conversion
        if (dropdowndata == '') dropdowndata = ';'

        //convert into an array
        var sepchars = (dropdowndata.indexOf(vm) >= 0 || dropdowndata.indexOf(fm) >= 0) ? fm + vm : ':;'
        dropdowndata = dropdowndata.neosyssplit(sepchars)

    }

    //if not required make sure that the first dropdown option is ""
    if (dropdowndata.length && !element.getAttribute('neosysrequired')) {
        if (dropdowndata[0][0] != '' && dropdowndata[dropdowndata.length - 1][0] != '') {
            //ripple up, then replace first element
            for (var i = dropdowndata.length; i > 0; i--) dropdowndata[i] = dropdowndata[i - 1]
            dropdowndata[0] = ['', '']
        }
    }

    // if (dropdowndata.length) neosysbreak(element.id+' '+element.getAttribute('neosysrequired')+' '+dropdowndata.tostatement())

    //colns may be array(2) being pointers into dropdowndata for option value and text
    //otherwise [0,1]
    if (!is(colns)) colns = [0, 1]
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

    //neosysinvalid(selectedvalues.join())
    //method
    ////////

    if (dropdowndata.length) {

        cleardropdown(element, true)

        for (var i = 0; i < dropdowndata.length; i++) {
            var value = dropdowndata[i][valuecoln]
            if (no(requiredvalues) || requiredvalues.neosyslocate(value)) {

                var text = dropdowndata[i][textcoln]
                if (typeof (text) == 'undefined' || text == '') {
                    text = value
                    if (typeof text == 'undefined') text = ''
                    else text = text.toString().neosysconvert('_', ' ').neosyscapitalise()
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
                //if (selectedvalues.neosyslocate(option.value)) option.selected=true
                //if (selectedvalues.neosyslocate(option.text)) option.selected=true
                if (option.value == selectedvalue || option.text == selectedvalue) {
                    option.selected = true
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
    // if (element.getAttribute('neosysfieldno')!='0'&&element.selectedIndex==-1) element.selectedIndex=0

    logout('setdropdown3')

    return true

}

function addoption(element, value, text) {

    //build an option
    var option1 = document.createElement('OPTION')
    option1.value = value
    option1.text = text

    //insert it into the dropdown
    //element.add(option1)//does not work on the mac for option 1 after clearing
    element[element.length] = option1

    return element[element.length - 1]

}

//pls keep this routine synchronised in decide2.htm and scripts/client.htm
function neosysxml2obj(xmltext) {

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
                    cell.text = frag[1]
                    currentrow[frag[0]] = cell
                }
            }
        }
    }

    return dataobj

}


//copy any modifications to decide2.htm as well
function neosysobj2xml(obj) {

    var xml = '<record>' + cr
    for (var propname in obj) {
        if (propname.slice(0, 5) == 'group') {

            xml += '<' + propname + '>' + cr

            var group = obj[propname]
            for (var rown = 0; rown < group.length; rown++) {
                xml += '<' + 'row' + rown + '>' + cr
                var row = group[rown]
                for (var propname2 in row) {
                    //     if (row[propname2].element)
                    {
                        xml += '[' + row[propname2].element.id + ']'
                        xml += '<' + propname2 + '>'
                        xml += row[propname2].text
                        xml += '</' + propname2 + '>' + cr
                    }
                }
                xml += '</' + 'row' + rown + '>' + cr
            }

            xml += '</' + propname + '>' + cr
        }
        else {
            if (obj[propname].element) {
                xml += '[' + obj[propname].element.id + ']'
                xml += '<' + propname + '>'
                xml += obj[propname].text
                xml += '</' + propname + '>' + cr
            }
        }
    }

    xml += '</record>'
    return xml

}

function neosyssetdropdown(element, request, colarray, selectedvalues, xyz) {

    if (!checkisdropdown(element)) return (0)

    db.request = request

    if (db.send()) {
        dataobj = neosysxml2obj(db.data)

        //xmltemp=new ActiveXObject('Microsoft.XMLDOM')
        //xmltemp.loadXML(db.data.replace(/\&/g,'+'))
        ////xmltemp.loadXML(db.data)
        setdropdown2(element, dataobj, colarray, selectedvalues, null)
        if (db.data == '' || !dataobj.group1.length)
            element.neosysdropdown = ''
    }
    else {
        neosysinvalid(db.response)
    }
    return true
}

function getdropdown0(element) {

    var index = element.selectedIndex
    // neosysbreak(index)

    //ie5 on mac appears to use index=length sometimes (when only one option?)
    if (index >= element.length) index = 0

    //ie5 on pc uses -1 to indicate not selected
    if (index < 0) return ''

    return element[index].value

}

function neosysgetdropdown(element, mode) {
    if (!is(mode)) mode = 'selected'//otherwise 'all'
    var selectedvalues = []
    for (var i = 0; i < element.options.length; i++) {
        if (mode == 'all' || element.options[i].selected) {
            selectedvalues[selectedvalues.length] = element.options[i].value
            if (mode != 'all' && !element.multiple) return (selectedvalues.join())
        }
    }
    if (mode != 'all' && !element.multiple) selectedvalues = selectedvalues.join()
    return (selectedvalues)
}


//this function provides a place to debug into asp client code
//stepping out of this function allows you to step through code in client for asp
function aspbreak() {
    var x = ''
}

function neosysinput(question, text, texthidden) {
    if (!text) text = ''
    return neosysyesnocancel(question, '', 'OK', '', 'Cancel', text, texthidden)
}

function neosysyesno(question, defaultbutton) {
    return (neosysyesnocancel(question, defaultbutton, 'Yes', 'No') == 1)
}

function neosysokcancel(question, defaultbutton) {
    return neosysyesnocancel(question, defaultbutton, 'OK', '', 'Cancel')
}

function neosysyesnocancel(question, defaultbutton, yesbuttontitle, nobuttontitle, cancelbuttontitle, text, texthidden, image) {
    log('neosysyesnocancel ' + question)

    question = question.toString()
    if (question.slice(0, 6) == 'Error:')

        question = question.slice(6)
    var dialogargs = [question, defaultbutton, yesbuttontitle, nobuttontitle, cancelbuttontitle, text, texthidden, image]
    var dialogstyle
    //dialogstyle=(question.indexOf('\r')>=2)
    //?'dialogHeight: 300px; dialogWidth: 600px;'
    //:'dialogHeight: 220px; dialogWidth: 500px;'
    //dialogstyle+=' center: Yes; help: No; resizable: No; status: No;'

    var response = neosysshowmodaldialog(NEOSYSlocation + 'confirm.htm', dialogargs, dialogstyle)

    if (!response)
        response = 0

    return response

}

function striptags(string) {
    var temp = document.createElement('DIV')
    temp.innerHTML = string
    return temp.innerText
}

/*
function makeXMLisland(xmlelement,cmd)
{
db.request=cmd+'\rXML'
if(!db.send())
{
neosysinvalid(db.response)
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

window.geventlog = ''
window.geventstack = ['', '']
window.geventdepth = 1

function login(eventdesc) {

    if (!glogging) return

    //if (window_location.hostname!='sb_compaq') return

    // geventdepth=geventdepth+1
    geventstack = geventstack.slice(0, geventdepth + 1)
    geventstack[geventdepth] = '}' + eventdesc
    log(geventstack[geventdepth])
    geventdepth = geventdepth + 1
}

function logout(eventdesc) {

    //returns false to return logout('xxx')

    if (!glogging) return false

    geventdepth = geventdepth - 1
    if (geventdepth < 0) geventdepth = 0
    // window.one
    if (geventstack[geventdepth].split(' ')[0] != '}' + eventdesc.split(' ')[0]) {
        alert('logout does not match login\n\n' + eventdesc + ' ' + geventdepth + '\n\n' + geventstack.join('~'))
        windowx = windowopen()
        windowx.document.body.innerText = geventlog
    }

    geventstack[geventdepth] = '{' + eventdesc

    log(geventstack[geventdepth])

    return false

}

var gfirstlogtime
var glastlogtime

function log(entrytext) {

    if (!glogging)
        return

    //log to console if present
    if (window.console) {
        window.console.log(entrytext)
        return
    }

    //if (glogging>1) alert(entrytext)

    //limit the log size
    if (geventlog.length > 100000) {
        //geventlog=geventlog.slice(geventlog.indexOf('\r',16000)+1)
        geventlog = geventlog.slice(50000)
    }

    //work out the delay
    var logtime = new Date()
    if (typeof glastlogtime == 'undefined') {
        glastlogtime = logtime
        gfirstlogtime = logtime
    }
    var totlogtime = '0000000000000000' + (logtime - gfirstlogtime)
    totlogtime = totlogtime.slice(totlogtime.length - 9)
    var delayms = '0000000000000000' + (logtime - glastlogtime)
    delayms = delayms.slice(delayms.length - 9)

    //log it
    geventlog += totlogtime + ' ' + delayms + neosysstr(' .', geventdepth) + entrytext + '<br />\r\n'

    glastlogtime = logtime

    //display it
    if (entrytext == geventstack[geventdepth]) entrytext = ''
    window.status = totlogtime + geventstack.join('').slice(0, 1000) + ' ' + entrytext

    // if (!geventlogwindow) geventlogwindow=windowopen()
    // geventlogwindow.document.body.innerHTML=geventlog.toString.slice(-1000)

    // if (gstepping)
    // {
    //  //alert (entrytext)
    //  neosysoswrite(geventlog,'c:\\debug.txt')
    // }

}
var geventlogwindow

/////////////////
//CACHE FUNCTIONS
/////////////////

function loadcache() {

    if (typeof dbcache == 'object' && dbcache != null)
        return true

    login('loadcache')

    //create a global variable span to hold the userdata
    dbcache = document.createElement('SPAN')
    dbcache.values = new Object

    logout('loadcache - ok loaded')

    return true

}

function showcache() {

    if (!dbcache) return

    var values = dbcache.values

    //work out the total cache size (values only)
    var cachesize = 0
    var cachen = 0
    var html = ''
    for (var key in values) {
        if (key.slice(0, 6) != 'neosys') {
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
window.setInterval('trimcache()', gcachepruneintervalsecs * 1000)
function trimcache() {

    if (!dbcache) return

    var values = dbcache.values

    //work out the total cache size (values only)
    var cachesize = 0
    var cachen = 0
    for (var key in values) {
        if (key.slice(0, 6) != 'neosys') {
            cachesize += values[key].length
            cachen++
        }
    }

    var result = [cachesize, cachen]
    if (gusername == 'NEOSYS') wstatus('trimcache() cachesize:' + cachesize + ' cacheitems:' + cachen)

    //delete initial keys until the size is below the desired limit 
    //hopefully the initial keys are the oldest
    for (var key in values) {
        if (cachesize <= gmaxcachesize && cachen <= gmaxcachen) break
        if (key.slice(0, 6) != 'neosys') {
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

    if (!dbcache) return

    var values = dbcache.values
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

    login('readcache')

    if (!loadcache()) {
        logout('readcache - loadcache failed')
        return null
    }

    var key = cachekey(request)
    var result
    if (dbcache.values) {
        result = dbcache.values[key]

        //delete and restore to implement simple LRU cache
        //delete dbcache.values[key]
        //dbcache.values[key]=result

    }
    else {
        result = dbcache.getAttribute(key)

        //delete and restore to implement simple LRU cache
        //dbcache.removeAttribute(key)
        //dbcache.addAttribute(key,result)

    }

    logout('readcache ok')

    return result

}

function cachekey(request) {

    login('cachekey')

    //return escape(this.dataset+'\r'+this.username+'\r'+request).replace(/%/g,'_')
    // var temp=escape(gdataset+'_'+request).replace(/%/g,'_').replace(/@/g,'_AT_').replace(/\//g,'_SLASH_').replace(/\*/g,'_STAR_')
    var temp = escape(request).replace(/%/g, '_').replace(/@/g, '_AT_').replace(/\+/g, '_PLUS_').replace(/\//g, '_SLASH_').replace(/\*/g, '_STAR_')

    //alert(temp)

    logout('cachekey ' + temp)

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
    login('writecache')

    if (!loadcache()) {
        logout('writecache loadcache failed')
        return false
    }

    if (dbcache.values)
        dbcache.values[cachekey(request)] = data
    else
        dbcache.setAttribute(cachekey(request), data)

    dbcache.cacheupdated = true

    logout('writecache')

    return true

}

function deletecache(request) {
    login('deletecache')

    if (!loadcache()) {
        logout('deletecache loadcache failed')
        return false
    }

    if (dbcache.values) delete dbcache.values[cachekey(request)]
    else dbcache.removeAttribute(cachekey(request))

    dbcache.cacheupdated = true

    logout('deletecache')

    return true

}

function clearcache() {

    login('clearcache')

    //force refresh security table
    gtasks = null

    if (!loadcache()) {
        logout('clearcache loadcache failed')
        return false
    }

    try {
        //will fail on mac
        if (typeof dbcache.load != 'undefined')
            dbcache.load('emptycache')

        dbcache.values = new Object

        if (typeof dbcache.save != 'undefined')
            dbcache.save('\neosyscache')
    }
    catch (e) {
        logout('clearcache fail ' + e.description)
        return false
    }

    dbcache.cacheupdated = false
    logout('clearcache ok')

    return true

}

function sorttable(event, order) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    var colid = eventtarget.sorttableelementid

    if (typeof form_presort == 'function') {
        //neosysevaluate('form_presort()','formfunctions_onload()')
        if (!form_presort(colid)) return neosysinvalid()
    }

    //determine the groupno
    var dictitem = gds.dictitem(colid)
    if (!dictitem) return neosysinvalid()
    var groupno = dictitem.groupno
    if (!groupno) return neosysinvalid(colid + ' is not multivalued for sorting')

    window.status = 'Sorting, please wait ...'

    //get the data rows
    if (!groupno) groupno = 1
    var datarows = gds.data['group' + groupno]
    var fromrown = 0
    var uptorown = datarows.length - 1

    var up2down = false
    try {

        var clickedelement = event.srcElement
        //var clickedelement = document.getElementsByName('sortbutton_' + groupno)[0]

        //check if reverting from reverse to normal
        up2down = clickedelement.src.indexOf('up') >= 0

        //set the order image
        var order = (clickedelement.src.indexOf('down') >= 0) ? 'up' : 'down'
        resetsortimages(groupno)
        clickedelement.src = '../neosys/images/smallsort' + order + '.gif'
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
                var temp = new neosysamount(sortdata)
                sortdata = temp.amount.split('.')
                if (!sortdata[1]) sortdata[1] = ''
                sortdata = temp.unit + neosysspace(dictitemlength - sortdata[0].length) + sortdata[0] + temp.amount
            }
        }
        else {
            //if indented then work out prefix from higher levels
            if (sortdata.match(/(^\s+)/)) {
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
        temp = neosysspace(6 - temp.length) + temp

        //save the two column array for sorting
        //need the +temp so that indented fields sort correctly
        data[rown] = [sortdata + temp, temp]

    }

    //get the table rows
    var tablex = $$('neosysgroup' + groupno)
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
        //neosysevaluate('form_postsort()','formfunctions_onload()')
        if (!form_postsort(colid))
            return neosysinvalid()
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

    if (!element.menuok)
        okmenus = neosysgetcookie(glogincode, 'NEOSYS2', 'm').split(',')

    if (!element.neosysmenuaccesskeys)
        element.neosysmenuaccesskeys = []

    var childnodes = element.childNodes

    nextchild:
    for (var i = element.childNodes.length - 1; i >= 0; i--) {
        var child = childnodes[i]

        //delete some menus
        if (okmenus && okmenus.join() != '' && child.id && child.id.slice(0, 5) == 'menu_') {
            if (!okmenus.neosyslocate(child.id.slice(5))) {
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
                child.onmouseout = menuonmouseout
                //child.onfocus=menuonmouseover
                //child.onblur=menuonmouseout
                child.className = 'menuitem'
                child.style.paddingLeft = '0px'

                if (typeof child.neosyssubmenuoption == 'undefined') {

                    //save an array of elements by accesskey in the menu
                    var underlineelement
                    if ((underlineelement = child.getElementsByTagName('U')).length) {
                        var menuaccesskey = underlineelement[0].innerText.neosystrim().slice(0, 1).toUpperCase()
                        var temp = element.neosysmenuaccesskeys[menuaccesskey]
                        if (gusername == 'NEOSYS' && temp) neosysnote('Duplicate menu access key ' + menuaccesskey.neosysquote() + ' for\r' + child.innerText + '\rand\r' + temp.innerText)
                        element.neosysmenuaccesskeys[menuaccesskey] = child
                    }
                }

                //add submenu indicators
                if (!child.href && !child.onclick && typeof child.neosyssubmenuoption == 'undefined') {
                    child.neosyssubmenuoption = true
                    //child.style.width=child.parentNode.runtimeStyle.width
                    //child.style.border='1px solid'
                    if (element.id != 'menudiv')
                        child.innerHTML += ' ...'

                }
                else {
                    child.neosyssubmenuoption = false
                }

            }
        }
    }

    element.menuok = true

}

var gnmenus = 0
var gmenutimeout = ''
var gselecthidden = false

function menuonclick(event) {
    menuonmouseover(event, 'click')
}

function menuonmouseover(event, menuoption) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    if (!gmenuloaded)
        return false

    //like esc
    if (menuoption == 'click') {
        menuoption = ''
        if (gnmenus) {
            gnmenus = 0
            menuclose()
            return neosyscancelevent(event)
        }
    }

    //indicate menuing and cancel any request to close menus
    var keyboarding = menuoption
    gnmenus = 1
    window.clearTimeout(gmenutimeout)

    //hide select elements as they show show through the drop down menus (dont respect z-order)
    if (!gselecthidden) {
        var style0 = document.styleSheets[0]
        var rules = style0.cssRules
        if (!rules)
            rules = style0.rules
        if (rules[0].selectorText != 'select') {
            gselecthidden = true
            //http://www.quirksmode.org/dom/w3c_css.html
            if (style0.addRule)
                style0.addRule('select', 'visibility:hidden', 0)
            else
                style0.insertRule('select {visibility:hidden}', 0)
        }
    }

    //get the menuoption
    if (!menuoption) {
        menuoption = eventtarget
        if (!menuoption)
            return
    }
    //click detected on childnode
    if (menuoption.id == 'menubutton_label')
        menuoption = menuoption.parentNode

    var menu = menuoption.parentNode

    //window.status=new Date+' '+menu.clientLeft+' '+menu.clientWidth
    var leftoffset
    var topoffset
    if (menuoption.id == 'menubutton' || menuoption.tagName == 'BUTTON' || menuoption.id.slice(0, 5) == 'menu_') {
        //  leftoffset=menuoption.offsetWidth+menuoption.offsetLeft-4
        //  topoffset=0
        //make the menu visible just beneath the menu button
        leftoffset = menuoption.offsetLeft + 1
        topoffset = menuoption.offsetHeight + menuoption.offsetTop - 1
    }
    else {
        //make the menu visible just to the right of the menu option
        leftoffset = menu.offsetWidth - menu.clientLeft - 8
        topoffset = -menu.clientTop
        topoffset = 0
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
            submenu = eventtarget
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

    }

}

//sets a timeout to focus on an element
//OR executes the timeout function
var gmenuelement;
function menufocus(menu) {

    //if given an element save it global and set timeout callback
    if (menu) {
        gmenuelement = menu
        window.setTimeout('menufocus()', 1)
        return
    }

    //focus on the saved element
    try { gmenuelement.focus() } catch (e) { }

}

function menuonmouseout(event) {
    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    gnmenus = 0
    gmenutimeout = window.setTimeout('menuclose()', 1000)
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

    //redisplay select elements
    var style0 = document.styleSheets[0]
    while (true) {

        //rules object is live on ie7/ff3 but not safari3.1
        var rules = style0.cssRules
        if (!rules)
            rules = style0.rules

        if (!rules[0] || rules[0].selectorText.toLowerCase() != 'select')
            break

        gselecthidden = false

        //http://www.quirksmode.org/dom/w3c_css.html
        if (style0.removeRule)
            style0.removeRule(0)
        else
            style0.deleteRule(0)

    }

}

function menuonkeydown(event, menu, key) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    //usually called as an event with no parameters
    //doesnt seem to work well except called as an event
    if (!menu || !key) {
        menu = eventtarget
        key = event.keyCode
    }

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

    //wstatus(new Date+' '+key)

    //esc ... close menu and return focus to where it was when menu was opened
    if (esckey || (!horizontal && leftkey)) {
        if (menu.parentNode.parentNode.highlightedelement) {
            menuonmouseover(event, menu.parentNode.parentNode.highlightedelement)
            //window.setTimeout('menuonkeydown(13)',10)
            //attempt to press Enter on it
            //menuonkeydown(menu.parentNode.parentNode,13)
            return neosyscancelevent(event)
        }
        else {
            gnmenus = 0
            menuclose()
            return neosyscancelevent(event)
        }
    }

    //menu access key?
    if (alphakey && menu.neosysmenuaccesskeys) {
        var newmenuoption = menu.neosysmenuaccesskeys[String.fromCharCode(key)]
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
                        windowopen(menuoption.href)
                    else
                        window.location = menuoption.href
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
        return neosyscancelevent(event)
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
                    if (newmenuoption.innerText.neosystrim().slice(0, 1).toUpperCase() == String.fromCharCode(key))
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
                    if (newmenuoption.innerText.neosystrim().slice(0, 1).toUpperCase() == String.fromCharCode(key))
                        break

                }
                newmenuoption = ((endkey || leftkey || upkey) ? newmenuoption.previousSibling : newmenuoption.nextSibling)
            }
        }
    }

    //if found a new option then move highlight to it
    if (newmenuoption && newmenuoption.tagName == 'A' && newmenuoption != menuoption) {

        menuchangeoption(menu, newmenuoption)

        return neosyscancelevent(event)

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
        window.setTimeout('menufocus()', 10)
    }
    menu.onkeydown = menuonkeydown

}

function menubuttonhtml(id, imagesrc, name, title, accesskey, align) {

    //var tx='<span'
    var tx = '<div'

    if (id)
        tx += ' id="' + id + 'button' + '"'

    tx += ' class=graphicbutton'
    tx += ' onmousedown=this.style.borderStyle="inset"'
    tx += ' onmouseup=this.style.borderStyle="outset"'
    tx += ' onmouseout=this.style.borderStyle="outset"'

    if (title)
        tx += ' title="' + title + '"'

    //there is no float:center?!
    if (align == 'center')
    //tx+=' style="float:'+align+'"'
    //tx+=' style="margin:0 auto"'
    //switch off the default from graphicbutton class?
        tx += ' style="float:none"'
    else if (align)
        tx += ' style="float:' + align + '"'

    if (accesskey)
        tx += ' accesskey="' + accesskey + '"'

    tx += ' onclick="' + id + '_onclick(event)"'

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

    //tx+='</span>'
    tx += '</div>'

    //create a tiny button to capture the accesskey to prevent it from being used by the browser
    //firefox access key is shift+alt unless reconfigured in about:config
    //http://kb.mozillazine.org/Ui.key.contentAccess
    //so ALSO implemented in onkeydown
    if (accesskey) {
        tx += '<button xtabindex=-1 style="background-color:white; height:1px; width:1px; border-style:none; margin:0px ;padding:0px"'
        tx += ' accesskey="' + accesskey + '"'
        tx += ' onclick="' + id + '_onclick(event)"'
        tx += '></button>'
    }

    return tx

}

function setgraphicbutton(button, labeltext, src) {

    var label = $$(button.id + '_label')
    label.innerHTML = labeltext
    if (src)
        button.getElementsByTagName('IMG')[0].src = src
}

function refreshcache_onclick() {
    if (clearcache())
        neosysnote('All NEOSYS data cached in this window has been cleared\rand will be retrieved from the server again as and when required.')
    // \r\rN.B. NEOSYS forms and scripts will remain cached and may\rbe updated when you close and reopen all browser\rwindows - depending on the cache settings in your browser.')
    else
        neosysnote('Cannot clear cache.')
    return true
}

function neosyscancelevent(event) {

    //should error maybe
    if (!event)
        event = window.event
    if (!event)
        return

    event.cancelBubble = true
    event.returnValue = false

    if (event.stopPropagation)
        event.stopPropagation()
    if (event.preventDefault)
        event.preventDefault()
    event.cancel = true

    return false

}

var eventtarget

function geteventtarget(event) {
    if (!event)
        return
    if (event.target)
        eventtarget = event.target;
    else if (event.srcElement)
        eventtarget = event.srcElement;
    if (!eventtarget)
        return
    if (eventtarget.nodeType == 3) // defeat Safari bug
        eventtarget = eventtarget.parentNode;
    return eventtarget
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

    //scan all elements adding id to classname as neosys_idname
    if (!element)
        element = document.body
    while (element = nextelement(element)) {
        if (element.id) {
            if (element.className) {
                if (element.className.indexOf('neosysid_') < 0) {
                    element.className += ' neosysid_' + element.id
                }
            }
            else
                element.className = 'neosysid_' + element.id
        }
    }

}

function neosysautofitwindow() {

    var element = $$('autofitwindowelement')
    if (!element)
        return

    //width
    var newwidth = element.offsetWidth + 50
    //removed because problem with too wide calendar popup
    //if (newwidth < 500)
    //    newwidth = 500

    //height
    var newheight = element.offsetTop + element.offsetHeight + 100

    //allow for surrounding bars
    if (window.dialogTop) {
        newwidth += 20
        newheight += 10
    }
    else if (false) {
        newwidth += 10
        newheight += 80
    }
    else {
        newwidth += 60
        newheight += 80
    }

    //constrain to screen size
    if (newwidth > window.screen.availWidth)
        newwidth = window.screen.availWidth
    if (newheight > window.screen.availHeight)
        newheight = window.screen.availHeight

    //top
    var newtop = (window.screen.availHeight - newheight) / 2

    //left
    var newleft = (window.screen.availWidth - newwidth) / 2
    //debug(window.resizeTo)
    //w3c (but present in IE but doesnt work - so try both)
    //if (window.resizeTo)
    try {
        window.resizeTo(newwidth, newheight)
        window.moveTo(newleft, newtop)
    }
    catch (e) { }
    //msie (and later versions of FF3 but setting doesnt seem to work in 3.03)
    //else if (window.dialogWidth)
    try {
        window.dialogWidth = newwidth + 'px'
        window.dialogHeight = newheight + 'px'
        window.dialogTop = newtop + 'px'
        window.dialogLeft = newleft + 'px'
    }
    catch (e) { }

}

function addeventlistener(element, eventname, functionx) {

    if (typeof functionx == 'string') {
        //element.setAttribute('on'+eventname,functionx+'(event)')
        //return
        functionx = window[functionx]
    }

    //if (isMSIE)
    //{
    // element['on'+eventname]=functionx
    // return
    //}

    if (element.addEventListener)
    //native code in FF/Safari
        element.addEventListener(eventname, functionx, true)
    else if (element.attachEvent)
    //if cant attachEvent then possibly it isnt a DOM element
        element.attachEvent('on' + eventname, functionx)
    //else
    //run function in the environment of the element
    //element.attachEvent('on'+eventname,function(){functionx.call(element)})

}

/*
function neosysgetattribute(element,attributename)
{
var attribute=element.getAttribute(attributename)
if (!attribute)
return attribute
if (typeof attribute=='string')
{
if (attribute=='true')
return true
else if (attribute=='false')
return false
return attribute
}
return attribute
}
*/

function neosysremovenode(element) {
    element.parentNode.removeChild(element)
}
