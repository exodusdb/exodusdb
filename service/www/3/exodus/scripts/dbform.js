// Copyright NEOSYS All Rights Reserved.

var gdatatagnames = /(^SPAN$)|(^INPUT$)|(^TEXTAREA$)|(^SELECT$)/
var gtexttagnames = /(^SPAN$)|(^INPUT$)|(^TEXTAREA$)/
var gradiocheckboxtypes = /(^radio$)|(^checkbox$)/
//var nbsp160 = String.fromCharCode(160)

gnewimage = gimagetheme + 'application_form_add.png'
//gnewimage = gimagetheme + 'application.png'//'add.png'
gopenimage = gimagetheme + 'open.gif'
gfindimage = gimagetheme + 'zoom.png'
gcalendarimage = gimagetheme + 'calendar.gif'
gsaveimage = gimagetheme + 'tick.png'
gsavegreyimage = gimagetheme + 'tickgrey.png'
gcopyimage = gimagetheme + 'page_copy.png'
gcloseimage = gimagetheme + 'cross.png'
//greleaseimage = gimagetheme + 'lock.png'
//geditimage = gimagetheme + 'key.png'
greleaseimage = gimagetheme + 'application_form_pencil.png'
geditimage = gimagetheme + 'pencil_edit.png'
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
    if (window.opener && window.opener.gwindowopenparameters) {
        gwindowopenerparameters = window.opener.gwindowopenparameters
        //delete the parameters so that they are only used once
        window.opener.gwindowopenparameters = null
    }
}
catch (e) {
}
if (gwindowopenerparameters) {
    for (var paramname in gwindowopenerparameters)
        gparameters[paramname] = gwindowopenerparameters[paramname]
}
if (gparameters.readonlymode)
    greadonlymode = true
//gparameters.openreadonly allows edit/release

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
var gelementthatjustcalledsetchangesmade
var gallowsavewithoutchanges = false//allows locked records (with keys) to be saved anyway
var glocked = false//true means record is locked and available for edit
var grelockingdoc = false
var gwindowunloading = false//this seems to be about closing, not unloading and perhaps should be renamed
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
var grows = null
var gfocusonelement
var gcell
var gvalue
var goldvalue
var gorigvalue
var gvalidatingpopup
var gtimers
//var gsavemode replaced by gparameters.savemode
var greadonlymode
var gupdateonlymode
var gpreventcreation//no new
var gpreventdeletion//no delete
var gcalendarscript//1 if needed, 2 if script inserted, 3 if script loaded

//form customisation (must be in a script loaded prior to this dbform script)
//currently CANNOT have any async db or ui
if (typeof (form_preinit) == 'function')
    form_preinit()

//add a script for the data file definition in case not included in the main html
if (typeof gmodule == 'undefined') {
    gmodule = ''
    exodussettimeout('formfunctions_onload', 10)
}
if (typeof gdictfilename == 'undefined' || !gdictfilename)
    gdictfilename = gdatafilename
if (typeof gdatafilename == 'undefined' || !gdatafilename)
    gdatafilename = gdictfilename
//  var temp='<SCR'+'IPT onload="yield* formfunctions_onload()" id=maindict src="scripts/'+gdictfilename.toLowerCase()+'_dict.js"></SCR'+'IPT>'
if (eval('typeof dict_' + gdictfilename + '=="undefined"')) {
    var temp = '<SCR' + 'IPT id=maindict src="scripts/' + gdictfilename.toLowerCase() + '_dict.js"></SCR' + 'IPT>'
    document.writeln(temp)
}

var gds

//'WINDOW LOAD
//''''''''''''

function* formfunctions_onload() {

    wstatus('Initialising, please wait ...')

    //has the problem that clicking on popup images "activates" the surrounding TD!
    //old method is to instate onfocus on all exodus data elements

    //document.body.onactivate=document_onfocus
    //document.body.onfocus='yield* document_onfocus(event)'
    //document.body.onfocus=document_onfocus
    var activateorfocus = typeof document.body.onactivate == 'undefined' ? 'focus' : 'activate'
    addeventlistener(document.body, activateorfocus, 'document_onfocus')

    gds = new exodusdatasource
    gds.onreadystatechange = gds_onreadystatechange

    //flag to later events that onload has not finished (set true at end of window_onload)
    ginitok = false

    //make sure not "framed"
    if (window != window.top)
        top.location.href = self.location.href

    //login('formfunctions_onload')

    //form customisation
    if (typeof form_onload == 'function') {
        if (!(yield* exodusevaluate('yield* form_onload()', 'yield* form_onload()')))
            return
    }

    //make sure we are logged in
    if (!gusername)
        db.login()

    //done in client.js after db
    //gbasecurr=exodusgetcookie2('bc')
    //gbasefmt=exodusgetcookie2('bf')

    //check a parameter
    if (typeof gmodule == 'undefined') {
        systemerror('yield* formfunctions_onload()', 'gmodule is not defined')
        return false //logout('formfunctions_onload - no gmodule')
    }

    //check dictionary function exists
    var temp
    try {
        temp = eval('dict_' + gdictfilename)
    }
    catch (e) {
    }
    if (typeof temp != 'function') {
        systemerror('dict_' + gdictfilename + ' dict function not defined')
        return false //logout('formfunctions_onload - dict function missing')
    }

    //gro = new exodusrecord(yield* exodusevaluate('dict_' + gdictfilename + '(gparameters)', 'yield* formfunctions_onload()'), gdatafilename)
    var dictfunctionname = (guseyield ? 'yield * ' : '') + 'dict_' + gdictfilename
    var dictarray = yield* exodusevaluate(dictfunctionname + '(gparameters)', 'yield* formfunctions_onload()')
    gro = new exodusrecord(dictarray, gdatafilename)

    gds.dict = gro.dict

    //calendar popup functions
    if (gcalendarscript == 1) {
        var element = document.createElement('script')
        element.src = EXODUSlocation + 'scripts/calendar.js'
        //document.body.insertBefore(element, null)
        //solve htmlfile invalid argument error
        document.body.insertBefore(element, document.body.firstChild)
        //document.writeln('<SCR'+'IPT src="'+EXODUSlocation+'scripts/calendar.js" type="text/javascript"></SCR'+'IPT>')
    }

    //greadonlymode=false

    //set the datasource of all elements
    datasrcelements = []

    var element = document.body
    while (element = nextelement(element)) {

        //skip weird elements
        if (typeof element.getAttribute == 'unknown' || !element.getAttribute)
            continue

        //ensure buttons havew tabIndex 999 - to make them come last
        if (element.tagName == 'BUTTON' && !element.tabIndex)
            element.tabIndex = 999

        //backward compatible with old style datafld attributes
        var datafld = null
        if (!element.id) {
            try {
                datafld = element.getAttribute('datafld')
                if (!datafld)
                    datafld = element.getAttribute('datafld')
                if (datafld)
                    element.id = datafld
            } catch (e) {
            }
        }

        //force cursor on exodustype B for button
        if (element.getAttribute('exodustype') == 'B' && element.style)
            element.style.cursor = 'pointer'

        //NB inserting elements within the loop means that the same element
        //may be processed more than once so ensure skip on 2ndtime
        //  if(typeof element.getAttribute('exodusgroupno')=='undefined'&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
        //element.name ... gives error "class does not support automation"
        //  if(typeof element.getAttribute('exodusgroupno')=='undefined'&&element.name&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
        //  if(typeof element.getAttribute('exodusgroupno')=='undefined'&&element.tagName!='OPTION'&&element.name!=''&&typeof element.name!='undefined')
        var fieldname = element.id
        if (!fieldname)
            continue

        //if (typeof element.getAttribute('exodusgroupno')=='undefined'
        //    &&element.tagName!='OPTION'
        //    &&(fieldname=element.name))
        if (element.getAttribute('exodusgroupno') == null && element.tagName != 'OPTION') {
            //locate the dictionary item otherwise skip the field
            var dictitem = gro.dictitem(fieldname)
            if (!dictitem) {
                //dont error if id not in dictionary because could be non form element but do error if datafld specified
                if (datafld)
                    systemerror('yield* formfunctions_onload()', 'Form element ' + exodusquote(fieldname) + ' is not in the ' + exodusquote(gdictfilename) + ' dictionary\nor is not in the correct group.')
                continue
            }

            //authorisation to make any field read only on bound forms (files)
            //eg CLIENT UPDATE "MARKET"
            //eg CLIENT UPDATE "MARKET CODE"
            if (gKeyNodes) {
                var words = fieldname.split('_')
                for (var wordn = 0; wordn < words.length; ++wordn) {
                    if (!(yield* exodussecurity(gdatafilename.exodussingular() + ' UPDATE ' + words.slice(0, wordn + 1).join(' ').exodusquote()))) {
                        dictitem.readonly = gmsg
                        break;
                    }
                }
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
                    systemerror('name ' + exodusquote(fieldname) + ' is already in use elsewhere\n' + eval(temp).toString().slice(0, 100) + '\n...')
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

            ///log('formfunctions_onload ' + fieldname)

            //dictionary modifications
            //none - currently done in dictrec builder

            //convert long text input to spans so that it can flow (if length not defined)
            if (element.tagName == 'INPUT' && dictitem.align == 'T') {

                //replace original element
                var newspan = document.createElement('span')
                element.parentNode.replaceChild(newspan, element)
                newspan.style.marginLeft = element.style.marginLeft

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
            if (
                (element.getAttribute('exodusradio') && element.type != 'radio')
                ||
                (element.getAttribute('exoduscheckbox') && element.type != 'checkbox')
            ) {

                //radio has preference over checkbox
                if (element.getAttribute('exodusradio'))
                    element.setAttribute('exoduscheckbox', '')

                //gdefault=yield* getdefault(element)

                //build html for multiple inputs
                var options = (element.getAttribute('exodusradio') ? element.getAttribute('exodusradio') : element.getAttribute('exoduscheckbox')).exodussplit(':;')
                var temp = ''
                var elementtype = element.getAttribute('exodusradio') ? 'radio' : 'checkbox'
                for (var ii = 0; ii < options.length; ii++) {

                    //build an input item
                    temp += '<input type=' + elementtype + ' id=' + fieldname
                    temp += ' style="vertical-align:middle"'
                    if (element.getAttribute('exodusreadonly'))
                        temp += ' disabled=true'
                    //temp+='<span style="white-space: nowrap"><input type='+elementtype+' id='+element.name
                    //must be done to group radio boxes
                    temp += ' name=' + fieldname
                    if (typeof (options[ii][0]) != 'undefined')
                        temp += ' value=' + options[ii][0].toString().exodusquote()

                    //set default but it has to be done again in cleardoc anyway
                    //if (gdefault==options[ii][0]) temp+=' checked=true'

                    //temp+=' onfocus="return yield* document_onfocus(event)">'
                    temp += '>'

                    //postfix the option title
                    if (typeof (options[ii][1]) != 'undefined')
                        temp += '<span style="vertical-align:middle; white-space: nowrap">' + options[ii][1] + '</span>'

                    //horizontal or vertical
                    //if vertical then add <br /> between options
                    //but not after last option (so that next input can appear inline with the last option.
                    if (element.getAttribute('exodushorizontal'))
                        temp += '&nbsp;&nbsp;'
                    else if (ii < options.length - 1)
                        temp += '<br />'
                    //temp+='</span>'+(element.getAttribute('exodushorizontal')?'&nbsp;':'<br />')

                }

                //replace original element
                var newspan = document.createElement('span')
                newspan.innerHTML = temp
                //element.swapNode(temp)
                element.parentNode.replaceChild(newspan, element)

                //setup onfocus and onclick for all boxes
                for (var ii = 0; ii < newspan.childNodes.length; ii++) {
                    element = newspan.childNodes[ii]
                    if (element.type == elementtype) {
                        element.tabIndex = elementtabindex

                        //sadly onfocus doesnt seem to get called on checkboxes in safari 4/chrome 5.0
                        //addeventlistener(element, 'focus', 'document_onfocus')
                        //addlistener doesnt work on rows since cloning rows doesnt clone listeners
                        //addeventlistener(element, 'click', 'onclickradiocheckbox')
                        element.setAttribute('exodusonclick', 'yield* onclickradiocheckbox()')
                    }

                }


                //having converted the element to a SPAN/array of INPUT elements ...
                element = newspan
                continue

            }

            var exodusdropdown = element.getAttribute('exodusdropdown')
            if (exodusdropdown) {

                exodusassertobject(element, 'formfunctions_onload', 'element (exodusdropdown)')
                if (typeof exodusdropdown == 'string')
                    exodusdropdown = exodusdropdown.split(fm)
                var request = exodusdropdown[0]
                var colarray = exodusdropdown[1]
                var noautoselection = exodusdropdown[2]

                //convert element to a SELECT
                //var temp=document.createElement('select '+element.outerHTML.slice(7)
                var selectelement = document.createElement('select')
                copydictitem(dictitem, selectelement)

                //make sure tabindex is copied over
                selectelement.tabIndex = element.tabIndex

                //indicate that it is a single selection, one line drop down.
                selectelement.size = 1

                //element.swapNode(t)
                element.parentNode.replaceChild(selectelement, element)
                selectelement.id = element.id

                //create all the options of the element
                yield* exodussetdropdown(selectelement, 'CACHE\r' + request, colarray, '', noautoselection)

                element = selectelement
                //    element.innerHTML=element.innerHTML+' '

                element.tabIndex = elementtabindex

            }

            //convert to select element
            if (element.getAttribute('exodusconversion')) {

                //conversion is a routine eg [NUMBER] [DATE]
                if (
                    typeof (element.getAttribute('exodusconversion')) == 'string'
                    &&
                    element.getAttribute('exodusconversion').slice(0, 1) == '['
                ) {
                    //do nothing
                }

                //conversion is an array of options
                else {

                    //convert element to a SELECT

                    // if (element.id=='USER_CODE') yield* exodusbreak('x '+element.id+' '+element.getAttribute('exodusrequired')+' '+element.outerHTML)
                    var temp = document.createElement('select')
                    copydictitem(dictitem, temp)

                    //if (element.id=='USER_CODE') yield* exodusbreak('y '+temp.id+' '+temp.getAttribute('exodusrequired')+' '+temp.outerHTML)
                    temp.size = 1
                    //element.swapNode(t)
                    element.parentNode.replaceChild(temp, element)
                    origid = element.id
                    element = temp
                    element.id = origid

                    setdropdown3(element, element.getAttribute('exodusconversion'))

                    element.tabIndex = elementtabindex

                    //element.onchange=validateupdate
                    // noyield // addeventlistener(element, 'change', 'validateupdate')
                    /* yield */   addeventlistener(element, 'change', 'form_onchangeselect')
                }
            }

            //onchange for readonly elements
            if (
                element.getAttribute('exodusreadonly')
                &&
                !element.getAttribute('disabled')
                &&
                element.tagName && element.tagName == 'SELECT'
            ) {
                setdisabledandhidden(element, true)
            }
            //onchange for non-select elements (ie9+)
            else if (element.tagName != 'SELECT' && typeof element.oninput != 'undefined') {
                addeventlistener(element, 'input', 'form_oninput')
            }

            //add some events - done on document.body now
            //if (element.tagName.match(gdatatagnames))
            //{
            //}

            //check for duplicate field numbers
            with (dictitem) {
                if (type == 'F' && !dictitem.allowduplicatefieldno) {

                    var fieldandwordn = fieldno
                    if (keypart)
                        fieldandwordn += '.' + keypart
                    if (dictitem.wordno)
                        fieldandwordn += '.' + wordno

                    //skip radio/checkbox elements with the same id
                    if (gds.dict.fieldandwordns[fieldandwordn] != element.id) {

                        if (typeof gds.dict.fieldandwordns[fieldandwordn] != 'undefined') {
                            //throw(new Error(0,exodusquote(name)+' duplicate dictionary field and word no '+fieldandwordn+' is not allowed'))
                            systemerror('formfunction_onload', exodusquote(name) + ' duplicate dictionary name, field no, word no ' + fieldandwordn + ' is not allowed without .allowduplicatefieldno=true\n' + (gds.dict.fieldandwordns[fieldandwordn]))
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
            if (element.getAttribute('exodusfieldno') == '0') {
                if (!gKeyNodes)
                    gKeyNodes = []
                //gKeyNodes[gKeyNodes.length] = element
                gKeyNodes.push(element)
                if (gKeyNodes.length == 1) {
                    var tt = element.getAttribute('exodusprintfunction')
                    if (tt)
                        gKeyNodes[0].setAttribute('exodusprintfunction', tt)
                    var tt = element.getAttribute('exoduslistfunction')
                    if (tt)
                        gKeyNodes[0].setAttribute('exoduslistfunction', tt)
                    element.accessKey = 'K'
                }

            }

            //allow excess spaces in EXODUS data using pre-wrap
            //"Sequences of whitespace are preserved. Lines are broken at newline characters, at <br>, and as necessary to fill line boxes."
            if (element.tagName == 'SPAN' && typeof element.style.whiteSpace != 'undefined') {
                try {
                    element.style.whiteSpace = 'pre-wrap'
                } catch (e) {
                    try {
                        //pre-wrap above errors before IEv8+ XP/Win2003
                        element.style.whiteSpace = 'pre'
                        element.style.wordWrap = 'break-word'
                    }
                    catch (e) {
                    }
                }
            }

            //allow for data entry in SPAN elements (unless hidden)
            if (element.getAttribute('exodustype') == 'F' && element.tagName == 'SPAN' && element.style.display != 'none') {
                var minwidth = element.getAttribute('exoduslength') * 7
                //buggy and not necessary on msie7
                //dont set display block if there is a link or popup so that the image stays to the left of the field
                //if (!isMSIE) {
                if (!isMSIE && !element.getAttribute('exodusreadonly')) {
                    //    element.style.width=(element.getAttribute('exoduslength')*7)+'px'
                    element.style.minWidth = minwidth + 'px'
                    //moved to css_old.css as SPAN min-width:13px;
                    //element.style.minHeight = '13px'
                    //element.style.minHeight='12px'
                    //element.style.maxWidth=(element.getAttribute('exoduslength')*7*2)+'px'
                    //element.style.maxWidth=minwidth+'px'
                    //element.multiLine=true
                    //element.style.display = 'inline-block'
                    //perhaps we ought to be using <div>
                    //chrome and firefox produce different results with inline-block
                    if (element.getAttribute('exoduspopup') || element.getAttribute('exoduslink')) {
                        element.style.display = 'inline-block'
                        //                      element.style.float='left'
                    } else
                        element.style.display = 'block'
                    //element.style.float='left'
                }

                if (!(element.getAttribute('exodusreadonly'))) {
                    element.contentEditable = 'true'
                    //element.contentEditable = true
                    //fixed width in msie but buggy in ff?
                    if (isMSIE) {
                        //setting minWidth only causes problem in plan/schedule dates and extras entry
                        //setting width only causes problem almost everywhere that span data entry has no size initially
                        element.style.minWidth = minwidth + 'px'
                        element.style.Width = minwidth + 'px'
                    }
                    if (!(element.getAttribute('tabindex')))
                        element.setAttribute('tabindex', 999)
                }
            }

            //prevent tab focussing on uneditable spans
            if (element.tagName == 'SPAN' && element.getAttribute('exodustype') == 'S')
                element.tabIndex = -1

            //add button before element with popups (and selects to make it clear to users that F7 is available - especially since useful when selecting multivalues)
            if (
                (
                    element.tagName == 'SELECT'
                    ||
                    element.getAttribute('exoduspopup')
                )
                &&
                !element.getAttribute('exodusreadonly')
                &&
                (
                    element.type == 'text'
                    ||
                    element.type == 'textarea'
                    ||
                    element.isContentEditable
                    ||
                    element.tagName == 'SELECT'
                )
            ) {
                if (typeof element.getAttribute('exoduspopup') == 'string'
                    || element.tagName == 'SELECT') {
                    //conversion is a routine eg [yield* exodusfilepopup(filename,cols,coln,sortselect] [popup.clients]

                    element.style.verticalAlign = 'top'

                    var element2 = document.createElement('img')
                    //add the button right before/after the field
                    element2.id = element.id + '_popup'
                    //                    element2.style.float='left'

                    //ensure popup icon stays to the left of the input field
                    var nowrapper = document.createElement('span')
                    if (element.getAttribute('exodusalign') != 'T')
                        nowrapper.style.whiteSpace = 'noWrap'
                    element = element.parentNode.replaceChild(nowrapper, element)
                    nowrapper.insertBefore(element, null)
                    nowrapper.insertBefore(element2, null)

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

                    element2.style.verticalAlign = 'top'
                    if (fieldname.indexOf('DATE') >= 0)
                        element2.src = gcalendarimage
                    else
                        element2.src = gfindimage
                    element2.title = 'Find a' + ('aeioAEIO'.indexOf(element.getAttribute('exodustitle').slice(0, 1)) != -1 ? 'n' : '') + ' ' + element.getAttribute('exodustitle')
                    element2.title += ' (F7)'
                    element2.style.cursor = 'pointer'

                    //addeventlistener(element2,'click','exoduspopup')
                    element2.setAttribute('isexoduspopup', '1')

                }
            }

            //add button before element for link (or after if right justified)
            if (element.getAttribute('exoduslink')) {
                if (typeof element.getAttribute('exoduslink') != 'string') {
                    systemerror('formfunction_onload', exodusquote(fieldname) + ' link must be a string')
                }
                else {
                    //conversion is a routine eg [yield* exodusfilepopup(filename,cols,coln,sortselect] [popup.clients]

                    element.style.verticalAlign = 'top'

                    var element2 = document.createElement('img')
                    //add the button right after the field

                    //ensure popup icon stays to the left of the input field
                    var nowrapper = document.createElement('span')
                    if (element.getAttribute('exodusalign') != 'T')
                        nowrapper.style.whiteSpace = 'noWrap'
                    element = element.parentNode.replaceChild(nowrapper, element)
                    nowrapper.insertBefore(element, null)
                    nowrapper.insertBefore(element2, null)

                    //insertafter(element,element2)
                    element.parentNode.insertBefore(element2, element)

                    /*
                                        //put nowrap flag on TD in 3 levels
                                        var td = element.parentNode
                                        if (element.tagName != 'SPAN') {
                                            if (td && td.tagName != 'TD') td = td.parentNode
                                            if (td && td.tagName != 'TD') td = td.parentNode
                                            if (td && td.tagName == 'TD') td.noWrap = true
                                        }
                    
                                        if (Number(element.getAttribute('exodusgroupno')) && element.getAttribute('exodusalign') == 'R') {
                                            element.parentNode.insertBefore(element2, null)
                                        }
                                        else {
                                            element.parentNode.insertBefore(element2, element)
                                        }
                    */
                    element2.src = glinkimage
                    //element2.style.border='solid 1px'
                    element2.style.verticalAlign = 'top'
                    //                    element2.style.verticalAlign = 'middle'
                    element2.title = 'Open this ' + element.getAttribute('exodustitle') + ' (F6)'
                    element2.style.cursor = 'pointer'

                    //addeventlistener(element2,'click','exoduslink')
                    element2.setAttribute('isexoduslink', '1')

                }
            }

            //add image element and hide element
            if (element.getAttribute('exodusimage')) {
                var element2 = document.createElement('img')
                element.parentNode.insertBefore(element2, element.nextSibling)
                element.style.display = 'none'
            }

            //use the data field name as the id and name of the element
            //NB the name appears to be lost on databinding table rows
            element.id = fieldname

            //NAME attribute cannot be set at run time on elements dynamically
            // created with the createElement method
            //element.name=element.name

            //group no
            //var groupno=parseInt(element.getAttribute('exodusgroupno'),10)
            var groupno = Number(element.getAttribute('exodusgroupno'))

            //align
            //right align parent TD/TH if in columns (groupn>0)
            if (
                !element.getAttribute('exodusalign')
                &&
                typeof (element.getAttribute('exodusconversion')) == 'string'
                &&
                element.getAttribute('exodusconversion').indexOf('[NUMBER') >= 0
            ) {
                element.setAttribute('exodusalign', 'R')
            }
            //if (groupno>0&&element.getAttribute('exodusalign')=='R'&&'THTD'.indexOf(element.parentNode.tagName)>=0)
            //if (element.getAttribute('exodusalign')=='R'&&'THTD'.indexOf(element.parentNode.tagName)>=0)
            if (
                element.getAttribute('exodusalign') == 'R'
                &&
                (
                    groupno > 0
                    ||
                    getancestor(element, 'TFOOT')
                )
            ) {
                var cellnode = getancestor(element, ' TD TH ')
                if (cellnode && !cellnode.parentNode.align && !cellnode.align) cellnode.align = 'right'
            }

            //length and maxlength
            if (element.tagName.match(gtexttagnames)) {
                if (element.size != 1 && element.getAttribute('exoduslength')) {
                    if (!(parseInt(element.getAttribute('exoduslength')))) {
                        systemerror('yield* formfunctions_onload()', element.id + '.getAttribute("exoduslength")=' + element.getAttribute('exoduslength') + ' is invalid. 10 used.')
                        element.setAttribute('exoduslength', 10)
                    }
                    element.size = parseInt(element.getAttribute('exoduslength'), 10)
                    if (element.size > 2)
                        element.size += 2
                }
                if (element.tagName == 'TEXTAREA') {

                    //doesnt work in ie6 works in ie10 and 11, unknown if works in ie7,8,9
                    if (typeof CKEDITOR != 'undefined'/*&&!isMSIE*/) {

                        CKEDITOR.config.toolbarCanCollapse = true;
                        CKEDITOR.config.toolbarStartupExpanded = false;
                        CKEDITOR.config.enterMode = CKEDITOR.ENTER_BR;
                        CKEDITOR.config.extraPlugins = 'autogrow';
                        CKEDITOR.config.disableNativeSpellChecker = false;
                        CKEDITOR.config.autoGrow_minHeight = 100;

                        //CKEDITOR may not be ready until after first yield* opendoc2()
                        CKEDITOR.on('instanceReady', function (event) {
                            gCKEDITOR_EDITOR = event.editor
                            gCKEDITOR_EDITOR.setReadOnly(!glocked)//also in yield* opendoc2() post read document_onfocus
                        })

                        var verticalpercent = 100
                        if (element.getAttribute('exodusrows'))
                            verticalpercent *= element.getAttribute('exodusrows') / 10

                        //var ockeditor = new CKEDITOR(element.id,'100%',verticalpercent+'%','EXODUS')

                        //http://docs.cksource.com/Talk:CKEditor_3.x/Developers_Guide
                        ockeditor = CKEDITOR.replace(element.id, {
                            extraPlugins: 'autogrow'
                            , autoGrow_maxHeight: 800
                            //                            , autoGrow_minHeight: 100
                        })

                        //element.id is passed as data on event to document_onfocus_sync
                        //which calls document_onfocus to set/check gpreviouselement for changes
                        ockeditor.on('focus', document_onfocus_sync, null, element.id)

                        //doesnt seem to be needed (actually crashes out)
                        //element.onfocus=onfocus
                    }
                    else {

                        element.cols = element.size ? element.size : 30
                        var exodusrows = element.getAttribute('exodusrows')
                        if (exodusrows && exodusrows > 1)
                            element.rows = exodusrows
                    }
                }
                if (element.getAttribute('exodusmaxlength'))
                    element.maxLength = parseInt(element.getAttribute('exodusmaxlength'), 10)
            }

            //lower case
            if (!(element.getAttribute('exoduslowercase'))) {
                if (element.tagName == 'SELECT'
                    //exodus_dict_text(di) now sets lowercase true but can be removed
                    //to allow capitalised flowing text eg ratecard columns
                    //|| element.getAttribute('exodusalign') == 'T'
                    || element.getAttribute('exodustype') == 'S') {
                    element.setAttribute('exoduslowercase', true)
                }
                else {
                    element.setAttribute('exoduslowercase', '')// not 'false' as user properties must be strings not boolean for row bound elements
                }
            }
            //force visual uppercase (actual conversion done in onbeforeupdate)
            if (
                !element.getAttribute('exoduslowercase')
                &&
                element.type != 'radio'
                &&
                element.type != 'checkbox'
            )
                element.style.textTransform = 'uppercase'

            //non calculated fields may be displayed as/converted to uppercase
            //if (element.getAttribute('exodustype')=='F'&&element.tagName!='SPAN')
            if (element.getAttribute('exodustype') == 'F') {

                //as of ie5 we can only focus on elements which have tabindex
                //make them all the same and tab will work nicely
                //tabindex can also be hard coded in the form design
                //use <999 to come before defaults and >999 to come after
                if (!element.tabIndex) {
                    if (element.getAttribute('exodusreadonly')) {
                        element.tabIndex = -1
                    }
                    else {
                        element.tabIndex = 999
                    }
                }

                yield* setfirstlastelement(element)

            }

            //calculated fields can display HTML
            else {
                try {
                    element.dataFormatAs = 'HTML'
                }
                catch (e) {
                    systemerror('yield* formfunctions_onload()', element.name + ' should not be bound to an editable element ' + element.tagName)
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
            if (
                element.tagName.match(gdatatagnames)
                &&
                element.type != 'radio'
                &&
                element.type != 'checkbox'
            ) {

                //spans are only input if .isContentEditable
                if (
                    element.getAttribute('exodustype') == 'F'
                    &&
                    !element.getAttribute('exodusreadonly')
                    &&
                    (
                        element.tagName != 'SPAN'
                        ||
                        element.isContentEditable
                    )
                ) {
                    elementclassname = 'clsNotRequired'
                }
                else {
                    elementclassname = 'clsReadOnly'
                }

                if (
                    (
                        element.getAttribute('exodusrequired')
                        ||
                        element.getAttribute('exodusfieldno') == '0'
                    )
                    //mark elements with default values as required even though probably no data entry if required
                    //&&!element.getAttribute('exodusdefaultvalue')
                    && element.type != 'radio'
                    && element.type != 'checkbox'
                ) {
                    if (
                        element.tagName != 'SELECT'
                        ||
                        (
                            element.tagName == 'SELECT'
                            &&
                            element.options[element.selectedIndex].value == ''
                        )
                    ) {
                        elementclassname = 'clsRequired'
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

            //handle groups

            //build a list of name elements
            var screenfn = gfields.length
            gfields[screenfn] = element

            //element.setAttribute('exodusscreenfn',gfields.length-1)
            //why -1 ???
            element.setAttribute('exodusscreenfn', screenfn)
            //gfields[screenfn].setAttribute('exodusscreenfn',screenfn)

            var rowx = getancestor(element, 'TR')
            var tablex = getancestor(rowx, 'TABLE')

            //set the datasrc of single valued elements
            if (groupno == 0) {

                //but allow it in THEAD/TFOOT elements!
                //prevent group 0 element in rows of a multivalued table
                //while (tablex)
                //{
                // if (Number(tablex.getAttribute('exodusgroupno'))) systemerror('formfunction_onload()',exodusquote(element.name)+' is not multivalued and cannot be in a multivalued TABLE')
                // tablex=getancestor(tablex,'TABLE')
                //}

                //prevent group 0 element in rows of a multivalued table
                if (rowx && Number(rowx.getAttribute('exodusgroupno')))
                    systemerror('formfunction_onload()', exodusquote(element.name) + ' is not multivalued and cannot be in a multivalued TABLE')

                datasrcelements[datasrcelements.length] = element

                //elements with group no - set the data source of repeating lines
            }
            else {

                if (groupno > gmaxgroupno)
                    gmaxgroupno = groupno

                //locate the table element in the parents
                if (tablex == null) {
                    systemerror('formfunction_onload()', exodusquote(element.name) + ' is multivalued and must exist inside a TABLE element')
                }

                //add sorting button to column title
                var titleelement
                if (1 && (titleelement = $$(element.id + '_title'))) {

                    var element2 = document.createElement('img')
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

                    element2.id = 'sortbutton_' + Number(element.getAttribute('exodusgroupno'))
                    element2.src = gsortimage
                    element2.originalsrc = element2.src
                    element2.title = 'Sort by ' + element.getAttribute('exodustitle')
                    //addeventlistener(element2, 'click', 'sorttable')
                    element2.setAttribute('exodusonclick', 'yield* sorttable(event)')
                    element2.sorttableelementid = element.id
                    gsortimages[element.id] = element2

                }

                //any element can determine if emptyrows are allowed
                if (temp = element.getAttribute('exodusallowemptyrows'))
                    tablex.setAttribute('exodusallowemptyrows', temp)

                //any element can determine if a row is required
                if (temp = element.getAttribute('exodusrowrequired'))
                    tablex.setAttribute('exodusrowrequired', temp)

                //need to know the last data entry column
                if (!element.getAttribute('exodusreadonly') && element.tagName.match(gdatatagnames)) {
                    //spans are only input if .isContentEditable
                    if (element.tagName != 'SPAN' || element.isContentEditable) {
                        tablex.setAttribute('exoduslastinputcolscreenfn', element.getAttribute('exodusscreenfn'))
                        //element.setAttribute('exodusislastinputcolumn',true)
                    }
                }
                //nb textarea.isContentEditable is false! so need to test for .type='textarea' or .tagName

                //need to know the first data entry column
                if (
                    !tablex.getAttribute('exodusfirstinputcolscreenfn')
                    && !element.getAttribute('exodusreadonly')
                    && element.tagName.match(gdatatagnames)
                ) {
                    //spans are only input if .isContentEditable
                    if (
                        element.tagName != 'SPAN'
                        || element.isContentEditable
                    ) {
                        tablex.setAttribute('exodusfirstinputcolscreenfn', element.getAttribute('exodusscreenfn'))
                        element.setAttribute('exodusisfirstinputcolumn', true)
                    }
                }

                //prevent use of separator characters unless multiword
                if (element.tagName != 'SELECT') {
                    var wordsep = element.getAttribute('exoduswordsep')
                    if (wordsep && element.getAttribute('exodusnwords') <= 1) {
                        var invalidchars = element.getAttribute('exodusinvalidcharacters')
                        if (!invalidchars)
                            invalidchars = ''
                        element.setAttribute('exodusinvalidcharacters', invalidchars + wordsep)
                    }
                }

                //table already bound and used for group elements
                if (Number(tablex.getAttribute('exodusgroupno'))) {

                    //check that all fields in same table have same group no
                    //zzz should also check that the table has no other name in subtables
                    if (Number(tablex.getAttribute('exodusgroupno')) != groupno) {
                        systemerror('yield* formfunctions_onload()', element.name + ' is in group ' + groupno + ' but the table has already been bound to group ' + Number(tablex.getAttribute('exodusgroupno')))
                        return
                    }

                    //tablex.ncols++
                    //element.coln=tablex.ncols
                    gtables[groupno][gtables[groupno].length] = screenfn

                    //allow finding table element via groupno
                    gtables[groupno].tablex = tablex

                    function* maybe_remove_rowbutton(insertdelete) {
                        if (
                            element.getAttribute('exodusno' + insertdelete + 'row')
                            && !tablex.getAttribute('no' + insertdelete + 'row')
                        ) {
                            tablex.setAttribute('no' + insertdelete + 'row', true)
                            exodusremoveelementsbyid(insertdelete + 'rowbutton' + groupno)
                        }
                    }

                    //maybe remove insertrow/deleterow buttons (in case first group dictionary element does not have the flag)
                    //if (element.getAttribute('exodusnoinsertrow') && !tablex.getAttribute('noinsertrow')) {
                    //if (element.getAttribute('exodusnodeleterow')&& !tablex.getAttribute('nodeleterow')) {
                    yield* maybe_remove_rowbutton('insert')
                    yield* maybe_remove_rowbutton('delete')

                }
                else {

                    //first column is required
                    //this should perhaps not be set since we have rowrequired and allowemptyrows
                    //element.setAttribute('exodusrequired',true)

                    //check this groupno not used on other tables
                    if (gtables[groupno]) {
                        systemerror('yield* formfunctions_onload()', element.name + ' is in group ' + groupno + ' but that group is also used in another table by ' + gfields[gtables[groupno][0]].id)
                        return
                    }

                    //tablex.ncols=0
                    gtables[groupno] = []
                    gtables[groupno][0] = screenfn
                    gtables[groupno].tableelement = tablex
                    tablex.setAttribute('exodusgroupno', groupno)
                    tablex.setAttribute('name', 'group' + groupno)
                    tablex.id = 'exodusgroup' + groupno
                    datasrcelements[datasrcelements.length] = tablex
                    tablex.setAttribute('exodusdependents', '')

                    //unfortunately this can only be set on the first element in the row at the moment
                    //TODO process tables after processing all fields
                    //add insert/delete buttons at the end instead of on the first table element discovered
                    if (element.getAttribute('exodusnoinsertrow'))
                        tablex.setAttribute('noinsertrow', 'noinsertrow')
                    if (element.getAttribute('exodusnodeleterow'))
                        tablex.setAttribute('nodeleterow', 'nodeleterow')

                    //      if (!tablex.className) tablex.className='exodusform'
                    //      tablex.border=1

                    //capture all double clicks for potential filtering
                    //tablex.ondblclick=form_ondblclick
                    addeventlistener(tablex, 'dblclick', 'form_ondblclick')
                    //tablex.oncontextmenu=form_onrightclick

                    //tablex.onreadystatechange=tablex_onreadystatechange
                    addeventlistener(tablex, 'readystatechange', 'tablex_onreadystatechange')

                    //mark group in row to allow check/prevent group0 in same row
                    rowx.setAttribute('exodusgroupno', groupno)

                    //add insert and delete row buttons at the first column in the tbody

                    //button shortcut keys are ctrl+ on mac and alt+ on pc
                    var t2
                    if (isMac) {
                        t2 = 'x'
                        t3 = 'x'
                    }
                    else {
                        //duplicate keycodes in 3 places
                        t2 = '(Ctrl+I or Ctrl+Insert)'
                        t3 = '(Ctrl+D or Ctrl+Delete)'
                    }
                    var t = ''
                    t += ' <span style="white-space: nowrap">'
                    //if (!(exodusgetattribute(element,'exodusnoinsertrow')))
                    if (!(element.getAttribute('exodusnoinsertrow'))) {
                        t += '  <IMG id=insertrowbutton' + groupno
                            + ' title="Insert a new row here ' + t2
                            + '" exodusonclick="yield* insertrow_onclick(event)" src="' + ginsertrowimage
                            + '" style="cursor:pointer;">'//: solid 1px">'
                    }
                    //if (!(exodusgetattribute(element,'exodusnodeleterow')))
                    if (!(element.getAttribute('exodusnodeleterow'))) {
                        t += '  <IMG id=deleterowbutton' + groupno
                            + ' title="Delete this row ' + t3
                            + '" exodusonclick="yield* deleterow_onclick(event)" src ="' + gdeleterowimage
                            + '" style="cursor:pointer">'//: solid 1px">'
                    }
                    t += ' </span>'
                    var insertdeletebuttons = document.createElement('td')
                    insertdeletebuttons.innerHTML = t
                    //insertdeletebuttons.style.borderRightWidth='0px'
                    //insertdeletebuttons.style.paddingRight='3px'

                    //locate the TR element in the parents
                    var trx = getancestor(element, 'tr')
                    if (trx == null)
                        systemerror('formfunction_onload()', exodusquote(element.name) + ' is multivalued and must exist inside a TABLE element')

                    trx.insertBefore(insertdeletebuttons, trx.firstChild)

                    //add page up/down buttons at the first column in the thead and tfoot
                    var pgupdownbuttons = document.createElement('th')
                    pgupdownbuttons.width = '1%'

                    var t = '&nbsp;'
                    t += '<button id=exodusgroup' + groupno + 'showall class=exodusbutton'
                    t += ' style=display:none exodusonclick="yield* form_filter(\'unfilter\',' + groupno + ')"'
                    t += '>Show All</button>'

                    if (groupno == 1 && typeof gallowfilter != 'undefined' && gallowfilter) {
                        t += '<input id="exodusgroup' + groupno + 'filter"'
                        t += ' class="clsNotRequired"'
                        t += ' onblur="yield* form_filter(\'filterall\',' + groupno + ',null,null,this)"'
                        t += ' onfocus="yield* form_filter(\'filterfocus\',' + groupno + ',null,null,this)"'
                        t += ' contenteditable="true"'
                        t += ' size="3"'
                        t += ' tabIndex="-1"'
                        t += ' />'
                    }

                    pgupdownbuttons.innerHTML = t

                    //locate the THEAD element in the parents
                    var thx = tablex.getElementsByTagName('thead')[0]
                    if (thx) {

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
                    var tfx = tablex.getElementsByTagName('tfoot')[0]
                    if (tfx) {
                        pgupdownbuttons = pgupdownbuttons.cloneNode(true)
                        pgupdownbuttons.innerHTML = '&nbsp;'

                        //allow for multiple rows in thead
                        var tfxr = tfx.getElementsByTagName('tr')[0]
                        var tfxrd = tfxr.firstChild
                        tfxr.insertBefore(pgupdownbuttons, tfxrd)
                        pgupdownbuttons.rowSpan = tfx.rows.length

                    }

                }
            }
        } //name element
    }

    //set classname so that we can find multiple elements (screen fields) for the same database field name
    //(which occurs in the case of multivalues (rows), radio buttons and checkboxes)
    //using document.getElementsByClassName()
    id2classname()

    //zero all the dependents (nb exodusdependents is used in gds.js once)
    for (var ii = 0; ii < gfields.length; ii++)
        gfields[ii].setAttribute('exodusdependents', '')

    //for each calculated field add its field number
    // to all elements whose name (in double or single quotes)
    // appear in its functioncode
    for (var fieldn = 0; fieldn < gfields.length; fieldn++) {
        //with (gfields[fieldn])
        var field = gfields[fieldn]

        var deplist = ''
        //   alert(gfields[fieldn].outerHTML)
        if (field.getAttribute('exodustype') == 'S') {
            var functioncode = field.getAttribute('exodusfunctioncode').toString()
            for (var fieldn2 = 0; fieldn2 < gfields.length; fieldn2++) {
                var field2 = gfields[fieldn2]
                if (functioncode.indexOf('"' + field2.getAttribute('exodusname') + '"', 0) >= 0
                    || functioncode.indexOf("'" + field2.getAttribute('exodusname') + "'", 0) >= 0) {
                    var deps = field2.getAttribute('exodusdependents')
                    if (deps)
                        deps += ';'
                    else
                        deps = ''
                    deps += fieldn
                    field2.setAttribute('exodusdependents', deps)

                    if (
                        Number(field2.getAttribute('exodusgroupno'))
                        &&
                        Number(field.getAttribute('exodusgroupno')) != Number(field2.getAttribute('exodusgroupno'))
                    ) {
                        var tablex = getancestor(field2, 'TABLE')
                        var deps = tablex.getAttribute('exodusdependents')
                        if (deps)
                            deps += ';'
                        else
                            deps = ''
                        deps += fieldn
                        tablex.setAttribute('exodusdependents', deps)
                    }

                }
            }
        }
    }

    //get copies of the gfields in case they are deleted from the document (eg table row 1 is deleted)
    for (var fieldn = 0; fieldn < gfields.length; fieldn++) {
        var id = gfields[fieldn].id
        gfields[fieldn] = gfields[fieldn].cloneNode(false)
        gfields[fieldn].id = id
    }

    if (!gfirstnonkeyelement)
        gfirstnonkeyelement = gfields[0]
    if (!gfirstnonkeyelement) {
        systemerror('yield* formfunctions_onload()', 'There are no non-key elements in the form or no data elements')
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
    var buttonhtml = ''

    var buttonalign = gKeyNodes ? 'left' : 'center'

    //wrap form buttons in a span so they align the same as the menu, logout and refresh buttons
    function menubuttonhtml2(id, imagesrc, name, title, accesskey, align) {
        return '<span>' + menubuttonhtml(id, imagesrc, name, title, accesskey, align) + '</span>'
    }

    //bound form buttons NEW/OPEN/EDIT-RELEASE
    if (gKeyNodes) {
        buttonhtml += menubuttonhtml2('newrecord', gnewimage, '<u>N</u>ew', 'Create a new document. ' + AltorCtrl + '+N', 'N')
        buttonhtml += menubuttonhtml2('openrecord', gopenimage, '<u>O</u>pen', 'Open an existing document. ' + AltorCtrl + '+O', 'O')
        buttonhtml += menubuttonhtml2('editreleaserecord', greleaseimage, 'R<u>e</u>lease', 'Edit/Release the current document. ' + AltorCtrl + '+E', 'E')
    }

    //bound and unbound form buttons have OK/SAVE and CLOSE buttons
    buttonhtml += menubuttonhtml2('saverecord', gsaveimage, '<u>S</u>ave', 'Save the current document. ' + AltorCtrl + '+S, Ctrl+Enter or F9', 'S', buttonalign)
    buttonhtml += menubuttonhtml2('closerecord', gcloseimage, '<u>C</u>lose', 'Close the current document. ' + AltorCtrl + '+C', 'C', buttonalign)

    //bound form buttons
    if (gKeyNodes) {

        //COPY
        if (typeof form_copyrecord == 'function')
            buttonhtml += menubuttonhtml2('copyrecord', gcopyimage, 'Copy', 'Copy the current document.', '', buttonalign)

        //DELETE
        //Alt+D not allowed in IE or FF since it goes to the address bar
        if (typeof gpreventdeletion == 'undefined' || gpreventdeletion != false) {
            //buttonhtml += menubuttonhtml2('deleterecord', gdeleteimage, '<u>D</u>elete', 'Delete the current document. ' + AltorCtrl + '+D', 'D', buttonalign)
            buttonhtml += menubuttonhtml2('deleterecord', gdeleteimage, 'Delete', 'Delete the current document.', '', buttonalign)
        }

        //PRINT
        if (gKeyNodes[0].getAttribute('exodusprintfunction'))
            buttonhtml += menubuttonhtml2('printsendrecord', gprintsendimage, '<u>P</u>rint/Send', 'Print/Send this or these documents. ' + AltorCtrl + '+P', 'P', buttonalign)

        //LIST
        var tt2 = gKeyNodes[0].getAttribute('exoduslistfunction')
        if (tt2)
            buttonhtml += menubuttonhtml2('listrecord', glistimage, '<u>L</u>ist', 'List the current file. ' + AltorCtrl + '+L', 'L', buttonalign)

        //NAVIGATION multirecord
        buttonhtml += menubuttonhtml2('firstrecord', gfirstimage, ' ', 'Open the first document. ' + AltorCtrl + '+{', '{', buttonalign)
        buttonhtml += menubuttonhtml2('previousrecord', gpreviousimage, ' ', 'Open the previous document. ' + AltorCtrl + '+[', '[', buttonalign)
        buttonhtml += menubuttonhtml2('selectrecord', gspacerimage, ' ', 'Select document. ' + AltorCtrl + '+^', '^', buttonalign)
        buttonhtml += menubuttonhtml2('nextrecord', gnextimage, ' ', 'Open the next document. ' + AltorCtrl + '+]', ']', buttonalign)
        buttonhtml += menubuttonhtml2('lastrecord', glastimage, ' ', 'Open the last document. ' + AltorCtrl + '+}', '}', buttonalign)

    }

    //login details (database/username)
    var loginhtml = ''
    //loginhtml+='<div style="float:left; white-space:nowrap">'
    loginhtml += '<span style="float:left; vertical-align:middle">'
    loginhtml += gdatasetname + ' - ' + gdataset + ' - ' + gusername + '</span>'
    if (gKeyNodes)
        buttonhtml += loginhtml

    //a span to keep the ok and cancel buttons side by side
    if (!gKeyNodes)
        buttonhtml = '<span style="white-space:nowrap">' + buttonhtml + '</span>'

    //a separator span to keep the ok and cancel buttons from overlapping a floating div
    //if (!gKeyNodes && $$('autofitwindowelement'))
    //    buttonhtml = '<div style="clear:both">&nbsp;</div>' + buttonhtml

    //create the button rank and insert it into the form
    var formbuttons = document.createElement(gKeyNodes ? 'SPAN' : 'DIV')
    if (gKeyNodes) {
        formbuttons.style.padding = 0;
        formbuttons.style.margin = 0;
    }
    //var formbuttons = document.createElement('span')
    //formbuttons.style.float='left'
    //formbuttons.style.float = 'left'
    formbuttons.innerHTML = buttonhtml

    add_exodus_menubar()

    //insert a div that has NO boxes to the left of it so the form (title especially)
    //cannot appear to right of the (left floating) menu buttons
    var temp = document.createElement('div')
    temp.style.cssText = 'clear: left; min-height: 1px;'
    gexodus_menubar.insertBefore(temp, gexodus_menubar.firstChild)

    if (gKeyNodes)
        //document.body.insertBefore(formbuttons, document.body.firstChild)
        gexodus_menubar.insertBefore(formbuttons, gexodus_menubar.firstChild)
    else {

        //login status at the top
        var temp = document.createElement('span')
        temp.style.float = 'left'
        temp.innerHTML = loginhtml
        //temp.style.textAlign='CENTER'
        //document.body.insertBefore(temp, document.body.firstChild)
        gexodus_menubar.insertBefore(temp, gexodus_menubar.firstChild)

        //formbuttons.align='CENTER'
        //NB there is no float "center"
        //formbuttons.style.textAlign = 'center'
        //style="margin-left: auto; margin-right: auto; float: left"
        formbuttons.setAttribute('align', 'center')
        //padding is required because graphic button has VERY LITTLE padding for some reason
        formbuttons.style.paddingTop = '12px'
        formbuttons.style.paddingBottom = '12px'

        //form buttons (save/ok etc.) at the bottom
        document.body.insertBefore(formbuttons, null)//document.body.firstChild)
        //var wholepage = document.getElementById('autofitwindowelement')
        //if (!wholepage)
        //    wholepage = document.body
        //wholepage.insertBefore(formbuttons, null)//document.body.firstChild)

        //alert(formbuttons.parentNode.innerHTML)
    }
    formbuttons.id = 'formbuttonsdiv'
    //formbuttons.style.float='left'
    //attempt to solve vertical alignment of buttons
    //formbuttons.removeNode(false)//msie leaves childnode buttons

    //make global variables to correspond to the buttons
    //to provide backward compatibility with IE code which can refer to document elements like global variables
    var buttonnames = ['new', 'open', 'editrelease', 'save', 'close', 'delete', 'copy', 'list', 'printsend', 'first', 'last', 'select', 'next', 'previous']
    for (var buttonn = 0; buttonn < buttonnames.length; ++buttonn) {
        buttonname = buttonnames[buttonn] + 'record'
        buttonelement = $$(buttonname + 'button')
        window[buttonname] = buttonelement
    }

    //program the various buttons to be visible when enabled
    exodussetexpression(saverecord, 'style:display', 'saverecord.getAttribute("disabled")?"none":""')
    exodussetexpression(closerecord, 'style:display', 'closerecord.getAttribute("disabled")?"none":""')

    //program printsend button to be invisible when disabled
    if (printsendrecord)
        exodussetexpression(printsendrecord, 'style:display', 'printsendrecord.getAttribute("disabled")?"none":""')

    //program new/release/delete buttons to be invisible if disabled
    if (gKeyNodes) {
        exodussetexpression(newrecord, 'style:display', 'newrecord.getAttribute("disabled")?"none":""')
        exodussetexpression(editreleaserecord, 'style:display', 'editreleaserecord.getAttribute("disabled")?"none":""')
        if (deleterecord)
            exodussetexpression(deleterecord, 'style:display', 'deleterecord.getAttribute("disabled")?"none":""')
    }

    if (firstrecord)
        yield* setgkeys([])

    if (newrecord && (greadonlymode || gupdateonlymode || gpreventcreation)) {
        setdisabledandhidden(newrecord, true)
        newrecord.title = 'Creating new records is not allowed'
    }
    if (deleterecord && (greadonlymode || gupdateonlymode || gpreventdeletion)) {
        setdisabledandhidden(deleterecord, true)
        deleterecord.title = 'Deleting records is not allowed'
    }

    var openfunction = ''
    var popupfunction = ''
    if (gKeyNodes) {
        var nvisiblekeys = 0
        for (var keyn = 0; keyn < gKeyNodes.length; keyn++) {
            //find the first openfunction (visible or not)
            if (openfunction = gKeyNodes[keyn].getAttribute('exodusopenfunction'))
                break
            if (exodusenabledandvisible(gKeyNodes[keyn])) {
                //count the number of visible keys
                nvisiblekeys++
                //remember the first visible popupfunction
                if (!popupfunction)
                    popupfunction = gKeyNodes[keyn].getAttribute('exoduspopup')
            }
        }
        //if no openfunction and only one visible key with a popup function
        //then use the popup function as the open function
        if (
            !openfunction
            && nvisiblekeys == 1
            && popupfunction
        )
            openfunction = popupfunction
    }
    //hide the open button if no openfunction
    if (openfunction) {
        openrecord.setAttribute('exoduspopup', openfunction)
        openrecord.style.display = ''
    }
    else if (openrecord)
        openrecord.style.display = 'none'

    //remove record orientated buttons if no key fields
    if (!gKeyNodes) {

        saverecord.tabIndex = 9999 - 1//before menu and logout
        closerecord.tabIndex = 9999 - 1

        /*
        newrecord.style.display='none'
        openrecord.style.display='none'
        deleterecord.style.display='none'
        editreleaserecord.style.display='none'
        //firstrecord.style.display='none'
        //nextrecord.style.display='none'
        //previousrecord.style.display='none'
        //lastrecord.style.display='none'
        */

        //saverecord.value='OK'
        //saverecord.value='OK'
        if (typeof gdisableok != 'undefined' && gdisableok) {
            setdisabledandhidden(saverecord, true)
            //saverecord.style.display='none'
        }
        else {
            setdisabledandhidden(saverecord, false)
            setgraphicbutton(saverecord, 'OK')
            saverecord.title = 'OK - Continue. ' + (gfields.length > 1 ? 'Ctrl+' : '') + 'Enter or F9'
            //saverecord.style.display=''
            saverecord.style.marginRight = '5px'
        }

        //change order of buttons ?
        //saverecord.swapNode(closerecord)
        //following does not work so comment out
        //var tempnode=saverecord.parentNode.replaceChild(closerecord,saverecord)
        //closerecord.parentNode.replaceChild(tempnode,closerecord)

        //no cancel button on main non-database forms
        //if (!gparameters.cancel&&typeof(form_write)=='function'||!window.dialogArguments)
        if (!gparameters.cancel && !window.dialogArguments) {
            setdisabledandhidden(closerecord, true)
        }
        else {
            //closerecord.value='Cancel'
            setgraphicbutton(closerecord, 'Cancel')
            closerecord.title = 'Cancel and exit. Esc'
            setdisabledandhidden(closerecord, false)
        }

    }

    var temp = document.createElement('div')
    //temp.innerHTML='<p align=center style="margin-top:5px"><small>Software by <a tabindex="-1" href="http://www.neosys.com">EXODUS.COM</a></small></p>'
    document.body.insertBefore(temp, null)

    //start focused on the first key field
    if (gKeyNodes) {
        gstartelement = gKeyNodes[0]
        if (!(exodusenabledandvisible(gstartelement)))
            gstartelement = gfirstnonkeyelement
    }
    else
        gstartelement = gfirstnonkeyelement

    //set the document titles from the parameters
    if (gparameters.documenttitle) {
        document.title = gparameters.documenttitle
        gdoctitle = document.title
    }

    //if form has a custom postinit routine
    if (typeof form_postinit == 'function') {
        //login('form_postinit before')
        yield* exodusevaluate('yield* form_postinit()', 'form_functions()')
        //logout('form_postinit after')

        //reverse the effect of any setvalue commands in postinit
        setchangesmade(false)

    }

    //set the title hx element
    gdoctitle = document.title
    var temp = $$('documenttitle')
    if (temp && temp.tagName)
        temp.innerHTML = document.title

    //many routines will exit immediately if this is not set (to avoid further error messages)
    ginitok = true
    wstatus('')

    //yielding code cannot stop to ask questions in onbeforeunload
    window.onbeforeunload = window_onbeforeunload_sync

    //unstoppable - unlocks gkey if glocked
    //addeventlistener(window, 'unload', 'window_onunload')
    window.onunload = window_onunload_sync

    addeventlistener(document, 'keydown', 'document_onkeydown')
    //to prevent ctrl+N opening documents in not msie browsers but kills enter key in msie for some reason
    //if (!isMSIE)
    //    addeventlistener(document, 'keypress', 'document_onkeypress')

    addeventlistener(document, 'click', 'document_onclick')

    addeventlistener(document, 'paste', 'document_onpaste')

    addeventlistener(document, 'copy', 'document_oncopy')

    //record based forms
    if (gKeyNodes) {
        if (gparameters.key) {

            //allow multiple keys to be loaded
            if (typeof gparameters.key == 'object') {
                yield* setgkeys(gparameters.key)
                gparameters.key = gparameters.key[0]
            }

            //necessary to initialise gds else any expressions relying on gds.data will
            //crash after this routine returns
            yield* cleardoc()

            //exodussettimeout('yield* opendoc(' + exodusquote(gparameters.key.replace(/\\/g, '\\\\')) + ')', 1)
            //we cant allow another event like focus to occur before this event is over
            //because there is only one geventhandler to rememeber which yielding function is pending resumption
            //therefore call opendoc immediately - seems to cause no problem
            yield* opendoc(gparameters.key.replace(/\\/g, '\\\\'))
        }
        else
            yield* cleardoc()
    }

    //non-record based forms can get parameters from URL or dialog arguments
    else {

        //initial form can come from a revstr
        if (gparameters.defaultrevstr)
            gro.defaultrevstr = gparameters.defaultrevstr

        yield* cleardoc()

        yield* validateall('filldefaults')

        grecn = null
        yield* calcfields()
        yield* updatedisplay()

    }

    if (gparameters.savebuttonvalue) {
        //saverecord.value=gparameters.savebuttonvalue
        setgraphicbutton(saverecord, gparameters.savebuttonvalue)
    }

    //logout('formfunctions_onload')

}

function* setfirstlastelement(element) {

    //discover first non key input element
    if (element.getAttribute('exodusfieldno') > 0 && !element.getAttribute('exodusreadonly')) {
        if (!gfirstnonkeyelement)
            gfirstnonkeyelement = element
        if (element.tabIndex < gfirstnonkeyelement.tabIndex)
            gfirstnonkeyelement = element
    }

    //discover last input element
    if (element.getAttribute('exodusfieldno') > 0 && !element.getAttribute('exodusreadonly')) {
        if (!gfinalinputelement)
            gfinalinputelement = element
        if (element.tabIndex >= gfinalinputelement.tabIndex)
            gfinalinputelement = element
    }

    return
}

function* setfirstlastcolumn(groupno) {

    var tablex = $$('exodusgroup' + groupno)
    if (!tablex)
        yield* exodusinvalid('yield* setfirstlastcolumn() table' + groupno + ' is missing')

    for (var ii = 0; ii < gtables[groupno].length; ii++) {
        var screenfn = gtables[groupno][ii]
        //shouldnt this also be restricted to elements with exodusfieldno?
        if (
            !gfields[screenfn].getAttribute('exodusreadonly')
            && exodusenabledandvisible(gfields[screenfn].id)
        )
            tablex.setAttribute('exoduslastinputcolscreenfn', screenfn)
    }

}

function* updatedisplay(elements) {

    //seems to be only implemented for non-bound forms

    //be careful not to update elements as it may be a global object

    //option to do all elements recursively
    if (typeof elements == 'undefined')
        return yield* updatedisplay(gfields)

    //do multiple elements recursively
    if (!elements.tagName) {
        for (var ii = 0; ii < elements.length; ii++)
            yield* updatedisplay(elements[ii])
        return
    }

    var element = elements

    //only do elements with a conditional display expression
    if (!(element.getAttribute('exodusdisplay')))
        return

    //login('updatedisplay ' + element.id)

    //determine required display
    var display = yield* gds.evaluate(element.getAttribute('exodusdisplay'))
    if (typeof display == 'undefined') {
        systemerror('yield* updatedisplay(' + element.id + ')', '.display() returned undefined')
    }
    display = display ? '' : 'none'

    //quit if display not changed
    //if (element.style.display == display)
    //    return false //logout('updatedisplay ' + element.id + ' still ' + display)

    //show/hide all elements with the same id
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

    //show/hide associated elements (same id plus '_title')
    var elements = $$(element.id + '_title')
    if (elements) {
        if (elements.tagName)
            elements = [elements]
        for (var ii = 0; ii < elements.length; ii++)
            elements[ii].style.display = display
    }

    //logout('updatedisplay ' + element.id + ' became ' + display)

    return

}

function* element_exodussetdropdown(element, request, noautoselection) {

    assertelement(element, 'element_setdropdown', 'element')

    if (!(element.getAttribute('exodusdropdown'))) {
        systemerror('yield* element_exodussetdropdown()', element.id + ' has no dropdown')
        return
    }

    var dropdown = element.getAttribute('exodusdropdown').split(fm)
    var request = 'CACHE\r' + dropdown[0]
    var colarray = dropdown[1].split('\r')

    yield* exodussetdropdown(element, request, colarray, '', noautoselection)

}

//be careful this is a sync function without _sync in name so only convert exodus3 to exodus2
//or add this function name to the list of functions that dont require yield if converting exodus2 to exodus3
function setgpreviouselement(element, value) {

    if (!element) {
        gpreviouselement = null
        gpreviousvalue = ''
        return
    }

    if (element[0])
        gpreviouselement = element[0]
    else
        gpreviouselement = element

    //handle unusual case that if focused on OPTION instead of SELECT eg from newrecordfocus()
    if (gpreviouselement.tagName == 'OPTION')
        gpreviouselement = gpreviouselement.parentNode

    //set gpreviousvalue as well
    if (typeof value == 'undefined') {
        //assumes grecn set if mv element
        gpreviousvalue = getvalue(gpreviouselement)
    } else
        gpreviousvalue = value
}

function* newrecordfocus() {
    //return
    //login('newrecordfocus')
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
            if (gevent)
                element = gevent.target
            if (!element || !element.getAttribute || !Number(element.getAttribute('exodusfieldno'))) {
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
        yield* checkrequired(gfields, element, 0)
    }

    //required so that if still focused on an element AFTER loading the record
    //then editing the field and moving onto another element causes a proper validateupdate
    //gpreviouselement = $$(element.id)
    //if (gpreviouselement[0])
    //    gpreviouselement = gpreviouselement[0]
    setgpreviouselement($$(element.id))

    //why false??? if (false&&gpreviouselement)
    //false results in repetitive batch number changing still asking discard? when no changes made
    //dont want to do this when in yield* cleardoc() otherwise always starts a batch
    if (gpreviouselement) {
        yield* setdefault(gpreviouselement)
        //gpreviouselement = $$(element.id)
        //if (gpreviouselement && gpreviouselement[0])
        //    gpreviouselement = gpreviouselement[0]
        //gpreviousvalue = getvalue(gpreviouselement)
        setgpreviouselement($$(element.id))
    }

    //prevent focussing on hidden keys
    if (!(exodusenabledandvisible(element)))
        element = gstartelement

    window.scrollTo(0, 0)

    // element.focus()
    tt = element
    if (tt.id)
        tt = tt.id
    //increased to 100 to avoid "xxx is required" when key provided in dialog window
    //by allowing any initial opendoc to do its work first
    exodussettimeout('focuson("' + tt + '")', 100)

    //logout('newrecordfocus')

}

function* gds_onreadystatechange() {

    if (gds.readystate != 'complete') return
    //login('gds_onreadystatechange')
    grecn = null
    yield* calcfields()
    yield* newrecordfocus()
    //logout('gds_onreadystatechange')
}

function* tablex_onreadystatechange(event) {

    event = getevent(event)

    //if need to focus on a table field then
    //do it when the table is complete

    if (event.target.readyState != 'complete') return

    //login('tablex_onreadystatechange ' + Number(event.target.getAttribute('exodusgroupno')))

    yield* newrecordfocus()

    //logout('tablex_onreadystatechange ' + Number(event.target.getAttribute('exodusgroupno')))

}

function* printsendrecord_onclick(event) {

    event = getevent(event)
    exoduscancelevent(event)

    //work out the print function else return
    var printfunction = gKeyNodes && gKeyNodes[0].getAttribute('exodusprintfunction')
    if (!printfunction)
        return
    //printfunction=printfunction.replace(/%KEY%/g,gkeyexternal)
    //journal print does not work properly so ...
    //NB case INsensitive
    printfunction = printfunction.replace(/%KEY%/gi, gkey)
    //alert('DEBUG: saoc')
    yield* validateupdate()
    if (gchangesmade && !(yield* saveandorcleardoc('PRINT'))) {
        focusongpreviouselement()
        return
    }
    //alert('DEBUG: printfunction')
    yield* exodusevaluate(printfunction, 'yield* printsendrecord_onclick()')
    //exodussettimeout("yield* exodusevaluate('"+printfunction+"','yield* printsendrecord_onclick()')",100)

}

function* listrecord_onclick(event) {

    event = getevent(event)
    exoduscancelevent(event)

    //work out the print function else return
    var listfunction = gKeyNodes && gKeyNodes[0].getAttribute('exoduslistfunction')
    if (!listfunction)
        return

    yield* validateupdate()
    listfunction = listfunction.replace(/%KEY%/gi, gkey)

    if (gchangesmade && !(yield* saveandorcleardoc('PRINT'))) {
        focusongpreviouselement()
        return
    }
    yield* exodusevaluate(listfunction, 'yield* listrecord_onclick()')

}

//'WINDOW ONBEFORE UNLOAD
//'''''''''''''''''''''''
function window_onbeforeunload_sync(event) {
    var result = window_onbeforeunload2_sync(event)
    console.log('onbeforeunload returning ' + result)
    return result
}

function window_onbeforeunload2_sync(event) {

    console.log('WINDOW_ONBEFOREUNLOAD')

    //no questions asked if not finished initialisation
    if (!ginitok)
        return undefined

    //persuade user to cancel unload if any request is pending
    if (db.requesting) {
        if (event.preventDefault)
            event.preventDefault()
        return '!!! WAITING FOR REQUEST TO COMPLETE !!!'

        //persuade user to cancel unload if any unsaved changes
    } else if (glocked && gchangesmade) {
        if (event.preventDefault)
            event.preventDefault()
        return '!!! YOU HAVE NOT SAVED YOUR DATA !!!'
    }

    //not possible to cancel from here on

    //makes the unlock synchronous which seems to be more reliable in onbeforeunload
    gonbeforeunload = true

    //ok to unload. dont ask user anything
    return undefined

}

//WINDOW_ONUNLOAD
/////////////////
//function* window_onunload() {
function window_onunload_sync() {

    //alert() is not available in onunload
    console.log('---------- WINDOW_ONUNLOAD SYNC ----------')

    //ensure any unlocking is synchronous - which seems to reach server, unlike async xmlhttp
    gonunload = true

    if (!ginitok)
        return

    //save gdataset in case we are refreshing and the parent window isnt there to get it from
    exodussetcookie('', 'EXODUSlogincode', glogincode, 'logincode')

    //unlock any document before unloading
    //using .next() instead of yield* to call async code
    if (glocked) {

        //Save unlock request for following window to perform hopefully
        //in case following code doesnt successfully unlock its record in onbeforeunload
        var pending = ['UNLOCK', gro.filename, gkey, gro.sessionid].join('\r')
        exodussetcookie(glogincode, 'EXODUSpending', pending)

        console.log('trying to unlock ' + gkey + ' immediately but async request doesnt seem to reach server reliably while unloading')
        //yield* unlockdoc()
        var result = unlockdoc()
        if (result.next)
            result.next()

        //or cancel any pending request
    } else if (gxhttp && gxhttp.status != 200) {
        //        gxhttp.abort()
    }

}

////////////////// DOCUMENT EVENTS /////////////////////

function* document_onclick(event) {

    event = getevent(event)

    if (!event.target.getAttribute) {
        console.log('yield* document_onclick() missing event.target or event.target.getAttribute() ' + event)
        return
    }

    if (event.target && event.target.getAttribute)
        grecn = getrecn(event.target)

    ///log('document_onclick ' + event.target.id)

    var result

    if (event.target.getAttribute('isexoduspopup'))
        result = yield* exoduspopup(event)

    else if (event.target.getAttribute('isexoduslink'))
        result = yield* exoduslink(event)

    //call the first exodusonclick expression found in element then parents
    var target = event.target
    do {
        var onclickexpression = target.getAttribute('exodusonclick')
        target = target.parentNode
    } while (!onclickexpression && target && target.parentNode && target.parentNode.getAttribute)
    if (onclickexpression) {
        //replace trailing "()" with "(event)" to pass event
        //simulating how document.onclick="funcx()" passes event into funcx()
        //exodusevaluate3 will then be able to arrange that
        // the noclick function can refer to the event variable
        result = yield* exodusevaluate(onclickexpression.replace(/\(\)$/, '(event)'), null, 'event', event)
    }

    //logout('document_onclick ' + event.target.id)

    return result
}

function* tabit2() {
    var element = gpreviouselement
    //older defacto ff/chrome/ie9+ (createEvent/initMousEvent+dispatchEvent)
    if (document.createEventxxx) {
        //mozilla http://developer.mozilla.org/en/docs/DOM:event.initMouseEvent
        var eventx = document.createEvent("KeyboardEvent");
        form_blockevents(false, 'tabit2')
        var modifiers = ''
        //if (event.shiftKey)
        //    modifiers+=' Shift'
        eventx.initKeyboardEvent('keydown', true, true, window, 9, 0, modifiers)//, repeat, locale);
        var cancelled = !element.dispatchEvent(eventx);
        form_blockevents(true, 'tabit2')
    }
    //newer standard in ff/chrome and win10 (new MouseEvent+despatchEvent)
    else if (typeof KeyboardEvent != 'undefined') {
        var eventx = new KeyboardEvent('keydown', {
            'view': window,
            'bubbles': true,
            'cancelable': true,
            'keyCode': 9
        });
        form_blockevents(false, 'tabit2')
        var cancelled = !element.dispatchEvent(eventx);
        form_blockevents(true, 'tabit2')
        //older msie pre 9?
    } else
        focusdirection(direction, element, notgroupno)
}

/*
function* document_onkeypress(event) {
    console.log('document_onkeypress()')

    //event=getevent(event)
    //var keycode = event.keyCode ? event.keyCode : event.which
    //console.log('onkeypress '+keycode)
    ////try to block ctrl+N from opening a new page but doesnt work
    //if (event.ctrlKey && keycode == 110)
    //    return exoduscancelevent(event)

    return false
}
*/
//DOCUMENT ON KEY DOWN
//////////////////////
var gonkeydown
function* document_onkeydown(event) {

    //document_onkeydown also occurs in non-form windows not using dbform.js - like confirm.htm/default.js etc

    //prevent keys while child window is open or rapid concurrent keydown events
    if (gonkeydown || $$('uiblockerdiv')) {
        console.log('still responding to gonkeydown')
        //ignore this until fix resuming after ok/cancel
        //return exoduscancelevent(event)
    }

    gonkeydown = true
    var result = yield* document_onkeydown2(event)
    gonkeydown = false

    return result
}

function* document_onkeydown2(event) {

    /*
    8   	Backspace
    9   	Tab
    13  	Enter
    16  	Shift
    17  	Ctrl
    18  	Alt
    19  	Pause, Break
    20  	CapsLock
    27  	Esc
    32  	Space
    33  	Page Up/PgUp
    34  	Page Down/PgDn
    35  	End
    36  	Home
    37  	Left arrow
    38  	Up arrow
    39  	Right arrow
    40  	Down arrow
    38  	Up arrow
    39  	Right arrow
    40  	Down arrow
    44  	PrntScrn
    45	    Insert
    46	    Delete
    48-57	0 to 9
    96-105	0-9 on Numpad
    65-90	A to Z
    91	    WIN Key (Start)
    93	    WIN Menu
    112-123	F1 to F12
    144	    NumLock
    145	    ScrollLock
    
    188	    , <
    190	    . >
    46	    . on Numpad
    191	    / ?
    192 	~
    219	    [ {
    220	    \ |
    221	    ] }
    222	    ' "
    
    Numpad
    45	    Ins
    35  	End
    40  	Down
    34  	PgDown
    37  	Left
    101/12	5
    39  	Right
    36  	Home
    38  	Up
    33  	PgUp
    106 	*
    107 	+
    109 	-
    110/46	. Del
    111     /
    */

    event = getevent(event)
    var keycode = event.keyCode ? event.keyCode : event.which
    var tt = 'onkeydown ' + keycode
    if (event.ctrlKey)
        tt += ' + ctrl'
    if (event.shiftKey)
        tt += ' + shift'
    if (event.altKey)
        tt += ' + alt'
    console.log(tt)

    ///log('document_onkeydown ' + event.target.id + ' ' + keycode)

    //no processing if not initialised
    if (!ginitok)
        return

    ////log(event.target.tagName+' '+event.keyCode)

    //must be before custom key handlers which may use gkey
    gkeycode = keycode
    if (gstepping)
        wstatus(gkeycode)

    //custom key handlers
    //must return false to prevent further action
    if (typeof form_onkeydown == 'function') {
        if (!(yield* form_onkeydown(event))) {
            return exoduscancelevent(event)
        }
    }

    gwindowunloading = false
    element = event.target
    // if (typeof(closerecord)=='undefined') return

    try {
        if (element.tagName == 'TD') {
            if (element.firstChild.tagName == 'SPAN') {
                element = element.firstChild
            }
        }
    }
    catch (e) { }

    //ignore shift and ctrl by themselves
    if (keycode == 16 || keycode == 17)
        return true

    //alert(gkeycode)

    //F5
    //refresh (F5) system key refresh (do manually because mac does not seem to bubble key events)
    if (keycode == 116) {
        if (isMac)
            document.location.reload(true)//true means from server, not cache
        return true
    }

    //F6 is now link
    if (keycode == 117) {
        yield* exoduslink(event)
        return exoduscancelevent(event)
    }

    //F7 is now popup (used to be F2 in DOS) also replaces windows standard alt+down combination
    if (keycode == 118 || (event.altKey && keycode == 40 && element.tagName == 'SELECT')) {
        yield* exoduspopup(event)
        return exoduscancelevent(event)
    }

    //close (F8)
    if (keycode == 119) {
        yield* closedoc('CLEAR')
        return exoduscancelevent(event)
    }

    //alt+k or alt+home is goto top (but alt+home goes to home page and cannot be cancelled)
    if ((keycode == 75 && event.altKey) || (keycode == 36 && event.altKey)) {
        exodussettimeout('focuson(gstartelement.id)', 1)
        return exoduscancelevent(event)
    }

    //menu bar hot keys for non-msie (including msie now) alt keys
    //if (event.altKey && !document.all && [77, 78, 76, 79, 83, 67, 82, 69, 73, 80, 88].exoduslocate(gkeycode)) {
    if (event.altKey && ! event.shiftKey && [71, 78, 76, 79, 83, 67, 82, 69, 77, 80, 88].exoduslocate(gkeycode)) {
        exoduscancelevent(event)
        var found = true
        //alt+m main menu
        if (gkeycode == 77) {
            window.scrollTo(0, 0)
            var menubutton = $$("menubutton");

            /* do it by directly calling menuonmouseover since clicking suffers conflict with onmouseover on mobile
            //older defacto ff/chrome/ie9+ (createEvent/initMousEvent+dispatchEvent)
            if (document.createEvent) {
                //mozilla http://developer.mozilla.org/en/docs/DOM:event.initMouseEvent
                var mouseevent = document.createEvent("MouseEvents");
                form_blockevents(false,'onkeydown2 alt')
                mouseevent.initMouseEvent("click", true, true, window, 0, 0, 0, 0, 0, false, false, false, false, 0, null);
                var cancelled = !menubutton.dispatchEvent(mouseevent);
                form_blockevents(true,'onkeydown2 alt')
            }
            //newer standard in ff/chrome and win10 (new MouseEvent+despatchEvent)
            else if (typeof MouseEvent!='undefined') {
                var mouseevent = new MouseEvent('click', {
                'view': window,
                'bubbles': true,
                'cancelable': true
                });
                form_blockevents(false,'onkeydown2 not mouse')
                var cancelled = !menubutton.dispatchEvent(mouseevent);
                form_blockevents(true,'onkeydown2 not mouse')
            //older msie pre 9?
            } else
            */
            exodussettimeout('menuonmouseover(null,$$("menubutton"),13)', 1)
            //menubutton.click()
        }
        //was done by accesskeys on hidden buttons but firefox requires shift+alt for access
        // unless configure http://kb.mozillazine.org/Ui.key.contentAccess
        // dont use timeout since allows the user interface to resume and send ANOTHER event in FF3
        //else if (gkeycode==76) exodussettimeout('yield* exoduslogout_onclick()',1)
        //Logout and List swapped to be G and L respectively
        else if (gkeycode == 71) yield* exoduslogout_onclick()//g
        else if (gkeycode == 78) yield* newrecord_onclick()//n
        else if (gkeycode == 79) yield* openrecord_onclick()//o
        else if (gkeycode == 83) yield* saverecord_onclick()//s
        else if (gkeycode == 67) yield* closerecord_onclick()//c
        else if (gkeycode == 69) yield* editreleaserecord_onclick()//e
        //else if (gkeycode == 68) yield* deleterecord_onclick()//d reserved
        else if (gkeycode == 76) yield* listrecord_onclick()//l
        else if (gkeycode == 80) yield* printsendrecord_onclick()//p
        else if (gkeycode == 82) yield* refreshcache_onclick()//r
        else
            found = false
        return exoduscancelevent(event)
    }

    //alt+{ is first record
    if (keycode == 219 && event.altKey && event.shiftKey) {
        exodussettimeout('yield* firstrecord_onclick()', 1)
        return exoduscancelevent(event)
    }

    //alt+} is last record
    if (keycode == 221 && event.altKey && event.shiftKey) {
        exodussettimeout('yield* lastrecord_onclick()', 1)
        return exoduscancelevent(event)
    }

    //alt+[ is previous record
    if (keycode == 219 && event.altKey) {
        exodussettimeout('yield* previousrecord_onclick()', 1)
        return exoduscancelevent(event)
    }

    //alt+] is next record
    if (keycode == 221 && event.altKey) {
        exodussettimeout('yield* nextrecord_onclick()', 1)
        return exoduscancelevent(event)
    }

    //alt+^ is select record
    if (keycode == 54 && event.altKey && event.shiftKey) {
        exodussettimeout('yield* selectrecord_onclick()', 1)
        return exoduscancelevent(event)
    }

    //alt+end is goto bottom
    if (keycode == 35 && event.altKey) {
        exodussettimeout('focuson(gfinalinputelement.id)', 1)
        return exoduscancelevent(event)
    }

    //F12 is exodus debug key
    //if (keycode==123/*&&gusername.indexOf('EXODUS')+1*/)
    if (keycode == 123 && gusername.indexOf('EXODUS') + 1) {

        //plain F12 - toggle stepping2
        if (!event.shiftKey && !event.ctrlKey && !event.altKey) {
            gstepping = !gstepping
            wstatus('stepping=' + gstepping)
            return exoduscancelevent(event)
        }

        //ctrl+shift+F12 - break
        //force error and into exodusbreak
        if (event.shiftKey && event.ctrlKey) {
            gstepping = true
            yield* exodusbreak('', 'F12', '')
            return exoduscancelevent(event)
        }

        //alt+shift+F12 - show event log
        if (event.shiftKey && event.altKey) {
            glogging = true
            wstatus('glogging=true')
            if (geventlog) {
                windowx = yield* windowopen()
                if (windowx)
                    windowx.document.body.innerHTML = geventlog
            }
            return exoduscancelevent(event)
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
            var windowx = yield* windowopen()
            if (windowx)
                //windowx.document.body.innerText=decodehtmlcodes(temp.outerHTML?temp.outerHTML:temp.innerHTML)
                windowx.document.body.innerHTML = encodehtmlcodes(temp.outerHTML ? temp.outerHTML : temp.innerHTML)
        }

        return exoduscancelevent(event)

    }

    //F2 is now edit to be like office
    var textrange
    if (keycode == 113) {

        //internet explorer
        if (document.selection
            && document.selection.createRange
            && (textrange = document.selection.createRange()).text != '') {
            textrange.collapse(false)
            textrange.select()
            return exoduscancelevent(event)
        }

        //.selectionStart throws and error on ff/chrome/standard on radio/checkbox etc
        //works on INPUT text only
        if (element.type == 'text') {
            try {
                var selectionStart = element.selectionStart
            } catch (e) {
                var selectionStart = false
            }
            if (typeof selectionStart == 'number') {
                //select nothing if anything selected
                if (element.selectionStart != element.selectionEnd) {
                    element.selectionStart = element.selectionEnd
                    element.selectionEnd = document.activeElement.selectionStart
                    //select all if nothing selected
                } else {
                    element.selectionStart = 0
                    element.selectionEnd = 999999999
                }
                return exoduscancelevent(event)
            }
        }

        //seems to be the most standards based way and works on spans too
        //http://stackoverflow.com/questions/11451353/how-to-select-the-text-of-a-span-on-click
        if (window.getSelection && document.createRange) {
            selection = window.getSelection();
            if (selection.isCollapsed) {
                //following selects all of text
                selection.removeAllRanges();
                range = document.createRange();
                range.selectNodeContents(element);
                selection.addRange(range);
            } else {
                //following removes any selection
                selection.collapseToEnd();
            }
            return exoduscancelevent(event)
        } else if (document.selection && document.body.createTextRange) {
            range = document.body.createTextRange();
            range.moveToElementText(element);
            range.select();
            return exoduscancelevent(event)
        }

    }

    //F9 is also save
    if (keycode == 120) {

        //first update the current field
        //otherwise the db is updated without the last entry!!!
        if (!(yield* validateupdate()))
            return exoduscancelevent(event)

        //prevent document save unless the save button is enabled
        if (!(saverecord.getAttribute('disabled'))) {
            //yield* savedoc()
            yield* saverecord_onclick()
        }

        return exoduscancelevent(event)

    }

    //esc - when no changes means closerecord
    // if(keycode==27&&gloaded)
    if (keycode == 27) {

        //close any "modal" popups
        if (yield* form_closepopups()) {
            //prevent normal esc handling
            return exoduscancelevent(event)
        }

        //if changing current field then let system use it to 'undo' changes
        if (element.name && element == gpreviouselement) {

            value = getvalue(element)

            //old value
            gdefaultvalue = yield* getdefault(element)
            if (gpreviousvalue == '' && gdefaultvalue)
                gpreviousvalue = gdefaultvalue

            if (value != gpreviousvalue) {

                //restore the original value
                //get from datasource
                setvalue(element, gpreviousvalue)

                //if reverting element that setchangesmade then revert that too
                if (element == gelementthatjustcalledsetchangesmade)
                    setchangesmade(false)

                //prevent normal esc handling
                exoduscancelevent(event)

                //select it
                try {
                    if (element.select)
                        element.select()
                }
                catch (e) { }

                return exoduscancelevent(event)
            }
        }

        yield* closerecord_onclick()

        return exoduscancelevent(event)

    }

    //get groupno otherwise return
    ggroupno = element.getAttribute('exodusgroupno')

    //the rest of the keys are only when located on a exodus data entry field
    if (typeof ggroupno == 'undefined' || ggroupno == null) {
        if (keycode == 37 || keycode == 38) {
            focusprevious(element)
            return exoduscancelevent()
        }
        if (keycode == 39 || keycode == 40) {
            focusnext(element)
            return exoduscancelevent()
        }
        return true
    }

    ggroupno = Number(ggroupno)

    grecn = null
    grows = null
    if (ggroupno > 0) {

        //get the current table and ggroupno else exit
        var rowx = getancestor(element, 'TR')
        var tablex = getancestor(rowx, 'TABLE')
        if (!tablex)
            return
        grows = tablex.tBodies[0].getElementsByTagName('tr')

        //get the rown else exit
        //if (ggroupno > 0) {
        var rown = rowx.rowIndex
        if (tablex.tHead)
            rown -= tablex.tHead.rows.length
        //}

        //get the recordset
        var rs = gds.data['group' + ggroupno]
        if (!rs) {
            exoduscancelevent(event)
            return yield* exodusinvalid('Group number on non-group field')
        }
        //zzz if "paging" should be offset by subtracting record number of first row
        grecn = rown
        gnrecs = rs.length

        var nrows = grows.length
        var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
        var pagen = exodusint(grecn / pagesize)
        var npages = Math.ceil((rs.length) / pagesize)

        var tablex_firstinputcolscreenfn = form_getfirstinputcolscreenfn(tablex)
        var tablex_lastinputcolscreenfn = form_getlastinputcolscreenfn(tablex)

    }

    //ctrl+G is goto line
    if (keycode == 71 & event.ctrlKey) {
        if (!(yield* validateupdate()))
            return false
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

        return exoduscancelevent(event)
    }

    //alt+Y is copy previous record/column
    if (keycode == 89 && event.altKey && element.type != 'button') {
        if (grecn > 0) {
            setvalue(element, yield* getpreviousrow(element.id, true))
            try { element.select() } catch (e) { }
            return exoduscancelevent(event)
        }
    }

    //in tables goto first/last data entry column of first/last visible row
    //ctrl+home 36 = first col, first row (top left)
    //ctrl+end  35 = last col, last row (bottom right)
    if (tablex && (keycode == 36 || keycode == 35) && event.ctrlKey && !event.shiftKey && !event.altKey) {
        if (keycode == 36) {
            var id = gfields[tablex_firstinputcolscreenfn].id
            var startrown = 0
            var increment = 1
        } else {
            var id = gfields[tablex_lastinputcolscreenfn].id
            var startrown = grows.length - 1
            var increment = -1
        }
        //work forwards from first to last row, or backwards from last to first row, looking for a visible row to focus on
        for (var ii = startrown; ii >= 0 && ii < grows.length; ii += increment) {
            var element = grows[ii].exodusfields[id]
            if (exodusenabledandvisible(element)) {
                focuson(element)
                break
            }
        }
        return exoduscancelevent(event)
    }

    //Enter on textarea
    if (!isMac && keycode == 13 && (element.tagName == 'TEXTAREA' && !event.ctrlKey && !event.shiftKey)) {
        if (gKeyNodes && !glocked) {
            keycode = 9
            event.keyCode = keycode
        } else if (element.value == '' || element.value.slice(-4) == '\r\n\r\n') {
            element.value = element.value.slice(0, -4)
            keycode = 9
            event.keyCode = keycode
        } else if (element.value == '' || element.value.slice(-2) == '\n\n') {
            element.value = element.value.slice(0, -2)
            keycode = 9
            event.keyCode = keycode
        } else {
            //.selectionStart throws and error on ff/chrome/standard on radio/checkbox etc
            try { var selectionStart = element.selectionStart } catch (e) { var selectionStart = false }
            if (typeof selectionStart == 'number' && element.selectionStart == 0 && element.selectionEnd == element.value.length) {
                keycode = 9
                event.keyCode = keycode
            }
        }
        if (event.keyCode != keycode) {
            var direction = event.shiftKey * -2 + 1
            focusdirection(direction, element)
            return exoduscancelevent(event)
        }
    }

    //tab or down or enter on first or last col of LAST row is special
    if ((ggroupno > 0) && grecn == (rs.length - 1)) {
        if ((keycode == 9 || keycode == 13 || keycode == 40) && !event.shiftKey && !event.ctrlKey && !tablex.getAttribute('noinsertrow')) {

            //down on select handled by browser, not EXODUS
            if (keycode == 40 && element.tagName == 'SELECT')
                return exoduscancelevent(event)

            //on last column - add a row
            //exodusfirstinputcolscreenfn
            var firstcolelement = rowx.exodusfields[gfields[gtables[ggroupno][0]].id]
            if ((glocked || !gKeyNodes)
                //&& element.getAttribute('exodusscreenfn') == tablex.getAttribute('exoduslastinputcolscreenfn')
                && element.getAttribute('exodusscreenfn') == tablex_lastinputcolscreenfn
                && getvalue(firstcolelement) != ''
            ) {
                if (glocked || !gKeyNodes) {
                    //exodusaddrow(ggroupno)
                    if (!(yield* form_insertrow(event, true)))
                        return exoduscancelevent(event)
                    //continue on to focus on it
                    //var nextelement=$$(gfields[tablex.getAttribute('exodusfirstinputcolscreenfn')].id)[grecn+1]
                    //focuson(nextelement)
                    focusnext()
                    return exoduscancelevent(event)
                }
            }
            //on first column, if empty - go to next field after current table
            if (element.getAttribute('exodusscreenfn') == tablex_firstinputcolscreenfn && getvalue(element) == '') {
                focusdirection(1, element, ggroupno)
                return exoduscancelevent(event)
            }
        }
    }

    //enter key becomes tab (or ok if only one entry field)
    if (keycode == 13 && (element.tagName != 'TEXTAREA' || event.ctrlKey || event.shiftKey)) {

        //ctrl+enter or enter with single field is ok/save record
        //if (event.ctrlKey||(gfields.length==1&&Number(gfields[0].getAttribute('exodusgroupno'))))
        if (event.ctrlKey
            || (
                !Number(element.getAttribute('exodusgroupno'))
                && gfinalinputelement
                && gfinalinputelement.id == gstartelement.id
            )
        ) {
            if (!(saverecord.getAttribute('disabled'))) {
                //yield* savedoc()
                yield* saverecord_onclick()
            }
            return exoduscancelevent(event)
        }

        //enter on last field goes to start of doc
        if (!event.shiftKey && element == gfinalinputelement && (ggroupno == 0 || (ggroupno > 0 && grecn == gnrecs))) {
            focuson(gstartelement)
            return exoduscancelevent(event)
        }

        //shift+enter on 1st key field goes to end of doc
        if (event.shiftKey && element == gstartelement) {
            focusdirection(-1, element)
            return exoduscancelevent(event)
        }
        /*
                var keyEvt = document.createEvent("KeyboardEvent");
                if (keyEvt.initKeyEvent){
                    //mozilla deprecated
                    keyEvt.initKeyEvent(
                        "keydown",   //event type
                        true,       //bubbled
                        true,       //cancelable
                        null,       //viewArg
                        event.ctrlKey,
                        event.altKey,
                        event.shiftKey,
                        false,      //metaKeyArg
                        keycode,    //virtual keycode of depressed key else 0
                        0           //unicode of depressed key if any else 0
                        );
                    gonkeydown=false
                    element.dispatchEvent(keyEvt);
                    keyEvt.stopPropagation;
                } else if (keyEvt.initKeyboardEvent){
                              // Safari?
                }
        */
        keycode = 9
        event.keyCode = keycode

        //setting keyCode is only allowed on msie
        //if (!document.all && !event.ctrlKey) {
        if (event.keyCode != keycode) {
            var notgroupno
            if (event.shiftKey)
                direction = -1
            else {
                direction = 1
                //enter on first column if required and no default skips (TODO and not rowrequired) first field after the table)
                if (ggroupno
                    && element.getAttribute('exodusrequired')
                    && element.getAttribute('exodusscreenfn') == tablex_firstinputcolscreenfn
                    && !getvalue(element)
                ) {
                    notgroupno = ggroupno
                }
            }

            focusdirection(direction, element, notgroupno)
            //window.setTimeout('yield* tabit2()',1)

            return exoduscancelevent(event)
        }

        //dont return because enter key has some special processing on rows below
        //return

    }

    //all remaining key events are related to loaded records
    if (gKeyNodes && closerecord.getAttribute('disabled'))
        return true

    //left arrow, right arrow, up arrow, down arrow on radio or SELECT all handled by browser
    if (keycode == 37 || keycode == 38 || keycode == 39 || keycode == 40) {
        if (element.type && element.type == 'radio')
            return true
        if (element.tagName == 'SELECT')
            return true
    }

    //up and down keys not on radio or select tags
    if ((keycode == 38 || keycode == 40) && !event.ctrlKey && !event.shiftKey && !event.altKey) {

        //up/down on select and textarea leave system to process it normally
        if (element.tagName == 'TEXTAREA' && getvalue(element) && !onkeydown_allisselected(element))
            return true
    }

    function onkeydown_allisselected(element) {

        var text = getvalue(element)

        //empty field is considered as all selected
        if (!text)
            return true

        //standard and ie9+ (not spans)
        //.selectionStart throws and error on ff/chrome/standard on radio/checkbox etc
        //try {var selectionStart=element.selectionStart} catch (e) {var selectionStart=false}
        //if (selectionStart) {
        if (typeof element.selectionStart == 'number') {
            if (element.selectionStart != 0 || element.selectionEnd != text.length)
                return false

            //msie (doesnt work on spans?)
        } else if (document.selection) {
            //var textrange = document.selection.createRange()
            //var textrange=document.forms[0].selection.createRange()
            //            alert(document.selection.createRange().text+' ... '+text)
            if (document.selection.createRange().text.replace(/[\r\n]/g, '') != text.replace(/[\r\n]/g, ''))
                return false

            //standard spans firefox/chrome
        } else if (window.getSelection) {
            //        alert('z')
            //anchorOffset: 3, focusNode: #text "sdvsdvsdv ", focusOffset: 7, isCollapsed: false
            var selection = window.getSelection()

            //            if (selection.toString && selection.text.replace(/[\r\n]/g,'') == text.replace(/[\r\n]/g,'')) {
            if (selection.toString && selection.toString().replace(/[\r\n]/g, '') == text.replace(/[\r\n]/g, '')) {
                return true
            }

            //this doesnt seem to work
            //            alert(selection.isCollapsed + ' : ' + selection.getRangeAt(0).startOffset  + ' : ' +  selection.getRangeAt(0).endOffset + ' : ' + selection.rangeCount + ' : ' + text.length)
            if (selection.isCollapsed || selection.getRangeAt(0).startOffset != 0 || selection.getRangeAt(0).endOffset != text.length)
                return false

        }

        return true
    }

    //left arrow or right arrow and whole field is selected else return
    if ((keycode == 37 || keycode == 39) && !event.ctrlKey && !event.shiftKey && !event.altKey) {

        //handle left/right normally if not all text selected
        if (element.maxLength != 1 && element.tagName.match(gtexttagnames) && element.type != 'button' && element.type != 'checkbox') {
            if (!onkeydown_allisselected(element)) {
                console.log('exiting onkeydown normally because not all text is selected')
                return
            }

        }

        //left or right not in tables
        if (ggroupno == 0 && !event.ctrlKey && !event.shiftKey && !event.altKey) {
            if (keycode == 37)
                focusprevious()
            else
                focusnext()
            return exoduscancelevent(event)
        }

    }

    //up down in multiline text treated normally - up and down in the text
    if ((keycode == 38 || keycode == 40) && !event.ctrlKey && !event.shiftKey && !event.altKey) {
        if (!onkeydown_allisselected(element) && getvalue(element).indexOf('\n') >= 0)
            return
    }

    //up down keys outside a table are like left/right
    //if((keycode==38||keycode == 40)&&ggroupno==0&&element.tagName!='SELECT'&&element.type!='radio')
    if ((keycode == 38 || keycode == 40) && ggroupno == 0 && !event.ctrlKey && !event.shiftKey && !event.altKey) {

        //option to revert to windows standard (eg to show rapid effect of different values)
        if (element.tagName == 'SELECT' && element.getAttribute('exodusallowcursor'))
            return true

        if (keycode == 38)
            focusprevious()
        else
            focusnext()

        //prevent the key from being processed as a cursor movement
        return exoduscancelevent(event)

    }

    //left arrow and right arrow in tables, except on SELECT since they change it
    if (ggroupno != 0 && rown >= 0 && (keycode == 37 || keycode == 39) && !event.ctrlKey && !event.shiftKey && !event.altKey && element.tagName != 'SELECT') {
        //if not on the sole input element in the row (isfirst and islast)
        //if (!element.getAttribute('exodusisfirstinputcolumn') || !element.getAttribute('exodusislastinputcolumn')) {

        //if (!element.getAttribute('exodusisfirstinputcolumn') || element.getAttribute('exodusscreenfn') != tablex.getAttribute('exoduslastinputcolscreenfn')) {
        var screenfn = element.getAttribute('exodusscreenfn')
        if (screenfn != tablex_firstinputcolscreenfn || screenfn != tablex_lastinputcolscreenfn) {

            var scope = grows[grecn].getElementsByTagName('*')
            if (keycode == 37)
                focusprevious(element, scope)
            else
                focusnext(element, scope)
        }
        return exoduscancelevent(event)
    }

    //PGUP/PGDN/UP/DOWN/LEFT/RIGHT
    if (keycode == 34 || keycode == 33 || keycode == 40 || keycode == 38 || keycode == 37 || keycode == 39) {

        if (ggroupno == 0)
            return

        if (rown < 0)
            return//must be in a thead row

        //if down arrow on last row
        if (keycode == 40 && grecn == (grows.length - 1)) {

            //not on first column, focus next column
            if (element.getAttribute('exodusscreenfn') != tablex_firstinputcolscreenfn) {
                focusnext(element)
                return exoduscancelevent(event)
            }

            //on first column, add a new row
            if (glocked || !gKeyNodes) {
                //exodusaddrow(ggroupno)
                yield* form_insertrow(event, true)
                //and continue to focus on it
            }
        }

        var nrows = grows.length
        var id = element.id
        if (!id)
            return//exodus like button with no id?

        var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
        var pagen = exodusint(grecn / pagesize)
        var npages = Math.ceil((rs.length) / pagesize)

        //pgdn or down arrow
        if (keycode == 34 || (keycode == 40 && !event.ctrlKey && !event.shiftKey && !event.altKey)) {

            //ctrl+pgdn sadly not supported since reserved by firefox to change tabs

            if (rown < nrows - 1) {
                if (keycode == 40) {
                    if (!grows[rown + 1].exodusfields[id]) {
                        systemerror('yield* document_onkeydown()', 'Cannot locate id ' + id + ' in rown: ' + rown + ' nrows:' + grows.length + ' tagname:' + grows[rown + 1].exodusfields[id])
                        return false
                    }
                    focuson(grows[rown + 1].exodusfields[id])
                }
                else {
                    var newrown = grecn + gpagenrows
                    if (newrown > (nrows - 1))
                        newrown = nrows - 1
                    focuson(grows[newrown].exodusfields[id])
                }
            }

            //if on last row go to first row of next page if any, otherwise stay put
            else {

                if (pagen < npages - 1) {
                    //tablex.nextPage()
                    focuson(grows[0].exodusfields[id])
                }
            }

        }

        //pgup or up arrow
        if (keycode == 33 || (keycode == 38 && !event.ctrlKey && !event.shiftKey && !event.altKey)) {

            //ctrl+pgup or up (sadly not since reserved by firefox to change tabs
            //goes to first line of first page
            if (event.ctrlKey) {
                //tablex.firstPage()
                focuson(grows[0].exodusfields[id])
            }
            else {

                //if not on first row of page go up one row (up arrow) or to first line (pgup)
                if (rown > 0) {
                    //up
                    if (keycode == 38) {
                        var newrown = rown - 1
                        while (newrown > 0 && !exodusenabledandvisible(grows[newrown])) {
                            newrown--
                        }
                        focuson(grows[newrown].exodusfields[id])
                    }
                    //page up
                    else {
                        var newrown = grecn - gpagenrows
                        if (newrown < 0)
                            newrown = 0
                        //focuson(grows[newrown].exodusfields[id])
                        if (document.getElementsByClassName)
                            focuson(grows[newrown].exodusfields[id])
                        else
                            focuson(grows[newrown].all[id])
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
                        focuson(grows[nrows - 1].exodusfields[id])
                    }
                }
            }
        }

        //allow various control and shift key combinations
        if (!event.ctrlKey && !event.shiftKey && !event.altKey)
            return exoduscancelevent(event)

    }

    //following are all keycodes when within tabular rows section
    //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    if (event.altKey && event.ctrlKey && keycode == 18)
        return true

    //Ctrl+I or Ctrl+Insert is insert row, but only in rows.
    //with or without shift
    if (event.ctrlKey && (keycode == 73 || keycode == 45) && rowx) {
        var button = rowx.exodusfields['insertrowbutton' + ggroupno]
        if (!event.repeat && button && button.style && button.style.display != 'none') {
            exoduscancelevent(event)
            yield* form_insertrow(event)
        }
        return false
    }

    //Ctrl+D or Ctrl+Delete is delete row, but only in rows.
    //with or without shift
    if (event.ctrlKey && (keycode == 68 || keycode == 46) && rowx) {
        var button = rowx.exodusfields['deleterowbutton' + ggroupno]
        if (!event.repeat && button && button.style && button.style.display != 'none') {
            exoduscancelevent(event)
            yield* form_deleterow(event, event.target)
        }
        return false
    }

    //protect readonly fields
    if (!(event.altKey || event.ctrlKey)
        && (element.getAttribute('exodusreadonly')
            || (gKeyNodes
                && !glocked
                && element
                && element.getAttribute('exodusfieldno')
                && element.getAttribute('exodusfieldno') != 0
            ))) {
        if (![9, 16, 17, 18, 20, 35, 36, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123].exoduslocate(keycode)) {
            exoduscancelevent(event)
            if (gKeyNodes && !glocked) {
                return yield* readonlydocmsg()
            }
            else if (element.getAttribute('exodusreadonly') != 'true') {
                yield* exodusinvalid(element.getAttribute('exodusreadonly'))
            }
            return false
        }
    }

}//document_onkeydown2

//return the first or last enabled and visible input exodusscreenfn given a table element
//or the precalculated ones if none
function form_getlastinputcolscreenfn(tablex) {
    return form_getfirstinputcolscreenfn(tablex, true)
}
function form_getfirstinputcolscreenfn(tablex, last) {
    var groupno = tablex.getAttribute('exodusgroupno')
    var sfns = gtables[groupno]
    var sfn
    for (var ii = 0; ii < sfns.length; ++ii) {
        var element = gfields[sfns[ii]]

        //skip non-input elements
        if (!element.getAttribute('exodusfieldno') || element.getAttribute('exodusreadonly') || !element.tagName.match(gdatatagnames))
            continue

        //capture only enabled and visible elements
        if (exodusenabledandvisible(element)) {
            sfn = sfns[ii]
            //if first then skip out as soon as found one
            if (!last)
                return sfn
        }
    }

    //if last then skip out with last found sfn
    if (sfn)
        return sfn

    //otherwise return the precalculated defaults
    return tablex.getAttribute(last ? 'exoduslastinputcolscreenfn' : 'exodusfirstinputcolscreenfn')
}

//checks and blocks if document is readonly
function document_onchange_sync(event) {
    if (ockeditor)
        ockeditor.setReadOnly(gkeys.length && !glocked)
}

function focusprevious(element, scope) {
    focusdirection(-1, element, '', scope)
}

function focusnext(element, scope) {
    focusdirection(1, element, '', scope)
}

function focusdirection(direction, element, notgroupno, scopex) {
    //currently required sourceIndex which is msie only
    //if (!document.body.sourceIndex)
    //    return

    if (typeof notgroupno == 'undefined')
        notgroupno = ''

    //find next input field
    var nextelement = element
    if (!nextelement) {
        //if (!window.event) return
        if (gevent)
            nextelement = gevent.target
        if (!nextelement)
            return
    }
    // if (!element) return
    var elementtabindex = nextelement ? nextelement.tabIndex : 999999999999

    var origgroupno = nextelement.getAttribute('exodusgroupno')

    var scope = scopex
    if (!scope)
        scope = document.all
    if (!scope)
        scope = document.getElementsByTagName('*')

    //work out index into scope of elements (all or one row usually)
    //sourceIndex of document.all[0].sourceIndex is 0 in MSIE6 and 1 in later versions
    var scopeindex
    //if (typeof nextelement.sourceIndex != 'undefined') {
    //    var sourceindex = nextelement.sourceIndex
    //    scopeindex = sourceindex - scope[0].sourceIndex
    //} else
    {
        //find nextelement in scope
        if (scope.indexOf) {
            //probably never occurs since scope is a collection not an array
            scopeindex = scope.indexOf(nextelement)
        } else {
            for (scopeindex = 0; scopeindex <= scope.length; ++scopeindex) {
                if (scope[scopeindex] == nextelement)
                    break;
            }
        }
        //fail if cannot locate nextelement (eg element is no longer linked into document due to  something like deleteallrows()?)
        if (scopeindex >= 0 && scope[scopeindex] != nextelement) {
            //focuson(nextelement)
            return
        }
    }

    //fail if cannot index into scope
    if (scope[scopeindex] != nextelement) {
        //wrongscopeindex()
        //focuson(nextelement)
        return
    }

    //test to prevent endless loop
    var passzero = scopeindex

    while (true) {
        var scopeindex = scopeindex + direction

        //prevent infinite loop
        if (passzero == scopeindex) {
            //break
            return
        }

        //wrap/scroll to the bottom
        if (scopeindex < 0) {
            if (typeof scopex == 'undefined')
                window.scrollBy(0, 999999)
            scopeindex = scope.length - 1
        }

        //wrap/scroll to the top
        else if (scopeindex > scope.length - 1) {
            if (typeof scopex == 'undefined')
                window.scrollBy(0, -999999)
            scopeindex = 0
        }

        //get the next possible element by scopeindex
        nextelement = scope[scopeindex]
        var nextid = nextelement.id

        //scroll into view
        if (typeof (notgroupno) == 'undefined') {
            scrollintoview(nextelement)
        }

        //skip uninteresting tags with no id or non-data entry tag
        if (!nextelement.id || !nextelement.tagName.match(gdatatagnames)) {
            //console.log('SKIP '+nextid+' '+nextelement.tagName+' skipped')
            continue
        }

        //skip uneditable elements
        if (nextelement.tagName != 'INPUT' && !nextelement.isContentEditable && nextelement.tagName != 'SELECT' && nextelement.tagName != 'TEXTAREA') {
            //console.log('SKIP '+nextid+' is not contentEditable')
            continue
        }

        //skip undesired group
        if (notgroupno !== '' && Number(nextelement.getAttribute('exodusgroupno')) == notgroupno) {
            //console.log('SKIP '+nextid+' is group '+notgroupno)
            continue
        }

        //skip readonly
        if (nextelement.getAttribute('exodusreadonly')) {
            //console.log('SKIP '+nextid+' is readonly')
            continue
        }

        //skip disabled
        if (nextelement.disabled) {
            //console.log('SKIP '+nextid+' is disabled')
            continue
        }

        //skip invisible
        if (!nextelement.offsetWidth) {
            //console.log('SKIP '+nextid+' no offsetWidth (not displayed)')
            continue
        }

        //skip tabindex -1
        if (nextelement.tabIndex == -1) {
            //console.log('SKIP '+nextid+' tabindex '+nextelement.tabIndex)
            continue
        }

        //skip lower tabindex if forward direction
        if (direction > 0 && elementtabindex && nextelement.tabIndex < elementtabindex) {
            //console.log('SKIP '+nextid+' tabindex '+nextelement.tabIndex+' < '+elementtabindex)
            continue
        }

        //skip higher tabindex if backward direction
        if (direction < 0 && elementtabindex && nextelement.tabIndex > elementtabindex) {
            //console.log('SKIP '+nextid+' tabindex '+nextelement.tabIndex+' > '+elementtabindex)
            continue
        }

        //(isMSIE && nextelement.currentStyle && nextelement.currentStyle.display == 'none' && nextelement.parentNode.currentStyle.display == 'none')
        //should also text mozilla currentstyle
        //(yield* getcurrentstyle(nextelement) && yield* getcurrentstyle(nextelement).display == 'none' && yield* getcurrentstyle(nextelement.parentNode).display == 'none')
        if (isMSIE && !exodusenabledandvisible(nextelement)) {
            //console.log('SKIP '+nextid+' isMSIE and not enabledandvisible')
            continue
        }

        //left/right/up/down keys skip over SELECT or radio buttons because they cannot be prevented from changing its value if pressed again
        if ((gkeycode == 37 || gkeycode == 39 || gkeycode == 38 || gkeycode == 40) && (nextelement.tagName == 'SELECT' || nextelement.type == 'radio')) {
            //console.log('SKIP '+nextid+' cursor keys skip over SELECT or radio items')
            continue
        }

        //enter key skips over buttons to avoid pressing them on the next press
        if (gkeycode == 13 && nextelement.type == 'button') {
            //console.log('SKIP '+nextid+' Enter key skips over buttons')
            continue
        }

        if (nextelement == element)
            continue

        break
    }

    //if arrived on last column and row of a table using up then focus on the first column last row
    if (direction < 0) {
        var nextgroupno = nextelement.getAttribute('exodusgroupno')
        if (nextgroupno != origgroupno && nextgroupno != 0) {
            if (gtables[nextgroupno]) {
                var tablex = gtables[nextgroupno].tablex
                if (tablex) {
                    var tablex_lastinputcolscreenfn = form_getlastinputcolscreenfn(tablex)
                    if (nextelement.getAttribute('exodusscreenfn') == tablex_lastinputcolscreenfn) {
                        var tablex_firstinputcolscreenfn = form_getfirstinputcolscreenfn(tablex)
                        var firstinputcolid = gfields[tablex_firstinputcolscreenfn].id
                        var row = tablex.tBodies[0].getElementsByTagName('tr')[getrecn(nextelement)]
                        setupnewrow(row)//create .exodusfields
                        var firstcolelement = row.exodusfields[firstinputcolid]
                        if (firstcolelement)
                            nextelement = firstcolelement
                    }
                }
            }
        }
    }

    //found it. focus on it
    console.log('focusdirection ' + nextelement.tagName + ' ' + nextelement.id)
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
function* newrecord_onclick() {

    //save and close existing document otherwise cancel
    if (gkey && !(yield* closedoc('NEW')))
        return false

    //suppress defaulting if is a fixed default (eg current market in market file)
    //only look at single part keys
    if (gKeyNodes.length == 1) {
        var keyelement = gKeyNodes[0]
        var defaultvalue = keyelement.getAttribute('exodusdefaultvalue')
        //if no default and key field not required, that means that key will be generated by server on request
        //if default and default starts with " character then considered to be a fixed default and not required if user clicks "New" button
        if (
            (!defaultvalue && keyelement.getAttribute('exodusrequired'))
            ||
            (defaultvalue && defaultvalue.slice(0, 1) == '"')
        ) {
            //if already defaulted with fixed string then remove default value and prevent defaulting
            //if they enter something then click New it will be like they clicked Tab to the first non-key element
            var keyid = keyelement.id
            var currentvalue = yield* gds.getx(keyid)
            if (currentvalue && currentvalue == eval(defaultvalue)) {
                yield* gds.setx(keyid, '', '')
                gds.setdefaulted(keyid, null, true)
            }
            //focus on the key element
            //exodussettimeout('focuson("' + gKeyNodes[0].id + '")', 200)
            //return true
            //skip the above to allow focussing on first non-key element because prompts them with a message
        }
    }

    // element.focus()
    var tt = gfirstnonkeyelement
    if (tt.id)
        tt = tt.id

    //if new clicked while on gfirstnonkeyelement ensure that focussing on it triggers read record
    //gpreviouselement=null
    if (gstartelement && gstartelement.id)
        exodussettimeout('focuson("' + gstartelement.id + '")', 100)

    exodussettimeout('focuson("' + tt + '")', 200)

    //yield* newrecordfocus()
    return true

}

//RELEASE BUTTON
////////////////
function* editreleaserecord_onclick() {

    //not locked and there is a key
    if (!glocked) {
        yield* opendoc(gkey)
        return true
    }

    //if (!(yield* saveandunlockdoc())) return false
    if (!(yield* saveandorcleardoc('RELEASE')))
        return false

    //editreleaserecord.value='Edit'
    setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
    setdisabledandhidden(editreleaserecord, false)

    return true

}

//SAVE BUTTON
/////////////
function* saverecord_onclick() {

    //first update the current field
    //otherwise the db is updated without the last entry!!!
    if (!(yield* validateupdate()))
        return false

    //check for missing required
    if (!(yield* validateall()))
        return false

    // saverecord.focus()

    //if databound form then save it
    if (gKeyNodes) {

        //save the record
        if (!(yield* savedoc()))
            return false

        //prune the cache to reselect any edited record
        prunecache('SELECT\r' + gdatafilename + '\r')
        prunecache('READ\r' + gdatafilename + '\r' + gkey)
        prunecache('GETINDEXVALUES\r' + gdatafilename + '\r')

        //custom postwrite function
        if (typeof form_postwrite == 'function') {
            //yield* exodusevaluateall('yield* form_postwrite()')
            if (!(yield* exodusevaluateall('yield* form_postwrite()')))
                return false
        }
        //otherwise automatic option to print if available
        else {
            //TODO put this back as a form level opt in
            //   if (printsendrecord&&!printsendrecord.getAttribute('disabled')&&(yield* exodusyesno('Print/Send '+gkeyexternal+' ?',2))) yield* printsendrecord_onclick()
        }

        return true

    }

    //from here on is an unbound form and not a databound form
    //////////////////////////////////////////////////////////

    //custom prewrite function
    if (typeof (form_prewrite) == 'function')
        if (!(yield* exodusevaluateall('yield* form_prewrite()', 'yield* saverecord_onclick()')))
            return false

    //option to confirm
    if (gparameters.confirm || gparameters.savemode && gparameters.savemode.indexOf('CONFIRM') >= 0) {
        if (!(yield* exodusyesno('OK to continue?', 2)))
            return false
    }

    ///convert screen data into a rev string
    gro.data = gds.data
    if (!(gro.obj2rev()))
        return false

    //alert(gro.revstr.split(fm))

    //custom write and postwrite routine
    if (typeof form_write == 'function') {

        if (!(yield* exodusevaluateall('yield* form_write()', 'yield* saverecord_onclick()')))
            return false
        setchangesmade(false)

        if (typeof (form_postwrite) == 'function')
            yield* exodusevaluateall('yield* form_postwrite(db)', 'yield* saverecord_onclick()')

        return true
    }

    //option to return the form data to a calling program
    //instead of sending to the database
    if (window.dialogArguments && (!gparameters.savemode || !gparameters.savemode.exodusswap('CONFIRM', ''))) {

        //window.returnValue = gro.revstr
        //window.close()
        //return false
        return exoduswindowclose(gro.revstr)
    }

    //standard unbound write routine
    if (!(yield* unbound_form_write()))
        return false

    //if postwrite routine
    if (typeof (form_postwrite) == 'function')
        yield* exodusevaluateall('yield* form_postwrite(db)', 'yield* saverecord_onclick()')

    //otherwise, if no postwrite function then assume that
    //returned data is a url and open it (in a new tab by preference or a new window if cannot)
    else if (db.data) {
        //dont open in tab if dialog window and not closing
        //because user cant focus on underlying window tabs while in (pseudo) modal dialog window
        if (gisdialog && !closeafter)
            yield* windowopen(db.data)//new window
        else
            yield* windowopen(db.data, '', 'tab')//tab
    }

    //option to close window
    var closeafter = gparameters.savemode && gparameters.savemode.indexOf('CLOSE') >= 0
    if (closeafter) {
        //window.returnValue=true
        //window.close()
        exoduswindowclose(true)
    }

    return true

}

function* unbound_form_write() {

    //send the instructions for processing and open the report
    db.request = 'EXECUTE\r' + gmodule + '\r' + gdatafilename
    if (!(yield* db.send(gro.revstr))) {
        yield* exodusinvalid(db.response)
        return false
    }
    setchangesmade(false)
    if (db.response.slice(0, 3) == 'OK ') {
        yield* exodusnote(db.response.slice(3))
    }

    return true

}

//CLOSE BUTTON
//////////////
function* closerecord_onclick() {

    //cancel on dialoged window closes window returns false
    if (window.dialogArguments || !gKeyNodes && gparameters.savemode && gparameters.savemode.indexOf('CONFIRM') >= 0)
    //if (window.dialogArguments)
    {

        //user confirms or cancels
        if (gchangesmade) {
            var response = yield* exodusconfirm('Discard data or instructions entered ?', 1, '', 'D<u>i</u>scard', '<u>C</u>ancel')
            if (response != 2) return false
        }

        var returnvalue = ''

        //clear the document so user is not asked again in onunload
        if (gKeyNodes) {
            gwindowunloading = true
            if (!gchangesmade)
                returnvalue = 'ACCESS ' + gkey
            yield* cleardoc()
        }

        if (window.dialogArguments) {

            //close the window
            //window.returnValue = returnvalue
            //return window.close() && false
            return exoduswindowclose(returnvalue)
        }

    }

    return yield* closedoc('CLOSE')

}

//DELETE BUTTON
///////////////
function* deleterecord_onclick(event) {

    event = getevent(event)

    if (!(yield* deletedoc()))
        return exoduscancelevent(event)

    //prune the cache to reselect any deleted record
    prunecache('SELECT\r' + gdatafilename + '\r')
    prunecache('GETINDEXVALUES\r' + gdatafilename + '\r')

    //custom postdelete function
    if (typeof form_postdelete == 'function') {
        //yield* exodusevaluateall('form_postdelete()')
        if (!(yield* exodusevaluateall('form_postdelete()')))
            return false
    }

    return true

}

/*
//CHANGE PAGE BUTTONS
/////////////////////
function* changepage(pagen) {
    if (gKeyNodes && closerecord.getAttribute('disabled')) return false

    var tablex = getancestor(window.event.target, 'TABLE')
    //if (tablex!=null&&typeof(tablex.getAttribute('exodusgroupno'))!='undefined')
    if (tablex != null && Number(tablex.getAttribute('exodusgroupno'))) {
        switch (pagen) {
            case 0: { tablex.firstPage(); break }
            case -1: { tablex.previousPage(); break }
            case 1: { tablex.nextPage(); break }
            default: { tablex.lastPage(); break }
        }
    }

}*/

/////////////////////// DOCUMENT FUNCTIONS //////////////////


//'''''''''
//'OPENDOC
//'''''''''

var gopening = false

function* opendoc(newkey) {

    //login('opendoc')
    var result = yield* opendoc_body(newkey)
    //logout('opendoc '+result)

    return result
}

function* opendoc_body(newkey) {
    //yield* debug('opendoc:'+newkey)
    //how to avoid double calls from multiple timeouts
    if (newkey == gkey && glocked)
        return true

    if (newkey == '%NEW%') newkey = ''

    //check potential key has as many parts as there are key fields
    if (newkey && gKeyNodes.length > 1) {
        if (typeof newkey == 'string')
            newkey1 = newkey
        else
            newkey1 = newkey[0]
        var sepchar = gKeyNodes[0].exoduskeypart ? '*' : gKeyNodes[0].exoduswordsep
        if (sepchar && newkey1.split(sepchar).length != gKeyNodes.length) {
            systemerror('opendoc', newkey1.exodusquote() + ' does not have the correct number of key parts (' + gKeyNodes.length + ')')
            return false
        }
    }

    //gopening = true
    //login('pre opendoc2')
    var opened = yield* opendoc2(newkey)
    //logout('pre opendoc2 opened='+opened)
    gopening = false

    //add key into gkeys
    if (opened && gKeyNodes) {
        if (!gkeys.exoduslocate(gkey)) {
            gkeys = gkeys.exodusinsert(gkeyn + 1 + 1, gkey)
            yield* setgkeys(gkeys)
            if (gkeys.length > 1)
                gkeyn++
        }
    }

    //focus on key (last if more than one) element if opendoc2 fails
    if (!opened) {
        exodussettimeout('try{gKeyNodes[gKeyNodes.length-1].focus()}catch(e){}', 10)
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

function* opendoc2(newkey0) {
    //yield* debug('opendoc2:'+newkey0)

    //login('opendoc2')

    //detect if a new key is present and quit if not
    var newkey = ''
    if (newkey0) {
        newkey = newkey0
    }
    else {
        if (gKeyNodes) {
            newkey = yield* getkey()
            if (newkey == gkey && gloaded) {
                //newkey=''
                return false //logout('opendoc2 - no new key')
            }
        }
        if (!newkey) {
            //qqq allow read without key to return next sequential key
            //   return false //logout('opendoc2 - no new key')
        }
    }

    //unload previous doc first
    ///////////////////////////
    //have to clear even if not loaded othewise setkeyvalues fails because gds.data not yet available when key=xxx in the url
    if (!(yield* closedoc('OPEN'))) {
        //qqq should try and restore the previous key data?
        return false //logout('opendoc2 - did not close currently open doc')
    }

    yield* setkeyvalues(newkey)//closedoc zaps them.
    gkey = newkey
    gkeyexternal = yield* getkeyexternal()

    //optional preread function (returns false to abort)
    if (typeof (form_preread) == 'function') {

        ///log('preread external key=' + gkeyexternal + ' internalkey=' + gkey)
        if (!(yield* exodusevaluateall('yield* form_preread()', 'yield* opendoc2()')))
            return false //logout('opendoc2 - preread false')

    }

    wstatus('Requesting data, please wait ...')

    //read record (with or without a lock)
    /////////////

    //gkey can be blank in which case a new key will be
    //returned in gro.key

    gro.key = gkey
    if (gkey || !gupdateonlymode)
    //read with key='' means get the next sequential number
        /**/ yield* gro.read(!greadonlymode && !gparameters.openreadonly)

    //switch off one-time option
    gparameters.openreadonly = false

    yield* loadnewkey()

    glocked = gro.sessionid != ''
    //ensure lock is automatically re-locked every x minutes while record is active on-screen
    if (glocked)
        startrelocker()

    //fail if no data (failure to reach backend database)
    //listen always provides data even if only an empty dataset (eg new record)

    if (!gro.data) {
        if (glocked)
            yield* unlockdoc()//fail safe

        //logout('opendoc2 - no data')
        return yield* exodusinvalid(gro.response)
    }

    var lockholder = getlockholder(db.response)

    if (db.response.toUpperCase().slice(0, 16) == 'ERROR: NO RECORD') {
        if (gupdateonlymode) {
            //yield* exodusinvalid(exodusquote(gkeyexternal)+' does not exist.')
            yield* exodusinvalid(exodusquote(yield* getkeyexternal()) + ' does not exist.')
            //('opendoc2 - cannot create new record because gupdateonlymode is true')
            return false
        }

        if (!glocked) {

            //cannot create records if cannot lock them
            if (db.response.toUpperCase().indexOf('LOCK NOT AUTHORISED') >= 0) {
                yield* exodusinvalid('Sorry, you are not authorised to create new records in this file.')
                return false //logout('opendoc2 - cannot create new record because lock not authorised')
            }

            //may not be authorised to read a record
            if (db.response.toUpperCase().indexOf('NOT AUTHORISED') >= 0) {
                yield* exodusinvalid(db.response)
                return false //logout('opendoc2 - cannot access record because not authorised')
            }

            //cannot create a record in read only mode or update only mode
            if (greadonlymode || gupdateonlymode || gpreventcreation) {
                //yield* exodusinvalid('Sorry, '+gkeyexternal+' does not exist and\nyou are not authorised to create new records in this file.')
                yield* exodusinvalid('Sorry, ' + (yield* getkeyexternal()) + ' does not exist and\nyou are not authorised to create new records in this file.')
                return false //logout('opendoc2 - cannot create new record in read only mode')
            }

            //cannot create a record without a lock
            //if possible get another default key
            if (gKeyNodes.length == 1 && gKeyNodes[0].getAttribute('exodusdefaultvalue')) {
                var newkey = yield* getdefault(gKeyNodes[0])
                if (newkey && newkey != gkey) {
                    //logout('opendoc2 - cannot create new record because ' + lockholder + ' is creating it.')
                    return yield* opendoc2(newkey)
                }
            }

            yield* exodusinvalid(exodusquote(gkeyexternal) + ' is being created by ' + lockholder + '.\r\n\r\nYou cannot view or update it until they have finished or cancel.')
            //logout('opendoc2 - cannot create new record because ' + lockholder + ' is creating it.')
            return false
        }

        /*
        //optionally cancel if (record does not exist
        if (!(yield* exodusokcancel('Document '+exodusquote(getkeyexternal)+' does not exist. Create a new document ?',2))) {

        if (glocked)
        yield* unlockdoc()
        exoduscancelevent(event)
        return false //logout('opendoc - user chose not to create new record')
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
                        if ((yield* exodusconfirm(exodusquote(gkeyexternal) + ' is being updated by ' + lockholder + '.\nOpen for viewing only?', 1, 'Yes', '', 'Cancel')) != 1) {
                            return false //logout('opendoc2 - because it is being updated by ' + lockholder + ' and the user chose not to open it in read only mode')
                        }
                        //editreleaserecord.value='Edit'
                        setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
                        setdisabledandhidden(editreleaserecord, false)
                    }
                    else {
                        if (db.response.toString().slice(0, 2) != 'OK')
                            return yield* exodusinvalid(db.response)
                        else
                            yield* exodusnote(db.response.toString().slice(2))
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
    //form_postread should set setchangesmade(true) to allow save without user edits
    setchangesmade(false)
    //gpreviouselement = null
    //gpreviousvalue = ''
    setgpreviouselement(null)

    //postread
    if (typeof form_postread == 'function') {
        grecn = null
        if (!(yield* exodusevaluateall('yield* form_postread()', 'yield* opendoc2()')))
        //if (!(yield* form_postread()))
        {
            if (glocked)
                yield* unlockdoc()//fail safe
            yield* cleardoc()
            return false //logout('opendoc2 - postread failed')
        }

    }

    //load the record
    if (gds.isnewrecord)
        wstatus('New record')
    else
        wstatus('Formatting, please wait ...')
    gloaded = true
    yield* gds.load(gro)

    if (!gds.isnewrecord)
        wstatus('Existing record')

    document.title = gdoctitle + ' ' + gkeyexternal.exodusconvert('*', ' ')

    //enable the delete and save buttons if locked
    //according to need
    if (glocked) {
        setdisabledandhidden(saverecord, false)
        setdisabledandhidden(editreleaserecord, false)
        //editreleaserecord.value='Release'
        setgraphicbutton(editreleaserecord, 'R<u>e</u>lease', greleaseimage)
        if (deleterecord && !gds.isnewrecord && !greadonlymode && !gupdateonlymode && !gpreventdeletion)
            setdisabledandhidden(deleterecord, false)
    }
    else if (!greadonlymode) {
        setdisabledandhidden(editreleaserecord, false)
        setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
    }

    //set editor textarea readonly or not
    //gCKEDITOR_EDITOR may not be ready yet when first opening the page
    //so we also do the same in its instanceReady event.
    //MSIE has a problem with this, "permission denied" - so skip on error
    try {
        if (typeof gCKEDITOR_EDITOR != 'undefined')
            gCKEDITOR_EDITOR.setReadOnly(!glocked)
    } catch (e) { }

    //enable the close and print buttons
    setdisabledandhidden(closerecord, false)
    if (printsendrecord)
        setdisabledandhidden(printsendrecord, false)

    //hide the enter button
    //setdisabledandhidden(enter,true)

    //postdisplay (in opendoc2 and )
    //TODO convert all postpostread( to use this new hook function instead of timeout
    if (typeof form_postdisplay == 'function') {
        grecn = null
        if (!(yield* exodusevaluateall('yield* form_postdisplay()', 'yield* opendoc2()'))) {
            if (glocked)
                yield* unlockdoc()//fail safe
            yield* cleardoc()
            return false //logout('opendoc2 - postdisplay failed')
        }

    }

    //logout('opendoc2')

    return true

}

function* loadnewkey() {

    //load the new key
    if (gkey != gro.key) {
        yield* setkeyvalues(gro.key)
        gkey = yield* getkey()
        //form key  x*y*z* could be a little different than the db key x*y*z
        gkeyexternal = yield* getkeyexternal()
    }

}

///////////
//CLOSE DOC
///////////
function* closedoc(mode) {

    //WINDOWUNLOAD,CLOSE,NEW,OPEN
    if (!mode || mode.type)
        mode = 'CLOSE'

    //first update the current field
    //otherwise the db is updated without the last entry!!!
    //if (save&&!(yield* validateupdate()))
    // return false
    //should not do validateupdate since maybe called from validateupdate to close/ timeout to open a new record

    //return yield* saveandorcleardoc(mode,save,clear=mode!='OPEN')
    //return yield* saveandorcleardoc(mode,save=mode!='CLEAR',clear=mode!='OPEN')
    var result = yield* saveandorcleardoc(mode)

    /*/msie needs to result null otherwise prompts to leave or stay in window
    //this is how we tell that we are in msie and onbeforeunload event
    if (isMSIE&&window.event&&window.event.type=='beforeunload') {
        if (result)
           return
        else
            return 'Warning: You will lose any unsaved data'
    }*/

    return result
}

//makedoc readonly
function* saveandunlockdoc() {
    if (!(yield* validateupdate()))
        return false
    if (gchangesmade && !(yield* savedoc()))
        return false
    if (!(yield* unlockdoc()))
        return false
    return true
}

//'''''''''''
//'SAVE DOC
//'''''''''''
var gform_in_savedoc = false
function* savedoc(mode) {

    //prevent reentry in async environment
    if (gform_in_savedoc)
        return false

    gform_in_savedoc = true
    var result = yield* savedoc_body(mode)
    gform_in_savedoc = false

    return result
}

function* savedoc_body(mode) {

    //called from yield* saverecord_onclick()
    if (!mode)
        mode = 'SAVE'

    //first update the current field
    //otherwise the db is updated without the last entry!!!
    if (!(yield* validateupdate()))
        return false

    if (gKeyNodes && (!gchangesmade || !glocked) && !gallowsavewithoutchanges) {
        yield* exodusinvalid('Nothing to be saved.\n\nPlease enter or change some data first or just click Close')
        return false
    }

    //return yield* saveandorcleardoc(mode,save=true,clear=false)
    return yield* saveandorcleardoc(mode)

}

//'''''''''''
//'SAVE DOC
//'''''''''''
var gform_in_saveandorcleardoc = false
function* saveandorcleardoc(mode) {

    //prevent reentry in async environment
    if (gform_in_saveandorcleardoc)
        return false

    gform_in_saveandorcleardoc = true

    var result = yield* saveandorcleardoc_body(mode)

    gform_in_saveandorcleardoc = false

    return result
}

function* saveandorcleardoc_body(mode) {

    // yield* debug('saveandorcleardoc:'+mode)

    //mode CLOSE, SAVE, CLEAR, CHANGEKEY, PRINT
    var save = (mode != 'CLOSE')
    //var clear=(mode!='SAVE'&&mode!='OPEN'&&mode!='CLOSE')
    var clear = (mode == 'CLOSE' || mode == 'NEW' || mode == 'CLEAR')
    var unlock = (mode != 'SAVE' && mode != 'PRINT' && mode != 'LIST' && mode != 'NEW')
    //yield* debug('saveandorclear '+save+' '+clear+' '+unlock)
    //called from
    //should not do validateupdate since maybe called from validateupdate to close/ timeout to open a new record

    //function* savedoc(mode)
    //('SAVE',true,false)
    //mode can be SAVE

    //function* printsendrecord_onclick()
    //('PRINT',true,false)

    //function* closedoc(mode)

    //(mode,save,true)
    //mode can be WINDOWUNLOAD,CLOSE,NEW,OPEN

    //login('saveandorcleardoc ' + save + ' ' + clear)

    //first update the current field
    //otherwise the db is updated without the last entry!!!
    //if (save&&!(yield* validateupdate()))
    // return false //logout('saveandorcleardoc - invalidateupdate failed')

    //if anything updated then option to save
    if (glocked
        && (gchangesmade
            || (gallowsavewithoutchanges
                && !clear && mode != 'OPEN' && mode != 'RELEASE'
            )
            || (gpreviouselement
                && gpreviouselement.getAttribute('exodusfieldno') > 0
                && getvalue(gpreviouselement) != gpreviousvalue
            )
        )
    ) {

        //check last data entry is valid unless closing in which case discard will not validate
        if (mode != 'CHANGEKEY' && mode != 'CLOSE' && !(yield* validateupdate())) {
            focusongpreviouselement()
            return false //logout('saveandorcleardoc - invalid input')
        }

        //opportunity to save or cancel
        if (typeof form_prewrite != 'undefined' && mode == 'SAVE') {

            //confirm specific save
            if (gparameters.savemode == 'CONFIRM') {
                if (!(yield* exodusokcancel('OK to save ' + exodusquote(gkeyexternal.exodusconvert('*', ' ')) + ' ?', 1))) {
                    //return false
                    return false //logout('saveandorcleardoc - user cancelled')
                }
            }

            var response = 1
        }
        else {

            //save or discard?
            var savetitle = ''
            var discardtitle = ''
            var action = ''
            if (true || save)
                savetitle = '<u>S</u>ave'//Save
            action = savetitle
            if (true) {
                //discardtitle = 'Do<u>n</u>\'t Save'
                discardtitle = 'D<u>i</u>scard'//Discard
                if (true || save)
                    action += ' or '
                action += discardtitle
            }
            var canceltitle = '<u>C</u>ancel'//Cancel
            var response = yield* exodusconfirm(action + ' ' + exodusquote(gkeyexternal.exodusconvert('*', ' ')) + ' ?', 1, savetitle, discardtitle, canceltitle)
        }

        //user cancels
        if (response == 0) {
            focusongpreviouselement()
            return false //logout('saveandorcleardoc - user cancelled')
        }

        //prepare to close modal window
        window.returnValue = ''

        //user chooses to save
        if (response == 1) {

            //first update the current field
            //otherwise the db is updated without the last entry!!!
            //if (mode!='CHANGEKEY'&&!(yield* validateupdate()))
            if (mode == 'CLOSE' && !(yield* validateupdate())) {
                focusongpreviouselement()
                return false //logout('saveandorcleardoc - user cancelled')
            }

            var savekey = gkey
            var wasnewrecord = gds.isnewrecord

            //WRITEDOC
            if (!(yield* writedoc(unlock)))
                return false //logout('saveandorcleardoc - writedoc failed')

            //prepare to close modal window
            window.returnValue = (wasnewrecord ? 'NEW ' : 'UPDATE ') + savekey

        }

        //close window if modal also in deletedoc
        if (window.dialogArguments && (mode == 'SAVE' || mode == 'DELETE' || mode == 'CLOSE')) {
            gwindowunloading = true
            yield* cleardoc()

            //close the window
            //window.returnValue=''
            //return window.close() && false
            return exoduswindowclose(window.returnValue)

        }

        //enable deleterecord
        if (!greadonlymode && !gupdateonlymode && !gpreventdeletion)
            setdisabledandhidden(deleterecord, false)

    }

    //clear
    if (clear || response == 2) {
        yield* cleardoc()
    }

    //unlock
    else if (unlock && glocked) {
        if (!(yield* unlockdoc()))
            return false //logout('saveandorcleardoc - could not unlock')
    }

    //reset
    setchangesmade(false)

    //logout('saveandorcleardoc ok')

    return true

}

function* cleardoc() {
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

    //login('cleardoc')

    if (gKeyNodes && !(yield* unlockdoc()))
        return false //logout('cleardoc - unlockdoc failed')

    //disable the buttons
    setchangesmade(false)
    if (gKeyNodes) {
        setdisabledandhidden(saverecord, true)
        setdisabledandhidden(editreleaserecord, true)
        //editreleaserecord.value='Release'
        setgraphicbutton(editreleaserecord, 'R<u>e</u>lease', greleaseimage)
        if (deleterecord)
            setdisabledandhidden(deleterecord, true)
        setdisabledandhidden(closerecord, true)
        //if (printsendrecord)
        //    setdisabledandhidden(printsendrecord, true)
    }

    //set this before clear because .load will call yield* newrecordfocus()
    //gpreviouselement = null
    //gpreviousvalue = ''
    setgpreviouselement(null)

    //close it
    if (!gwindowunloading) {

        //gkey=''
        //gkeyexternal=''
        gloaded = false
        setchangesmade(false)
        document.title = gdoctitle

        //get an empty record and load it
        wstatus('Clearing, please wait ...')
        //  if (!glogging)
        //  {
        gro.key = ''
        gro.revstr = gro.defaultrevstr
        gro.rev2obj()
        yield* gds.load(gro)

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
                yield* insertallrows(element, paramvalue.split(vm))
            }

            //update single values
            else {
                yield* gds.setx(paramid, null, paramvalue)
            }

        }

        //  }
        wstatus('')

        yield* resetsortimages()

    }

    //if key is defaulted then clearing may have loaded a new record already
    if (!glocked && gkey) {
        glastkey = gkey
        gkey = ''
    }

    //not if clearing
    if (!gwindowunloading && gkey) {

        //setfocus on first non-defaulted key node
        var focussed = false
        if (gKeyNodes && gKeyNodes.length > 1) {
            for (var ii = 0; ii < gKeyNodes.length; ii++) {
                yield* setdefault(gKeyNodes[ii])
                if (!(getvalue(gKeyNodes[ii]))) {
                    focuson(gKeyNodes[ii])
                    focussed = true
                    break
                }
            }
        }

        if (!focussed) {
            yield* setdefault(gstartelement)
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

        setchangesmade(false)

    }

    //force any updates to be validated
    yield* validateupdate()

    //postdisplay in cleardoc and postinit
    //TODO convert all postpostread( to use this new hook function instead of timeout
    if (typeof form_postdisplay == 'function') {
        grecn = null
        yield* exodusevaluateall('yield* form_postdisplay()', 'yield* formfunctions_onload()')
    }

    //logout('cleardoc')

}

function* resetsortimages(groupno) {
    //reset the sort buttons
    if (!groupno) {
        for (groupno = 1; groupno < 99; groupno++) {
            yield* resetsortimages(groupno)
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

function* deletedoc() {

    //prevent delete if not locked
    if (!glocked)
        return yield* exodusinvalid(yield* readonlydocmsg())

    //prevent delete if new record
    if (gds.isnewrecord)
        return yield* exodusinvalid('You cannot delete this document because it hasnt been saved')

    //login('deletedoc')

    var question = exodusquote(gkeyexternal) + '\nWarning! Are you SURE that you want to delete this document?'
    if ((yield* exodusyesno(question, 2)) != 1) {
        yield* exodusinvalid('The document has NOT been deleted\nbecause you did not confirm.')
        return false //logout('deletedoc - user cancelled')
    }

    //stop relocking while trying to delete
    //not really necessary as DELETE is synchronous
    stoprelocker()
    var savekey = gkey

    //delete it
    db.request = 'DELETE\r' + gdatafilename + '\r' + gkey + '\r\r' + gro.sessionid
    if (!(yield* db.send())) {

        //yield* exodusnote(db.response)
        yield* exodusinvalid(db.response)

        //start the relocker again
        startrelocker()

        return false //logout('deletedoc - db returns false')

    }

    //any warnings are appended after response like 'OK xxx'
    if (db.response != 'OK' && typeof form_postdelete == 'undefined')
        yield* exoduswarning(db.response.slice(2))

    //deleting a record automatically unlocks it
    glocked = false

    yield* exoduswarning(exodusquote(gkeyexternal) + ' has been deleted.')

    //close window if modal also in writedoc
    if (window.dialogArguments) {
        gwindowunloading = true
        yield* cleardoc()

        //close the window
        //window.returnValue = 'DELETE ' + savekey
        //return window.close()
        return exoduswindowclose('DELETE ' + savekey)

    }

    yield* cleardoc()

    //logout('deletedoc')

    return true

}

function* form_oninput(event) {
    if (gchangesmade)
        return true
    event = getevent(event)

    //changing key fields does not cause gchangesmade
    var fn = Number(event.target.getAttribute('exodusfieldno'))
    if (!fn)
        return true

    //remember this element so pressing escape can cancel gchangesmade
    //removed in onfocus
    gelementthatjustcalledsetchangesmade = event.target
    setchangesmade(true)
    return true
}

function* form_onchangeselect(event) {

    event = getevent(event)

    if (!gpreviouselement) {
        //gpreviouselement = event.target
        //gpreviousvalue=null
        setgpreviouselement(event.target)
    }

    if (!(yield* validateupdate(event)))
        return exodusinvalid()

    //encourage changing key or key part in a SELECT to change record
    if (gpreviouselement.getAttribute('exodusfieldno') == '0')
        focusnext()

    return true
}

function* validateall(mode) {

    //login('validateall ' + mode)

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

        var tablex = gtables[groupno]
        var allowemptyrows = (groupno == 0) ? false : tablex.tableelement.getAttribute('exodusallowemptyrows')
        var rowrequired = (groupno == 0) ? false : tablex.tableelement.getAttribute('exodusrowrequired')
        var nrowsfound = 0

        for (var rown = 0; rown < rows.length; rown++) {
            var row = rows[rown]

            ////only validate rows if the first input column is entered
            ////if (groupno==0||row[firstcolumnname].element.text!='')
            //if (groupno==0||row[firstcolumnname].text!='')

            var anydata = false
            var missingelement = ''
            var propname
            for (propname in row) {
                var cell = row[propname]

                //skip unbound, calculated and not required cells
                var element = cell.element
                if (!element || element.getAttribute('exodustype') != 'F')
                    continue

                //skip cells with data
                if (cell.text && (typeof cell.text != 'string' || cell.text.replace(/ *$/, ''))) {

                    //SELECT elements
                    //if (element.tagName=='SELECT'&&element.options[0].selected)

                    anydata = true
                    continue
                }

                //only interested in required fields from here on
                if (!(element.getAttribute('exodusrequired')))
                    continue

                //skip cells that can be defaulted
                //do not default in rows to avoid problem of blank line and first/line line
                //filling in unwanted data (perhaps select tags also have similar problem)
                if (!groupno) {
                    gdefault = yield* getdefault(element)
                    if (gdefault != null && gdefault != '') {
                        cell.text = gdefault
                        anydata = true
                        continue
                    }
                }

                //flag first element that is required, enabled and visible and missing
                if (mode != 'filldefaults'
                    && !missingelement
                    //setting .disabled to true seems to have the effect of
                    // setting attribute disabled to "" in modern browsers!
                    //&& !element.getAttribute('disabled')
                    && !element.disabled
                    && exodusenabledandvisible(element)) {
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
                yield* exodusinvalid(missingelement.getAttribute('exodustitle') + ' is required.')
                focuson(missingelement)
                return false //logout('validateall ' + mode)
            }

            //prevent blank rows (except last one) unless allowed
            if (mode != 'filldefaults'
                && !anydata
                && groupno != 0
                && rown < (rows.length - 1)) {
                if (!allowemptyrows) {
                    var missingelement = rows[0][firstcolumnname].element
                    yield* exodusinvalid('Empty rows are not allowed for ' + missingelement.getAttribute('exodustitle'))
                    focuson(missingelement)
                    return false //logout('validateall ' + mode + ' empty row')
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
            && exodusenabledandvisible($$('exodusgroup' + groupno))
        ) {
            var missingelement = rows[0][firstcolumnname].element
            yield* exodusinvalid('At least one ' + missingelement.getAttribute('exodustitle') + ' is required.')
            focuson(missingelement)
            return false //logout('validateall ' + mode + ' no rows')
        }

    }

    //logout('validateall ' + mode)
    return true

}

function* writedoc(unlock) {

    //login('writedoc')

    //check all required fields are present
    if (!(yield* validateall()))
        return false //logout('writedoc - validate all failed')

    //custom prewrite routine
    goldvalue = ''
    gvalue = ''
    if (typeof (form_prewrite) == 'function') {
        if (!(yield* exodusevaluateall('yield* form_prewrite()', 'yield* writedoc()'))) return false
        ///log('form_prewrite - after')
    }

    //stop the relocker while saving the document
    stoprelocker()

    //ok save it

    gro.request = unlock ? 'WRITEU' : 'WRITE' + '\r' + gdatafilename + '\r' + gkey
    gro.data = gds.data
    if (!(/**/ yield* gro.writex(unlock))) {

        //yield* exodusnote('Cannot save '+exodusquote(gkeyexternal)+' because: \r\r'+gro.response)
        yield* exodusinvalid('Cannot save ' + exodusquote(gkeyexternal) + ' because: \n\n' + gro.response)

        return false //logout('writedoc - write failed')

    }

    //any warnings are appended after response like 'OK xxx'
    if (gro.response != 'OK' && typeof form_postwrite == 'undefined') {
        yield* exoduswarning(gro.response.slice(2))
    }

    //if a cached is written then remove it from the cache (could update it instead?)
    deletecacherecord(gdatafilename, gkey)

    //option to unlock after saving
    if (unlock)
        glocked = false
    else
        //restart the relocker if failed to save
        startrelocker()

    yield* loadnewkey()

    //merge new data back into the screen
    if (gro.data) {
        wstatus('Formatting, please wait ...')

        //existing gds' elements are pointing to screen elements so do not copy
        //gds.data=gro.data

        /**/ yield* gds.rebind(gro.data)

        var recn = grecn
        grecn = null
        yield* calcfields()
        grecn = recn

        wstatus('')

        gds.isnewrecord = false
        if (!gupdateonlymode && !greadonlymode && !gpreventdeletion)
            setdisabledandhidden(deleterecord, false)

        //update in case changed (eg rows resorted)
        if (gpreviouselement) {
            var temp = getvalue(gpreviouselement)
            if (gpreviousvalue != temp) {
                gpreviousvalue = temp
                try { gpreviouselement.select() } catch (e) { }
            }
        }

    }

    //logout('writedoc ok')

    return true

}

function startrelocker() {
    //2.2 ie try at least two relocks within the locktimeout period
    grelocker = exodussetinterval('yield* relockdoc()', glocktimeoutinmins / 2.2 * 60 * 1000)
}

function stoprelocker() {
    window.clearInterval(grelocker)
}

function* relockdoc() {

    // if (!gloaded) return false

    //login('relockdoc')

    //dont relock if (already in the process of relocking or something else)
    if (grelockingdoc)
        return false //logout('relockdoc')
    if (db.requesting)
        return false //logout('relockdoc')

    grelockingdoc = true

    var relockdb = new exodusdblink()
    relockdb.request = 'RELOCK\r' + gdatafilename + '\r' + gkey + '\r' + gro.sessionid + '\r' + glocktimeoutinmins
    //document.bgcolor='green'
    var result
    if (yield* relockdb.send()) {
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
            setchangesmade(false)
            setdisabledandhidden(saverecord, true)
            setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)
            setdisabledandhidden(deleterecord, true)
            yield* exoduswarning(response)
        }
        else {
            yield* exodusnote(response)
        }

    }

    grelockingdoc = false

    //logout('relockdoc')

    return result

}

//'''''''
//'UNLOCK
//'''''''
function* unlockdoc() {

    if (!gKeyNodes)
        return

    //login('unlockdoc')

    //always return true even if cannot unlock

    //stop the relocker while trying to unlock
    stoprelocker()

    if (glocked) {
        while (true) {
            db.request = 'UNLOCK\r' + gdatafilename + '\r' + gkey + '\r' + gro.sessionid
            if (yield* db.send())
                break
            if (!(yield* exodusyesno('Cannot release document - try again?\n\n' + db.response))) break
        }
    }

    glocked = false
    setdisabledandhidden(deleterecord, true)
    setdisabledandhidden(saverecord, true)
    //setdisabledandhidden(editreleaserecord,true)
    setgraphicbutton(editreleaserecord, '<u>E</u>dit', geditimage)

    //logout('unlockdoc')

    return true

}

////////////////////// FIELD FUNCTIONS //////////////////////

function focuson(element) {

    //login('focuson')

    if (!element)
        return false //logout('focuson - no element')

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
            return false //logout('focuson - cannot getElementById ' + exodusquote(element))

        element = element2
    }

    //use the first if more than one
    if (typeof element == 'object' && !element.tagName && element.length > 0)
        element = element[0]

    ///log(element.id + ' ' + element.outerHTML)

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
        catch (e) { }

        //gpreviouselement = element
        //gpreviousvalue = getvalue(gpreviouselement)
        setgpreviouselement(element)
        yield * setdefault(element)
        //should really call setgpreviouselement again here
        ///log('gpreviouselement and value set to ' + gpreviouselement.id + ' ' + exodusquote(gpreviousvalue))

        try {
            if (element.tagName != 'TEXTAREA')
                element.select()
        }
        catch (e) { }

    } else {

        //needed because delete line leaves grecn>nrecs
        grecn = getrecn(element)

        //taken out otherwise F7 on job number goes to market code
        // and validateupdate fails because record has not been loaded
        //yield* setdefault(element)

        gfocusonelement = element
        exodussettimeout('focuson2()', 10)
    }

    //logout('focuson ' + element.id)

}

function focuson2() {

    if (!gfocusonelement)
        return

    var focusonelement = gfocusonelement
    gfocusonelement = null

    console.log('focuson2 ' + focusonelement.tagName + ' ' + focusonelement.id)

    //allowreadonly=true
    if (!(exodusenabledandvisible(focusonelement, true)))
        return focusnext(focusonelement)

    try {
        focusonelement.blur()
        focusonelement.focus()


        //focusing may cause validation that returns focus back to gpreviouselement
        //activeElement not available everywhere
        //if (document.activeElement!=focusonelement)
        //commented attempting to allow left/right key to work
        //if (event.target != focusonelement)
        //    return

        //if (focusonelement.tagName!='TEXTAREA')
        if (focusonelement.select)
            focusonelement.select()

    }
    catch (e) { }

}

//'''''''''
//'ON FOCUS
//'''''''''
function document_onfocus_sync(event) {
    //LEAVE SPACE after function name to prevent this having yield * prefixed
    var result = document_onfocus(event)
    //above in yielding code returns an object with a method .next() to actually run it
    /* yield */ result = result.next()
    return result
}

function* document_onfocus(event) {

    event = getevent(event)
    //window.status='onfocus '+new Date

    //ckeditor configured to pass element.id in event
    if (event.target && event.target.listenerData) {
        event.target = $$(event.target.listenerData)
        event.target.isckeditor = true
        if (gCKEDITOR_EDITOR)
            gCKEDITOR_EDITOR.setReadOnly(!glocked)//also in yield* opendoc2() post read document_onfocus
    }

    if (!event.target)
        return

    //do nothing if not initialised
    if (!ginitok)
        return

    exodussetcookie('', 'EXODUSlogincode', glogincode, 'logincode')
    //window.status=new Date()+' '+glogincode

    var text = 'document_onfocus' + ' tag:' + event.target.tagName + ' id:' + event.target.id + (gpreviouselement ? ' gpreviouselement:' + gpreviouselement.id : '')
    console.log(text)
    ///log(text)
    //window.status=text

    //window.status='document_onfocus'+' tag:'+event.target.tagName+' id:'+event.target.id+(gpreviouselement?' gpreviouselement:'+gpreviouselement.id:'')

    ///log('get element and elementid')
    var element = event.target

    ///log('quit if no element.id')
    if (!element.id)
        return false //logout('document_onfocus')

    ///log('quit calendar focus')
    if (element.className == 'calendar')
        return false //logout('document_onfocus')

    ///log('get elementid')
    var elementid = ''
    try {
        elementid = element.id.toString()
        ///log('handle focus on ckeditor')
        if (elementid.indexOf('___Frame') > 0) {
            ///log('to prevent endless loop after cancelling postread (due to ckeditor refocussing on itself?')
            ///log('minor problem is that this prevents click on ckeditor from triggering a record read')
            if (gKeyNodes && !gkey) {
                ///log('timeout to focus on last key element')
                exodussettimeout('try{gKeyNodes[gKeyNodes.length-1].focus()}catch(e){}', 10)
                return false //logout('document_onfocus')
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
    ////log('ensure checkboxes in rows have onclick set')
    //if (!element.onclick&&element.type=='checkbox'||element.type=='radio')
    // //element.onclick=onclickradiocheckbox
    // addeventlistener(element,'click','onclickradiocheckbox')

    ///log('if arrived on a readonly field by tab (or enter as tab)\nthen skip forwards (or backwards) to the next field')
    if (gkeycode == 9 || gkeycode == 13) {
        ///log('but only if normal tabindex because focusnext doesnt work otherwise')
        if (element.getAttribute('exodusreadonly') && element.tabIndex == 999) {
            if (event.shiftKey)
                focusprevious(element)
            else
                focusnext(element)
            return false //logout('document_onfocus')
        }
    }

    ///log('no validation/update except changing exodus elements:' + element.getAttribute('exodustype'))
    if (!(element.getAttribute('exodustype'))) {
        //logout('document_onfocus')
        return
    }

    ///log('quit if opening')
    if (gopening) {
        console.log('gopening still!')
        //ignore this until fix resuming after ok/cancel
        //logout('document_onfocus')
        return exoduscancelevent(event)
    }

    ///log('drop down any "modal" popup divs')
    yield* form_closepopups()

    ///log('quit if refocussing')
    if (element == gonfocuselement) {
        //  if (yield* setdefault(element))  if (element.tagName!='TEXTAREA') element.select()
        return false //logout('document_onfocus')
    }
    gonfocuselement = element

    ///log('quit if refocussing on gpreviouselement')
    if (gpreviouselement && element == gpreviouselement)
        return false //logout('document_onfocus')

    //dont validate update if clicked popup
    //if (element&&element.id==(gpreviouselement.id+'_popup'))
    // return false //logout('document_onfocus')

    ///log('check if changed element')
    if (element != gpreviouselement) {
        ///log('validate/update previous data entry ' + (gpreviouselement ? gpreviouselement.id : ''))
        if (!(yield* validateupdate()))
            return false //logout('document_onfocus')
    }

    ///log('quit if we are not focused on a proper element')
    if (!elementid)
        return false //logout('document_onfocus')

    ///log('OK. Now previous element is valid')

    ///log('find what row we are on')
    grecn = getrecn(element)

    ///log('quit if not exodus data entry field')
    if (!element.tagName.match(gdatatagnames))
        return false //logout('document_onfocus')

    ///log('check key fields')
    if (gKeyNodes && !glocked) {
        if (!(yield* checkrequired(gKeyNodes, element, 0)))
            return false //logout('document_onfocus' + ' ' + elementid + ' is required but is blank (0)')
    }

    ///log('check no required fields are missing in prior data')
    ///log(' fill in any defaults where possible')
    if (!gKeyNodes || glocked)//check keynodes as well now
    {

        ///log('check no missing data in group 0 always')
        if (!(yield* checkrequired(gfields, element, 0)))
            return false //logout('document_onfocus' + ' ' + elementid + ' a prior element is visible and required but is blank (1)')

        //check specific group if >0
        //var elements
        //if (Number(element.getAttribute('exodusgroupno')))
        //{
        // elements=getancestor(element,'TR').all
        // if (!(yield* checkrequired(elements,element,Number(element.getAttribute('exodusgroupno')))))
        //  return false //logout('document_onfocus'+' '+elementid+' a prior element is visible and required but is blank (2)')
        //}

    }

    ///log('all previous fields are valid now')
    //wwww
    //triggered by timeout from validateupdate now
    //but do it ALSO here in case triggered by defaulted key field
    ///log('if the key is complete and not on a key field, get the record')
    //if (!gloaded&&gKeyNodes&&element.getAttribute('exodusfieldno')!=0)
    //dont check if element not changed to avoid endless loop if opendoc errors
    if (gKeyNodes && element != gpreviouselement) {
        var nextkey = yield* getkey()
        //if (nextkey.toUpperCase() != nextkey)
        //    xxx = 1
        if (nextkey && (gloaded && nextkey != gkey) || (!gloaded && element.getAttribute('exodusfieldno') != 0))
        //if (key&&(gloaded&&key!=gkey)||(!gloaded))
        {
            //exodussettimeout('yield* opendoc()',100)
            yield* opendoc(nextkey)
            return false //logout('document_onfocus' + ' ' + exodusquote(elementid) + ' new record')
        }
    }
    ///log('there is no new record so setup current element')

    ///log('scroll to top left if the key field')
    if (element == gstartelement || element.getAttribute('exodusfieldno') == 0)
        window.scrollTo(0, 0)

    //if (element.tagName.match(gtexttagnames))  if (element.tagName!='TEXTAREA') element.select()

    ///log('scroll into view')
    scrollintoview(element)

    ///log('remove blanks used to force formatting of spans')
    if (element.tagName == 'SPAN' && element.innerText == ' ')
        element.innerText = ''

    ///log('change gpreviouselement/value to current element/value')
    //gpreviouselement = element
    //gpreviousvalue = getvalue(gpreviouselement)
    setgpreviouselement(element)

    ///log('set the default of the current element')
    yield* setdefault(element, donotvalidateupdate = true)

    ///log('deselect previous (this does not work unless .select() is used')
    ///log(' but causes a problem in focussing on the previous element and rescrolling')
    //if (element.tagName == 'SELECT' && document.selection && document.selection.createRange) {
    //    var textrange = document.selection.createRange()
    //    textrange.collapse()
    //    //  textrange.select()
    //}

    //log('select the whole text')
    if (element.tagName != 'TEXTAREA' && element.tagName != 'OPTION' && element.tagName != 'SELECT')
        try {

            //select works on INPUT elements
            if (element.select)
                element.select()
            else {

                //TODO remove code duplication

                //seems to be the most standards based way and works on spans too
                //http://stackoverflow.com/questions/11451353/how-to-select-the-text-of-a-span-on-click
                if (window.getSelection && document.createRange) {
                    selection = window.getSelection();
                    if (selection.isCollapsed) {
                        //following selects all of text
                        selection.removeAllRanges();
                        range = document.createRange();
                        range.selectNodeContents(element);
                        selection.addRange(range);
                    } else {
                        //following removes any selection
                        selection.collapseToEnd();
                    }
                    //            return exoduscancelevent(event)
                } else if (document.selection && document.body.createTextRange) {
                    range = document.body.createTextRange();
                    range.moveToElementText(element);
                    range.select();
                    //            return exoduscancelevent(event)
                }
            }

        } catch (e) { }

    //log('focus on current or next element')
    //this is not necessary unless .select() is used above
    //note that validation code from previous field may make current element disappear
    if (!element.isckeditor) {
        try {
            //element.focus()
            focuson(element)
            //log('focussed on current element')
        }
        catch (e) {
            //log('try focusnext')
            focusnext(element)
        }
    }

    //logout('document_onfocus' + ' ' + elementid)

}

/*
function* onclickradiocheckbox(event) {

event=getevent(event)

//wstatus(getvalue(event.target.id)+' '+gpreviousvalue)
//space or arrow keys also simulate a click event

//update immediately
//if (element.type=='radio') element.checked=true
//if (element.type=='checkbox') element.checked=!element.checked
//alert(event.target.getAttribute)
//alert('onclickradiocheckbox')
//validate or return to original
if (!(yield* validateupdate())) {

setvalue(gpreviouselement,gpreviousvalue)
return
}
gpreviousvalue=getvalue(gpreviouselement)

}
*/

function* onclickradiocheckbox(event) {

    event = getevent(event)

    //wstatus(getvalue(event.target.id)+' '+gpreviousvalue)
    //space or arrow keys also simulate a click event

    //handle case where onfocus has not been called before onclick
    if (gpreviouselement != event.target) {

        //get the CURRENT value BEFORE it was clicked
        //NB converted to grecn/gpreviousvalue IF AND AFTER previous element validates below
        var recn = getrecn(event.target)
        var previousvalue = yield* gds.getx(event.target, recn)
        //checkbox and radio buttons are currently not supported in multivalues
        //so the following is probably never executed
        if (typeof recn == 'number')
            previousvalue = previousvalue[0]

        //validate the PREVIOUS field
        if (!(yield* validateupdate())) {
            //if returning to previous field then reset the CURRENT element
            // back to what it was prior to clicking it
            setvalue(event.target, previousvalue)
            return
        }

        //setup as if onfocus had been called prior to onclick
        grecn = recn
        //gpreviouselement = event.target
        //gpreviousvalue = previousvalue
        setgpreviouselement(event.target, previousvalue)

    }

    //update immediately
    //if (element.type=='radio') element.checked=true
    //if (element.type=='checkbox') element.checked=!element.checked
    //alert(event.target.getAttribute)
    //alert('onclickradiocheckbox')
    //validate or return to original
    if (!(yield* validateupdate())) {
        setvalue(gpreviouselement, gpreviousvalue)
        return
    }
    gpreviousvalue = getvalue(gpreviouselement)

}

function* validateupdate() {

    var id = gpreviouselement ? gpreviouselement.id : 'undefined'
    //login('validateupdate ' + id)

    //return if no previouselement
    if (!gpreviouselement) {
        //logout('validateupdate - no previous element')
        return true
    }

    //return if no changes (ignoring case if lowercase not allowed)
    var newvalue = getvalue(gpreviouselement)
    //done in getvalue now
    //if (!gpreviouselement.getAttribute('exoduslowercase') && gpreviouselement != 'radio' && gpreviouselement.type != 'checkbox')
    //    newvalue = newvalue.toUpperCase()
    if (newvalue == gpreviousvalue) {
        //logout('validateupdate - gpreviousvalue:' + gpreviousvalue + ' same as newvalue:' + newvalue)
        return true
    }

    //log('User/setdefault changed ' + id + '\nfrom ' + exodusquote(gpreviousvalue) + '\nto ' + exodusquote(newvalue))

    //check for prior required fields if a grouped element
    var elements
    if (Number(gpreviouselement.getAttribute('exodusgroupno'))) {
        elements = getancestor(gpreviouselement, 'TR').getElementsByTagName('*')
        if (!(yield* checkrequired(elements, gpreviouselement, Number(gpreviouselement.getAttribute('exodusgroupno'))))) {
            setvalue(gpreviouselement, gpreviousvalue)
            return false //logout('validateupdate ' + id + ' a prior element is visible and required but is blank')
        }
    }

    //check arguments
    assertelement(gpreviouselement, 'validateupdate', 'gpreviouselement')
    if (!gpreviouselement.tagName.match(gdatatagnames)) {
        //logout('validateupdate - not an INPUT or SELECT')
        return true
    }

    //quit ok if gpreviouselement is no longer bound
    grecn = getrecn(gpreviouselement)
    if (grecn == -1) {
        //logout('validateupdate - gprevious no longer exists')
        return true
    }

    //get the old value (internal format) and grecn
    gcell = gds.getcells(gpreviouselement, grecn)[0]
    goldvalue = gcell.text
    gorigvalue = (typeof gcell.oldtext != 'undefined') ? gcell.oldtext : gcell.text

    //validate it and put back focus if it fails
    ////////////////////////////////////////////
    //NOTE gets/sets gvalue
    if (!(yield* onbeforeupdate(gpreviouselement))) {
        focusongpreviouselement()

        //logout('validateupdate - onbeforeupdate failed')
        return false

    }

    //pressing escape will no longer cancel gchangesmade
    gelementthatjustcalledsetchangesmade = null

    //update
    ////////

    //update the element multiple selections
    //if reply is an array use replace all rows
    var multiplegvalue = false
    if (typeof (gvalue) == 'object') {
        if (gvalue.length > 1 && Number(gpreviouselement.getAttribute('exodusgroupno'))) {
            multiplegvalue = true

            var elementid = gpreviouselement.id
            yield* insertallrows(gpreviouselement, gvalue)

            //focus on next element AFTER table
            element = $$(elementid)
            if (element && element[0])
                element = element[0]
            focusdirection(1, element, Number(gpreviouselement.getAttribute('exodusgroupno')))

        }
        else {
            gvalue = gvalue[0]
        }
    }

    //update the element if not multiple selections
    if (!multiplegvalue) {
        var defaulted = gpreviouselement.dataitem && gpreviouselement.dataitem.defaulted

        yield* gds.setx(gpreviouselement, grecn, gvalue)

        //restore defaulted status if case it was reset by setx
        if (defaulted)
            gpreviouselement.dataitem.defaulted = defaulted
    }

    //wwww
    /* cant do this here otherwise will retrieve multipart key records when only clearing
        //switch to new key if new and user accepts to close the current one
        var nextkey
        if (gKeyNodes
         && gpreviouselement.getAttribute('exodusfieldno') == 0
         && (nextkey = yield* getkey())
         && nextkey != gkey) {
            //do not change key if user chooses not to unload an existing document
            if (!(yield* closedoc('CHANGEKEY'))) {
                focusongpreviouselement()
                return false //logout('validateupdate - closedoc refused')
            }
    
            //return false and just allow opendoc to happen
    
            //exodussettimeout('yield* opendoc("'+nextkey+'")',1)
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
            yield* opendoc(nextkey)
    
            //logout('validateupdate - opendoc')
            return false
        }
    */
    //post update
    /////////////

    //flag record edited
    if (gpreviouselement.getAttribute('exodusfieldno') != 0
        && !gpreviouselement.getAttribute('exodusnochangeswarning')) {
        setchangesmade(true)
    }

    //calculate dependencies
    yield* calcfields(gdependents)
    gdependents = []

    //why is this necessary?
    gpreviousvalue = getvalue(gpreviouselement)

    //logout('validateupdate - done')

    //if (gautofitwindow)
    //    exodussettimeout('exodusautofitwindow()', 1)

    return true

}

function focusongpreviouselement() {
    window.setTimeout(focusongpreviouselement2, 1)
}

function focusongpreviouselement2() {

    //log('focusing back to ' + gpreviouselement.id)

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

    //exoduscancelevent(event)

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

function* earlyupdate() {

    //skip update on buttons
    //if (!gpreviouselement || !gpreviouselement.name) return
    if (!gpreviouselement || !gpreviouselement.id || (gpreviouselement.type && gpreviouselement.type == 'button'))
        return

    yield* gds.setx(gpreviouselement, grecn, gvalue)

}

function* checkrequired(elements, element, groupno) {

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
            //if $$ return more than one (nb SELECT elements may have [] for its options)
            if (element2 && !element2.tagName && element2[0])
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
        if (!element2.tagName.match(gdatatagnames))
            continue

        //don't check current but continue looking for lower tabindexed fields
        //if (element2==element) //this does not work because gfields<> bound table elements for rows
        //if (element2.getAttribute('exodusscreenfn')==element.getAttribute('exodusscreenfn'))
        //check ids because exodusscreenfn is repeated in gfields for radio 4,4,4
        //but not in the 2nd and subsequent repeated form elements 4,5,6
        if (element2.id == element.id) {
            foundelement = true
            continue
        }

        //skip elements not in desired group
        //must be after detection of foundelement above
        if (Number(element2.getAttribute('exodusgroupno')) != groupno)
            continue

        if (Number(element2.getAttribute('exodusgroupno')) == groupno) {
            if (((!foundelement && element2.tabIndex <= element.tabIndex) || (element2.tabIndex != -1 && element2.tabIndex < element.tabIndex))) {
                //if (element&&element2.getAttribute('exodusrequired')&&gds.getcells(element2,grecn)[0].text=='')
                //if (element&&element2.getAttribute('exodusrequired')&&getvalue(element2)=='')
                //if (element&&(!Number(element.getAttribute('exodusgroupno'))||element2.getAttribute('exodusrequired'))&&getvalue(element2)=='')
                if (element
                    && ((gds.isnewrecord && !Number(element2.getAttribute('exodusgroupno')))
                        || element2.getAttribute('exodusrequired'))
                    && getvalue(element2) == '') {

                    //try to set the default
                    if (!(yield* setdefault(element2)) && exodusenabledandvisible(element2)) {
                        focuson(element2)
                        return false
                    }

                    //if still empty then fail
                    //if (getvalue(element2)=='')
                    if (element2.getAttribute('exodusrequired') && !element2.getAttribute('exodusreadonly') && gds.getcells(element2, grecn)[0].text == '') {

                        //disabled or invisible elements may be blank and required (even after setdefault)
                        if (element2.disabled || element2.getAttribute('disabled') || !exodusenabledandvisible(element2))
                            return true

                        //put up a message unless is the first column of a row
                        if (true || !(element2.getAttribute('exodusisfirstinputcolumn')))
                            yield* exodusinvalid(element2.getAttribute('exodustitle') + ' is required..')

                        focuson(element2)
                        //if (!(Number(element2.getAttribute('exodusgroupno'))))
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
            systemerror('getvalue()', exodusquote(element0) + ' does not exist')
            return
        }
        //get first one only
        if (!element.tagName) {
            //element = element[0]
            if (element[0].getAttribute('exodusgroupno')) {
                if (typeof recn == 'undefined') recn = getrecn()
            }
            else recn = 0
            element = element[recn]
            if (!element) {
                systemerror('getvalue()', exodusquote(element0) + ' does not exist')
                return
            }
        }
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

        case 'INPUT': {

            switch (element.type) {

                case 'text':
                    //var tx = element.value.exodustrimr()

                    //similar code in INPUT and SPAN
					//always trim trailing white space
                    var tx = element.value.replace(/\s+$/, '')
                    if (!element.getAttribute('exoduslowercase')) {
                        //may be visibly uppercase due to style but internally lowercase so change it here
                        tx = tx.toUpperCase()
						//trim leading white space if lower case not allowed
						tx = tx.replace(/^\s+/, '')
					}
                    return tx

                case 'radio': {

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

                case 'checkbox': {

                    var elements = getradiocheckboxelements(element)
                    var values = []
                    for (var ii = 0; ii < elements.length; ii++) {
                        var element = elements[ii]
                        if (element.checked) {
                            values[values.length] = (typeof element.value == 'undefined') ? ii : element.value
                        }
                    }
                    return values.join(sm)

                }

                case 'button': {

                    return ''
                }

                default: {

                    return systemerror('getvalue()', exodusquote(element.type) + ' invalid INPUT element type')
                }

            }

        }

        case 'OPTION': return getdropdown0(element.parentNode)

        case 'SELECT': return getdropdown0(element)

        //crlf becomes space to prevent entry of tm characters in text fields
        case 'SPAN':
            //if (element.isContentEditable)
            //var value = element.innerText.replace(/([\r\n]+)/g, ' ').exodustrimr()
            //allow /r by itself but replace \\r\n combinations (to allow spans to have multiple lines)
            //var value = element.innerText.replace(/\r\n/g, ' ').exodustrimr()

            //var value = element.innerText //returns with random missing spaces in IE11
            //works on IE11 as well only from IE9
            //alternatives are .data and .wholeText
            if (typeof element.textContent != 'undefined')
                var value = element.textContent
            else
                //the old method seems to work everywhere except later versions of IE
                //Note: .innerText property shim to textContent is added to HTMLElement.prototype in client.js
                var value = element.innerText

            //MSIE (IE7) has \r\n for line marks whereas FIREFOX et al have \n
            //value = value.replace(/\r\n/g, ' ').exodustrimr()
            //else
            //    var value = element.innerHTML.replace(/([\r\n]+)/g, ' ').exodustrimr()
            //if (value == nbsp160)
            //    return "";
            //remove trailing spaces and end of lines
            //return value.replace(/[ \r\n]+$/, '')

			//similar code in INPUT and SPAN
            //remove trailing white space
            value = value.replace(/\s+$/, '')
            if (!element.getAttribute('exoduslowercase')) {
                //may be visibly uppercase due to style but internally lowercase so change it here
                value = value.toUpperCase()
                //trim leading white space if lower case not allowed
                value = value.replace(/^\s+/, '')
            }
            return value

        case 'TEXTAREA': {

            //http://wiki.ckeditor.net/Developer%27s_Guide/Javascript_API
            if (typeof CKEDITOR != 'undefined') {
                var oEditor = CKEDITOR.instances[element.id]
                if (oEditor) {
                    //needs encoding set correctly eg UTF-8 in meta of html page
                    var result = oEditor.getData()
                    return result
                }
            }

            return element.value.exodustrimr()
        }

        default: {

            return systemerror('getvalue()', exodusquote(element.tagName) + ' invalid tagName')
        }
    }

}

function exodussetreadonly(elements, msg, options, recn) {

    //note: cannot set readonly off if the dictionary says that it is readonly

    //if msg contains something then set the element(s) to readonly and vice versa
    //tabbing and cursor keys will skip past readonly fields
    //msg will be displayed if they click and try and change the field

    var elementx = elements

    //if elements is a number then it means set all one group (for selected recn or null for all)
    if (!(isNaN(+elementx))) {
        var elements = []
        for (var ii = 0; ii < gfields.length; ii++) {
            if (Number(gfields[ii].getAttribute('exodusgroupno')) == elementx
                && Number(gfields[ii].getAttribute('exodusfieldno'))
                && (!(exodussetreadonly(gfields[ii].id, msg, options, recn))))
                return false
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
            if (!(gds.dictitem(elementxstring)))
                return systemerror('exodussetreadonly()', exodusquote(elements) + ' is not in the form')
            return false
        }

        //handle arrays of recn recursively
        //handles arrays of fieldnames and recns efficiently when items are string ids
        //since extraction of all elements for one element id is probably time consuming
        if (typeof recn == 'object' && recn.length && !elementx.tagName) {
            if (typeof elementx.length == 'undefined')
                elementx = [elementx]
            for (var ii = 0; ii < recn.length; ii++) {
                if (!(exodussetreadonly(elementx[recn[ii]], msg, options)))
                    return false
            }
            return true
        }

        if (!elementx.tagName && typeof recn != 'undefined' && recn != null && recn !== '') {
            elementx = elementx[recn]
            if (typeof elementx == 'undefined') {
                return systemerror('exodussetreadonly(' + elements + ',' + msg + ',' + recn + ')')
            }
            elementx = [elementx]
        }
        if (!elementx.tagName && elementx.length == 1)
            elementx = elementx[0]
    }

    //handle arrays recursively
    //if (typeof elementx=='object'&&elementx.length&&!elementx.name)
    if (typeof elementx == 'object' && elementx.length && !elementx.getAttribute) {
        for (var ii = 0; ii < elementx.length; ii++) {
            //if (!(exodussetreadonly(elementx[ii],msg,options,recn))) return false
            //dont stop just because one doesnt exist
            exodussetreadonly(elementx[ii], msg, options, recn)
        }
        return true
    }

    //from here on we are handling one element

    if (!options) options = ''

    //skip dict items that are marked readonly
    //in chart of accounts dictitem is not available
    if (gds.dictitem) {
        var di = gds.dictitem(elementx.id)
        if (!di || di.readonly)
            return true
    }

    //provide hasAttribute method for ie6 etc
    if (!elementx.hasAttribute) {
        elementx.hasAttribute =
            function hasAttribute(attrName) {
                return typeof this[attrName] !== 'undefined'
            }
    }

    if (msg) {
        elementx.setAttribute('exodusreadonly', msg)

        //activeElement not available everywhere
        //if (elementx.id!=document.activeElement.id)
        if (gevent && (typeof gevent.target == 'undefined' || elementx.id != gevent.target.id)) {
            //remove tabindex (unless we are on the element otherwise tab key doesnt work anymore!)
            elementx.setAttribute('oldtabindex', elementx.tabIndex)
            elementx.tabIndex = -1
        }

        //if (elementx.onchange)
        // elementx.oldonchange=elementx.onchange
        //elementx.onchange=readonly_onchange
        addeventlistener(elementx, 'change', 'readonly_onchange')

        if (options && options.indexOf('BGCOLOR') >= 0) {
            if (!elementx.getAttribute('oldbgcolor'))
                elementx.setAttribute('oldbgcolor', elementx.style.backgroundColor)
            //elementx.style.backgroundColor='#ffffc0'
            var newbgcolor = elementx.parentNode.style.backgroundColor
            if (!newbgcolor)
                // newbgcolor=document.styleSheets[0].rules[0].style.backgroundColor
                newbgcolor = '#f6f6f6'
            elementx.style.backgroundColor = newbgcolor

            //spans have no type
            if (!elementx.type || !elementx.type.match(gradiocheckboxtypes)) {
                if (!elementx.oldborderwidth)
                    elementx.setAttribute('oldborderwidth', elementx.style.borderWidth)
                elementx.style.borderWidth = '1px'

                if (!elementx.hasAttribute('oldborderstyle'))
                    elementx.setAttribute('oldborderstyle', elementx.style.borderStyle)
                elementx.style.borderStyle = 'solid'

                if (!elementx.hasAttribute('oldbordercolor'))
                    elementx.setAttribute('oldbordercolor', elementx.style.borderColor)
                elementx.style.borderColor = '#dddddd'
            }

        }

        if (elementx.type && elementx.type.match(gradiocheckboxtypes))
            elementx.setAttribute('disabled', true)

        if (options && options.indexOf('BORDER') >= 0) {
            if (!elementx.hasAttribute('oldborderstyle'))
                elementx.setAttribute('oldborderstyle', elementx.style.borderStyle)
            elementx.style.borderStyle = 'none'
        }

    }
    else {

        elementx.removeAttribute('exodusreadonly')
        //if (elementx.oldonchange)
        // elementx.onchange=elementx.oldonchange
        //addeventlistener(elementx,'change',elementx.oldonchange)
        //else
        {
            if (elementx.hasAttribute('onchange') || elementx.onchange) {
                elementx.removeAttribute('onchange')
                //removeAttribute does not seem to work on functions so also
                elementx.onchange = null
                //TODO check if this is a memory leak
                //TODO implement REMOVEEVENTLISTENER and do it here
                //addeventlistener(elementx,'change',function(){})
            }
        }
        if (elementx.hasAttribute('oldtabindex')) {
            elementx.tabIndex = elementx.getAttribute('oldtabindex')
            //elementx.removeAttribute('oldtabindex')
        }
        if (elementx.hasAttribute('oldbgcolor'))
            elementx.style.backgroundColor = elementx.getAttribute('oldbgcolor')

        elementx.removeAttribute('disabled')

        if (elementx.hasAttribute('oldborderstyle'))
            elementx.style.borderStyle = elementx.getAttribute('oldborderstyle')

    }

    //move onto next field if setting current focus field to readonly
    //but only if default tabindex since focusnext cant find the next tabindex properly
    //activeElement not available everywhere
    //if (document.activeElement.getAttribute('exodusreadonly')&&gpreviouselement&&document.activeElement.tabIndex==999)
    if (gevent && typeof gevent.target != 'undefined' && gevent.target.getAttribute && gevent.target.getAttribute('exodusreadonly') && gpreviouselement && document.activeElement.tabIndex == 999)
        focusnext(gpreviouselement)

    return true

}

function* readonly_onchange(event) {
    event = getevent(event)

    //called when user starts to change a readonly element

    //ckeditor configured to pass element.id in event
    if (event.target && event.target.listenerData) {
        event.target = $$(event.target.listenerData)
    }

    var element = event.target
    var readonlymsg = element.getAttribute('exodusreadonly')
    if (!readonlymsg || readonlymsg == 'true')
        return

    setvalue(gpreviouselement, gpreviousvalue)

    yield* exodusinvalid(readonlymsg)

    return exoduscancelevent(event)

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
            systemerror('setvalue', exodusquote(elementid) + ' element does not exist')
            return
        }
    }

    //login('setvalue ' + elementid + ' ' + valueorvalues)

    setvalue2(element, valueorvalues)

    //logout('setvalue ' + elementid + ' ' + valueorvalues)

}

function setvalue2(element, value) {

    //unprotected (faster) core of setvalue()
    value = value.toString()

    switch (element.tagName) {
        case 'INPUT': {

            switch (element.type) {

                case 'text': {

                    setexoduslink(element, value)
                    element.value = value
                    break
                }

                case 'radio': {

                    var elements = getradiocheckboxelements(element)
                    for (var ii = 0; ii < elements.length; ii++) {
                        var element = elements[ii]
                        if (value.exoduslocate(element.value.toString()) > 0) {
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

                case 'checkbox': {

                    //multiple values as : or , separated string
                    if (typeof value == 'string' && value.indexOf(':') >= 0) value = value.split(':')
                    if (typeof value == 'string') value = value.split(sm)

                    var elements = getradiocheckboxelements(element)
                    for (var ii = 0; ii < elements.length; ii++) {
                        var element = elements[ii]
                        if (value.exoduslocate(element.value.toString()) > 0)
                            element.checked = true
                        else
                            element.checked = false
                    }

                    break

                }

            }

            break//INPUT
        }

        case 'TEXTAREA': {

            setexoduslink(element, value)

            //http://wiki.ckeditor.net/Developer%27s_Guide/Javascript_API
            if (typeof CKEDITOR != 'undefined') {
                oEditor = CKEDITOR.instances[element.id]
                if (oEditor) {
                    //oEditor.SetHTML(value)
                    //oEditor.setData(value)
                    //using .setTimeout() because Internet Explorer 10/11 at least give error "permission denied"
                    //if you call setData before previous setData has completed (asynchronously)
                    window.setTimeout(function () {
                        try {
                            oEditor.setData(value)
                            //oEditor.updateElement(value)
                        } catch (e) { }
                    }, 500)
                }
            }

            element.value = value
            break
        }

        case 'SPAN': {

            setexoduslink(element, value)

            if (element.getAttribute('exodusimage')) {
                var img = element.nextSibling
                if (!img)
                    img = element.parentNode.nextSibling
                if (img && img.tagName == 'IMG')
                    img.src = value
            }

            //this puts a char 160 no-break-space to ensure height isnt zero
            //the other solution, putting style min-height doesnt zoom with ctrl+ ctrl- (does in ff 32)
            //if (value == '')
            //    value = '&nbsp;'

            //treat as text
            //IF element is contenteditable and the new value doesnt look like HTML
            // and browser doesnt support "Sequences of whitespace are preserved."
            if (element.isContentEditable && (value.slice(0, 1) != '<' || value.slice(-1) != '>') && element.style.whiteSpace != 'pre-wrap')

                //WARNING:
                //leading, trailing and excess spaces will be converted to character 160 (&nbsp;) in order to retain
                //their appearance since HTML spans dont show leading, trailing or more than one consecutive space
                //THEREFORE using span tags (but not input tags) CAN CONTAIN CHAR(160) IN TEXT ON THE SERVER
                element.innerText = value

            //treat is HTML
            //if element is not contenteditable, or the new value looks like HTML
            else
                element.innerHTML = value
            break
        }

        case 'SELECT': {

            setdropdown3(element, null, '', value)
            break
        }

        case 'OPTION': {

            setdropdown3(element.parentNode, null, '', value)
            break
        }

        default: {

            return systemerror('setvalue2()', exodusquote(element) + ' ' + exodusquote(element.tagName) + ' invalid tagName in setvalue2(' + element + ',' + value + ')')
        }

    } //of switch

}

function getradiocheckboxelements(element) {

    var scope
    var elements
    if (Number(element.getAttribute('exodusgroupno'))) {
        var scope = getancestor(element, 'TR')
        if (!scope) {
            return systemerror('getradiocheckboxelements', element.id + ' has no parentNode')
        }
    }
    else {
        scope = document
    }
    if (document.getElementsByClassName)
        elements = scope.getElementsByClassName('exodusid_' + (element.id ? element.id : element))
    else {
        elements = scope.all[element.id ? element.id : element]
        if (elements && elements.tagName)
            elements = [elements]
    }

    if (!elements.length)
        systemerror('getradiocheckboxelements("' + element.id + '") No elements found.')

    return elements

}

function setexoduslink(element, value) {
    return//always display now
    /*    if (element.getAttribute('exoduslink')) {
            if (element.previousSibling && element.previousSibling.getAttribute('exoduslink')) {
                element.previousSibling.style.display = value ? '' : 'none'
            }
            else if (element.nextSibling && element.nextSibling.getAttribute('exoduslink')) {
                element.nextSibling.style.display = value ? '' : 'none'
            }
        }
    */
}

function* getdefault(element) {

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
    //if (!element.getAttribute('exodusdefaultvalue')&&element.tagName=='SELECT')
    //{
    // //login('getdefault '+element.id)
    // var defaultvalue=getvalue(element)
    // logout('getdefault '+element.id+' SELECT '+defaultvalue)
    // return defaultvalue
    //}

    //return '' if no default
    defaultvalueexpression = element.getAttribute('exodusdefaultvalue')
    if (!defaultvalueexpression || defaultvalueexpression == '""')
        return ''

    //login('getdefault ' + element.id)

    //calculate default
    var defaultvalue = yield* exodusevaluate(defaultvalueexpression, 'yield* getdefault(' + element.id + ')')

    //select elements always have a default
    if (element.tagName == 'SELECT') {
        if (defaultvalue == '' && element.tagName == 'SELECT')
            defaultvalue = getvalue(element)
    }

    //don't default if unique and already present
    if (defaultvalue && element.getAttribute('exodusunique')) {
        var othervalues = getvalues(element.id)
        if (othervalues.exoduslocate(defaultvalue))
            defaultvalue = ''
    }

    //convert numbers to strings and check is a string
    if (typeof defaultvalue == 'number')
        defaultvalue = defaultvalue.toString()
    if (typeof defaultvalue != 'string') {
        yield* exodusinvalid(element.id + ' default returned is ' + typeof defaultvalue + ' - "" used\nExpression:' + defaultvalueexpression)
        defaultvalue = ''
    }

    //logout('getdefault ' + element.id + ' ' + defaultvalue)
    return defaultvalue

}

//var gautofitwindowpending
function setchangesmade(value, savebuttonactive) {
    gchangesmade = value
    if (!gchangesmade)
        gelementthatjustcalledsetchangesmade = null
    if (typeof savebuttonactive == 'undefined')
        savebuttonactive = value || !gKeyNodes
    var savebuttonstyle = savebuttonactive ? gsaveimage : gsavegreyimage
    setgraphicbutton(saverecord, null, savebuttonstyle)

    //if (gautofitwindow && !gautofitwindowpending) {
    //    gautofitwindowpending = true
    //    exodussettimeout('exodusautofitwindow()', 1)
    //}

}

function* setdefault(element, donotupdate) {

    //used in
    //1. newrecordfocus and cleardoc to point to 1st element
    //2. focuson (why? should be setvalue?)
    //3. onfocus at end
    //4. checkrequired

    //if no value, get default and set value

    if (typeof element == 'string')
        element = document.getElementById(element)

    //cannot update anything but key field if not locked or save button not enabled
    if (element.getAttribute('exodusfieldno') != 0 && gKeyNodes && (!glocked || saverecord.getAttribute('disabled'))) {
        //  return true
    }

    //return true if already defaulted

    if (element.dataitem) {
        if (element.dataitem.defaulted)
            return true
    }
    else {
        //only the first element of radio and checkboxes are bound to data
        //should really check if the first element is already defaulted
        if ('checkbox radio'.indexOf(element.type) >= 0)
            return true
    }

    //return true if already has a value
    //var value=gds.getcells(element,grecn)[0].text<A HREF="http://localhost/exodus7/EXODUS/images/">http://localhost/exodus7/EXODUS/images/</A>
    var value = getvalue(element)
    if (value)
        return true

    //signal that it has been defaulted (even if there is no default)
    if (element.dataitem)
        element.dataitem.defaulted = true

    //login('setdefault ' + element.id)

    //get the default value
    var storegrecn = grecn
    grecn = getrecn(element)
    gdefault = yield* getdefault(element)
    grecn = storegrecn

    //for select elements with no default, the first element is the default
    //is this not done in the binding routine now?
    if (gdefault == '' || gdefault == null || typeof (gdefault) == 'undefined') {
        if (element.childNodes.length)
            gdefault = element.childNodes[0].value
    }

    //if no default return
    if (!gdefault) {
        //logout('setdefault ' + element.id + ' no default')
        return true
    }

    //save the default for use eg by custom validation routines to avoid work
    element.setAttribute('exodusdefault', gdefault)

    //set the value (externally only)
    //yield* gds.setx(element,grecn,gdefault)

    setvalue(element, yield* oconvertvalue(gdefault, element.getAttribute('exodusconversion')))

    //call the validate/update routine
    if (!donotupdate) {

        //changed to not mess with gpreviouselement
        savegpreviouselement = gpreviouselement
        savegpreviousvalue = gpreviousvalue

        //gpreviouselement = element
        //gpreviousvalue = ''//gpreviousvalue=value
        setgpreviouselement(element, '')

        if (!(yield* validateupdate())) {
            //logout('setdefault ' + element.id + ' invalid')
            return false
        }

        //restore
        if (savegpreviouselement != gpreviouselement) {
            gpreviouselement = savegpreviouselement
            gpreviousvalue = savegpreviousvalue
        }

    }

    //log(element.id + ' defaulted to ' + gdefault)

    //logout('setdefault ' + element.id + ' ' + exodusquote(gdefault))

    return true

}

//''''''''''''''
//'BEFORE UPDATE
//''''''''''''''
function* onbeforeupdate(element) {
    //move to validateupdate?

    //ignore updates while closing
    if (gclosing) return false

    //login('onbeforeupdate')

    // if (typeof(element)=='undefined')
    // element=event.target

    //get grecn to be safe
    grecn = getrecn(element)

    //skip if value not changed
    var value = getvalue(element)
    if (value == gpreviousvalue) {
        //logout('onbeforeupdate')
        return true
    }

    //cannot update anything but key field if not locked or save button not enabled
    //if (element.getAttribute('exodusfieldno')!=0&&gKeyNodes&&(!glocked||saverecord.getAttribute('disabled')))
    if (element.getAttribute('exodusfieldno') != 0 && gKeyNodes && !glocked) {
        setvalue(gpreviouselement, gpreviousvalue)
        yield* readonlydocmsg()
        //logout('onbeforeupdate')
        return yield* exodusinvalid('')
    }

    //post entry processing
    if (!(yield* validate(element))) {
        //this should already have been done in the validate routine
        //element.focus()
        return false //logout('onbeforeupdate')
    }

    //logout('onbeforeupdate')

    return true

}

//only called from yield* validateupdate() so some code might be unnecessary duplicated like getvalue() etc
function* validate(element) {

    //new value
    gvalue = getvalue(element).toString()

    //login('validate ' + element.id + ' ' + gvalue)

    //extract other infor
    var name = element.name

    //extract global info

    //group number
    ggroupno = Number(element.getAttribute('exodusgroupno'))

    grecn = getrecn(element)

    //log('grecn=' + grecn)

    //old value
    goldvalue = gds.getcells(element, grecn)
    if (goldvalue != null)
        goldvalue = goldvalue[0].text

    //convert to uppercase
    if (!element.getAttribute('exoduslowercase') && element.type != 'radio' && element.type != 'checkbox') {
        var tt = gvalue.toUpperCase()
        if (tt != gvalue) {
            gvalue = tt
            //update it immediately in case something like yield* getkey()
            setvalue(element, gvalue)
        }
    }

    //get something to show in error messages
    var elementtitle = element.getAttribute('exodustitle')
    if (!elementtitle)
        elementtitle = element.id

    if (element.tagName != 'SELECT' && element.type != 'radio') {

        /*
        //invalid character check (nearly all punctuation except - / and #)
        if (element.getAttribute('exodusinvalidcharacters')) {

        var invalidcharacters=element.getAttribute('exodusinvalidcharacters')+'\xF8\xF9\xFA\xFB'//\xFC\xFD\xFE\xFF'

        //ignore any valid characters
        if (element.getAttribute('exodusvalidcharacters')) invalidcharacters=invalidcharacters.exodusconvert(element.getAttribute('exodusvalidcharacters'),'')

        }
        else {

        var invalidcharacters=FMs//'\xF8\xF9\xFA\xFB'//\xFC\xFD\xFE\xFF'
        }
        */

        //special key field validation and conversion
        if (Number(element.getAttribute('exodusfieldno')) == 0) {

            //remove all non-printing ASCII characters (eg tab) from key fields
            gvalue = gvalue.replace(/[\x00-\x1F]/g, '')

            //prevent anything that is effectively 0 unless it is a checkbox
            if (element.type != 'checkbox' && exodusnum(gvalue) && !Number(gvalue)) {
                yield* exodusinvalid(elementtitle + ' cannot be zero')
                return false //logout('validate')
            }
        }

        //invalid character check
        //generally dont allow any field marks (can allow specific ones by putting them in valid characters)
        var invalidcharacters = FMs
        if (element.type == 'checkbox')
            invalidcharacters = invalidcharacters.exodusconvert(sm, '')
        if (element.getAttribute('exodusinvalidcharacters')) {
            invalidcharacters += element.getAttribute('exodusinvalidcharacters')

            //valid characters override invalid characters (but be careful not to allow field marks
            if (element.getAttribute('exodusvalidcharacters'))
                invalidcharacters = invalidcharacters.exodusconvert(element.getAttribute('exodusvalidcharacters'), '')

        }

        var temp = gvalue.exodusconvert(invalidcharacters, '')
        if (temp != gvalue) {
            //   alert('Punctuation characters and spaces (except / - and #) are not allowed in key fields and have been removed\n')
            //var charsx=invalidcharacters
            //get unused invalid characters
            var charsx = invalidcharacters.exodusconvert(gvalue, '')
            //get used invalid characters
            charsx = invalidcharacters.exodusconvert(charsx, '').exodusswap('|', '&#124;')
            //.exodusswap(fm,'&u'+fm.charCodeAt(0)+';')
            yield* exodusinvalid('The following characters are not allowed in ' + elementtitle + '.\n\n&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span style="border:1px solid #DDDDDD;padding-bottom:2px"> ' + charsx.exodusswap(' ', ' space ') + '&nbsp;</span><br />&nbsp;')
            return false //logout('validate')
        }

        //valid character check
        if (element.getAttribute('exodusvalidcharacters')) {

            var temp = gvalue.exodusconvert(element.getAttribute('exodusvalidcharacters'), '')
            if (temp != '') {
                yield* exodusinvalid('Only the following characters are allowed in ' + elementtitle + '.\n\&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"' + element.getAttribute('exodusvalidcharacters').exodusswap('|', '&#124;') + '\"')
                return false //logout('validate')
            }
        }

    }

    //required check
    //log('required check')
    if (gvalue == ''
        && element.getAttribute('exodusrequired')
        && exodusenabledandvisible(element)) {
        yield* exodusinvalid(elementtitle + ' is required...')
        return false //logout('validate')
    }

    //log('before file check')
    //file check (skip if has dropdown)
    if (element.getAttribute('exodusfilename')
        && gvalue != ''
        && !element.getAttribute('exodusdropdown')) {

        var filename = element.getAttribute('exodusfilename')
        var key = gvalue

        //exodus hack (possibly not used anymore as .filename='ACCOUNTS' not used?
        if (filename == 'ACCOUNTS') key = '.' + gvalue

        db.request = 'CACHE\rREAD\r' + filename + '\r' + key
        if (!(yield* db.send())) {

            if (db.response.indexOf('NO RECORD') >= 0) db.response = exodusquote(gvalue) + ' ' + element.getAttribute('exodustitle') + ' is not on file.'

            yield* exodusinvalid(db.response)

            return false //logout('validate - not on file ' + gvalue)

        }

    }

    //log('before conversion')

    gvaluebeforeiconv = gvalue

    //if conversion is a routine. eg [NUMBER] [DATE] are standard
    var conversion = element.getAttribute('exodusconversion')
    if (typeof (conversion) != 'string' || conversion.slice(0, 1) != '[')
        conversion = false

    //input conversion
    if (conversion && gvalue != '') {

        //format is '[functionname,options]'
        var convarray = conversion.slice(1, -1).split(',')
        //if no options, make them ''
        if (convarray.length == 1)
            convarray[1] = ''

        //see also OCONV below

        //var quotechar = (gvalue.indexOf('"') >= 0) ? "'" : '"'
        //var expression = convarray[0] + '(' + '"ICONV",' + quotechar + gvalue.replace(/\\/g, '\\\\') + quotechar + ',"' + convarray.slice(1) + '")'

        var value = gvalue.replace(/\\/g, '\\\\')
        value = value.replace(/\"/g, '\\"')
        value = value.replace(/[\x0D]/g, '\\r')
        value = value.replace(/[\x0A]/g, '\\n')
        var expression = convarray[0] + '(' + '"ICONV","' + value + '","' + convarray.slice(1) + '")'

        gmsg = ''
        ivalue = yield* exodusevaluate(expression, 'yield* validate(' + element.id + ') iconv')
        if (typeof ivalue == 'undefined')
            return false //logout('validate - system error in input conversion')

        //null means failed to convert to internal value therefore invalid
        if (gvalue == null || ivalue == null) {
            //error message (use the conversion program name in the message)
            yield* exodusinvalid(exodusquote(gvalue) + ' is not a valid ' + convarray[0].toLowerCase().replace(/_/g, ' ') + '\n\n' + gmsg)
            return false //logout('validate - input conversion returned null')
        }

        //switch the data into internal format
        gvalue = ivalue

    }

    //custom validation - data in internal format
    //log('before custom validation')
    var storegrecn = grecn
    var elementvalidation = element.getAttribute('exodusvalidation')
    if (elementvalidation) {

        var ok
        if (typeof elementvalidation == 'function')
            ok = elementvalidation()
        else
            ok = yield* exodusevaluate(elementvalidation, 'yield* validate() functioncode')

        if (gvalue == null)
            yield* exoduswarning(element.id + ' validation routine returned gvalue=null')
        if (!ok || gvalue == null) {
            grecn = storegrecn
            yield* exodusinvalid()
            console.log('validate - VALIDATION FUNCTION CODE RETURNED FALSE OR GVALUE AS NULL')
            return false //logout('validate - validation function code returned false or gvalue as null')
        }

    }

    //get grecn again in case any prior lines have been deleted in the validation routine
    //grecn=storegrecn
    grecn = getrecn(gpreviouselement)

    //check for uniqueness for multivalues
    //log('before unique check')
    if (gvalue && ggroupno > 0 && element.getAttribute('exodusunique')) {
        var othervalues = yield* gds.getall(element.id)
        var ln
        //othervalues[grecn]='' //not needed because only validate if changed
        othervalues[grecn] = '' //put back because of a validation after a multiple choice popup fails
        if (ln = othervalues.exoduslocate(gvalue)) {
            gmsg = exodusquote(gvaluebeforeiconv) + ' is already used in line ' + ln + '.'
            if (element.getAttribute('exodusnonuniquewarning')) {
                if (!(confirm('Warning:\n\n' + gmsg, 1))) {
                    //logout('validate - not unique warning')
                    return yield* exodusinvalid()
                }
            }
            else {
                //logout('validate - not unique')
                return yield* exodusinvalid(gmsg)
            }
        }
    }

    //check for sequential multivalues
    //assumes that numbers will be in number format
    var elementsequence = element.getAttribute('exodussequence')
    if (gvalue
        && ggroupno > 0
        && elementsequence) {
        var title = element.getAttribute('exodustitle')
        if (elementsequence == 'A') {
            var temp
            if ((temp = yield* getpreviousrow('', true, true))
                && gvalue < temp) {
                //    alert(typeof gvalue+' '+gvalue+' < '+typeof temp+' '+temp)
                //logout('validate - not sequential')
                return yield* exodusinvalid(title + ' cannot be less than ' + title + ' in the previous row above')
            }
            var temp
            if ((temp = yield* getnextrow('', true, true))
                && gvalue > temp) {
                //    alert(typeof gvalue+' '+gvalue+' > '+typeof temp+' '+temp)
                //logout('validate - not sequential')
                return yield* exodusinvalid(title + ' cannot be greater than ' + title + ' in the next row below')
            }
        }
    }

    //output conversion
    //log('before output conversion')

    ovalue = yield* validateoconv(element, gvalue)
    if (ovalue == 'undefined' || ovalue == null)
        return false //logout('validate - oconv failed')

    //not needed because setx will update the screen with oconverted data
    //gvalue=ovalue

    //logout('validate ' + element.id + ' ' + gvalue)

    return true

}

function* validateoconv(element, ivalue) {

    //returns ovalue or null if oconv fails

    //skip if nothing to convert
    ivalue = ivalue.toString()
    var ovalue = ivalue
    if (ovalue == '')
        return ovalue

    //if conversion is a routine. eg [NUMBER] [DATE] are standard
    var conversion = element.getAttribute('exodusconversion')
    if (typeof conversion != 'string' || conversion.slice(0, 1) != '[')
        conversion = false
    if (!conversion)
        return ovalue

    //format is '[functionname,options]'
    var convarray = conversion.slice(1, -1).split(',')
    //if no options, make them ''
    if (convarray.length == 1)
        convarray[1] = ''

    //convert internal to external format
    //nb cannot cancel after updating element.value
    //var quotechar = (ivalue.indexOf('"') >= 0) ? "'" : '"'
    //var expression = convarray[0] + '(' + '"OCONV",' + quotechar + ivalue.replace(/\\/g, '\\\\') + quotechar + ',"' + convarray[1] + '")'

    ivalue = ivalue.replace(/\\/g, '\\\\')
    ivalue = ivalue.replace(/\"/g, '\\"')
    ivalue = ivalue.replace(/[\x0D]/g, '\\r')
    ivalue = ivalue.replace(/[\x0A]/g, '\\n')
    var expression = convarray[0] + '(' + '"OCONV","' + ivalue + '","' + convarray[1] + '")'

    gmsg = ''
    var ovalue = yield* exodusevaluate(expression)
    if (typeof ovalue == 'undefined')
        return false

    //null means failed to convert to external value therefore invalid
    if (ovalue == null) {
        yield* exodusinvalid(exodusquote(ivalue) + ' is not a valid ' + convarray[0].toLowerCase() + '\n' + gmsg)
        return false
    }

    return ovalue

}

//given an array of field numbers calculate and set their contents
function* calcfields(fieldns) {

    //login('calcfields')

    if (gKeyNodes && !gloaded)
        return false //logout('calcfields no record')

    //if list is empty then do all calculated fields
    //exclude real fields (type=F) that have functioncode specified
    var origfieldns = fieldns
    if (fieldns == null) {
        fieldns = []
        var field
        for (var fn = 0; fn < gfields.length; fn++) {
            var field = gfields[fn]
            if (!field) {
                systemerror('yield* calcfields()', 'gfields[' + fn + '] is undefined.')
            }
            else {
                if (field.getAttribute('exodusfunctioncode') && field.getAttribute('exodustype') != 'F') {
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
            if (field.getAttribute('exodusfunctioncode')) {

                //add dependents of dependents to the list to recalc
                var deps = field.getAttribute('exodusdependents')
                if (deps) {
                    deps = deps.split(';')
                    for (var depn = 0; depn < deps.length; depn++) {
                        //prevent recursion by limiting to 1000 fields
                        if (fieldns.length < 1000)
                            fieldns[fieldns.length] = deps[depn]
                    }
                }

                //recalculate and update record
                yield* gds.regetx(field.id)

            }
        }
    }

    if (typeof origfieldns == 'undefined') gdependents = []

    //logout('calcfields')

}

//version to make grecn null and restore it afterwards
function* exodusevaluateall(functioncode, callerfunctionname) {
    var storegrecn = grecn
    grecn = null
    var result = yield* exodusevaluate(functioncode, callerfunctionname)
    grecn = storegrecn
    return result
}

function* exodusevaluate(functionorcode, callerfunctionname, arg1name, arg1, thisobject) {

    //wrapper to call custom functions or expressions (eg validation etc)
    //arg1name and arg1 are optional way of getting data into
    // arguments or variable listed in function source if functionorcode is code (text)

    //returns result of exodusevaluate
    //displays message if the function does not return something and returns ''
    //failure results in error message and returns undefined!!!
    // or if gstepping then dump to de-bugger

    var result

    if (typeof functionorcode == 'undefined') {
        systemerror('yield* exodusevaluate()', 'The required argument "functionorcode" is missing. Called from\n' + callerfunctionname)
        if (gstepping || gusername == 'EXODUS') crashhere
        return
    }

    //boolean just gets returned
    if (typeof functionorcode == 'boolean' || typeof functionorcode == 'number')
        return functionorcode

    if (typeof functionorcode == 'string') {

        //simple quoted values just get returned
        //(starting and ending with single or double quote
        //and no single or double quotes internally)
        if (functionorcode.match(/^(["'])[^'"]*\1$/))
            return functionorcode.slice(1, -1)

        //special code to return today's date
        if (functionorcode == 'TODAY')
            return exodusdate()

        if (functionorcode == '')
            return ''
    }

    //if (gstepping||(!ginitok&&gusername=='EXODUS'))
    // if (true||gstepping||gusername=='EXODUS')
    if (gstepping || gusername == 'EXODUS' || gusername == 'STEVE')
        result = yield* exodusevaluate3(functionorcode, null, arg1name, arg1, thisobject)
    else {

        //var e
        try {
            result = yield* exodusevaluate3(functionorcode, null, arg1name, arg1, thisobject)
        }
        catch (e) {
            //chrome exception is not available except inside catch clause
            //} if (e) {
            //if (typeof callerfunctionname == 'undefined') callerfunctionname = '"not specified"'
            systemerror('yield* exodusevaluate()' + functionorcode, e)
            return yield* exodusinvalid()
        }

    }

    //check that the function returned something
    //if (typeof result == 'undefined') {
    //    yield* exodusinvalid('Error in' + '\n' + functioncode.toString().substr(0, 500))
    //    systemerror('yield* exodusevaluate()', 'Function returned "undefined"\nCalled from ' + callerfunctionname + '\n\n' + functioncode.toString().substr(0, 500))
    //    result = ''
    //}

    return result

}

//var gcatcherrors=true//use try/catch to show errors to users on screen
var gcatcherrors = false//avoid try/catch thereby allowing javascript error line number etc to show in console and/or be caught by debugger

function* exodusevaluate3(functionorcode, functionname, arg1name, arg1, thisobject) {

    //arg1name and arg1 are 'event' and event in some use cases
    //in order to pass event into exodusonclick functions
    if (!arg1name)
        arg1name = 'dummyarg1name'
    if (!arg1)
        arg1 = null

    //straight function call
    if (typeof functionorcode == 'function')
        return functionorcode(arg1)

    var functioncode = functionorcode
    if (functioncode.indexOf('return ') < 0)
        functioncode = 'return ' + functioncode

    //yielding function code - wrap in a generator function
    if (functioncode.match && functioncode.match(gyieldregex)) {
        if (guseyield) {
            try {
                /* yield */ functioncode = 'return function *(){' + functioncode + '}'
                //arg1name can be the textual name of any variable or argumentname in the text of the function code
                functionx = new Function(arg1name, functioncode)
            }
            catch (e) {
                return systemerror('yield* exodusevaluate3()\nError creating function \n' + functioncode, e)
            }
            if (gcatcherrors) {
                try {

                    //we pass in the value of arg1 when calling the function
                    /* yield */var result = yield* functionx(arg1).apply(thisobject || this)
                    return result
                }
                catch (e) {
                    return systemerror('yield* exodusevaluate3()\n' + functioncode, e)
                }
                //commented out to enable showing of run time script errors
            } else {
                    //we pass in the value of arg1 when calling the function
                    /* yield */var result = yield* functionx(arg1).apply(thisobject || this)
                return result
            }

        } else
            functioncode = functioncode.replace(gyieldregex, '')
    }

    //non-yielding function code
    try {

        //arg1name can be the textual name of any variable in the text of the function code
        //TODO should only add return if no return in functioncode - to allow multi-line function code with returns in later lines
        functionx = new Function(arg1name, functioncode)
    } catch (e) {
        return systemerror('yield* exodusevaluate3()\n' + functioncode, e)
    }
    //we pass in the value of the argument when calling the function
    //return functionx.apply(this,arg1)
    return functionx.call(thisobject || this, arg1)
}

function* oconvertvalue(ivalue, conversion) {
    if (!conversion) return ivalue
    if (typeof (conversion) != 'string' || conversion.slice(0, 1) != '[') return ivalue
    return ivalue.exodusoconv(conversion)
}

function* deleterow_onclick(event) {
    return yield* form_deleterow(event)
}

function* insertrow_onclick(event) {
    return yield* form_insertrow(event)
}

//'''''''''''
//'DELETE ROW
//'''''''''''
function* form_deleterow(event, element) {

    //login('deleterow')

    event = getevent(event)

    if (!element)
        element = event.target

    //fast key repeats generates delete on the table element which can be ignored
    if (element.tagName == 'TABLE')
        return

    var row = getancestor(element, 'TR')
    var tablex = getancestor(row, 'TABLE')
    grows = tablex.tBodies[0].getElementsByTagName('tr')
    var groupno = Number(tablex.getAttribute('exodusgroupno'))

    if (groupno == 0)
        return false //logout('deleterow group 0')

    //does not work on mac ie5 ... var rown=row.sectionRowIndex
    var rown = row.rowIndex
    if (tablex.tHead)
        rown -= tablex.tHead.rows.length

    //make sure any previous data entered is valid EXCEPT if on the current line benig deleted
    if
        (gpreviouselement
        && (Number(gpreviouselement.getAttribute('exodusgroupno')) != groupno
            || getrecn(gpreviouselement) != rown)
        && !(yield* validateupdate())
    )
        return false //logout('deleterow gprevious invalid')

    //cannot delete if keyed and not locked
    if (gKeyNodes && !glocked) {
        yield* readonlydocmsg()
        return false //logout('deleterow')
    }

    //alert('rown='+rown)
    var nrows = grows.length
    //var grecn=row.recordNumber-1

    //zzz should be offset when paging
    grecn = rown

    //form specific before row delete function
    var predeleterow = window['form_predeleterow' + groupno]
    if (typeof predeleterow == 'function') {
        //if (!(yield* exodusevaluate('yield * form_predeleterow' + groupno + '()')))
        if (!(yield* predeleterow(event)))
            return false //logout('deleterow - predelete false')
    }

    //committed from here on

    //gpreviouselement = null
    //gpreviousvalue = ''
    setgpreviouselement(null)

    setchangesmade(true)
    //setdisabledandhidden(saverecord,false)
    //setdisabledandhidden(editreleaserecord,false)

    var pagesize = tablex.dataPagesize ? tablex.dataPagesize : 999999
    var pagen = exodusint(grecn / pagesize)

    var id
    if (event.target.name)
        id = event.target.id
    else
        id = gfields[gtables[groupno][0]].id

    gds.deleterow(groupno, rown)

    //focus on first column of new current row
    if (rown > 0 && rown == (nrows - 1)) rown--//if deleting last row then focus on previous row
    grows = tablex.tBodies[0].getElementsByTagName('tr')
    var element
    //rows[rown].getElementsByClassName('exodusid_'+id)
    if (document.getElementsByClassName)
        element = grows[rown].getElementsByClassName('exodusid_' + id)[0]
    else
        //using (id) instead of [id] because will return only one?
        element = grows[rown].all(id)
    focuson(element)
    //gpreviouselement = element
    //gpreviousvalue = getvalue(gpreviouselement)
    setgpreviouselement(element)

    //form specific after row delete function
    var postdeleterow = window['form_postdeleterow' + groupno]
    if (typeof postdeleterow == 'function') {
        yield* exodusevaluate('yield * form_postdeleterow' + groupno + '()')
    }

    var deps = tablex.getAttribute('exodusdependents')
    if (deps) {

        yield* calcfields(deps.split(';'))
    }

    //logout('deleterow')

}

//TODO merge deleterows and deleteallrows
function* deleterows(groupnoorelement, rowns) {

    //login('deleterows')

    var groupno = groupnoorelement
    if (typeof groupno == 'object') {
        groupno = groupno.getAttribute('exodusgroupno')
        if (!groupno && groupnoorelement[0] && groupnoorelement[0].getAttribute)
            groupno = groupnoorelement[0].getAttribute('exodusgroupno')
        if (!groupno && groupnoorelement.parentNode.getAttribute)
            groupno = groupnoorelementparenNode.getAttribute('exodusgroupno')
    }

    var rows = gds.data['group' + groupno]
    if (!rows) {
        systemerror('deleterows()', 'group number ' + groupnoorelement + ' doesnt exist')
    }

    for (var rownn = rowns.length - 1; rownn >= 0; --rownn) {
        var rown = rowns[rownn]
        //if deleting first row then a blank row (to clear formatting)
        if (rown == 0)
            gds.insertrow(groupno, 0)
        //never actually delete first row
        gds.deleterow(groupno, (rown == 0 ? 1 : rown))
    }

    //logout('deleterows')

}

function* deleteallrows(groupnoorelement, fromrecn) {

    //login('deleteallrows')

    var groupno = groupnoorelement
    if (typeof groupno == 'object') {
        groupno = groupno.getAttribute('exodusgroupno')
        if (!groupno && groupnoorelement[0] && groupnoorelement[0].getAttribute)
            groupno = groupnoorelement[0].getAttribute('exodusgroupno')
        if (!groupno && groupnoorelement.parentNode.getAttribute)
            groupno = groupnoorelementparenNode.getAttribute('exodusgroupno')
    }

    if (!fromrecn)
        fromrecn = 0

    if (!gds.data['group' + groupno]) {
        systemerror('yield* deleteallrows()', 'group number ' + groupnoorelement + ' doesnt exist')
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

    //logout('deleteallrows')

}

function* insertallrows(elements, values, fromrecn) {

	//preserve the basics while insertallrows2 is called
	var save_gpreviouselement = gpreviouselement
	var save_gvalue = gvalue
	var save_gpreviousvalue = gpreviousvalue
	var save_grecn = grecn

	var result = yield* insertallrows2(elements, values, fromrecn)

	//if (gdataset.split('_')[0] == 'gravity' || gdataset.split('_')[1] == 'test') {
		gpreviouselement = save_gpreviouselement
		gvalue = save_gvalue
		gpreviousvalue = save_gpreviousvalue
		if (!save_grecn)
			grecn = save_grecn
	//}

	return result
}

function* insertallrows2(elements, values, fromrecn) {

    //login('insertallrows')

    //get first element if an array passed
    //otherwise convert elements to an array
    //var element = elements
    if (!elements.tagName && elements.length && elements[0].tagName) {
        //element = element[0]
    }
    else
        elements = [elements]
    var element = elements[0]

    if (!fromrecn)
        fromrecn = 0

    assertelement(element, 'insertallrows', 'element')

    var groupno = Number(element.getAttribute('exodusgroupno'))

    //if (elements.length == 1)
    //yield* deleteallrows(element, fromrecn)
    yield* deleteallrows(element, fromrecn + 1)

    //get the group
    var rows = gds.data['group' + groupno]

    //cater for passing one row/one cell value not in array form
    if (typeof values != 'object')
        values = [values]

    for (var rown = 0; rown < values.length; rown++) {

        //insert a new row (except for the last)
        if (rown > 0 && rown < (values.length - 1))
            gds.insertrow(groupno, rown + fromrecn)

        var rowvalues = values[rown]
        if (typeof rowvalues != 'object')
            rowvalues = [rowvalues]

        //set the record and form values
        for (var coln = 0; coln < elements.length && coln < rowvalues.length; coln++) {
            //yield* gds.setx(elements[coln], rown + fromrecn, rowvalues[coln])

            var element = elements[coln]
            var oldvalue = yield* gds.get1(element, rown + fromrecn)
            var newvalue = rowvalues[coln]

            //set and call validation to trigger any consequences
            if (newvalue != oldvalue) {
                grecn = rown + fromrecn
                gpreviousvalue = oldvalue
                //gpreviouselement = document.getElementsByName(element.id)[grecn]
                //if (!gpreviouselement)
                //  gpreviouselement = element
                if (element.id) {
                    gpreviouselement = $$(element.id)
                    if (gpreviouselement[grecn])
                        gpreviouselement = gpreviouselement[grecn]
                } 
                //yield* gds.setx(element, grecn, newvalue)
                var ovalue = yield* validateoconv(gpreviouselement, newvalue)
                if (typeof ovalue == 'undefined' || ovalue == null) {
                    return false                                       
                }
                //const conversion = gpreviouselement.getAttribute('exodusconversion')
                //if (conversion && conversion.substr(0,1) == '[')
                //    newvalue = newvalue.exodusoconv(conversion)
                setvalue(gpreviouselement,ovalue)
                if ((!gKeyNodes || glocked) && !(yield* validateupdate()))
                    return false
                gpreviousvalue = newvalue
            }
        }

    }

    //no because often done programmatically which requires no specific work by user
    //popup etc may set it specifically
    //setchangesmade(true)

    //recalculate any dependents
    var dependentfieldnos = ''
    for (var ii = 0; ii < elements.length; ii++) {
        var deps = elements[ii].getAttribute('exodusdependents')
        if (deps)
            dependentfieldnos += ';' + deps
    }
    dependentfieldnos = dependentfieldnos.slice(1).split(';')
    grecn = null

    yield* calcfields(dependentfieldnos)

    //logout('insertallrows')

}

function exodusaddrow(groupno) {
    var tablex = $$('exodusgroup' + groupno)
    if (!tablex || tablex.getAttribute('noinsertrow'))
        return false
    gds.addrow(groupno)
}

function* readonlydocmsg() {

    //restore any previous value
    if (gpreviouselement && gvalue != gpreviousvalue)
        setvalue(gpreviouselement, gpreviousvalue)

    //readonly after key has been entered
    if (gKeyNodes || gkey)
        return yield* exodusinvalid('This document is currently "read only"')

    //readonly before key has been entered
    else if (!gkey)
        return yield* exodusinvalid('Please open a document first')
}

//'''''''''''
//'INSERT ROW
//'''''''''''
function* form_insertrow(event, append) {

    event = getevent(event)
    if (event.target.tagName == 'BODY')
        return

    //make sure any previous stuff is validatedupdated
    if (!(yield* validateupdate(event)))
        return false

    //login('insertrow')

    //setdisabledandhidden(saverecord,false)
    //setdisabledandhidden(editreleaserecord,false)

    var row = getancestor(event.target, 'TR')
    if (!row)
        systemerror('insertrow', event.target.id + ' has no tr ancestor')

    var tablex = getancestor(row, 'TABLE')
    if (!tablex || tablex.getAttribute('noinsertrow'))
        return false

    //var tbodies = tablex.tBodies
    var groupno = Number(tablex.getAttribute('exodusgroupno'))
    if (groupno == 0)
        return false

    var rown = row.rowIndex
    if (tablex.tHead) rown -= tablex.tHead.rows.length
    if (append)
        rown++
    grecn = rown

    grows = tablex.tBodies[0].getElementsByTagName('tr')
    var nrows = grows.length

    //first and only row is considered to be clicking on last row, ie appends
    //but how would we insert a line before line 1?
    //if (nrows==1)
    //     append=true

    //option to insert AFTER if clicking the one row or clicking the penultimate row of many
    if (typeof append == 'undefined') {
        //clicking on the final row of multiple rows always adds AFTER/BELOW since most common and natural
        if (nrows > 1 && rown == nrows - 1)
            grecn++
        //clicking a single row OR clicking the penultimate row
        else if (nrows == 1 || rown == nrows - 2) {
            var defaultchoice = (nrows == 1) ? 2 : 1
            var choice = yield* exodusconfirm('Insert row before or after?', 1, 'Before', 'After', 'Cancel')
            if (!choice)
                return false
            if (choice == 2)
                grecn++//AFTER/BELOW
        }
    }

    //if following row is hidden then expand following hidden rows INSTEAD of inserting
    if (grecn < (nrows - 1) && grows[grecn + 1].style.display == 'none') {

        //return to insertrow image
        setinsertimage('insert', grows[grecn], groupno)

        for (rown = grecn + 1; rown < nrows; rown++) {
            if (grows[rown].style.display !== 'none') return true
            //grows[rown].style.display=''
            grows[rown].style.display = ''
        }
        return true
    }

    //cannot update if (locked
    if (gKeyNodes && !glocked) {
        yield* readonlydocmsg()
        return false //logout('insertrow')
    }

    //form specific before row insert function
    var preinsertrow = window['form_preinsertrow' + groupno]
    if (typeof preinsertrow == 'function') {
        if (!(yield* exodusevaluate('yield * form_preinsertrow' + groupno + '()')))
            return false //logout('insertrow - preinsert false')
    }

    var id
    if (event.target.name)
        id = event.target.id
    else
        id = gfields[gtables[groupno][0]].id

    if (append) {
        gds.addrow(groupno)
    }
    else {
        gds.insertrow(groupno, grecn)
        //inserting a row sets gchangesmade, but appending does not
        setchangesmade(true)
    }

    //get new row again
    var row = tablex.tBodies[0].getElementsByTagName('tr')[grecn]

    //ensure not copied a row with expand marker
    setinsertimage('insert', row, groupno)

    //form specific after row insert function
    var postinsertrow = window['form_postinsertrow' + groupno]
    if (typeof postinsertrow == 'function')
        yield* exodusevaluate('yield * form_postinsertrow' + groupno + '()')

    //focus on first column of new row (after running postinsert routine to avoid event within event - if postinsert yields for some user input for example)
    if (!append) {
        if (document.getElementsByClassName)
            focuson(row.getElementsByClassName('exodusid_' + id)[0])
        else
            focuson(row.all[id])
    }

    //logout('insertrow')

    return true

}

function setinsertimage(mode, row, groupno) {

    //return to insertrow graphic zzz hardcoded should be located
    //var insertimage=row.childNodes[0].childNodes[0].childNodes[0]
    //var insertimage=row.childNodes[0].exodusfields['insertrowbutton'+groupno]
    var insertimage = row.exodusfields
    if (!insertimage)
        return
    insertimage = insertimage['insertrowbutton' + groupno]
    if (!insertimage)
        return

    if (mode == 'expand') {
        insertimage.src = gexpandrowimage
        //duplicate keycodes in 3 places
        insertimage.title = 'Expand hidden rows here (Ctrl+I or Ctrl+Insert)'
    }
    else {
        insertimage.src = ginsertrowimage
        //duplicate keycodes in 3 places
        insertimage.title = 'Insert a new row here (Ctrl+I or Ctrl+Insert)'
    }
    return
}

function* openrecord_onclick() {

    //login('openrecord')

    if (!(yield* validateupdate()))
        return false

    var reply
    //get a reply or return false
    if ((reply = yield* exoduspopup2(openrecord)) == null)
        return false //logout('openrecord')

    //forget it if no change
    if (reply == (yield* getkey()))
        return false //logout('openrecord no change')

    //do not change key if user chooses not to unload an existing document
    if (!(yield* closedoc('OPEN'))) {
        //logout('openrecord user cancelled')
        return false
    }

    yield* opendoc(reply)

    //logout('openrecord')

    return true

}

function* firstrecord_onclick(event) {
    return yield* nextrecord2(event, 'first')
}

function* previousrecord_onclick(event) {
    return yield* nextrecord2(event, -1)
}

function* selectrecord_onclick(event) {
    return yield* nextrecord2(event, 0)
}

function* nextrecord_onclick(event) {
    return yield* nextrecord2(event, 1)
}

function* lastrecord_onclick(event) {
    return yield* nextrecord2(event, 'last')
}

function* nextrecord2(event, direction) {

    //direction is 'first', -1, 0, 1, 'last'

    var nextkeys = gkeys
    var nextkeyn = gkeyn

    //goto one of many ... or if select many then reduce list to those selected
    if (direction == 0) {
        var selkeys = yield* exodusdecide('', gkeys, [[0, 'Key']], 0, '', many = true)
        if (!selkeys) return false
        if (selkeys.length > 1) {
            nextkeys = selkeys
            nextkeyn = 1
        }
        else {
            nextkeyn = nextkeys.exoduslocate(selkeys[0]) - 1
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
        if (!(yield* closedoc('OPEN')))
            return false
        yield* setgkeyn(nextkeyn)
        yield* opendoc(nextkey)
    }

    yield* setgkeys(nextkeys, nextkeyn)

    return true

}

function* exoduslink(event, element) {

    event = getevent(event)

    //login('exoduslink')

    exoduscancelevent(event)

    //search next then previous siblings for popup
    //if (element.type!='text') element=element.previousSibling
    //if (element.type!='text') element=element.previousSibling
    if (!element)
        element = event.target
    while (element && element.getAttribute && !element.getAttribute('exoduslink')) {
        element = element.nextSibling
    }
    if (!element || !element.getAttribute || !element.getAttribute('exoduslink')) {
        var element = event.target
        while (element && (!element.getAttribute || !element.getAttribute('exoduslink'))) {
            element = element.previousSibling
        }
    }

    //quit if no link defined
    if (!element || !element.getAttribute('exoduslink'))
        return false //logout('exoduslink - no link')

    //prevent popups except on the key field unless a record is present
    if (gKeyNodes && !gloaded && element.getAttribute('exodusfieldno') != 0) {
        focuson(gKeyNodes[0])
        return false //logout('exoduslink - no record')
    }

    grecn = getrecn(element)

    gvalue = getvalue(element, grecn)
    if (!(element.getAttribute('exoduslowercase')))
        gvalue = gvalue.toUpperCase()

    var reply = yield* exodusevaluate(element.getAttribute('exoduslink'), 'yield* exoduslink()')

    //logout('exoduslink')

    return

}

function* exoduspopup(event, element) {

    event = getevent(event)

    //element is only provided from f7/alt+down keyboard events

    //login('exoduspopup')

    exoduscancelevent(event)

    //search current and following siblings for popup
    if (!element) {
        element = event.target
        while (element
            && (!element.getAttribute || !element.getAttribute('exoduspopup'))
            && element.tagName != 'SELECT') {
            element = element.nextSibling
        }
        if (!element) {
            var elements = event.target.parentNode.getElementsByTagName('*')
            for (var elementn = 0; elementn < elements.length; ++elementn) {
                element = elements[elementn]
                if (element.getAttribute && element.getAttribute('exoduspopup'))
                    break
            }
        }
    }

    //quit if no element
    if (!element)
        return false //logout('exoduspopup - no element')

    //log('check no missing data in group 0 always')
    if (!(yield* checkrequired(gfields, element, 0)))
        return false //logout('exoduspopup' + ' ' + element.id + ' a prior element is visible and required but is blank (1)')

    //validateupdate previous field
    //if (element.id!=gpreviouselement.id)
    if (element != gpreviouselement) {
        if (!(yield* validateupdate()))
            return false //logout('exoduspopup - validateupdate failed')
    }

    //cannot update anything but key field if not locked or save button not enabled
    if (element.getAttribute('exodusfieldno') != 0 && gKeyNodes && (!glocked || saverecord.getAttribute('disabled'))) {
        yield* readonlydocmsg()
        return false //logout('exoduspopup - read only document')
    }

    //quit if readonly
    var readonly = element.getAttribute('exodusreadonly')
    if (readonly) {
        if (readonly != 'true')
            yield* exodusinvalid(element.getAttribute('exodusreadonly'))
        return false //logout('exoduspopup - read only')
    }

    //quit if no popup defined
    if (!element.getAttribute('exoduspopup') && element.tagName != 'SELECT')
        return false //logout('exoduspopup - no popup')

    //prevent popups except on the key field unless a record is present
    if (gKeyNodes && !gloaded && element.getAttribute('exodusfieldno') != 0) {
        focuson(gKeyNodes[0])
        return false //logout('exoduspopup - no record')
    }

    grecn = getrecn(element)
    gvalue = getvalue(element)
    //gpreviouselement = element
    //gpreviousvalue = gvalue
    setgpreviouselement(element, gvalue)

    //get a reply or return false
    /////////////////////////////
    if ((reply = yield* exoduspopup2(element)) == null) {
        //    alert('xxx')
        focuson(element)
        return false //logout('exoduspopup - no reply')
    }

    //update the element multiple selections qqq
    //if reply is an array use replace all rows
    if (reply.length > 1
        && Number(element.getAttribute('exodusgroupno'))
        && typeof (reply) == 'object') {
        //var elementid = element.id

        var elementid = element.id

        //deselect items already selected
        if (grecn > 0) {
            var prevalues = ((yield* gds.getall(elementid))).slice(0, grecn)
            for (var ii = 0; ii < prevalues.length; ++ii) {
                var replyn
                if (replyn = reply.exoduslocate(prevalues[ii]))
                    reply.splice(replyn - 1, 1)
            }
            if (!reply.length)
                return false //logout('exoduspopup - no new selections')
        }

        yield* insertallrows(element, reply, grecn)
        setchangesmade(true)//should this be done in insertallrows to ensure Save button is enabled in other cases too?

        //focus on next element AFTER table
        element = $$(elementid)
        if (element && element[0])
            element = element[0]
        focusdirection(1, element, Number(element.getAttribute('exodusgroupno')))

        //logout('exoduspopup - insert all rows')
        return true

    }

    //if reply is still an array use the first one
    if (typeof (reply) == 'object')
        reply = reply[0]

    //forget it if no change (changed to allow early update on popup eg pop_target)
    //if (reply==getvalue(element))
    if (reply == gpreviousvalue) {
        focusnext()
        return false //logout('exoduspopup - no change')
    }

    //do not change key if user chooses not to unload an existing document
    if (element.getAttribute('exodusfieldno') == 0 && gloaded && !(yield* closedoc('OPEN')))
        return false //logout('exoduspopup - user cancelled unloaddoc')

    //output convert it
    if (element.getAttribute('exodusconversion')) {
        reply = yield* validateoconv(element, reply, reply)
        if (typeof reply == 'undefined' || reply == null) {
            return false //logout('exoduspopup - oconv failed')
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
    if (yield* validateupdate())
        focusnext(element)
    gvalidatingpopup = false
    //exodussettimeout('focusnext()',10)
    //yield* validateupdate()

    //logout('exoduspopup')

}

function* exoduspopup2(element) {

    //given a SELECT item or element with a popupfunction to evaluate, returns a reply or null

    //quit if no popup defined and not SELECT
    var expression = element.getAttribute('exoduspopup')
    if (!expression && element.tagName != 'SELECT')
        return false

    //evaluate popup expression if provided
    if (expression) {
        var reply = yield* exodusevaluate(expression, 'yield* exoduspopup2()')
    }

    //otherwise build a list and select from the SELECT
    else {

        //build a popup from the SELECT tag
        var selectvalues = []
        for (var ii = 0; ii < element.childNodes.length; ii++) {
            var option = element.childNodes[ii]
            var optionvalue = option.value
            if (!optionvalue)
                optionvalue = ''
            var optiontext = option.innerText
            if (!optiontext)
                optiontext = ''
            selectvalues[selectvalues.length] = [optionvalue, optiontext]
        }

        //allow multiple selection if element is in a group with only one input column
        var multipleselection = false
        var elgroupno = Number(element.getAttribute('exodusgroupno'))
        if (elgroupno) {
            var tableelement = gtables[elgroupno].tableelement
            var multipleselection = tableelement && tableelement.getAttribute('exoduslastinputcolscreenfn') == tableelement.getAttribute('exodusfirstinputcolscreenfn')
        }

        //get the response(s)
        reply = yield* exodusdecide2('', selectvalues, '1', '', '', multipleselection)

        //have to do this to cancel the standard dropdown if they press alt+down then press escape on the popup
        if (!reply)
            focuson(element)

        //convert an array of row number(s) to an array of value(s)
        for (var ii = 0; ii < reply.length; ii++) {
            reply[ii] = selectvalues[reply[ii] - 1][0]
        }

        //remove empty values
        reply = reply.exodustrim('')

        //minimum one value
        if (!reply.length)
            reply = ['']

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
            yield* calcfields(gdependents)
        }
    }

    //if sole key field or the open function, setup a list of keys
    if (reply && ((openrecord && element == openrecord) || (element.getAttribute('exodusfieldno') == 0 && gKeyNodes.length == 1))) {
        if (typeof reply == 'object') {

            if (reply.length > 1 && reply.length <= 50) {
                var openall = 2
                openall = yield* exodusconfirm('Open all in one tab?', 1, 'One', 'Many')
                if (!openall)
                    return false
                if (openall == 2) {
                    //open 2nd and subsequent keys in tabs
                    for (var keyn = 1; keyn <= reply.length; ++keyn) {
                        var key = reply[keyn]
                        if (key)
                            //doesnt work when multiple .. due to using cookies to communicate? perhaps chain opening passing remaining keys to each window
                            //yield* windowopenkey(window.location.href.toString(), key)
                            yield* windowopen(window.location.href.toString().split('?')[0] + '?key=' + key)
                    }
                    //window.location.assign(window.location.href.toString().split('?')[0] + '?key=' + reply[0])
                    reply = [reply[0]]//return the first key only
                }
            }

            yield* setgkeys(reply, 0)
            reply = reply[0]
        }
        else {
            yield* setgkeys([reply], 0)
        }
    }

    return reply

}

function* setgkeys(keys, keyn) {
    if (typeof keys == 'string') {
        keys = [keys]
    }
    gkeys = keys
    var tt = gkeys.length > 1 ? "" : "none"
    firstrecord.style.display = tt
    previousrecord.style.display = tt
    selectrecord.style.display = tt
    nextrecord.style.display = tt
    lastrecord.style.display = tt
    if (typeof keyn == 'undefined') {
        for (keyn = 0; keyn < gkeys.length; ++keyn) {
            if (keys[keyn] == gkey) {
                break
            }
        }
    }
    if (keyn < gkeys.length) {
        yield* setgkeyn(keyn)
    }
}

function* setgkeyn(keyn) {
    gkeyn = keyn
    firstrecord.disabled = gkeyn == 0 ? true : false
    selectrecord.innerHTML = (gkeyn + 1) + " of " + gkeys.length
    lastrecord.disabled = (gkeyn == (gkeys.length - 1)) ? true : false
}

function* getkeyexternal() {
    return yield* getkey('oconv')
}

function* debug(v) {
    if (!(confirm(v))) {
        yield* exodusbreak()
    }
}

function* getkey(mode) {

    //login('getkey ' + mode)

    //returns a string containing the key or '' if any key parts are required and ''
    var key = new Array(gKeyNodes.length)
    for (var ii = 0; ii < gKeyNodes.length; ii++) {
        var temp = getvalue(gKeyNodes[ii])
        if (temp == '' && gKeyNodes[ii].getAttribute('exodusrequired') != false) {
            //logout('getkey ""')
            return ''
        }
        if (mode != 'oconv') {
            var conversion = gKeyNodes[ii].getAttribute('exodusconversion')
            if (conversion.slice(0, 1) == '[') temp = exodusiconv(temp, conversion)
        }
        key[gKeyNodes[ii].getAttribute('exoduskeypart') - 1] = temp
    }

    var key
    if (mode != 'oconv') {
        key = key.join('*')
    }
    else {
        key = key.join(' ')
    }
    //yield* debug(key)

    //logout('getkey ' + key)

    return key

}

function* setkeyvalues(key) {
    //given a string updates the keyfield(s)
    for (var ii = 0; ii < gKeyNodes.length; ii++) {
        var temp = key.exodusfield('*', Number(gKeyNodes[ii].getAttribute('exoduskeypart')))
        //var conversion=gKeyNodes[ii].getAttribute('exodusconversion')
        //if (conversion.slice(0,1)=='[') temp=temp.exodusoconv(conversion)
        //setvalue(gKeyNodes[ii],temp)
        yield* gds.setx(gKeyNodes[ii].id, '', temp)
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
        //global variable set in any getevent(event)
        element = gevent.target
    }
    if (typeof element != 'object') {
        systemerror('getrecn', element[0].id + ' ' + element[0].tagName + ' ' + element[0].innerHTML + ' is not an object')
    }

    exodusassertobject(element, 'getrecn', 'element')
    //returns null if element not part of a table

    var recn
    if (element.getAttribute && !(Number(element.getAttribute('exodusgroupno')))) {
        recn = null
    }
    else {
        var rowx = getancestor(element, 'TR')
        if (!rowx) {
            return null
        }
        var tablex = getancestor(rowx, 'TABLE')
        recn = rowx.rowIndex
        if (tablex && tablex.tHead) {
            recn -= tablex.tHead.rows.length
        }
    }

    //partially solve a bug that deleteallrows deletes our row and leaves us unable to find recn from its rowIndex
    //since rowIndex of a deleted row (removed from DOM) is -1
    if (recn < 0) {
        //var msg='getrecn'+recn
        //if (rowx)
        //    msg+=' rowx.rowIndex'+rowx.rowIndex
        //    msg+=' ' + row.innerHTML
        //if (tablex)
        //    msg+=' tablex.tHead.rows.length'+tablex.tHead.rows.length
        //alert(msg)
        recn = 0
    }

    return recn

}

function* getnextrow(dictid, skipblanks, iconv) {
    return yield* getrowx(dictid, skipblanks, true, iconv)
}

function* getpreviousrow(dictid, skipblanks, iconv) {
    return yield* getrowx(dictid, skipblanks, false, iconv)
}

function* getrowx(dictid, skipblanks, forward, iconv) {

    //given a dictionary id (or use gpreviouselement) and an option to skip over blanks,
    //return the contents of the previous row to the current row determined by grecn
    //forward true means search forwards not backwards

    if (!dictid)
        dictid = gpreviouselement.id
    var value = ''

    //quick previous search
    if (!forward) {
        if (grecn == 0)
            return ''
        if (iconv)
            value = yield* gds.get1(dictid, grecn - 1)
        else
            value = getvalue(dictid, grecn - 1)
        if (value)
            return value
        if (!skipblanks)
            return ''
    }

    //more comprehensive search
    // var values=getvalue(dictid, null)
    //var values=getvalues(dictid)
    var values
    if (iconv)
        values = yield* gds.getall(dictid)
    else
        //values = getvalues(document.getElementsByName(dictid))
        values = getvalues(dictid)

    //return undefined if no next row
    if (forward) {
        var ln = grecn + 1
        var ln2 = skipblanks ? values.length : ln + 1
        for (ln = ln; ln <= ln2; ln++) {
            if ((value = values[ln]) != '')
                return value
        }
    }
    else {
        var ln = grecn - 1
        var ln2 = skipblanks ? 0 : ln
        for (ln = ln; ln >= ln2 && ln >= 0; ln--) {
            if ((value = values[ln]) != '')
                return value
        }
    }
    return ''

}

function* form_ondblclick(event) {
    event = getevent(event)
    return yield* form_filter('filter')
}

function* form_onrightclick(event) {
    event = getevent(event)
    return true
}

function* form_filter(mode, colidorgroupno, regexp, maxrecn, elem) {

    //NB regexp to be filtered OUT not IN

    if (mode == 'refilter') {
        colidorgroupno2 = colidorgroupno
        yield* form_filter('unfilter', colidorgroupno2)
        mode = 'filter'
    }

    if (mode == 'filterfocus') {
        //following only applies to schedules! convert to any form
        if (!(yield* gds.get1('VEHICLE_CODE', 0))) {
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
        colid = gevent.target.id
        if (!colid) {
            var element = gevent.target.parentNode
            while (element) {
                colid = element.name
                if (colid)
                    break
                element = element.parentNode
            }
        }
    }

    //get dictitem
    //if (mode == 'filter' || mode == 'unfilter') {
    //}

    //get groupno
    var groupno
    if (typeof colidorgroupno != 'number' && (mode == 'filter' || mode == 'unfilter')) {
        var dictitem = gds.dictitem(colid)
        //if (!dictitem)
        // return yield* exodusinvalid(colid+' dictitem does not exist in yield* form_filter()')
        groupno = dictitem.groupno
        if (!groupno)
            return yield* exodusinvalid()//colid+' is not multivalued for sorting'
    }
    else {
        groupno = colidorgroupno
    }

    //ensure the function name is recognised by the yield converter
    //function* prefilter(){}

    //prefilter
    if (typeof form_prefilter == 'function') {
        //yield* exodusevaluate('form_prefilter()','yield* formfunctions_onload()')
        if (!(form_prefilter(mode, colid)))
            return yield* exodusinvalid()
    }

    //get the table rows
    var tablex = $$('exodusgroup' + groupno)
    if (!tablex)
        return yield* exodusinvalid(colid + ' is not part of a table')
    grows = tablex.tBodies[0].rows

    //unfilter and exit (not tested or used anywhere)
    if (mode == 'expand') {
        for (var rown = grecn + 1; rown < grows.length; rown++) {
            if (grows[rown].style.display != '')
                break
            //grows[rown].style.display=''
            grows[rown].style.display = ''
        }
        yield* calcfields()
        return true
    }

    //get the show all button
    var tablexshowall = $$('exodusgroup' + groupno + 'showall')
    var tablexfilter = $$('exodusgroup' + groupno + 'filter')
    if (!tablexshowall) {
        //syserror('yield* form_filter()','Cannot find showall button, are you missing a thead?')
        return true
    }

    //unfilter and exit
    if (mode == 'unfilter') {
        for (var rown = 0; rown < grows.length; rown++) {
            //change to expandrow image
            setinsertimage('insert', grows[rown], groupno)

            //grows[rown].style.display=''
            grows[rown].style.display = ''
        }
        //hide the show all buttons (can be two - in THEAD and TFOOT)
        //tablexshowall.style.display='none'
        showhide('exodusgroup' + groupno + 'showall', false)
        if (typeof tablexfilter != 'undefined' && tablexfilter)
            tablexfilter.size = 3
        yield* calcfields()
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
        filtervalues = document.getElementById('exodusgroup' + groupno + 'filter').value.split(' ')
        for (var ii = 0; ii < filtervalues.length; ++ii) {
            filtervalues[ii] = filtervalues[ii].replace(/([\\,\^,\$,\*,\+,\?,\.,\(,\),\|,\{,\},\[,\]])/g, "\\$1")//convert metacharacters to real characters
            filtervalues[ii] = new RegExp(filtervalues[ii], 'gi')
        }

    }
    else {
        values = yield* gds.regetx(colid, null)
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

        //grows[rown].style.display=(values[rown]==value)?'':'none'
        var hiderow = false
        if (grows[rown].style.display == 'none') {
            continue
        }
        else if (regexp) {
            if (values[rown].match(regexp)) hiderow = true
            //console.log(hiderow+' '+values[rown]+' '+regexp)
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
                if (grows[rown].innerText.search(filtervalue) >= 0)
                    hiderow = false
                else {
                    var inputs = grows[rown].getElementsByTagName('input')
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

        }
        else if (values[rown] != value)
            hiderow = true

        if (hiderow) {
            hiderowns.push(rown)
            if (lastunhiddenrow) {
                lastunhiddenrows.push(lastunhiddenrow)
                lastunhiddenrow = ''
            }
        }
        else {
            var lastunhiddenrow = grows[rown]
            anyunhiddenrows = true
        }

    }

    if (!anyunhiddenrows && !maxrecn) {
        //yield* form_filter('unfilter', colidorgroupno)
    }
    else {
        for (rownn = 0; rownn < hiderowns.length; ++rownn) {
            var rown = hiderowns[rownn]
            grows[rown].style.display = 'none'
            if (tablexshowall)
                tablexshowall.style.display = ''
        }
        //mark last unhidden row as expand image
        for (var rown = 0; rown < lastunhiddenrows.length; ++rown)
            //change to expandrow image
            setinsertimage('expand', lastunhiddenrows[rown], groupno)
        yield* calcfields()
    }

    window.status = ''

    return true

}

var calendar_checkInDatePicker

function* form_pop_calendar() {
    //do this so that it pops up after focussing on the entry element
    exodussettimeout('yield* form_popcalendar2()', 100)
    return false
}

function* form_popcalendar2() {

    //remove any previous calendar
    //if (calendar_checkInDatePicker) calendar_checkInDatePicker.hide()

    var datevalue = gvalue.toString().exodusiconv('[DATE]')
    if (datevalue) {
        var msdate = new Date()
        msdate.setDate(Number(datevalue.exodusoconv('[DATE,DOM]')))
        msdate.setMonth(Number(datevalue.exodusoconv('[DATE,MONTH]') - 1))
        msdate.setFullYear(Number(datevalue.exodusoconv('[DATE,YEAR]')))
        calendar_checkInDatePicker = new Calendar(msdate);
    }
    else {
        calendar_checkInDatePicker = new Calendar();
    }
    /*
    calendar_checkInDatePicker.setMonthNames(new Array("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"));
    calendar_checkInDatePicker.setShortMonthNames(new Array("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"));
    calendar_checkInDatePicker.setWeekDayNames(new Array("Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"));
    calendar_checkInDatePicker.setShortWeekDayNames(new Array("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"));
    */

    //calendar_checkInDatePicker.setFormat("d/M/yyyy");
    //calendar_checkInDatePicker.setFirstDayOfWeek(6);
    calendar_checkInDatePicker.setFormat(gdateformat);
    calendar_checkInDatePicker.setFirstDayOfWeek(Number(gfirstdayofweek));
    calendar_checkInDatePicker.setMinimalDaysInFirstWeek(1);
    calendar_checkInDatePicker.setIncludeWeek(false);

    calendar_checkInDatePicker.create();

    function calendar_checkInDatePicker_onchange() {

        if (!calendar_checkInDatePicker)
            return true

        setvalue(gpreviouselement, calendar_checkInDatePicker.formatDate())

        //clear the object
        exodussettimeout('calendar_checkInDatePicker=null', 1)

        focusnext(gpreviouselement)
        return true
    }
    //dont use addeventlistener here because onchange is special to DatePicker
    calendar_checkInDatePicker.onchange = calendar_checkInDatePicker_onchange

    calendar_checkInDatePicker.show(gpreviouselement)
    calendar_checkInDatePicker._calDiv.focus()
    //calendar_checkInDatePicker._calDiv.setActive()
    //NB activeElement not available everywhere
    //xyz=exodussetinterval('window.status=document.activeElement.outerHTML',10)
    return false

}

//drop down any "modal" popup divs
//return true if any closed or false if none
function* form_closepopups() {

    var anyclosed = false

    if (calendar_checkInDatePicker) {
        try {
            calendar_checkInDatePicker.hide()
        }
        catch (e) {
            if (gusername == 'EXODUS')
                yield* exodusnote('couldnt drop calendar\n' + e.description)
        }
        calendar_checkInDatePicker = ''
        anyclosed = true

    }

    return anyclosed

}

function* form_val_index(filename, fieldname, allownew) {

    //check if an existing

    if (!filename)
        filename = gdatafilename
    if (!fieldname)
        fieldname = gpreviouselement.name

    //get the existing values
    var indexvalues = yield* form_get_index(filename, fieldname)
    if (!indexvalues)
        return true
    indexvalues = indexvalues.group1

    //ok if is an existing indexvalue (Exact Case)
    for (var ii = 0; ii < indexvalues.length; ii++) {
        if (indexvalues[ii][fieldname].text == gvalue)
            return true
    }

    //optionally change case
    for (var ii = 0; ii < indexvalues.length; ii++) {
        if (indexvalues[ii][fieldname].text.toUpperCase() == gvalue.toUpperCase()) {
            if (!(reply = yield* exodusconfirm('Change the capitalisation of your entry?\n\nFrom: ' + gvalue.exodusquote() + '\n\n  To: ' + indexvalues[ii][fieldname].text.exodusquote(), 1)))
                return yield* exodusinvalid()
            if (reply == 1)
                gvalue = indexvalues[ii][fieldname].text
            return true
        }
    }

    //option to allow/prevent new index values
    if (!allownew)
        return yield* exodusinvalid(gvalue + ' ' + fieldname.toLowerCase() + ' does not exist')

    //confirm is new index value
    if (gvalue && ((yield* exodusyesno(' Is ' + exodusquote(gvalue) + ' to be a new ' + fieldname.toLowerCase() + ' for ' + filename.toLowerCase() + ' ?', 1)) != 1))
        return false

    return true

}

function* form_get_index(filename, fieldname) {

    db.request = 'CACHE\rGETINDEXVALUESXML\r' + filename + '\r' + fieldname
    if (!(yield* db.send())) {
        yield* exodusinvalid(db.response)
        return ''
    }

    if (!db.data || db.data == '<records></records>')
        return ''

    return exodusxml2obj(db.data)

}

function* form_pop_index(filename, fieldname, many) {

    //provides a simple way to select previously entered values on an indexed field

    if (!filename)
        filename = gdatafilename
    if (!fieldname)
        fieldname = gpreviouselement.name

    //get index values
    var indexvalues = yield* form_get_index(filename, fieldname)
    if (!indexvalues || indexvalues.group1.length == 0)
        return yield* exodusinvalid('No ' + fieldname.toLowerCase().exodusconvert('_', ' ') + '(s) have been entered on ' + filename.toLowerCase().exodusconvert('._', '  ') + ' file yet')

    //user selects index value(s)
    var result = yield* exodusdecide2('', indexvalues, [[fieldname, fieldname.exoduscapitalise()]], 0, '', many)

    return result

}

function* copyrecord_onclick() {

    if (!gkey || !glastkey || !gds.isnewrecord || gchangesmade)
        return yield* exodusinvalid('To copy a record you must:\n\n1. Open the record to copy\n2. Start a new record&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n3. Click the Copy button&nbsp;&nbsp;&nbsp;&nbsp;')

    //read the record to be copied
    var copyrecord = []
    if (!(yield* copyrecord.exodusread(gdatafilename, glastkey)))
        return yield* exodusinvalid(copyrecord.exodusresponse)

    //remove any uncopyable data
    for (var dictname in gds.dict) {
        var dictitem = gds.dictitem(dictname)
        if (dictitem && dictitem.type && dictitem.type == 'F' && typeof dictitem.copyable != 'undefined' && !dictitem.copyable)
            copyrecord[dictitem.fieldno] = ''
    }

    //validate copy
    //if (!(yield* exodusevaluate('yield* form_copyrecord()','yield* copyrecord_onclick()')))
    if (!(yield* form_copyrecord(copyrecord)))
        return yield* exodusinvalid()

    //load the new record
    gro.revstr = copyrecord.slice(1).join(fm)
    gro.rev2obj()
    yield* gds.load(gro)

    return true

}

function copydictitem(dictitem, element) {

    for (var propertyname in dictitem) {
        if (typeof element['exodus' + propertyname] == 'undefined' && typeof dictitem[propertyname] != 'undefined') {
            //element['exodus'+propertyname]=dictitem[propertyname]
            //use setAttribute because only msie will clone expando properties and needed for row cloning
            //store false as "" otherwise since attributed are stored as strings it becomes "false"
            //which does not evaluate to false
            var value = dictitem[propertyname]
            if (value === false || value === null)
                value = ''
            else
                element.setAttribute('exodus' + propertyname, value)
        }
    }

    element.name = dictitem.name

    //will be setup later
    //element.id=dictitem.name
    ////element.name=dictitem.name

    //key elements default to required
    //(no longer as the next seq key can be provided by the database)
    //if (element.getAttribute('exodusfieldno')=='0'&&(typeof(element.getAttribute('exodusrequired'))=='undefined'||element.getAttribute('exodusrequired')=='false'))
    //{
    // //element.setAttribute('exodusrequired','true')
    // element.setAttribute('exodusrequired',true)
    //}

    if (!element.getAttribute('exodusrequired') || element.getAttribute('exodusrequired') == 'false')
        element.setAttribute('exodusrequired', '')

}

function form_setdefault(str) {
    gro.defaultrevstr = unescape(str.exodusconvert('`^]}\~', rm + fm + vm + sm + tm + stm))
    return 'ok'
}

function* form_postread_noteifdeleted(descending) {

    //skip if not deleted. deleted = gds.isnewrecord but has some version
    if (!gds.isnewrecord || !(yield* gds.getx('VERSION')))
        return false

    //descending means the latest log is inserted as first mv, which is the least common option, in PLAN/SCHEDULE/BATCHES
    var logn = descending ? 0 : -1

    var note = 'This record was deleted by ' + (yield* gds.get1('USERNAME_UPDATED', logn)) + ' on ' + (yield* gds.get1('DATETIME_UPDATED', logn)).exodusoconv('[DATE_TIME]') + '.'
    note += '\n\nYou can restore it by saving it again'
    yield* exodusnote(note)

    gallowsavewithoutchanges = true
    setchangesmade(false, true)//change style of Save button

    //indicate is deleted
    return true
}

function* document_oncopy(event) {

    event = getevent(event)
    var element = event.target

    //choose
    //form_oncopy if present
    //form_oncopy_generic
    if (typeof form_oncopy == 'undefined')
        form_oncopy = form_oncopy_generic

    //return true if handled EVEN IF FAILED
    //return false to allow normal handler below
    if (yield* form_oncopy(event)) {
        return exoduscancelevent(event)
    }

    return true

}

function* form_oncopy_generic(event) {

    event = getevent(event)

    //resolve any data entry issues first
    if (!(yield* validateupdate()))
        return true//prevent normal copy

    //if no cols to copy then return false to trigger normal copy
    var cols = form_copypaste_getcols(event, false)
    if (!cols)
        return false

    var elementid = cols[0][0]

    //determine the lns that are selected
    var selection = window.getSelection()
    //var elements=document.getElementsByName('ITEM_DESCRIPTION')
    //getElementByName doesnt work on span tags
    var elements = document.getElementsByClassName('exodusid_' + elementid)
    var lns = []
    if (!selection.containsNode)
        return false//not supported in MSIE so trigger normal copy?
    for (var ln = 0; ln < elements.length; ++ln) {
        if (selection.containsNode(elements[ln], true))
            lns.push(ln)
    }

    //if nothing to copy, false = allow normal copy
    if (!lns.length)
        return false

    var ncols = cols.length
    var data = []

    //first line output is tab separated column titles (dictids)
    var txln = []
    for (var coln = 0; coln < ncols; ++coln) {
        txln.push(cols[coln][1])
    }
    data.push(txln.join('\t'))

    //ln used in gds.get1(dictid,ln) below
    for (var lnn = 0; lnn < lns.length; ++lnn) {
        var ln = lns[lnn]

        //build a line of text using the column names
        var txln = []
        for (var coln = 0; coln < ncols; ++coln) {
            var colpars = cols[coln]
            var dictid = colpars[0]
            if (!dictid)
                //empty col for letters and dates multi-row generated below
                txln.push('')
            else {
                var cell = yield* gds.get1(dictid, ln)
                if (colpars[2] && colpars[2].slice(0, 1) == '[')
                    cell = cell.exodusoconv(colpars[2])
                txln.push(cell)
            }
        }

        //tab separated columns
        data.push(txln.join('\t'))

    }

    //using dos/windows style line endings to be compatible with windows notepad
    //everything else seems to handle them properly
    event.clipboardData.setData('text/plain', data.join('\r\n'));
    ///event.clipboardData.setData('text/html', '<b>Hello, world!</b>');

    //prevent any data from selection being copied normally
    exoduscancelevent(event)

    return false

}

function* document_onpaste(event) {

    event = getevent(event)
    var element = event.target

    //prevent paste into readonly
    var msg = element.getAttribute('exodusreadonly')
    if (msg) {
		//Generic message for read-only fields
		if (msg == 'true') {
			msg = 'This is a read-only field'
		}
        exoduscancelevent()
   	    return yield* exodusinvalid(msg)
	}

    //only supporting form_paste in first column
    if (!element.getAttribute('exodusisfirstinputcolumn')) {
        //perform normal paste before any yielding is done which loses it
        return true
    }

    var clipboarddata = event.clipboardData || window.clipboardData || event.originalEvent.clipboardData
    if (!clipboarddata)
        return false

    //var data = clipboarddata.getData('text/html')
    var text = clipboarddata.getData('text/plain')

    //standardise on \n
    text = text.replace(/\r\n/g, '\n')

    //only supporting form_paste with multiple lines of paste (col header plus min one line)
    if (text.indexOf('\n') < 0)
        return true

    //use form_onpaste_generic if form_onpaste not defined
    if (typeof form_onpaste == 'undefined')
        form_onpaste = form_onpaste_generic

    //return true to suppress normal handler EVEN IF FAILED
    if (yield* form_onpaste(event))
        return exoduscancelevent(event)

    //in case we selected a text node
    if (!element.getAttribute && element.parentNode.getAttribute)
        element = element.parentElement

    //the following doesnt seem to simulate pasting anymore
    //so we rely on early return above to perform normal paste
    /*
    //prevent pasting HTML into SPANS
    if (element.tagName!='SPAN')
        return true

    var clipboarddata = event.clipboardData || window.clipboardData || event.originalEvent.clipboardData
    if (!clipboarddata)
        return false

    //var data = clipboarddata.getData('text/html')
    var text = clipboarddata.getData('text/plain')

    //standardise on \n
    text = text.replace(/\r\n/g, '\n')

    //these dont work on INPUT or TEXTAREA
    //document.execCommand("insertText", false, text);
    //non-MSIE
    document.execCommand("insertHTML", false, text);
    //MSIE
    document.execCommand('paste', false, text)

    //dont continue to normal paste
    return exoduscancelevent(event)

    */
}

function form_copypaste_getcols(event, pasting) {

    event = getevent(event)

    var element = event.target
    if (!element || !element.getAttribute) {
        element = element.parentElement
        if (!element || !element.getAttribute)
            return false
    }

    var selection = window.getSelection()
    var tablex = getancestor(selection.anchorNode, 'TABLE')
    if (!tablex)
        return false
    var groupn = tablex.getAttribute('exodusgroupno')
    if (groupn < 1)
        return false

    //only copy/paste on rows - get groupn or quit
    //var groupn=element.getAttribute('exodusgroupno')
    //if (groupn<1)
    //    return false

    //only cut/paste on first col
    //if (!element.getAttribute('exodusisfirstinputcolumn'))
    //    return false

    //get table columns
    var tablefieldns = gtables[groupn]
    var cols = []
    for (var fn = 0; fn < tablefieldns.length; ++fn) {
        var element = gfields[tablefieldns[fn]]
        if (pasting && element.getAttribute('exodusreadonly'))
            continue
        var elementtitle = element.id.exodusconvert('_', ' ')
        var conversion = element.getAttribute('exodusconversion')
        var col = [element.id, elementtitle, conversion]
        cols.push(col)
    }

    if (!cols.length)
        return false

    return cols

}

function* form_onpaste_generic(event, elementid, validatedata_function, importdata_function) {

    //return false - to indicate normal paste should occur
    //return true - to avoid normal paste
    /////////////////////////////////////
    if (!validatedata_function)
        validatedata_function = form_onpaste_generic_validatedata
    if (!importdata_function)
        importdata_function = form_onpaste_generic_importdata

    event = getevent(event)

    //do normal paste if not on a row
    if (getrecn() == null)
        return false

    //determine columns to paste otherwise quit
    if (!elementid) {
        var cols = form_copypaste_getcols(event, true)
        if (!cols)
            return false
        elementid = cols[0][0]
    }

    //    //indicate normal paste should occur except on first column of rows
    //    if (!event.target||event.target.id!=elementid)
    //      return false
    //
    //    //rather horrible way to reverse default paste which happens later
    //    var beforepaste_element=gpreviouselement
    //    var beforepaste_value=gpreviousvalue
    //    window.setTimeout(
    //        function () {
    //            setvalue(beforepaste_element,beforepaste_value)
    //        }
    //    ,1)
    //    
    //resolve any data entry issues first
    if (!(yield* validateupdate()))
        return true

    //from here on we do NOT want normal paste to happen afterwards
    exoduscancelevent()

    //skip if no clipboard
    var clipboarddata = gevent.clipboardData ? gevent.clipboardData : window.clipboardData
    if (!clipboarddata)
        return true

    //var data = clipboarddata.getData('Text/HTML')
    //var data = clipboarddata.getData('Text/Plain')
    var data = clipboarddata.getData('Text')

    //standardise on \n and remove trailing \n
    data = data.replace(/\r\n/g, '\n').replace(/\n+$/, '')

    //normal paste if not multi-line
    if (data.indexOf('\n') < 0)
        return false

    //    //user pastes data
    //    if (!data) {
    //        //args.SCHEDULE_TEXT = gscheduleimportdata.exodusconvert(tm,vm)
    //        var args = {}
    //        data = yield* exodusshowmodaldialog('../media/schedules_import.htm', args)
    //        if (!data)
    //            return true
    //        data = data.replace(tm, '\n')
    //        //gscheduleimportdata = data
    //    }

    //convert pairs of double quotes to one double quote and remove surrounding double quotes
    //eg "30""" becomes 30"
    //data = data.exodusswap('""', '&quote;').exodusconvert('"', '').exodusswap('&quote;', '"')

    //convert data to array and tidy up
    data = data.split('\n')
    var nlines = data.length
    var line1 = data[0].split('\t')
    var ncols = line1.length
    for (var ln = nlines - 1; ln >= 0; --ln) {

        var line = data[ln].split('\t')

        //trim leading and trailing spaces and space-like characters
        //line = line.replace(/^ +/, '').replace(/ +$/, '')
        //trim all space before and after all tabs
        //line = line.replace(/ +\t/g, '\t').replace(/\t +/g, '\t')
        //unfortunately array.exodustrim removes empty cells
        //line=line.exodustrim()
        for (var ii = line.length - 1; ii >= 0; --ii)
            line[ii] = line[ii].exodustrim()

        // skip blank lines
        //if (!line.join('')) {
        //    data.splice(ln,1)
        //    --nlines
        //    continue
        //}

        //ensure full set of columns
        while (line.length < ncols)
            line.push('')

        data[ln] = line
    }

    data.startln = 1
    if (nlines < data.startln + 1) {
        //yield* form_undopaste(beforepaste_element,beforepaste_value)
        yield* exodusinvalid('Not enough lines to import')
        return true
    }

    //pass the desired target cols with the data
    data.cols = cols

    //////////        
    //validate
    //////////
    var ninvalid = yield* validatedata_function(data)
    if (ninvalid === false)
        return true

    if (data.length <= 1) {
        yield* exodusinvalid('Nothing to import')
        return true
    }

    //option to quit if any invalid items
    if (ninvalid) {
        if (!(yield* exodusyesno('There were ' + ninvalid + ' invalid items\nDo you want to continue?\n\n(Invalid items will be imported as blank)')))
            return true
    }

    ////////
    //update
    ////////
    var savegrecn = grecn
    var result = yield* importdata_function(data)

    yield* calcfields()

    //focus on new row
    focuson(document.getElementsByName(elementid)[savegrecn])

    return true

}

function* form_onpaste_ignore_cancel(rown, coln, coltitle, data, msg) {
    return yield* exodusokcancel('WARNING! in Row:' + (rown + 1) + ' Col:' + (coln + 1) + '\n\n' + coltitle + ' = "' + data + '"\n\n' + msg, 'Ignore', 'Cancel')
}

function* form_onpaste_generic_validatedata(data) {

    //returns ninvalid (or false to cancel)

    var ninvalid = 0
    var nlns = data.length

    //configuration

    var line1 = data[0]
    var rowtitles = line1
    var ncols = line1.length

    //this translates form coln to coln in the input/pasted coln
    var colns = []
    data.colns = colns

    //data.cols=gcostestimatecols
    var ncols = data.cols.length

    //add conversions and validations to data.cols[]
    for (var coln = 0; coln < ncols; ++coln) {
        var col = data.cols[coln]
        var element0 = $$(col[0])
        if (element0[0])
            element0 = element0[0]

        col.conversion = element0.getAttribute('exodusconversion')
        if (!col.conversion)
            col.conversion = element0.getAttribute('exoduscheckbox')
        if (col.conversion && col.conversion.slice(0, 1) != '[')
            col.conversion = col.conversion.exodussplit(':;')

        col.filename = element0.getAttribute('exodusfilename')
        col.validation = element0.getAttribute('exodusvalidation')
    }

    //parse line1 for column headings and find the datacoln for each screencoln
    var nimportablecols = 0
    for (var datacoln = 0; datacoln < line1.length; ++datacoln) {
        var coltitle = line1[datacoln].toUpperCase()
        for (var coln = 0; coln < ncols; ++coln) {
            if (data.cols[coln][1] == coltitle) {
                data.colns[coln] = datacoln
                ++nimportablecols
                break
            }
        }
    }
    //ensure at least one column to be imported
    if (!nimportablecols) {
        var allcoltx = []
        for (var coln = 0; coln < ncols; ++coln)
            allcoltx.push(data.cols[coln][1])
        return yield* exodusinvalid('No recognisable columns in data\n\nPossible columns: ' + allcoltx.join(', '))
    }

    //for each line of pasted data
    for (var ln = 1; ln < nlns; ++ln) {

        var line = data[ln]

        //for each screen coln
        for (coln = 0; coln < ncols; ++coln) {

            //find the data col
            var datacoln = data.colns[coln]
            if (datacoln) {

                var ivalue = null
                var ovalue = line[datacoln]
                if (!ovalue)
                    continue

                var col = data.cols[coln]

                //basic input conversion/validation
                if (col.conversion) {
                    //things like [NUMBER,NDECS]
                    if (col.conversion.slice(0, 1) == '[') {
                        ivalue = ovalue.exodusiconv(col.conversion)
                    }
                    //things like dropdowns and checkboxes
                    else {
                        ivalue = null
                        for (var ii = 0; ii < col.conversion.length; ++ii) {
                            var option = col.conversion[ii]
                            if (ovalue == option[1])
                                ivalue = option[0]
                            else if (ovalue == option[0])
                                ivalue = ovalue
                        }
                        if (ivalue == null) {
                            gmsg = 'Allowable values are ' + col.conversion.exodusjoin(',=')
                        }
                    }
                    //blank any invalid values
                    if (ivalue === null) {
                        //'Format must be '+conversion.slice(1,-1).split(',')[0]
                        if (!(yield* form_onpaste_ignore_cancel(ln, datacoln, data.cols[coln][1], ovalue, gmsg)))
                            return false
                        ivalue = ''
                        ninvalid++
                    }
                }
                else
                    ivalue = ovalue

                //basic file check
                if (col.filename) {
                    ivalue = ivalue.toUpperCase()

                    //warn and blank if not on file
                    var rec = []
                    if (!(yield* rec.exodusread(col.filename, ivalue))) {
                        //'Format must be '+conversion.slice(1,-1).split(',')[0]
                        if (!(yield* form_onpaste_ignore_cancel(ln, datacoln, data.cols[coln][1], ivalue, 'Code does not exist or cannot be accessed')))
                            return false
                        ivalue = ''
                        ninvalid++
                    }
                }

                line[datacoln] = ivalue
            }
        }
    }

    return ninvalid

}

function* form_onpaste_generic_importdata(data) {

    //returns undefined if successful or false if any invalid vehicle code


    var ninserted = 0
    var nlines = data.length
    var ncols = data.cols.length

    //for each line of pasted data
    for (var ln = data.startln; ln < nlines; ++ln) {

        var line = data[ln]

        //add a new row if last row is not blank
        //var rown = gds.data.group1.length - 1
        //if (yield* gds.get1('VEHICLE_CODE', rown)) {
        //    exodusaddrow(1)
        //    rown += 1
        //}
        if (ninserted)
            ++grecn
        else
            ninserted = 1
        gds.insertrow(1, grecn)

        //for each screen coln
        for (coln = 0; coln < ncols; ++coln) {

            //find the data col
            var datacoln = data.colns[coln]
            if (typeof datacoln == 'number') {

                var colid = data.cols[coln][0]
                gpreviouselement = $$(colid)[grecn]
                gpreviousvalue = getvalue(gpreviouselement)
                setvalue(gpreviouselement, line[datacoln])

                if (!(yield* validateupdate()))
                    return false

            }
        }
    }

    return

}
