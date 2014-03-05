//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**//

function dict_TYPES(parameters) {

    //returns an array representing the client dictionary

    var dict = []//of dictrecs
    var din = -1

    //single valued fields

    var groupn = 0//group number
    di = dict[++din] = dictrec('TYPE_CODE', 'F', 0)
    var type = ''
    if (gparameters.MODE) type = gparameters.MODE.slice(0, 1)
    di.popup = 'agency_pop_types("' + type + '",true,"","",' + parameters.activities + ')'
    di.length = 3
    di.maxlength = 3
    di.required = true
    if (parameters.activities) di.listfunction = 'agency_list_types("TIMESHEET")'
    else if (parameters.MODE == 'JOBS') di.listfunction = 'agency_list_types("JOBS")'
    else if (parameters.MODE == 'MEDIA') di.listfunction = 'agency_list_types("MEDIA")'
    else di.listfunction = 'agency_list_types()'

    di = dict[++din] = dictrec('TYPE_NAME', 'F', 1)
    neosys_dict_text(di)
    di.required = true
    //di.length=60

    di = dict[++din] = dictrec('SEQUENCE', 'F', 2, '', '', '', '', '[NUMBER]')

    di = dict[++din] = dictrec('MEDIA_NONMEDIA', 'F', 3)
    di.radio = 'M;Media:N;Jobs'
    di.defaultvalue = '"N"'
    if (parameters.MODE == 'MEDIA') di.defaultvalue = '"M"'

    ++groupn

    di = dict[++din] = dictrec('ORDER_FOOTER', 'F', 4, '', groupn)
    neosys_dict_text(di)
    di.length = 80
    di.allowemptyrows = true
    //di.rows=10

    di = dict[++din] = dictrec('INCOME_ACCOUNT_NO', 'F', 5)
    general_dict_acno(di, validaccs = '', companycodedictids = 'COMPANY_CODE', accounttype = '', controltype = false)

    di = dict[++din] = dictrec('COST_ACCOUNT_NO', 'F', 6)
    general_dict_acno(di, validaccs = '', companycodedictids = 'COMPANY_CODE', accounttype = '', controltype = false)

    di = dict[++din] = dictrec('INCOME_ACCOUNT_NAME', 'S')
    general_dict_accountname(di, 'INCOME_ACCOUNT_NO')

    di = dict[++din] = dictrec('COST_ACCOUNT_NAME', 'S')
    general_dict_accountname(di, 'COST_ACCOUNT_NO')

    di = dict[++din] = dictrec('DEADLINE_DAYS', 'F', 7)
    neosys_dict_number(di, 0)

    di = dict[++din] = dictrec('CONFLICT_MINS', 'F', 8)
    neosys_dict_number(di, 0)

    di = dict[++din] = dictrec('COMPANY_CODE', 'F', 9)
    di.required = false
    general_dict_companycode(di)

    di = dict[++din] = dictrec('COMPANY_NAME', 'S')
    general_dict_companyname(di)

    di = dict[++din] = dictrec('GROUP_CODE', 'F', 10)
    agency_dict_typecode(di, 'gds.getx("MEDIA_NONMEDIA")', false, false, true)
    //dicti,medianonmediaexpression,manyexpression,withacno,onlygroups

    di = dict[++din] = dictrec('DATE_TIME', 'F', '11')
    neosys_dict_datetimeupdated(di)

    //appears on the activities.htm but not types.htm
    di = dict[++din] = dictrec('DEPARTMENT_CODE', 'F', 12)
    types_dict_departmentcode(di)
    di.required = true

    di = dict[++din] = dictrec('TAX_ACCOUNT_NO', 'F', 13)
    general_dict_acno(di, validaccs = '', companycodedictids = 'COMPANY_CODE', accounttype = '', controltype = false)

    di = dict[++din] = dictrec('COST_TAX_ACCOUNT_NO', 'F', 14)
    general_dict_acno(di, validaccs = '', companycodedictids = 'COMPANY_CODE', accounttype = '', controltype = false)

    di = dict[++din] = dictrec('TAX_ACCOUNT_NAME', 'S')
    general_dict_accountname(di, 'TAX_ACCOUNT_NO')

    di = dict[++din] = dictrec('COST_TAX_ACCOUNT_NAME', 'S')
    general_dict_accountname(di, 'COST_TAX_ACCOUNT_NO')

    di = dict[++din] = dictrec('GROUP_NAME', 'S')
    agency_dict_typename(di, 'GROUP_CODE')

    ++groupn

    di = dict[++din] = dictrec('CERTIFICATION_CODE', 'F', 20, '', groupn)
    di.required = true
    di.invalidcharacters = ' '

    di = dict[++din] = dictrec('CERTIFICATION_DESCRIPTION', 'F', 21, '', groupn)
    di.required = true
    neosys_dict_text(di, 40)

    di = dict[++din] = dictrec('STOPPED', 'F', 25)
    di = dict[++din] = dictrec('VERSION', 'F', 26)

    ++groupn

    //TYPE is the codeword describing the foreign system, CODE is the supplier code in that system
    di = dict[++din] = dictrec('ALIAS_SYSTEM', 'F', 27, '', groupn)
    general_dict_aliassystem(di)
    di = dict[++din] = dictrec('ALIAS_CODE', 'F', 28, '', groupn)
    general_dict_aliascode(di)

    di = dict[++din] = dictrec('NON_OPERATIONAL', 'F', 29)//was 27
    di.checkbox = '1;'

    ++groupn
    general_dict_addlogfields(dict, groupn, 30, 'general_open_version()')
    din = dict.length - 1

    return dict

}
