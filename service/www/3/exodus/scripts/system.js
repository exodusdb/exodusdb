//Copyright NEOSYS All Rights Reserved.

//moved from general.js

async function system_dict_datasetcode(di, many, orcurrent, test) {
    many = many || false
    orcurrent = orcurrent || false
    //test=test||false
    //"" means no restriction, true=only TEST databases, false=not TEST databases
    if (typeof test == 'undefined')
        test = '""'
    di.popup = 'await system_pop_datasetcode(' + many + ',' + orcurrent + ')'
	di.lowercase = true;
    di.validation = 'await system_val_datasetcode(' + many + ',' + orcurrent + ',' + test + ')'
    di.length = 8;
}

async function system_pop_datasetcode(many, orcurrent) {
    if (!(await system_getdatasets())) return false

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
    var datasetcode = await exodusdecide('Which dataset' + (many ? '(s' : '') + ' do you want?' + tt, popdata, cols, returncoln, defaultreply, many, inverted)
    return datasetcode
}

async function system_val_datasetcode(many, orcurrent, test) {

    if (gvalue == 'CURRENT') {
        if (!orcurrent) return await exodusinvalid('"CURRENT" is not allowed here')
        return true
    }

    if (!(await system_getdatasets()))
        return false

    if (gvalue != 'CURRENT' && gvalue.indexOf('CURRENT') >= 0)
        return await exodusinvalid('You cannot choose CURRENT and other datasets')

    if (typeof test == 'boolean') {
        if (test && gvalue && gvalue.substr(-5) != '_test')
            return await exodusinvalid('You can only choose TEST databases here')
        else if (!test && gvalue.substr(-5) == '_test')
            return await exodusinvalid('You cannot choose TEST databases here')
    }

    var values = many ? gvalue.split(':') : [gvalue]
    for (var ii = 0; ii < values.length; ii++) {
        if (!gdatasets[1].exoduslocate(values[ii]))
            return await exodusinvalid(values[ii].exodusquote() + ' is not a valid dataset code')
    }

    return true

}

var gdatasets
async function system_getdatasets(refresh) {
    if (refresh || !gdatasets) {
        db.request = 'EXECUTE\rGENERAL\rGETDATASETS'
        if (!(await db.send())) return await exodusinvalid(db.response)
        //split inverted
        gdatasets = db.data.exodussplit(vm + sm, true)
    }
    return gdatasets.length > 0
}

//users and security
////////////////////

// Session cache of USERS rows for typeahead (keyed by sortselect). Like general_typeahead_master.
var system_typeahead_user_rows = {}

// Same SELECT clauses as system_pop_users (BY RANK + optional filters).
function system_users_sortselect(withtask, haslocks, sselect) {
    var sortselect = ' AND WITH ID NOT STARTING "%"'
    if (sselect)
        sortselect += ' AND ' + sselect
    if (withtask)
        sortselect += ' AND WITH AUTHORISED_' + String(withtask).exodusconvert(' ', '_').toUpperCase()
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
        sselect = '"' + sselect.exodusswap('"', '\\"') + '"'
    di.popup = 'await system_pop_users(' + many + ',"' + withtask + '",' + haslocks + ',' + sselect + ')'
    // Live typeahead: same filters as F7 (sselect already quoted for the expression)
    di.onchange = 'await system_typeahead_users("' + withtask + '",' + haslocks + ',' + sselect + ')'
    di.filename = 'USERS'
    di.validation = 'await system_val_users()'
    if (many)
        di.unique = true
}

async function system_pop_users(many, withtask, haslocks, sselect) {
    var sortselect = system_users_sortselect(withtask, haslocks, sselect)
    var selcol0 = 1

    return await exodusfilepopup('USERS', [['USER_NAME', 'User Name'], ['USER_CODE', 'User Code'], ['DEPARTMENT_CODE2', 'Department'], ['EMAIL_ADDRESS', 'Email'], ['LAST_LOGIN_DATETIME', 'Last Login Datetime'], ['LAST_LOGIN_LOCATION', 'Last Login Location']], selcol0, sortselect, many)
}

// Find-as-you-type for USER_CODE. coln 0 = USER_CODE (pick). Session SELECT once per filter set.
async function system_typeahead_users(withtask, haslocks, sselect) {
    var key = String(gvalue || '').replace(/^\s+|\s+$/g, '')
    if (!key) {
        if (typeof form_typeahead_hide == 'function')
            form_typeahead_hide()
        return true
    }
    var sortselect = system_users_sortselect(withtask, haslocks, sselect)
    var cols = [
        ['USER_CODE', 'User Code'],
        ['USER_NAME', 'User Name'],
        ['DEPARTMENT_CODE2', 'Department'],
        ['EMAIL_ADDRESS', 'Email']
    ]
    var colids = []
    for (var i = 0; i < cols.length; i++)
        colids[i] = cols[i][0]
    var cachekey = sortselect + '\t' + colids.join(' ')
    if (!system_typeahead_user_rows[cachekey]) {
        var typeahead_limitn = 1000
        var request = 'CACHE\rSELECT\rUSERS\r' + sortselect + '\r' + colids.join(' ') + '\rXML\r' + typeahead_limitn
        var tdb = (typeof form_typeahead_dblink == 'function') ? form_typeahead_dblink() : db
        tdb.request = request
        if (!(await tdb.send()) || !tdb.data)
            return await exodus_typeahead(null, cols, 0, { rows: [] })
        system_typeahead_user_rows[cachekey] = (typeof exodus_typeahead_parserows == 'function')
            ? exodus_typeahead_parserows(tdb.data, colids)
            : []
    }
    return await exodus_typeahead(null, cols, 0, {
        rows: system_typeahead_user_rows[cachekey],
        wordstart: true
    })
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
    if (!(await security.exodusread('DEFINITIONS', 'SECURITY*USERS')))
        return await exodusinvalid(security.exodusresponse)
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

async function system_dict_departmentcode(di, many, deptoptions) {
    // Style: uppercase code host (dept ids); preferred length 10
    exodus_dict_code(di)
    if (!many)
        many = false
    if (!deptoptions)
        deptoptions = ''
    di.popup = 'await system_pop_department(' + many + ',' + deptoptions.exodusquote() + ')'
    di.validation = 'await system_val_department(' + deptoptions.exodusquote() + ')'
    // Same dept list as F7 (SECURITY*USERS via system_getdepartments)
    di.onchange = 'await system_typeahead_department(' + deptoptions.exodusquote() + ')'
}

async function system_pop_department(many, deptoptions) {
    if (!many)
        many = false
    if (!(await system_getdepartments(deptoptions)))
        return false
    //return await exodusdecide('', gdepartments.split(fm), '', 0, '', many)
    var cols = [[0, 'Department'], [1, 'Name'], [2, 'Users with email']]
    return await exodusdecide('', gdepts, cols, 0, gvalue, many, true)
}

// Find-as-you-type for department/group codes (emailusers TO_GROUP_CODE, etc.)
async function system_typeahead_department(deptoptions) {
    var key = String(gvalue || '').replace(/^\s+|\s+$/g, '')
    if (!key) {
        if (typeof form_typeahead_hide == 'function')
            form_typeahead_hide()
        return true
    }
    var cols = [
        ['DEPT', 'Department'],
        ['NAME', 'Name'],
        ['USERS', 'Users with email']
    ]
    if (!(await system_getdepartments(deptoptions)))
        return await exodus_typeahead(null, cols, 0, { rows: [] })
    var rows = []
    for (var i = 0; i < gdepts[0].length; i++)
        rows.push([gdepts[0][i], gdepts[1][i] || '', gdepts[2][i] || ''])
    return await exodus_typeahead(null, cols, 0, { rows: rows, wordstart: true })
}

async function system_val_department(deptoptions) {
    if (!(await system_getdepartments(deptoptions)))
        return false
    if (gvalue && !gdepartments.split(fm).exoduslocate(gvalue))
        return await exodusinvalid(gvalue + ' department does not exist')
    return true
}
