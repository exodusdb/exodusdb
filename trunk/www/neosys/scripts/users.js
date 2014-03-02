// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

function form_postinit() {

    //enable/disable password changing button
    neosyssetexpression('button_password', 'disabled', '!gusers_authorisation_update&&gkey!=gusername')

    //force retrieval of own record
    if (gro.dictitem('USER_ID').defaultvalue)
        window.setTimeout('focuson("USER_NAME")', 100)

    gwhatsnew = neosysgetcookie(glogincode, 'NEOSYS2', 'wn').toLowerCase()
    if (gwhatsnew) {
        if (window.location.href.toString().slice(0, 5) == 'file:') gwhatsnew = 'file:///' + gwhatsnew
        else gwhatsnew = '..' + gwhatsnew.slice(gwhatsnew.indexOf('\\data\\'))
        neosyssetcookie(glogincode, 'NEOSYS2', '', 'wn')
        neosyssetcookie(glogincode, 'NEOSYS2', gwhatsnew, 'wn2')
        window.setTimeout('windowopen(gwhatsnew)', 1000)
        //windowopen(gwhatsnew)
    }

    $$('button_password').onclick = user_setpassword

    return true

}

//just to avoid confirmation			
function form_prewrite() {
    return true
}

//in authorisation.js and users.htm
var gtasks_newpassword
function form_postwrite() {

    //if change own password then login with the new one
    //otherwise cannot continue/unlock document so the lock hangs
    if (gtasks_newpassword) db.login(gusername, gtasks_newpassword)
    gtasks_newpassword = false

    //to avoid need full login to get new font/colors
    neosyssetcookie(glogincode, 'NEOSYS2', gds.getx('SCREEN_BODY_COLOR'), 'fc')
    neosyssetcookie(glogincode, 'NEOSYS2', gds.getx('SCREEN_FONT'), 'ff')
    neosyssetcookie(glogincode, 'NEOSYS2', gds.getx('SCREEN_FONT_SIZE'), 'fs')

    return true

}

function users_postdisplay() {

    var signatureimageelement = document.getElementById('signature_image')
    if (signatureimageelement) {
        signatureimageelement.src = ''
        signatureimageelement.height = 0
        signatureimageelement.width = 0
        signatureimageelement.src = '../images/'+gdataset+'SHARP/UPLOAD/USERS/' + gkey.neosysconvert(' ', '') + '_signature.jpg'
    }

    //show only first five lines
    form_filter('refilter', 'LOGIN_DATE', '', 4)

    var reminderdays = 6
    var passwordexpires = gds.getx('PASSWORD_EXPIRY_DATE')
    $passwordexpiryelement = $$('passwordexpiryelement')
    $passwordexpiryelement.innerHTML = ''
    if (passwordexpires) {
        var expirydays = (neosysint(passwordexpires) - neosysdate())
        if (expirydays <= reminderdays)
            $passwordexpiryelement.innerHTML = '<font color=red>&nbsp;&nbsp;&nbsp;Password expires in ' + expirydays + ' days.</font>'
    }

    return true
}

function form_postread() {
    window.setTimeout('users_postdisplay()', 10)
    return true
}

function users_upload_signature() {

    //return openwindow('EXECUTE\r\MEDIA\r\OPENMATERIAL',scheduleno+'.'+materialletter)
    /*
    params=new Object()
    params.scheduleno=scheduleno
    params.materialletter=materialletter
    neosysshowmodaldialog('../NEOSYS/upload.htm',params)
    */

    //images are not stored per dataset at the moment so that they can be
    //nor are they stored per file or per key so that they can be easily saved into a shared folder instead of going by web upload
    params = {}
    params.database = gdataset
    params.filename = 'USERS'
    params.key = gkey.neosysconvert(' ', '') + '_signature'
    params.versionno = ''//newarchiveno
    params.updateallowed = true
    params.deleteallowed = true
    //params.allowimages = true
    //we only allow one image type merely so that the signature file name
    //is always know and doesnt need saving in the user file
    //and no possibility of uploading two image files with different extensions
    params.allowablefileextensions = 'jpg'

    var targetfilename = neosysshowmodaldialog('../NEOSYS/upload.htm', params)
    window.setTimeout('users_postdisplay()', 1)
    if (!targetfilename)
        return false

    return true

}
function user_signature_onload(el) {
    el.removeAttribute("width")
    el.removeAttribute("height")
}
