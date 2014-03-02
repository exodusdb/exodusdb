//Copyright© 2008 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**//

function form_postinit() {
    window.setTimeout("focuson('NOTES')", 2000)
    neosyssetexpression('databasesection', 'style:display', 'gds.getx("SCOPE",null)=="SYSTEM.CFG"?"inline":"none"')
    return true
}

function sysconfig_copynow() {
    //determine row
    if (typeof rown == 'undefined')
        rown = getrecn(event.srcElement)
    var fromdatabase=gds.getx('DATABASE_CODE')[0]

    if (!saveandunlockdoc())
        return false
    
    var question='WARNING! You should not copy large databases while users are active other than reporting. There is a serious risk of unpredictable damaged files in the live database and, less importantly, damaged files in the target database. Such databases must only be copied automatically overnight after the backup\r\rOK to copy '+fromdatabase+' now?'
    if (!neosysyesno(question,2))
        return false
    
    db.request='EXECUTE\rGENERAL\rFILEMAN\rCOPYDB\r'+fromdatabase
    if (!db.send())
        return neosysinvalid(db.response)
    neosysnote(db.response.slice(2))
}

function dict_SYSTEMCONFIGURATION(parameters) {

    //returns an array representing the client dictionary 
    var dict = []//of dictrecs

    var din = -1
    var di
    groupn = 0

    di = dict[++din] = dictrec('SCOPE', 'F', 0)
    di.conversion = '..\\..\\SYSTEM.CFG;All Installations:SYSTEM.CFG;Current Installation:SYSTEM;Current Database'
    di.defaultvalue = '"SYSTEM.CFG"'
    di.required = true
    // di.validation='configuration_val_scope()'

    di = dict[++din] = dictrec('IP_NUMBERS', 'F', 39)
    neosys_dict_ipno(di, true, true)

    di = dict[++din] = dictrec('SYSTEM_ID', 'F', 57)
    di.validcharacters = 'abcdefghijklmnopqrstuwvxyz0123456789';
    di.lowercase = true;
    di.length = 12;
    //di.required=true;
    //if (gdataset!='BASIC') di.defaultvalue=gdataset.toLowerCase().neosysquote()

    groupn=1

    di = dict[++din] = dictrec('DATABASE_CODE', 'F', 58, '', groupn)
    general_dict_datasetcode(di, true, false)
    //di.required=true;
    di.unique = true

    di = dict[++din] = dictrec('NUMBER_OF_PROCESSES', 'F', 59, '', groupn)
    neosys_dict_number(di, 0, 1, 9)
    di.length = 3;
    di.align = 'left'
    //di.required=true;
    if (gusername != 'NEOSYS') di.readonly = true

    di = dict[++din] = dictrec('BACKUP_REQUIRED', 'F', 60, '', groupn)
    di.checkbox = '1;';

    di = dict[++din] = dictrec('COPY_REQUIRED', 'F', 62, '', groupn)
    di.checkbox='1;'

    di = dict[++din] = dictrec('TEST_DATABASE_CODE', 'F', 63, '', groupn)
    general_dict_datasetcode(di, true, false)

    di = dict[++din] = dictrec('BACKUP_TIME_FROM', 'F', 73)
    neosys_dict_time(di)
    di.length = 3
    //di.defaultvalue='02:00'.neosysiconv('[TIME]').neosysquote()

    di = dict[++din] = dictrec('BACKUP_TIME_UPTO', 'F', 74)
    neosys_dict_time(di)
    di.length = 3
    //di.defaultvalue='02:05'.neosysiconv('[TIME]').neosysquote()

    di = dict[++din] = dictrec('BACKUP_DAY_OF_WEEK', 'F', 75)
    di.checkbox = '1;Mon:2;Tue:3;Wed:4;Thu:5;Fri:6;Sat:7;Sun'
    di.horizontal = true
    //di.defaultvalue='"1:2:3:4:5:6:7"'

    di = dict[++din] = dictrec('BACKUP_EMAIL_ADDRESSES', 'F', 76)
    neosys_dict_emailaddress(di, ';')

    di = dict[++din] = dictrec('BACKUP_DATABASE_TO', 'F', 77)
    neosys_dict_diskdrive(di)

    di = dict[++din] = dictrec('BACKUP_UPLOADS_TO', 'F', 82)
    neosys_dict_diskdrive(di)
    di.validcharacters += '0'

    di = dict[++din] = dictrec('LOG_EMAIL_ADDRESSES', 'F', 84)
    neosys_dict_emailaddress(di, ';')

    di = dict[++din] = dictrec('SMTP_SENDER_EMAIL_ADDRESS', 'F', 101)
    neosys_dict_emailaddress(di)

    //di=dict[++din]=dictrec('RESERVED','F',100)

    di = dict[++din] = dictrec('SMTP_HOST', 'F', 102)
    neosys_dict_text(di)
    di.validcharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    di.validcharacters += di.validcharacters.toLowerCase()
    di.validcharacters += '0123456789-_.'

    di = dict[++din] = dictrec('SMTP_PORT', 'F', 103)
    neosys_dict_number(di, 0, 1, 65535)
    di.length = 5
    //di.defaultvalue='"25"'

    di = dict[++din] = dictrec('SMTP_TIMEOUT_SECONDS', 'F', 104)
    neosys_dict_number(di, 0, 0)
    di.length = 3
    //di.defaultvalue='"60"'

    di = dict[++din] = dictrec('SMTP_USE_SSL', 'F', 105)
    di.conversion = '1;Yes:0;No'

    di = dict[++din] = dictrec('SMTP_AUTH_TYPE', 'F', 106)
    di.conversion = '0;Anonymous:1;Basic:2;NTLM'

    di = dict[++din] = dictrec('SMTP_USERNAME', 'F', 107)
    neosys_dict_text(di)

    di = dict[++din] = dictrec('SMTP_PASSWORD', 'F', 108)
    neosys_dict_text(di)

    di = dict[++din] = dictrec('NOTES', 'F', 110)
    neosys_dict_text(di)

    groupn=2

    di = dict[++din] = dictrec('WEB_ADDRESS', 'F', 114, '', groupn)
    neosys_dict_url(di)
    di.required = true

    di = dict[++din] = dictrec('WEB_ADDRESS_DESCRIPTION', 'F', 115, '', groupn)
    neosys_dict_text(di)
    di.required = true

    di = dict[++din] = dictrec('EMAIL_DOMAINS', 'F', 116)
    neosys_dict_text(di)
    di.validation = function () {
        var punctuation = "'" + '`¬!"£$%^&*()_+=[]{};:@#~,<>/?\\|'
        //ie dont allow punctuation except space and -
        gvalue = gvalue.neosysconvert(punctuation, neosysspace(punctuation.length)).neosystrim()
        return true
    }

    /*
    di=dict[++din]=dictrec('','F',58,'',1)
    di.length=12;
    di.required=true;
    */

    // if (typeof neosys_dict_colorfontsize!='undefined')
    {
        neosys_dict_colorfontsize(dict, 46)
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

    return dict

}
