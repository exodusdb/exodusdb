//Copyright NEOSYS All Rights Reserved.//

async function form_postinit() {
    //exodussettimeout('focuson("NOTES")', 1000)
    gparameters.key='SYSTEM.CFG'
    return true
}

async function form_postdisplay() {
    showhide('databasesection', (!gexodus_server && (await gds.getall('SCOPE'))=='SYSTEM.CFG'))
    showhide(['backupsection','old_server'], !gexodus_server)
    return true
}

async function form_prewrite() {
    if ((await gds.getx('SCOPE'))=='SYSTEM.CFG' && !(await gds.getx('HOST_NAME')))
        return await exoui_invalid('Host Name is required when Scope is "Current Installation"')
    return true
}

async function sysconfig_copynow(event) {
    
    event=getevent(event)
    grecn = getrecn()
    
    var fromdatabase=await gds.get1('DATABASE_CODE')
    var todatabase=await gds.get1('TEST_DATABASE_CODE')

    if (!(await saveandunlockdoc()))
        return false

    var question='WARNING! You MUST NOT copy large databases while users are actively'
    question+='\nworking on and updating the database. Reporting is ok.'
    question+='\n\nThere is a serious risk of unpredictable damaged files in the live database and,'
    question+='\n although not a critical issue, damaged files in the target database.'
    question+='\n\nLarge, actively updated databases must only be copied using\nthe overnight "Automatic Copy" configuration option.'
    question+='\n\nOK to copy '+fromdatabase+' -> '+todatabase+' now?'
    if (!(await exoui_yesno(question,2)))
        return false
    
    db.request='EXECUTE\rGENERAL\rFILEMAN\rCOPYDB\r'+fromdatabase
    if (!(await db.send()))
        return await exoui_invalid(db.response)
    await exoui_note(db.response.slice(2))
}

async function dict_SYSTEMCONFIGURATION(parameters) {

    //returns an array representing the client dictionary 
    var dict = []//of dictrecs

    var din = -1
    var di
    groupn = 0

    di = dict[++din] = dictrec('SCOPE', 'F', 0)
    di.conversion = '..\\..\\SYSTEM.CFG;All Installations:SYSTEM.CFG;Current Installation:SYSTEM;Current Database'
    //di.radio = '..\\..\\SYSTEM.CFG;All Installations:SYSTEM.CFG;Current Installation:SYSTEM;Current Database'
    //di.horizontal = true
    di.defaultvalue = '"SYSTEM.CFG"'
    di.required = true
    // di.validation='configuration_val_scope()'

    di = dict[++din] = dictrec('IP_NUMBERS', 'F', 39)
    exo_dict_ipno(di, true, true, true)

    //eg a hostname like unilever will get ddns like unilever.hosts.neosys.com on zoneedit
    di = dict[++din] = dictrec('HOST_NAME', 'F', 57)
    di.validcharacters = 'abcdefghijklmnopqrstuwvxyz-0123456789';
    di.lowercase = true;
    di.length = 12;
    //di.required=true; only required in Scope: CURRENT INSTALLATION. checked in form_prewrite
    //if (gdataset!='BASIC') di.defaultvalue=gdataset.toLowerCase().exodusquote()

    groupn=1

    di = dict[++din] = dictrec('DATABASE_CODE', 'F', 58, '', groupn)
    await system_dict_datasetcode(di, true, false)
    //di.required=true;
    di.unique = true

    di = dict[++din] = dictrec('NUMBER_OF_PROCESSES', 'F', 59, '', groupn)
    exo_dict_integer(di, { min: 1, max: 9 })
    //di.required=true;
    if (gusername != 'EXODUS') di.readonly = true

    di = dict[++din] = dictrec('BACKUP_REQUIRED', 'F', 60, '', groupn)
    di.checkbox = '1;';

    di = dict[++din] = dictrec('COPY_REQUIRED', 'F', 62, '', groupn)
    di.checkbox='1;'

    di = dict[++din] = dictrec('TEST_DATABASE_CODE', 'F', 63, '', groupn)
    await system_dict_datasetcode(di, true, false, true)

    di = dict[++din] = dictrec('BACKUP_TIME_FROM', 'F', 73)
    exo_dict_time(di)
    di.length = 3
    //di.defaultvalue='02:00'.exodusiconv('[TIME]').exodusquote()

    di = dict[++din] = dictrec('BACKUP_TIME_UPTO', 'F', 74)
    exo_dict_time(di)
    di.length = 3
    //di.defaultvalue='02:05'.exodusiconv('[TIME]').exodusquote()

    di = dict[++din] = dictrec('BACKUP_DAY_OF_WEEK', 'F', 75)
    di.checkbox = '1;Mon:2;Tue:3;Wed:4;Thu:5;Fri:6;Sat:7;Sun'
    di.horizontal = true
    //di.defaultvalue='"1:2:3:4:5:6:7"'

    // Tech/alert emails (sysmsg To). ; separates addresses; after ;; is Cc (sendmail.cpp).
    di = dict[++din] = dictrec('BACKUP_EMAIL_ADDRESSES', 'F', 76)
    exo_dict_emailaddress(di, ';')

    di = dict[++din] = dictrec('BACKUP_DATABASE_TO', 'F', 77)
    exo_dict_diskdrive(di)

    di = dict[++din] = dictrec('BACKUP_UPLOADS_TO', 'F', 82)
    exo_dict_diskdrive(di)
    di.validcharacters += '0'

    di = dict[++din] = dictrec('LOG_EMAIL_ADDRESSES', 'F', 84)
    exo_dict_emailaddress(di, ';')

    di = dict[++din] = dictrec('SMTP_SENDER_EMAIL_ADDRESS', 'F', 101)
    exo_dict_emailaddress(di)

    //di=dict[++din]=dictrec('RESERVED','F',100)

    di = dict[++din] = dictrec('SMTP_HOST', 'F', 102)
    exo_dict_text(di)
    di.validcharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    di.validcharacters += di.validcharacters.toLowerCase()
    di.validcharacters += '0123456789-_.'

    di = dict[++din] = dictrec('SMTP_PORT', 'F', 103)
    exo_dict_integer(di, { min: 1, max: 65535 })
    //di.defaultvalue='"25"'

    di = dict[++din] = dictrec('SMTP_TIMEOUT_SECONDS', 'F', 104)
    exo_dict_integer(di)
    //di.defaultvalue='"60"'

    di = dict[++din] = dictrec('SMTP_USE_SSL', 'F', 105)
    di.conversion = '1;Yes:0;No'

    di = dict[++din] = dictrec('SMTP_AUTH_TYPE', 'F', 106)
    di.conversion = '0;Anonymous:1;Basic:2;NTLM'

    di = dict[++din] = dictrec('SMTP_USERNAME', 'F', 107)
    exo_dict_text(di)

    di = dict[++din] = dictrec('SMTP_PASSWORD', 'F', 108)
    exo_dict_text(di)

    di = dict[++din] = dictrec('NOTES', 'F', 110)
    exo_dict_text(di)

    groupn=2

    di = dict[++din] = dictrec('WEB_ADDRESS', 'F', 114, '', groupn)
    exo_dict_url(di)
    di.required = true

    di = dict[++din] = dictrec('WEB_ADDRESS_DESCRIPTION', 'F', 115, '', groupn)
    exo_dict_text(di)
    di.required = true

    di = dict[++din] = dictrec('EMAIL_DOMAINS', 'F', 116)
    exo_dict_text(di)
    di.validation = function sysconfig_val_EMAIL_DOMAINS() {
        var punctuation = "'" + '`!"$%^&*()_+=[]{};:@#~,<>/?\\|'
        //ie dont allow punctuation except space and -
        gvalue = gvalue.exodusconvert(punctuation, exodusspace(punctuation.length)).exodustrim()
        return true
    }

    /*
    di=dict[++din]=dictrec('','F',58,'',1)
    di.length=12;
    di.required=true;
    */

    // if (typeof exo_dict_colorfontsize!='undefined')
    {
        exo_dict_colorfontsize(dict, 46)
        din = dict.length - 1
    }

    di = dict[++din] = dictrec('USER_TIMEZONE', 'F', 118)
    for (var ii = -12.5; ii <= 12.5; ii += .5) {
        if (di.conversion)
            di.conversion += ':'
        else
            di.conversion += ''
        if (ii <= 0)
            tz = ii
        else if (ii > 0)
            tz = '+' + ii
        di.conversion += ii * 60 * 60 + ';' + tz
    }

    di = dict[++din] = dictrec('ADD_SIGNATURE', 'F', 119)
    di.conversion = ';Below(default):n;Do not add:a;Above:b;Below:l;Left of:r;Right of'

    di = dict[++din] = dictrec('AUTOMATIC_UPGRADE', 'F', 124)
    di.radio = '1;Yes:0;No:;Default (Yes)'
    di.horizontal=true

    di = dict[++din] = dictrec('CLOSE_AFTER_BACKUP', 'F', 125)
    di.radio = '1;Yes:0;No:;Default (Yes)'
    di.horizontal=true

    di = dict[++din] = dictrec('TESTDB_CAN_START_PROCESSES', 'F', 126)
    di.radio = '1;Yes:0;No:;Default (No)'
    di.horizontal=true

    di = dict[++din] = dictrec('MAX_NOLOGIN_DAYS', 'F', 128)
    exo_dict_integer(di, { max: 9999 })

    //defaults to 4428 in GBP, CONTROL
    //could be set to 443 to pass outgoing fw
    di = dict[++din] = dictrec('MONITOR_PORT_NO', 'F', 131)
    exo_dict_integer(di, { max: 65535 })

    //see also INIT.GENERAL for system<*>
    
    return dict

}
