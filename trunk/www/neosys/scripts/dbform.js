// © 2000-2008 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

var gdatatagnames = /(^SPAN$)|(^INPUT$)|(^TEXTAREA$)|(^SELECT$)/
var gtexttagnames = /(^SPAN$)|(^INPUT$)|(^TEXTAREA$)/
var gradiocheckboxtypes = /(^radio$)|(^checkbox$)/
var nbsp160 = String.fromCharCode(160)

//gnewimage=gimagetheme+'application_form_add.png'//'add.png'
gnewimage = gimagetheme + 'application.png'//'add.png'
gopenimage = gimagetheme + 'open.gif'
gfindimage = gimagetheme + 'zoom.png'
gcalendarimage = gimagetheme + 'calendar.gif'
gsaveimage = gimagetheme + 'tick.png'
gcopyimage = gimagetheme + 'page_copy.png'
gcloseimage = gimagetheme + 'cross.png'
greleaseimage = gimagetheme + 'key.png'
greopenimage = gimagetheme + 'key.png'//'lock.png'
gdeleteimage = gimagetheme + 'application_form_delete.png'//'delete.png'
glistimage = gimagetheme + 'table.png'
gprintsendimage = gimagetheme + 'printer.png'//'application_form.png'
ginsertrowimage = gimagetheme + 'add.png'
gdeleterowimage = gimagetheme + 'delete.png'
gexpandrowimage = gimagetheme + 'smallexpand.gif'
gsortimage = gimagetheme + 'smallsort.gif'
glinkimage = gimagetheme + 'application_form.png'
gfirstimage = gimagetheme + 'resultset_first.png'
glastimage = gimagetheme + 'resultset_last.png'
gnextimage = gimagetheme + 'resultset_next.png'
gpreviousimage = gimagetheme + 'resultset_previous.png'
gspacerimage = gimagetheme + 'spacer.png'
gblankimage = gimagetheme + 'blank.gif'

// document.getElementsByTagName('BODY').onload=window_onload 

//addeventlistener(window,'beforeunload','window_onbeforeunload')
//addeventlistener(window,'unload','window_onunload')
//addeventlistener doesnt seem to work on window element in safari 3.1.2 but following works IE/FF/Saf
window.onbeforeunload = window_onbeforeunload
window.onunload = window_onunload

addeventlistener(document, 'keydown', 'document_onkeydown')
addeventlistener(document, 'click', 'document_onclick')

//form function global variables
var gpagenrows = 10
var gkeycode
var gdictfilename

var gparameters
//move any input parameters over from parent windows
//this is done to avoid using ?param=etc in the url which prevents caching
//and is therefore slower
var gwindowopenerparameters = ''
try {
    if (window.opener
  && window.opener.gwindowopenparameters) {
        gwindowopenerparameters = window.opener.gwindowopenparameters
        //delete the parameters so that they are only used once
        window.opener.gwindowopenparameters = null
    }
}
catch (e) { }
if (gwindowopenerparameters) {
    for (var paramname in gwindowopenerparameters) {
        gparameters[paramname] = gwindowopenerparameters[paramname]
    }
}
if (gparameters.readonlymode) greadonlymode = true

var gonfocuselement
var gpreviouselement = null
var gnextelement = null
var gdependents = []
var gKeyNodes = false//init will get an array of key nodes if any
var gkeyexternal = ''//external format of key eg STEVE*1/1/2000
var gkey = ''//internal format of key eg STEVE*12080
var gkeys = []
var gkeyn = 0
var glastkey = ''
var gloaded = false
var gfirstelement
var gfirstnonkeyelement = ''
var gstartelement
var gfinalinputelement
var gchangesmade = false//set true in validateupdate exit and delete row (not insert row)
var gallowsavewithoutchanges = false//allows locked records (with keys) to be saved anyway
var glocked = false//true means record is locked and available for edit
var grelockingdoc = false
var gwindowunloading = false
var gclosing = false
//var gisnewrecord replaced by gds.isnewrecord
var grelocker
var gro
var gmaxgroupno = 0
var gfields = []
var gtables = []
var gsortimages = []
var gdoctitle = document.title
var ginitok = false
var grecn = null
var gfocusonelement
var gcell
var gvalue
var gorigvalue
var gvalidatingpopup
var gtimers
//var gsavemode replaced by gparameters.savemode
var greadonlymode
var gupdateonlymode
var gpreventcreation
var gpreventdeletion
var gcalendarscript//1 if needed, 2 if script inserted, 3 if script loaded

//form customisation (must be in a script loaded prior to this dbform script)
if (typeof (form_preinit) == 'function') {
    neosysevaluate('form_preinit()', 'dbform.js()')
}

//add a script for the data file definition in case not included in the main html
if (typeof gmodule == 'undefined') {
    gmodule = ''
    window.setTimeout('formfunctions_onload', 10)
}
//commented out so that gmodule is optional if 
//else
{
    if (typeof gdictfilename == 'undefined' || !gdictfilename)
        gdictfilename = gdatafilename
    if (typeof gdatafilename == 'undefined' || !gdatafilename)
        gdatafilename = gdictfilename
    //  var temp='<SCR'+'IPT onload="formfunctions_onload()" id=maindict src="scripts/'+gdictfilename.toLowerCase()+'_dict.js"></SCR'+'IPT>'
    if (eval('typeof dict_' + gdictfilename + '=="undefined"')) {
        var temp = '<SCR' + 'IPT id=maindict src="scripts/' + gdictfilename.toLowerCase() + '_dict.js"></SCR' + 'IPT>'
        document.writeln(temp)
    }
}

var gds

//'WINDOW LOAD
//''''''''''''

function formfunctions_onload() {

    wstatus('Initialising, please wait ...')

    //has the problem that clicking on popup images "activates" the surrounding TD!
    //old method is to instate onfocus on all neosys data elements

    //document.body.onactivate=document_onfocus
    //document.body.onfocus='document_onfocus(event)'
    //document.body.onfocus=document_onfocus
    var activateorfocus = typeof document.body.onactivate == 'undefined' ? 'focus' : 'activate'
    addeventlistener(document.body, activateorfocus, 'document_onfocus')

    gds = new neosysdatasource
    gds.onreadystatechange = gds_onreadystatechange

    //flag to later events that onload has not finished (set true at end of window_onload)
    ginitok = false

    //make sure not "framed"
    if (window != window.top)
        top.location.href = self.location.href

    login('formfunctions_onload')

    //form customisation
    if (typeof form_onload == 'function') {
        if (!neosysevaluate('form_onload()', 'form_onload()'))
            return
    }

    //make sure we are logged in
    if (!gusername) {
        db.login()
    }
    //done in client.js after db
    //gbasecurr=neosysgetcookie(glogincode,'NEOSYS2','bc')
    //gbasefmt=neosysgetcookie(glogincode,'NEOSYS2','bf')

    //check a parameter
    if (typeof gmodule == 'undefined') {
        systemerror('formfunctions_onload()', 'gmodule is not defined')
        return logout('formfunctions_onload - no gmodule')
    }

    //check dictionary function exists
    var temp
    try {
        temp = eval('dict_' + gdictfilename)
    }
    catch (e) { }
    if (typeof temp != 'function') {
        systemerror('dict_' + gdictfilename + ' dict function not defined')
        return logout('formfunctions_onload - dict function missing')
    }

    gro = new neosysrecord(neosysevaluate('dict_' + gdictfilename + '(gparameters)', 'formfunctions_onload()'), gdatafilename)
    gds.dict = gro.dict

    //calendar popup functions
    if (gcalendarscript == 1) {
        var element = document.createElement('SCRIPT')
        element.src = NEOSYSlocation + 'scripts/calendar.js'
        //document.body.insertBefore(element,null)
        //solve htmlfile invalid argument error
        document.body.insertBefore(element, document.body.firstChild)
        //document.writeln('<SCR'+'IPT src="'+NEOSYSlocation+'scripts/calendar.js" type="text/javascript"></SCR'+'IPT>')
    }

    //greadonlymode=false

    //set the datasource of all elements
    datasrcelements = []

    element = document.body
    while (element = nextelement(element)) {

        //skip weird elements
        if (typeof element.getAttribute == 'unknown' || !element.getAttribute)
            continue

        //ensure buttons havew tabIndex 999 - to make them come last
        if (element.tagName == 'BUTTON' && !element.tabIndex)
            element.tabIndex = 999

        //NB inserting elements within the loop means that the same element
        //may be processed more than once so ensure skip on 2ndtime
        //  if(typeof element.getAttribute('neosysgroupno')=='undefined'&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
        //element.name ... gives error "class does not support automation"
        //  if(typeof element.getAttribute('neosysgroupno')=='undefined'&&element.name&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
        //  if(typeof element.getAttribute('neosysgroupno')=='undefined'&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
        var fieldname = element.id

        //if (typeof element.getAttribute('neosysgroupno')=='undefined'
        //    &&element.tagName!='OPTION'
        //    &&(fieldname=element.name))
        if (element.getAttribute('neosysgroupno') == null
  && element.tagName != 'OPTION') {
            //locate the dictionary item otherwise skip the field
            var dictitem = gro.dictitem(fieldname)
            if (!dictitem) {
                //    systemerror('formfunctions_onload()','Form element '+neosysquote(fieldname)+' is not in the '+neosysquote(gdictfilename)+' dictionary\ror is not in the correct group.')
                continue
            }

            //check that the name is not in use elsewhere (function or other field)
            //try
            //{
            var temp = window[fieldname]
            if (!temp)
                temp = document[fieldname]
            if (temp && temp != element) {
                //TODO should check all and not just ignore previously built radio or checkboxes
                var temp0 = temp[0] ? temp[0] : temp
                if (temp0.type != 'radio' && temp0.type != 'checkbox') {
                    systemerror('name ' + neosysquote(fieldname) + ' is already in use elsewhere\r' + eval(temp).toString().slice(0, 100) + '\r...')
                    continue
                }
            }
            //}
            //catch (e){}

            //convert into DOM node
            element.id = fieldname
            //there is a problem with this for radio boxes as there are multiple elements for one id
            if (element.type != 'radio' && element.type != 'checkbox') {
                element = $$(element.id)
            }
            //element=eval(element.id)

            log('formfunctions_onload ' + fieldname)

            //dictionary modifications
            //none - currently done in dictrec builder

            //convert long text input to spans so that it can flow (if length not defined)
            //if (element.tagName=='INPUT'&&element.neosysalign=='T')
            //if (element.tagName=='INPUT'&&dictitem.align=='T'&&!dictitem.length)
            //above stops text flowing - maybe put || dictitem.readonly
            if (element.tagName == 'INPUT' && dictitem.align == 'T') {

                //replace original element
                var newspan = document.createElement('SPAN')
                element.parentNode.replaceChild(newspan, element)

                //copy over attributes
                newspan.tabIndex = element.tabIndex
                //commented out because it prevent setting to -1 if readonly below
                //if (!newspan.tabIndex)
                // newspan.tabIndex=999
                if (typeof element.length != 'undefined')
                    newspan.length = element.length
                newspan.id = element.id

                //switch over to the new element
                element = newspan
            }

            //copy the dictionary to the field
            //repeated below because cannot seem to merge expando attributes on mac!!!)
            //TODO consider removing lower repetition
            //also repeated for ckeditor spans
            copydictitem(dictitem, element)

            var elementtabindex = element.tabIndex

            //from here on there should be no need for references to the dictitem

            //build radio and check boxes
            if ((element.getAttribute('neosysradio') && element.type != 'radio')
	   || (element.getAttribute('neosyscheckbox') && element.type != 'checkbox')
	  ) {

                //radio has preference over checkbox
                if (element.getAttribute('neosysradio'))
                    element.setAttribute('neosyscheckbox', '')

                //gdefault=getdefault(element)

                //build html for multiple inputs
                var options = (element.getAttribute('neosysradio') ? element.getAttribute('neosysradio') : element.getAttribute('neosyscheckbox')).neosyssplit(':;')
                var temp = ''
                var elementtype = element.getAttribute('neosysradio') ? 'radio' : 'checkbox'
                for (var ii = 0; ii < options.length; ii++) {

                    //build an input item
                    temp += '<input type=' + elementtype + ' id=' + fieldname
                    if (element.getAttribute('neosysreadonly'))
                        temp += ' disabled=true'
                    //temp+='<span style="white-space: nowrap"><input type='+elementtype+' id='+element.name
                    //must be done to group radio boxes
                    temp += ' name=' + fieldname
                    if (typeof (options[ii][0]) != 'undefined')
                        temp += ' value=' + options[ii][0].toString().neosysquote()

                    //set default but it has to be done again in cleardoc anyway
                    //if (gdefault==options[ii][0]) temp+=' checked=true'

                    //temp+=' onfocus="return document_onfocus(event)">'
                    temp += '>'

                    //postfix the option title
                    if (typeof (options[ii][1]) != 'undefined')
                        temp += '<span style="white-space: nowrap">' + options[ii][1] + '</span>'

                    //horizontal or vertical
                    //if vertical then add <br /> between options
                    //but not after last option (so that next input can appear inline with the last option.
                    //if (!element.neosyshradio) temp+='<br />'
                    //temp+=(element.getAttribute('neosyshorizontal')?'&nbsp;':'<br />')
                    if (element.getAttribute('neosyshorizontal'))
                        temp += '&nbsp;&nbsp;'
                    else if (ii < options.length - 1)
                        temp += '<br />'
                    //temp+='</span>'+(element.getAttribute('neosyshorizontal')?'&nbsp;':'<br />')

                }

                //replace original element
                var newspan = document.createElement('SPAN')
                newspan.innerHTML = temp
                //element.swapNode(temp)
                element.parentNode.replaceChild(newspan, element)

                //setup onfocus and onclick for all boxes
                for (var ii = 0; ii < newspan.childNodes.length; ii++) {
                    element = newspan.childNodes[ii]
                    if (element.type == elementtype) {
                        element.tabIndex = elementtabindex

                        //element.onfocus=onfocus
                        //element.onclick=onclickradiocheckbox
                        //sadly onfocus doesnt seem to get called on checkboxes in safari 4/chrome 5.0
                        addeventlistener(element, 'focus', 'document_onfocus')
                        addeventlistener(element, 'click', 'onclickradiocheckbox')
                    }

                }


                //having converted the element to a SPAN/array of INPUT elements ...
                element = newspan
                continue

            }

            if (element.getAttribute('neosysdropdown')) {

                neosysassertobject(element, 'formfunctions_onload', 'element (neosysdropdown)')
                neosysassertobject(element.getAttribute('neosysdropdown'), 'formfunctions_onload', element.id + '.getAttribute("neosysdropdown")')

                //convert element to a SELECT
                //var temp=document.createElement('SELECT '+element.outerHTML.slice(7)
                var temp = document.createElement('SELECT')
                copydictitem(dictitem, temp)

                //make sure tabindex is copied over
                temp.tabIndex = element.tabIndex

                //indicate that it is a single selection, one line drop down.
                temp.size = 1

                //element.swapNode(t)
                element.parentNode.replaceChild(temp, element)

                request = 'CACHE\r' + element.getAttribute('neosysdropdown')[0]
                colarray = element.getAttribute('neosysdropdown')[1]

                neosyssetdropdown(temp, request, colarray)

                element = temp
                //    element.innerHTML=element.innerHTML+' '

                element.tabIndex = elementtabindex

            }

            //convert to select element
            if (element.getAttribute('neosysconversion')) {

                //conversion is a routine eg [NUMBER] [DATE]
                if (typeof (element.getAttribute('neosysconversion')) == 'string'
    && element.getAttribute('neosysconversion').slice(0, 1) == '[') {
                }

                //conversion is an array of options
                else {

                    //convert element to a SELECT

                    // if (element.id=='USER_CODE') neosysbreak('x '+element.id+' '+element.getAttribute('neosysrequired')+' '+element.outerHTML)
                    var temp = document.createElement('SELECT')
                    copydictitem(dictitem, temp)

                    //if (element.id=='USER_CODE') neosysbreak('y '+temp.id+' '+temp.getAttribute('neosysrequired')+' '+temp.outerHTML)
                    temp.size = 1
                    //element.swapNode(t)
                    element.parentNode.replaceChild(temp, element)
                    origid = element.id
                    element = temp
                    element.id = origid

                    setdropdown3(element, element.getAttribute('neosysconversion'))

                    element.tabIndex = elementtabindex

                    //element.onchange=validateupdate
                    addeventlistener(element, 'change', 'validateupdate')

                }
            }

            if (element.neosysreadonly && !element.getAttribute('disabled') && element.tagName && element.tagName == 'SELECT')
                setdisabled(element, true)

            //add some events - done on document.body now
            //if (element.tagName.match(gdatatagnames))
            //{
            //}

            //check for duplicate field numbers
            with (dictitem) {
                if (type == 'F' && !dictitem.allowduplicatefieldno) {

                    var fieldandwordn = fieldno
                    if (keypart) fieldandwordn += '.' + keypart
                    if (dictitem.wordno) fieldandwordn += '.' + wordno

                    //skip radio/checkbox elements with the same id
                    if (gds.dict.fieldandwordns[fieldandwordn] != element.id) {

                        if (typeof gds.dict.fieldandwordns[fieldandwordn] != 'undefined') {
                            //throw(new Error(0,neosysquote(name)+' duplicate dictionary field and word no '+fieldandwordn+' is not allowed'))
                            systemerror('formfunction_onload', neosysquote(name) + ' duplicate dictionary name, field no, word no ' + fieldandwordn + ' is not allowed without .allowduplicatefieldno=true\r' + gds.dict.fieldandwordns[fieldandwordn])
                        }

                        //save a field pointer
                        gds.dict.fieldandwordns[fieldandwordn] = element.id

                    }

                }

            }

            //copy the dictionary to the field (repeated from above!)
            copydictitem(dictitem, element)

            //the first element is considered to be the 'key' field
            //zzz should be field 0
            if (element.getAttribute('neosysfieldno') == '0') {
                if (!gKeyNodes)
                    gKeyNodes = []
                gKeyNodes[gKeyNodes.length] = element
                if (gKeyNodes.length == 1) {
                    var tt = element.getAttribute('neosysprintfunction')
                    if (tt)
                        gKeyNodes[0].setAttribute('neosysprintfunction', tt)
                    var tt = element.getAttribute('neosyslistfunction')
                    if (tt)
                        gKeyNodes[0].setAttribute('neosyslistfunction', tt)
                    element.accessKey = 'K'
                }

            }

            //allow for data entry in SPAN elements
            if (element.getAttribute('neosystype') == 'F' && element.tagName == 'SPAN') {
                var minwidth = element.getAttribute('neosyslength') * 7
                //buggy and not necessary on msie7
                if (!isMSIE) {
                    //    element.style.width=(element.getAttribute('neosyslength')*7)+'px'
                    element.style.minWidth = minwidth + 'px'
                    //element.style.minHeight='12px'
                    //element.style.maxWidth=(element.getAttribute('neosyslength')*7*2)+'px'
                    //element.style.maxWidth=minwidth+'px'
                    //element.multiLine=true
                    element.style.display = 'inline-block'
                }

                if (!element.getAttribute('neosysreadonly')) {
                    element.contentEditable = 'true'
                    //fixed width in msie but buggy in ff?
                    if (isMSIE)
                        element.style.width = element.neosyslength * 7
                    if (!element.getAttribute('tabindex'))
                        element.setAttribute('tabindex', 999)
                }
            }

            //prevent tab focussing on uneditable spans
            if (element.tagName == 'SPAN' && element.getAttribute('neosystype') == 'S')
                element.tabIndex = -1

            //add button before element with popups (and selects to make it clear to users that F7 is available - especially since useful when selecting multivalues)
            if (
   (element.tagName == 'SELECT'
   || element.getAttribute('neosyspopup')
   )
   && !element.getAttribute('neosysreadonly')
   &&
   (element.type == 'text'
   || element.contentEditable == 'true'
   || element.tagName == 'SELECT'))
            //if (element.getAttribute('neosyspopup')&&!element.getAttribute('neosysreadonly'))
            {
                if (typeof element.getAttribute('neosyspopup') == 'string'
    || element.tagName == 'SELECT') {
                    //conversion is a routine eg [neosysfilepopup(filename,cols,coln,sortselect] [popup.clients]

                    var element2 = document.createElement('IMG')
                    //add the button right before/after the field
                    element2.id = element.id + '_popup'

                    /*
                    var nowrapper = document.createElement('SPAN')
                    if (element.neosysalign != 'T')
                    nowrapper.style.whiteSpace = 'noWrap'
                    element = element.parentNode.replaceChild(nowrapper, element)
                    nowrapper.insertBefore(element, null)
                    nowrapper.insertBefore(element2, null)
                    */

                    /*
                    //put nowrap flag on TD in 3 levels
                    //except on span fields because otherwise they dont text wrap
                    if (element.tagName != 'SPAN') {
                    var td = element.parentNode
                    if (td && td.tagName != 'TD') td = td.parentNode
                    if (td && td.tagName != 'TD') td = td.parentNode
                    if (td && td.tagName == 'TD') td.noWrap = true
                    }
                    */

                    //insertafter(element,element2)
                    element.parentNode.insertBefore(element2, element)

                    //element2.src='../neosys/images/search.gif'
                    //element2.style.marginTop='3px'
                    element2.style.verticalAlign = 'top'
                    //element2.style.border='solid 1px'
                    if (fieldname.indexOf('DATE') >= 0)
                        element2.src = gcalendarimage
                    else
                        element2.src = gfindimage
                    element2.title = 'Find a' + ('aeioAEIO'.indexOf(element.getAttribute('neosystitle').slice(0, 1)) != -1 ? 'n' : '') + ' ' + element.getAttribute('neosystitle')
                    element2.title += ' (F7)'
                    element2.style.cursor = 'pointer'

                    //element2.onclick='neosyspopup()'
                    //addeventlistener(element2,'click','neosyspopup')
                    //element2.setAttribute('onclick','neosyspopup()')
                    element2.setAttribute('isneosyspopup', '1')

                }
            }

            //add button before element for link (or after if right justified)
            if (element.getAttribute('neosyslink')) {
                if (typeof element.getAttribute('neosyslink') != 'string') {
                    systemerror('formfunction_onload', neosysquote(fieldname) + ' link must be a string')
                }
                else {
                    //conversion is a routine eg [neosysfilepopup(filename,cols,coln,sortselect] [popup.clients]

                    var element2 = document.createElement('IMG')
                    //add the button right after the field

                    //put nowrap flag on TD in 3 levels
                    var td = element.parentNode
                    if (element.tagName != 'SPAN') {
                        if (td && td.tagName != 'TD') td = td.parentNode
                        if (td && td.tagName != 'TD') td = td.parentNode
                        if (td && td.tagName == 'TD') td.noWrap = true
                    }

                    if (Number(element.getAttribute('neosysgroupno')) && element.getAttribute('neosysalign') == 'R') {
                        element.parentNode.insertBefore(element2, null)
                    }
                    else {
                        element.parentNode.insertBefore(element2, element)
                    }

                    //element2.src='../neosys/images/smallopen.gif'
                    element2.src = glinkimage
                    //element2.style.border='solid 1px'
                    element2.style.verticalAlign = 'top'
                    element2.title = 'Open this ' + element.getAttribute('neosystitle') + ' (F6)'
                    element2.style.cursor = 'pointer'

                    //element2.onclick='neosyslink()'
                    //addeventlistener(element2,'click','neosyslink')
                    //element2.setAttribute('onclick','neosyslink()')
                    element2.setAttribute('isneosyslink', '1')

                }
            }

            //use the data field name as the id and name of the element
            //NB the name appears to be lost on databinding table rows
            element.id = fieldname

            //NAME attribute cannot be set at run time on elements dynamically
            // created with the createElement method
            //element.name=element.name

            //group no
            //var groupno=parseInt(element.getAttribute('neosysgroupno'),10)
            var groupno = Number(element.getAttribute('neosysgroupno'))

            /* no point to this since attributes can only be strings
            //function code
            if(typeof element.getAttribute('neosysfunctioncode')=='string'
            &&element.getAttribute('neosysfunctioncode')!='')
            {
            var tempfunction=null
            //    try
            //    {
            //cannot use neosysevaluate here unless perhaps the scope of tempfunction is global
            //neosysevaluate('new function tempfunction(){'+element.getAttribute('neosysfunctioncode')+'}','formfunctions_onload() '+element.id)

            //eval('new function tempfunction(){'+element.getAttribute('neosysfunctioncode')+'}')
            tempfunction=new Function(element.getAttribute('neosysfunctioncode'))
            //    }
            //    catch(e){}
            //NB assign regardless of any error above otherwise fails with "return outside function" later on
            element.setAttribute('neosysfunctioncode',tempfunction)
    
            //if (!tempfunction)
            //{
            // return logout('formfunctions_onload - error in '+element.id+'.functioncode')    
            //}
    
            }
            */

            //align
            //right align parent TD/TH if in columns (groupn>0)
            if (!element.getAttribute('neosysalign') && typeof (element.getAttribute('neosysconversion')) == 'string' && element.getAttribute('neosysconversion').indexOf('[NUMBER') >= 0)
                element.setAttribute('neosysalign', 'R')

            //if (groupno>0&&element.getAttribute('neosysalign')=='R'&&'THTD'.indexOf(element.parentNode.tagName)>=0)
            //if (element.getAttribute('neosysalign')=='R'&&'THTD'.indexOf(element.parentNode.tagName)>=0)
            if (element.getAttribute('neosysalign') == 'R' && (groupno > 0 || getancestor(element, 'TFOOT'))) {
                var cellnode = getancestor(element, ' TD TH ')
                if (cellnode && !cellnode.parentNode.align && !cellnode.align) cellnode.align = 'right'
            }

            //length and maxlength
            if (element.tagName.match(gtexttagnames)) {
                if (element.size != 1 && element.getAttribute('neosyslength')) {
                    if (!parseInt(element.getAttribute('neosyslength'))) {
                        systemerror('formfunctions_onload()', element.id + '.getAttribute("neosyslength")=' + element.getAttribute('neosyslength') + ' is invalid. 10 used.')
                        element.setAttribute('neosyslength', 10)
                    }
                    element.size = parseInt(element.getAttribute('neosyslength'), 10)
                    if (element.size > 2) element.size += 2
                }
                if (element.tagName == 'TEXTAREA') {

                    if (typeof CKEDITOR != 'undefined') {

                        var verticalpercent = 100
                        if (element.getAttribute('neosysrows'))
                            verticalpercent *= element.getAttribute('neosysrows') / 10

                        //var ockeditor = new CKEDITOR(element.id,'100%',verticalpercent+'%','NEOSYS')

                        //http://docs.cksource.com/Talk:CKEditor_3.x/Developers_Guide
                        ockeditor = CKEDITOR.replace(element.id)

                        //ockeditor.config.toolbar="Basic"
                        ockeditor.config.toolbarStartupExpanded = false
                        //element is passed as data to document_onfocus since the editor element
                        //doesnt seem to be treatable as a normal neosys dom element
                        ockeditor.on('focus', document_onfocus, null, element)

                        /*
                        ockeditor.Config.ToolbarStartExpanded=false
                        ockeditor.Config.AutoDetectLanguage=false
                        ockeditor.Config.DefaultLanguage="en-US"

                        ockeditor.Config.ToolbarSet='NEOSYS'

                        ockeditor.Config.ImageBrowser=false
                        ockeditor.Config.FlashBrowser=false
                        ockeditor.Config.LinkUpload=false
                        ockeditor.Config.ImageUpload=false
                        ockeditor.Config.FlashUpload=false

                        ockeditor.BasePath = "../NEOSYS/ckeditor/" ;

                        ockeditor.ReplaceTextarea() ;
                        */

                        //doesnt seem to be needed (actually crashes out)
                        //element.onfocus=onfocus
                    }

                    element.cols = element.size ? element.size : 30
                    if (element.getAttribute('neosysrows'))
                        element.rows = element.getAttribute('neosysrows')
                }
                if (element.getAttribute('neosysmaxlength'))
                    element.maxLength = parseInt(element.getAttribute('neosysmaxlength'), 10)
            }

            //lower case
            if (!element.getAttribute('neosyslowercase')) {
                if (element.tagName == 'SELECT'
    || element.getAttribute('neosysalign') == 'T'
    || element.getAttribute('neosystype') == 'S') {
                    element.setAttribute('neosyslowercase', true)
                }
                else {
                    element.setAttribute('neosyslowercase', '')// not 'false' as user properties must be strings not boolean for row bound elements
                }
            }
            //force visual uppercase (actual conversion done in onbeforeupdate)
            if (!element.getAttribute('neosyslowercase')
   && element.type != 'radio'
   && element.type != 'checkbox')
                element.style.textTransform = 'uppercase'

            //non calculated fields may be displayed as/converted to uppercase
            //if (element.getAttribute('neosystype')=='F'&&element.tagName!='SPAN')
            if (element.getAttribute('neosystype') == 'F') {

                //as of ie5 we can only focus on elements which have tabindex
                //make them all the same and tab will work nicely
                //tabindex can also be hard coded in the form design
                //use <999 to come before defaults and >999 to come after
                if (!element.tabIndex) {
                    if (element.getAttribute('neosysreadonly')) {
                        element.tabIndex = -1
                    }
                    else {
                        element.tabIndex = 999
                    }
                }

                setfirstlastelement(element)

            }

            //calculated fields can display HTML
            else {
                try {
                    element.dataFormatAs = 'HTML'
                }
                catch (e) {
                    systemerror('formfunctions_onload()', element.name + ' should not be bound to an editable element ' + element.tagName)
                }
            }

            //clear any existing values (f5 refresh does not seem to clear databinding)
            /////but now we bind an empty record so this is not necessary
            // if (element.tagName.match(gdatatagnames))
            // {
            //  setvalue(element,'')
            // }

            //highlight required fields
            var elementclassname = ''
            //if (!element.className&&element.type!='radio'&&element.type!='checkbox')
            if (element.tagName.match(gdatatagnames)
   && element.type != 'radio'
   && element.type != 'checkbox') {

                //spans are only input if .contentEditable
                if (element.getAttribute('neosystype') == 'F'
    && !element.getAttribute('neosysreadonly')
    &&
    (
    element.tagName != 'SPAN'
    || element.contentEditable == 'true')
    ) {
                    elementclassname = 'clsNotRequired'
                }
                else {
                    elementclassname = 'clsReadOnly'
                }

                if (element.getAttribute('neosysrequired')
                //mark elements with default values as required even though probably no data entry if required
                //&&!element.getAttribute('neosysdefaultvalue')
    && element.type != 'radio'
    && element.type != 'checkbox') {
                    if (element.tagName != 'SELECT'
     ||
     (
     element.tagName == 'SELECT'
     && element.options[element.selectedIndex].value == '')
     ) {
                        if (!Number(element.getAttribute('neosysgroupno')) || element.getAttribute('neosysrowrequired')) {
                            elementclassname = 'clsRequired'
                            //      element.style.border='solid 1px'
                            //      element.style.backgroundColor='#f0f0f0'
                        }
                    }
                }
            }
            //set the class
            if (elementclassname) {
                if (element.className)
                    element.className = element.className + ' ' + elementclassname
                else
                    element.className = elementclassname
            }

            //provide a way to find all neosys elements
            //if (document.getElementsByClassName)
            //{
            // var tt=element.className
            // element.className='neosysdb'+(tt?(' '+tt):'')
            //}

            //handle groups

            //build a list of name elements
            var screenfieldno = gfields.length
            gfields[screenfieldno] = element
            //element.setAttribute('neosysscreenfieldno',gfields.length-1)
            //why -1 ???
            element.setAttribute('neosysscreenfieldno', screenfieldno)
            //gfields[screenfieldno].setAttribute('neosysscreenfieldno',screenfieldno)

            var rowx = getancestor(element, 'TR')
            var tablex = getancestor(rowx, 'TABLE')

            //set the datasrc of single valued elements
            if (groupno == 0) {

                //but allow it in THEAD/TFOOT elements!
                //prevent group 0 element in rows of a multivalued table
                //while (tablex)
                //{
                // if (Number(tablex.getAttribute('neosysgroupno'))) systemerror('formfunction_onload()',neosysquote(element.name)+' is not multivalued and cannot be in a multivalued TABLE')
                // tablex=getancestor(tablex,'TABLE')
                //}

                //prevent group 0 element in rows of a multivalued table
                if (rowx && Number(rowx.getAttribute('neosysgroupno')))
                    systemerror('formfunction_onload()', neosysquote(element.name) + ' is not multivalued and cannot be in a multivalued TABLE')

                datasrcelements[datasrcelements.length] = element

            }

            //set the data source of repeating lines
            else {

                if (groupno > gmaxgroupno) gmaxgroupno = groupno

                //locate the table element in the parents
                if (tablex == null) {
                    systemerror('formfunction_onload()', neosysquote(element.name) + ' is multivalued and must exist inside a TABLE element')
                }
                else {

                    //add sorting button to column title
                    var titleelement
                    if (1 && (titleelement = $$(element.id + '_title'))) {

                        var element2 = document.createElement('IMG')
                        //add the button right after the field

                        //put nowrap flag on TD in 3 levels
                        //var td=element.parentNode
                        //if (element.tagName!='SPAN')
                        //{
                        // if (td&&td.tagName!='TD') td=td.parentNode
                        // if (td&&td.tagName!='TD') td=td.parentNode
                        // if (td&&td.tagName=='TD') td.noWrap=true
                        //}

                        titleelement.insertBefore(element2, null)

                        element2.id = 'sortbutton_' + Number(element.getAttribute('neosysgroupno'))
                        //element2.name=element2.id
                        //element2.src='../neosys/images/smallsort.gif'
                        element2.src = gsortimage
                        element2.originalsrc = element2.src
                        element2.title = 'Sort by ' + element.getAttribute('neosystitle')
                        //element2=setAttribute(element2,'onclick="return sorttable(\''+element.id+'\')"')
                        //element2.setAttribute('onclick','return sorttable(\''+element.id+'\')')
                        //element2.onclick=sorttable
                        addeventlistener(element2, 'click', 'sorttable')
                        element2.sorttableelementid = element.id
                        gsortimages[element.id] = element2

                    }

                    //any element can determine if emptyrows are allowed
                    if (element.getAttribute('neosysallowemptyrows'))
                        tablex.neosysallowemptyrows = element.getAttribute('neosysallowemptyrows')

                    //any element can determine if a row is required
                    if (element.getAttribute('neosysrowrequired'))
                        tablex.neosysrowrequired = element.getAttribute('neosysrowrequired')

                    //need to know the last data entry column
                    if (!element.getAttribute('neosysreadonly')
     && element.tagName.match(gdatatagnames)) {
                        //spans are only input if .contentEditable
                        if (element.tagName != 'SPAN'
      || element.contentEditable == 'true') {
                            tablex.neosyslastinputcolumnscreenfieldno = element.getAttribute('neosysscreenfieldno')
                        }
                    }

                    //need to know the first data entry column
                    if (!tablex.neosysfirstinputcolumnscreenfieldno
     && !element.getAttribute('neosysreadonly')
     && element.tagName.match(gdatatagnames)) {
                        //spans are only input if .contentEditable
                        if (element.tagName != 'SPAN'
      || element.contentEditable == 'true') {
                            tablex.neosysfirstinputcolumnscreenfieldno = element.getAttribute('neosysscreenfieldno')
                            element.neosysisfirstinputcolumn = true
                        }
                    }

                    //prevent use of separator characters unless multiword
                    if (element.tagName != 'SELECT') {
                        var wordsep = element.getAttribute('neosyswordsep')
                        if (wordsep && element.getAttribute('neosysnwords') <= 1) {
                            var invalidchars = element.getAttribute('neosysinvalidcharacters')
                            if (!invalidchars)
                                invalidchars = ''
                            element.setAttribute('neosysinvalidcharacters', invalidchars + wordsep)
                        }
                    }

                    if (Number(tablex.getAttribute('neosysgroupno'))) {

                        //check that all fields in same table have same group no
                        //zzz should also check that the table has no other name in subtables
                        if (Number(tablex.getAttribute('neosysgroupno')) != groupno) {
                            systemerror('formfunctions_onload()', element.name + ' is in group ' + groupno + ' but the table has already been bound to group ' + Number(tablex.getAttribute('neosysgroupno')))
                            return
                        }

                        //tablex.ncols++
                        //element.coln=tablex.ncols
                        gtables[groupno][gtables[groupno].length] = screenfieldno

                        //hide insertrow buttons (in case first element does not have the flag)
                        if (element.getAttribute('neosysnoinsertrow') & !tablex.noinsertrow) {
                            tablex.noinsertrow = true
                            var temp = $$('insertrowbutton' + groupno)
                            if (temp)
                                temp.style.display = 'none'
                        }

                        //hide deleterow buttons (in case first element does not have the flag)
                        if (element.getAttribute('neosysnodeleterow')
      && !tablex.nodeleterow) {
                            tablex.nodeleterow = true
                            var temp = $$('deleterowbutton' + groupno)
                            if (temp)
                                temp.style.display = 'none'
                        }

                    }
                    else {

                        //first column is required
                        //this should perhaps not be set since we have rowrequired and allowemptyrows
                        //element.setAttribute('neosysrequired',true)

                        //check this groupno not used on other tables
                        if (gtables[groupno]) {
                            systemerror('formfunctions_onload()', element.name + ' is in group ' + groupno + ' but that group is also used in another table by ' + gfields[gtables[groupno][0]].id)
                            return
                        }

                        //tablex.ncols=0
                        gtables[groupno] = []
                        gtables[groupno][0] = screenfieldno
                        gtables[groupno].tableelement = tablex
                        tablex.setAttribute('neosysgroupno', groupno)
                        tablex.setAttribute('name', 'group' + groupno)
                        tablex.id = 'neosysgroup' + groupno
                        datasrcelements[datasrcelements.length] = tablex
                        tablex.setAttribute('neosysdependents', '')
                        if (element.getAttribute('neosysnoinsertrow'))
                            tablex.noinsertrow = true
                        if (element.getAttribute('neosysnodeleterow'))
                            tablex.nodeleterow = true
                        //      if (!tablex.className) tablex.className='neosystable'
                        //      tablex.border=1
                        //capture all double clicks for potential filtering
                        //tablex.ondblclick=form_ondblclick
                        addeventlistener(tablex, 'dblclick', 'form_ondblclick')
                        //tablex.oncontextmenu=form_onrightclick

                        //tablex.onreadystatechange=tablex_onreadystatechange
                        addeventlistener(tablex, 'readystatechange', 'tablex_onreadystatechange')

                        //mark group in row to allow check/prevent group0 in same row
                        rowx.setAttribute('neosysgroupno', groupno)

                        //add insert and delete row buttons at the first column in the tbody

                        //button shortcut keys are ctrl+ on mac and alt+ on pc
                        var t2
                        if (isMac) {
                            t2 = 'x'
                            t3 = 'x'
                        }
                        else {
                            t2 = '(Ctrl+N)'
                            t3 = '(Ctrl+D)'
                        }
                        var t = ''
                        t += ' <span style="white-space: nowrap">'
                        //if (!neosysgetattribute(element,'neosysnoinsertrow'))
                        if (!element.getAttribute('neosysnoinsertrow'))
                            t += '  <IMG id=insertrowbutton' + groupno + ' alt="Insert a new row here ' + t2 + '" id=insertrowbutton onclick="insertrow(event)" src="' + ginsertrowimage + '" style="cursor:pointer;">'//: solid 1px">'
                        //if (!neosysgetattribute(element,'neosysnodeleterow'))
                        if (!element.getAttribute('neosysnodeleterow'))
                            t += '  <IMG id=deleterowbutton' + groupno + ' alt="Delete this row ' + t3 + '" id=deleterowbutton onclick="deleterow(event)" src ="' + gdeleterowimage + '" style="cursor:pointer">'//: solid 1px">'
                        t += ' </span>'
                        var insertdeletebuttons = document.createElement('TD')
                        insertdeletebuttons.innerHTML = t

                        //locate the TR element in the parents
                        var trx = getancestor(element, 'TR')
                        if (trx == null) {
                            systemerror('formfunction_onload()', neosysquote(element.name) + ' is multivalued and must exist inside a TABLE element')
                        }
                        else {
                            trx.insertBefore(insertdeletebuttons, trx.firstChild)
                        }

                        //add page up/down buttons at the first column in the thead and tfoot
                        var pgupdownbuttons = document.createElement('TH')
                        pgupdownbuttons.width = '1%'

                        var t = '&nbsp;'
                        if (tablex.dataPageSize) {
                            t = '<span id="currentpageno"></span>'
                            t += '<span style="white-space: nowrap">'
                            t += ' <button class="neosysbutton" id="firstpage" onclick="return changepage(0)" title="First page (Ctrl+PgUp)" '
                            t += ' >&lt;&lt;</button>'
                            t += ' <button class="neosysbutton" id="previouspage" onclick="return changepage(-1)" title="Previous page (PgUp)" '
                            t += ' >&lt;</button>'
                            t += ' <button class="neosysbutton" id="nextpage" onclick="return changepage(1)" title="Next page (PgDn)" '
                            t += ' >&gt;</button>'
                            t += '</span>'
                            t += ' <button class="neosysbutton" id="lastpage" onclick="return changepage(9999)" title="Last page (Ctrl+PgDn)" '
                            t += ' >&gt;&gt;</button>'
                            t += '</span>'
                        }

                        t += '<button id=neosysgroup' + groupno + 'showall'
                        t += ' style=display:none onclick="form_filter(\'unfilter\',' + groupno + ')"'
                        t += '>Show All</button>'

                        if (groupno == 1 && typeof gallowfilter != 'undefined' && gallowfilter) {
                            t += '<input id="neosysgroup' + groupno + 'filter"'
                            t += ' class="clsNotRequired"'
                            t += ' style="display:inline"'
                            t += ' onblur="form_filter(\'filterall\',' + groupno + ',null,null,this)"'
                            t += ' onfocus="form_filter(\'filterfocus\',' + groupno + ',null,null,this)"'
                            t += ' contentEditable="true"'
                            t += ' size="3"'
                            t += ' tabIndex="99999"'
                            t += ' />'
                        }

                        pgupdownbuttons.innerHTML = t

                        //locate the THEAD element in the parents
                        var thx = tablex.getElementsByTagName('THEAD')[0]
                        if (!thx) {
                            //taken out 30/6/2001 to allow simpler forms eg Trial Balance Printout
                            //       alert(element.name+' Multivalued column TABLE must have a THEAD element')
                        }
                        else {

                            //allow for multiple rows in thead
                            //macie needs to be updated AFTER insertion of element
                            //pgupdownbuttons.rowSpan=thx.children.length
                            if (!thx.firstChild) {
                                //ignore thead with no rows
                                //systemerror('Missing table structure for multivalued field?')
                                thx = null
                            }
                            else {
                                var thxr = thx.getElementsByTagName('tr')[0]
                                //var thxrd=thxr.getElementsByTagName('td')[0]
                                var thxrd = thxr.firstChild
                                thxr.insertBefore(pgupdownbuttons, thxrd)
                                //pgupdownbuttons.rowSpan=thx.childNodes.length
                                pgupdownbuttons.rowSpan = thx.rows.length
                            }
                        }

                        //locate the TFOOT element in the parents
                        var thx = tablex.getElementsByTagName('TFOOT')[0]
                        if (!thx) {
                        }
                        else {
                            pgupdownbuttons = pgupdownbuttons.cloneNode(true)
                            pgupdownbuttons.innerHTML = '&nbsp;'

                            //allow for multiple rows in thead
                            var thxr = thx.getElementsByTagName('tr')[0]
                            //var thxrd=thxr.getElementsByTagName('td')[0]
                            var thxrd = thxr.firstChild
                            thxr.insertBefore(pgupdownbuttons, thxrd)
                            //pgupdownbuttons.rowSpan=thx.childNodes.length
                            pgupdownbuttons.rowSpan = thx.rows.length

                        }

                    }
                }
            }
        } //name element
    }

    //set classname so that we can find multiple elements (screen fields) for the same database field name
    //(which occurs in the case of multivalues (rows), radio buttons and checkboxes)
    //using document.getElementsByClassName()
    id2classname()

    //zero all the dependents (nb neosysdependents is used in gds.js once)
    for (var ii = 0; ii < gfields.length; ii++)
        gfields[ii].setAttribute('neosysdependents', '')

    //for each calculated field add its field number
    // to all elements whose name (in double or single quotes)
    // appear in its functioncode
    for (var fieldn = 0; fieldn < gfields.length; fieldn++) {
        //with (gfields[fieldn])
        var field = gfields[fieldn]
        {
            var deplist = ''
            //   alert(gfields[fieldn].outerHTML)
            if (field.getAttribute('neosystype') == 'S') {
                var functioncode = field.getAttribute('neosysfunctioncode').toString()
                for (var fieldn2 = 0; fieldn2 < gfields.length; fieldn2++) {
                    var field2 = gfields[fieldn2]
                    if (functioncode.indexOf('"' + field2.getAttribute('neosysname') + '"', 0) >= 0
     || functioncode.indexOf("'" + field2.getAttribute('neosysname') + "'", 0) >= 0) {
                        var deps = field2.getAttribute('neosysdependents')
                        if (deps)
                            deps += ';'
                        else
                            deps = ''
                        deps += fieldn
                        field2.setAttribute('neosysdependents', deps)

                        if (Number(field2.getAttribute('neosysgroupno'))
      && Number(field.getAttribute('neosysgroupno')) != Number(field2.getAttribute('neosysgroupno'))) {
                            var tablex = getancestor(field2, 'TABLE')
                            //if (tablex.neosysdependents!='')
                            // tablex.neosysdependents+=';'
                            //tablex.neosysdependents+=fieldn
                            var deps = tablex.getAttribute('neosysdependents')
                            if (deps)
                                deps += ';'
                            else
                                deps = ''
                            deps += fieldn
                            tablex.setAttribute('neosysdependents', deps)
                        }

                    }
                }
            }
        }
    }

    //get copies of the gfields in case they are deleted from the document (eg table row 1 is deleted)
    for (var fieldn = 0; fieldn < gfields.length; fieldn++)
        gfields[fieldn] = gfields[fieldn].cloneNode(false)

    if (!gfirstnonkeyelement)
        gfirstnonkeyelement = gfields[0]
    if (!gfirstnonkeyelement) {
        systemerror('formfunctions_onload()', 'There are no non-key elements in the form or no data elements')
        return
    }

    //do not do the following otherwise if a grouped checkbox is the firstnokeyelement
    //then there is a failure in getvalue because it cannot find the parentNode
    //name=gfirstnonkeyelement.name
    //gfirstnonkeyelement=gfirstnonkeyelement.cloneNode(false)
    //gfirstnonkeyelement.name=name

    //when user enters the key, get a record

    //add the open, save, close and delete buttons

    //button shortcut keys are ctrl+ on mac and alt+ on pc
    var AltorCtrl = isMac ? 'Ctrl' : 'Alt'

    //tabindex buttons at 9999 to come after other fields at 999 
    var tt = ''
    //tt+='<div style="float:left; white-space: nowrap">'
    var buttonalign
    if (gKeyNodes.length) {
        buttonalign = 'left'
        tt += '<div style="float:' + buttonalign + ';">'
    }
    else {
        buttonalign = 'center'
        tt += '<div style="text-align:center;">'
    }

    /*
    newrecord.onclick=newrecord_onclick
    openrecord.onclick=openrecord_onclick
    releasereopen.onclick=releasereopen_onclick

    saverecord.onclick=saverecord_onclick
    closerecord.onclick=closerecord_onclick
    */

    if (gKeyNodes) {
        tt += menubuttonhtml('newrecord', gnewimage, '<u>N</u>ew', 'Create a new document. ' + AltorCtrl + '+N', 'N')
        tt += menubuttonhtml('openrecord', gopenimage, '<u>O</u>pen', 'Open an existing document. ' + AltorCtrl + '+O', 'O')
        tt += menubuttonhtml('releasereopenrecord', greleaseimage, '<u>R</u>elease', 'Release/Reopen the current document. ' + AltorCtrl + '+R', 'R')
    }
    else {
        newrecord = ''
        openrecord = ''
        releasereopenrecord = ''
    }
    tt += menubuttonhtml('saverecord', gsaveimage, '<u>S</u>ave', 'Save the current document. ' + AltorCtrl + '+S, Ctrl+Enter or F9', 'S', buttonalign)
    tt += menubuttonhtml('closerecord', gcloseimage, '<u>C</u>lose', 'Close the current document. ' + AltorCtrl + '+C', 'C', buttonalign)
    if (typeof form_oncopy == 'function')
        tt += menubuttonhtml('copyrecord', gcopyimage, 'Copy', 'Copy the current document.', '', buttonalign)
    if (gKeyNodes && (typeof gpreventdeletion == 'undefined' || gpreventdeletion != false))
        tt += menubuttonhtml('deleterecord', gdeleteimage, '<u>D</u>elete', 'Delete the current document. ' + AltorCtrl + '+D', 'D', buttonalign)
    else
        deleterecord = ''
    if (gKeyNodes && gKeyNodes[0].getAttribute('neosysprintfunction'))
        tt += menubuttonhtml('printsendrecord', gprintsendimage, '<u>P</u>rint/Send', 'Print/Send the current document. ' + AltorCtrl + '+P', 'P', buttonalign)
    else
        gprintsendrecord = false
    if (gKeyNodes) {
        var tt2 = gKeyNodes[0].getAttribute('neosyslistfunction')
        if (tt2) {
            tt += menubuttonhtml('listrecord', glistimage, 'L<u>i</u>st', 'List the current file. ' + AltorCtrl + '+I', 'I', buttonalign)
        }
        else
            listrecord = false
    }

    tt += menubuttonhtml('firstrecord', gfirstimage, ' ', 'Open the first document. ' + AltorCtrl + '+{', '{', buttonalign)
    tt += menubuttonhtml('previousrecord', gpreviousimage, ' ', 'Open the previous document. ' + AltorCtrl + '+[', '[', buttonalign)
    tt += menubuttonhtml('selectrecord', gspacerimage, ' ', 'Select document. ' + AltorCtrl + '+^', '^', buttonalign)
    tt += menubuttonhtml('nextrecord', gnextimage, ' ', 'Open the next document. ' + AltorCtrl + '+]', ']', buttonalign)
    tt += menubuttonhtml('lastrecord', glastimage, ' ', 'Open the last document. ' + AltorCtrl + '+}', '}', buttonalign)

    tt += '</div>'

    //login details
    var loginhtml = ''
    //loginhtml+='<div style="float:left; white-space:nowrap">'
    loginhtml += '<div style="float:left">'
    loginhtml += gdatasetname + ' - ' + gdataset + ' - ' + gusername + '</div>'
    if (gKeyNodes)
        tt += loginhtml

    //create the button rank and insert it into the form
    //var formbuttons=document.createElement(gKeyNodes?'SPAN':'DIV')
    var formbuttons = document.createElement('DIV')
    formbuttons.style.float = 'left'
    formbuttons.innerHTML = tt

    //insert a div that has NO boxes to the left of it so the form (title especially)
    //cannot appear to right of the (left floating) menu buttons
    var temp = document.createElement('DIV')
    temp.style.clear = 'left'
    document.body.insertBefore(temp, document.body.firstChild)

    if (gKeyNodes)
        document.body.insertBefore(formbuttons, document.body.firstChild)
    else {

        //login status
        var temp = document.createElement('DIV')
        temp.style.float = 'left'
        temp.innerHTML = loginhtml
        //temp.style.textAlign='CENTER'
        document.body.insertBefore(temp, document.body.firstChild)

        //formbuttons.align='CENTER'
        //NB there is no float "center"
        //formbuttons.style.float='center'
        formbuttons.style.textAlign = 'center'
        formbuttons.style.marginTop = '5px'
        document.body.insertBefore(formbuttons, null)

    }
    formbuttons.id = 'formbuttonsdiv'

    //make global variables to correspond to the buttons
    //to provide backward compatibility with IE code which can refer to document elements like global variables
    newrecord = $$('newrecordbutton')
    openrecord = $$('openrecordbutton')
    releasereopenrecord = $$('releasereopenrecordbutton')
    //
    firstrecord = $$('firstrecordbutton')
    previousrecord = $$('previousrecordbutton')
    selectrecord = $$('selectrecordbutton')
    nextrecord = $$('nextrecordbutton')
    lastrecord = $$('lastrecordbutton')
    //
    saverecord = $$('saverecordbutton')
    closerecord = $$('closerecordbutton')
    copyrecord = $$('copyrecordbutton')
    deleterecord = $$('deleterecordbutton')
    gprintsendrecord = $$('printsendrecordbutton')

    //program the various buttons to be visible when enabled
    neosyssetexpression(saverecord, 'style:display', 'saverecord.getAttribute("disabled")?"none":"inline"')
    neosyssetexpression(closerecord, 'style:display', 'closerecord.getAttribute("disabled")?"none":"inline"')

    if (gprintsendrecord)
        neosyssetexpression(gprintsendrecord, 'style:display', 'gprintsendrecord.getAttribute("disabled")?"none":"inline"')

    if (gKeyNodes) {
        neosyssetexpression(newrecord, 'style:display', 'newrecord.getAttribute("disabled")?"none":"inline"')
        neosyssetexpression(releasereopenrecord, 'style:display', 'releasereopenrecord.getAttribute("disabled")?"none":"inline"')
        if (deleterecord)
            neosyssetexpression(deleterecord, 'style:display', 'deleterecord.getAttribute("disabled")?"none":"inline"')
    }

    //program the first, previous, next and last record buttons to be visible when required
    if (firstrecord) {
        var setnone = !firstrecord.style.setExpression

        //ie8 leaves the function in but throws an error when used?!
        //so cant use "if (firstrecord.setExpression)"
        if (!setnone) {
            try {
                var tt = 'gkeys.length>1?"inline":"none"';

                //this will fail where setExpression isnt supported
                //until we have a working neosyssetexpression then use failure to force execution of display:none below
                firstrecord.style.setExpression('display', tt);

                neosyssetexpression(firstrecord, 'style:display', tt);
                neosyssetexpression(previousrecord, 'style:display', tt);
                neosyssetexpression(selectrecord, 'style:display', tt);
                neosyssetexpression(nextrecord, 'style:display', tt);
                neosyssetexpression(lastrecord, 'style:display', tt);

                neosyssetexpression(firstrecord, 'disabled', 'gkeyn==0?true:false');
                neosyssetexpression(selectrecord, 'innerText', '(gkeyn+1)+" of "+gkeys.length');
                neosyssetexpression(lastrecord, 'disabled', '(gkeyn==(gkeys.length-1))?true:false');
            }
            catch (e) {
                setnone = true
            }
        }

        if (setnone) {
            firstrecord.style.display = 'none';
            previousrecord.style.display = 'none';
            selectrecord.style.display = 'none';
            nextrecord.style.display = 'none';
            lastrecord.style.display = 'none';
        }
    }

    if (greadonlymode || gupdateonlymode || gpreventcreation) {
        setdisabled(newrecord, true)
        newrecord.title = 'Creating new records is not allowed'
    }
    if (greadonlymode || gupdateonlymode || gpreventdeletion) {
        setdisabled(deleterecord, true)
        deleterecord.title = 'Deleting records is not allowed'
    }

    var openfunction = ''
    var popupfunction = ''
    if (gKeyNodes) {
        var nvisiblekeys = 0
        for (var keyn = 0; keyn < gKeyNodes.length; keyn++) {
            //find the first openfunction (visible or not)
            if (openfunction = gKeyNodes[keyn].getAttribute('neosysopenfunction'))
                break
            if (neosysenabledandvisible(gKeyNodes[keyn])) {
                //count the number of visible keys
                nvisiblekeys++
                //remember the first visible popupfunction
                if (!popupfunction)
                    popupfunction = gKeyNodes[keyn].getAttribute('neosyspopup')
            }
        }
        //if no openfunction and only one visible key with a popup function
        //then use the popup function as the open function
        if (!openfunction
  && nvisiblekeys == 1
  && popupfunction)
            openfunction = popupfunction
    }
    //hide the open button if no openfunction
    if (openfunction) {
        openrecord.neosyspopup = openfunction
        openrecord.style.display = ''
    }
    else {
        if (openrecord)
            openrecord.style.display = 'none'
    }

    //remove record orientated buttons if no key fields
    if (!gKeyNodes) {

        saverecord.tabIndex = 9999 - 1//before menu and logout  
        closerecord.tabIndex = 9999 - 1

        /*
        newrecord.style.display='none'
        openrecord.style.display='none'
        deleterecord.style.display='none'
        releasereopenrecord.style.display='none'
        //firstrecord.style.display='none'
        //nextrecord.style.display='none'
        //previousrecord.style.display='none'
        //lastrecord.style.display='none'
        */

        //saverecord.value='OK'
        //saverecord.value='OK'
        if (typeof gdisableok != 'undefined' && gdisableok) {
            setdisabled(saverecord, true)
            //saverecord.style.display='none'
        }
        else {
            setdisabled(saverecord, false)
            setgraphicbutton(saverecord, 'OK')
            saverecord.title = 'OK - Continue. ' + (gfields.length > 1 ? 'Ctrl+' : '') + 'Enter or F9'
            //saverecord.style.display='inline'
            saverecord.style.marginRight = '5px'
        }

        //change order of buttons ?
        //saverecord.swapNode(closerecord)
        //following does not work so comment out
        //var tempnode=saverecord.parentNode.replaceChild(closerecord,saverecord)
        //closerecord.parentNode.replaceChild(tempnode,closerecord)

        //no cancel button on main non-database forms
        //if (!gparameters.cancel&&typeof(form_write)=='function'||!window.dialogArguments)
        if (!gparameters.cancel && !window.dialogArguments)
            setdisabled(closerecord, true)
        else {
            //closerecord.value='Cancel'
            setgraphicbutton(closerecord, 'Cancel')
            closerecord.title = 'Cancel and exit. Esc'
            setdisabled(closerecord, false)
        }

    }

    var temp = document.createElement('DIV')
    //temp.innerHTML='<p align=center style="margin-top:5px"><small>Software by <a tabindex="-1" href="http://www.neosys.com">NEOSYS.COM</a></small></p>'
    document.body.insertBefore(temp, null)

    //start focused on the first key field
    if (gKeyNodes) {
        gstartelement = gKeyNodes[0]
        if (!neosysenabledandvisible(gstartelement))
            gstartelement = gfirstnonkeyelement
    }
    else
        gstartelement = gfirstnonkeyelement

    //many routines will exit immeditately if this is not set (to avoid further error messages)
    //moved down now ... after any form_init
    //ginitok=true
    //wstatus('')

    //set the document titles from the parameters
    if (gparameters.documenttitle) {
        document.title = gparameters.documenttitle
        gdoctitle = document.title
    }

    //if form has a custom postinit routine
    if (typeof form_postinit == 'function') {
        login('form_postinit before')
        neosysevaluate('form_postinit()', 'form_functions()')
        logout('form_postinit after')

        //reverse the effect of any setvalue commands in postinit
        gchangesmade = false

    }

    //set the title hx element
    gdoctitle = document.title
    var temp = $$('documenttitle')
    if (temp && temp.tagName)
        temp.innerHTML = document.title

    //get the record from the page URL if any
    /* 
    var key=window.location.toString().split('?')[1]
    if (key=='vid:1+1') key='' //'only in development
    if (key) key=unescape(key)
    */

    //many routines will exit immeditately if this is not set (to avoid further error messages)
    ginitok = true
    wstatus('')

    //record based forms
    if (gKeyNodes) {
        if (gparameters.key) {

            //allow multiple keys to be loaded
            if (typeof gparameters.key == 'object') {
                gkeys = gparameters.key
                gparameters.key = gparameters.key[0]
            }

            //necessary to initialise gds else any expressions relying on gds.data will
            //crash after this routine returns
            cleardoc()

            window.setTimeout('opendoc(' + neosysquote(gparameters.key.replace(/\\/g, '\\\\')) + ')', 10)
        }
        else {
            cleardoc()
        }
    }

    //non-record based forms can get parameters from URL or dialog arguments
    else {

        //initial form can come from a revstr
        if (gparameters.defaultrevstr) gro.defaultrevstr = gparameters.defaultrevstr

        cleardoc()

        validateall('filldefaults')

        grecn = null
        calcfields()
        updatedisplay()

    }

    if (gparameters.savebuttonvalue) {
        //saverecord.value=gparameters.savebuttonvalue
        setgraphicbutton(saverecord, gparameters.savebuttonvalue)
    }

    //moved to end of cleardoc
    /*
    //TODO convert all postpostread( to use this new hook function instead of timeout
    if (typeof form_postdisplay=='function')
    {
    grecn=null
    neosysevaluate2('form_postdisplay()','formfunctions_onload()')  
    }
    */

    logout('formfunctions_onload')

}

function setdisabled(element, truefalse) {
    if (!element)
        return
    if (truefalse) {
        element.disabled = true
        element.setAttribute('disabled', 'disabled')
        element.style.display = 'none'
    }
    else {
        element.disabled = false
        element.removeAttribute('disabled')
        element.style.display = ''
    }
}

function setfirstlastelement(element) {

    //discover first non key input element
    if (element.getAttribute('neosysfieldno') > 0 && !element.getAttribute('neosysreadonly')) {
        if (!gfirstnonkeyelement)
            gfirstnonkeyelement = element
        if (element.tabIndex < gfirstnonkeyelement.tabIndex)
            gfirstnonkeyelement = element
    }

    //discover last input element
    if (element.getAttribute('neosysfieldno') > 0 && !element.getAttribute('neosysreadonly')) {
        if (!gfinalinputelement)
            gfinalinputelement = element
        if (element.tabIndex >= gfinalinputelement.tabIndex)
            gfinalinputelement = element
    }

    return
}

function setfirstlastcolumn(groupno) {

    var tablex = $$('neosysgroup' + groupno)
    if (!tablex) neosysinvalid('setfirstlastcolumn() table' + groupno + ' is missing')

    for (var ii = 0; ii < gtables[groupno].length; ii++) {
        var screenfieldno = gtables[groupno][ii]
        //if (!gfields[screenfieldno].getAttribute('neosysreadonly')&&neosysenabledandvisible(gfields[screenfieldno].id)) tablex.neosyslastinputcolumnscreenfieldno=screenfieldno-1
        if (!gfields[screenfieldno].getAttribute('neosysreadonly') && neosysenabledandvisible(gfields[screenfieldno].id)) tablex.neosyslastinputcolumnscreenfieldno = screenfieldno
    }

}

function systemerror(functionname, errormsg) {
    if (typeof functionname == 'undefined') functionname = 'undefined'
    if (typeof errormsg == 'undefined') errormsg = ''
    alert('System Error in ' + functionname + '\n\n' + errormsg)
    //if (gstepping||(!ginitok&&gusername=='NEOSYS')) crashhere2
    if (gstepping || (gusername == 'NEOSYS') || (gdataset && gdataset.slice(-4) == 'TEST'))
        neosys_javascript_system_error
}

function updatedisplay(elements) {

    //seems to be only implemented for non-bound forms

    //be careful not to update elements as it may be a global object

    //option to do all elements recursively
    if (typeof elements == 'undefined') return updatedisplay(gfields)

    //do multiple elements recursively
    if (!elements.tagName) {
        for (var ii = 0; ii < elements.length; ii++)
            updatedisplay(elements[ii])
        return
    }

    var element = elements

    //only do elements with a conditional display expression
    if (!element.getAttribute('neosysdisplay'))
        return

    login('updatedisplay ' + element.id)

    //determine required display
    var display = gds.evaluate(element.getAttribute('neosysdisplay'))
    if (typeof display == 'undefined') {
        systemerror('updatedisplay(' + element.id + ')', '.display() returned undefined')
    }
    display = display ? 'inline' : 'none'

    //quit if display not changed
    if (element.style.display == display) return logout('updatedisplay ' + element.id + ' still ' + display)

    //hide all elements with the same id
    var subelements = $$(element.id)
    if (subelements && subelements.tagName)
        subelements = [subelements]
    for (var subn = 0; subn < subelements.length; subn++) {

        var subelement = subelements[subn]

        //radio elements are surrounded by DIV
        if ((subelement.type == 'radio' || subelement.type == 'checkbox') && subelement.parentNode.tagName == 'SPAN') subelement = subelement.parentNode

        //hide parent node if td or th with only one child
        var parent = subelement.parentNode
        if (parent.tagName.match(/^(TD)|(TH)$/)) {
            //IE5.5 why does a TD enclosing a DIV have two childnodes the second being blank???
            var nchildnodes = 0
            for (var ii = 0; ii < parent.childNodes.length; ii++) if (parent.childNodes[ii].tagName) nchildnodes++
            if (nchildnodes == 1) subelement = parent
        }

        subelement.style.display = display

    }

    //hide associated elements (same id plus '_title')
    var elements = $$(element.id + '_title')
    if (elements) {
        if (elements.tagName)
            elements = [elements]
        for (var ii = 0; ii < elements.length; ii++)
            elements[ii].style.display = display
    }

    logout('updatedisplay ' + element.id + ' became ' + display)

    return

}

function element_neosyssetdropdown(element, request) {

    assertelement(element, 'element_setdropdown', 'element')

    if (!element.getAttribute('neosysdropdown')) {
        systemerror('element_neosyssetdropdown()', element.id + ' has no dropdown')
        return
    }

    var dropdown = element.getAttribute('neosysdropdown').split(fm)
    var request = 'CACHE\r' + dropdown[0]
    var colarray = dropdown[1].split('\r')

    neosyssetdropdown(element, request, colarray)

}






function newrecordfocus() {
    //return
    login('newrecordfocus')
    if (is(gfirstelement) && gloaded && !gds.isnewrecord) {
        gnextelement = $$(gfirstelement)
        if (gnextelement[0])
            gnextelement = gnextelement[0]
    }
    var element = gnextelement
    if (element)
        gnextelement = ''
    else {
        if (gloaded || !gKeyNodes) {
            //element=gfirstnonkeyelement
            //element=document.activeElement
            element = eventtarget
            if (!element || !Number(element.getAttribute('neosysfieldno'))) {
                if (typeof gfirstnonkeyelement == 'string') {
                    gfirstnonkeyelement = $$(gfirstnonkeyelement)
                    if (gfirstnonkeyelement[0])
                        gfirstnonkeyelement = gfirstnonkeyelement[0]
                }
                element = gfirstnonkeyelement
            }
            //   assertelement(element,'newrecordfocus','gfirstnonkeyelement')
        }
        else {
            element = gKeyNodes[0]
            assertelement(element, 'newrecordfocus', 'gKeyNodes[0]')
        }
    }

    //check no required fields are missing in prior data
    //fill in any defaults where possible
    //do this BEFORE setting gpreviouselement as setdefault will overwrite it
    if (!gKeyNodes || glocked) {
        //check group 0 always
        checkrequired(gfields, element, 0)
    }

    //required so that if still focused on an element AFTER loading the record
    //then editing the field and moving onto another element causes a proper validateupdate
    //alert(element.id)
    gpreviouselement = $$(element.id)
    if (gpreviouselement[0])
        gpreviouselement = gpreviouselement[0]
    //why false??? if (false&&gpreviouselement)
    //false results in repetitive batch number changing still asking discard? when no changes made
    if (gpreviouselement) {
        setdefault(gpreviouselement)
        gpreviouselement = $$(element.id)
        if (gpreviouselement && gpreviouselement[0])
            gpreviouselement = gpreviouselement[0]
        gpreviousvalue = getvalue(gpreviouselement)
    }

    //prevent focussing on hidden keys
    if (!neosysenabledandvisible(element))
        element = gstartelement

    window.scrollTo(0, 0)

    // element.focus()
    tt = element
    if (tt.id)
        tt = tt.id
    window.setTimeout('focuson("' + tt + '")', 1)

    logout('newrecordfocus')

}

function gds_onreadystatechange() {

    if (gds.readystate != 'complete') return
    login('gds_onreadystatechange')
    grecn = null
    calcfields()
    newrecordfocus()
    logout('gds_onreadystatechange')
}

function tablex_onreadystatechange(event) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    //if need to focus on a table field then
    //do it when the table is complete

    if (eventtarget.readyState != 'complete') return

    login('tablex_onreadystatechange ' + Number(eventtarget.getAttribute('neosysgroupno')))

    newrecordfocus()

    logout('tablex_onreadystatechange ' + Number(eventtarget.getAttribute('neosysgroupno')))

}

function printsendrecord_onclick(event) {

    if (!event)
        event = window.event
    neosyscancelevent(event)

    //work out the print function else return
    var printfunction = gKeyNodes[0].getAttribute('neosysprintfunction')
    if (!printfunction)
        return
    //printfunction=printfunction.replace(/%KEY%/g,gkeyexternal)
    //journal print does not work properly so ...
    //NB case INsensitive
    printfunction = printfunction.replace(/%KEY%/gi, gkey)
    //alert('DEBUG: saoc')
    if (!saveandorcleardoc('PRINT')) {
        focusongpreviouselement()
        return
    }
    //alert('DEBUG: printfunction')
    neosysevaluate(printfunction, 'printsendrecord_onclick()')
    //window.setTimeout("neosysevaluate('"+printfunction+"','printsendrecord_onclick()')",100)

}

function listrecord_onclick(event) {

    if (!event)
        event = window.event

    neosyscancelevent(event)

    //work out the print function else return
    var listfunction = gKeyNodes[0].getAttribute('neosyslistfunction')
    if (!listfunction)
        return

    if (!saveandorcleardoc('PRINT')) {
        return
    }
    neosysevaluate(listfunction, 'listrecord_onclick()')

}

//'WINDOW ONBEFORE UNLOAD
//'''''''''''''''''''''''
function window_onbeforeunload(event) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    if (!ginitok) return

    login('window_onbeforeunload')

    gwindowunloading = true

    if (closedoc('WINDOWUNLOAD')) {
        //gwindowunloading=true
    }
    else {
        //set the following and the user can cancel the window unload event
        event.returnValue = 'WARNING: YOU HAVE NOT SAVED YOUR DATA!'
    }

    gwindowunloading = false

    logout('window_onbeforeunload')

}

//WINDOW_ONUNLOAD
/////////////////
function window_onunload() {

    if (!ginitok) return

    if (glocked)
        unlockdoc()

    gwindowunloading = true
    //savecache()
    gwindowunloading = false

    //save gdataset in case we are refreshing and the parent window isnt there to get it from
    neosyssetcookie('', 'NEOSYSlogincode', glogincode, 'logincode')

}

////////////////// DOCUMENT EVENTS /////////////////////

function document_onclick(event) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    log('document_onclick ' + eventtarget.id)

    if (eventtarget.getAttribute('isneosyspopup'))
        return neosyspopup(event)

    if (eventtarget.getAttribute('isneosyslink'))
        return neosyslink(event)

}

//DOCUMENT ON KEY DOWN
//////////////////////
var gonkeydown
function document_onkeydown(event) {
    //prevent concurrent keydown events in ff3
    if (gonkeydown)
        return neosyscancelevent(event)

    gonkeydown = true
    var result = document_onkeydown2(event)
    gonkeydown = false

    return result
}

function document_onkeydown2(event) {

    //http://www.quirksmode.org/js/keys.html

    /* other event pages
    http://www.quirksmode.org/js/events_compinfo.html#prop
    http://www.quirksmode.org/dom/w3c_events.html
    http://www.brainjar.com/dhtml/events/
    http://www.w3.org/TR/DOM-Level-2-Events/events.html
    http://www.oreillynet.com/pub/a/javascript/synd/2001/09/25/event_models.html?page=1
    http://www.quirksmode.org/js/events_advanced.html
    http://www.quirksmode.org/js/events_compinfo.html
    http://developer.apple.com/internet/webcontent/eventmodels.html
    http://www.quirksmode.org/js/introevents.html
    http://www.javascriptkit.com/dhtmltutors/domeventp2-1.shtml
    */
    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    log('document_onkeydown ' + eventtarget.id + ' ' + event.keyCode)

    //no processing if not initialised
    if (!ginitok)
        return

    //log(eventtarget.tagName+' '+event.keyCode)

    //must be before custom key handlers which may use gkey
    keycode = event.keyCode ? event.keyCode : event.which
    gkeycode = keycode
    if (gstepping)
        wstatus(gkeycode)

    //custom key handlers
    //must return false to prevent further action
    if (typeof form_onkeydown == 'function') {
        if (!form_onkeydown()) {
            return neosyscancelevent(event)
        }
    }

    gwindowunloading = false
    element = eventtarget
    // if (typeof(closerecord)=='undefined') return

    try {
        if (element.tagName == 'TD') {
            if (element.firstChild.tagName == 'SPAN') {
                element = element.firstChild
            }
        }
    }
    catch (e) { }

    //alert(gkeycode) 
    //refresh (F5) system key refresh (do manually because mac does not seem to bubble key events)
    if (keycode == 116) {
        if (isMac)
            document.location.reload(true)//true means from server, not cache
        return true
    }

    //close (F8)
    if (keycode == 119) {
        closedoc('CLEAR')
        return neosyscancelevent(event)
    }

    //alt+k or alt+home is goto top (but alt+home goes to home page and cannot be cancelled)
    if ((keycode == 75 && event.altKey) || (keycode == 36 && event.altKey)) {
        window.setTimeout('focuson(gstartelement.id)', 1)

        return neosyscancelevent(event)
    }
    //menu bar hot keys for non-msie
    if (event.altKey && !document.all && [77, 78, 76, 79, 83, 67, 82, 69, 73, 80, 88].neosyslocate(gkeycode)) {
        var found = true
        if (gkeycode == 77) {
            if (document.createEvent) {
                //mozilla http://developer.mozilla.org/en/docs/DOM:event.initMouseEvent
                var evt = document.createEvent("MouseEvents");
                evt.initMouseEvent("click", true, true, window, 0, 0, 0, 0, 0, false, false, false, false, 0, null);
                var cb = $$("menubutton_label");
                var canceled = !cb.dispatchEvent(evt);
            }
            else
            //msie
                window.setTimeout('menuonmouseover(null,$$("menubutton"),13)', 1)
        }
        //was done by accesskeys on hidden buttons but firefox requires shift+alt for access
        // unless configure http://kb.mozillazine.org/Ui.key.contentAccess
        /* dont use timeout since allows the user interface to resume and send ANOTHER event in FF3
        else if (gkeycode==76) window.setTimeout('neosyslogout_onclick()',1)
        else if (gkeycode==78) window.setTimeout('newrecord_onclick()',1)
        else if (gkeycode==79) window.setTimeout('openrecord_onclick()',1)
        else if (gkeycode==83) window.setTimeout('saverecord_onclick()',1)
        else if (gkeycode==67) window.setTimeout('closerecord_onclick()',1)
        else if (gkeycode==82) window.setTimeout('releasereopenrecord_onclick()',1)
        else if (gkeycode==69) window.setTimeout('deleterecord_onclick()',1)
        else if (gkeycode==73) window.setTimeout('listrecord_onclick()',1)
        else if (gkeycode==80) window.setTimeout('printsendrecord_onclick()',1)
        else if (gkeycode==88) window.setTimeout('refreshcache_onclick()',1)
        */
        else if (gkeycode == 76) neosyslogout_onclick()
        else if (gkeycode == 78) newrecord_onclick()
        else if (gkeycode == 79) openrecord_onclick()
        else if (gkeycode == 83) saverecord_onclick()
        else if (gkeycode == 67) closerecord_onclick()
        else if (gkeycode == 82) releasereopenrecord_onclick()
        else if (gkeycode == 69) deleterecord_onclick()
        else if (gkeycode == 73) listrecord_onclick()
        else if (gkeycode == 80) printsendrecord_onclick()
        else if (gkeycode == 88) refreshcache_onclick()
        else
            found = false
        if (found) {
            //try {gpreviouselement.focus()}
            //catch(e){}
            return neosyscancelevent(event)
        }
    }

    //alt+end is goto bottom
    if (keycode == 35 && event.altKey) {
        window.setTimeout('focuson(gfinalinputelement.id)', 1)
        return neosyscancelevent(event)
    }

    //F12 is neosys debug key
    //if (keycode==123/*&&gusername.indexOf('NEOSYS')+1*/)
    if (keycode == 123 && gusername.indexOf('NEOSYS') + 1) {

        //plain F12 - toggle stepping2
        if (!event.shiftKey && !event.ctrlKey && !event.altKey) {
            gstepping = !gstepping
            wstatus('stepping=' + gstepping)
            return neosyscancelevent(event)
        }

        //shift+ctrl+F12 - break
        //force error and into neosysbreak
        if (event.shiftKey && event.ctrlKey) {
            gstepping = true
            neosysbreak('', 'F12', '')
            return neosyscancelevent(event)
        }

        //shift+alt+F12 - show event log
        if (event.shiftKey && event.altKey) {
            glogging = true
            wstatus('glogging=true')
            if (geventlog) {
                windowx = windowopen()
                if (windowx)
                    windowx.document.body.innerHTML = geventlog
            }
            return neosyscancelevent(event)
        }

        temp = element

        //ctrl+F12 - element.parentNode.outerHTML
        var xxx
        if (xxx = (event.ctrlKey && !!temp.parentNode)) {
            temp = temp.parentNode
            //alt+F12 - element.parentNode.parentNode.outerHTML
            if (event.altKey && temp.parentNode)
                temp = temp.parentNode
        }

        if (temp && (temp.outerHTML || temp.innerHTML)) {
            var windowx = windowopen()
            if (windowx)
            //windowx.document.body.innerText=decodehtmlcodes(temp.outerHTML?temp.outerHTML:temp.innerHTML)
                windowx.document.body.innerHTML = encodehtmlcodes(temp.outerHTML ? temp.outerHTML : temp.innerHTML)
        }

        return neosyscancelevent(event)

    }

    //F2 is now edit to be like office
    var textrange
    if (keycode == 113
 && document.selection
 && document.selection.createRange
 && (textrange = document.selection.createRange()).text != '') {
        textrange.collapse(false)
        textrange.select()
        return neosyscancelevent(event)
    }

    //F6 is now link
    if (keycode == 117) {
        neosyslink(event)
        return neosyscancelevent(event)
    }

    //F7 is now popup (used to be F2 in arev) also replaces windows standard alt+down combination
    if (keycode == 118 || (event.altKey && keycode == 40 && element.tagName == 'SELECT')) {
        neosyspopup(event)
        return neosyscancelevent(event)
    }

    //get groupno otherwise return
    ggroupno = Number(element.getAttribute('neosysgroupno'))

    grecn = null
    if (ggroupno > 0) {

        //get the current table and ggroupno else exit
        var rowx = getancestor(element, 'TR')
        var tablex = getancestor(rowx, 'TABLE')
        if (!tablex)
            return
        var rows = tablex.tBodies[0].childNodes

        //get the rown else exit
        if (ggroupno > 0) {
            var rown = rowx.rowIndex
            if (tablex.tHead)
                rown -= tablex.tHead.rows.length
        }

        //get the recordset
        var rs = gds.data['group' + ggroupno]
        if (!rs) {
            neosyscancelevent(event)
            return neosysinvalid('Group number on non-group field')
        }
        //zzz if "paging" should be offset by subtracting record number of first row
        grecn = rown
        gnrecs = rs.length

        var nrows = rows.length
        var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
        var pagen = neosysint(grecn / pagesize)
        var npages = Math.ceil((rs.length) / pagesize)

    }

    //ctrl+G is goto line
    if (keycode == 71 & event.ctrlKey) {
        if (!validateupdate()) return false
        while (true) {

            var elements = ggroupno ? element : gfields[gtables[1][0]]
            elements = $$(elements.id)
            if (elements && elements.tagName)
                elements = [elements]

            var reply = window.prompt('Go to line number ? (1-' + elements.length + ')', grecn + 1)
            if (!reply)
                break

            reply = (+reply)
            if (reply < 1) reply = 1
            if (reply > elements.length)
                reply = elements.length

            focuson(elements[reply - 1])
            break

        }

        return neosyscancelevent(event)
    }

    //alt+Y is copy previous record/column
    if (keycode == 89 && event.altKey && element.type != 'button') {
        if (grecn > 0) {
            setvalue(element, getpreviousrow(element.id, true))
            try { element.select() } catch (e) { }
            return neosyscancelevent(event)
        }
    }

    //ctrl+home is goto first column
    if (tablex && keycode == 36 && event.ctrlKey) {
        var id = gfields[tablex.neosysfirstinputcolumnscreenfieldno]
        if (id)
            window.setTimeout('focuson("' + id.id + '")', 1)
        return neosyscancelevent(event)
    }

    //ctrl+end is goto last column
    if (tablex && keycode == 35 && event.ctrlKey) {
        var id = gfields[tablex.neosyslastinputcolumnscreenfieldno]
        if (id)
            window.setTimeout('focuson("' + id.id + '")', 1)
        return neosyscancelevent(event)
    }

    //Enter on textarea
    if (!isMac && keycode == 13 && (element.tagName == 'TEXTAREA' && !event.ctrlKey && !event.shiftKey)) {
        if (gKeyNodes.length && !glocked) {
            keycode = 9
            event.keyCode = keycode
        }
        if (element.value == '' || element.value.slice(-4) == '\r\n\r\n') {
            element.value = element.value.slice(0, -4)
            keycode = 9
            event.keyCode = keycode
        }
    }

    //enter key becomes tab (or ok if only one entry field)
    if (keycode == 13 && (element.tagName != 'TEXTAREA' || event.ctrlKey || event.shiftKey)) {

        //ctrl+enter or enter with single field is ok/save record
        //if (event.ctrlKey||(gfields.length==1&&Number(gfields[0].getAttribute('neosysgroupno'))))
        if (event.ctrlKey || (!Number(element.getAttribute('neosysgroupno')) && gfinalinputelement && gfinalinputelement.id == gstartelement.id)) {
            if (!saverecord.getAttribute('disabled')) {
                //savedoc()
                saverecord_onclick()
            }
            return neosyscancelevent(event)
        }

        //enter on last field goes to start of doc
        if (!event.shiftKey && element == gfinalinputelement && (ggroupno == 0 || (ggroupno > 0 && grecn == gnrecs))) {
            focuson(gstartelement)
            return neosyscancelevent(event)
        }

        //shift+enter on 1st key field goes to end of doc
        if (event.shiftKey && element == gstartelement) {
            focusdirection(-1, element)
            return neosyscancelevent(event)
        }

        //setting keyCode is only allowed on msie
        if (!document.all && !event.ctrlKey) {
            if (event.shiftKey)
                direction = -1
            else
                direction = 1
            focusdirection(direction, element)
            return neosyscancelevent(event)
        }

        //keyCode is readonly except on msie but focusdirection() isnt working yet due to lack of sourceIndex property
        keycode = 9
        event.keyCode = keycode
        //focusdirection(1,element)
        //return neosyscancelevent(event)

        //dont return because enter key has some special processing on rows below
        //return

    }

    //F9 is also save
    if (keycode == 120) {

        //first update the current field
        //otherwise the db is updated without the last entry!!!
        if (!validateupdate())
            return neosyscancelevent(event)

        //prevent document save unless the save button is enabled
        if (!saverecord.getAttribute('disabled')) {
            //savedoc()
            saverecord_onclick()
        }

        return neosyscancelevent(event)

    }

    //esc
    // if(keycode==27&&gloaded)
    if (keycode == 27) {

        //close any "modal" popups
        if (form_closepopups()) {
            //prevent normal esc handling
            return neosyscancelevent(event)
        }

        //if changing current field then let system use it to 'undo' changes
        if (element.name && element == gpreviouselement) {

            value = getvalue(element)

            //old value
            gdefaultvalue = getdefault(element)
            if (gpreviousvalue == '' && gdefaultvalue)
                gpreviousvalue = gdefaultvalue

            if (value != gpreviousvalue) {

                //restore the original value
                //get from datasource
                setvalue(element, gpreviousvalue)

                //prevent normal esc handling
                neosyscancelevent(event)

                //select it
                try {
                    if (element.select)
                        element.select()
                }
                catch (e) { }

                return neosyscancelevent(event)
            }
        }

        closerecord_onclick()

        return neosyscancelevent(event)

    }

    //all remaining key events are related to loaded records
    if (gKeyNodes && closerecord.getAttribute('disabled'))
        return

    //esc when no changes means closerecord

    //the rest of the keys are
    if (typeof ggroupno == 'undefined' || ggroupno == null)
        return

    //up and down keys
    if (keycode == 38 || keycode == 40) {

        //on textarea leave system to process it
        if (element.tagName == 'TEXTAREA')
            return

        //leave system to process alt+down
        //cannot do this because up/down is then used to select
        //but up/down is required to move between rows
        //if (event.altKey&&keycode==40&&element.tagName=='SELECT') return

        //on radio button leave system to process it
        //unless up on first element or down on last element
        if (element.type == 'radio' || element.type == 'checkbox') {
            var elements = $$(element.id)
            if (elements && elements.tagName)
                elements = [elements]
            if (keycode == 38 && element != elements[0])
                return
            if (keycode == 40 && element != elements[elements.length - 1])
                return
        }

        //alt+down (or alt+up!) implements popups
        if (event.altKey && element.getAttribute('neosyspopup')) {
            neosyspopup()
            return neosyscancelevent(event)
        }

    }

    //left arrow or right arrow and whole field is selected else return
    if (keycode == 37 || keycode == 39) {

        //allow alt+left and right to do usual IE navigation
        if (event.altKey)
            return true

        //suppress left right if text not wholy selected and maxlen!=1
        if (isMac) {
            //mac docs say document.selection object works but it is null
            var selection = document.selection
            if (selection)
                selection = selection.type
            if (element.tagName.match(gtexttagnames) && element.type != 'button' && selection != 'Text')
                return
        }
        else {
            if (document.selection && element.maxLength != 1) {
                var textrange = document.selection.createRange()
                //var textrange=document.forms[0].selection.createRange()
                if (element.tagName.match(gtexttagnames) && element.type != 'button' && textrange.text != getvalue(element))
                    return
            }
        }

        //left or right not in tables
        if (ggroupno == 0) {
            if (keycode == 37)
                focusprevious()
            else
                focusnext()
            return neosyscancelevent(event)
        }

    }

    if (keycode == 40) {
        //event.returnValue=false
        //event.cancelBubble=true
        //return false
        //neosyscancelevent(event)
    }

    //up down keys outside a table are like left/right
    //if((keycode==38||keycode==40)&&ggroupno==0&&element.tagName!='SELECT'&&element.type!='radio')
    if ((keycode == 38 || keycode == 40) && ggroupno == 0) {

        //option to revert to windows standard (eg to show rapid effect of different values)
        if (element.tagName == 'SELECT'
  && element.getAttribute('neosysallowcursor'))
            return true

        if (keycode == 38)
            focusprevious()
        else
            focusnext()

        //prevent the key from being processed as a cursor movement
        return neosyscancelevent(event)

    }

    //tab or down on first or last col of LAST row is special
    if ((ggroupno > 0) && grecn == (rs.length - 1)) {
        if ((keycode == 9 || keycode == 40) && !event.shiftKey) {
            //on last column - add a row
            var firstcolelement = rowx.neosysfields[gfields[gtables[ggroupno][0]].id]
            if ((glocked
   || !gKeyNodes
   )
   && element.getAttribute('neosysscreenfieldno') == tablex.neosyslastinputcolumnscreenfieldno
   && getvalue(firstcolelement) != '') {
                if (glocked || !gKeyNodes) {
                    //neosysaddrow(ggroupno)
                    if (!insertrow(event, true))
                        return neosyscancelevent(event)
                    //continue on to focus on it
                }
            }
            //on first column, if empty - go to next field after current table
            if (element.neosysisfirstinputcolumn && getvalue(element) == '') {
                focusdirection(1, element, ggroupno)
                return neosyscancelevent(event)
            }
        }
    }

    //left arrow and right arrow in tables
    if (ggroupno != 0 && rown >= 0 && (keycode == 37 || keycode == 39)) {
        var scope = rows[grecn].getElementsByTagName('*')
        if (keycode == 37)
            focusprevious(element, scope)
        else
            focusnext(element, scope)
        return neosyscancelevent(event)
    }

    //PGUP/PGDN/UP/DOWN
    if (keycode == 34 || keycode == 33 || keycode == 40 || keycode == 38 || keycode == 37 || keycode == 39) {

        if (ggroupno == 0)
            return
        if (rown < 0)
            return//must be in a thead row

        //if down arrow on last row then add a new row
        if (keycode == 40 && grecn == (rows.length - 1)) {

            if (glocked || !gKeyNodes) {
                //neosysaddrow(ggroupno)
                insertrow(event, true)
                //and continue to focus on it
            }
        }

        var nrows = rows.length
        var id = element.id
        if (!id)
            return//neosys like button with no id?

        var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
        var pagen = neosysint(grecn / pagesize)
        var npages = Math.ceil((rs.length) / pagesize)

        //pgdn or down arrow
        if (keycode == 34 || keycode == 40) {

            //ctrl+pgdn or down arrow goes to last row of last page
            if (event.ctrlKey) {
                //tablex.lastPage()
                focuson(rows[rows.length - 1].neosysfields[id])
            }

            //if not on last row of page then goto last row
            else {

                if (rown < nrows - 1) {
                    if (keycode == 40) {
                        if (!rows[rown + 1].neosysfields[id]) {
                            systemerror('document_onkeydown()', 'Cannot locate id ' + id + ' in rown: ' + rown + ' nrows:' + rows.length + ' tagname:' + rows[rown + 1].neosysfields[id])
                            return false
                        }
                        focuson(rows[rown + 1].neosysfields[id])
                    }
                    else {
                        var newrown = grecn + gpagenrows
                        if (newrown > (nrows - 1))
                            newrown = nrows - 1
                        focuson(rows[newrown].neosysfields[id])
                    }
                }

                //if on last row go to first row of next page if any, otherwise stay put
                else {

                    if (pagen < npages - 1) {
                        //tablex.nextPage()
                        focuson(rows[0].neosysfields[id])
                    }
                }
            }

        }

        //pgup or up arrow
        if (keycode == 33 || keycode == 38) {
            //ctrl+pgup or up
            //goes to first line of first page
            if (event.ctrlKey) {
                //tablex.firstPage()
                focuson(rows[0].neosysfields[id])
            }
            else {

                //if not on first row of page go up one row (up arrow) or to first line (pgup)
                if (rown > 0) {
                    if (keycode == 38) {
                        focuson(rows[rown - 1].neosysfields[id])
                    }
                    else {
                        var newrown = grecn - gpagenrows
                        if (newrown < 0)
                            newrown = 0
                        //focuson(rows[newrown].neosysfields[id])
                        if (document.getElementsByClassName)
                            focuson(rows[newrown].neosysfields[id])
                        else
                            focuson(rows[newrown].all[id])
                    }
                }

                //if on first line of page go to last line of previous page if any, or stay put
                //(same for pgup and up)
                else {
                    if (grecn == 0) {
                        focusprevious(element)
                    }
                    else {
                        //tablex.previousPage()
                        focuson(rows[nrows - 1].neosysfields[id])
                    }
                }
            }
        }

        return neosyscancelevent(event)

    }

    //following are all keycodes when within tabular rows section
    //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

    //defeat holding insert/delete row key down
    if (!element.getAttribute('neosystype'))
        return neosyscancelevent(event)

    //what does this accomplish?
    //if the element has no value, the user must have selected many elements
    //try
    //{
    // var srcvalue=getvalue(element)
    //}
    //catch(e){}

    //row insert and delete
    if (ggroupno > 0) {
        var button

        //Ctrl+N is insert row
        if (event.ctrlKey && keycode == 78) {
            button = rowx.neosysfields['insertrowbutton' + ggroupno]
            if (button && button.style && button.style.display != 'none')
                insertrow(event, false)
            return neosyscancelevent(event)
        }

        //Ctrl+D is delete row
        if (event.ctrlKey && keycode == 68) {
            button = rowx.neosysfields['deleterowbutton' + ggroupno]
            if (button && button.style && button.style.display != 'none')
                deleterow(event, eventtarget)
            return neosyscancelevent(event)
        }

    }

    //protect readonly fields
    if (!(event.altKey || event.ctrlKey)
 && (element.getAttribute('neosysreadonly')
 || (gKeyNodes
 && !glocked
 && element
 && element.getAttribute('neosysfieldno')
 && element.getAttribute('neosysfieldno') != 0
 ))) {
        if (![9, 16, 17, 18, 20, 35, 36, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123].neosyslocate(keycode)) {
            if (gkeys.length && !glocked)
                readonlydocmsg()
            else if (element.getAttribute('neosysreadonly') != 'true')
                neosysinvalid(element.getAttribute('neosysreadonly'))
            return neosyscancelevent(event)
        }
    }

}

function focusprevious(element, scope) {
    focusdirection(-1, element, '', scope)
}

function focusnext(element, scope) {
    focusdirection(1, element, '', scope)
}

function focusdirection(direction, element, notgroupno, scopex) {
    //currently required sourceIndex which is msie only
    if (!document.body.sourceIndex)
        return

    if (typeof notgroupno == 'undefined')
        notgroupno = ''

    //find next input field
    var nextelement = element
    if (!nextelement) {
        //if (!window.event) return
        //nextelement=eventtarget
        nextelement = window.event.srcElement
        if (!nextelement)
            return
    }
    // if (!element) return
    var elementtabindex = element ? element.tabIndex : 999999999999

    if (typeof scopex == 'undefined')
        scope = document.all
    else scope = scopex

    //check scope. why?
    var sourceindex = nextelement.sourceIndex
    var scopeindex = sourceindex - scope[0].sourceIndex
    if (scope[scopeindex] != nextelement) {
        //wrongscopeindex()
        focuson(nextelement)
        return
    }

    //test to prevent endless loop
    var passzero = scopeindex

    do {
        var scopeindex = scopeindex + direction

        //prevent infinite loop
        if (passzero == scopeindex) {
            //break
            return
        }

        //wrap/scroll down to the bottom
        if (scopeindex < 0) {
            if (typeof scopex == 'undefined') window.scrollBy(0, 999999)
            scopeindex = scope.length - 1
        }

        //wrap/scroll up to the top
        else {
            if (scopeindex > scope.length - 1) {
                if (typeof scopex == 'undefined') window.scrollBy(0, -999999)
                scopeindex = 0
            }
        }

        nextelement = scope[scopeindex]

        if (typeof (notgroupno) == 'undefined') {
            scrollintoview(nextelement)
        }

        // } while((nextelement.tagName!='INPUT'&&nextelement.tagName!='SELECT')
        //} while((!nextelement.tagName.match(gdatatagnames)||typeof nextelement.isTextEdit!='undefined'&&nextelement.isTextEdit==false)
    } while (!nextelement.tagName.match(gdatatagnames)
         ||
         (typeof nextelement.isContentEditable != 'undefined' && nextelement.isContentEditable != 'true')
         ||
         (notgroupno !== '' && Number(nextelement.getAttribute('neosysgroupno')) == notgroupno)
         ||
         (nextelement == element)
         ||
         nextelement.getAttribute('neosysreadonly')
         ||
         !nextelement.clientWidth
         ||
         nextelement.currentStyle.display == 'none'
         ||
         nextelement.parentNode.currentStyle.display == 'none'
         ||
         nextelement.tabIndex < elementtabindex

        )

    //found it. focus on it
    focuson(nextelement)

}

function scrollintoview(element) {
    if (!element) return

    //only scroll input fields
    var tagname = element.tagName
    if (!tagname.match(gdatatagnames)) return
    if (!element.name) return

    //get total left offset
    var offsetleft = 0
    var element2 = element
    do {
        offsetleft += element2.offsetLeft
        element2 = element2.offsetParent
    }
    while (element2)

    leftextra = 100
    rightextra = 100

    if (offsetleft < leftextra) {
        window.scrollBy(-99999, 0)
        return
    }

    //scroll left
    var scrollleft = document.body.scrollLeft - offsetleft
    if (scrollleft > 0) {
        window.scrollBy(-scrollleft - leftextra, 0)
    }

    //scroll right
    else {
        var scrollright = offsetleft + element.offsetWidth - document.body.clientWidth - document.body.scrollLeft
        if (scrollright > 0) {
            window.scrollBy(scrollright + rightextra, 0)
        }
    }

    return

}

///////////////////// BUTTON EVENTS /////////////////////////

//NEW button
////////////
function newrecord_onclick() {

    //save and close existing document otherwise cancel
    if (gkey && !closedoc('NEW'))
        return false

    // element.focus()
    var tt = gfirstnonkeyelement
    if (tt.id)
        tt = tt.id
    window.setTimeout('focuson("' + tt + '")', 10)

    //newrecordfocus()

}

//RELEASE BUTTON
////////////////
function releasereopenrecord_onclick() {

    //not locked and there is a key
    if (!glocked) {
        saveopenreadonly = gparameters.openreadonly
        if (saveopenreadonly)
            gparameters.openreadonly = false
        opendoc(gkey)
        if (saveopenreadonly)
            gparameters.openreadonly = saveopenreadonly
        return true
    }

    //if (!saveandunlockdoc()) return false
    if (!saveandorcleardoc('RELEASE'))
        return false

    //releasereopenrecord.value='Reopen'
    setgraphicbutton(releasereopenrecord, 'Reopen', greopenimage)
    setdisabled(releasereopenrecord, false)

    return true

}

//SAVE BUTTON
/////////////
function saverecord_onclick() {

    //first update the current field
    //otherwise the db is updated without the last entry!!!
    if (!validateupdate()) return false

    //check for missing required
    if (!validateall()) return false

    // saverecord.focus()

    //if databound form then save it
    if (gKeyNodes) {

        //save the record
        if (!savedoc()) return false

        //prune the cache to reselect any edited record
        prunecache('SELECT\r' + gdatafilename + '\r')
        prunecache('READ\r' + gdatafilename + '\r' + gkey)
        prunecache('GETINDEXVALUES\r' + gdatafilename + '\r')

        //custom postwrite function
        if (typeof form_postwrite == 'function') {
            //neosysevaluate2('form_postwrite()')
            if (!neosysevaluate2('form_postwrite()')) return false
        }
        //otherwise automatic option to print if available
        else {
            //TODO put this back as a form level opt in
            //   if (gprintsendrecord&&!gprintsendrecord.getAttribute('disabled')&&neosysyesno('Print/Send '+gkeyexternal+' ?',2)) printsendrecord_onclick()
        }

        return true

    }

    //from here on is NOT a databound form
    //////////////////////////////////////

    //custom prewrite function
    if (typeof (form_prewrite) == 'function') if (!neosysevaluate2('form_prewrite()', 'saverecord_onclick()')) return false

    //option to confirm
    if (gparameters.confirm || gparameters.savemode && gparameters.savemode.indexOf('CONFIRM') >= 0) {
        if (!neosysyesno('OK to continue?', 2)) return false
    }

    ///convert screen data into a rev string
    gro.data = gds.data
    if (!gro.obj2rev()) return false

    //alert(gro.revstr.split(fm))

    //custom write and postwrite routine
    if (typeof form_write == 'function') {
        if (!neosysevaluate2('form_write()', 'saverecord_onclick()')) return false
        if (typeof (form_postwrite) == 'function') neosysevaluate2('form_postwrite(db)', 'saverecord_onclick()')
        return true
    }

    //option to return the form data to a calling program
    //instead of sending to the database
    if (window.dialogArguments && (!gparameters.savemode || !gparameters.savemode.neosysswap('CONFIRM', ''))) {
        window.returnValue = gro.revstr
        window.close()
        return false
    }

    //standard unbound write routine
    if (!unbound_form_write()) return false

    //postwrite routine
    if (typeof (form_postwrite) == 'function')
        neosysevaluate2('form_postwrite(db)', 'saverecord_onclick()')
    else {
        if (db.data) {
            var newurl = db.data
            if (window.location.href.toString().toLowerCase().indexOf('//localhost/neosys7/') >= 0 && db.data.toLowerCase().slice(0, 8) == '..\\data\\') {
                newurl = window.location.href.toString().split('/').slice(0, 3).join('/') + '/neosys/' + db.data.slice(3)
            }
            windowopen(newurl)
        }
    }
    if (gparameters.savemode && gparameters.savemode.indexOf('CLOSE') >= 0) window.close()

    return true

}

function unbound_form_write() {

    //send the instructions for processing and open the report
    db.request = 'EXECUTE\r' + gmodule + '\r' + gdatafilename
    if (!db.send(gro.revstr)) {
        neosysinvalid(db.response)
        return false
    }
    if (db.response.slice(0, 3) == 'OK ') {
        neosysnote(db.response.slice(3))
    }

    return true

}

//CLOSE BUTTON
//////////////
function closerecord_onclick() {

    //cancel on dialoged window closes window returns false
    if (window.dialogArguments || !gKeyNodes.length && gparameters.savemode && gparameters.savemode.indexOf('CONFIRM') >= 0)
    //if (window.dialogArguments)
    {

        //user confirms or cancels
        if (gchangesmade) {
            var response = neosysyesnocancel('Discard data entered ?', 1, '', 'Discard', 'Cancel')
            if (response != 2) return false
        }

        var returnvalue = ''

        //clear the document so user is not asked again in onunload
        if (gKeyNodes) {
            gwindowunloading = true
            if (!gchangesmade) returnvalue = 'ACCESS ' + gkey
            cleardoc()
        }

        if (window.dialogArguments) {

            //close the window
            window.returnValue = returnvalue
            window.close()

            //fail safe
            return false
        }

    }

    return closedoc('CLOSE')

}

//DELETE BUTTON
///////////////
function deleterecord_onclick(event) {

    if (!event)
        event = window.event

    if (!deletedoc())
        return neosyscancelevent(event)

    //prune the cache to reselect any deleted record
    prunecache('SELECT\r' + gdatafilename + '\r')
    prunecache('GETINDEXVALUES\r' + gdatafilename + '\r')

    //custom postdelete function
    if (typeof form_postdelete == 'function') {
        //neosysevaluate2('form_postdelete()')
        if (!neosysevaluate2('form_postdelete()'))
            return false
    }

    return true

}

//CHANGE PAGE BUTTONS
/////////////////////
function changepage(pagen) {
    if (gKeyNodes && closerecord.getAttribute('disabled')) return false

    var tablex = getancestor(window.eventtarget, 'TABLE')
    //if (tablex!=null&&typeof(tablex.getAttribute('neosysgroupno'))!='undefined')
    if (tablex != null && Number(tablex.getAttribute('neosysgroupno'))) {
        switch (pagen) {
            case 0: { tablex.firstPage(); break }
            case -1: { tablex.previousPage(); break }
            case 1: { tablex.nextPage(); break }
            default: { tablex.lastPage(); break }
        }
    }

}

/////////////////////// DOCUMENT FUNCTIONS //////////////////


//'''''''''
//'OPENDOC
//'''''''''

var gopening = false

function opendoc(newkey) {
    //debug('opendoc:'+newkey)
    //how to avoid double calls from multiple timeouts
    if (newkey == gkey && glocked)
        return true

    if (newkey == '%NEW%') newkey = ''

    //check potential key has as many parts as there are key fields
    if (newkey && gKeyNodes.length > 1) {
        var sepchar = Number(gKeyNodes[0].getAttribute('neosyskeypart')) ? '*' : gKeyNodes[0].getAttribute('neosyswordsep')
        if (sepchar && newkey.split(sepchar).length != gKeyNodes.length) {
            systemerror('opendoc', newkey.neosysquote() + ' newkey does not return the correct number of keyparts')
            return false
        }
    }

    gopening = true
    var opened = opendoc2(newkey)
    gopening = false

    //add key into gkeys
    if (opened && gKeyNodes.length) {
        if (!gkeys.neosyslocate(gkey)) {
            gkeys = gkeys.neosysinsert(gkeyn + 1 + 1, gkey)
            if (gkeys.length > 1) gkeyn++
        }
    }

    //focus on key (last if more than one) element if opendoc2 fails
    if (!opened) {
        window.setTimeout('try{gKeyNodes[gKeyNodes.length-1].focus()}catch(e){}', 10)
        return false
    }

    return true

}

function getlockholder(response) {
    var lockholder = 'somebody else'
    var tt = db.response.indexOf('LOCKHOLDER:')
    if (tt >= 0) lockholder = db.response.slice(tt + 11).split('"')[1]
    return lockholder
}

function opendoc2(newkey0) {
    //debug('opendoc2:'+newkey0)

    login('opendoc2')

    //detect if a new key is present and quit if not
    var newkey = ''
    if (newkey0) {
        newkey = newkey0
    }
    else {
        if (gKeyNodes) {
            newkey = getkey()
            if (newkey == gkey && gloaded) {
                //newkey=''
                return logout('opendoc2 - no new key')
            }
        }
        if (!newkey) {
            //qqq allow read without key to return next sequential key
            //   return logout('opendoc2 - no new key')
        }
    }

    //unload previous doc first
    ///////////////////////////
    //have to clear even if not loaded othewise setkeyvalues fails because gds.data not yet available when key=xxx in the url
    if (!closedoc('OPEN')) {
        //qqq should try and restore the previous key data?
        return logout('opendoc2 - did not close currently open doc')
    }

    setkeyvalues(newkey)//closedoc zaps them.
    gkey = newkey
    gkeyexternal = getkeyexternal()

    //optional preread function (returns false to abort) 
    if (typeof (form_preread) == 'function') {

        log('preread external key=' + gkeyexternal + ' internalkey=' + gkey)
        if (!neosysevaluate2('form_preread()', 'opendoc2()'))
            return logout('opendoc2 - preread false')

    }

    wstatus('Requesting data, please wait ...')

    //read record (with or without a lock)
    /////////////

    //gkey can be blank in which case a new key will be
    //returned in gro.key

    gro.key = gkey
    if (gkey || !gupdateonlymode)
    //read with key='' means get the next sequential number
        gro.read(!greadonlymode && !gparameters.openreadonly)

    loadnewkey()

    glocked = gro.sessionid != ''
    //ensure lock is automatically re-locked every x minutes while record is active on-screen
    if (glocked) startrelocker()

    //fail if no data (failure to reach backend database)
    //listen always provides data even if only an empty dataset (eg new record)

    if (!gro.data) {
        if (glocked)
            unlockdoc()//fail safe

        logout('opendoc2 - no data')
        return neosysinvalid(gro.response)
    }

    var lockholder = getlockholder(db.response)

    if (db.response.toUpperCase().slice(0, 16) == 'ERROR: NO RECORD') {
        if (gupdateonlymode) {
            //neosysinvalid(neosysquote(gkeyexternal)+' does not exist.')
            neosysinvalid(neosysquote(getkeyexternal()) + ' does not exist.')
            logout('opendoc2 - cannot create new record because gupdateonlymode is true')
            return false
        }

        if (!glocked) {

            //cannot create records if cannot lock them
            if (db.response.toUpperCase().indexOf('LOCK NOT AUTHORISED') >= 0) {
                neosysinvalid('Sorry, you are not authorised to create new records in this file.')
                return logout('opendoc2 - cannot create new record because lock not authorised')
            }

            //may not be authorised to read a record
            if (db.response.toUpperCase().indexOf('NOT AUTHORISED') >= 0) {
                neosysinvalid(db.response)
                return logout('opendoc2 - cannot access record because not authorised')
            }

            //cannot create a record in read only mode or update only mode
            if (greadonlymode || gupdateonlymode || gpreventcreation) {
                //neosysinvalid('Sorry, '+gkeyexternal+' does not exist and\ryou are not authorised to create new records in this file.')
                neosysinvalid('Sorry, ' + getkeyexternal() + ' does not exist and\ryou are not authorised to create new records in this file.')
                return logout('opendoc2 - cannot create new record in read only mode')
            }

            //cannot create a record without a lock
            //if possible get another default key
            if (gKeyNodes.length == 1 && gKeyNodes[0].getAttribute('neosysdefaultvalue')) {
                var newkey = getdefault(gKeyNodes[0])
                if (newkey && newkey != gkey) {
                    logout('opendoc2 - cannot create new record because ' + lockholder + ' is creating it.')
                    return opendoc2(newkey)
                }
            }

            neosysinvalid(neosysquote(gkeyexternal) + ' is being created by ' + lockholder + '.\r\n\r\nYou cannot view or update it until they have finished or cancel.')
            logout('opendoc2 - cannot create new record because ' + lockholder + ' is creating it.')
            return false
        }

        /*
        //optionally cancel if (record does not exist
        if (neosysokcancel('Document '+neosysquote(getkeyexternal)+' does not exist. Create a new document ?',2)!=1)
        {
        if (glocked)
        unlockdoc()
        neosyscancelevent(event)
        return logout('opendoc - user chose not to create new record')
        }
        */

        gds.isnewrecord = true

    }
    else {

        if (!glocked) {

            //cannot create records if cannot lock them
            if (db.response.toUpperCase().indexOf('LOCK NOT AUTHORISED') >= 0) {
                //alert('Sorry, you are not authorised to update this file.')
            }
            else {

                if (greadonlymode) {
                    //open in read only mode
                }
                else {
                    if (db.response.toUpperCase().indexOf('CANNOT LOCK RECORD') >= 0) {
                        if (neosysyesnocancel(neosysquote(gkeyexternal) + ' is being updated by ' + lockholder + '.\rOpen for viewing only?', 1, 'Yes', '', 'Cancel') != 1) {
                            return logout('opendoc2 - because it is being updated by ' + lockholder + ' and the user chose not to open it in read only mode')
                        }
                        //releasereopenrecord.value='Reopen'
                        setgraphicbutton(releasereopenrecord, 'Reopen', greopenimage)
                        setdisabled(releasereopenrecord, false)
                    }
                    else {
                        if (db.response.toString().slice(0, 2) != 'OK')
                            return neosysinvalid(db.response)
                        else
                            neosysnote(db.response.toString().slice(2))
                    }
                }
            }
        }

        gds.isnewrecord = false

    }

    //make sure gds is available in postread
    gds.data = gro.data
    gds.dict = gro.dict
    gds.dictitem = gro.dictitem

    //if record created/updated in form_postread
    //form_postread should set gchangesmade=true to allow save without user edits
    gchangesmade = false
    gpreviouselement = null
    gpreviousvalue = ''

    //postread
    if (typeof form_postread == 'function') {
        grecn = null
        if (!neosysevaluate2('form_postread()', 'opendoc2()'))
        //if (!form_postread())
        {
            if (glocked)
                unlockdoc()//fail safe
            cleardoc()
            return logout('opendoc2 - postread failed')
        }

    }

    //load the record
    if (gds.isnewrecord) {
        wstatus('New record')
    }
    else {
        wstatus('Formatting, please wait ...')
    }
    gloaded = true
    gds.load(gro)
    if (!gds.isnewrecord) wstatus('Existing record')

    document.title = gdoctitle + ' ' + gkeyexternal.neosysconvert('*', ' ')

    //enable the delete and save buttons if locked
    //according to need
    if (glocked) {
        setdisabled(saverecord, false)
        setdisabled(releasereopenrecord, false)
        //releasereopenrecord.value='Release'
        setgraphicbutton(releasereopenrecord, 'Release', greleaseimage)
        if (deleterecord && !gds.isnewrecord && !greadonlymode && !gupdateonlymode && !gpreventdeletion)
            setdisabled(deleterecord, false)
    }
    else if (!greadonlymode && gparameters.openreadonly) {
        setdisabled(releasereopenrecord, false)
        setgraphicbutton(releasereopenrecord, 'Reopen', greopenimage)
    }

    //enable the close and print buttons
    setdisabled(closerecord, false)
    if (gprintsendrecord)
        setdisabled(gprintsendrecord, false)

    //hide the enter button
    //setdisabled(enter,true)

    //postdisplay (in opendoc2 and )
    //TODO convert all postpostread( to use this new hook function instead of timeout
    if (typeof form_postdisplay == 'function') {
        grecn = null
        if (!neosysevaluate2('form_postdisplay()', 'opendoc2()')) {
            if (glocked)
                unlockdoc()//fail safe
            cleardoc()
            return logout('opendoc2 - postdisplay failed')
        }

    }

    logout('opendoc2')

    return true

}

function loadnewkey() {

    //load the new key
    if (gkey != gro.key) {
        setkeyvalues(gro.key)
        gkey = getkey()
        //form key  x*y*z* could be a little different than the db key x*y*z
        gkeyexternal = getkeyexternal()
    }

}

///////////
//CLOSE DOC
///////////
function closedoc(mode) {

    //WINDOWUNLOAD,CLOSE,NEW,OPEN
    if (!mode)
        mode = 'CLOSE'

    //first update the current field
    //otherwise the db is updated without the last entry!!!
    //if (save&&!validateupdate())
    // return false
    //should not do validateupdate since maybe called from validateupdate to close/ timeout to open a new record

    //return saveandorcleardoc(mode,save,clear=mode!='OPEN')
    //return saveandorcleardoc(mode,save=mode!='CLEAR',clear=mode!='OPEN')
    return saveandorcleardoc(mode)

}

//makedoc readonly
function saveandunlockdoc() {
    if (!validateupdate())
        return false
    if (gchangesmade && !savedoc())
        return false
    if (!unlockdoc())
        return false
    return true
}

//'''''''''''
//'SAVE DOC
//'''''''''''
function savedoc(mode) {

    //called from saverecord_onclick()
    if (!mode)
        mode = 'SAVE'

    //first update the current field
    //otherwise the db is updated without the last entry!!!
    if (!validateupdate())
        return false

    if (gKeyNodes && (!gchangesmade || !glocked) && !gallowsavewithoutchanges) {
        neosysinvalid('Nothing to be saved.\r\rPlease enter or change some data first or just click Close')
        return false
    }

    //return saveandorcleardoc(mode,save=true,clear=false)
    return saveandorcleardoc(mode)

}

//'''''''''''
//'SAVE DOC
//'''''''''''
function saveandorcleardoc(mode) {

    // debug('saveandorcleardoc:'+mode)

    //mode CLOSE, SAVE, CLEAR, CHANGEKEY, PRINT
    var save = (mode != 'CLOSE')
    //var clear=(mode!='SAVE'&&mode!='OPEN'&&mode!='CLOSE')
    var clear = (mode == 'CLOSE' || mode == 'NEW' || mode == 'CLEAR')
    var unlock = (mode != 'SAVE' && mode != 'PRINT' && mode != 'LIST' && mode != 'NEW')
    //debug('saveandorclear '+save+' '+clear+' '+unlock)
    //called from
    //should not do validateupdate since maybe called from validateupdate to close/ timeout to open a new record

    //function savedoc(mode)
    //('SAVE',true,false)
    //mode can be SAVE

    //function printsendrecord_onclick()
    //('PRINT',true,false)

    //function closedoc(mode)

    //(mode,save,true)
    //mode can be WINDOWUNLOAD,CLOSE,NEW,OPEN

    login('saveandorcleardoc ' + save + ' ' + clear)

    //first update the current field
    //otherwise the db is updated without the last entry!!!
    //if (save&&!validateupdate())
    // return logout('saveandorcleardoc - invalidateupdate failed')

    //if anything updated then option to save
    if (glocked
     && (gchangesmade
        || (gallowsavewithoutchanges
           && !clear)
        || (gpreviouselement
           && gpreviouselement.getAttribute('neosysfieldno') > 0
           && getvalue(gpreviouselement) != gpreviousvalue)
       )
    ) {

        //check last data entry is valid unless closing in which case discard will not validate
        if (mode != 'CHANGEKEY' && mode != 'CLOSE' && !validateupdate()) {
            focusongpreviouselement()
            return logout('saveandorcleardoc - invalid input')
        }

        //opportunity to save or cancel
        if (typeof form_prewrite != 'undefined' && mode == 'SAVE') {

            //confirm specific save
            if (gparameters.savemode == 'CONFIRM') {
                if (!neosysokcancel('OK to save ' + neosysquote(gkeyexternal.neosysconvert('*', ' ')) + ' ?', 1)) {
                    //return false
                    return logout('saveandorcleardoc - user cancelled')
                }
            }

            var response = 1
        }
        else {

            var savetitle = ''
            var discardtitle = ''
            var action = ''
            if (true || save)
                savetitle = 'Save'
            action = savetitle
            if (true) {
                discardtitle = 'Discard'
                if (true || save)
                    action += ' or '
                action += discardtitle
            }
            var response = neosysyesnocancel(action + ' ' + neosysquote(gkeyexternal.neosysconvert('*', ' ')) + ' ?', 1, savetitle, discardtitle, 'Cancel')
        }

        //user cancels
        if (response == 0) {
            focusongpreviouselement()
            return logout('saveandorcleardoc - user cancelled')
        }

        //prepare to close modal window
        window.returnValue = ''

        //user chooses to save
        if (response == 1) {

            //first update the current field
            //otherwise the db is updated without the last entry!!!
            //if (mode!='CHANGEKEY'&&!validateupdate())
            if (mode == 'CLOSE' && !validateupdate()) {
                focusongpreviouselement()
                return logout('saveandorcleardoc - user cancelled')
            }

            var savekey = gkey
            var wasnewrecord = gds.isnewrecord

            if (!writedoc(unlock))
                return logout('saveandorcleardoc - writedoc failed')

            //prepare to close modal window
            window.returnValue = (wasnewrecord ? 'NEW ' : 'UPDATE ') + savekey

        }

        //close window if modal also in deletedoc
        if (window.dialogArguments && (mode == 'SAVE' || mode == 'DELETE' || mode == 'CLOSE')) {
            gwindowunloading = true
            cleardoc()

            //close the window
            //window.returnValue=''
            window.close()

            return false

        }

        //enable deleterecord
        if (!greadonlymode && !gupdateonlymode && !gpreventdeletion)
            setdisabled(deleterecord, false)

    }

    //clear
    if (clear || response == 2) {
        cleardoc()
    }

    //unlock
    else if (unlock && glocked) {
        if (!unlockdoc())
            return logout('saveandorcleardoc - could not unlock')
    }

    //reset
    gchangesmade = false

    logout('saveandorcleardoc ok')

    return true

}

function cleardoc() {
    //no processing if not initialised
    if (!ginitok) return

    //unconditionally closes the document and prepares a new blank record
    //but returns false if cannot unlock record
    //1. unlock
    //2. disable buttons
    //3. clear keys and record

    //called by:
    //unloaddoc if no changes or user does not cancel
    //deletedoc if users confirms

    login('cleardoc')

    if (gKeyNodes && !unlockdoc())
        return logout('cleardoc - unlockdoc failed')

    //disable the buttons
    gchangesmade = false
    if (gKeyNodes) {
        setdisabled(saverecord, true)
        setdisabled(releasereopenrecord, true)
        //releasereopenrecord.value='Release'
        setgraphicbutton(releasereopenrecord, 'Release', greleaseimage)
        if (deleterecord)
            setdisabled(deleterecord, true)
        setdisabled(closerecord, true)
        if (gprintsendrecord)
            setdisabled(gprintsendrecord, true)
    }

    //set this before clear because .load will call newrecordfocus()
    gpreviouselement = null
    gpreviousvalue = ''

    //close it
    if (!gwindowunloading) {

        //gkey=''
        //gkeyexternal=''
        gloaded = false
        gchangesmade = false
        document.title = gdoctitle

        //get an empty record and load it
        wstatus('Clearing, please wait ...')
        //  if (!glogging)
        //  {
        gro.key = ''
        gro.revstr = gro.defaultrevstr
        gro.rev2obj()
        gds.load(gro)

        //any parameters named like form by matching name
        for (var paramid in gparameters) {
            var paramvalue = gparameters[paramid]
            var dictitem = gro.dictitem(paramid)
            if (!dictitem) continue

            //update multiple values (only works for single field groups)
            if (typeof paramvalue == 'string' && dictitem.groupno > 0 && paramvalue.indexOf(vm) >= 0) {
                var element = $$(paramid)
                if (element && element[0])
                    element = element[0]
                insertallrows(element, paramvalue.split(vm))
            }

            //update single values
            else {
                gds.setx(paramid, null, paramvalue)
            }

        }

        //  }
        wstatus('')

        resetsortimages()

    }

    //if key is defaulted then clearing may have loaded a new record already
    if (!glocked && gkey) {
        glastkey = gkey
        gkey = ''
    }

    if (!gwindowunloading) {

        //setfocus on first non-defaulted key node
        var focussed = false
        if (gKeyNodes && gKeyNodes.length > 1) {
            for (var ii = 0; ii < gKeyNodes.length; ii++) {
                setdefault(gKeyNodes[ii])
                if (!getvalue(gKeyNodes[ii])) {
                    focuson(gKeyNodes[ii])
                    focussed = true
                    break
                }
            }
        }

        if (!focussed) {
            setdefault(gstartelement)
            focuson(gstartelement)
            //if (gstartelement&&gstartelement.tagName!='TEXTAREA')
            if (gstartelement.select)
                try {
                    gstartelement.select()
                }
                catch (e) {
                    stoprighthere
                }
        }

        gchangesmade = false

    }

    //force any updates to be validated
    validateupdate()

    //postdisplay in cleardoc and postinit
    //TODO convert all postpostread( to use this new hook function instead of timeout
    if (typeof form_postdisplay == 'function') {
        grecn = null
        neosysevaluate2('form_postdisplay()', 'formfunctions_onload()')
    }

    logout('cleardoc')

}

function resetsortimages(groupno) {
    //reset the sort buttons
    if (!groupno) {
        for (groupno = 1; groupno < 99; groupno++) {
            resetsortimages(groupno)
        }
        return
    }

    var elements = $$('sortbutton_' + groupno)
    if (elements && elements.tagName)
        elements = [elements]
    if (elements) {
        for (elementn = 0; elementn < elements.length; elementn++)
            elements[elementn].src = elements[elementn].originalsrc
    }

}

//not used
function copychildnodes(fromcell, tocell) {
    for (var childn = 0; childn < fromcell.childNodes.length; childn++) {
        var fromchildcell = fromcell.childNodes[childn]
        var tochildcell = tocell.insertBefore(fromchildcell.cloneNode(false), null)
        if (tochildcell.tagName == 'TD') tochildcell.innerText = ''
        if (fromchildcell.childNodes.length > 0) copychildnodes(fromchildcell, tochildcell)
    }
}

function deletedoc() {

    //prevent delete if not locked
    if (!glocked)
        return neosysinvalid(readonlydocmsg())

    //prevent delete if new record
    if (gds.isnewrecord)
        return neosysinvalid('You cannot delete this document because it hasnt been saved')

    login('deletedoc')

    var question = neosysquote(gkeyexternal) + '\rAre you SURE that you want to\rPERMANENTLY and IRREVERSIBLY\rdelete this document?'
    if (neosysyesno(question, 2) != 1) {
        neosysinvalid('The document has NOT been deleted\rbecause you did not confirm.')
        return logout('deletedoc - user cancelled')
    }

    //stop relocking while trying to delete
    //not really necessary as DELETE is synchronous
    stoprelocker()
    var savekey = gkey

    //delete it
    db.request = 'DELETE\r' + gdatafilename + '\r' + gkey + '\r\r' + gro.sessionid
    if (!db.send()) {

        //neosysnote(db.response)
        neosysinvalid(db.response)

        //start the relocker again
        startrelocker()

        return logout('deletedoc - db returns false')

    }

    //any warnings are appended after response like 'OK xxx'
    if (db.response != 'OK' && typeof form_postdelete == 'undefined') {
        neosyswarning(db.response.slice(2))
    }

    //deleting a record automatically unlocks it
    glocked = false

    neosyswarning(neosysquote(gkeyexternal) + ' has been deleted.')

    //close window if modal also in writedoc
    if (window.dialogArguments) {
        gwindowunloading = true
        cleardoc()

        //close the window
        window.returnValue = 'DELETE ' + savekey
        window.close()

        return false

    }

    cleardoc()

    logout('deletedoc')

    return true

}

function validateall(mode) {
    //alert('validateall')
    login('validateall ' + mode)

    //check all required fields are present
    //if mode is 'filldefaults' use default if possible
    for (groupno = 0; groupno == 0 || groupno <= gmaxgroupno; groupno++) {

        //skip missing groups
        if (groupno > 0 && !gtables[groupno])
            continue

        //for groups, we need to know the first input column name
        //if (groupno>0) var firstcolumnname=gfields[gtables[groupno][0]].name
        if (groupno > 0)
            var firstcolumnname = gfields[gtables[groupno][0]].id

        //get header row or single row of group
        var rows = (groupno == 0) ? [gds.data] : gds.data['group' + groupno]

        var allowemptyrows = (groupno == 0) ? false : gtables[groupno].tableelement.neosysallowemptyrows
        var rowrequired = (groupno == 0) ? false : gtables[groupno].tableelement.neosysrowrequired
        var nrowsfound = 0

        for (var rown = 0; rown < rows.length; rown++) {
            var row = rows[rown]

            ////only validate rows if the first input column is entered
            ////if (groupno==0||row[firstcolumnname].element.text!='')
            //if (groupno==0||row[firstcolumnname].text!='')

            var anydata = false
            var missingelement = ''
            for (propname in row) {
                var cell = row[propname]

                //skip unbound, calculated and not required cells
                var element = cell.element
                if (!element || element.getAttribute('neosystype') != 'F')
                    continue

                //skip cells with data
                if (cell.text && (typeof cell.text != 'string' || cell.text.replace(/ *$/, ''))) {

                    //SELECT elements 
                    //if (element.tagName=='SELECT'&&element.options[0].selected)

                    anydata = true
                    continue
                }

                //only interested in required fields from here on
                if (!element.getAttribute('neosysrequired'))
                    continue

                //skip cells that can be defaulted
                //do not default in rows to avoid problem of blank line and first/line line
                //filling in unwanted data (perhaps select tags also have similar problem)
                if (!groupno) {
                    gdefault = getdefault(element)
                    if (gdefault != null && gdefault != '') {
                        cell.text = gdefault
                        anydata = true
                        continue
                    }
                }

                //flag first element that is required, enabled and visible and missing
                if (mode != 'filldefaults'
    && !missingelement
    && !element.getAttribute('disabled')
    && neosysenabledandvisible(element)) {
                    missingelement = element
                    if (groupno == 0)
                        break
                }

            } //propname in row

            //empty last row is always ok
            if (groupno > 0 && !anydata && rown == rows.length - 1)
                missingelement = false

            //fail if any missing data
            if (missingelement && (!allowemptyrows || anydata)) {
                focuson(missingelement)
                neosysinvalid(missingelement.getAttribute('neosystitle') + ' is required.')
                return logout('validateall ' + mode)
            }

            //prevent blank rows (except last one) unless allowed
            if (mode != 'filldefaults'
   && !anydata
   && groupno != 0
   && rown < (rows.length - 1)) {
                if (!allowemptyrows) {
                    var missingelement = rows[0][firstcolumnname].element
                    focuson(missingelement)
                    logout('validateall ' + mode + ' empty row')
                    return neosysinvalid('Empty rows are not allowed for ' + missingelement.getAttribute('neosystitle'))
                }
            }

            //count the number of rows with data
            if (anydata) nrowsfound++

        } //rows

        //error if not enough rows
        if
  (
   groupno > 0
   && mode != 'filldefaults'
   && !nrowsfound
   && rowrequired
   && neosysenabledandvisible($$('neosysgroup' + groupno))
  ) {
            var missingelement = rows[0][firstcolumnname].element
            focuson(missingelement)
            logout('validateall ' + mode + ' no rows')
            return neosysinvalid('At least one ' + missingelement.getAttribute('neosystitle') + ' is required.')
        }

    }

    logout('validateall ' + mode)
    return true

}

function writedoc(unlock) {

    login('writedoc')

    //check all required fields are present
    if (!validateall())
        return logout('writedoc - validate all failed')

    //custom prewrite routine
    goldvalue = ''
    gvalue = ''
    if (typeof (form_prewrite) == 'function') {
        if (!neosysevaluate2('form_prewrite()', 'writedoc()')) return false
        log('form_prewrite - after')
    }

    //stop the relocker while saving the document
    stoprelocker()

    //ok save it

    gro.request = unlock ? 'WRITEU' : 'WRITE' + '\r' + gdatafilename + '\r' + gkey
    gro.data = gds.data
    if (!gro.writex(unlock)) {

        //neosysnote('Cannot save '+neosysquote(gkeyexternal)+' because: \r\r'+gro.response)
        neosysinvalid('Cannot save ' + neosysquote(gkeyexternal) + ' because: \r\r' + gro.response)

        return logout('writedoc - write failed')

    }

    //any warnings are appended after response like 'OK xxx'
    if (gro.response != 'OK' && typeof form_postwrite == 'undefined') {
        neosyswarning(gro.response.slice(2))
    }

    //if a cached is written then remove it from the cache (could update it instead?)
    deletecacherecord(gdatafilename, gkey)

    //option to unlock after saving
    if (unlock) {
        glocked = false
    }
    else {

        //restart the relocker if failed to save
        startrelocker()
    }

    loadnewkey()

    //merge new data back into the screen
    if (gro.data) {
        wstatus('Formatting, please wait ...')

        //existing gds' elements are pointing to screen elements so do not copy
        //gds.data=gro.data

        gds.rebind(gro.data)

        var recn = grecn
        grecn = null
        calcfields()
        grecn = recn

        wstatus('')

        gds.isnewrecord = false
        if (!gupdateonlymode && !greadonlymode && !gpreventdeletion)
            setdisabled(deleterecord, false)

        //update in case changed (eg rows resorted)
        var temp = getvalue(gpreviouselement)
        if (gpreviousvalue != temp) {
            gpreviousvalue = temp
            try { gpreviouselement.select() } catch (e) { }
        }

    }

    logout('writedoc ok')

    return true

}

function startrelocker() {
    grelocker = window.setInterval('relockdoc()', glocktimeoutinmins / 3 * 60 * 1000)
}

function stoprelocker() {
    window.clearInterval(grelocker)
}

function relockdoc() {

    // if (!gloaded) return false

    login('relockdoc')

    //dont relock if (already in the process of relocking or something else)
    if (grelockingdoc) return logout('relockdoc')
    if (db.requesting) return logout('relockdoc')

    grelockingdoc = true

    var relockdb = new neosysdblink()
    relockdb.request = 'RELOCK\r' + gdatafilename + '\r' + gkey + '\r' + gro.sessionid + '\r' + glocktimeoutinmins
    //document.bgcolor='green'
    if (relockdb.send()) {
        //document.bgcolor='white'
        result = true
    }
    else {
        response = relockdb.response
        result = false
        //document.bgColor='RED'

        //lost control so prevent further editing of document
        if (response.toUpperCase().indexOf('EXPIRED') >= 0) {
            stoprelocker()
            glocked = false
            gchangesmade = false
            setdisabled(saverecord, true)
            setgraphicbutton(releasereopenrecord, 'Reopen', greopenimage)
            setdisabled(deleterecord, true)
            neosyswarning(response)
        }
        else {
            neosysnote(response)
        }

    }

    grelockingdoc = false

    logout('relockdoc')

    return result

}

//'''''''
//'UNLOCK
//'''''''
function unlockdoc() {

    if (!gKeyNodes)
        return

    login('unlockdoc')

    //always return true even if cannot unlock

    //stop the relocker while trying to unlock
    stoprelocker()

    if (glocked) {
        while (true) {
            db.request = 'UNLOCK\r' + gdatafilename + '\r' + gkey + '\r' + gro.sessionid
            if (db.send()) break
            if (!neosysyesno('Cannot release document - try again?\r\r' + db.response)) break
        }
    }

    glocked = false
    setdisabled(deleterecord, true)
    setdisabled(saverecord, true)
    //setdisabled(releasereopenrecord,true)
    setgraphicbutton(releasereopenrecord, 'Reopen', greopenimage)

    logout('unlockdoc')

    return true

}

////////////////////// FIELD FUNCTIONS //////////////////////

function focuson(element) {

    login('focuson')

    if (!element)
        return logout('focuson - no element')

    //convert element name into element 
    //if (typeof(element)=='string'||!element.parentNode)
    if (typeof (element) == 'string') {
        //element=$$(element)
        //var element2=$$(element)

        var element2 = $$(element)
        if (element2 && element2.tagName)
            element2 = [element2]

        if (grecn && element2[grecn])
            element2 = element2[grecn]
        else
            element2 = element2[0]

        if (!element2)
            return logout('focuson - cannot getElementById ' + neosysquote(element))

        element = element2
    }

    //use the first if more than one
    if (typeof element == 'object' && !element.tagName && element.length > 0)
        element = element[0]

    log(element.id + ' ' + element.outerHTML)

    //if already focused on the element then .focus event will not occur so ...
    //NOW will because do .blur() beforehand
    //if (gpreviouselement==null) gpreviouselement=element
    //nb activeElement not present in safari 3.1 at least
    if (false && document.activeElement == element) {
        try {
            element.focus()
            //if (element.tagName!='TEXTAREA')
            if (element.select)
                element.select()
        }
        catch (e)
  { }

        gpreviouselement = element
        gpreviousvalue = getvalue(gpreviouselement)
        setdefault(element)
        log('gpreviouselement and value set to ' + gpreviouselement.id + ' ' + neosysquote(gpreviousvalue))

        try {
            if (element.tagName != 'TEXTAREA')
                element.select()
        }
        catch (e)
  { }

    }
    else {

        //needed because delete line leaves grecn>nrecs
        grecn = getrecn(element)

        //taken out otherwise F7 on job number goes to market code
        // and validateupdate fails because record has not been loaded
        //setdefault(element)

        gfocusonelement = element
        window.setTimeout('focuson2()', 10)
    }

    logout('focuson ' + element.id)

}

function focuson2() {
    if (!gfocusonelement)
        return
    var focusonelement = gfocusonelement
    gfocusonelement = null
    if (!neosysenabledandvisible(focusonelement))
        return focusnext(focusonelement)

    try {
        focusonelement.blur()
        focusonelement.focus()

        //focusing may cause validation that returns focus back to gpreviouselement
        //activeElement not available everywhere
        //if (document.activeElement!=focusonelement)
        if (eventtarget != focusonelement)
            return

        //if (focusonelement.tagName!='TEXTAREA')
        if (focusonelement.select)
            focusonelement.select()

    }
    catch (e) { }

}

//'''''''''
//'ON FOCUS
//'''''''''
function document_onfocus(event) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)
    //window.status='onfocus '+new Date

    //ckeditor
    if (event.listenerData) {
        eventtarget = event.listenerData
    }

    if (!eventtarget)
        return

    //do nothing if not initialised
    if (!ginitok) return

    neosyssetcookie('', 'NEOSYSlogincode', glogincode, 'logincode')
    //window.status=new Date()+' '+glogincode

    var fnam = 'document_onfocus'
    var text = fnam + ' tag:' + eventtarget.tagName + ' id:' + eventtarget.id + (gpreviouselement ? ' gpreviouselement:' + gpreviouselement.id : '')
    log(text)
    //window.status=text

    //window.status=fnam+' tag:'+eventtarget.tagName+' id:'+eventtarget.id+(gpreviouselement?' gpreviouselement:'+gpreviouselement.id:'')

    log('get element and elementid')
    var element = eventtarget

    log('quit if no element.id')
    if (!element.id)
        return logout(fnam)

    log('quit calendar focus')
    if (element.className == 'calendar')
        return logout(fnam)

    log('get elementid')
    var elementid = ''
    try {
        elementid = element.id.toString()
        log('handle focus on ckeditor')
        if (elementid.indexOf('___Frame') > 0) {
            log('to prevent endless loop after cancelling postread (due to ckeditor refocussing on itself?')
            log('minor problem is that this prevents click on ckeditor from triggering a record read')
            if (gKeyNodes && !gkey) {
                log('timeout to focus on last key element')
                window.setTimeout('try{gKeyNodes[gKeyNodes.length-1].focus()}catch(e){}', 10)
                return logout(fnam)
            }
            elementid = elementid.slice(0, -8)
            var ckeditor = element
            element = window.$$(elementid)
            element.isckeditor = ckeditor
        }
    }
    catch (e) {
        elementid = ''
    }

    //dont add twice otherwise event occurs twice!
    //log('ensure checkboxes in rows have onclick set')
    //if (!element.onclick&&element.type=='checkbox'||element.type=='radio')
    // //element.onclick=onclickradiocheckbox
    // addeventlistener(element,'click','onclickradiocheckbox')

    log('if arrived on a readonly field by tab (or enter as tab)\nthen skip forwards (or backwards) to the next field')
    if (gkeycode == 9 || gkeycode == 13) {
        log('but only if normal tabindex because focusnext doesnt work otherwise')
        if (element.getAttribute('neosysreadonly') && element.tabIndex == 999) {
            if (event.shiftKey)
                focusprevious(element)
            else
                focusnext(element)
            return logout(fnam)
        }
    }

    log('no validation/update except changing neosys elements:' + element.getAttribute('neosystype'))
    if (!element.getAttribute('neosystype'))
        return logout(fnam)

    log('quit if opening')
    if (gopening)
        return logout(fnam)

    log('drop down any "modal" popup divs')
    form_closepopups()

    log('quit if refocussing')
    if (element == gonfocuselement) {
        //  if (setdefault(element))  if (element.tagName!='TEXTAREA') element.select()
        return logout(fnam)
    }
    gonfocuselement = element

    log('quit if refocussing on gpreviouselement')
    if (gpreviouselement && element == gpreviouselement)
        return logout(fnam)

    //dont validate update if clicked popup
    //if (element&&element.id==(gpreviouselement.id+'_popup'))
    // return logout(fnam)

    log('check if changed element')
    if (element != gpreviouselement) {
        log('validate/update previous data entry ' + (gpreviouselement ? gpreviouselement.id : ''))
        if (!validateupdate())
            return logout(fnam)
    }

    log('quit if we are not focused on a proper element')
    if (!elementid)
        return logout(fnam)

    log('OK. Now previous element is valid')

    log('find what row we are on')
    grecn = getrecn(element)

    log('quit if not neosys data entry field')
    if (!element.tagName.match(gdatatagnames))
        return logout(fnam)

    log('check key fields')
    if (gKeyNodes && !glocked) {
        if (!checkrequired(gKeyNodes, element, 0))
            return logout(fnam + ' ' + elementid + ' is required but is blank (0)')
    }

    log('check no required fields are missing in prior data')
    log(' fill in any defaults where possible')
    if (!gKeyNodes || glocked)//check keynodes as well now
    {

        log('check no missing data in group 0 always')
        if (!checkrequired(gfields, element, 0))
            return logout(fnam + ' ' + elementid + ' a prior element is visible and required but is blank (1)')

        //check specific group if >0
        //var elements
        //if (Number(element.getAttribute('neosysgroupno')))
        //{
        // elements=getancestor(element,'TR').all
        // if (!checkrequired(elements,element,Number(element.getAttribute('neosysgroupno'))))
        //  return logout(fnam+' '+elementid+' a prior element is visible and required but is blank (2)')
        //}

    }

    log('all previous fields are valid now')
    //wwww
    //triggered by timeout from validateupdate now
    //but do it ALSO here in case triggered by defaulted key field
    log('if the key is complete and not on a key field, get the record')
    //if (!gloaded&&gKeyNodes&&element.getAttribute('neosysfieldno')!=0)
    //dont check if element not changed to avoid endless loop if opendoc errors
    if (gKeyNodes && element != gpreviouselement) {
        var nextkey = getkey()
        //if (nextkey.toUpperCase() != nextkey)
        //    xxx = 1
        if (nextkey && (gloaded && nextkey != gkey) || (!gloaded && element.getAttribute('neosysfieldno') != 0))
        //if (key&&(gloaded&&key!=gkey)||(!gloaded))
        {
            //window.setTimeout('opendoc()',100)
            opendoc(nextkey)
            return logout(fnam + ' ' + neosysquote(elementid) + ' new record')
        }
    }
    log('there is no new record so setup current element')

    log('scroll to top left if the key field')
    if (element == gstartelement || element.getAttribute('neosysfieldno') == 0)
        window.scrollTo(0, 0)

    //if (element.tagName.match(gtexttagnames))  if (element.tagName!='TEXTAREA') element.select()

    log('scroll into view')
    scrollintoview(element)

    log('remove blanks used to force formatting of spans')
    if (element.tagName == 'SPAN' && element.innerText == ' ')
        element.innerText = ''

    log('change gpreviouselement/value to current element/value')
    gpreviouselement = element
    gpreviousvalue = getvalue(gpreviouselement)

    log('set the default of the current element')
    setdefault(element, donotvalidateupdate = true)

    log('deselect previous (this does not work unless .select() is used')
    log(' but causes a problem in focussing on the previous element and rescrolling')
    if (element.tagName == 'SELECT' && document.selection && document.selection.createRange) {
        var textrange = document.selection.createRange()
        textrange.collapse()
        //  textrange.select()
    }

    log('select the whole text')
    if (element.tagName != 'TEXTAREA' && element.select)
        try { element.select() } catch (e) { }

    log('focus on current or next element')
    //this is not necessary unless .select() is used above
    //note that validation code from previous field may make current element disappear
    if (!element.isckeditor) {
        try {
            element.focus()
            log('focussed on current element')
        }
        catch (e) {
            log('try focusnext')
            focusnext(element)
        }
    }

    logout(fnam + ' ' + elementid)

}

/*
function onclickradiocheckbox(event)
{

if (!event)
event=window.event
eventtarget=geteventtarget(event)

//wstatus(getvalue(eventtarget.id)+' '+gpreviousvalue)
//space or arrow keys also simulate a click event
 
//update immediately
//if (element.type=='radio') element.checked=true
//if (element.type=='checkbox') element.checked=!element.checked
//alert(eventtarget.getAttribute)
//alert('onclickradiocheckbox')  
//validate or return to original
if (!validateupdate())
{
setvalue(gpreviouselement,gpreviousvalue)
return
}
gpreviousvalue=getvalue(gpreviouselement)

}
*/

function onclickradiocheckbox(event) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    //wstatus(getvalue(eventtarget.id)+' '+gpreviousvalue)
    //space or arrow keys also simulate a click event

    //handle case where onfocus has not been called before onclick
    if (gpreviouselement != eventtarget) {

        //get the CURRENT value BEFORE it was clicked
        //NB converted to grecn/gpreviousvalue IF AND AFTER previous element validates below
        var recn = getrecn(eventtarget)
        var previousvalue = gds.getx(eventtarget, recn)
        //checkbox and radio buttons are currently not supported in multivalues
        //so the following is probably never executed
        if (typeof recn == 'number')
            previousvalue = previousvalue[0]

        //validate the PREVIOUS field
        if (!validateupdate()) {
            //if returning to previous field then reset the CURRENT element
            // back to what it was prior to clicking it
            setvalue(eventtarget, previousvalue)
            return
        }

        //setup as if onfocus had been called prior to onclick
        gpreviouselement = eventtarget
        grecn = recn
        gpreviousvalue = previousvalue

    }

    //update immediately
    //if (element.type=='radio') element.checked=true
    //if (element.type=='checkbox') element.checked=!element.checked
    //alert(eventtarget.getAttribute)
    //alert('onclickradiocheckbox')  
    //validate or return to original
    if (!validateupdate()) {
        setvalue(gpreviouselement, gpreviousvalue)
        return
    }
    gpreviousvalue = getvalue(gpreviouselement)

}

function validateupdate() {

    var id = gpreviouselement ? gpreviouselement.id : 'undefined'
    login('validateupdate ' + id)

    //return if no previouselement
    if (!gpreviouselement) {
        logout('validateupdate - no previous element')
        return true
    }

    //return if no changes (ignoring case if lowercase not allowed)
    var newvalue = getvalue(gpreviouselement)
    if (!gpreviouselement.neosyslowercase && gpreviouselement != 'radio' && gpreviouselement.type != 'checkbox')
        newvalue = newvalue.toUpperCase()
    if (newvalue == gpreviousvalue) {
        logout('validateupdate - gpreviousvalue:' + gpreviousvalue + ' same as newvalue:' + newvalue)
        return true
    }

    log('User/setdefault changed ' + id + '\rfrom ' + neosysquote(gpreviousvalue) + '\rto ' + neosysquote(newvalue))

    //check for prior required fields if a grouped element
    var elements
    if (Number(gpreviouselement.getAttribute('neosysgroupno'))) {
        elements = getancestor(gpreviouselement, 'TR').getElementsByTagName('*')
        if (!checkrequired(elements, gpreviouselement, Number(gpreviouselement.getAttribute('neosysgroupno')))) {
            setvalue(gpreviouselement, gpreviousvalue)
            return logout('validateupdate ' + id + ' a prior element is visible and required but is blank')
        }
    }

    //check arguments
    assertelement(gpreviouselement, 'validateupdate', 'gpreviouselement')
    if (!gpreviouselement.tagName.match(gdatatagnames)) {
        logout('validateupdate - not an INPUT or SELECT')
        return true
    }

    //quit ok if gpreviouselement is no longer bound
    grecn = getrecn(gpreviouselement)
    if (grecn == -1) {
        logout('validateupdate - gprevious no longer exists')
        return true
    }

    //get the old value (internal format) and grecn
    gcell = gds.getcells(gpreviouselement, grecn)[0]
    goldvalue = gcell.text
    gorigvalue = (typeof gcell.oldtext != 'undefined') ? gcell.oldtext : gcell.text

    //validate it and put back focus if it fails
    ////////////////////////////////////////////
    if (!onbeforeupdate(gpreviouselement)) {
        focusongpreviouselement()

        logout('validateupdate - onbeforeupdate failed')
        return false

    }

    //wwww
    //switch to new key if new and user accepts to close the current one
    var nextkey
    if (gKeyNodes
     && gpreviouselement.getAttribute('neosysfieldno') == 0
     && (nextkey = getkey())
     && nextkey != gkey) {
        //do not change key if user chooses not to unload an existing document
        if (!closedoc('CHANGEKEY')) {
            focusongpreviouselement()
            return logout('validateupdate - closedoc refused')
        }

        //return false and just allow opendoc to happen

        //window.setTimeout('opendoc("'+nextkey+'")',1)
        //alert('DEBUG: settimeout opendoc gkey:'+gkey+' nextkey:'+nextkey)
        //settimeout results in overlapping xmlhttp requests in FF 3.0.3
        //eg alt+P
        //1.triggers update
        //2.settimeout opendoc
        //3.settimeout printrecord
        //4.printrecord STARTS an XML request and waits synchronously for the result
        //5.opendoc timesout(?!)
        //6.opendoc issues ANOTHER XML request and we get a trap message due to this.listening being already set
        //try to avoid the above by calling opendoc directly BUT this may cause errors on return from validateupdate
        opendoc(nextkey)

        logout('validateupdate - opendoc')
        return false
    }

    //update
    ////////

    //update the element multiple selections qqq
    //if reply is an array use replace all rows
    var multiplegvalue = false
    if (typeof (gvalue) == 'object') {
        if (gvalue.length > 1 && Number(gpreviouselement.getAttribute('neosysgroupno'))) {
            multiplegvalue = true

            var elementid = gpreviouselement.id
            insertallrows(gpreviouselement, gvalue)

            //focus on next element AFTER table
            element = $$(elementid)
            if (element && element[0])
                element = element[0]
            focusdirection(1, element, Number(gpreviouselement.getAttribute('neosysgroupno')))

        }
        else {
            gvalue = gvalue[0]
        }
    }
    if (!multiplegvalue)
        gds.setx(gpreviouselement, grecn, gvalue)

    //post update
    /////////////

    //flag record edited
    if (gpreviouselement.getAttribute('neosysfieldno') != 0
 && !gpreviouselement.getAttribute('neosysnochangeswarning')) {
        gchangesmade = true
    }

    //calculate dependencies
    calcfields(gdependents)
    gdependents = []

    //why is this necessary?
    gpreviousvalue = getvalue(gpreviouselement)

    logout('validateupdate - done')

    return true

}

function focusongpreviouselement() {
    window.setTimeout(focusongpreviouselement2, 1)
}

function focusongpreviouselement2() {

    log('focusing back to ' + gpreviouselement.id)

    //prevent loop since focus doesnt go back to radio button immediately
    //doesnt seem to work though
    //if (gpreviouselement.type=='radio')
    //{
    // gvalue=goldvalue
    // setvalue(element.id,gvalue)
    // gpreviouselement.select()
    //}

    //dont focus back to checkboxes because that causes instant revalidation thereby causing endless loop if invalid
    if (gpreviouselement.type != 'checkbox') {
        try { gpreviouselement.focus() } catch (e) { }
    }
    if (isMac && gpreviouselement.tagName != 'SELECT' && gpreviouselement.tagName != 'TEXTAREA')
        gpreviouselement.select()

    //neosyscancelevent(event)

    //try and put cursor at end of gprevious text
    if (!isMac && gpreviouselement.tagName != 'SELECT' && document.selection && document.selection.createRange) {
        try {
            gpreviouselement.select()
            var textrange = document.selection.createRange()
            textrange.collapse(false)
            textrange.select()
        }
        catch (e) { }
    }

}

function earlyupdate() {

    //skip update on buttons
    if (!gpreviouselement || !gpreviouselement.name) return

    gds.setx(gpreviouselement, grecn, gvalue)

}

function checkrequired(elements, element, groupno) {

    //check the given elements with the given group number
    //and prior to the given field for required
    //and try to set default otherwise return false

    grecn = getrecn(element)

    var foundelement = false
    for (var ii = 0; ii < elements.length; ii++) {
        var element2 = elements[ii]

        //if given gfields then get the equivalent DOM element
        //group 0 fields are not bound to DOM initially but are in the next paragraph
        //group 1+ fields are never bound to DOM because there are many rows
        if (!element2.parentNode) {
            element2 = $$(element2.id)
            if (element2 && element2[0])
                element2 = element2[0]
            if (!element2) {
                //no longer check this so that elements can be "unbound" for greater security etc
                //by changing the id to something like id_unbound
                //    alert(elements[ii].id+' has disappeared from the document\nPerhaps it need to be in its own HTML table')
                continue
            }
            gfields[ii] = element2
        }

        //only check input fields
        if (!element2.tagName.match(gdatatagnames)) continue

        //don't check current but continue looking for lower tabindexed fields
        //if (element2==element) //this does not work because gfields<> bound table elements for rows
        //if (element2.getAttribute('neosysscreenfieldno')==element.getAttribute('neosysscreenfieldno'))
        //check ids because neosysscreenfieldno is repeated in gfields for radio 4,4,4
        //but not in the 2nd and subsequent repeated form elements 4,5,6
        if (element2.id == element.id) {
            foundelement = true
            continue
        }

        //skip elements not in desired group
        //must be after detection of foundelement above
        if (Number(element2.getAttribute('neosysgroupno')) != groupno)
            continue

        if (Number(element2.getAttribute('neosysgroupno')) == groupno) {
            if (((!foundelement && element2.tabIndex <= element.tabIndex) || (element2.tabIndex != -1 && element2.tabIndex < element.tabIndex))) {
                //if (element&&element2.getAttribute('neosysrequired')&&gds.getcells(element2,grecn)[0].text=='')
                //if (element&&element2.getAttribute('neosysrequired')&&getvalue(element2)=='')
                //if (element&&(!Number(element.getAttribute('neosysgroupno'))||element2.getAttribute('neosysrequired'))&&getvalue(element2)=='')
                if (element
    && ((gds.isnewrecord && !Number(element2.getAttribute('neosysgroupno')))
    || element2.getAttribute('neosysrequired'))
    && getvalue(element2) == '') {

                    //try to set the default
                    if (!setdefault(element2) && neosysenabledandvisible(element2)) {
                        focuson(element2)
                        return false
                    }

                    //if still empty then fail
                    //if (getvalue(element2)=='')
                    if (element2.getAttribute('neosysrequired') && !element2.getAttribute('neosysreadonly') && gds.getcells(element2, grecn)[0].text == '') {

                        //disabled or invisible elements may be blank and required (even after setdefault)
                        if (element2.getAttribute('disabled') || !neosysenabledandvisible(element2)) return true

                        //put up a message unless is the first column of a row
                        //if (!element2.neosysisfirstinputcolumn&&neosysenabledandvisible(element2))
                        if (true || !element2.neosysisfirstinputcolumn) {
                            neosysinvalid(element2.getAttribute('neosystitle') + ' is required.')
                        }

                        focuson(element2)
                        //if (!Number(element2.getAttribute('neosysgroupno')))
                        // focuson(element2)
                        return false

                    }
                }
            }
        }
    }

    return true

}

function getvalues(elementx, sepchar) {

    //return a array of values, or string separated by sepchar if defined

    var element = elementx
    if (typeof element == 'string')
        element = $$(element)
    else if (typeof element.id != 'undefined')
        element = $$(element.id)

    var values = getvalue(element)

    if (typeof (values) == 'object' && typeof (sepchar) != 'undefined')
        values = values.join(sepchar)

    return values

}

function getvalue(element, recn) {

    if (element == null) {
        systemerror('getvalue()', ' element cannot be null')
        return ''
    }

    // if (recn==null) return getvalues(element)

    //return input value or select value(s) ... or undefined
    //returns a string or array of strings if element is array of elements

    //element is required and must be an element of type
    // INPUT text
    // INPUT radio
    // INPUT checkbox
    // SELECT
    // SPAN

    //get element if given element name
    if (typeof (element) == 'string') {
        var element0 = element
        var element = $$(element0)
        if (!element) {
            systemerror('getvalue()', neosysquote(element0) + ' does not exist')
            return
        }
        //get first one only
        if (!element.tagName)
            element = element[0]
    }

    //radio/checkbox buttons appear like arrays multiple elements
    //force radio/checkbox to be first element
    if (element[0] && (element[0].type == 'radio' || element[0].type == 'checkbox')) element = element[0]

    if (element.length && element[0].type != 'radio') {

        if (element.tagName) {
            //radio buttons appear like arrays multiple elements
            if (element[0].type == 'radio' || element[0].type == 'checkbox') element = element[0]
        }
        else

        //if passed an array of elements, return an array of values
        //is this used anywhere?
        //NB SELECT element on Mac but not PC has length and options[]
        {
            var values = []
            for (var ii = 0; ii < element.length; ii++) {
                values[ii] = getvalue(element[ii])
            }

            return values

        }

    }

    //ensure is an element
    assertelement(element, 'getvalue', 'element')

    switch (element.tagName) {

        case 'INPUT':
            {

                switch (element.type) {

                    case 'text':
                        return element.value.neosystrimr()

                    case 'radio':
                        {

                            var elements = getradiocheckboxelements(element)
                            for (var ii = 0; ii < elements.length; ii++) {
                                var element = elements[ii]
                                if (element.checked) {
                                    if (typeof element.value == 'undefined')
                                        return ii
                                    else
                                        return element.value
                                }
                            }
                            //nothing selected
                            return ''

                        }

                    case 'checkbox':
                        {

                            var elements = getradiocheckboxelements(element)
                            var values = []
                            for (var ii = 0; ii < elements.length; ii++) {
                                var element = elements[ii]
                                if (element.checked) {
                                    values[values.length] = (typeof element.value == 'undefined') ? ii : element.value
                                }
                            }
                            return values.join(sm2)

                        }

                    case 'button':
                        {
                            return ''
                        }

                    default:
                        {
                            raiseerror(1000, neosysquote(element.type) + ' invalid INPUT element type in getvalue()')
                            return
                        }

                }

            }

        case 'OPTION': return getdropdown0(element.parentNode)

        case 'SELECT': return getdropdown0(element)

            //crlf becomes space to prevent entry of tm characters in text fields
        case 'SPAN':
            var value = element.innerText.replace(/([\r\n]+)/g, ' ').neosystrimr()
            if (value == nbsp160)
                return "";
            return value

        case 'TEXTAREA':
            {

                //http://wiki.ckeditor.net/Developer%27s_Guide/Javascript_API
                if (typeof CKEDITOR != 'undefined') {
                    var oEditor = CKEDITOR.instances[element.id]
                    if (oEditor) {
                        //var result=oEditor.GetHTML()
                        var result = oEditor.getData()
                        return result
                    }
                }

                return element.value.neosystrimr()
            }

        default:
            {
                raiseerror(1000, neosysquote(element.tagName) + ' invalid tagName in getvalue()')
                return
            }
    }

}

function neosyssetreadonly(elements, msg, options, recn) {

    //note: cannot set readonly off if the dictionary says that it is readonly

    //if msg contains something then set the element(s) to readonly and vice versa
    //tabbing and cursor keys will skip past readonly fields
    //msg will be displayed if they click and try and change the field

    var elementx = elements

    //if elements is a number then it means set all one group (for selected recn or null for all)
    if (!isNaN(+elementx)) {
        var elements = []
        for (var ii = 0; ii < gfields.length; ii++) {
            if (Number(gfields[ii].getAttribute('neosysgroupno')) == elementx
   && Number(gfields[ii].getAttribute('neosysfieldno'))) {
                if (!neosyssetreadonly(gfields[ii].id, msg, options, recn))
                    return false
            }
        }
        return true
    }

    //if elements is a simple string then get all the elements for it
    if (typeof elementx == 'string') {
        var elementxstring = elementx
        elementx = $$(elementx)
        //if (!elementx||elementx.length==0)
        if (!elementx) {
            //only give error if it is not even in the dictionary
            //so that we can remove fields from the screen without changing the setreadonly field lists
            if (!gds.dictitem(elementxstring)) return systemerror('neosyssetreadonly()', neosysquote(elements) + ' is not in the form')
            return false
        }

        //handle arrays of recn recursively
        //handles arrays of fieldnames and recns efficiently when items are string ids
        //since extraction of all elements for one element id is probably time consuming
        if (typeof recn == 'object' && recn.length) {
            for (var ii = 0; ii < recn.length; ii++) {
                if (!neosyssetreadonly(elementx[recn[ii]], msg, options)) return false
            }
            return true
        }

        if (typeof recn != 'undefined' && recn != null && recn !== '') {
            elementx = elementx[recn]
            if (typeof elementx == 'undefined') {
                return systemerror('neosyssetreadonly(' + elements + ',' + msg + ',' + recn + ')')
            }
            elementx = [elementx]
        }
        if (elementx.length == 1) elementx = elementx[0]
    }

    //handle arrays recursively
    //if (typeof elementx=='object'&&elementx.length&&!elementx.name)
    if (typeof elementx == 'object' && elementx.length && !elementx.getAttribute) {
        for (var ii = 0; ii < elementx.length; ii++) {
            //if (!neosyssetreadonly(elementx[ii],msg,options,recn)) return false
            //dont stop just because one doesnt exist
            neosyssetreadonly(elementx[ii], msg, options, recn)
        }
        return true
    }

    //from here on we are handling one element

    if (!options) options = ''

    //skip dict items that are marked readonly
    //in chart of accounts dictitem is not available
    if (gds.dictitem) {
        var di = gds.dictitem(elementx.id)
        if (!di || di.readonly) return true
    }

    if (msg) {
        elementx.setAttribute('neosysreadonly', msg)
        //activeElement not available everywhere
        //if (elementx.id!=document.activeElement.id)
        if (elementx.id != eventtarget.id) {
            //remove tabindex (unless we are on the element otherwise tab key doesnt work anymore!)
            elementx.oldtabindex = elementx.tabIndex
            elementx.tabIndex = -1
        }

        //if (elementx.onchange)
        // elementx.oldonchange=elementx.onchange
        //elementx.onchange=readonly_onchange
        addeventlistener(elementx, 'change', 'readonly_onchange')

        if (options && options.indexOf('BGCOLOR') >= 0) {
            if (!elementx.oldbgcolor) elementx.oldbgcolor = elementx.style.backgroundColor
            //elementx.style.backgroundColor='#ffffc0'
            var newbgcolor = elementx.parentNode.currentStyle.backgroundColor
            if (!newbgcolor)
            // newbgcolor=document.styleSheets[0].rules[0].style.backgroundColor
                newbgcolor = '#f6f6f6'
            elementx.style.backgroundColor = newbgcolor

            //spans have no type
            if (!elementx.type || !elementx.type.match(gradiocheckboxtypes)) {
                if (!elementx.oldborderwidth) elementx.oldborderwidth = elementx.style.borderWidth
                elementx.style.borderWidth = '1px'

                if (!elementx.oldborderstyle) elementx.oldborderstyle = elementx.style.borderStyle
                elementx.style.borderStyle = 'solid'

                if (!elementx.oldbordercolor) elementx.oldbordercolor = elementx.style.borderColor
                elementx.style.borderColor = '#dddddd'
            }

        }

        if (options && options.indexOf('BORDER') >= 0) {
            if (!elementx.oldborderstyle) elementx.oldborderstyle = elementx.style.borderStyle
            elementx.style.borderStyle = 'none'
        }

    }
    else {
        elementx.removeAttribute('neosysreadonly')
        //if (elementx.oldonchange)
        // elementx.onchange=elementx.oldonchange
        //addeventlistener(elementx,'change',elementx.oldonchange)
        //else
        {
            if (elementx.getAttribute('onchange') || elementx.onchange) {
                elementx.removeAttribute('onchange')
                //removeAttribute does not seem to work on functions so also
                elementx.onchange = null
                //TODO check if this is a memory leak
                //addeventlistener(elementx,'change',function(){})
            }
        }
        if (elementx.oldtabindex) {
            elementx.tabIndex = elementx.oldtabindex
            elementx.oldtabindex = ''
        }
        if (typeof elementx.oldbgcolor != 'undefined') elementx.style.backgroundColor = elementx.oldbgcolor
        if (typeof elementx.oldborderstyle != 'undefined') elementx.style.borderStyle = elementx.oldborderstyle
    }

    //move onto next field if setting current focus field to readonly
    //but only if default tabindex since focusnext cant find the next tabindex properly
    //activeElement not available everywhere
    //if (document.activeElement.getAttribute('neosysreadonly')&&gpreviouselement&&document.activeElement.tabIndex==999)
    if (typeof eventtarget != 'undefined' && eventtarget.getAttribute('neosysreadonly') && gpreviouselement && document.activeElement.tabIndex == 999)
        focusnext(gpreviouselement)

    return true

}

function readonly_onchange(event) {
    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    //called when user starts to change a readonly element

    var element = eventtarget
    var readonlymsg = element.getAttribute('neosysreadonly')
    if (!readonlymsg)
        return

    if (readonlymsg == 'true')
        readonlymsg = ''
    neosysinvalid(readonlymsg)

    setvalue(gpreviouselement, gpreviousvalue)

    return neosyscancelevent(event)

}

function setvalue(element, valueorvalues) {

    //given an element (array or first element for radio/checkbox buttons)
    //set the value of INPUT text/INPUT radio/INPUT checkbox/SPAN/SELECT elements

    if (!element) {
        systemerror('setvalue()', 'required argument "element" is missing')
        return
    }

    //convert element name to element
    var elementid = element.id
    if (typeof element == 'string') {
        var elementid = element
        element = $$(element)
        if (!element) {
            systemerror('setvalue', neosysquote(elementid) + ' element does not exist')
            return
        }
    }

    login('setvalue ' + elementid + ' ' + valueorvalues)

    setvalue2(element, valueorvalues)

    logout('setvalue ' + elementid + ' ' + valueorvalues)

}

function setvalue2(element, value) {

    //unprotected (faster) core of setvalue()
    value = value.toString()

    switch (element.tagName) {
        case 'INPUT':
            {

                switch (element.type) {

                    case 'text':
                        {
                            setneosyslink(element, value)
                            element.value = value
                            break
                        }

                    case 'radio':
                        {
                            var elements = getradiocheckboxelements(element)
                            for (var ii = 0; ii < elements.length; ii++) {
                                var element = elements[ii]
                                if (value.neosyslocate(element.value.toString()) > 0) {
                                    element.checked = true
                                    value = element.value
                                    if (value == 'undefined')
                                        value = ii
                                }
                                else
                                    element.checked = false
                            }

                            break

                        }

                    case 'checkbox':
                        {

                            //multiple values as : or , separated string
                            if (typeof value == 'string' && value.indexOf(':') >= 0) value = value.split(':')
                            if (typeof value == 'string') value = value.split(sm2)

                            var elements = getradiocheckboxelements(element)
                            for (var ii = 0; ii < elements.length; ii++) {
                                var element = elements[ii]
                                if (value.neosyslocate(element.value.toString()) > 0)
                                    element.checked = true
                                else
                                    element.checked = false
                            }

                            break

                        }

                }

                break//INPUT
            }

        case 'TEXTAREA':
            {
                setneosyslink(element, value)

                //http://wiki.ckeditor.net/Developer%27s_Guide/Javascript_API
                if (typeof CKEDITOR != 'undefined') {
                    oEditor = CKEDITOR.instances[element.id]
                    if (oEditor) {
                        //oEditor.SetHTML(value)
                        oEditor.setData(value)
                        break
                    }
                }

                element.value = value
                break
            }

        case 'SPAN':
            {
                setneosyslink(element, value)
                //this puts a char 160 no-break-space to ensure height isnt zero
                //the other solution, putting style min-height doesnt zoom with ctrl+ ctrl-
                if (value == '')
                    value = '&nbsp;'
                element.innerHTML = value
                break
            }

        case 'SELECT':
            {
                setdropdown3(element, null, '', value)
                break
            }

        case 'OPTION':
            {
                setdropdown3(element.parentNode, null, '', value)
                break
            }

        default:
            {
                raiseerror(1000, neosysquote(element) + ' ' + neosysquote(element.tagName) + ' invalid tagName in setvalue2(' + element + ',' + value + ')')
                return
            }

    } //of switch

}

function getradiocheckboxelements(element) {

    var scope
    var elements
    if (Number(element.getAttribute('neosysgroupno'))) {
        var scope = getancestor(element, 'TR')
        if (!scope) {
            raiseerror(1000, element.id + ' has no parentNode in getradiocheckboxelements()')
            return []
        }
    }
    else {
        scope = document
    }
    if (document.getElementsByClassName)
        elements = document.getElementsByClassName('neosysid_' + (element.id ? element.id : element))
    else {
        elements = scope.all[element.id ? element.id : element]
        if (elements && elements.tagName)
            elements = [elements]
    }

    if (!elements.length)
        systemerror('getradiocheckboxelements("' + element.id + '") No elements found.')

    return elements

}

function setneosyslink(element, value) {
    return//always display now
    if (element.getAttribute('neosyslink')) {
        if (element.previousSibling && element.previousSibling.getAttribute('neosyslink')) {
            element.previousSibling.style.display = value ? 'inline' : 'none'
        }
        else if (element.nextSibling && element.nextSibling.getAttribute('neosyslink')) {
            element.nextSibling.style.display = value ? 'inline' : 'none'
        }
    }
}

function getdefault(element) {

    //return default value string or ''

    //make sure is element
    if (typeof element == 'string') {
        element = $$(element)
        if (element && element[0])
            element = element[0]
    }
    assertelement(element, 'getdefault', 'element')

    //default value of a select item is the selected item if not already defaulted
    //suppress otherwise cannot have "" value except as first item eg (X;XXXX:;YYYY)
    //if (!element.getAttribute('neosysdefaultvalue')&&element.tagName=='SELECT')
    //{
    // login('getdefault '+element.id)
    // var defaultvalue=getvalue(element)
    // logout('getdefault '+element.id+' SELECT '+defaultvalue)
    // return defaultvalue
    //}

    //return '' if no default
    if (!element.getAttribute('neosysdefaultvalue'))
        return ''

    login('getdefault ' + element.id)

    //calculate default
    var defaultvalue = neosysevaluate(element.getAttribute('neosysdefaultvalue'), 'getdefault(' + element.id + ')')

    //select elements always have a default
    if (element.tagName == 'SELECT') {
        if (defaultvalue == '' && element.tagName == 'SELECT')
            defaultvalue = getvalue(element)
    }

    //don't default if unique and already present
    if (defaultvalue && element.getAttribute('neosysunique')) {
        var othervalues = getvalues(element.id)
        if (othervalues.neosyslocate(defaultvalue))
            defaultvalue = ''
    }

    //convert numbers to strings and check is a string
    if (typeof defaultvalue == 'number')
        defaultvalue = defaultvalue.toString()
    if (typeof defaultvalue != 'string') {
        neosysinvalid(element.id + ' default returned is ' + typeof defaultvalue + ' - "" used')
        defaultvalue = ''
    }

    logout('getdefault ' + element.id + ' ' + defaultvalue)
    return defaultvalue

}

function setdefault(element, donotupdate) {

    //used in
    //1. newrecordfocus and cleardoc to point to 1st element
    //2. focuson (why? should be setvalue?)
    //3. onfocus at end
    //4. checkrequired

    //if no value, get default and set value

    if (typeof element == 'string') {
        element = $$(element)
        if (element && element[0])
            element = element[0]
    }
    //cannot update anything but key field if not locked or save button not enabled
    if (element.getAttribute('neosysfieldno') != 0 && gKeyNodes && (!glocked || saverecord.getAttribute('disabled'))) {
        //  return true
    }

    //return true if already defaulted

    if (element.dataitem) {
        if (element.dataitem.defaulted) return true
    }
    else {
        //only the first element of radio and checkboxes are bound to data
        //should really check if the first element is already defaulted
        if ('checkbox radio'.indexOf(element.type) >= 0) return true
    }

    //return true if already has a value
    //var value=gds.getcells(element,grecn)[0].text<A HREF="http://localhost/neosys7/NEOSYS/images/">http://localhost/neosys7/NEOSYS/images/</A>
    var value = getvalue(element)
    if (value) return true

    //signal that it has been defaulted (even if there is no default)
    if (element.dataitem)
        element.dataitem.defaulted = true

    login('setdefault ' + element.id)

    //get the default value
    var storegrecn = grecn
    grecn = getrecn(element)
    gdefault = getdefault(element)
    grecn = storegrecn

    //for select elements with no default, the first element is the default
    //is this not done in the binding routine now?
    if (gdefault == '' || gdefault == null || typeof (gdefault) == 'undefined') {
        if (element.childNodes.length)
            gdefault = element.childNodes[0].value
    }

    //if no default return
    if (!gdefault) {
        logout('setdefault ' + element.id + ' no default')
        return true
    }

    //save the default for use eg by custom validation routines to avoid work
    element.neosysdefault = gdefault

    //set the value (externally only)
    //gds.setx(element,grecn,gdefault)

    setvalue(element, oconvertvalue(gdefault, element.getAttribute('neosysconversion')))

    //call the validate/update routine
    if (!donotupdate) {

        //changed to not mess with gpreviouselement
        savegpreviouselement = gpreviouselement
        savegpreviousvalue = gpreviousvalue

        gpreviouselement = element
        gpreviousvalue = ''//gpreviousvalue=value

        if (!validateupdate()) {
            logout('setdefault ' + element.id + ' invalid')
            return false
        }

        //restore
        if (savegpreviouselement != gpreviouselement) {
            gpreviouselement = savegpreviouselement
            gpreviousvalue = savegpreviousvalue
        }

    }

    log(element.id + ' defaulted to ' + gdefault)

    logout('setdefault ' + element.id + ' ' + neosysquote(gdefault))

    return true

}

//''''''''''''''
//'BEFORE UPDATE
//''''''''''''''
function onbeforeupdate(element) {
    //move to validateupdate?

    //ignore updates while closing
    if (gclosing) return false

    login('onbeforeupdate')

    // if (typeof(element)=='undefined')
    // element=eventtarget

    //get grecn to be safe
    grecn = getrecn(element)

    //skip if value not changed
    var value = getvalue(element)
    if (value == gpreviousvalue) {
        logout('onbeforeupdate')
        return true
    }

    //cannot update anything but key field if not locked or save button not enabled
    //if (element.getAttribute('neosysfieldno')!=0&&gKeyNodes&&(!glocked||saverecord.getAttribute('disabled')))
    if (element.getAttribute('neosysfieldno') != 0 && gKeyNodes && (!glocked)) {
        setvalue(gpreviouselement, gpreviousvalue)
        readonlydocmsg()
        logout('onbeforeupdate')
        return neosysinvalid('')
    }

    //post entry processing 
    if (!validate(element)) {
        //this should already have been done in the validate routine
        //element.focus()
        return logout('onbeforeupdate')
    }

    logout('onbeforeupdate')

    return true

}

//only called from validateupdate() so some code might be unnecessary duplicated like getvalue() etc
function validate(element) {

    //new value
    gvalue = getvalue(element).toString()

    login('validate ' + element.id + ' ' + gvalue)

    //extract other infor
    var name = element.name

    //extract global info

    //group number 
    ggroupno = Number(element.getAttribute('neosysgroupno'))

    grecn = getrecn(element)

    log('grecn=' + grecn)

    //old value
    goldvalue = gds.getcells(element, grecn)
    if (goldvalue != null)
        goldvalue = goldvalue[0].text

    //convert to uppercase
    if (!element.getAttribute('neosyslowercase') && element.type != 'radio' && element.type != 'checkbox') {
        var tt = gvalue.toUpperCase()
        if (tt != gvalue) {
            gvalue = tt
            //update it immediately in case something like getkey()
            setvalue(element, gvalue)
        }
    }

    if (element.tagName != 'SELECT') {

        /*
        //invalid character check (nearly all punctuation except - / and #)
        if (element.getAttribute('neosysinvalidcharacters'))
        {

        var invalidcharacters=element.getAttribute('neosysinvalidcharacters')+'\xF8\xF9\xFA\xFB'//\xFC\xFD\xFE\xFF'

        //ignore any valid characters
        if (element.getAttribute('neosysvalidcharacters')) invalidcharacters=invalidcharacters.neosysconvert(element.getAttribute('neosysvalidcharacters'),'')
  
        }
        else
        {
        var invalidcharacters=FMs//'\xF8\xF9\xFA\xFB'//\xFC\xFD\xFE\xFF'
        }
        */

        //invalid character check
        //generally dont allow any field marks (can allow specific ones by putting them in valid characters)
        var invalidcharacters = FMs
        if (element.type == 'checkbox') invalidcharacters = invalidcharacters.neosysconvert(sm2, '')
        if (element.getAttribute('neosysinvalidcharacters')) {
            invalidcharacters += element.getAttribute('neosysinvalidcharacters')

            //valid characters override invalid characters (but be careful not to allow field marks
            if (element.getAttribute('neosysvalidcharacters'))
                invalidcharacters = invalidcharacters.neosysconvert(element.getAttribute('neosysvalidcharacters'), '')

        }

        var temp = gvalue.neosysconvert(invalidcharacters, '')
        if (temp != gvalue) {
            //   alert('Punctuation characters and spaces (except / - and #) are not allowed in key fields and have been removed\n')
            //var charsx=invalidcharacters
            //get unused invalid characters
            var charsx = invalidcharacters.neosysconvert(gvalue, '')
            //get used invalid characters
            charsx = invalidcharacters.neosysconvert(charsx, '')
            //.neosysswap(fm,'&u'+fm.charCodeAt(0)+';')
            neosysinvalid('The following characters are not allowed here.\n' + charsx.neosysswap(' ', ' space '))
            return logout('validate')
        }

        //valid character check
        if (element.getAttribute('neosysvalidcharacters')) {

            var temp = gvalue.neosysconvert(element.getAttribute('neosysvalidcharacters'), '')
            if (temp != '') {
                neosysinvalid('Only the following characters are allowed here.\n\"' + element.getAttribute('neosysvalidcharacters') + '\"')
                return logout('validate')
            }
        }

    }

    //required check
    log('required check')
    if (gvalue == ''
 && element.getAttribute('neosysrequired')
 && neosysenabledandvisible(element)) {
        neosysinvalid(element.getAttribute('neosystitle') + ' is required...')
        return logout('validate')
    }

    log('before file check')
    //file check (skip if has dropdown)
    if (element.getAttribute('neosysfilename')
 && gvalue != ''
 && !element.getAttribute('neosysdropdown')) {

        var filename = element.getAttribute('neosysfilename')
        var key = gvalue

        //neosys hack (possibly not used anymore as .filename='ACCOUNTS' not used?
        if (filename == 'ACCOUNTS') key = '.' + gvalue

        db.request = 'CACHE\rREAD\r' + filename + '\r' + key
        if (!db.send()) {

            if (db.response.indexOf('NO RECORD') >= 0) db.response = neosysquote(gvalue) + ' ' + element.getAttribute('neosystitle') + ' is not on file.'

            neosysinvalid(db.response)

            return logout('validate - not on file ' + gvalue)

        }

    }

    log('before conversion')

    var gvaluebeforeiconv = gvalue

    //if conversion is a routine. eg [NUMBER] [DATE] are standard
    var conversion = element.getAttribute('neosysconversion')
    if (typeof (conversion) != 'string' || conversion.slice(0, 1) != '[') conversion = false

    //input conversion
    if (conversion && gvalue != '') {

        //format is '[functionname,options]'
        var convarray = conversion.slice(1, -1).split(',')
        //if no options, make them ''
        if (convarray.length == 1) convarray[1] = ''

        //see also OCONV below

        var quotechar = (gvalue.indexOf('"') >= 0) ? "'" : '"'
        var expression = convarray[0] + '(' + '"ICONV",' + quotechar + gvalue.replace(/\\/g, '\\\\') + quotechar + ',"' + convarray.slice(1) + '")'
        gmsg = ''
        ivalue = neosysevaluate(expression, 'validate(' + element.id + ') iconv')
        if (typeof ivalue == 'undefined')
            return logout('validate - system error in input conversion')

        //null means failed to convert to internal value therefore invalid
        if (gvalue == null || ivalue == null) {
            //error message (use the conversion program name in the message)
            neosysinvalid(neosysquote(gvalue) + ' is not a valid ' + convarray[0].toLowerCase().replace(/_/g, ' ') + '\r\r' + gmsg)
            return logout('validate - input conversion returned null')
        }

        //switch the data into internal format
        gvalue = ivalue

    }

    //custom validation - data in internal format
    log('before custom validation')
    var storegrecn = grecn
    var elementvalidation = element.getAttribute('neosysvalidation')
    if (elementvalidation) {

        var ok
        if (typeof elementvalidation == 'function')
            ok = elementvalidation()
        else
            ok = neosysevaluate(elementvalidation, 'validate() functioncode')

        if (gvalue == null)
            neosyswarning(element.id + ' validation routine returned gvalue=null')
        if (!ok || gvalue == null) {
            grecn = storegrecn
            neosysinvalid()
            return logout('validate - validation function code returned false or gvalue as null')
        }

    }

    //get grecn again in case any prior lines have been deleted in the validation routine
    //grecn=storegrecn
    grecn = getrecn(gpreviouselement)

    //check for uniqueness for multivalues
    log('before unique check')
    if (gvalue && ggroupno > 0 && element.getAttribute('neosysunique')) {
        var othervalues = gds.getx(element.id, null)
        var ln
        //othervalues[grecn]='' //not needed because only validate if changed
        othervalues[grecn] = '' //put back because of a validation after a multiple choice popup fails
        if (ln = othervalues.neosyslocate(gvalue)) {
            gmsg = neosysquote(gvaluebeforeiconv) + ' is already used in line ' + ln + '.'
            if (element.getAttribute('neosysnonuniquewarning')) {
                if (!confirm('Warning:\n\n' + gmsg, 1)) {
                    logout('validate - not unique warning')
                    return neosysinvalid()
                }
            }
            else {
                logout('validate - not unique')
                return neosysinvalid(gmsg)
            }
        }
    }

    //check for sequential multivalues
    //assumes that numbers will be in number format
    var elementsequence = element.getAttribute('neosyssequence')
    if (gvalue
 && ggroupno > 0
 && elementsequence) {
        var title = element.getAttribute('neosystitle')
        if (elementsequence == 'A') {
            var temp
            if ((temp = getpreviousrow('', true))
   && gvalue < temp) {
                //    alert(typeof gvalue+' '+gvalue+' < '+typeof temp+' '+temp)
                logout('validate - not sequential')
                return neosysinvalid(title + ' cannot be less than ' + title + ' in the previous row above')
            }
            var temp
            if ((temp = getnextrow('', true))
   && gvalue > temp) {
                //    alert(typeof gvalue+' '+gvalue+' > '+typeof temp+' '+temp)
                logout('validate - not sequential')
                return neosysinvalid(title + ' cannot be greater than ' + title + ' in the next row below')
            }
        }
    }

    //output conversion
    log('before output conversion')

    ovalue = validateoconv(element, gvalue)
    if (ovalue == 'undefined'
 || ovalue == null)
        return logout('validate - oconv failed')

    //not needed because setx will update the screen with oconverted data
    //gvalue=ovalue

    logout('validate ' + element.id + ' ' + gvalue)

    return true

}

function validateoconv(element, ivalue) {

    //returns ovalue or null if oconv fails

    //skip if nothing to convert
    ivalue = ivalue.toString()
    var ovalue = ivalue
    if (ovalue == '') return ovalue

    //if conversion is a routine. eg [NUMBER] [DATE] are standard
    var conversion = element.getAttribute('neosysconversion')
    if (typeof conversion != 'string' || conversion.slice(0, 1) != '[') conversion = false
    if (!conversion) return ovalue

    //format is '[functionname,options]'
    var convarray = conversion.slice(1, -1).split(',')
    //if no options, make them ''
    if (convarray.length == 1) convarray[1] = ''

    //convert internal to external format
    //nb cannot cancel after updating element.value
    var quotechar = (ivalue.indexOf('"') >= 0) ? "'" : '"'
    var expression = convarray[0] + '(' + '"OCONV",' + quotechar + ivalue.replace(/\\/g, '\\\\') + quotechar + ',"' + convarray[1] + '")'
    gmsg = ''
    var ovalue = neosysevaluate(expression)
    if (typeof ovalue == 'undefined') {
        return false
    }

    //null means failed to convert to external value therefore invalid
    if (ovalue == null) {
        neosysinvalid(neosysquote(ivalue) + ' is not a valid ' + convarray[0].toLowerCase() + '\r' + gmsg)
        return false
    }

    return ovalue

}

//given an array of field numbers calculate and set their contents
function calcfields(fieldns) {

    login('calcfields')

    if (gKeyNodes && !gloaded)
        return logout('calcfields no record')

    //if list is empty then do all calculated fields
    //exclude real fields (type=F) that have functioncode specified
    var origfieldns = fieldns
    if (fieldns == null) {
        fieldns = []
        var field
        for (var fn = 0; fn < gfields.length; fn++) {
            var field = gfields[fn]
            if (!field) {
                systemerror('calcfields()', 'gfields[' + fn + '] is undefined.')
            }
            else {
                if (field.getAttribute('neosysfunctioncode') && field.getAttribute('neosystype') != 'F') {
                    fieldns[fieldns.length] = fn
                }
            }
        }
    }

    //convert fieldns to an array if necessary
    if (typeof (fieldns) != 'object') fieldns = [fieldns]

    for (var fn = 0; fn < fieldns.length; fn++) {

        if (fieldns[fn] != '' || typeof fieldns[fn] == 'number') {

            var field = gfields[fieldns[fn]]

            //only do calculated fields
            if (field.getAttribute('neosysfunctioncode')) {

                //add dependents of dependents to the list to recalc
                var deps = field.getAttribute('neosysdependents')
                if (deps) {
                    deps = deps.split(';')
                    for (var depn = 0; depn < deps.length; depn++) {
                        //prevent recursion by limiting to 1000 fields
                        if (fieldns.length < 1000)
                            fieldns[fieldns.length] = deps[depn]
                    }
                }

                //recalculate and update record
                gds.regetx(field.id)

            }
        }
    }

    if (typeof origfieldns == 'undefined') gdependents = []

    logout('calcfields')

}

//version to make grecn null and restore it afterwards
function neosysevaluate2(functioncode, callerfunctionname) {
    var storegrecn = grecn
    grecn = null
    var result = neosysevaluate(functioncode, callerfunctionname)
    grecn = storegrecn
    return result
}

function neosysevaluate(functioncode, callerfunctionname) {

    //wrapper to call custom functions or expressions (eg validation etc)

    //returns result of neosysevaluate
    //displays message if the function does not return something and returns ''
    //failure results in error message and returns undefined!!!
    // or if gstepping then dump to de-bugger

    var result

    if (typeof functioncode == 'undefined') {
        systemerror('neosysevaluate()', 'The required argument "functioncode" is missing. Called from\r' + callerfunctionname)
        if (gstepping || gusername == 'NEOSYS') crashhere
        return
    }

    //boolean just gets returned
    if (typeof functioncode == 'boolean')
        return functioncode

    //special code to return today's date 
    if (typeof functioncode == 'string' && functioncode == 'TODAY')
        functioncode = 'neosysdate()'

    //if (gstepping||(!ginitok&&gusername=='NEOSYS'))
    // if (true||gstepping||gusername=='NEOSYS')
    if (gstepping || gusername == 'NEOSYS')
        result = typeof functioncode == 'function' ? functioncode() : eval(functioncode)

    else {
        var e
        try {
            result = typeof functioncode == 'function' ? functioncode() : eval(functioncode)
        }
        catch (e) { }
        if (e) {
            if (typeof callerfunctionname == 'undefined') callerfunctionname = '"not specified"'
            systemerror('neosysevaluate()', 'Error No: ' + e.number + '\rDescription: ' + e.description + '\r\rin ' + callerfunctionname + '\r\r' + functioncode.toString().substr(0, 50))
            neosysinvalid()
            return
        }

    }

    //check that the function returned something
    if (typeof result == 'undefined') {
        neosysinvalid('Error in' + '\r' + functioncode.toString().substr(0, 500))
        systemerror('neosysevaluate()', 'Function returned "undefined"\rCalled from ' + callerfunctionname + '\r\r' + functioncode.toString().substr(0, 500))
        result = ''
    }

    return result

}

/*
function getrecord(elementname,groupn,recn)
{

//if not used anywhere by say dec 2001 then delete
 
//not really necessary as getx does the same?
 
if (typeof groupn=='undefined') groupn=0
 
var rec
var element
if (!groupn)
{
element=gds.data[elementname]
}
else
{
if (typeof recn=='undefined')
{
systemerror('getrecord()','recn is undefined. (element:'+elementname+')')
return ''
}
rec=gds.data['group'+groupn]
if (!rec)
{
systemerror('getrecord()','group '+groupn+' not in gds.data (element:'+elementname+')')
return ''
}
rec=rec[recn]
if (!rec)
{
systemerror('getrecord()','recn '+recn+' is missing in gds.data[group'+groupn+' (element:'+elementname+')')
return ''
}
  
}

if (!element)
{
systemerror('getrecord()',elementname+' is missing from gds.data['+groupn+']['+recn+'] (element:'+elementname+')')
return ''
}
  
if (typeof element.text=='undefined')
{
systemerror('getrecord()','.text property is missing from gds.data['+groupn+']['+recn+'].'+elementname)
return ''
}
  
return element.text
  
}
*/

function oconvertvalue(ivalue, conversion) {
    if (!conversion) return ivalue
    if (typeof (conversion) != 'string' || conversion.slice(0, 1) != '[') return ivalue
    return ivalue.neosysoconv(conversion)
}

//'''''''''''
//'DELETE ROW
//'''''''''''
function deleterow(event, element) {

    login('deleterow')

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    if (!element)
        element = eventtarget

    //fast key repeats generates delete on the table element which can be ignored
    if (element.tagName == 'TABLE')
        return

    var row = getancestor(element, 'TR')
    var tablex = getancestor(row, 'TABLE')
    var rows = tablex.tBodies[0].childNodes
    var groupno = Number(tablex.getAttribute('neosysgroupno'))

    if (groupno == 0) return logout('deleterow group 0')

    //does not work on mac ie5 ... var rown=row.sectionRowIndex
    var rown = row.rowIndex
    if (tablex.tHead)
        rown -= tablex.tHead.rows.length

    //make sure any previous data entered is valid EXCEPT if on the current line benig deleted
    if
  (gpreviouselement
   && (Number(gpreviouselement.getAttribute('neosysgroupno')) != groupno
   || getrecn(gpreviouselement) != rown)
   && !validateupdate()
   )
        return logout('deleterow gprevious invalid')

    //cannot delete if keyed and not locked
    if (gKeyNodes && !glocked) {
        readonlydocmsg()
        return logout('deleterow')
    }

    //alert('rown='+rown)
    var nrows = rows.length
    //var grecn=row.recordNumber-1

    //zzz should be offset when paging
    grecn = rown

    //form specific before row delete function
    var predeleterow = window['form_predeleterow' + groupno]
    if (typeof predeleterow == 'function') {
        if (!neosysevaluate('form_predeleterow' + groupno + '()'))
            return logout('deleterow - predelete false')
    }

    //committed from here on

    gpreviouselement = null
    gpreviousvalue = ''

    gchangesmade = true
    //setdisabled(saverecord,false)
    //setdisabled(releasereopenrecord,false)

    var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
    var pagen = neosysint(grecn / pagesize)

    var id
    if (eventtarget.name)
        id = eventtarget.id
    else
        id = gfields[gtables[groupno][0]].id

    gds.deleterow(groupno, rown)

    //focus on first column of new current row
    if (rown > 0 && rown == (nrows - 1)) rown--//if deleting last row then focus on previous row
    rows = tablex.tBodies[0].childNodes
    var element
    //rows[rown].getElementsByClassName('neosysid_'+id)
    if (document.getElementsByClassName)
        element = rows[rown].getElementsByClassName('neosysid_' + id)[0]
    else
    //using (id) instead of [id] because will return only one?
        element = rows[rown].all(id)
    focuson(element)
    gpreviouselement = element
    gpreviousvalue = getvalue(gpreviouselement)

    //form specific after row delete function
    var postdeleterow = window['form_postdeleterow' + groupno]
    if (typeof postdeleterow == 'function') {
        neosysevaluate('form_postdeleterow' + groupno + '()')
    }

    var deps = tablex.getAttribute('neosysdependents')
    if (deps) {

        calcfields(deps.split(';'))
    }

    logout('deleterow')

}

function deleteallrows(groupnoorelement, fromrecn) {

    login('deleteallrows')

    var groupno = groupnoorelement
    if (typeof groupno == 'object') {
        groupno = groupno.neosysgroupno
        if (!groupno)
            groupno = groupnoorelement[0].neosysgroupno
    }

    if (!fromrecn)
        fromrecn = 0

    if (!gds.data['group' + groupno]) {
        neosyssystemerror('deleteallrows()', 'group number ' + groupnoorelement + ' doesnt exist')
    }

    //insert a blank row (to clear any formatting)
    //if (!exceptfirst)
    gds.insertrow(groupno, fromrecn)

    //delete all rows but the newly inserted first row
    //var recs=gds.data['group'+groupno]
    while (gds.data['group' + groupno].length > (1 + fromrecn)) {
        //delete the last row
        gds.deleterow(groupno, gds.data['group' + groupno].length - 1)
    }

    logout('deleteallrows')

}

function insertallrows(elements, values, fromrecn) {

    login('insertallrows')

    //get first element if an array passed
    //otherwise convert elements to an array
    var element = elements
    if (!element.tagName && element.length && element[0].tagName)
        element = element[0]
    else
        elements = [elements]

    if (!fromrecn)
        fromrecn = 0

    assertelement(element, 'insertallrows', 'element')

    var groupno = Number(element.getAttribute('neosysgroupno'))

    deleteallrows(element, fromrecn)

    //get the group
    var rows = gds.data['group' + groupno]

    //cater for passing one row/one cell value not in array form
    if (typeof values != 'object')
        values = [values]

    for (var rown = 0; rown < values.length; rown++) {

        //insert a new row (except for the last)
        if (rown < (values.length - 1))
            gds.insertrow(groupno, rown + fromrecn)

        var rowvalues = values[rown]
        if (typeof rowvalues != 'object')
            rowvalues = [rowvalues]

        //set the record and form values
        for (var coln = 0; coln < elements.length && coln < rowvalues.length; coln++) {
            gds.setx(elements[coln], rown + fromrecn, rowvalues[coln])
        }

    }

    gchangesmade = true

    //recalculate any dependents
    var dependentfieldnos = ''
    for (var ii = 0; ii < elements.length; ii++) {
        var deps = elements[ii].getAttribute('neosysdependents')
        if (deps)
            dependentfieldnos += ';' + deps
    }
    dependentfieldnos = dependentfieldnos.slice(1).split(';')
    grecn = null

    calcfields(dependentfieldnos)
    // var tablex=getancestor(element,'TABLE')
    // calcfields(tablex.neosysdependents.split(';'))

    logout('insertallrows')

}

function neosysaddrow(groupno) {
    var tablex = $$('neosysgroup' + groupno)
    if (tablex && tablex.noinsertrow) return false
    gds.addrow(groupno)
}

function readonlydocmsg() {
    return neosysinvalid('This document is currently "read only"')
}

//'''''''''''
//'INSERT ROW
//'''''''''''
function insertrow(event, append) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)
    if (eventtarget.tagName == 'BODY')
        return

    //make sure any previous stuff is validatedupdated
    if (!validateupdate(event))
        return false

    login('insertrow')

    //insert blank row does not indicate any data entered
    //gchangesmade=true

    //setdisabled(saverecord,false)
    //setdisabled(releasereopenrecord,false)

    var row = getancestor(eventtarget, 'TR')
    if (!row)
        systemerror('insertrow', eventtarget.id + ' has no tr ancestor')

    var tablex = getancestor(row, 'TABLE')
    if (tablex && tablex.noinsertrow)
        return false

    var tbodies = tablex.tBodies
    var groupno = Number(tablex.getAttribute('neosysgroupno'))
    if (groupno == 0)
        return false

    var rown = row.rowIndex
    if (tablex.tHead) rown -= tablex.tHead.rows.length
    if (append)
        rown++
    grecn = rown

    var trows = tbodies[0].childNodes
    var nrows = trows.length

    //if following row is hidden then expand following hidden rows INSTEAD of inserting
    if (grecn < (nrows - 1) && trows[rown + 1].style.display == 'none') {

        //return to insertrow image
        setinsertimage('insert', trows[grecn], groupno)

        for (rown = grecn + 1; rown < nrows; rown++) {
            if (trows[rown].style.display !== 'none') return true
            //trows[rown].style.display=''
            trows[rown].style.display = ''
        }
        return true
    }

    //cannot update if (locked
    if (gKeyNodes && !glocked) {
        readonlydocmsg()
        return logout('insertrow')
    }

    //form specific before row insert function
    var preinsertrow = window['form_preinsertrow' + groupno]
    if (typeof preinsertrow == 'function') {
        if (!neosysevaluate('form_preinsertrow' + groupno + '()'))
            return logout('insertrow - preinsert false')
    }

    var nrows = tbodies[0].childNodes.length
    var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
    var pagen = neosysint(grecn / pagesize)

    var id
    if (eventtarget.name)
        id = eventtarget.id
    else
        id = gfields[gtables[groupno][0]].id

    gchangesmade = true

    if (append)
        gds.addrow(groupno)
    else
        gds.insertrow(groupno, rown)

    var row = tbodies[0].childNodes[rown]

    //focus on first column of new row
    if (!append) {
        if (document.getElementsByClassName)
            focuson(row.getElementsByClassName('neosysid_' + id)[0])
        else
            focuson(row.all[id])
    }

    //ensure not copied a row with expand marker
    setinsertimage('insert', row, groupno)

    //form specific after row insert function
    var postinsertrow = window['form_postinsertrow' + groupno]
    if (typeof postinsertrow == 'function')
        neosysevaluate('form_postinsertrow' + groupno + '()')

    logout('insertrow')

    return true

}

function setinsertimage(mode, row, groupno) {

    //return to insertrow graphic zzz hardcoded should be located
    //var insertimage=row.childNodes[0].childNodes[0].childNodes[0]
    //var insertimage=row.childNodes[0].neosysfields['insertrowbutton'+groupno]
    var insertimage = row.neosysfields
    if (!insertimage)
        return
    insertimage = insertimage['insertrowbutton' + groupno]
    if (!insertimage)
        return

    if (mode == 'expand') {
        //insertimage.src='../neosys/images/smallexpand.gif'
        insertimage.src = gexpandrowimage
        insertimage.alt = 'Expand hidden rows here (Ctrl+N)'
    }
    else {
        //insertimage.src='../neosys/images/smallinsert.gif'
        insertimage.src = ginsertrowimage
        insertimage.alt = 'Insert a new row here (Ctrl+N)'
    }
    return
}

function openrecord_onclick() {

    login('openrecord')

    if (!validateupdate())
        return false

    //get a reply or return false
    if ((reply = neosyspopup2(openrecord)) == null)
        return logout('openrecord')

    //forget it if no change
    if (reply == getkey())
        return logout('openrecord no change')

    //do not change key if user chooses not to unload an existing document
    if (!closedoc('OPEN')) {
        logout('openrecord user cancelled')
        return false
    }

    opendoc(reply)

    logout('openrecord')

    return true

}

function firstrecord_onclick(event) {
    return nextrecord2(event, 'first')
}

function previousrecord_onclick(event) {
    return nextrecord2(event, -1)
}

function selectrecord_onclick(event) {
    return nextrecord2(event, 0)
}

function nextrecord_onclick(event) {
    return nextrecord2(event, 1)
}

function lastrecord_onclick(event) {
    return nextrecord2(event, 'last')
}

function nextrecord2(event, direction) {

    //direction is 'first', -1, 0, 1, 'last'

    var nextkeys = gkeys
    var nextkeyn = gkeyn

    //goto one of many ... or if select many then reduce list to those selected
    if (direction == 0) {
        var selkeys = neosysdecide('', gkeys, [[0, 'Key']], 0, '', many = true)
        if (!selkeys) return false
        if (selkeys.length > 1) {
            nextkeys = selkeys
            nextkeyn = 1
        }
        else {
            nextkeyn = nextkeys.neosyslocate(selkeys[0]) - 1
        }
    }
    else {

        if (!direction) direction = 1

        //get the next key
        if (direction == 'first') nextkeyn = 0
        else if (direction == 'last') nextkeyn = nextkeys.length - 1
        else {
            nextkeyn += direction
            if (nextkeyn >= nextkeys.length) nextkeyn = 0
            if (nextkeyn < 0) nextkeyn = nextkeys.length - 1
        }
    }

    var nextkey = nextkeys[nextkeyn]

    //switch to new key if new and user accepts to close the current one
    if (nextkey != gkey) {
        //do not change key if user chooses not to unload an existing document
        if (!closedoc('OPEN'))
            return false
        gkeyn = nextkeyn
        opendoc(nextkey)
    }

    gkeys = nextkeys
    gkeyn = nextkeyn

    return true

}

function neosyslink(event, element) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    login('neosyslink')

    neosyscancelevent(event)

    //search next then previous siblings for popup
    //if (element.type!='text') element=element.previousSibling
    //if (element.type!='text') element=element.previousSibling
    if (!element)
        element = eventtarget
    while (element && !element.getAttribute('neosyslink')) {
        element = element.nextSibling
    }
    if (!element || !element.getAttribute('neosyslink')) {
        var element = eventtarget
        while (element && !element.getAttribute('neosyslink')) {
            element = element.previousSibling
        }
    }

    //quit if no link defined
    if (!element || !element.getAttribute('neosyslink'))
        return logout('neosyslink - no link')

    //prevent popups except on the key field unless a record is present
    if (gKeyNodes && !gloaded && element.getAttribute('neosysfieldno') != 0) {
        focuson(gKeyNodes[0])
        return logout('neosyslink - no record')
    }

    grecn = getrecn(element)

    gvalue = getvalue(element, grecn)
    if (!element.getAttribute('neosyslowercase'))
        gvalue = gvalue.toUpperCase()

    var reply = neosysevaluate(element.getAttribute('neosyslink'), 'neosyslink()')

    logout('neosyslink')

    return

}

function neosyspopup(event, element) {

    if (!event)
        event = window.event
    eventtarget = geteventtarget(event)

    //element is only provided from f7/alt+down keyboard events

    login('neosyspopup')

    neosyscancelevent(event)

    //search current and following siblings for popup
    if (!element) {
        element = eventtarget
        while (element && !element.getAttribute('neosyspopup') && element.tagName != 'SELECT') {
            element = element.nextSibling
        }
    }

    //quit if no element
    if (!element)
        return logout('neosyspopup - no element')

    //validateupdate previous field
    //if (element.id!=gpreviouselement.id)
    if (element != gpreviouselement) {
        if (!validateupdate())
            return logout('neosyspopup - validateupdate failed')
    }

    //cannot update anything but key field if not locked or save button not enabled
    if (element.getAttribute('neosysfieldno') != 0 && gKeyNodes && (!glocked || saverecord.getAttribute('disabled'))) {
        readonlydocmsg()
        return logout('neosyspopup - read only document')
    }

    //quit if readonly
    var readonly = element.getAttribute('neosysreadonly')
    if (readonly) {
        if (readonly != 'true')
            neosysinvalid(element.getAttribute('neosysreadonly'))
        return logout('neosyspopup - read only')
    }

    //quit if no popup defined
    if (!element.getAttribute('neosyspopup') && element.tagName != 'SELECT')
        return logout('neosyspopup - no popup')

    //prevent popups except on the key field unless a record is present
    if (gKeyNodes && !gloaded && element.getAttribute('neosysfieldno') != 0) {
        focuson(gKeyNodes[0])
        return logout('neosyspopup - no record')
    }

    grecn = getrecn(element)
    gvalue = getvalue(element)
    gpreviouselement = element
    gpreviousvalue = gvalue

    //get a reply or return false
    /////////////////////////////
    if ((reply = neosyspopup2(element)) == null) {
        focuson(element)
        return logout('neosyspopup - no reply')
    }

    //update the element multiple selections qqq
    //if reply is an array use replace all rows
    if (reply.length > 1
 && Number(element.getAttribute('neosysgroupno'))
 && typeof (reply) == 'object') {
        //var elementid = element.id
        //insertallrows(element, reply)

        var elementid = element.id

        //deselect items already selected
        if (grecn > 0) {
            var prevalues = gds.getx(elementid, null).slice(0, grecn)
            for (var ii = 0; ii < prevalues.length; ++ii) {
                var replyn
                if (replyn = reply.neosyslocate(prevalues[ii]))
                    reply.splice(replyn - 1, 1)
            }
            if (!reply.length)
                return logout('neosysformpopup - no new selections')
        }

        insertallrows(element, reply, grecn)

        //focus on next element AFTER table
        element = $$(elementid)
        if (element && element[0])
            element = element[0]
        focusdirection(1, element, Number(element.getAttribute('neosysgroupno')))

        logout('neosyspopup - insert all rows')
        return true

    }

    //if reply is still an array use the first one
    if (typeof (reply) == 'object')
        reply = reply[0]

    //forget it if no change (changed to allow early update on popup eg pop_target)
    //if (reply==getvalue(element))
    if (reply == gpreviousvalue) {
        focusnext()
        return logout('neosyspopup - no change')
    }

    //do not change key if user chooses not to unload an existing document
    if (element.getAttribute('neosysfieldno') == 0 && gloaded && !closedoc('OPEN')) {
        return logout('neosyspopup - user cancelled unloaddoc')
    }

    //output convert it
    if (element.getAttribute('neosysconversion')) {
        reply = validateoconv(element, reply, reply)
        if (typeof reply == 'undefined' || reply == null) {
            return logout('neosyspopup - oconv failed')
        }
    }

    //setup next onfocus to validateupdate
    //done before calling popup now
    //gpreviouselement=element
    //gpreviousvalue=getvalue(element)

    //update the screen
    setvalue(element, reply)

    //move focus to next field (should trigger validation etc)
    //have to force validation since the focus may already be on the next field
    //if clicked on the popup
    gvalidatingpopup = true
    if (validateupdate())
        focusnext(element)
    gvalidatingpopup = false
    //window.setTimeout('focusnext()',10)
    //validateupdate()

    logout('neosyspopup')

}

function neosyspopup2(element) {

    //given a SELECT item or element with a popupfunction to evaluate, returns a reply or null

    //quit if no popup defined and not SELECT
    var expression = element.getAttribute('neosyspopup')
    if (!expression && element.tagName != 'SELECT') return false

    //evaluate popup expression if provided
    if (expression) {
        var reply = neosysevaluate(expression, 'neosyspopup2()')
    }

    //otherwise build a list and select from the SELECT
    else {

        //build a popup from the SELECT tag
        var values = []
        for (var ii = 0; ii < element.childNodes.length; ii++) {
            var option = element.childNodes[ii]
            values[values.length] = [option.value, option.innerText]
        }

        //allow multiple selection if element is in a group with only one input column
        var multipleselection = false
        if (Number(element.getAttribute('neosysgroupno'))) {
            var tableelement = gtables[Number(element.getAttribute('neosysgroupno'))].tableelement
            var multipleselection = tableelement && tableelement.neosyslastinputcolumnscreenfieldno == tableelement.neosysfirstinputcolumnscreenfieldno
        }

        //get the response(s)
        reply = neosysdecide2('', values, '1', '', '', multipleselection)

        //have to do this to cancel the standard dropdown if they press alt+down then press escape on the popup
        if (!reply) focuson(element)

        for (var ii = 0; ii < reply.length; ii++) {
            reply[ii] = values[reply[ii] - 1][0]
        }

    }

    //undefined, null or '' all return as null
    if
  (typeof (reply) == 'undefined'
  || (typeof (reply) == 'boolean' && reply == false)
  || reply == null
  || (typeof (reply) == 'string' && reply == '')
  ) {
        reply = null

        //if popup returns nothing but gdependents is set then the popup must have updated
        //something itself so recalc dependents
        if (gdependents.length) {
            calcfields(gdependents)
        }
    }

    //if sole key field or the open function, setup a list of keys
    if (reply && ((openrecord && element == openrecord) || (element.getAttribute('neosysfieldno') == 0 && gKeyNodes.length == 1))) {
        gkeyn = 0
        if (typeof reply == 'object') {

            if (reply.length > 1 && reply.length <= 20) {
                var openall = neosysyesnocancel('Open all in separate windows?', 1)
                if (!openall)
                    return false
                if (openall == 1) {
                    for (var keyn = 1; keyn <= reply.length; ++keyn) {
                        //doesnt work when multiple .. due to using cookies to communicate? perhaps chain opening passing remaining keys to each window
                        //windowopen(window.location.href.toString(), { 'key': reply[keyn] })
                        var key = reply[keyn]
                        if (key)
                            windowopen(window.location.href.toString().split('?')[0] + '?key=' + key)
                    }
                    window.location = window.location.href.toString().split('?')[0] + '?key=' + reply[0]
                }
            }

            gkeys = reply
            //if reply is an array use the first
            reply = reply[0]
        }
        else {
            gkeys = [reply]
        }
    }

    return reply

}

function getkeyexternal() {
    return getkey('oconv')
}

function debug(v) {
    if (!confirm(v)) {
        neosysbreak()
    }
}

function getkey(mode) {

    login('getkey ' + mode)

    //returns a string containing the key or '' if any key parts are required and ''
    var key = new Array(gKeyNodes.length)
    for (var ii = 0; ii < gKeyNodes.length; ii++) {
        var temp = getvalue(gKeyNodes[ii])
        if (temp == '' && gKeyNodes[ii].getAttribute('neosysrequired') != false) {
            logout('getkey ""')
            return ''
        }
        if (mode != 'oconv') {
            var conversion = gKeyNodes[ii].getAttribute('neosysconversion')
            if (conversion.slice(0, 1) == '[') temp = neosysiconv(temp, conversion)
        }
        key[gKeyNodes[ii].getAttribute('neosyskeypart') - 1] = temp
    }

    var key
    if (mode != 'oconv') key = key.join('*')
    else key = key.join(' ')

    //debug(key) 

    logout('getkey ' + key)

    return key

}

function setkeyvalues(key) {
    //given a string updates the keyfield(s)
    for (var ii = 0; ii < gKeyNodes.length; ii++) {
        var temp = key.neosysfield('*', Number(gKeyNodes[ii].getAttribute('neosyskeypart')))
        //var conversion=gKeyNodes[ii].getAttribute('neosysconversion')
        //if (conversion.slice(0,1)=='[') temp=temp.neosysoconv(conversion)
        //setvalue(gKeyNodes[ii],temp)
        gds.setx(gKeyNodes[ii].id, '', temp)
    }
}

function mergeattributes(sourceelement, targetelement) {

    //does not work on mac despite docs
    //targetelement.mergeAttributes(sourceelement)

    //targetelement.id=sourceelement.id
    ////targetelement.name=sourceelement.name

    //merges all strin and number attributes except id, outerText and outerHTML
    for (var attribname in sourceelement) {
        var attribute = sourceelement[attribname]
        //alert(attribname)
        try {
            //if (attribname!='id'&&attribname!='name'&&attribname!='outerText'&&attribname!='outerHTML'&&(typeof(attribute)=='string'||typeof(attribute)=='number'))
            if (attribname != 'innerText' && attribname != 'innerHTML' && attribname != 'outerText' && attribname != 'outerHTML' && (typeof (attribute) == 'string' || typeof (attribute) == 'number' || attribute == true || attribute == false)) {
                //    alert(attribname+':'+attribute)
                targetelement.setAttribute(attribname, attribute)
                //    targetelement[attribname]=attribute
            }
        }
        catch (e) {
        }
    }

}

function getrecn(element) {

    if (typeof element == 'undefined') {
        //global variable set in any geteventtarget(event)
        element = eventtarget
    }
    if (typeof element != 'object') {
        systemerror('getrecn', element[0].id + ' ' + element[0].tagName + ' ' + element[0].innerHTML + ' is not an object')
    }

    neosysassertobject(element, 'getrecn', 'element')
    //returns null if element not part of a table

    var recn
    if (!Number(element.getAttribute('neosysgroupno')))
        recn = null
    else {
        var rowx = getancestor(element, 'TR')
        if (!rowx) return null
        var tablex = getancestor(rowx, 'TABLE')
        recn = rowx.rowIndex
        if (tablex && tablex.tHead) recn -= tablex.tHead.rows.length
    }

    return recn

}

function getnextrow(dictid, skipblanks, forward) {
    if (typeof forward == 'undefined') forward = true
    return getpreviousrow(dictid, skipblanks, forward)
}

function getpreviousrow(dictid, skipblanks, forward, getrecord) {

    //given a dictionary id (or use gpreviouselement) and an option to skip over blanks,
    //return the contents of the previous row to the current row determined by grecn
    //forward true means search forwards not backwards

    if (!dictid) dictid = gpreviouselement.id
    var value = ''

    //quick previous search
    if (!forward) {
        if (grecn == 0) return ''
        if (getrecord)
            value = gds.getx(dictid, grecn - 1)[0]
        else
            value = getvalue(dictid, grecn - 1)
        if (value) return value
        if (!skipblanks) return ''
    }

    //more comprehensive search
    // var values=getvalue(dictid,null)
    //var values=getvalues(dictid)
    if (getrecord)
        values = gds.getx(dictid, null)
    else
        values = getvalues(dictid)

    if (forward) {
        var ln = grecn + 1
        var ln2 = skipblanks ? values.length : ln + 1
        for (ln = ln; ln <= ln2; ln++) {
            if ((value = values[ln]) != '') return value
        }
    }
    else {
        var ln = grecn - 1
        var ln2 = skipblanks ? 0 : ln
        for (ln = ln; ln >= ln2 && ln >= 0; ln--) {
            if ((value = values[ln]) != '') return value
        }
    }
    return ''

}

function form_ondblclick() {
    return form_filter('filter')
}

function form_onrightclick() {
    // alert('x')
    return true
}

function form_filter(mode, colidorgroupno, regexp, maxrecn, elem) {

    if (mode == 'refilter') {
        colidorgroupno2 = colidorgroupno
        form_filter('unfilter', colidorgroupno2)
        mode = 'filter'
    }

    if (mode == 'filterfocus') {
        //following only applies to schedules! convert to any form
        if (!gds.getx('VEHICLE_CODE', 0)[0]) {
            focuson('VEHICLE_CODE')
            return false
        }
        textrange = elem.select()
        elem.size = 10
        return true
    }

    //get colid
    var colid = colidorgroupno
    if (!colid) {
        colid = eventtarget.id
        if (!colid) {
            var element = eventtarget.parentNode
            while (element) {
                colid = element.name
                if (colid) break
                element = element.parentNode
            }
        }
    }

    //get dictitem
    if (mode == 'filter' || mode == 'unfilter') {
    }

    //get groupno
    var groupno
    if (typeof colidorgroupno != 'number' && (mode == 'filter' || mode == 'unfilter')) {
        var dictitem = gds.dictitem(colid)
        //if (!dictitem)
        // return neosysinvalid(colid+' dictitem does not exist in form_filter()')
        groupno = dictitem.groupno
        if (!groupno)
            return neosysinvalid()//colid+' is not multivalued for sorting'
    }
    else {
        groupno = colidorgroupno
    }

    //prefilter
    if (typeof form_prefilter == 'function') {
        //neosysevaluate('form_prefilter()','formfunctions_onload()')
        if (!form_prefilter(mode, colid))
            return neosysinvalid()
    }

    //get the table rows
    var tablex = $$('neosysgroup' + groupno)
    if (!tablex)
        return neosysinvalid(colid + ' is not part of a table')
    var trows = tablex.tBodies[0].rows

    //unfilter and exit (not tested or used anywhere)
    if (mode == 'expand') {
        for (var rown = grecn + 1; rown < trows.length; rown++) {
            if (trows[rown].style.display == 'inline') break
            //trows[rown].style.display=''
            trows[rown].style.display = ''
        }
        calcfields()
        return true
    }

    //get the show all button
    var tablexshowall = $$('neosysgroup' + groupno + 'showall')
    var tablexfilter = $$('neosysgroup' + groupno + 'filter')
    if (!tablexshowall) {
        //syserror('form_filter()','Cannot find showall button, are you missing a thead?')
        return true
    }

    //unfilter and exit
    if (mode == 'unfilter') {
        for (var rown = 0; rown < trows.length; rown++) {
            //change to expandrow image
            setinsertimage('insert', trows[rown], groupno)

            //trows[rown].style.display=''
            trows[rown].style.display = ''
        }
        //hide the show all buttons (can be two - in THEAD and TFOOT)
        //tablexshowall.style.display='none'
        showhide('neosysgroup' + groupno + 'showall', false)
        if (typeof tablexfilter != 'undefined' && tablexfilter)
            tablexfilter.size = 3
        calcfields()
        return true
    }

    window.status = 'Filtering, please wait ...'

    //get the value and values to be filtered
    var value
    var values
    var filterall = false
    if (mode == 'filterall') {
        filterall = true
        values = gds.data['group' + groupno]

        //turn filtervalues into a case insensitive regular expression
        filtervalues = document.getElementById('neosysgroup' + groupno + 'filter').value.split(' ')
        for (var ii = 0; ii < filtervalues.length; ++ii) {
            filtervalues[ii] = filtervalues[ii].replace(/([\\,\^,\$,\*,\+,\?,\.,\(,\),\|,\{,\},\[,\]])/g, "\\$1")//convert metacharacters to real characters
            filtervalues[ii] = new RegExp(filtervalues[ii], 'gi')
        }

    } else {
        values = gds.regetx(colid, null)
        if (!regexp && !maxrecn) {
            grecn = getrecn()
            var value = values[grecn]
        }
    }

    //hide unmatched rows
    var lastunhiddenrow = ''
    var anyunhiddenrows = false
    var hiderowns = []
    var lastunhiddenrows = []

    var anyhiddenrows = false
    for (var rown = 0; rown < values.length; rown++) {

        //trows[rown].style.display=(values[rown]==value)?'inline':'none'
        var hiderow = false
        if (trows[rown].style.display == 'none') {
            continue
        }
        else if (regexp) {
            if (values[rown].match(regexp)) hiderow = true
        }
        else if (maxrecn) {
            hiderow = rown > maxrecn
        }
        else if (filterall) {
            var row = values[rown]

            //search all columns
            for (var filtern = 0; filtern < filtervalues.length; ++filtern) {
                hiderow = true
                var filtervalue = filtervalues[filtern]
                /* search in internal data format
                for (var propname in row) {
                //should skip propname 'dbordinal'
                //first matching column indicates row should NOT be hidden
                if (row[propname].text.toString().search(filtervalue) >= 0) {
                hiderow = false
                break
                }
                }
                */
                if (trows[rown].innerText.search(filtervalue) >= 0)
                    hiderow = false
                else {
                    var inputs = trows[rown].getElementsByTagName('input')
                    for (var ii = 0; ii < inputs.length; ++ii) {
                        if (inputs[ii].value.search(filtervalue) >= 0) {
                            hiderow = false
                            break
                        }
                    }
                }
                if (hiderow)
                    break
            }

        } else if (values[rown] != value)
            hiderow = true

        if (hiderow) {
            hiderowns.push(rown)
            if (lastunhiddenrow) {
                lastunhiddenrows.push(lastunhiddenrow)
                lastunhiddenrow = ''
            }
        }
        else {
            var lastunhiddenrow = trows[rown]
            anyunhiddenrows = true
        }

    }

    if (!anyunhiddenrows && !maxrecn) {
        //form_filter('unfilter', colidorgroupno)
    }
    else {
        for (rownn = 0; rownn < hiderowns.length; ++rownn) {
            var rown = hiderowns[rownn]
            trows[rown].style.display = 'none'
            if (tablexshowall)
                tablexshowall.style.display = 'inline'
        }
        //mark last unhidden row as expand image
        for (var rown = 0; rown < lastunhiddenrows.length; ++rown)
        //change to expandrow image
            setinsertimage('expand', lastunhiddenrows[rown], groupno)
        calcfields()
    }

    window.status = ''

    return true

}

var calendar_checkInDatePicker

function form_pop_calendar() {
    //do this so that it pops up after focussing on the entry element
    window.setTimeout('form_popcalendar2()', 100)
    return false
}

function form_popcalendar2() {

    //remove any previous calendar
    //if (calendar_checkInDatePicker) calendar_checkInDatePicker.hide()

    var datevalue = gvalue.toString().neosysiconv('[DATE]')
    if (datevalue) {
        var msdate = new Date()
        msdate.setDate(Number(datevalue.neosysoconv('[DATE,DOM]')))
        msdate.setMonth(Number(datevalue.neosysoconv('[DATE,MONTH]') - 1))
        msdate.setFullYear(Number(datevalue.neosysoconv('[DATE,YEAR]')))
        calendar_checkInDatePicker = new Calendar(msdate);
    }
    else
        calendar_checkInDatePicker = new Calendar();

    /*
    calendar_checkInDatePicker.setMonthNames(new Array("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"));
    calendar_checkInDatePicker.setShortMonthNames(new Array("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"));
    calendar_checkInDatePicker.setWeekDayNames(new Array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"));
    calendar_checkInDatePicker.setShortWeekDayNames(new Array("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"));
    */

    setdateformat()
    //calendar_checkInDatePicker.setFormat("d/M/yyyy");
    //calendar_checkInDatePicker.setFirstDayOfWeek(6);
    calendar_checkInDatePicker.setFormat(gdateformat);
    calendar_checkInDatePicker.setFirstDayOfWeek(Number(gfirstdayofweek));
    calendar_checkInDatePicker.setMinimalDaysInFirstWeek(1);
    calendar_checkInDatePicker.setIncludeWeek(false);

    calendar_checkInDatePicker.create();

    /*
    calendar_checkInDatePicker.onchange = function() 
    {
 
    if (!calendar_checkInDatePicker) return true
  
    setvalue(gpreviouselement,calendar_checkInDatePicker.formatDate())

    //clear the object
    window.setTimeout('calendar_checkInDatePicker=null',1)
  
    focusnext(gpreviouselement)
    return true
    }
    */
    function calendar_checkInDatePicker_onchange() {

        if (!calendar_checkInDatePicker) return true

        setvalue(gpreviouselement, calendar_checkInDatePicker.formatDate())

        //clear the object
        window.setTimeout('calendar_checkInDatePicker=null', 1)

        focusnext(gpreviouselement)
        return true
    }
    //dont use addeventlistener here because onchange is special to DatePicker
    calendar_checkInDatePicker.onchange = calendar_checkInDatePicker_onchange

    calendar_checkInDatePicker.show(gpreviouselement)
    calendar_checkInDatePicker._calDiv.focus()
    //calendar_checkInDatePicker._calDiv.setActive()
    //NB activeElement not available everywhere
    //xyz=window.setInterval('window.status=document.activeElement.outerHTML',10)
    return false

}

//drop down any "modal" popup divs
//return true if any closed or false if none
function form_closepopups() {

    var anyclosed = false

    if (calendar_checkInDatePicker) {
        try {
            calendar_checkInDatePicker.hide()
        }
        catch (e) {
            if (gusername == 'NEOSYS') neosysnote('couldnt drop calendar\r' + e.description)
        }
        calendar_checkInDatePicker = ''
        anyclosed = true

    }

    return anyclosed

}

function form_val_index(filename, fieldname, allownew) {

    //check if an existing

    if (!filename) filename = gdatafilename
    if (!fieldname) fieldname = gpreviouselement.name

    //get the existing values
    var indexvalues = form_get_index(filename, fieldname)
    if (!indexvalues) return true
    indexvalues = indexvalues.group1

    //ok if is an existing indexvalue (Exact Case)
    for (var ii = 0; ii < indexvalues.length; ii++) {
        if (indexvalues[ii][fieldname].text == gvalue) {
            return true
        }
    }

    //optionally change case
    for (var ii = 0; ii < indexvalues.length; ii++) {
        if (indexvalues[ii][fieldname].text.toUpperCase() == gvalue.toUpperCase()) {
            if (!(reply = neosysyesnocancel('Change the capitalisation of your entry?\r\rFrom: ' + gvalue.neosysquote() + '\r\r  To: ' + indexvalues[ii][fieldname].text.neosysquote(), 1))) return neosysinvalid()
            if (reply == 1) gvalue = indexvalues[ii][fieldname].text
            return true
        }
    }

    //option to allow/prevent new index values
    if (!allownew) return neosysinvalid(gvalue + ' ' + fieldname.toLowerCase() + ' does not exist')

    //confirm is new index value
    if (gvalue && (neosysyesno(' Is ' + neosysquote(gvalue) + ' to be a new ' + fieldname.toLowerCase() + ' for ' + filename.toLowerCase() + ' ?', 1) != 1)) return false

    return true

}

function form_get_index(filename, fieldname) {

    db.request = 'CACHE\rGETINDEXVALUESXML\r' + filename + '\r' + fieldname
    if (!db.send()) {
        neosysinvalid(db.response)
        return ''
    }

    if (!db.data || db.data == '<records></records>') return ''

    return neosysxml2obj(db.data)

}

function form_pop_index(filename, fieldname, many) {

    //provides a simple way to select previously entered values on an indexed field

    if (!filename) filename = gdatafilename
    if (!fieldname) fieldname = gpreviouselement.name

    //get index values
    var indexvalues = form_get_index(filename, fieldname)
    if (!indexvalues || indexvalues.group1.length == 0) return neosysinvalid('No records have been entered on ' + filename.toLowerCase() + ' yet')

    //user selects index value(s)
    var result = neosysdecide2('', indexvalues, [[fieldname, fieldname.neosyscapitalise()]], 0, '', many)

    return result

}

function copyrecord_onclick() {

    if (!gkey || !glastkey || !gds.isnewrecord || gchangesmade) return neosysinvalid('To copy a record you must:\r\r1. Open the record to copy\r2. Start a new record&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\r3. Click the Copy button&nbsp;&nbsp;&nbsp;&nbsp;')

    //read the record to be copied
    var copyrecord = []
    if (!copyrecord.neosysread(gdatafilename, glastkey))
        return neosysinvalid(copyrecord.neosysresponse)

    //remove any uncopyable data
    for (var dictname in gds.dict) {
        var dictitem = gds.dictitem(dictname)
        if (dictitem && dictitem.type && dictitem.type == 'F' && typeof dictitem.copyable != 'undefined' && !dictitem.copyable)
            copyrecord[dictitem.fieldno] = ''
    }

    //validate copy
    //if (!neosysevaluate('form_oncopy()','copyrecord_onclick()'))
    if (!form_oncopy(copyrecord))
        return neosysinvalid()

    //load the new record
    gro.revstr = copyrecord.slice(1).join(fm)
    gro.rev2obj()
    gds.load(gro)

    return true

}

function copydictitem(dictitem, element) {

    for (var propertyname in dictitem) {
        if (typeof element['neosys' + propertyname] == 'undefined' && typeof dictitem[propertyname] != 'undefined') {
            //element['neosys'+propertyname]=dictitem[propertyname]
            //use setAttribute because only msie will clone expando properties and needed for row cloning
            //store false as "" otherwise since attributed are stored as strings it becomes "false"
            //which does not evaluate to false
            var value = dictitem[propertyname]
            if (value === false)
                value = ''
            element.setAttribute('neosys' + propertyname, value)
        }
    }

    element.name = dictitem.name

    //will be setup later
    //element.id=dictitem.name
    ////element.name=dictitem.name

    //key elements default to required
    //(no longer as the next seq key can be provided by the database)
    //if (element.getAttribute('neosysfieldno')=='0'&&(typeof(element.getAttribute('neosysrequired'))=='undefined'||element.getAttribute('neosysrequired')=='false'))
    //{
    // //element.setAttribute('neosysrequired','true')
    // element.setAttribute('neosysrequired',true)
    //}

    if (!element.getAttribute('neosysrequired') || element.getAttribute('neosysrequired') == 'false')
        element.setAttribute('neosysrequired', '')

}

