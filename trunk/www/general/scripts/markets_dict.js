//Copyright© 2000-2007 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**
//C++ style

function dict_MARKETS() {

    //returns an array representing the markets dictionary

    var dict = []//of dictrecs
    var din = -1;

    //single valued fields

    var groupn = 0; //group number

    var istimesheetadmin = neosyssecurity('TIMESHEET ADMINISTRATION')
    var timesheetadminreadonlymsg = gmsg

    di = dict[++din] = dictrec("MARKET_CODE", "F", 0);
    di.popup = "general_pop_markets(true)";
    di.length = 3;
    di.maxlength = 3;
    di.required = true;
    di.listfunction = "general_list_markets()";
    di.defaultvalue = gmarketcode.neosysquote()

    di = dict[++din] = dictrec("MARKET_NAME", "F", 1);
    neosys_dict_text(di);
    di.required = true;
    di.length = 30;

    //2 was population

    di = dict[++din] = dictrec("SEQUENCE", "F", 3, "", "", "", "", "[NUMBER]", "", "R", 5);

    //4 was vehicle codes

    di = dict[++din] = dictrec("CURRENCY_CODE", "F", 5);
    general_dict_currencycode(di);

    di = dict[++din] = dictrec("DATE_TIME", "F", "6");
    neosys_dict_datetimeupdated(di);

    di = dict[++din] = dictrec("CURRENCY_NAME", "S");
    general_dict_currencyname(di);

    di = dict[++din] = dictrec("STOPPED", "F", 25);
    di = dict[++din] = dictrec("VERSION", "F", 26);

    groupn = 1

    di = dict[++din] = dictrec('HOLIDAY_DATE', 'F', 7, '', groupn)
    di.required = true
    di.readonly = timesheetadminreadonlymsg
    neosys_dict_date(di, '')
    di.nodeleterow = !!timesheetadminreadonlymsg
    di.noinsertrow = !!timesheetadminreadonlymsg

    di = dict[++din] = dictrec('HOLIDAY_REASON', 'F', 8, '', groupn)
    di.required = true
    di.readonly = timesheetadminreadonlymsg
    neosys_dict_text(di, 60)

    di = dict[++din] = dictrec('WEEKEND_DAYS_OFF', 'F', 9)
    neosys_dict_dow(di, many = true)
    di.defaultvalue = '"6' + sm + '7"'

    di = dict[++din] = dictrec('FIRST_DAY_OF_WEEK', 'F', 10)
    neosys_dict_dow(di)
    di.defaultvalue = '1'

    di = dict[++din] = dictrec('HIGHLIGHT_DAYS', 'F', 11)
    neosys_dict_dow(di, many = true)
    di.defaultvalue = 'gds.getx("FIRST_DAY_OF_WEEK")'

    groupn++

    //SYSTEM is the codeword describing the foreign system, CODE is the market code in that system
    di = dict[++din] = dictrec('ALIAS_SYSTEM', 'F', 27, '', groupn)
    general_dict_aliassystem(di)
    di = dict[++din] = dictrec('ALIAS_CODE', 'F', 28, '', groupn)
    general_dict_aliascode(di)

    //var groupn = 3;
    groupn++
    general_dict_addlogfields(dict, groupn, 30, "general_open_version()");
    din = dict.length - 1;

    return dict;

}
