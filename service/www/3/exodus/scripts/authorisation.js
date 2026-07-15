// Copyright NEOSYS All Rights Reserved.
///users

async function authorisation_changeallemaildomains() {
    var oldemails = await gds.getall('EMAIL_ADDRESS')
    //replace all ";xxxxx@" with ; to end up with ; separated list of domains
    var olddomains = (';' + oldemails.join(';')).replace(/;.*?@/gi, ';').exodustrim(';')
    olddomains = olddomains.split(';').exodusunique()
    var reply = await exodusdecide('Change which domain?', olddomains)
    if (!reply)
        return await exodusinvalid()
    var olddomain = olddomains[reply - 1]
    var newdomain = ''
    while (true) {
        newdomain = await exodusinput('Change ' + olddomain + ' to what?', newdomain)
        if (!newdomain)
            return await exodusinvalid()
        if (!newdomain.match(/^[^\. -_][\w.-]*\.[a-z]{2,}$/)) {
            await exodusinvalid(newdomain + ' is not a valid email domain name.\rPut something like mycompany.com')
            continue;
        }
        break;
    }
    var newemails = oldemails.join(fm).exodusswap(olddomain, newdomain).split(fm)
    if (newemails != oldemails) {
        await gds.setx('EMAIL_ADDRESS', null, newemails)
        setchangesmade(true)
    }
    return true
}

async function form_presort(colid) {
    if (colid == 'LOCKS')
        await form_filter('unfilter', colid)
    return true
}

var gtasks_usern = 999999
var gtasks_updatehighergroups = false
var gtasks_otheruserids = []
var gtasks_otherkeys = []
var gtasks_newpassword

async function form_preread() {

    //used to distinguish between read security for authorisation and read security for hourlyrates
    securitymode = window.location.href.toString().indexOf('authorisation.htm') >= 0 ? 'SECURITY' : 'HOURLYRATES'
    gro.readenvironment = securitymode
    return true

}

function authorisation_tabs_show(which) {

    var userspanel = $$('authorisation_tabpanel_users')
    var taskspanel = $$('authorisation_tabpanel_tasks')
    var usersbtn = $$('authorisation_tabbtn_users')
    var tasksbtn = $$('authorisation_tabbtn_tasks')
    if (!userspanel || !taskspanel)
        return

    var showusers = which == 'users'
    userspanel.className = showusers ? 'exodustab_panel exodustab_active' : 'exodustab_panel'
    taskspanel.className = showusers ? 'exodustab_panel' : 'exodustab_panel exodustab_active'
    if (usersbtn)
        usersbtn.className = showusers ? 'exodustab exodustab_active' : 'exodustab'
    if (tasksbtn)
        tasksbtn.className = showusers ? 'exodustab' : 'exodustab exodustab_active'
}

async function user_showtasks(event) {

    //if (!(await validateupdate())) return false
    event=getevent(event)
    grecn = getrecn(event.target)

    var userid = await gds.get1('USER_ID',grecn)

    db.request = 'EXECUTE\rGENERAL\rGETTASKS\r\r' + userid
    if (!(await db.send()))
        return await exodusinvalid(db.response)

    var taskid = await exodusdecide('Tasks authorised for ' + userid, db.data, [[0, 'Task'], [1, 'Lock']], 0, '', '', inverted = true)
    if (!taskid)
        return false

    var recn = (await gds.getall('TASK_ID')).exoduslocate(taskid) - 1
    if (recn < 0)
        return false

    authorisation_tabs_show('tasks')
    focuson(document.getElementsByName('LOCKS')[recn])

    return true

}

async function authorisation_print() {
    await openwindow('EXECUTE\rGENERAL\rLISTAUTH.TASKS')
    await openwindow('EXECUTE\rGENERAL\rLISTAUTH.USERS')
    return true
}

//in authorisation.js and users.htm
async function form_postwrite() {

    //if change own password then login with the new one
    //otherwise cannot continue/unlock document so the lock hangs
    if (gtasks_newpassword)
        db.login(gusername, gtasks_newpassword)

    return true

}

async function form_prewrite() {

    //skip out if in hourly rates screen
    //EMAIL_ADDRESS not available in hourly rates
    if (!(gds.dictitem('EMAIL_ADDRESS')))
        return true
        
    var userids = await gds.getall('USER_ID')
        
    //prevent insertion of blank lines if not authorised
    if (!(await exodussecurity('AUTHORISATION UPDATE GROUPS'))) {
        var userrows = gds.data.group1
        for (var rown = userrows.length - 1; rown >= 0; --rown) {
            //new rows have gds.data.group1.db_ordinal.text blank
            if (userids[rown] == '' && userrows[rown].db_ordinal.text == '') {
                focuson($$('USER_ID')[rown])
                return await exodusinvalid('User in line ' + (rown + 1) + ' must not be left blank\r\r' + gmsg)
            }
        }
    }

    var userids = await gds.getall('USER_ID')

    //check for duplicated keys
    var allkeys = await gds.getall('KEYS')
    var accumkeys=[]
    var accumusers=[]
    //work backwards through users (low to high rank) so we can more easily remove duplicates
    for (var usern=userids.length-1;usern>=0;--usern) {

        var userid=userids[usern]
        
        //reset list on gap lines between users - which determine groups
        if (!userid) {
            accumkeys=[]
            accumusers=[]
        }

        var userkeys=allkeys[usern]
        if (!userkeys)
            continue

        //work backwards through user keys so we can more easily remove duplicates
        userkeys=userkeys.exodusconvert(';',' ').split(' ')
        for (var userkeyn=userkeys.length-1;userkeyn>=0;--userkeyn) {
            userkey=userkeys[userkeyn]
            var accumn1=accumkeys.exoduslocate(userkey)
            if (!accumn1) {
                accumkeys.push(userkey)
                accumusers.push(userid)
            } else {

                var question='Duplicate key "'+userkey+'" on user '+userids[usern]+' is not required'
                if (userids[usern]!=accumusers[accumn1-1])
                    question+='|because it is already granted to '+accumusers[accumn1-1]+' at a lower level.'
                var response=await exodusconfirm(question,1,'<u>R</u>emove','<u>L</u>eave','Cancel')
                if (!response){
                    focuson($$('KEYS')[usern])
                    return await exodusinvalid()
                }
                if (response==1) {
                    userkeys.splice(userkeyn,1)
                    await gds.setx('KEYS',usern,userkeys.join(','))
                }    
            }
        }
    }
    
    //check for new users
    var newusers = false
    var emails = await gds.getall('EMAIL_ADDRESS')
    var origuserids = gro.revstr.split(fm)[0].split(vm)
    for (var usern = userids.length - 1; usern >= 0; --usern) {
        if (emails[usern]) {
            newusers = !origuserids.exoduslocate(userids[usern])
        }
        if (newusers)
            break
    }
    if (!newusers)
        return await exodusokcancel('OK to save?', 1)

    var reply = await exodusconfirm('OK to save?', 3, 'Email new users', 'Save only', 'Cancel')
    if (!reply)
        return false

    await gds.setx('EMAIL_NEW_USERS', null, reply == 1 ? 1 : 0)

    return true
}

async function form_postread() {

    gtasks_otheruserids = (await gds.getx('TEMP_OTHER_USERS')).split(' ')

    gtasks_otherkeys = (await gds.getx('TEMP_OTHER_KEYS')).split(' ')

    gtasks_usern = (await gds.getall('USER_ID')).exoduslocate(gusername) - 1
    if (gtasks_usern < 0) gtasks_usern = 999999

    gtasks_updatehighergroups = await exodussecurity('AUTHORISATION UPDATE HIGHER GROUPS')

    exodussettimeout('await authorisation_postdisplay()', 10)

    return true

}

async function authorisation_postdisplay() {

    //hide expired users
    await form_filter('filter', 'EXPIRY_DATE', /[0123456789]/)
    
    //hide similar tasks
    await form_filter('filter', 'HIDDEN_LINES', /1/)

    //prevent changing username of groups ("users" with keys) or group separator lines (blank lines)
    if (!(await exodussecurity('AUTHORISATION UPDATE GROUPS'))) {
        var userids = await gds.getall('USER_ID')
        var keys = await gds.getall('KEYS')
        for (var ln = keys.length - 1; ln >= 0; --ln) {
            if (keys[ln] || !userids[ln])
                exodussetreadonly('USER_ID', gmsg, '', ln)
        }
    }

    return true
}

async function user_val_userid() {

    //ZZZ check if user in hidden higher/lower groups!

    if (!(await task_checkrank())) return false

    //prevent duplicates/allow move
    var usernames = await gds.getall('USER_ID')
    var otherln1 = usernames.exoduslocate(gvalue)
    if (otherln1) {
        var msg = gvalue + ' already exists in line ' + otherln1

        //prevent moving above yourself unless allowed access to higher groups
        //but this cannot really happen since you could not insert or change higher than yourself in the first place
        //if (otherln1-1<grecn&&!gtasks_updatehighergroups) return await exodusinvalid(msg)

        //check user wants to move the account here
        if ((await exodusyesno(msg + '\r\rDo you want to move it here?', 2)) != 1)
            return await exodusinvalid()

        //copy the other row here  
        var datarow = gds.updaterow('copy', gds.data.group1[grecn], gds.data.group1[otherln1 - 1])
        /**/ await gds.rebind(datarow, datarow, forced = true)

        //delete the other row
        gds.deleterow(1, otherln1 - 1)

        return true

    }

    if (gvalue != '---' && 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'.indexOf(gvalue.slice(0, 1)) < 0)
        return await exodusinvalid('Username must start with an alphabetic letter A-Z')

    if (gusername.indexOf('EXODUS') < 0 && gvalue.indexOf('EXODUS') + 1)
        return await exodusinvalid('User name cannot include "EXODUS"')

    //prevent hidden users
    if (gtasks_otheruserids.exoduslocate(gvalue))
        return await exodusinvalid(gvalue + ' user already exists in another group')

    //have to reenter password
    //passwords are all blank in the UI except those created in the UI before saving
    var newpass=await gds.get1('PASSWORD',grecn)
    if (goldvalue && !newpass && (await exodusokcancel('If you change the user code,\nany existing password will no longer be usable.\nA new password will have to be created.', 2))!=1)
        return false

        return true

    }

    async function user_val_keys() {

        if (!(await task_checkrank())) return false

        if (!(await user_haskeys())) return false

        return true

    }

    async function user_haskeys() {
        if (!gvalue) return true
        gvalue = gvalue.exodustrim()
        var keys = gvalue.split(' ')
        for (var keyn = 0; keyn < keys.length; keyn++) {
            var key = keys[keyn].exodustrim()
            if ((key == 'EXODUS' && gusername != 'EXODUS') || gtasks_otherkeys.exoduslocate(key))
                return await exodusinvalid('You are not authorised to use key ' + key.exodusquote())
            keys[keyn] = key
        }
        gvalue = keys.join(' ')
        return true
    }

    ///tasks

    async function task_val_taskid() {

        //prevent change of unauthorised tasks
        if (!(await task_authorised())) return false

        //prevent addition of unauthorised tasks
        if (!(await task_authorised(gvalue))) return false

        return true

    }

    async function task_val_lock() {

        //prevent change of unauthorised tasks
        if (gusername!='EXODUS' && !(await task_authorised()))
            return false

        //prevent addition of existing keys that user does not have
        if (!(await user_haskeys())) return false

        return true

    }

    async function form_preinsertrow1() {

        //double check prevent addition of users
        if (!(await exodussecurity('USER CREATE'))) return await exodusinvalid(gmsg)

        //prevent addition of users above yourself
        if (!gtasks_updatehighergroups && grecn <= gtasks_usern) return await exodusinvalid('You cannot insert users above yourself')

        return true

    }

    async function form_predeleterow1() {

        //always allowed to delete rows that you have managed to insert yourself
        var userrows = gds.data.group1
        if (userrows[grecn].db_ordinal.text == '')
            return true

        //double check prevent deletion of users
        if (!(await exodussecurity('USER DELETE')))
            return await exodusinvalid(gmsg)

        //prevent deletion of yourself and higher users
        if (!gtasks_updatehighergroups && grecn <= gtasks_usern)
            return await exodusinvalid('You cannot delete yourself or higher users')

        //cannot delete group separator/blank lines or lines with keys
        if ((((await gds.get1('USER_ID')) == '') || (await gds.get1('KEYS'))) && !(await exodussecurity('AUTHORISATION UPDATE GROUPS')))
            return await exodusinvalid('You cannot delete this line because\r' + gmsg)

        return true

    }

    async function form_predeleterow2() {

        //prevent deletion of unauthorised tasks
        if (!(await task_authorised()))
            return false

        if ((await gds.get1('LOCKS')) && !(await exodussecurity('AUTHORISATION UPDATE LOCKS')))
            return await exodusinvalid('You cannot delete this line because\r' + gmsg)

        return true

    }

    async function task_authorised(task) {

        if (!task)
            task = await gds.get1('TASK_ID')
        if (!task)
            return true

        if (!(await exodussecurity(task)))
            return await exodusinvalid('You cannot delete, change or add tasks that\ryou are not authorised to do yourself\r\r' + task)

        return true
    }

    async function task_checkrank() {

        //prevent modification of users above yourself
        if (!gtasks_updatehighergroups && grecn < gtasks_usern) return await exodusinvalid('You cannot modify users above yourself')

        return true

    }
