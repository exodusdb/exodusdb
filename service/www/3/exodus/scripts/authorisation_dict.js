// Copyright NEOSYS All Rights Reserved.

//partial copy betweem authorisation_dict and hourlyrates_dict
function* dict_AUTHORISATION(parameters) {

    var dict = []
    var di
    var din = -1

    di = dict[++din] = dictrec('KEY', 'F', 0)
    di.defaultvalue = '"SECURITY"'
    di.required = true
    di.listfunction = 'yield* authorisation_print()'

    ///users f1-8 are parallel (maybe partial section of mvs for some users)
    var usersgroupn = 1

    di = dict[++din] = dictrec('USER_ID', 'F', 1, '', usersgroupn)
    di.required = true
    di.validation = 'yield* user_val_userid()'
    di.link = 'yield* windowopenkey("../exodus/users.htm")'
    //need to allow to move user up/down
    //di.unique=true
    di.noinsertrow = !(yield* exodussecurity('USER CREATE'))
    //allow deleting blank rows if allowed to create users but how to stop them creating new groups or joining groups 
    di.nodeleterow = !(yield* exodussecurity('USER DELETE')) && di.noinsertrow
    //di.validcharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890-'
    di.validcharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-'
    di.allowemptyrows = true

    di = dict[++din] = dictrec('KEYS', 'F', 2, '', usersgroupn)
    if (!(yield* exodussecurity('AUTHORISATION UPDATE GROUPS'))) di.readonly = gmsg
    exodus_dict_text(di)
    di.length = 40
    di.lowercase = false
    di.validation = 'yield* user_val_keys()'
    //prevent accepting bad white space characters like 160 nbsp which dont act as key separators (on LOCKS and KEYS)
    di.validcharacters='ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_ '

    //moved to user<34> for legacy privileges
    //di=dict[++din]=dictrec('MENU','F',3,'',1)

    di = dict[++din] = dictrec('EXPIRY_DATE', 'F', 3, '', usersgroupn)
    exodus_dict_date(di)
    di.align = 'L'

    di = dict[++din] = dictrec('PASSWORD', 'F', 4, '', usersgroupn)
    di.required = true

    di = dict[++din] = dictrec('HOURLY_RATE', 'F', 5, '', usersgroupn)

    di = dict[++din] = dictrec('IP_NUMBERS', 'F', 6, '', usersgroupn)
    if (!(yield* exodussecurity('AUTHORISATION UPDATE GROUPS'))) di.readonly = gmsg
    exodus_dict_ipno(di, true, true, true)
    di.length = 40

    di = dict[++din] = dictrec('EMAIL_ADDRESS', 'F', 7, '', usersgroupn)
    exodus_dict_emailaddress(di, ';')

    di = dict[++din] = dictrec('USER_NAME', 'F', 8, '', usersgroupn)
    exodus_dict_text(di)

    //was used to buffer valid companies by user for validcode()
    //di=dict[++din]=dictrec('FIELD9','F',9)

    ///tasks
    var tasksgroupn = 2

    di = dict[++din] = dictrec('TASK_ID', 'F', 10, '', tasksgroupn)
    di.required = true
    di.unique = true
    di.length = 40
    di.validation = 'yield* task_val_taskid()'
    if (!(yield* exodussecurity('AUTHORISATION UPDATE LOCKS'))) {
        di.readonly = gmsg
        di.nodeleterow = true
        di.noinsertrow = true
    }

    di = dict[++din] = dictrec('HIDDEN_LINES', 'S', '', '', tasksgroupn)
    di.functioncode = function* authorisation_HIDDEN_LINES() {
        var hides = ['0']//dont hide first item
        var taskcodes = yield* gds.getall('TASK_ID')
        var lasttaskcode = ''
        for (var taskn = 0; taskn < taskcodes.length; taskn++) {
            var thistaskcode = taskcodes[taskn] = taskcodes[taskn].split(' ')[0]
            if (taskn > 0) hides[taskn] = thistaskcode == lasttaskcode ? '1' : '0'
            lasttaskcode = thistaskcode
        }
        return hides
    }

    di = dict[++din] = dictrec('LOCKS', 'F', 11, '', tasksgroupn)
    di.validation = 'yield* task_val_lock()'
    if (!(yield* exodussecurity('AUTHORISATION UPDATE LOCKS'))) di.readonly = gmsg
    //prevent accepting bad white space characters like 160 nbsp which dont act as key separators (on LOCKS and KEYS)
    di.validcharacters='ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_ '

    ///other
    var groupn = 3

    di = dict[++din] = dictrec('TEMP_FIRST_LINE', 'F', 20)
    di = dict[++din] = dictrec('TEMP_LAST_LINE', 'F', 21)
    di = dict[++din] = dictrec('TEMP_MENUS', 'F', 22, '', groupn)

    //other users/keys comma separated (to restriction creation of)
    di = dict[++din] = dictrec('TEMP_OTHER_USERS', 'F', 23)
    di = dict[++din] = dictrec('TEMP_OTHER_KEYS', 'F', 24)

    di = dict[++din] = dictrec('PASSWORD_AUTOEXPIRY_DAYS', 'F', 25)
    exodus_dict_number(di, 0, 0)
    di.length = 3
    if (!(yield* exodussecurity('AUTHORISATION UPDATE LOCKS'))) di.readonly = gmsg

    di = dict[++din] = dictrec('EMAIL_NEW_USERS', 'F', 26)
    di.conversion = ';Ask:1;Yes:0;No'

    di = dict[++din] = dictrec('LAST_LOGIN_DATE_TIME', 'S', '', '', usersgroupn)
    exodus_dict_datetime(di)
    di.functioncode = 'return yield* gds.xlate("USER_ID", "USERS",13)'

    di = dict[++din] = dictrec('LAST_LOGIN_LOCATION', 'S', '', '', usersgroupn)
    di.functioncode = 'return yield* gds.xlate("USER_ID", "USERS",14)'

    return dict

}
