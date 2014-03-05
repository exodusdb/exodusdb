//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

function dict_TIMESHEETS() {

    gistimesheetadmin = neosyssecurity('TIMESHEET ADMINISTRATION')

    //returns an array representing the client dictionary

    var dict = []//of dictrecs
    var din = -1
    var di

    //header

    var groupn = 0//group number

    di = dict[++din] = dictrec('USER_CODE', 'F', '0', 'User name', '', '1', '', '', '', 20)
    //di.popup='general_pop_users()'
    //di.filename='USERS'
    di.conversion = ';'//force a dropdown to be filled later
    di.defaultvalue = 'timesheet_def_usercode()'
    di.validation = 'timesheet_val_usercode()'
    di.validcharacters = ' '
    di.printfunction = 'open_monthly_timesheet()'
    di.required = true

    di = dict[++din] = dictrec('TIMESHEET_DATE', 'F', '0', 'Date', '', '2')
    neosys_dict_date(di)
    // di.defaultvalue='TODAY'
    // di.defaultvalue='"0"'
    // di.required=true

    di = dict[++din] = dictrec('DAY', 'S', '0', 'Day', '')
    di.functioncode = 'return this.getx("TIMESHEET_DATE").neosysoconv("[DATE,DAYNAME]")'

    di = dict[++din] = dictrec('TOTAL_HOURS', 'S', '', 'Total Hours', '', '', '', '[PERIOD_OF_TIME]', '', 'R', 10)
    di.functioncode = 'return this.getx("HOURS").neosyssum()'

    //lines

    groupn = 1
    di = dict[++din] = dictrec('JOB_NO', 'F', '1', 'Job No', groupn, '', '', '', '', 'L', 10)
    jobs_dict_jobno(di, many = false, onlyopen = true)
    di.popup = 'jobs_pop_jobno(many = !gds.getx("JOB_NO", null).join(),onlyopen=true)'
    //dont use usual link since it opens the file
    di.link = 'open_job(gvalue,"printsend")'
    di.required = true

    //will be checked in timesheet_val_job()
    di.filename = ''
    di.validation = 'timesheet_val_job()'

    //di=dict[++din]=dictrec('JOB_DESCRIPTION','F','100','Job Description',groupn,'','','','','T',30)
    //di.readonly=true
    /* 
    di.functioncode=function ()
    {
    var descs=this.getx("JOB_NO").neosysxlate("JOBS",9)

    //the description is the first line of the brief
    for (var i=0;i<descs.length;i++)
    {
    descs[i]=descs[i].split(vm)[0].split(tm)[0]
    }
  
    return descs
  
    }
    */
    di = dict[++din] = dictrec('JOB_DESCRIPTION', 'S', '', '', groupn)
    neosys_dict_text(di)
    di.functioncode = 'return this.getx("JOB_NO").neosysxlate("JOBS",9).neosysfield(tm,1)'

    //di=dict[++din]=dictrec('BRAND_NAME','F','101','Brand Name',groupn,'','','','','T',30)
    //di.readonly=true
    di = dict[++din] = dictrec('BRAND_NAME', 'S', '', '', groupn)
    agency_dict_brandname(di)
    di.functioncode = 'return this.getx("JOB_NO").neosysxlate("JOBS",2).neosysxlate("BRANDS",2)'

    di = dict[++din] = dictrec('DETAILS', 'F', '3', 'Details', groupn, '', '', '', '', 'T', 30)

    di = dict[++din] = dictrec('ACTIVITY_CODE', 'F', '4', 'Activity Code', groupn, '', '', '', '', 'L', 3)
    //di.filename='MEDIA.TYPES'//no need to check as is a dropdown
    //di.popup='agency_pop_jobtypes()'
    // di.dropdown='SELECT\rMEDIA.TYPES\rWITH PRODUCTION "Y"\rCODE NAME\rXML'+fm+'CODE\rNAME'
    // di.dropdown='EXECUTE\rPRODUCTION\rGETUSERACTIVITIES'+fm+'CODE\rNAME'
    di.conversion = ';'//force a dropdown
    //di.required=true//depends on parameters
    di.required = true

    di = dict[++din] = dictrec('ACTIVITY_NAME', 'S', '', 'Activity Name', groupn)
    di.functioncode = 'return this.getx("ACTIVITY_CODE").neosysxlate("MEDIA.TYPES",1)'

    di = dict[++din] = dictrec('HOURS', 'F', '2', 'Hours', groupn, '', '', '[PERIOD_OF_TIME]', '', 'L', 5)
    di.validation = 'timesheet_val_hours()'

    di = dict[++din] = dictrec('DATE_TIME', 'F', '6')
    neosys_dict_datetimeupdated(di)

    di = dict[++din] = dictrec('VERSION', 'F', 7)

    //added to cost side 31/3/2006
    di = dict[++din] = dictrec('STATUS', 'F', 8)
    di.copyable = false
    if (gistimesheetadmin) {
        di.required = true
        di.conversion = 'DRAFT;Draft:APPROVED;Approved'
    }
    else
        di.readonly = true

    groupn = 3
    general_dict_addlogfields(dict, groupn, 30, 'general_open_version()')
    din = dict.length - 1

    return (dict)

}