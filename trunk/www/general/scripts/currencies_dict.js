//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

var gnconversiondigits = 10

function form_postread() {
    neosyssetreadonly('DECIMALS', gkey == gbasecurr ? 'Cannot be changed here' : '')
    return true
}

function currency_val_decimals() {
    if (gds.isnewrecord)
        return true

    //if (gvalue<goldvalue)
    // return neosysinvalid('You cannot decrease the number of decimal places\rbecause it may cause rounding errors on posted accounts')

    //generally prevent even SENIOR users from changing the number of decimals
    //since can cause havoc difficult to recover from
    //eg increasing to 3 decimal places and then it is almost impossible to revert to 2 decimal places
    //without causing lots of errors especially in finance which cannot tolerate them
    if (!neosyssecurity('CURRENCY UPDATE DECIMALS'))
        return neosysinvalid(gmsg)

    //last resort confirm changing number of decimal places
    var increasereduce = gvalue < goldvalue ? 'reduce' : 'increase'
    var question = 'Are you SURE that you want to ' + increasereduce + ' the number of decimal places?'
    question += '\r\rReducing the number of decimals is NOT allowed if the currency has already been used. This is to prevent rounding errors'
    question += '\r\rReducing the number of decimals takes time to save the new currency record in order to check all the files to ensure that the currency has not already been'
    if (!neosysyesno(question, 2))
        return neosysinvalid()

    return true
}

function currency_val_rate(otherdictid) {
    var rate2 = ''
    //var ndecs=otherdictid=='RATE'?8:6
    if (gvalue) rate2 = (1 / gvalue).neosysoconv('[NUMBER,' + gnconversiondigits + ']')
    rate2 = (Number(rate2) + 0).toString()
    gds.setx(otherdictid, grecn, rate2)
    return true
}

function dict_CURRENCIES() {

    neosyssecurity('CURRENCY EXCHANGE RATE OVERRIDE')
    var currencyexchangerateoverride = gmsg

    //returns an array representing the currency dictionary

    var dict = []//of dictrecs
    var din = -1

    //single valued fields

    var groupn = 0//group number

    di = dict[++din] = dictrec('CURRENCY_CODE', 'F', 0)
    di.popup = 'general_pop_currencies(true)'
    di.length = 3
    di.maxlength = 3
    di.validcharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
    di.required = true
    //currency code must not start with any character that can be part of a number
    //eg 1-9 . , space dash
    //because splitting numbers like 123.12USD would be impossible
    di.listfunction = 'general_list_currencies()'

    di = dict[++din] = dictrec('CURRENCY_NAME', 'F', 1)
    di.lowercase = true
    di.required = true
    di.length = 50

    di = dict[++din] = dictrec('SUBUNIT_NAME', 'F', 2, '', '', '', '', '', '', 'T', 20)

    di = dict[++din] = dictrec('DECIMALS', 'F', 3)
    di.conversion = '0;none:2;.00:3;.000'
    di.required = true
    di.defaultvalue = 2
    di.validation = 'currency_val_decimals()'
    di.readonly = currencyexchangerateoverride

    groupn++

    di = dict[++din] = dictrec('EFFECTIVE_DATE', 'F', 4, '', groupn)
    neosys_dict_date(di)
    di.unique = true
    di.readonly = currencyexchangerateoverride

    di = dict[++din] = dictrec('RATE', 'F', 5, '', groupn)
    neosys_dict_number(di)
    di.validation = 'currency_val_rate("RATE2")'
    di.readonly = currencyexchangerateoverride

    //di=dict[++din]=dictrec('RATE2','S','','',1)
    //di.functioncode='"999"'

    di = dict[++din] = dictrec('PAYMENT_INSTRUCTIONS', 'F', '8', '', '', '', '', '', '', 'T', 60)
    neosys_dict_text(di, 80, 5)

    di = dict[++din] = dictrec('ISO_CODE', 'F', 9)
    di.length = 3
    di.maxlength = 3
    di.validcharacters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

    di = dict[++din] = dictrec('DATE_TIME', 'F', '11')
    neosys_dict_datetimeupdated(di)

    //the reciprocal of RATE
    di = dict[++din] = dictrec('RATE2', 'F', 12, '', groupn)
    neosys_dict_number(di)
    di.validation = 'currency_val_rate("RATE")'
    di.readonly = currencyexchangerateoverride

    //no longer used since RATE2 is available
    di = dict[++din] = dictrec('CONVERSION', 'S', '', '', groupn)
    di.functioncode = function currencies_conversion() {
        var rates = this.getx('RATE')
        for (var i = 0; i < rates.length; i++) {
            //if (rates[i]) rates[i]=neosysformatnumber(1/rates[i],6)
            if (rates[i]) rates[i] = (1 / rates[i]).neosysoconv('[NUMBER,' + gnconversiondigits + ']')
        }
        return rates
    }

    di = dict[++din] = dictrec('STOPPED', 'F', 25)
    neosys_dict_text(di)
    di.length = 60

    di = dict[++din] = dictrec('VERSION', 'F', 26)

    groupn++

    //SYSTEM is the codeword describing the foreign system, CODE is the currency code in that system
    di = dict[++din] = dictrec('ALIAS_SYSTEM', 'F', 27, '', groupn)
    general_dict_aliassystem(di)
    di = dict[++din] = dictrec('ALIAS_CODE', 'F', 28, '', groupn)
    general_dict_aliascode(di)

    groupn++

    general_dict_addlogfields(dict, groupn, 30, 'general_open_version()')
    din = dict.length - 1

    return dict

}
