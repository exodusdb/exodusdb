//Copyright© 2010 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**//

function dict_TIMESHEETCONFIGURATION(parameters) {

    //returns an array representing the client dictionary

    var dict = []//of dictrecs
    var din = -1
    var di
    groupn = 0

    di = dict[++din] = dictrec('KEY', 'F', 0)
    di.readonly = true
    di.defaultvalue = '"TIMESHEET.PARAMS"'

    di = dict[++din] = dictrec('REQUIRE_ACTIVITY', 'F', 1)
    di.checkbox = 'Y;'

    di = dict[++din] = dictrec('MINIMUM_HOURS_PER_DAY', 'F', 2)
    neosys_dict_number(di, 0, 0, 24)
    di.defaultvalue = '"8"'

    di = dict[++din] = dictrec('MAXIMUM_HOURS_PER_DAY', 'F', 3)
    neosys_dict_number(di, 0, 0)
    di.defaultvalue = '"16"'

    di = dict[++din] = dictrec('MAXIMUM_DAYS', 'F', 4)
    neosys_dict_number(di, 0, 0)

    di = dict[++din] = dictrec('MINIMUM_MINUTES_PER_ENTRY', 'F', 5)
    neosys_dict_number(di, 0, 0)
    di.defaultvalue = '"15"'

    di = dict[++din] = dictrec('MAXIMUM_DAYS_IN_ADVANCE', 'F', 7)
    neosys_dict_number(di, 0, 0)
    di.defaultvalue = '"3"'

    //from this date timesheets will be required/reminders sent
    di = dict[++din] = dictrec('START_DATE', 'F', 8)
    neosys_dict_date(di)

    di = dict[++din] = dictrec('MINIMUM_TIME', 'F', 9)
    neosys_dict_time(di)

    di = dict[++din] = dictrec('MAXIMUM_DAYS_DELAY', 'F', 10)
    neosys_dict_number(di, 0, 0)
    di.defaultvalue = '"7"'

    di = dict[++din] = dictrec('REMINDER_HOUR', 'F', 11)
    neosys_dict_number(di, 0, 0, 23)

    di = dict[++din] = dictrec('APPROVAL_HOUR', 'F', 12)
    neosys_dict_number(di, 0, 0, 23)

    var gg = 1
    di = dict[++din] = dictrec('TIMESHEET_MONITORS', 'F', 14, '', gg)
    general_dict_usercode(di, many = true, withtask = 'TIMESHEET ADMINISTRATION')

    di = dict[++din] = dictrec('TESTING_EMAIL', 'F', 17)
    neosys_dict_emailaddress(di, ';')
    di.length = 30

    return dict

}
	
