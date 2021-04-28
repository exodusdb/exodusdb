//Copyright NEOSYS All Rights Reserved.

//moved from general.js

function* system_dict_datasetcode(di, many, orcurrent, test) {
    many = many || false
    orcurrent = orcurrent || false
    //test=test||false
    //"" means no restriction, true=only TEST databases, false=not TEST databases
    if (typeof test == 'undefined')
        test = '""'
    di.popup = 'yield* system_pop_datasetcode(' + many + ',' + orcurrent + ')'
    di.validation = 'yield* system_val_datasetcode(' + many + ',' + orcurrent + ',' + test + ')'
    di.length = 8;
}

function* system_pop_datasetcode(many, orcurrent) {
    if (!(yield* system_getdatasets())) return false

    var popdata = exoduscloneobj(gdatasets)
    if (orcurrent) {
        popdata[0].splice(0, 0, 'Current')
        popdata[1].splice(0, 0, 'CURRENT')
    }

    var tt = ''
    if (popdata.join().indexOf('*') >= 0)
        tt = '\r(*=not available)'
    var cols = [[0, 'Name'], [1, 'Code']]
    var returncoln = 1
    var defaultreply = ''
    var inverted = true
    var datasetcode = yield* exodusdecide('Which dataset' + (many ? '(s' : '') + ' do you want?' + tt, popdata, cols, returncoln, defaultreply, many, inverted)
    return datasetcode
}

function* system_val_datasetcode(many, orcurrent, test) {

    if (gvalue == 'CURRENT') {
        if (!orcurrent) return yield* exodusinvalid('"CURRENT" is not allowed here')
        return true
    }

    if (!(yield* system_getdatasets()))
        return false

    if (gvalue != 'CURRENT' && gvalue.indexOf('CURRENT') >= 0)
        return yield* exodusinvalid('You cannot choose CURRENT and other datasets')

    if (typeof test == 'boolean') {
        if (test && gvalue && gvalue.substr(-4) != 'TEST')
            return yield* exodusinvalid('You can only choose TEST databases here')
        else if (!test && gvalue.substr(-4) == 'TEST')
            return yield* exodusinvalid('You cannot choose TEST databases here')
    }

    var values = many ? gvalue.split(':') : [gvalue]
    for (var ii = 0; ii < values.length; ii++) {
        if (!gdatasets[1].exoduslocate(values[ii]))
            return yield* exodusinvalid(values[ii].exodusquote() + ' is not a valid dataset code')
    }

    return true

}

var gdatasets
function* system_getdatasets(refresh) {
    if (refresh || !gdatasets) {
        db.request = 'EXECUTE\rGENERAL\rGETDATASETS'
        if (!(yield* db.send())) return yield* exodusinvalid(db.response)
        //split inverted
        gdatasets = db.data.exodussplit(vm + sm, true)
    }
    return gdatasets.length > 0
}

//users and security
////////////////////

function* system_dict_usercode(di, many, withtask, haslocks, sselect) {
    if (!many)
        many = false
    if (many && !di.groupno)
        systemerror(di.name, '"Select Many" requires group>0')
    if (typeof withtask == 'undefined' || withtask == '')
        withtask = '' // '""'
    if (typeof haslocks == 'undefined' || haslocks == '')
        haslocks = '""'
    if (typeof sselect == 'undefined' || sselect == '')
        sselect = '""'
    if ("'\"".indexOf(sselect.substr(0, 1)) == -1)
        sselect = '"' + sselect.exodusswap('"', '\\"') + '"'
    di.popup = 'yield* system_pop_users(' + many + ',"' + withtask + '",' + haslocks + ',' + sselect + ')'
    di.filename = 'USERS'
    di.validation = 'yield* system_val_users()'
    if (many)
        di.unique = true
}

function* system_pop_users(many, withtask, haslocks, sselect) {
    var sortselect = ' AND WITH ID NOT STARTING "%"'
    //currently available "authorisation groups" supported - easy to add any you like in DICT.USERS
    //AUTHORISED_JOURNAL_POST
    //AUTHORISED_TIMESHEET_ADMINISTRATION
    if (sselect)
        sortselect += ' AND ' + sselect
    if (withtask)
        sortselect += ' AND WITH AUTHORISED_' + withtask.exodusconvert(' ', '_').toUpperCase()
    //users tend not to have locks and departments/group tend to have locks
    if (typeof haslocks == 'boolean')
/       sortselect += ' AND WITH KEYS ' + (haslocks ? 'NE' : 'EQ') + ' ""'
//        sortselect += ' AND WITH KEYS ' + (haslocks ? 'EQ' : 'NE') + ' ""'
    sortselect = 'BY RANK ' + sortselect.slice(5)
    var selcol0 = 1

    return yield* exodusfilepopup('USERS', [['USER_NAME', 'User Name'], ['USER_CODE', 'User Code'], ['DEPARTMENT_CODE2', 'Department'], ['EMAIL_ADDRESS', 'Email'], ['LAST_LOGIN_DATETIME', 'Last Login Datetime'], ['LAST_LOGIN_LOCATION', 'Last Login Location']], selcol0, sortselect, many)
}

function* system_val_users() {
    return true
}

function* system_dict_username(di, usercodeid) {
    if (typeof usercodeid == 'undefined') usercodeid = 'USER_CODE'
    di.functioncode = 'return yield* this.xlate("' + usercodeid + '", "USERS",1)'
    di.length = 30
}

/*
function* system_getdepartments_old() {
db.request = 'EXECUTE\rGENERAL\rGETDEPTS'
if (!(yield* db.send())) return yield* exodusinvalid(db.response)
gdepartments = db.data
}
*/
var gdepartments = ''
var gdepts//0=deptids, 1=deptnames, 2=deptusernames
function* system_getdepartments(deptoptions) {
    if (!deptoptions)
        deptoptions = ''

    //departments ending with digits like STUDIO2 may be treated like STUDIO
    var withnumbers = deptoptions.indexOf('N') >= 0
    var onlyactivegroups = deptoptions.indexOf('A') >= 0

    var security = []
    if (!(yield* security.exodusread('DEFINITIONS', 'SECURITY*USERS')))
        return yield* exodusinvalid(security.exodusresponse)
    security = exodus_splitarray(security, [[[1, 9]], [[10, 11]]])
    gdepts = [[], [], []]
    var nusers = security[1].length
    var groupusers = []
    var datetoday = exodusdate()
    //for (var usern = 2; usern <= nusers; ++usern) {
    for (var usern = 0; usern <= nusers; ++usern) {
        var userid = security[1][usern]
        if (!userid || userid == '---' || userid == '') {
            //optionally skip groups with no users
            if (onlyactivegroups && groupusers.length == 0)
                continue
            deptid = security[1][usern - 1]
            if (!withnumbers)
                deptid = deptid.exodusconvert('0123456789', '').exodustrim()
            if (deptid != '---' && deptid != '') {
                if (!(gdepts[0].exoduslocate(deptid))) {
                    gdepts[0].push(deptid)
                    gdepts[1].push(security[8][usern - 1])
                    gdepts[2].push(groupusers.join(','))
                }
            }
            groupusers = []
        } else {
            //count active users with emails
            if (security[7][usern]) {
                var expired = Number(security[3][usern])
                if (!expired || expired > datetoday) {
                    var username = security[8][usern]
                    if (!username)
                        username = userid
                    groupusers.push(username)
                }
            }
        }

    }
    gdepartments = gdepts[0].join(fm)
    return true
}

function* system_dict_departmentcode(di, many, deptoptions) {
    if (!many)
        many = false
    if (!deptoptions)
        deptoptions = ''
    di.popup = 'yield* system_pop_department(' + many + ',' + deptoptions.exodusquote() + ')'
    di.validation = 'yield* system_val_department(' + deptoptions.exodusquote() + ')'
}

function* system_pop_department(many, deptoptions) {
    if (!many)
        many = false
    if (!(yield* system_getdepartments(deptoptions)))
        return false
    //return yield* exodusdecide('', gdepartments.split(fm), '', 0, '', many)
    var cols = [[0, 'Department'], [1, 'Name'], [2, 'Users with email']]
    return yield* exodusdecide('', gdepts, cols, 0, gvalue, many, true)
}

function* system_val_department(deptoptions) {
    if (!(yield* system_getdepartments(deptoptions)))
        return false
    if (gvalue && !gdepartments.split(fm).exoduslocate(gvalue))
        return yield* exodusinvalid(gvalue + ' department does not exist')
    return true
}
