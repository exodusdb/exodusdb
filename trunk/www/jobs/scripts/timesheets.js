//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

//global configuration obtained from server in postinit
var gistimesheetadmin = false
var gactivityrequired = true
var gminimumminutes = 15
var gminimumhours = 0
var gmaximumhours = 8
var gmaximumdays = 3
var gdepartmentcode = ''
var gactivities = ''
var guserlist = ''

var gdefaultusercode = ''

var gfirsttime = true//bring up current timesheet automatically

var glastusercode//determines when to refresh activity list

function timesheet_select(mode) {
    var idate = gkey.neosysfield('*', 2)
    if (!idate)
        return false
    var offset = (mode == 'next') ? 1 : -1
    var nextkey = gkey.neosysfieldstore('*', 2, 1, Number(idate) + offset)
    window.setTimeout('opendoc(' + neosysquote(nextkey) + ')', 10)
}

function form_postread() {
    //window.setTimeout('form_postpostread()',10)
    var jobnos = gds.getx('JOB_NO', null)
    if (jobnos.join()) {
        gfirstnonkeyelement = 'HOURS'
    }
    else {
        gfirstnonkeyelement = 'JOB_NO'
        //dont autopopup insert
        //  window.setTimeout('form_postinsertrow1()',10)
    }

    //activate previous and next buttons
    //var usercode=gkey.neosysfield('*',1)
    //var idate=Number(gkey.neosysfield('*',2))
    //gkeys=[usercode+'*'+(idate-1),usercode+'*'+idate,usercode+'*'+(idate+1)]
    //gkeyn=1

    return true
}

function form_postinsertrow1() {

    return true

    //auto click job selection popup
    var jobno = jobs_pop_jobno(many = false, notclosed = true)
    if (!jobno)
        return neosysinvalid()

    //store the selected job
    gds.setx('JOB_NO', grecn, jobno)
    calcfields()

    window.setTimeout("focuson('HOURS')", 10)

    return true

}

function form_postinit() {

    //is this necessary?
    if (gusername == 'undefined')
        db.login()

    //get timesheet parameters, and activities for login user
    db.request = 'EXECUTE\rPRODUCTION\rTIMESHEET.POSTINIT'
    if (!db.send()) {
        neosysinvalid(db.response)
        return false
    }
    // alert(db.data)

    //trigger automatic opening
    if (!gparameters.key && !gistimesheetadmin)
        gparameters.key = gusername + '*'

    glastusercode = gusername
    gdefaultusercode = gusername
    if (gdefaultusercode == 'NEOSYS')
        gdefaultusercode = ''

    var register = db.data.split(rm)

    var register1 = register[1].split(fm)
    gactivityrequired = register1[0]
    gminimumhours = neosysnumber(register1[1])
    gmaximumhours = neosysnumber(register1[2])
    gmaximumdays = neosysnumber(register1[3])
    gminimumminutes = neosysnumber(register1[4])

    gdepartmentcode = register[2]
    gactivities = register[3]
    //var pseudo2=register[6]
    gistimesheetadmin = is(register[7])
    guserlist = register[8]
    //var securitycodes=register[9]

    //set the usercode dropdown
    var element = document.getElementsByName('USER_CODE')[0]
    element.neosysdefault = gdefaultusercode
    //neosysbreak('element','before')
    if (gistimesheetadmin) {
        //element.setAttribute('neosysdropdown','SELECT\rUSERS\rBY USER_CODE\rUSER_CODE USER_CODE\rXML'+fm+'USER_CODE\rUSER_CODE')
        //element_neosyssetdropdown(element)

        //isadmin then all users are dropdown
        element.setAttribute('neosysconversion', guserlist)

        setdropdown3(element, element.getAttribute('neosysconversion'))

    }
    else {
        //not admin so only own name
        element.setAttribute('neosysconversion', gusername + ';' + gusername)
        setdropdown3(element, element.getAttribute('neosysconversion'))
    }

    //check we can still get the element
    var element = document.getElementsByName('USER_CODE')[0]
    //neosysbreak(element,'after')

    //set activity required
    element = document.getElementsByName('ACTIVITY_CODE')[0]

    //do not use false as it gets converted to a string in the element sometimes
    //and 'false' is true
    element.setAttribute('neosysrequired', gactivityrequired ? true : '')

    //set the activity dropdown
    element.setAttribute('neosysconversion', gactivities)
    setdropdown3(element, element.getAttribute('neosysconversion'))
    glastusercode = gusername

    return true

}

function timesheet_setactivities(usercode) {
    //set the dropdown now
    var element = document.getElementsByName('ACTIVITY_CODE')[0]
    element.setAttribute('neosysdropdown', 'EXECUTE\rPRODUCTION\rGETUSERACTIVITIES,' + usercode + fm + 'CODE\rNAME')
    element_neosyssetdropdown(element)

    //seems to always hide in ff so skip this for the time being
    //if need to hide it when no activities then will have to resolve
    // showhide(['activitiessection'],!!element.neosysdropdown)

}

function form_preread() {
    // alert('preread')
    var usercode = getvalue('USER_CODE')

    //only timesheet admins can enter other peoples timesheets
    if (!gistimesheetadmin && usercode != gusername) {
        neosysinvalid('Username must be ' + gusername)
        return false
    }

    timesheet_setactivities(usercode)

    if (!gistimesheetadmin)
        gdefaultusercode = usercode

    //each user can have a different set of activity codes
    if (glastusercode != usercode) {
        glastusercode = usercode
    }
    /*
    //timesheets older than gmaximumdays are read only except for admins
    greadonlymode=false
 
    //var timesheetdate=neosysnumber(gds.getcells('TIMESHEET_DATE')[0].text)
    var timesheetdate=neosysnumber(gkey.neosysfield('*',2))
    if (timesheetdate&&gmaximumdays&&timesheetdate<=(neosysdate()-gmaximumdays))
    {
    msg=neosysquote(neosysoconv(timesheetdate,'[DATE]'))
    msg=msg+'  - timesheet is older than '+gmaximumdays+' days.\r\r'  
    if (gistimesheetadmin)
    {
    msg=msg+'Are you sure that you want'
    }
    else
    {
    msg=msg+'You will not be able'
    greadonlymode=true
    }

    if (!confirm(msg+' to create or update this timesheet.'))
    return false

    }
    */
    return true

}

function timesheet_def_usercode() {

    if (!gdefaultusercode)
        return ''

    //alert('timesheet_def_usercode()')
    if ($$('TIMESHEET_DATE').value == '') {

        //bring up today's record for speed
        if (gfirsttime && !gistimesheetadmin) {
            //focuson(HOURS) does not work on Mac! (HOURS is multivalued)
            //focuson(document.getElementsByName('HOURS')[0]) ... works
            focuson('HOURS')
        }
        else {
            //focuson(TIMESHEET_DATE) DOES work on Mac! (it is not multivalued)
            focuson('TIMESHEET_DATE')
            //   focuson($$('TIMESHEET_DATE'))
        }
        gfirsttime = false
    }

    return gdefaultusercode

}

function timesheet_def_date() {

    return neosysdate()

}

function timesheet_val_usercode() {
    return true
}

function timesheet_val_job() {

    //get the job
    job = []
    if (!job.neosysread('JOBS', gvalue)) {

        if (job.neosysresponse.indexOf(' does not exist ') >= 0) {
            //dont allow multiple select if any line because it will overwrite them now as it is
            var many = !gds.getx('JOB_NO', null).join()
            var response = jobs_pop_jobno(many, notclosed = true, mode = '', filter = gvalue)
            if (!response)
                return false
            gvalue = response
            return true
        }
        return neosysinvalid(job.neosysresponse)
    }

    //check the job is open
    if (job[7] == 'Y') {
        msg = neosysquote(gvalue) + '  job is closed'
        if (!adminconfirm(msg))
            return false
    }

    return true

}

function timesheet_val_hours() {

    //check minimum time unit
    if (gminimumminutes) {
        var minimumhours = gminimumminutes / 60
        var gvalue2 = neosysint(gvalue / minimumhours) * minimumhours
        if (gvalue2 != gvalue) {
            msg = 'The minimum time interval is ' + gminimumminutes + ' minutes\r'
            msg += '\r1:30  1 30  1/30  1,30 = 1 hour and 30 minutes\r'
            msg += '\rBUT 1.30 = 1 hour and 3/10ths of an hour'
            if (!adminconfirm(msg))
                return false
        }
    }

    //prevent (warn?) if exceeded the max number of hours (will not be able to save)
    if (!totalhoursok(false, gvalue, goldvalue))
        return false

    return true

}

function totalhoursok(checkminimum, hours, oldhours) {

    if (!hours)
        hours = 0
    if (!oldhours)
        oldhours = 0

    //determine the new total number of hours
    //var totalhours=neosysnumber(gds.data.TOTAL_HOURS.text)
    var totalhours = Number(gds.regetx('TOTAL_HOURS')) - Number(oldhours) + Number(hours)

    msg = 'You have entered a total of ' + totalhours + ' hours but\r'

    //always error if exceeded unless timesheet admin
    if (gmaximumhours && totalhours > gmaximumhours) {
        msg += 'the maximum number of hours allowed is ' + gmaximumhours
        if (!adminconfirm(msg)) return false
    }
    /*
    //error if under limit (allow if today or admin)
    //this is now done on server where <min is allowed for non-working days
    if (checkminimum&&gminimumhours&&totalhours<gminimumhours)
    {
    msg+='the minimum number of hours allowed is '+gminimumhours

    var timesheetdate=neosysnumber(gkey.neosysfield('*',2))
    if (timesheetdate==neosysdate())
    {
    if (neosysokcancel(msg,2)!=1)
    return false
    }
    else
    {
    if (!adminconfirm(msg))
    return false
    }
    }
    */
    return true

}

function form_prewrite() {

    //check max/min hours
    if (!totalhoursok(true))
        return false

    //check not empty
    if (!gds.getx('JOB_NO', null).join())
        return neosysinvalid('Timesheets cannot be empty')

    return true

}

function adminconfirm(msg) {
    if (gistimesheetadmin) {
        if (neosysokcancel(msg, 2) == 1)
            return true
    }
    else {
        neosysinvalid(msg)
    }
    return false
}