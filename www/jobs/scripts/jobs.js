//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

//nb form_postread and jobs_postread in jobs_dict.htm
//since this file is included in many locations due to module name being same as file name and not bothered to change

function jobs_list_jobs() {
    windowopen('../jobs/joblist.htm')
    return true
}

function jobs_list_costestimates(mode) {
    var url = '../jobs/joblist.htm?MODE='
    if (mode) url += 'ESTIMATES'
    else {
        var reply = neosysdecide('', ['Purchase Orders', 'Purchase Invoices'])
        if (!reply) return false
        url += 'PURCHASE' + (reply == 1 ? 'ORDERS' : 'INVOICES')
    }
    windowopen(url)
    return true
}

function jobs_open_version() {
    var version = gds.getx('VERSION')
    if (!version) version = 'A'
    if (gvalue == version)
        return neosysinvalid('You are looking at version ' + gvalue + ' already')

    var key = gkey + gvalue
    windowopen(window.location.href.toString().split('?')[0], { "key": key })
    return true
}

function jobs_add_costestimate(costestimate, key) {

    //optionally prevent multiple estimates
    if (costestimate == 'estimate') {
        var estimatenos
        if (glocked)
            estimatenos = gds.getx('ESTIMATE_NO', null).join(vm)
        else {
            deletecacherecord('JOBS', gkey)
            estimatenos = gkey.neosysxlate('JOBS', 10)
        }
        if (estimatenos && !neosyssecurity('PRODUCTION ESTIMATE CREATE MANY PER JOB'))
            return neosysinvalid('This job already has an estimate\r' + estimatenos.split(vm).join(',') + '\r\r' + gmsg)
    }

    return jobs_open_costestimate(costestimate, key)

}

function jobs_open_costestimate(costestimate, key) {

    var args = {}
    if (!key) {

        if (gdatafilename == 'JOBS') {

            //check authorised to add if job is closed
            if (gds.getx('CLOSED') == 'Y') {
                if (!neosyssecurity('PRODUCTION ' + (costestimate == 'cost' ? 'ORDER' : 'ESTIMATE') + ' CREATE AFTER JOB CLOSED')) return neosysinvalid(gmsg)
            }

            key = '%NEW%'
            args.JOB_NO = gds.getx('JOB_NO')
        }
        else {
            key = gvalue
        }
    }
    args.key = key

    if (!saveandunlockdoc()) return false

    windowopen('../jobs/' + costestimate + 's.htm', args)

    return true

}

function jobs_open_cost(key) {
    return jobs_open_costestimate('cost', key)
}

function jobs_open_estimate(key) {
    return jobs_open_costestimate('estimate', key)
}

function jobs_val_jobno(jobno, onlyopen, job) {

    if (!onlyopen) onlyopen = false

    //get the job
    if (!job) job = []
    if (!job.neosysread('JOBS', jobno)) {
        return neosysinvalid(job.neosysresponse)
    }

    //check the job is open
    if (job[7] == 'Y') {
        msg = neosysquote(jobno)
        if (jobno != gvalue) msg += ' previous'
        msg += ' job is closed'
        if (onlyopen) return neosysinvalid(msg)
        if (neosysokcancel(msg, 2) != 1) return neosysinvalid()
    }

    //check brand code authorised, not stopped etc
    if (!validcode2('', '', job[2])) return neosysinvalid(gmsg)

    return true

}

function jobs_getstatuses() {

    db.request = "CACHE\rGETINDEXVALUESXML\rPRODUCTION.INVOICES\rSTATUS"
    if (!db.send()) {
        neosysinvalid(db.response)
        return ''
    }

    if (!db.data || db.data == '<records></records>') return ''

    var statuses = neosysxml2obj(db.data)

    return statuses

}

function jobs_pop_statuses(many) {
    var statuses = jobs_getstatuses()
    if (!statuses) return neosysinvalid('No statuses have been entered on job estimates yet')
    if (statuses.group1.length == 0) return neosysinvalid('No statuses have been entered yet')
    var result = neosysdecide2('', statuses, [['STATUS', 'Status']], 0, '', many)
    //for (var i=0;i<result.length;i++)
    //{
    // if (typeof result[i]=='string'&&result[i]==result[i].toUpperCase()) result[i]=result[i].neosyscapitalise()
    //}
    return result

}

function jobs_open_jobx() {
    var temp = { "key": gvalue }
    return true
}

function jobs_dict_jobno(di, many, onlyopen) {
    if (!many) many = false
    if (!onlyopen) onlyopen = false
    //di.link='open_job(gvalue)'
    di.link = 'windowopen2("../jobs/jobs.htm")'
    di.popup = 'jobs_pop_jobno(' + many + ',' + onlyopen + ')'
    di.filename = 'JOBS'
    di.validation = 'jobs_val_jobno(' + onlyopen + ')'
    if (gparameters.JOB_NO)
        di.defaultvalue = 'gparameters.JOB_NO'
}

function jobs_pop_jobno(many, notclosed, mode, filter) {

    if (!notclosed) {

        return agency_search("JOBS", many, options = notclosed)

    }
    else {

        var selectionclause = ''
        if (notclosed)
            selectionclause += ' WITH CLOSED NE "Y"'

        if (filter) {
            if (selectionclause)
                selectionclause += ' AND '
            filter = filter.replace(/["']/g, ' ').neosystrim().replace(/ /g, '" "')
            selectionclause += ' WITH DESCRIPTION1_UCASE CONTAINING "' + filter + '"'
        }

        /*
        if (!gistimesheetadmin)
        {
        selectionclause+=' AND WITH EXECUTIVE_CODE '+neosysquote(getvalue(USER_CODE))
        selectionclause+=' AND WITH DEPARTMENT_CODE '+neosysquote(gdepartmentcode)
        selectionclause+=' AND WITH COMPANY_CODE '+neosysquote('')
        }
        */

        selectionclause += ' BY CLIENT_NAME BY BRAND_NAME BY NUMBER '

        //  if (gdatafilename=='TIMESHEETS')
        //   var cols=[['BRAND_NAME','Brand'],['NUMBER','Job No'],['DESCRIPTION1','Job Description']]
        //  else
        //   var cols=[['BRAND_NAME','Brand'],['DESCRIPTION1','Job Description'],,['PERIOD','Period'],['NUMBER','Job No']]
        var cols = [['CLIENT_NAME', 'Client'], ['BRAND_NAME', 'Brand'], ['DESCRIPTION1', 'Job Description'], ['NUMBER', 'Job No']]

        if (!notclosed)
            cols[cols.length] = ['CLOSED', 'Closed']

        return neosysfilepopup('JOBS', cols, cols.length - 1, selectionclause, many)

    }

}

function jobs_costestimateprint(key, documenttype) {
    if (typeof documenttype == 'undefined') documenttype = 'COST'
    //windowopen('../jobs/costestimateprint.htm?DOCUMENT_TYPE='+documenttype+'&SAVEMODE=CLOSE',{'DOCUMENT_NO':escape(key)})
    windowopen('../jobs/costestimateprint.htm?DOCUMENT_TYPE=' + documenttype + '&SAVEMODE=CLOSE', { 'DOCUMENT_NO': key })
    return true
}

function open_job(key, mode) {
    if (mode == 'printsend')
        return openwindow('EXECUTE\rPRODUCTION\rPRINTJOB\r' + key)

    windowopen('../jobs/jobprint.htm', { 'JOB_NO': key })

    return true

}

function open_monthly_timesheet(date, username) {
    var date = getvalue('TIMESHEET_DATE')
    if (date == '') {
        return neosysinvalid('Please enter a date first')
    }
    var usercode = getvalue('USER_CODE')
    openwindow('EXECUTE\rPRODUCTION\rPRINTMONTHLYTIMESHEET\r' + date + '  ' + usercode)

    return true

}

//nb these are general job system data entry fields NOT dict_jobs()
function dict_JOB(parameters) {

    //returns an array representing the entry fields

    var dict = []//of dictrecs
    var din = -1

    var g = 0//group number

    if (!parameters.MODE) parameters.MODE = ''

    var di = dict[++din] = dictrec('MODE', 'F', 1)

    di = dict[++din] = dictrec('DOCUMENT_TYPE', 'F', 2)
    di.radio = '9;Jobs'
    di.required = true
    di.defaultvalue = '9'

    di = dict[++din] = dictrec('INFO_OPTIONS', 'F', 3)
    di.checkbox = '1;Estimate & Invoice Details:2;Profit Analysis'
    //di.defaultvalue = '"1:2"'
    di.defaultvalue = '"2"'

    /* 
    di=dict[++din]=dictrec('DOCUMENT_TYPE','F',2)
    di.radio='1;Plan:2;Schedule:3;Booking:4;Proforma:5;Invoice'
 
    di=dict[++din]=dictrec('COSTS', 'F',3)
    di.radio='1;Cost to Client:2;Cost to Agency:3;Cost to Client and Agency:4;No costs'
    di.required=true
    di.defaultvalue='1'
 
    di=dict[++din]=dictrec('CURRENCY', 'F',4, 'Currency',g,'','', '', '','L', '10')
    di.radio='1;Local and Base Currency:2;Local Currency:3;Base Currency'
    di.required=true
    di.defaultvalue='1'
    */

    //agencyfilter is reserved 21-99
    //?!field 40,42,43 overlap usage in jobsfilter and agencyfilter .. job filter could be moved to 50,52,53?
    dict = addagencyfilter(dict, parameters)
    dict = addjobsfilter(dict, parameters)

    return dict

}

function addjobsfilter(dict, parameters) {

    //?!field 40,42,43 overlap usage in jobsfilter and agencyfilter .. job filter could be moved to 50,52,53?

    //determine the last group no
    var g = 0
    for (var din = 0; din < dict.length; ++din) {
        if (dict[din].groupno > g) g = dict[din].groupno
    }

    //determine the number of entries
    var din = dict.length - 1
    var di

    di = dict[++din] = dictrec('JOB_NO', 'F', 40, '', ++g)
    //dict[i].filename='JOBS'
    di.popup = 'jobs_pop_jobno(true)'

    //was executive code move addagencyfilter
    //di=dict[++din]=dictrec('EXECUTIVE_CODE','F',41,'',++g)
    //agency_dict_executivecode(di,'JOBS',allownew=false,popmany=true)

    di = dict[++din] = dictrec('MASTER_JOB_NO', 'F', 42, '', ++g)
    //dict[i].filename='JOBS'
    // di.popup='jobs_pop_jobno(true)'
    di.popup = 'agency_search("JOBS",true)'

    di = dict[++din] = dictrec('STATUS', 'F', 43, '', ++g)
    di.popup = 'jobs_pop_statuses(true)'

    //note that "O" is "" in the Job File
    di = dict[++din] = dictrec('CLOSED', 'F', '44')
    //di.checkbox='O;Open:N;Reopened:Y;Closed'
    di.checkbox = '1;Open:2;Closed'
    di.horizontal = true
    //di.defaultvalue='" '+sm+'Y'+sm+'N"'

    return dict

}
