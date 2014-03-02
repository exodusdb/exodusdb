// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**
///users

function authorisation_changeallemaildomains() {
    var oldemails = gds.getx('EMAIL_ADDRESS', null)
    //replace all ";xxxxx@" with ; to end up with ; separated list of domains
    var olddomains = (';' + oldemails.join(';')).replace(/;.*?@/gi, ';').neosystrim(';')
    olddomains = olddomains.split(';').neosysunique()
    var reply = neosysdecide('Change which domain?', olddomains)
    if (!reply)
        return neosysinvalid()
    var olddomain = olddomains[reply - 1]
    var newdomain = ''
    while (true) {
        newdomain = neosysinput('Change ' + olddomain + ' to what?', newdomain)
        if (!newdomain)
            return neosysinvalid()
        if (!newdomain.match(/^[^\. -_][\w.-]*\.[a-z]{2,}$/)) {
            neosysinvalid(newdomain + ' is not a valid email domain name.\rPut something like mycompany.com')
            continue;
        }
        break;
    }
    var newemails = oldemails.join(fm).neosysswap(olddomain, newdomain).split(fm)
    if (newemails != oldemails) {
        gds.setx('EMAIL_ADDRESS', null, newemails)
        gchangesmade = true
    }
    return true
}

function form_presort(colid) {
    if (colid == 'LOCKS')
        form_filter('unfilter', colid)
    return true
}

var gtasks_usern = 999999
var gtasks_updatehighergroups = false
var gtasks_otheruserids = []
var gtasks_otherkeys = []
var gtasks_newpassword

function form_preread() {

    //used to distinguish between read security for authorisation and read security for hourlyrates
    securitymode = window.location.href.toString().indexOf('authorisation.htm') >= 0 ? 'SECURITY' : 'HOURLYRATES'
    gro.readenvironment = securitymode
    return true

}

function user_showtasks(userid) {

    //if (!validateupdate()) return false

    grecn = getrecn(eventtarget)
    var userid = gds.getx('USER_ID', grecn)[0]

    db.request = 'EXECUTE\rGENERAL\rGETTASKS\r\r' + userid
    if (!db.send()) return neosysinvalid(db.response)

    var taskid = neosysdecide('Tasks authorised for ' + userid, db.data, [[0, 'Task'], [1, 'Lock']], 0, '', '', inverted = true)
    if (!taskid) return false

    var recn = gds.getx('TASK_ID', null).neosyslocate(taskid) - 1
    if (recn < 0) return false

    focuson(document.getElementsByName('LOCKS')[recn])

    return true

}

function authorisation_print() {
    openwindow('EXECUTE\rGENERAL\rLISTAUTH.TASKS')
    openwindow('EXECUTE\rGENERAL\rLISTAUTH.USERS')
    return true
}

//in authorisation.js and users.htm
function form_postwrite() {

    //if change own password then login with the new one
    //otherwise cannot continue/unlock document so the lock hangs
    if (gtasks_newpassword) db.login(gusername, gtasks_newpassword)

    return true

}

function form_prewrite() {

    //prevent insertion of blank lines if not authorised
    if (!neosyssecurity('AUTHORISATION UPDATE GROUPS')) {
        var userrows = gds.data.group1
        var userids = gds.getx('USER_ID', null)
        for (var rown = userrows.length - 1; rown >= 0; --rown) {
            if (userids[rown] == '' && userrows[rown].db_ordinal.text == '') {
                focuson($$('USER_ID')[rown])
                return neosysinvalid('User in line ' + (rown + 1) + ' must not be left blank\r\r' + gmsg)
            }
        }
    }

    //check for new users
    var newusers = false
    var userids = gds.getx('USER_ID', null)
    var emails = gds.getx('EMAIL_ADDRESS', null)
    var origuserids = gro.revstr.split(fm)[0].split(vm)
    for (var usern = userids.length - 1; usern >= 0; --usern) {
        if (emails[usern]) {
            newusers = !origuserids.neosyslocate(userids[usern])
        }
        if (newusers)
            break
    }
    if (!newusers)
        return neosysokcancel('OK to save?', 1)

    var reply = neosysyesnocancel('OK to save?', 3, 'Email new users', 'Save only', 'Cancel')
    if (!reply)
        return false

    gds.setx('EMAIL_NEW_USERS', null, reply == 1 ? 1 : 0)

    return true
}

function form_postread() {

    gtasks_otheruserids = gds.getx('TEMP_OTHER_USERS').split(' ')

    gtasks_otherkeys = gds.getx('TEMP_OTHER_KEYS').split(' ')

    gtasks_usern = gds.getx('USER_ID', null).neosyslocate(gusername) - 1
    if (gtasks_usern < 0) gtasks_usern = 999999

    gtasks_updatehighergroups = neosyssecurity('AUTHORISATION UPDATE HIGHER GROUPS')

    window.setTimeout('authorisation_postdisplay()', 10)

    return true

}

function authorisation_postdisplay() {
    form_filter('filter', 'HIDDEN_LINES', /1/)

    //prevent changing username of groups ("users" with keys) or group separator lines (blank lines)
    if (!neosyssecurity('AUTHORISATION UPDATE GROUPS')) {
        var userids = gds.getx('USER_ID', null)
        var keys = gds.getx('KEYS', null)
        for (var ln = keys.length - 1; ln >= 0; --ln) {
            if (keys[ln] || !userids[ln])
                neosyssetreadonly('USER_ID', gmsg, '', ln)
        }
    }

    return true
}

function user_val_userid() {

    //ZZZ check if user in hidden higher/lower groups!

    if (!task_checkrank()) return false

    //prevent duplicates/allow move
    var usernames = gds.getx('USER_ID', null)
    var otherln1 = usernames.neosyslocate(gvalue)
    if (otherln1) {
        var msg = gvalue + ' already exists in line ' + otherln1

        //prevent moving above yourself unless allowed access to higher groups
        //but this cannot really happen since you could not insert or change higher than yourself in the first place
        //if (otherln1-1<grecn&&!gtasks_updatehighergroups) return neosysinvalid(msg)

        //check user wants to move the account here
        if (neosysyesno(msg + '\r\rDo you want to move it here?', 2) != 1) return neosysinvalid()

        //copy the other row here  
        var datarow = gds.updaterow('copy', gds.data.group1[grecn], gds.data.group1[otherln1 - 1])
        gds.rebind(datarow, datarow, forced = true)

        //delete the other row
        gds.deleterow(1, otherln1 - 1)

        return true

    }

    if (gvalue != '---' && 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'.indexOf(gvalue.slice(0, 1)) < 0)
        return neosysinvalid('Username must start with an alphabetic letter A-Z')

    if (gusername.indexOf('NEOSYS') < 0 && gvalue.indexOf('NEOSYS') + 1)
        return neosysinvalid('User name cannot include "NEOSYS"')

    //prevent hidden users
    if (gtasks_otheruserids.neosyslocate(gvalue))
        return neosysinvalid(gvalue + ' user already exists in another group')

    //have to reenter password
    if (gds.getx('PASSWORD', grecn)[0] == '')
        if (!neosysokcancel('You will have to enter a new password\rif you change the user name', 1))
            return false

        return true

    }

    function user_val_keys() {

        if (!task_checkrank()) return false

        if (!user_haskeys()) return false

        return true

    }

    function user_haskeys() {
        if (!gvalue) return true
        gvalue = gvalue.neosystrim()
        var keys = gvalue.split(' ')
        for (var keyn = 0; keyn < keys.length; keyn++) {
            var key = keys[keyn].neosystrim()
            if ((key == 'NEOSYS' && gusername != 'NEOSYS') || gtasks_otherkeys.neosyslocate(key))
                return neosysinvalid('You are not authorised to use key ' + key.neosysquote())
            keys[keyn] = key
        }
        gvalue = keys.join(' ')
        return true
    }

    ///tasks

    function task_val_taskid() {

        //prevent change of unauthorised tasks
        if (!task_authorised()) return false

        //prevent addition of unauthorised tasks
        if (!task_authorised(gvalue)) return false

        return true

    }

    function task_val_lock() {

        //prevent change of unauthorised tasks
        if (!task_authorised()) return false

        //prevent addition of existing keys that user does not have
        if (!user_haskeys()) return false

        return true

    }

    function form_preinsertrow1() {

        //double check prevent addition of users
        if (!neosyssecurity('USER CREATE')) return neosysinvalid(gmsg)

        //prevent addition of users above yourself
        if (!gtasks_updatehighergroups && grecn <= gtasks_usern) return neosysinvalid('You cannot insert users above yourself')

        return true

    }

    function form_predeleterow1() {

        //always allowed to delete rows that you have managed to insert yourself
        var userrows = gds.data.group1
        if (userrows[grecn].db_ordinal.text == '')
            return true

        //double check prevent deletion of users
        if (!neosyssecurity('USER DELETE'))
            return neosysinvalid(gmsg)

        //prevent deletion of yourself and higher users
        if (!gtasks_updatehighergroups && grecn <= gtasks_usern)
            return neosysinvalid('You cannot delete yourself or higher users')

        //cannot delete group separator/blank lines or lines with keys
        if (((gds.getx('USER_ID')[0] == '') || gds.getx('KEYS')[0]) && !neosyssecurity('AUTHORISATION UPDATE GROUPS'))
            return neosysinvalid('You cannot delete this line because\r' + gmsg)

        return true

    }

    function form_predeleterow2() {

        //prevent deletion of unauthorised tasks
        if (!task_authorised()) return false

        if (gds.getx('LOCKS')[0] && !neosyssecurity('AUTHORISATION UPDATE LOCKS')) return neosysinvalid('You cannot delete this line because\r' + gmsg)

        return true

    }

    function task_authorised(task) {

        if (!task) task = gds.getx('TASK_ID')[0]
        if (!task) return true

        if (!neosyssecurity(task)) return neosysinvalid('You cannot delete, change or add tasks that\ryou are not authorised to do yourself\r\r' + task)

        return true
    }

    function task_checkrank() {

        //prevent modification of users above yourself
        if (!gtasks_updatehighergroups && grecn < gtasks_usern) return neosysinvalid('You cannot modify users above yourself')

        return true

    }
