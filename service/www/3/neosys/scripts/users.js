// Copyright NEOSYS All Rights Reserved.

function* form_postinit() {

    //force retrieval of own record
    if (gro.dictitem('USER_ID').defaultvalue)
        neosyssettimeout('focuson("USER_NAME")', 100)

    gwhatsnew = neosysgetcookie2('wn').toLowerCase()
    if (gwhatsnew) {
        if (window.location.href.toString().slice(0, 5) == 'file:')
            gwhatsnew = 'file:///' + gwhatsnew
        else
            gwhatsnew = '..' + gwhatsnew.slice(gwhatsnew.indexOf('\\data\\'))
        neosyssetcookie(glogincode, 'NEOSYS2', '', 'wn')
        neosyssetcookie(glogincode, 'NEOSYS2', gwhatsnew, 'wn2')
        neosyssettimeout('yield* windowopen(gwhatsnew)', 1000)
        //yield* windowopen(gwhatsnew)
    }

    //addeventlistener($$('button_password'),'click','user_setpassword')

    return true

}

function* form_postdisplay() {

    //enable/disable password changing button
    //neosyssetexpression('button_password', 'disabled', '!gusers_authorisation_update&&gkey!=gusername')
    $$('button_password').disabled=!gusers_authorisation_update&&gkey!=gusername

    $expiryelement = $$('expiryelement')
    $expiryelement.innerHTML = ''

    var gettingstarted=$$('gettingstarted')
    gettingstarted.innerHTML='Browser Configuration/Reset'

    var firstlogin=false
    if (gkey==gusername) {
        var loginstatus=yield* gds.getall('LOGIN_RESULT')
        loginstatus[0]=''//remove current login OK
        if (true||!loginstatus.neosyslocate('OK'))
            gettingstarted.innerHTML='<font color=red><strong>Click HERE for browser configuration *REQUIRED*</strong></font>'
    }
        
    return true
}
    
//just to avoid confirmation			
function* form_prewrite() {
    return true
}

//in authorisation.js and users.htm
var gtasks_newpassword
function* form_postwrite() {

    //if change own password then login with the new one
    //otherwise cannot continue/unlock document so the lock hangs
    if (gtasks_newpassword)
        db.login(gusername, gtasks_newpassword)
    gtasks_newpassword = false

    //to avoid need full login to get new font/colors
    neosyssetcookie(glogincode, 'NEOSYS2', yield* gds.getx('SCREEN_BODY_COLOR'), 'fc')
    neosyssetcookie(glogincode, 'NEOSYS2', yield* gds.getx('SCREEN_FONT'), 'ff')
    neosyssetcookie(glogincode, 'NEOSYS2', yield* gds.getx('SCREEN_FONT_SIZE'), 'fs')

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

function* users_postdisplay() {

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
                
        signatureimageelement.src = '../../images/'+gdataset+'/upload/users/' + gkey.neosysconvert(' ', '').toLowerCase() + '_signature.jpg'
    }

    //show only first five lines
    yield* form_filter('refilter', 'LOGIN_DATE', '', 4)

    $expiryelement = $$('expiryelement')
    $expiryelement.innerHTML = ''//also done in form_postdisplay because this function doesnt get called after yield* cleardoc()?

    var userexpirydate=yield* gds.getx('EXPIRY_DATE')
    if (userexpirydate && userexpirydate <= neosysdate()) {
        $expiryelement.innerHTML = '<font color=red><strong>EXPIRED ' + userexpirydate.neosysoconv('[DATE]') + '</strong></font>'
    } else {
    
        var reminderdays = 6
        var passwordexpires = yield* gds.getx('PASSWORD_EXPIRY_DATE')
        if (passwordexpires) {
            var text=''
            var expirydays = (neosysint(passwordexpires) - neosysdate())
            if (expirydays < 0)
             text='Password has EXPIRED'
            else if (expirydays == 0)
             text='PASSWORD EXPIRES TODAY!'
            else if (expirydays <= reminderdays)
             text='Password expires in ' + expirydays + ' days.'
            if (text)
                $expiryelement.innerHTML = '<font color=red>&nbsp;&nbsp;&nbsp;'+text+'</font>'
        }
    }
    
    return true
}

function* form_postread() {
    neosyssettimeout('yield* users_postdisplay()', 10)
    return true
}

function* users_upload_signature() {

    //upload login users signature if no key
    var username=gkey
    if (!username)
        username=gusername
        
    //images are not stored per dataset at the moment so that they can be
    //nor are they stored per file or per key so that they can be easily saved into a shared folder instead of going by web upload
    params = {}
    params.database = gdataset
    params.filename = 'USERS'
    params.key = username.neosysconvert(' ', '') + '_signature'
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

    var targetfilename = yield* neosysshowmodaldialog('../neosys/upload.htm', params)
    if (gkey)
        neosyssettimeout('yield* users_postdisplay()', 1)
    if (!targetfilename)
        return false

    return true

}
