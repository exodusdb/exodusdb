//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**//

/*
function getdepartments_old() {
db.request = 'EXECUTE\rGENERAL\rGETDEPTS'
if (!db.send()) return neosysinvalid(db.response)
gdepartments = db.data
}
*/
var gdepartments = ''
var gdepts//0=deptids, 1=deptnames, 2=deptusernames
function getdepartments(deptoptions) {
    if (!deptoptions)
        deptoptions = ''

    //departments ending with digits like STUDIO2 may be treated like STUDIO
    var withnumbers = deptoptions.indexOf('N') >= 0
    var onlyactivegroups = deptoptions.indexOf('A') >= 0

    var security = []
    if (!security.neosysread('DEFINITIONS', 'SECURITY*USERS'))
        return neosysinvalid(security.neosysresponse)
    security = general_splitarray(security, [[[1, 9]], [[10, 11]]])
    gdepts = [[], [], []]
    var nusers = security[1].length
    var groupusers = []
    var datetoday = neosysdate()
    for (var usern = 2; usern < nusers; ++usern) {
        var userid = security[1][usern]
        if (userid == '---' || userid == '') {
            if (onlyactivegroups && groupusers.length == 0)
                continue
            deptid = security[1][usern - 1]
            if (!withnumbers)
                deptid = deptid.neosysconvert('0123456789', '').neosystrim()
            if (deptid != '---' && deptid != '') {
                if (!gdepts[0].neosyslocate(deptid)) {
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

function types_dict_departmentcode(di, many, deptoptions) {
    if (!many)
        many = false
    if (!deptoptions)
        deptoptions = ''
    di.popup = 'types_pop_department(' + many + ',' + deptoptions.neosysquote() + ')'
    di.validation = 'types_val_department(' + deptoptions.neosysquote() + ')'
}

function types_pop_department(many, deptoptions) {
    if (!many)
        many = false
    if (!getdepartments(deptoptions))
        return false
    //return neosysdecide('', gdepartments.split(fm), '', 0, '', many)
    var cols = [[0, 'Department'], [1, 'Name'], [2, 'Users with email']]
    return neosysdecide('', gdepts, cols, 0, '', many, true)
}

function types_val_department(deptoptions) {
    if (!getdepartments(deptoptions))
        return false
    if (gvalue && !gdepartments.split(fm).neosyslocate(gvalue))
        return neosysinvalid(gvalue + ' department does not exist')
    return true
}
