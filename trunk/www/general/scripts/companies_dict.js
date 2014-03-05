//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

var gmaxperiod = 12

function company_pop_payment_currencycodes() {
    var othercurrcodes = gds.getx('PAYMENT_CURRENCY_CODES', null)
    othercurrcodes[grecn] = ''
    othercurrcodes = othercurrcodes.join(' ').split(' ')
    var sselect = ''
    if (othercurrcodes)
        sselect = 'WITHOUT CURRENCY_CODE ' + othercurrcodes.neosysquote()
    var currcodes = general_pop_currencies(many = true, sselect)
    if (!currcodes)
        return false
    return currcodes.join(' ')
}

function company_val_payment_currencycodes() {
    gvalue = gvalue.neosysconvert(',/', '  ').neosystrim()
    var currcodes = gvalue.split(' ').neosysunique()
    gvalue = currcodes.join(' ')
    for (var ii = 0; ii < currcodes.length; ++ii) {
        var currency = []
        if (!currency.neosysread('CURRENCIES', currcodes[ii]))
            return neosysinvalid(currency.neosysresponse)
    }
    return true
}

function company_val_closedperiod() {

    //nb using gorigvalue (from the old record) instead of goldvalue (from the old field)
    //allows them to change their minds and change/revert before saving

    //always allow closing from nothing or returning to original value
    if (!gorigvalue || gvalue == gorigvalue) return true

    //removing the closed period is the same as reopening all years
    var periodsyears = 'YEARS'
    if (gvalue) {

        var oldperiod = gorigvalue.split('/')
        var newperiod = gvalue.split('/')

        //we need to compare old and new OPEN period not old and new CLOSED period
        //eg changing closed period to 12/2007 is not reopening 2007

        //convert to open period
        oldperiod[0] = Number(oldperiod[0]) + 1
        oldperiod[1] = Number(oldperiod[1].neosysaddcent())
        if (oldperiod[0] > gmaxperiod) {
            oldperiod[0] = 1
            oldperiod[1] += 1
        }

        //convert to open period
        newperiod[0] = Number(newperiod[0]) + 1
        newperiod[1] = Number(newperiod[1].neosysaddcent())
        if (newperiod[0] > gmaxperiod) {
            newperiod[0] = 1
            newperiod[1] += 1
        }

        //closing years is always allowed at the moment
        if (newperiod[1] > oldperiod[1]) return true

        //if years are the same
        if (newperiod[1] == oldperiod[1]) {
            //if the period has been reduced then check for re-opening periods
            if (Number(newperiod[0]) < Number(oldperiod[0]))
                periodsyears = 'PERIODS'
            //if the period has been increased then dont check anything
            else
                periodsyears = ''
        }

        //otherwise year must have been reduced so check for reopening years

    }

    if (periodsyears && !neosyssecurity('LEDGER REOPEN CLOSED ' + periodsyears)) return neosysinvalid(gmsg)

    return true

}

function dict_COMPANIES() {

    //returns an array representing the currency dictionary

    var dict = []//of dictrecs
    var din = -1

    //single valued fields

    di = dict[++din] = dictrec('COMPANY_CODE', 'F', 0)
    general_dict_companycode(di, true)
    di.required = true
    di.validation = null
    //di.defaultvalue='gcompanycode'

    di = dict[++din] = dictrec('COMPANY_NAME', 'F', 1)
    general_dict_companyname(di)
    di.required = true

    di = dict[++din] = dictrec('CURRENT_PERIOD', 'F', 2)
    di.wordsep = '/'
    di.wordno = 1
    di.defaultvalue = '1'
    di.required = true

    di = dict[++din] = dictrec('CURRENT_YEAR', 'F', 2)
    di.wordsep = '/'
    di.wordno = 2
    di.required = true
    di.conversion = '[CENTURY,2]'
    //di.validation=

    di = dict[++din] = dictrec('CLOSED_PERIOD_FINANCE', 'F', 16)
    neosys_dict_period(di, '', '', '[FINANCIAL_PERIOD]')
    di.validation = 'company_val_closedperiod()'

    di = dict[++din] = dictrec('OPEN_ITEMS_CLEARED_UPTO_PERIOD', 'F', 17)
    di.conversion = '[YEARPERIOD]'
    di.length = 5
    di.readonly = true

    di = dict[++din] = dictrec('FINANCIAL_YEAR', 'F', 6)
    di.required = true
    di.defaultvalue = 1

    di = dict[++din] = dictrec('LANGUAGE', 'F', 14)
    di.required = true
    di.defaultvalue = '"ENGLISH"'

    di = dict[++din] = dictrec('DATE_STYLE', 'F', 10)
    di.conversion = '31/01/2002:01/31/2002:2002/01/31'
    di.required = true
    di.defaultvalue = '"31/01/2002"'

    di = dict[++din] = dictrec('NUMBER_STYLE', 'F', 22)
    di.conversion = '1,000.00:1.000,00'
    di.required = true
    di.defaultvalue = '"1,000.00"'

    di = dict[++din] = dictrec('BASE_CURRENCY_CODE', 'F', 3)
    general_dict_currencycode(di)
    di.required = true

    di = dict[++din] = dictrec('MAIN_CURRENCY_CODE', 'F', 15)
    general_dict_currencycode(di)
    di.defaultvalue = ''

    di = dict[++din] = dictrec('COMPANY_PREFIX', 'F', 28)
    di.maxlength = 3
    di.validcharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    di.validation = function () {
        return neosysyesno('This may affect the invoice number sequence|Are you SURE that you want to change this?')
    }

    di = dict[++din] = dictrec('EXCHANGE_ACCOUNT_NO', 'F', 4)
    general_dict_acno(di, '', 'COMPANY_CODE')

    di = dict[++din] = dictrec('EXCHANGE_ACCOUNT_NO2', 'F', 5)
    general_dict_acno(di, '', 'COMPANY_CODE')

    di = dict[++din] = dictrec('REVALUATION_METHOD', 'F', 11)
    di.conversion = ''

    di = dict[++din] = dictrec('CONVERSION_ACCOUNT_NO', 'F', 12)
    general_dict_acno(di, '', 'COMPANY_CODE')

    di = dict[++din] = dictrec('TAX_ACCOUNT_NO', 'F', 19)
    //general_dict_acno(di,'','COMPANY_CODE','-')
    general_dict_acno(di, '', 'COMPANY_CODE')

    di = dict[++din] = dictrec('TAX_REGISTRATION_NO', 'F', 21)
    di.length = 20

    di = dict[++din] = dictrec('DATE_TIME', 'F', '26')
    neosys_dict_datetimeupdated(di)

    di = dict[++din] = dictrec('TERMS_OF_PAYMENT', 'F', 32)
    neosys_dict_text(di, 80)

    di = dict[++din] = dictrec('MARKET_CODE', 'F', 30)
    general_dict_marketcode(di)
    di.defaultvalue = ''
    if (gsystem == 'ADAGENCY') di.required = true

    var groupn = 2
    di = dict[++din] = dictrec('PAYMENT_INSTRUCTIONS', 'F', 29, '', groupn)
    neosys_dict_text(di, 80, 5)

    //space separated list since several currencies might go into one currency bank account
    di = dict[++din] = dictrec('PAYMENT_CURRENCY_CODES', 'F', 48, '', groupn)
    di.popup = 'company_pop_payment_currencycodes()'
    di.validation = 'company_val_payment_currencycodes()'

    di = dict[++din] = dictrec('MARKET_NAME', 'S')
    general_dict_marketname(di)

    var groupn = 1
    di = dict[++din] = dictrec('OTHER_COMPANY_CODE', 'F', 7, '', groupn)
    general_dict_companycode(di)
    di.required = true
    di.unique = true

    di = dict[++din] = dictrec('OTHER_COMPANY_ACCOUNT_NO', 'F', 8, '', groupn)
    general_dict_acno(di, '', 'COMPANY_CODE')
    di.required = true

    //calculated fields

    di = dict[++din] = dictrec('OTHER_COMPANY_NAME', 'S', '', '', groupn)
    general_dict_companyname(di, 'OTHER_COMPANY_CODE')

    di = dict[++din] = dictrec('OTHER_COMPANY_ACCOUNT_NAME', 'S', '', '', groupn)
    // general_dict_acname(di,'OTHER_COMPANY_ACCOUNT_NO')

    di = dict[++din] = dictrec('STOPPED', 'F', 35)
    di = dict[++din] = dictrec('VERSION', 'F', 36)

    di = dict[++din] = dictrec('CLOSED_PERIOD_OPERATIONS', 'F', 37)
    neosys_dict_period(di, '', '', '[FINANCIAL_PERIOD]')
    di.validation = 'company_val_closedperiod()'

    var groupn = 3
    general_dict_addlogfields(dict, groupn, 40, 'general_open_version()')
    din = dict.length - 1

    //used in voucher number generation (if configured)
    di = dict[++din] = dictrec('SERIAL_NO', 'F', 46)
    neosys_dict_number(di, 0, 0, 99)
    di.length = 3

    di = dict[++din] = dictrec('MAXIMUM_INVOICE_DATE', 'F', 47)
    neosys_dict_date(di)

    return dict

}
