var gusername_element
var gpassword_element
var gdataset_element
var gautologin_element
//var gpasswordreset_button
var gpasswordreset_link
var glogin_button
var gisdialog

// gsystem (ACCOUNTS|ADAGENCY): pre-login hint only (.VOL list / entry URL / cookie).
// Authoritative APPLICATION is decided by the backend at serve start
// (SCHEDULES open → ADAGENCY, else ACCOUNTS). After LOGIN, ap= overwrites EXODUSsystem.
var gsystem
var ghref
var gportno=''
var gwaitdiv

gloggedon = false

//autosize if a popup login
//gautofitwindow=gisdialog

async function formfunctions_onload() {

    ghref = document.location.href.split('?')
    startinglocation = ghref[1]
    gsystem = ghref[2]

    if (!gsystem)
        gsystem = exogetcookie2('', 'EXODUSsystem',null)
    if (!gsystem || gsystem == 'UNDEFINED')
        gsystem = 'ADAGENCY'
    gsystem = gsystem.toUpperCase()
    //save permanent default (pre-login hint; corrected from ap= after LOGIN below)
    document.cookie = 'EXODUSsystem=' + gsystem
    
    //+'; path=/'
    //expires=Sun, 31 Dec 2100 23:59:59 GMT'

    db = new exodblink

    gusername_element = $$('usernameelement')
    gpassword_element = $$('passwordelement')
    gdataset_element = $$('datasetelement')
    gautologin_element = $$('autologinelement')
    //gpasswordreset_button = $$('passwordresetbutton')
    gpasswordreset_link = $$('passwordresetlink')
    glogin_button = $$('loginbutton')
    gwaitdiv = $$('waitdivelement')

    // needjs display:inline / login-ui display:none in HTML; swap when script runs
    var needjs = document.getElementById('exo-needjs')
    if (needjs)
        needjs.style.display = 'none'
    var loginui = document.getElementById('exo-login-ui')
    if (loginui)
        loginui.style.display = ''

    // Warm both eye assets (open + slash) so first click is not blocked on fetch
    ;['login-eye.svg', 'login-eye-slash.svg'].forEach(function (name) {
        var img = new Image()
        img.src = '../exodus/images/theme2/' + name
    })

    addeventlistener(glogin_button,'click','login_onclick')
    addeventlistener(gautologin_element,'click','autologin_onclick')
    //addeventlistener(gpasswordreset_button,'click','passwordreset_onclick')
    addeventlistener(gpasswordreset_link,'click','passwordreset_onclick')
    var datasetx

    // ensure inputs are selectable in modal dialogs
    //by doing all popups in calling window
    if (gisdialog) {
        //gusername_element.value=gDialogArguments[0]
        //gpassword_element.value=gDialogArguments[1]
        var datasetx = gDialogArguments[2]
        var datasetlist = gDialogArguments[4]
        await setdropdown2(gdataset_element, datasetlist, Array("code", "name"), datasetx, null)

        if (gDialogArguments[5] == 'true') {
            gusername_element.innerText = gDialogArguments[0]
            gpassword_element.innerText = gDialogArguments[1]
            gautologin_element.checked = true
        } else {
            gusername_element.value=gDialogArguments[0]
            if (gusername_element.value) {
                //setdisabledandhidden(gusername_element,true)
                gusername_element.disabled=true
                gdataset_element.disabled=true
                gusername_element.setAttribute('disabled',true)
                gdataset_element.setAttribute('disabled',true)
            }
        }
    }
    else {
        //gsystem=ghref[2]
        //if (!gsystem) gsystem='ADAGENCY'
        datasetcode = exogetcookie('EXODUS','dataset')
        //guess/default the datasetcode to the first part of the domain name for the first time or after clearing cookies
        if (!datasetcode) {
            //datasetcode = window.location.host.toString().split('.')[0].toUpperCase().substr(0,8)
            datasetcode = window.location.host.toString().split('.')[0]
            if (typeof exogetcookie == 'undefined')
                datasetcode = datasetcode.substr(0,8).toUpperCase()
        }
        await exosetdropdown(gdataset_element, "GETDATASETS\r" + gsystem, Array("code", "name"), datasetcode, null)
        if (startinglocation != 'login' && exogetcookie2('a', 'EXODUS', '') == 'true') {
            gusername_element.value = exogetcookie2('u', 'EXODUS', '')
            gpassword_element.value = exogetcookie2('p', 'EXODUS', '')
            if (gusername_element.value && gpassword_element.value) {
                gautologin_element.checked = true
            }
        }
        else {
            gusername_element.value = ''
            gpassword_element.value = ''
            gautologin_element.checked = false
        }
        //if (gautologin_element.checked!=true) exosetcookie('','EXODUS','','ll',true)
    }

    gwaitdiv.style.display = "none"

    exosettimeout('gusername_element.disabled?gpassword_element.focus():gusername_element.focus()', 100)
    
    if (gusername_element.disabled) {
        gpassword_element.focus()
        gpassword_element.select()
    } else {
        gusername_element.focus()
        gusername_element.select()
    }
    
    //gdataset_element.onkeydown=dataset_onkeydown
    //document.onkeydown = document_onkeydown
    addeventlistener(document,'keydown','document_onkeydown')

    // Auto-login when credentials are already present — e.g. browser password
    // manager autofill, or Remember-me cookie path that prefilled u/p.
    // (Old test was checkbox .value == 'on', which is always true for a checkbox
    // and was never a real "Remember me checked" check.) Skip if no password yet;
    // dblogin would no-op anyway. Hold Shift to skip (window.event.shiftKey if present).
    if ((!window.event || !window.event.shiftKey)
        && gusername_element.value
        && gpassword_element.value)
        await dblogin()

}

async function autologin_onclick() {
    if (await hidepassword('await autologin_onclick()'))
        return
    
    if (!gautologin_element.checked) return true
    return confirm('PLEASE READ THE FOLLOWING CAREFULLY.\n\nAutologin requires your username and password to be stored on your computer in a "cookie"\rwhich could be viewed by anybody who has access to your computer.\r\rIF YOU DO NOT WANT THIS TO BE DONE, CLICK THE CANCEL BUTTON.')
}

//treat password reminders/resets like logins because we have validated session yet to pass requests
async function passwordreset_onclick(event) {

    // href="#" only for keyboard focus — do not jump to top of page
    if (event && event.preventDefault)
        event.preventDefault()

    if (await hidepassword('await passwordreset_onclick()'))
        return
    
    //var args={}
    //args.USER_ID=gusername_element.value
    //await exoui_showmodaldialog(EXODUSlocation+'passwordreset.htm')
    var usercode = gusername_element.value.toUpperCase()
    if (!usercode) {
        exosettimeout('gusername_element.focus()', 100)
        gusername_element.focus()
        gusername_element.select()
        return await exoui_invalid('Username is required')
    }
    var oldpass = gpassword_element.value
    var email = await exoui_input('Password reset for database "' + gdataset_element.value + '"\n\nWhat is your registered email address?')
    if (!email)
        return
    if (email.indexOf('@') < 0)
        return await exoui_invalid('Email address must contain an @ character')
    gpassword_element.value = email
    //await dblogin()
    await login_onclick()

    gpassword_element.value = oldpass
}

async function document_onkeydown(event) {

    event=getevent(event)

    //keycode=event.keyCode?event.keyCode:event.which
    gkeycode = event.keyCode

    //Enter or F9 moves on or clicks login — stay in Gate A (do not timeout-spawn)
    if (gkeycode == 13 || gkeycode == 120) {
        await dblogin()
        return exocancelevent(event)
    }

}

async function dblogin() {

    //force selection of current selection in drop down if dropped down
    glogin_button.focus()

    // Same flight as keydown — do not .click() (would re-enter Gate A / be blocked)
    await login_onclick()

}

async function login_onclick() {

    if (await hidepassword('await login_onclick()'))
        return
    
    //prevent double execution due to onkeydown and enter gkeycode hitting login
    if (gloggedon)
        return

    //prevent form being submitted
    //event.cancelBubble=true
    //event.returnValue=false
    //exocancelevent(event)

    var datasetx = exogetdropdown(gdataset_element)

	// Remember the last database logged into if not a test database
    if (datasetx.substr(-5) != '_test')
       exosetcookie('EXODUS','dataset',datasetx,'',true)

    // Ensure login fields are not empty
    if (!gusername_element.value) {
        //  alert('Please enter your username first')
        gusername_element.focus()
        return
    }
    if (!gpassword_element.value) {
        //  alert('Please enter your password first')
        gpassword_element.focus()
        return
    }

    // Ensure username is in uppercase and no trailing spaces
    gusername_element.value = gusername_element.value.toUpperCase().replace(/ *$/, '')

    // Ensure username has no spaces
    if (gusername_element.value.includes(' ')) {
        gusername_element.focus();
        gusername_element.select();
        return await exoui_invalid("<b>Wrong username or password</b> \r → Check details and try again.");
    }

    var authno = ''

    //mac modaldialogs cannot have interactivity so return to caller
    if (gisdialog) {
        var response=[gusername_element.value, gpassword_element.value, datasetx, authno, '', gautologin_element.checked, gsystem]
        //window.returnValue = response
        //return window.close()
        return exoui_windowclose(response)
    }

    while (true) {

        var db = new exodblink
        //  db.system=gsystem

        ghref = document.location.href.split('?')
        startinglocation = ghref[1]
        //gsystem=ghref[2]?ghref[2]:'ADAGENCY'

        //gdataset and gusername_element is used as the login id. this is the ONLY place that it is set. elsewhere it is copied around
        //if you change this then also change the check in xhttp.asp
        //http standard allows max 20 cookies of 4Kb each. if more are created then the old ones are dropped
        //2 are used by EXODUS for global. 1 is used by ASP for session id. Leaving 17 max for logins
        //using dataset and username allows multiple logins to differnt databases in the same domain
        //and multiple logins to the same db with different usernames but only one login per username per database
        //also may exist in ABP LEDGER2 on server in reports to generate drill down code
        glogincode = (datasetx + '*' + gusername_element.value + '*').replace(/ /g,'')
        exosetcookie('', 'EXODUSlogincode', glogincode, 'logincode')

        db.request = 'LOGIN\r' + gusername_element.value + '\r' + gpassword_element.value + '\r' + datasetx + '\r' + authno + '\r' + gsystem + '\r' + gportno + '\rnewpass'
        if (await db.send()) {

            //save the settings

            //any login messages/reminders/warnings
            if (db.response != 'OK') {
                await exoui_warning(db.response.slice(3))
            }

            //cannot set cookie in modal dialog so this also has to be done in caller (clientfunctions)
            var temp = 'dataset=' + datasetx

            if (gautologin_element.checked) {
                temp += '&u=' + gusername_element.value
                temp += '&p=' + gpassword_element.value
                temp += '&a=' + gautologin_element.checked
                temp += '&s=' + gsystem
            }
            else {
                temp += '&u='
                temp += '&p='
                temp += '&a='
                temp += '&s='
            }

            //permanent
            exosetcookie('', 'EXODUS', temp, '', true)

            //temporary cookie for menu, gcompany etc
            exosetcookie(glogincode, 'EXODUS2', db.data)

            // Backend APPLICATION (ap=) — authoritative; replaces any entry URL/cookie hint
            var tt = exogetcookie2('ap', '', glogincode)
            if (tt) {
                gsystem = tt;
            }

            document.cookie = 'EXODUSsystem=' + gsystem
            //temporary cookie for password
            //gsystem=ghref[2]
            //if (!gsystem) gsystem='ADAGENCY'

            var temp = 'dataset=' + datasetx + '&username=' + gusername_element.value + '&system=' + gsystem
            if (document.protocolcode == 'file') temp += '&password=' + gpassword_element.value

            exosetcookie(glogincode, 'EXODUS2', temp)

            //save gdataset so that opening page knows what database we are in/get cookies for
            //not really necessary since set above now
            exosetcookie('', 'EXODUSlogincode', glogincode, 'logincode')
            //loginalert('default set '+glogincode)

            //work out the starting location
            startinglocation = document.location.href.split('?')[1]
            if (startinglocation == 'login')
                startinglocation = ''
            else if (!startinglocation)
                startinglocation = unescape(exogetcookie2('ll', 'EXODUS', ''))
            //if (!startinglocation||(window.event&&window.event.shiftKey)) startinglocation=EXODUSlocation+'users.htm'
            // Shift+skip uses window.event when present
            if (!startinglocation || (window.event && window.event.shiftKey)) {
                startinglocation = EXODUSlocation
                if (exogetcookie2('m').split(',')[0] == 'TIMESHEETS')
                    startinglocation += '../jobs/timesheets.htm'
                else
                    startinglocation = EXODUSlocation + 'users.htm'
            }

            //open the starting location
            var temp=window.location.href.toString()
            if (temp.slice(-1)=='/')
                temp+=startinglocation
            else
                temp=startinglocation
            window.location.assign(temp)
            gloggedon = true

            return true

        }

        //quit login if actually a password reset
        if (db.response.indexOf('Password Reset ') >= 0) {
            if (db.response.indexOf('Password Reset Failed') >= 0)
                return await exoui_invalid(db.response)
            else
                return await exoui_note(db.response)
        }

        //option to relogin with authorisation number
        //otherwise put up error message
        if (db.response.indexOf('This computer is number') >= 0) {
            var q = db.response.split('|').slice(1, 2) + ' What is the authorisation number?\r(Please contact EXODUS or your technical support)'
            var authno = prompt(q, '')
            if (authno == '' || authno == null) break
        }
        else {
            if (db.response.toLowerCase().indexOf('password')>=0) {
                //var choice=await exoui_confirm(db.response.split('|').join('\n'),1,'Try a different password, database or user code','Reset Password')
                var choice=await exoui_invalid(db.response.split('|').join('\n'))
            } else {
                var choice=await exoui_invalid(db.response.split('|').join('\n'))
            }
            if (choice == 2) {
                //exosettimeout('gpasswordreset_button.click()',100)
                exosettimeout('gpasswordreset_link.click()',100)
            } else {
                gpassword_element.focus()
                gpassword_element.select()
            }
            break
        }

    } //while

}

//using timeout to ensure password is hidden before any sync windows appear and block ui
//only really required on older browsers?
async function hidepassword(chainexpression) {
    if (gsavedpasswordelement) {
        showpassword_sync()
        exosettimeout(chainexpression,1)
        return true
    }
    return false
}

var gsavedpasswordelement
function showpassword_sync() {
	var currpasswordelement=$$('passwordelement')
	var togglePasswordBtn=$$('togglePassword');
	var showpasswordelement = togglePasswordBtn.getElementsByTagName('img')[0];
	//var showpasswordelement=$$('showpasswordelement')
	if (gsavedpasswordelement){
		// hide password → closed/slash icon
		currpasswordelement.id='visiblepasswordelement'
		gsavedpasswordelement.id='passwordelement'
		gsavedpasswordelement.value=currpasswordelement.value

		currpasswordelement.parentNode.insertBefore(gsavedpasswordelement,currpasswordelement)
		currpasswordelement.parentNode.removeChild(currpasswordelement)
		gsavedpasswordelement=null
		showpasswordelement.src = '../exodus/images/theme2/login-eye-slash.svg'
		return true

	} else {

		// show password → open eye
		gsavedpasswordelement=currpasswordelement
		// get attributes from the parent
		var parent = gsavedpasswordelement.parentNode;
		var el=document.createElement('input')
		el.classList=gsavedpasswordelement.classList
		el.id='passwordelement'
		//el.size=gsavedpasswordelement.size
		//el.setAttribute('size', gsavedpasswordelement.getAttribute('size'));
		el.tabIndex=gsavedpasswordelement.tabIndex
		el.value=gsavedpasswordelement.value
		el.style.textTransform='uppercase'
		// apply parent's width and height to the new element, adjusted for any padding or borders
		var computedStyle = window.getComputedStyle(parent);
		el.style.width = computedStyle.width;
		el.style.height = computedStyle.height;
		el.style.boxSizing = 'border-box';
		// padding so password does not overlap eye icon, similar padding for show password done in index.html
		el.style.paddingRight = '25px';

		//currpasswordelement.parentNode.insertBefore(el,currpasswordelement)
		parent.insertBefore(el, gsavedpasswordelement);
		//gsavedpasswordelement.parentNode.removeChild(gsavedpasswordelement)
		parent.removeChild(gsavedpasswordelement);
		showpasswordelement.src = '../exodus/images/theme2/login-eye.svg'
		return false
	}
}