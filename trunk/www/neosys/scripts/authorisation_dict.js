// © 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

//partial copy betweem authorisation_dict and hourlyrates_dict
function dict_AUTHORISATION(parameters) {

    var dict = []
    var di
    var din = -1

    di = dict[++din] = dictrec('KEY', 'F', 0)
    di.defaultvalue = '"SECURITY"'
    di.required = true
    di.listfunction = 'authorisation_print()'

    ///users f1-8 are parallel (maybe partial section of mvs for some users)
    var usersgroupn = 1

    di = dict[++din] = dictrec('USER_ID', 'F', 1, '', usersgroupn)
    di.required = true
    di.validation = 'user_val_userid()'
    di.link = 'windowopen2("../neosys/users.htm")'
    //need to allow to move user up/down
    //di.unique=true
    di.noinsertrow = !neosyssecurity('USER CREATE')
    //allow deleting blank rows if allowed to create users but how to stop them creating new groups or joining groups 
    di.nodeleterow = !neosyssecurity('USER DELETE') && di.noinsertrow
    di.validcharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890-'
    di.allowemptyrows = true

    di = dict[++din] = dictrec('KEYS', 'F', 2, '', usersgroupn)
    if (!neosyssecurity('AUTHORISATION UPDATE GROUPS')) di.readonly = gmsg
    neosys_dict_text(di)
    di.length = 40
    di.lowercase = false
    di.validation = 'user_val_keys()'

    //moved to user<34> for legacy privileges
    //di=dict[++din]=dictrec('MENU','F',3,'',1)

    di = dict[++din] = dictrec('EXPIRY_DATE', 'F', 3, '', usersgroupn)
    neosys_dict_date(di)
    di.align = 'L'

    di = dict[++din] = dictrec('PASSWORD', 'F', 4, '', usersgroupn)
    di.required = true

    di = dict[++din] = dictrec('HOURLY_RATE', 'F', 5, '', usersgroupn)

    di = dict[++din] = dictrec('IP_NUMBERS', 'F', 6, '', usersgroupn)
    if (!neosyssecurity('AUTHORISATION UPDATE GROUPS')) di.readonly = gmsg
    neosys_dict_ipno(di, true, true)
    di.length = 40

    di = dict[++din] = dictrec('EMAIL_ADDRESS', 'F', 7, '', usersgroupn)
    neosys_dict_emailaddress(di, ';')

    di = dict[++din] = dictrec('USER_NAME', 'F', 8, '', usersgroupn)
    neosys_dict_text(di)

    //was used to buffer valid companies by user for validcode()
    //di=dict[++din]=dictrec('FIELD9','F',9)

    ///tasks
    var tasksgroupn = 2

    di = dict[++din] = dictrec('TASK_ID', 'F', 10, '', tasksgroupn)
    di.required = true
    di.unique = true
    di.length = 40
    di.validation = 'task_val_taskid()'
    if (!neosyssecurity('AUTHORISATION UPDATE LOCKS')) {
        di.readonly = gmsg
        di.nodeleterow = true
        di.noinsertrow = true
    }

    di = dict[++din] = dictrec('HIDDEN_LINES', 'S', '', '', tasksgroupn)
    di.functioncode = function authhidelines() {
        var hides = ['0']//dont hide first item
        var taskcodes = gds.getx('TASK_ID', null)
        var lasttaskcode = ''
        for (var taskn = 0; taskn < taskcodes.length; taskn++) {
            var thistaskcode = taskcodes[taskn] = taskcodes[taskn].split(' ')[0]
            if (taskn > 0) hides[taskn] = thistaskcode == lasttaskcode ? '1' : '0'
            lasttaskcode = thistaskcode
        }
        return hides
    }

    di = dict[++din] = dictrec('LOCKS', 'F', 11, '', tasksgroupn)
    di.validation = 'task_val_lock()'
    if (!neosyssecurity('AUTHORISATION UPDATE LOCKS')) di.readonly = gmsg

    ///other
    var groupn = 3

    di = dict[++din] = dictrec('TEMP_FIRST_LINE', 'F', 20)
    di = dict[++din] = dictrec('TEMP_LAST_LINE', 'F', 21)
    di = dict[++din] = dictrec('TEMP_MENUS', 'F', 22, '', groupn)

    //other users/keys comma separated (to restriction creation of)
    di = dict[++din] = dictrec('TEMP_OTHER_USERS', 'F', 23)
    di = dict[++din] = dictrec('TEMP_OTHER_KEYS', 'F', 24)

    di = dict[++din] = dictrec('PASSWORD_AUTOEXPIRY_DAYS', 'F', 25)
    neosys_dict_number(di, 0, 0)
    di.length = 3
    if (!neosyssecurity('AUTHORISATION UPDATE LOCKS')) di.readonly = gmsg

    di = dict[++din] = dictrec('EMAIL_NEW_USERS', 'F', 26)
    di.conversion = ';Ask:1;Yes:0;No'

    di = dict[++din] = dictrec('LAST_LOGIN_DATE_TIME', 'S', '', '', usersgroupn)
    neosys_dict_datetime(di)
    di.functioncode = 'return gds.getx("USER_ID").neosysxlate("USERS",13)'

    di = dict[++din] = dictrec('LAST_LOGIN_LOCATION', 'S', '', '', usersgroupn)
    di.functioncode = 'return gds.getx("USER_ID").neosysxlate("USERS",14)'

    return dict

}
