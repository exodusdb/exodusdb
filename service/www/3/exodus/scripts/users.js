// Copyright NEOSYS All Rights Reserved.

async function form_postinit() {

    // Own-user open is via gparameters.key set in dict_USERS (dbform opendoc path).
    // Only leftover: optional what's-new window after login cookie.
    gwhatsnew = exodusgetcookie2('wn').toLowerCase()
    if (gwhatsnew) {
        if (window.location.href.toString().slice(0, 5) == 'file:')
            gwhatsnew = 'file:///' + gwhatsnew
        else {
            //gwhatsnew = '..' + gwhatsnew.slice(gwhatsnew.indexOf('\\data\\'))
            //gwhatsnew = '..' + gwhatsnew.slice(gwhatsnew.replace('\\','/').indexOf('/data/'))
        }
        exodussetcookie(glogincode, 'EXODUS2', '', 'wn')
        exodussetcookie(glogincode, 'EXODUS2', gwhatsnew, 'wn2')
        // Defer open so form finishes init; takeoff when Gate A free (fail loud if not)
        exodus_begin_when_idle(function () { return windowopen(gwhatsnew) }, 'users whatsnew', {
            delay_ms: 1000
        })
    }

    return true

}

async function form_postdisplay() {

    //enable/disable password changing button
    //exodussetexpression('button_password', 'disabled', '!gusers_authorisation_update&&gkey!=gusername')
    $$('button_password').disabled=!gusers_authorisation_update&&gkey!=gusername

    $expiryelement = $$('expiryelement')
    $expiryelement.innerHTML = ''

    //var gettingstarted=$$('gettingstarted')
    //gettingstarted.innerHTML='Browser Configuration/Reset'

    var firstlogin=false
    if (gkey==gusername) {
        var loginstatus=await gds.getall('LOGIN_RESULT')
        loginstatus[0]=''//remove current login OK
        //if (true||!loginstatus.exoduslocate('OK'))
        //    gettingstarted.innerHTML='<font color=red><strong>Click HERE for browser configuration *REQUIRED*</strong></font>'
    }

    // Chrome = last-saved cookies (not unsaved field / popup play).
    // Discard and cleardoc re-enter postdisplay.
    if (typeof colors_restore_saved_chrome == 'function')
        colors_restore_saved_chrome()
    else if (typeof exodus_chrome_from_cookies == 'function')
        exodus_chrome_from_cookies()

    // After gds.load (not form_postread): form_filter and signature img need bound rows.
    await users_postdisplay()

    return true
}

//just to avoid confirmation
async function form_prewrite() {
    return true
}

//in authorisation.js and users.htm
var gtasks_newpassword
async function form_postwrite() {

    //if change own password then login with the new one
    //otherwise cannot continue/unlock document so the lock hangs
    if (gtasks_newpassword)
        db.login(gusername, gtasks_newpassword)
    gtasks_newpassword = false

    // Only cookie write site for screen chrome. Sanitize empty/Default/keywords → "".
    var store = typeof exodus_chrome_cookie_store == 'function'
        ? exodus_chrome_cookie_store
        : function (v) { return v == null ? '' : String(v) }
    var bodyColor = store(await gds.getx('SCREEN_BODY_COLOR'))
    var screenFont = store(await gds.getx('SCREEN_FONT'))
    var screenFontSize = store(await gds.getx('SCREEN_FONT_SIZE'))
    exodussetcookie(glogincode, 'EXODUS2', bodyColor, 'fc')
    exodussetcookie(glogincode, 'EXODUS2', screenFont, 'ff')
    exodussetcookie(glogincode, 'EXODUS2', screenFontSize, 'fs')
    // Same path as every other screen entry
    if (typeof exodus_chrome_from_cookies == 'function')
        exodus_chrome_from_cookies()

    return true

}

//fix for older MSIE browsers
function user_signature_onload_sync(event) {
    if (isMSIE) {
        event=getevent(event)
        event.target.removeAttribute("width")
        event.target.removeAttribute("height")
    }
}

async function users_postdisplay() {

    var signatureimageelement = document.getElementById('signature_image')
    if (signatureimageelement) {
        signatureimageelement.src = ''

        //older msie browsers IE6-8? show a red cross/missing file icon for users with no uploaded signature file
        //instead of the alt ("") which more recent browsers do
        //removed in image onload event - see user_signature_onload_sync
        if (isMSIE) {
            signatureimageelement.height = 0
            signatureimageelement.width = 0
        }
        signatureimageelement.src = '../../images/'+gdataset+'/upload/users/' + gkey.exodusconvert(' ', '').toLowerCase() + '_signature.jpg'
    }

    //show only first five lines
    await form_filter('refilter', 'LOGIN_DATE', '', 4)

    $expiryelement = $$('expiryelement')
    $expiryelement.innerHTML = ''//also done in form_postdisplay because this function doesnt get called after await cleardoc()?

    var userexpirydate=await gds.getx('EXPIRY_DATE')
    if (userexpirydate && userexpirydate <= exodusdate()) {
        $expiryelement.innerHTML = '<strong style="color:Highlight">EXPIRED ' + userexpirydate.exodusoconv('[DATE]') + '</strong>'
    } else {
        var reminderdays = 6
        var passwordexpires = await gds.getx('PASSWORD_EXPIRY_DATE')
        //if (passwordexpires) {
        if (!passwordexpires) {
            $expiryelement.innerHTML = '<strong style="color:var(--exodus-icon-green,#22b014)">ACTIVE</strong>'
        } else {
            var text=''
            var expirydays = (exodusint(passwordexpires) - exodusdate())
            if (expirydays < 0)
             text='Password has EXPIRED'
            else if (expirydays == 0)
             text='PASSWORD EXPIRES TODAY!'
            else if (expirydays <= reminderdays)
             text='Password expires in ' + expirydays + ' days.'
            if (text)
                $expiryelement.innerHTML = '<span style="color:Highlight">&nbsp;&nbsp;&nbsp;'+text+'</span>'
        }
    }
    return true
}

async function users_upload_signature() {

    //upload login users signature if no key
    var username=gkey
    if (!username)
        username=gusername

    //images are not stored per dataset at the moment so that they can be
    //nor are they stored per file or per key so that they can be easily saved into a shared folder instead of going by web upload
    params = {}
    params.database = gdataset
    params.filename = 'USERS'
    params.key = username.exodusconvert(' ', '') + '_signature'
    params.versionno = ''//newarchiveno
    params.updateallowed = true
    params.deleteallowed = true
    //params.allowimages = true
    //we only allow one image type merely so that the signature file name
    //is always know and doesnt need saving in the user file
    //and no possibility of uploading two image files with different extensions
    params.allowablefileextensions = 'jpg'
    params.minheight = 50//pixels
    params.maxheight = 200//pixels

    var targetfilename = await exodusshowmodaldialog('../exodus/upload.htm', params)
    if (gkey)
        await users_postdisplay()
    if (!targetfilename)
        return false

    return true

}
