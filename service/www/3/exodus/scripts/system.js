//Copyright NEOSYS All Rights Reserved.

//moved from general.js

async function system_dict_datasetcode(di, many, orcurrent, test) {
    many = many || false
    orcurrent = orcurrent || false
    //test=test||false
    //"" means no restriction, true=only TEST databases, false=not TEST databases
    if (typeof test == 'undefined')
        test = '""'
    exo_dict_code(di)
    // dataset codes keep natural case (not forced uppercase like most codes)
    di.lowercase = true
    di.popup = 'await system_pop_datasetcode(' + many + ',' + orcurrent + ')'
    di.validation = 'await system_val_datasetcode(' + many + ',' + orcurrent + ',' + test + ')'
}

async function system_pop_datasetcode(many, orcurrent) {
    if (!(await system_getdatasets())) return false

    var popdata = exocloneobj(gdatasets)
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
    var datasetcode = await exoui_decide('Which dataset' + (many ? '(s' : '') + ' do you want?' + tt, popdata, cols, returncoln, defaultreply, many, inverted)
    return datasetcode
}

async function system_val_datasetcode(many, orcurrent, test) {

    if (gvalue == 'CURRENT') {
        if (!orcurrent) return await exoui_invalid('"CURRENT" is not allowed here')
        return true
    }

    if (!(await system_getdatasets()))
        return false

    if (gvalue != 'CURRENT' && gvalue.indexOf('CURRENT') >= 0)
        return await exoui_invalid('You cannot choose CURRENT and other datasets')

    if (typeof test == 'boolean') {
        if (test && gvalue && gvalue.substr(-5) != '_test')
            return await exoui_invalid('You can only choose TEST databases here')
        else if (!test && gvalue.substr(-5) == '_test')
            return await exoui_invalid('You cannot choose TEST databases here')
    }

    var values = many ? gvalue.split(':') : [gvalue]
    for (var ii = 0; ii < values.length; ii++) {
        if (!gdatasets[1].exolocate(values[ii]))
            return await exoui_invalid(values[ii].exoquote() + ' is not a valid dataset code')
    }

    return true

}

var gdatasets
async function system_getdatasets(refresh) {
    if (refresh || !gdatasets) {
        db.request = 'EXECUTE\rGENERAL\rGETDATASETS'
        if (!(await db.send())) return await exoui_invalid(db.response)
        //split inverted
        gdatasets = db.data.exosplit(vm + sm, true)
    }
    return gdatasets.length > 0
}

//users and security
////////////////////

// Same SELECT clauses as system_pop_users / system_typeahead_users (BY RANK + filters).
// Always skip expired users (dict.users LIVE_USER: EXPIRY_DATE empty or > today).
function system_users_sortselect(withtask, haslocks, sselect) {
    var sortselect = ' AND WITH ID NOT STARTING "%"'
    sortselect += ' AND WITH LIVE_USER'
    if (sselect)
        sortselect += ' AND ' + sselect
    if (withtask)
        sortselect += ' AND WITH AUTHORISED_' + String(withtask).exoconvert(' ', '_').toUpperCase()
    if (typeof haslocks == 'boolean')
        sortselect += ' AND WITH KEYS ' + (haslocks ? 'NE' : 'EQ') + ' ""'
    return 'BY RANK ' + sortselect.slice(5)
}

async function system_dict_usercode(di, many, withtask, haslocks, sselect) {
    if (!many)
        many = false
    if (many && !di.groupno)
        systemerror(di.name, '"Select Many" requires group>0')
    //if (typeof withtask == 'undefined' || withtask == '')
    if (typeof withtask == 'undefined' || withtask === '')
        withtask = '' // '""'
    //if (typeof haslocks == 'undefined' || haslocks == '')
    //if (typeof haslocks == 'undefined' || haslocks === '')
    if (typeof haslocks != 'boolean')
        haslocks = '""'
    //if (typeof sselect == 'undefined' || sselect == '')
    if (typeof sselect == 'undefined' || sselect === '')
        sselect = '""'
    if ("'\"".indexOf(sselect.substr(0, 1)) == -1)
        sselect = '"' + sselect.exoswap('"', '\\"') + '"'
    di.popup = 'await system_pop_users(' + many + ',"' + withtask + '",' + haslocks + ',' + sselect + ')'
    // Live typeahead via existing general_typeahead_master (session list + wordstart)
    di.onchange = 'await system_typeahead_users("' + withtask + '",' + haslocks + ',' + sselect + ')'
    di.filename = 'USERS'
    di.validation = 'await system_val_users()'
    if (many)
        di.unique = true
}

async function system_pop_users(many, withtask, haslocks, sselect) {
    var sortselect = system_users_sortselect(withtask, haslocks, sselect)
    var selcol0 = 1

    return await exofilepopup('USERS', [['USER_NAME', 'User Name'], ['USER_CODE', 'User Code'], ['DEPARTMENT_CODE2', 'Department'], ['EMAIL_ADDRESS', 'Email'], ['LAST_LOGIN_DATETIME', 'Last Login Datetime'], ['LAST_LOGIN_LOCATION', 'Last Login Location']], selcol0, sortselect, many)
}

// USER_CODE typeahead: reuse general_typeahead_master (CACHE SELECT + wordstart). Needs general.js.
async function system_typeahead_users(withtask, haslocks, sselect) {
    var cols = [
        ['USER_CODE', 'User Code'],
        ['USER_NAME', 'User Name'],
        ['DEPARTMENT_CODE2', 'Department'],
        ['EMAIL_ADDRESS', 'Email']
    ]
    return await general_typeahead_master('USERS', system_users_sortselect(withtask, haslocks, sselect), cols, 0)
}

async function system_val_users() {
    return true
}

async function system_dict_username(di, usercodeid) {
    if (typeof usercodeid == 'undefined') usercodeid = 'USER_CODE'
    di.functioncode = 'return await this.xlate("' + usercodeid + '", "USERS",1)'
    di.length = 30
}

var gdepartments = ''
var gdepts//0=deptids, 1=deptnames, 2=deptusernames
async function system_getdepartments(deptoptions) {
    if (!deptoptions)
        deptoptions = ''

    //departments ending with digits like STUDIO2 may be treated like STUDIO
    var withnumbers = deptoptions.indexOf('N') >= 0
    var onlyactivegroups = deptoptions.indexOf('A') >= 0

    var security = []
    if (!(await security.exoread('DEFINITIONS', 'SECURITY*USERS')))
        return await exoui_invalid(security.exodusresponse)
    security = exo_splitarray(security, [[[1, 9]], [[10, 11]]])
    gdepts = [[], [], []]
    var nusers = security[1].length
    var groupusers = []
    var datetoday = exodate()
    //for (var usern = 2; usern <= nusers; ++usern) {
    for (var usern = 0; usern <= nusers; ++usern) {
        var userid = security[1][usern]
        if (!userid || userid == '---' || userid == '') {
            //optionally skip groups with no users
            if (onlyactivegroups && groupusers.length == 0)
                continue
            deptid = security[1][usern - 1]
            if (!withnumbers)
                deptid = deptid.exoconvert('0123456789', '').exotrim()
            if (deptid != '---' && deptid != '') {
                if (!(gdepts[0].exolocate(deptid))) {
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

async function system_dict_departmentcode(di, many, deptoptions) {
    // Style: uppercase code host (dept ids); preferred length 10
    exo_dict_code(di)
    if (!many)
        many = false
    if (!deptoptions)
        deptoptions = ''
    di.popup = 'await system_pop_department(' + many + ',' + deptoptions.exoquote() + ')'
    di.validation = 'await system_val_department(' + deptoptions.exoquote() + ')'
    // Same dept list as F7 (SECURITY*USERS via system_getdepartments)
    di.onchange = 'await system_typeahead_department(' + deptoptions.exoquote() + ')'
}

async function system_pop_department(many, deptoptions) {
    if (!many)
        many = false
    if (!(await system_getdepartments(deptoptions)))
        return false
    //return await exoui_decide('', gdepartments.split(fm), '', 0, '', many)
    var cols = [[0, 'Department'], [1, 'Name'], [2, 'Users with email']]
    return await exoui_decide('', gdepts, cols, 0, gvalue, many, true)
}

// Department/group typeahead: we already have the list (system_getdepartments → gdepts).
// Hand rows to exo_typeahead — same "have a list" path as general_typeahead_master after SELECT.
async function system_typeahead_department(deptoptions) {
    var cols = [
        ['DEPT', 'Department'],
        ['NAME', 'Name'],
        ['USERS', 'Users with email']
    ]
    if (!(await system_getdepartments(deptoptions)))
        return await exo_typeahead(null, cols, 0, { rows: [] })
    var rows = []
    for (var i = 0; i < gdepts[0].length; i++)
        rows.push([gdepts[0][i], gdepts[1][i] || '', gdepts[2][i] || ''])
    return await exo_typeahead(null, cols, 0, { rows: rows, wordstart: true })
}

async function system_val_department(deptoptions) {
    if (!(await system_getdepartments(deptoptions)))
        return false
    if (gvalue && !gdepartments.split(fm).exolocate(gvalue))
        return await exoui_invalid(gvalue + ' department does not exist')
    return true
}
