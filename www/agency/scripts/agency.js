//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

var agp = []

function agency_dict_marketcode(di) {
    general_dict_marketcode(di)
    di.required = true
    di.defaultvalue = 'agency_def_marketcode()'
    di.popup = 'agency_pop_marketcode()'
    di.validation = 'agency_val_marketcode()'
}

function agency_getbrand() {
    var brandcode = gds.getx('BRAND_CODE')
    var brand = []
    if (!brand.neosysread('BRANDS', brandcode))
        return neosysinvalid(brand.neosysresponse)
    return brand
}

function agency_def_marketcode() {
    var brand = agency_getbrand()
    if (!brand)
        return ''
    var firstmarketcode = brand[13].split(vm)[0]
    if (firstmarketcode)
        return firstmarketcode
    return general_def_market_code()
}

function agency_pop_marketcode() {
    var brand = agency_getbrand()
    if (!brand)
        return ''
    var sortselect = brand[13].split(vm).join('" "')
    if (sortselect)
        sortselect = ' WITH CODE "' + sortselect + '"'
    return general_pop_markets(false, sortselect)
}

function agency_val_marketcode(marketcode) {

    if (typeof marketcode == 'undefined')
        marketcode=gvalue
    if (!marketcode)
        return true

    var brand = agency_getbrand()
    if (!brand)
        return false

    //no market restrictions, allow any market
    if (!brand[13])
        return true

    if (!brand[13].neosyslocate(marketcode))
        return neosysinvalid('Market "' + marketcode + '" is not allowed for\r Client "' + brand[3] + '"\r\rValid markets are\r' + brand[13].split(vm).join(', '))
    return true
}

function agency_readagp() {
    db.request = ['EXECUTE', 'AGENCY', 'READAGP'].join('\r')
    if (!db.send())
        return neosysinvalid(db.response)
    agp = db.data.split(fm)
    agp.unshift('AGENCY.PARAMETERS')
    return true
}

function agency_dict_attention(di) {
    neosys_dict_text(di, 30)
    di.defaultvalue = 'agency_def_attention()'
}

function agency_def_attention() {

    var brandcode = gds.getx('BRAND_CODE')
    var brand = []
    if (!brand.neosysread('BRANDS', brandcode))
        return neosysinvalid(brand.neosysresponse)

    var contact = brand[16]
    if (!contact)
        contact = ''

    return contact

}

function agency_list_categories() {
    openwindow('EXECUTE\rAGENCY\rLISTCATEGORIES')
    return true
}

function agency_list_types(mode) {
    var submode = ''
    if (mode == 'MEDIA') {
        submode = neosysyesnocancel('Show booking text?', 2)
        if (!submode)
            return false
    }
    openwindow('EXECUTE\rAGENCY\rLISTTYPES\r' + mode + ' ' + submode)
    return true
}

function agency_list_suppliers() {
    windowopen('../agency/supplierlist.htm')
    return true
}

function agency_list_clients() {
    var cmd = 'EXECUTE\rAGENCY\rLISTCLIENTS'
    var clientcode = gds.getx('CLIENT_CODE')
    if (clientcode)
        cmd += "'" + clientcode + "'"
    openwindow(cmd)
    return true
}

function agency_val_period(newperiod) {

    if (!newperiod) newperiod = gvalue

    //warning if year is not usual
    var curryear = (new Date).getUTCFullYear()
    var newyear = Number(newperiod.split('/', 2)[1].neosysaddcent())
    //if (newyear<(curryear-1)||newyear>(+curryear+1))
    if (newyear < curryear || newyear > (+curryear + 1)) {
        if (!neosysyesno('Are you sure you want year ' + newyear + ' ?', 2)) return neosysinvalid()
    }

    return true

}

function agency_def_companycode() {
    if (gncompanies == 1) return gcompanycode
    var defcompanycode = gds.getx('BRAND_CODE').neosysxlate('BRANDS', 10)
    return defcompanycode
}

//product categories
////////////////////

function agency_dict_productcategorycode(di, medianonmediaexpression, many) {

    di.filename = 'PRODUCT.CATEGORIES'

    //set the default unless already set
    //if (typeof di.defaultvalue=='undefined') di.defaultvalue=neosysquote(general_def_product_category_code())

    //if required and there is a default then use a dropdown
    if (di.required && di.defaultvalue) {
        di.dropdown = 'SELECT\rPRODUCT.CATEGORIES\r\rCODE NAME\rXML' + fm + 'CODE\rNAME'
        di.length = 20
    }
    else {
        if (typeof many == 'undefined') many = false
        di.popup = 'agency_pop_productcategories(' + many + ')'
        di.length = 3
    }
    di.link = 'windowopen2("../agency/productcategories.htm")'

}

function agency_dict_productcategoryname(di) {
    di.functioncode = 'return this.getx("PRODUCT_CATEGORY_CODE").neosysxlate("PRODUCT.CATEGORIES",1)'
    di.length = 30
}

function agency_pop_productcategories(many) {
    return neosysfilepopup('PRODUCT.CATEGORIES', [['NAME', 'Product Category Name'], ['CODE', 'Product Category Code']], 1, 'BY NAME', many)
}

function agency_search(filename, many, options, filter) {

    var dialogargs = new Object
    dialogargs.cancel = true

    switch (filename) {
        case 'PLANS':
        case 'SCHEDULES':
            {
                if (options) dialogargs.BRAND_CODE = options
                dialogargs.indexids = 'BRAND_CODE YEAR_PERIOD EXECUTIVE_CODE JOB_NO'
                dialogargs.displayids = []
                var ii = 0
                dialogargs.displayids[ii++] = ['COMPANY_CODE', 'Co']
                dialogargs.displayids[ii++] = ['REF_NO', filename == 'PLANS' ? 'Plan No.' : 'Schedule No.']
                dialogargs.displayids[ii++] = ['PERIOD', 'Period']
                if (filename == 'PLANS') dialogargs.displayids[ii++] = ['PERIOD_TO', '- Period']
                dialogargs.displayids[ii++] = ['BRAND_NAME', 'Brand Name']
                dialogargs.displayids[ii++] = ['FIRST_VEHICLE', 'First Vehicle']
                dialogargs.displayids[ii++] = ['APPROVAL_DATE', 'Approved']
                dialogargs.returncolid = 'REF_NO'
                if (filename == 'PLANS') {
                    //dialogargs.displayids[0][1]='Plan No.'
                }
                else {
                    dialogargs.displayids[dialogargs.displayids.length] = ['FULLY_BOOKED', 'Fully Booked']
                    dialogargs.displayids[dialogargs.displayids.length] = ['INVOICED', 'Fully Invoiced']
                }
                dialogargs.displayids[dialogargs.displayids.length] = ['NLINES', 'Number of Lines']
                dialogargs.displayids[dialogargs.displayids.length] = ['TOT_NUMBER', 'Number of Ads']
                dialogargs.MEDIA_NONMEDIA = 2
                break
            }

        case 'BOOKING.ORDERS':
            {
                dialogargs.indexids = 'BRAND_CODE SCHEDULE_NO VEHICLE_CODE'
                dialogargs.displayids = [['COMPANY_CODE', 'Co'], ['ORDER_NO', 'Order No.'], ['PERIOD1', 'Period'], ['BRAND_NAME1', 'Brand Name'], ['SCHEDULE_NOS2', 'Schedule No.'], ['VEHICLE_CODES2', 'Vehicle']]
                dialogargs.returncolid = 'ORDER_NO'
                if (filename == 'PLANS') dialogargs.displayids[0][1] = 'Plan No.'
                dialogargs.MEDIA_NONMEDIA = 2
                break
            }

        case 'JOBS':
            {
                dialogargs.indexids = 'YEAR_PERIOD BRAND_CODE EXECUTIVE_CODE'
                if (document.location.toString().indexOf('search.htm') < 0) {
                    dialogargs.indexids += ' MASTER_JOB_NO'
                    filename = 'JOBS'
                }
                dialogargs.displayids = [['COMPANY_CODE', 'Co'], ['JOB_NO', 'Job No.'], ['PERIOD', 'Period'], ['BRAND_NAME', 'Brand Name'], ['DESCRIPTION1', 'Description'], ['CLOSED', 'Closed']]
                dialogargs.returncolid = 'JOB_NO'
                dialogargs.MEDIA_NONMEDIA = 3
                //   dialogargs.notclosed=options
                break
            }

        case 'PRODUCTION.ORDERS':
            {
                dialogargs.indexids = 'JOB_NO FROM_DATE BRAND_CODE EXECUTIVE_CODE SUPPLIER_CODE SUPPLIER_INVOICE_NO'
                dialogargs.displayids = [['COMPANY_CODE', 'Co'], ['ORDER_NO', 'Order No.'], ['DATE', 'Date'], ['JOB_NO', 'Job No.'], ['BRAND_NAME', 'Brand Name'], ['DESCRIPTION', 'Description'], ['SUPPLIER_NAME', 'Supplier Name'], ['SUPP_INV_NO', 'Supplier Invoice No.'], ['STATUS', 'Status']]
                dialogargs.returncolid = 'ORDER_NO'
                dialogargs.MEDIA_NONMEDIA = 3
                break
            }

        case 'PRODUCTION.INVOICES':
            {
                dialogargs.indexids = 'JOB_NO FROM_DATE BRAND_CODE EXECUTIVE_CODE STATUS INVOICE_NO CLIENT_ORDER_NO'
                dialogargs.displayids = [['COMPANY_CODE', 'Co'], ['ESTIMATE_NO', 'Estimate No.'], ['DATE', 'Date'], ['JOB_NO', 'Job No.'], ['BRAND_NAME', 'Brand Name'], ['DESCRIPTION', 'Description'], ['STATUS', 'Status']]
                dialogargs.returncolid = 'ESTIMATE_NO'
                dialogargs.MEDIA_NONMEDIA = 3
                break
            }

        case 'MATERIALS':
            {
                dialogargs.indexids = 'MATERIAL_CODE BRAND_CODE FIRST_APPEARANCE_DATE'
                dialogargs.displayids = [['MATERIAL_NO', 'Material No.'], ['MATERIAL_CODE', 'Job No.'], ['DESCRIPTION', 'Description'], ['CLIENT_NAME', 'Client Name'], ['BRAND_NAME', 'Brand Name'], ['VEHICLE_NAME', 'Vehicle Name'], ['FIRST_APPEARANCE_DATE', 'First Appearance Date']]
                break
            }

    }
    dialogargs.DOCUMENT_TYPE = filename
    dialogargs.many = many

    dialogargs.filter = filter
    dialoglocation = NEOSYSlocation + '../agency/search.htm'

    var results = neosysshowmodaldialog(dialoglocation, dialogargs)
    if (typeof results == 'undefined') results = ''

    return rearray(results)

}

function agency_def_refno(params) {

    db.request = 'EXECUTE\r' + gmodule + '\rGETNEXTNO\r' + gdatafilename
    if (params) db.request += '\r' + params
    db.send()
    if (db.response != 'OK') {
        neosysinvalid(db.response)
        return ''
    }

    return db.data

}

//periods
/////////
function agency_pop_periods(filename, many) {

    switch (filename) {
        case 1:
            {
                filename = 'PLANS'
                break
            }
        case 2:
            {
                filename = 'SCHEDULES'
                break
            }
        case 9:
        case "9":
        case 'JOBS':
            {
                filename = 'JOBS'
                break
            }

        case 10:
        case "10":
        case 'PRODUCTION.ORDERS':
            {
                filename = 'PRODUCTION.ORDERS'
                break
            }

        case 11:
        case "11":
        case 'PRODUCTION.INVOICES':
            {
                filename = 'PRODUCTION.INVOICES'
                break
            }

        case 'TIMESHEETS':
            {
                break
            }
        case 'PLANS':
            {
                break
            }
        default:
            {
                filename = 'SCHEDULES'
            }
    }

    var data = '@' + 'CACHE\rGETINDEXVALUES\r' + filename + '\rYEAR_PERIOD'
    var result = neosysdecide2('', data, '', 0, '', many)
    if (!result) return ''

    return result

}

//executives
////////////

function agency_dict_executivecode(di, filename, allownew, many) {
    if (!allownew)
        allownew = false
    if (!many)
        many = false
    di.popup = 'agency_pop_executives("' + filename + '",' + many + ')'
    di.validation = 'agency_val_executive("' + filename + '",' + allownew + ')'
    if (allownew)
        di.defaultvalue = 'agency_def_executivecode("' + filename + '")'
    di.lowercase = true
    di.invalidcharacters = '*'

    //protect executive code
    if (filename == 'JOBS' && !neosyssecurity('JOB CHANGE EXECUTIVE')) {
        di.readonly = gmsg
        di.defaultvalue = 'gusername'
    }

}

function agency_def_executivecode(filename) {

    //default to executive of the brand
    var result
    if (filename == 'SCHEDULES' || filename == 'PLANS' || filename == 'JOBS')
        result = gds.getx("BRAND_CODE").neosysxlate("BRANDS", 11)
    //or the current user
    if (!result) result = gusername

    return result

}

function agency_val_executive(filename, allownew) {

    if (!gvalue) return true

    //check if an existing exec
    var execs = agency_getexecutives(filename)
    if (!execs) return true
    execs = execs.group1

    //quit if an existing exec (Exact Case)
    for (var ii = 0; ii < execs.length; ii++) {
        if (execs[ii].EXECUTIVE_CODE.text == gvalue) {
            //gvalue=execs[ii].EXECUTIVE_CODE.text
            return true
        }
    }

    //optionally change case
    for (var ii = 0; ii < execs.length; ii++) {
        //alert(execs[ii].EXECUTIVE_CODE.text.toUpperCase()+' '+gvalue.toUpperCase())
        if (execs[ii].EXECUTIVE_CODE.text.toUpperCase() == gvalue.toUpperCase()) {
            if (!(reply = neosysyesnocancel('Change the capitalisation of your entry?\r\rFrom: ' + gvalue.neosysquote() + '\r\r  To: ' + execs[ii].EXECUTIVE_CODE.text.neosysquote(), 1))) return neosysinvalid()
            if (reply == 1) gvalue = execs[ii].EXECUTIVE_CODE.text
            return true
        }
    }

    //option to allow/prevent new executives
    if (!allownew) return neosysinvalid(gvalue + ' Executive does not exist')

    //confirm is new exec
    if (gvalue && (neosysyesno(' Is ' + neosysquote(gvalue) + ' a new executive for ' + filename.toLowerCase() + ' ?', 1) != 1)) return false

    return true

}

function agency_getexecutives(filename) {

    db.request = 'CACHE\rGETINDEXVALUESXML\r' + filename + '\rEXECUTIVE_CODE'
    if (!db.send()) {
        neosysinvalid(db.response)
        return ''
    }

    if (!db.data || db.data == '<records></records>') return ''

    var execs = neosysxml2obj(db.data)

    return execs

}

function agency_pop_executives(filename, many) {
    var execs = agency_getexecutives(filename)
    if (!execs) return neosysinvalid('No executives have been entered on ' + filename.toLowerCase() + ' yet')
    if (execs.group1.length == 0) return neosysinvalid('No executives have been entered on ' + filename.toLowerCase() + ' yet')
    var result = neosysdecide2('', execs, [['EXECUTIVE_CODE', 'Executive']], 0, '', many)
    //for (var ii=0;ii<result.length;ii++)
    //{
    // if (typeof result[ii]=='string'&&result[ii]==result[ii].toUpperCase()) result[ii]=result[ii].neosyscapitalise()
    //}
    return result
}

function agency_dict_executivename(di, executivecode_dictid) {
    if (!executivecode_dictid) executivecode_dictid = 'EXECUTIVE_CODE'
    //di.functioncode='return this.getx("'+executivecode_dictid+'").neosysxlate("USERS",1,"C")'
    //regetx because executive code is possibly not bound except on job file
    di.functioncode = 'return this.regetx("' + executivecode_dictid + '").toUpperCase().neosysxlate("USERS",1,"C")'
    di.align = 'T'
    di.length = 30
}

//clients
/////////
function agency_dict_clientcode(di, media_nonmediaexpression, manyexpression, groups, members, groupno) {
    //media_nonmediaexpression not implemented yet but here to be similar to agency_dict_suppliercode

    ///groups and members can be true or false. anything else means dont care
    if (typeof groups != 'boolean') groups = '""'
    if (typeof members != 'boolean') members = '""'
    if (!groupno) groupno = '""'

    //di.filename='CLIENTS'
    di.validation = 'agency_val_clientcode("","",' + manyexpression + ',' + groups + ',' + members + ',' + groupno + ')'
    di.popup = 'agency_pop_clients(' + manyexpression + ',' + groups + ',' + members + ',' + groupno + ')'
    di.length = 5
    di.link = 'windowopen2("../agency/clients.htm")'
}

function agency_dict_clientname(di, clientcode_dictid) {
    if (!clientcode_dictid)
        clientcode_dictid = 'CLIENT_CODE'
    di.functioncode = 'return this.getx("' + clientcode_dictid + '").neosysxlate("CLIENTS",1)'
    di.align = 'T'
    di.length = 30
}

function agency_pop_clients(many, groups, members, groupno, filter) {

    var cols = [['NAME', 'Client Name'], ['MARKET_CODE', 'Market'], ['CODE', 'Client Code'], ['ACNO2', 'A/c No'], ['NBRANDS', 'Brands', '', 'RIGHT'], ['LAST_DATE_UPDATED', 'Updated']]
    cols[cols.length] = ['STOPPED', 'Stopped']

    var sortselect = ''
    sortselect += ' BY STOPPED2'
    sortselect += ' BY NAME'

    if (groupno) {
        membercode = 'MEMBER2_CODE'
        groupcode = 'GROUP2_CODE'
    } else {
        membercode = 'MEMBER_CODE'
        groupcode = 'GROUP_CODE'
    }
    //NB groups are records with member codes, and members are records with groups codes!
    if (typeof groups == 'boolean') sortselect += ' WITH ' + membercode + ' ' + (groups ? '' : '"" ') + 'AND'
    if (typeof members == 'boolean') sortselect += ' WITH ' + groupcode + ' ' + (members ? '' : '"" ') + 'AND'
    if (filter) sortselect += ' ' + filter + ' ' + 'AND'

    if (!groups) {
        if (typeof CLIENT_GROUP_CODE != 'undefined') {
            var clientgroupcodes = gds.getx('CLIENT_GROUP_CODE', null)
            if (clientgroupcodes.join())
                sortselect += ' WITH GROUP_CODE ' + clientgroupcodes.neosystrim().neosysquote() + ' AND'
        }
        if (typeof CLIENT_GROUP2_CODE != 'undefined') {
            var clientgroup2codes = gds.getx('CLIENT_GROUP2_CODE', null)
            if (clientgroup2codes.join())
                sortselect += ' WITH GROUP2_CODE ' + clientgroup2codes.neosystrim().neosysquote() + ' AND'
        }
    }

    if (sortselect.slice(-4) == ' AND') sortselect = sortselect.slice(0, -4)

    return neosysfilepopup('CLIENTS', cols, 2, sortselect, many)

}

function agency_val_clientcode(clientcode, allownew, many, groups, members) {

    //try to keep in sync
    //agency_val_suppliercode
    //agency_val_clientcode

    //given a client code or client search string
    //(uses and updates gvalue if no clientcode provided)
    //get the client (if more than one then user selects one)
    //also checks if authorised

    if (!clientcode && !gvalue) return true

    //get the brand or select from a list OR FAIL
    cols = ['CLIENT_NAME', 'CODE', 'STOPPED']
    if (!(client = agency_findrecord('CLIENTS', clientcode, cols, 'CODE', '', allownew, many))) return false

    //allow for returning many codes after name search (gvalue as array)
    //unfortunately skips validations below
    if (many) {
        gvalue = client
        return true
    }

    clientcode = client[0]

    //check group and member
    if (typeof groups == 'boolean') {
        if (groups && !client[29]) return neosysinvalid(client[1] + ' is not a client group and cannot be used here')
        else if (client[29]) return neosysinvalid(client[1] + ' is a client group and cannot be used here')
    }
    if (typeof members == 'boolean') {
        if (members && !client[16]) return neosysinvalid(client[1] + ' is not a subsidiary client and cannot be used here')
        else if (client[16]) return neosysinvalid(client[1] + ' is a subsidiary client and cannot be used here')
    }

    //check this client is authorised
    if (!validcode2('', clientcode, reqauth = ['PLANS', 'SCHEDULES'].neosyslocate(gdatafilename) != 0))
        return neosysinvalid(gmsg)

    if (gKeyNodes.length && gpreviouselement.getAttribute('neosysfieldno') != 0) {

        //prevent referring back to same client
        if (gdatafilename == 'CLIENTS' && gvalue == gkey)
            return neosysinvalid('You cannot use the main client code here again')

        //prevent use of stopped clients (on new records?)
        if (client[35])
            return neosysinvalid('You cannot create new records for this client because\r\r' + client[35])
    }

    return true

}

function agency_dict_netgross(di) {
    di.conversion = 'N;Net:G;Gross:n;One before Net:g;One after Gross'
    di.defaultvalue = '"N"'
}

function agency_link_brandcode() {

    if (!gvalue) return windowopen('../agency/clients.htm')

    var brand = []
    if (!brand.neosysread('BRANDS', gvalue)) return neosysinvalid(brand.neosysresponse)

    windowopen('../agency/clients.htm', { "key": brand[1] })

    return true

}

//brands
////////

function agency_dict_brandcode(di, clientcodesexpression, manyexpression) {

    if (di.type == 'F') {
        //di.filename='BRANDS'//not included so validation can convert to correct code
        if (!clientcodesexpression) clientcodesexpression = '""'
        if (!manyexpression) manyexpression = '""'
        di.popup = 'agency_pop_brands(' + [clientcodesexpression, manyexpression].join(',') + ')'
        //di.validation='agency_val_brandcode('+[clientcodesexpression,manyexpression].join(',')+')'
        //di.validation='agency_val_brandcode('+[clientcodesexpression].join(',')+')'
        di.validation = 'agency_val_brandcode(' + clientcodesexpression + ',"",' + manyexpression + ')'
    }
    di.link = 'agency_link_brandcode()'
    di.length = 5
}

function agency_dict_brandclientcode(di, brandcode_dictid) {
    if (!brandcode_dictid) brandcode_dictid = 'BRAND_CODE'
    di.functioncode = 'return this.getx("' + brandcode_dictid + '").neosysxlate("BRANDS",1)'
    di.length = 5
}

function agency_dict_brandname(di, brandcode_dictid) {
    if (!brandcode_dictid) brandcode_dictid = 'BRAND_CODE'
    di.functioncode = 'return this.getx("' + brandcode_dictid + '").neosysxlate("BRANDS",2)'
    di.align = 'T'
    di.length = 30
}

function agency_dict_brandclientname(di, brandcode_dictid) {
    if (!brandcode_dictid)
        brandcode_dictid = 'BRAND_CODE'
    di.functioncode = 'return this.getx("' + brandcode_dictid + '").neosysxlate("BRANDS",3)'
    di.align = 'T'
    di.length = 30
}

function agency_clientandbrandname(gds, brandcode_dictid) {
    var brandnames = gds.getx(brandcode_dictid).neosysxlate("BRANDS", 2)
    var clientnames = gds.getx(brandcode_dictid).neosysxlate("BRANDS", 3)
    var clientandbrandnames = []
    for (var ii = 0; ii < brandnames.length; ++ii) {
        var tt = clientnames[ii]
        if (tt)
            tt += ', '
        tt += brandnames[ii]
        clientandbrandnames[ii] = tt
    }
    return clientandbrandnames
}

function agency_dict_clientandbrandname(di, brandcode_dictid) {
    if (!brandcode_dictid) brandcode_dictid = 'BRAND_CODE'
    di.functioncode = 'return agency_clientandbrandname(this,"' + brandcode_dictid + '")'
    di.align = 'T'
    di.length = 30
}

function agency_pop_brands(clientcodes, many) {

    //clientcodes is an array or quoted string of clientcodes

    if (no(clientcodes)) {
        if (!(clientcodes = agency_pop_clients(false, '', '', '', 'WITH NBRANDS GT "0"'))) return null
    }
    var cols = [['CLIENT_NAME', 'Client Name'], ['BRAND_NAME', 'Brand Name'], ['BRAND_CODE', 'Brand Code']]
    cols[cols.length] = ['STOPPED', 'Stopped']
    var sortselect = 'WITH CLIENT_CODE ' + clientcodes.neosystrim().neosysquote()
    sortselect += ' BY STOPPED2'
    sortselect += ' BY BRAND_NAME'

    return neosysfilepopup('BRANDS', cols, 2, sortselect, many)

}

function agency_val_brandcode(clientcodes, brandcode0, many) {

    var brandcode = brandcode0 ? brandcode0 : gvalue

    if (!brandcode) return true

    //given a brand code or brand search string
    //(uses and updates gvalue if no brandcode provided)
    //get the brand (if more than one then user selects one)
    //also checks if authorised

    //get the brand or select from a list OR FAIL
    cols = ['CLIENT_NAME', 'BRAND_NAME', 'CODE', 'STOPPED']
    if (!(brand = agency_findrecord('BRANDS', brandcode, cols, 'CODE', '', '', many))) return false

    //allow for returning many codes after name search (gvalue as array)
    if (many) {
        //unfortunately cannot return multiple brands except by gvalue
        if (!brandcode0) gvalue = brand
        return true
    }

    brandcode = brand[0]

    //update the input value to the selected brand
    if (!brandcode0) gvalue = brandcode

    //check client matches
    // if (clientcodes&&brand[1]!=clientcodes)
    if (clientcodes && !clientcodes.neosyslocate(brand[1])) {
        return neosysinvalid('Brand ' + brandcode + ' - ' + brand[1] + ' does not belong to client ' + neosysquote(clientcodes))
    }

    //check this brand is authorised
    if (!validcode2('', '', brandcode, reqauth = ['PLANS', 'SCHEDULES'].neosyslocate(gdatafilename) != 0))
        return neosysinvalid(gmsg)

    return true

}

//suppliers
///////////

function agency_dict_suppliercode(di, media_nonmediaexpression, manyexpression, groups, members) {

    ///groups and members can be true or false. anything else means dont care
    if (typeof groups != 'boolean') groups = '""'
    if (typeof members != 'boolean') members = '""'

    // di.filename='SUPPLIERS'
    if (!media_nonmediaexpression) media_nonmediaexpression = '""'
    if (!manyexpression) manyexpression = '""'
    di.popup = 'agency_pop_suppliers(' + media_nonmediaexpression + ',' + manyexpression + ',' + groups + ',' + members + ')'
    //nb should only use double quotes in validation
    // because can be stored and used in custom validation routines added afterwards
    di.validation = 'agency_val_suppliercode(' + media_nonmediaexpression + ',"","",' + manyexpression + ',' + groups + ',' + members + ')'
    di.length = 5
    di.link = 'agency_link_suppliers(' + media_nonmediaexpression + ')'

}

function agency_link_suppliers(media_nonmedia) {
    var mode = ''
    if (media_nonmedia == 'M')
        mode = '?MODE=MEDIA'
    else if (media_nonmedia == 'N')
        mode = '?MODE=JOBS'
    return windowopen2('../agency/suppliers.htm' + mode)

}

function agency_dict_suppliername(di, suppliercode_dictid) {
    if (!suppliercode_dictid) suppliercode_dictid = 'SUPPLIER_CODE'
    di.functioncode = 'return this.getx("' + suppliercode_dictid + '").neosysxlate("SUPPLIERS",1)'
}

function agency_pop_suppliers(media_nonmedia, many, groups, members) {

    //select from schedule (similar in media_pop_vehicles and agency_pop_suppliers
    if (typeof media_get_schnos != 'undefined') {
        var schnos = media_get_schnos()
        if (schnos.join('')) {
            return media_pop_suppliervehicle('SUPPLIER_CODE', schnos, many)
        }
    }

    var cols = [['NAME', 'Name']]
    cols[cols.length] = ['CODE', 'Code']
    cols[cols.length] = ['TYPE_NAME', 'Type']
    cols[cols.length] = ['MARKET_CODE', 'Market']
    cols[cols.length] = ['ACNO2', 'A/c No']
    if (!media_nonmedia || media_nonmedia == 'M' || media_nonmedia == '2') cols[cols.length] = ['NVEHICLES', 'Vehicles', '', 'RIGHT']
    cols[cols.length] = ['LAST_DATE_UPDATED', 'Updated']
    cols[cols.length] = ['STOPPED', 'Stopped']

    var sortselect = ''
    sortselect += ' BY STOPPED2'
    sortselect += ' BY TYPE'//Media, Jobs, Both
    sortselect += ' BY NAME'

    if (media_nonmedia == 'M' || media_nonmedia == '2') sortselect += ' WITH TYPE CONTAINING "M" AND'
    if (media_nonmedia == 'N' || media_nonmedia == '3') sortselect += ' WITH TYPE CONTAINING "P" AND'

    //NB groups are records with member codes, and members are records with groups codes!
    if (typeof groups == 'boolean') sortselect += ' WITH ' + (groups ? '' : 'NO ') + 'MEMBER_CODE AND'
    if (typeof members == 'boolean') sortselect = ' WITH ' + (members ? '' : 'NO ') + 'GROUP_CODE AND'

    if (sortselect.slice(-4) == ' AND') sortselect = sortselect.slice(0, -4)

    return neosysfilepopup('SUPPLIERS', cols, 1, sortselect, many)

}

function agency_val_suppliercode(media_nonmedia, suppliercode, allownew, many, groups, members) {

    //try to keep in sync
    //agency_val_suppliercode
    //agency_val_clientcode

    //given a client code or client search string
    //(uses and updates gvalue if no clientcode provided)
    //get the client (if more than one then user selects one)
    //also checks if authorised

    if (!suppliercode && !gvalue) return true

    var supplier
    if (!(supplier = agency_getsupplier(suppliercode, media_nonmedia, allownew, many))) return false

    //allow for returning many codes after name search (gvalue as array)
    //unfortunately skips validations below
    if (many) {
        gvalue = supplier
        return true
    }

    suppliercode = supplier[0]

    //check group and member
    if (typeof groups == 'boolean') {
        if (groups && !supplier[18]) return neosysinvalid(supplier[1] + ' is not a supplier group and cannot be used here')
        else if (supplier[18]) return neosysinvalid(supplier[1] + ' is a supplier group and cannot be used here')
    }
    if (typeof members == 'boolean') {
        if (members && !supplier[17]) return neosysinvalid(supplier[1] + ' is not a subsidiary supplier and cannot be used here')
        else if (supplier[17]) return neosysinvalid(supplier[1] + ' is a subsidiary supplier and cannot be used here')
    }

    //check if media supplier
    if (media_nonmedia == 'M' || media_nonmedia == '2') {
        if (supplier[13] == 'P') return neosysinvalid(supplier[1] + '\n is not a Media Supplier')
    }

    //check if nonmedia supplier
    if (media_nonmedia == 'N' || media_nonmedia == '3') {
        if (supplier[13] == 'M') return neosysinvalid(supplier[1] + '\n is a Media Supplier and cannot be used here')
    }

    if (gKeyNodes.length && gpreviouselement.getAttribute('neosysfieldno') != 0) {

        //prevent referring back to same supplier
        if (gdatafilename == 'SUPPLIERS' && gvalue == gkey)
            return neosysinvalid('You cannot use the main supplier code here again')

        //prevent use of stopped suppliers (on new records?)
        if (supplier[15])
            return neosysinvalid('You cannot create new records for this supplier because\r\r' + supplier[15] + '\r\r(See the Supplier File)')
    }

    return true

}

function agency_getsupplier(suppliercode, media_nonmedia, allownew, many) {

    //given a supplier code or supplier search string
    //(uses and updates gvalue if no suppliercode provided)
    //get the supplier (if more than one then user selects one)

    //get the brand or select from a list OR FAIL
    var cols = [['NAME', 'Supplier Name'], ['CODE', 'Supplier Code'], ['STOPPED', 'Stopped']]
    if (!(supplier = agency_findrecord('SUPPLIERS', suppliercode, cols, 'CODE', media_nonmedia, allownew, many))) return false

    return supplier

}


//types
///////

function agency_dict_typecode(di, medianonmediaexpression, manyexpression, withacno, onlygroups, onlydepartments) {
    di.filename = 'MEDIA.TYPES'
    if (!medianonmediaexpression) medianonmediaexpression = '""'
    if (!manyexpression) manyexpression = '""'
    if (!onlygroups) onlygroups = '""'
    if (!withacno) withacno = ''
    if (!onlydepartments) onlydepartments = ''
    di.validation = 'agency_val_typecode(' + medianonmediaexpression + ',"' + withacno + '",' + onlygroups + ',"' + onlydepartments + '")'
    di.popup = 'agency_pop_types(' + [medianonmediaexpression, manyexpression, '"' + withacno + '"', onlygroups, '"' + onlydepartments + '"'].join() + ')'
    di.length = 3
    var mode = ''
    if (medianonmediaexpression == '"M"') mode = '?MODE=MEDIA'
    else if (medianonmediaexpression == '"N"') mode = '?MODE=JOBS'
    di.link = 'windowopen2("../agency/types.htm' + mode + '")'
}

function agency_dict_typename(di, typecode_dictid) {
    if (!typecode_dictid) typecode_dictid = 'TYPE_CODE'
    di.functioncode = 'return this.getx("' + typecode_dictid + '").neosysxlate("MEDIA.TYPES",1)'
}

function agency_pop_types(media_nonmedia, many, withacno, onlygroups, onlydepartments) {
    //BY PRODUCTION BY DEPARTMENT BY GROUP BY SEQ BY NAME
    var sortselect = 'BY PRODUCTION'
    sortselect += onlydepartments ? ' BY DEPARTMENT' : ' BY GROUP'
    sortselect += ' BY NAME WITH DEPARTMENT'
    if (onlydepartments) sortselect += ' NE'
    sortselect += ' ""'
    if (media_nonmedia == 'M' || media_nonmedia == '2') sortselect += ' AND WITH PRODUCTION ""'
    if (media_nonmedia == 'J' || media_nonmedia == 'N' || media_nonmedia == '3') sortselect += ' AND WITH PRODUCTION "Y"'
    if (typeof withacno == 'undefined') withacno = false
    if (withacno) sortselect += ' AND WITH ' + ((typeof withacno == 'string') ? (withacno + '_') : '') + 'ACNO'
    if (onlygroups) sortselect += ' AND WITH NO GROUP'
    var coldefs = [['NAME', 'Name'], ['CODE', 'Code']]
    if (onlydepartments) {
        coldefs[coldefs.length] = ['DEPARTMENT', 'Department']
    }
    else {
        coldefs[coldefs.length] = ['GROUP', 'Group']
        coldefs[coldefs.length] = ['INCOME_ACNO2', 'Income A/c']
        coldefs[coldefs.length] = ['COST_ACNO2', 'Cost A/c']
    }
    return neosysfilepopup('MEDIA.TYPES', coldefs, 1, sortselect, many)
}

function agency_val_typecode(media_nonmedia, withacno, onlygroups) {

    if (!gvalue) return true

    //get the type
    var type = []
    if (!type.neosysread('MEDIA.TYPES', gvalue)) return neosysinvalid(type.neosysresponse)

    //check if Media/Jobs
    var reqtype = 'X'
    var typename = ''
    if (media_nonmedia == 'M' || media_nonmedia == '2') {
        reqtype = 'M'
        typename = 'media'
    }
    if (media_nonmedia == 'N' || media_nonmedia == '3') {
        reqtype = 'N'
        typename = 'job'
    }

    //fix a problem where SELECT doesnt call postread on every record
    //to convert records to external format
    var type3 = type[3]
    if (type3 == 'Y') type3 = 'N'//jobs
    if (type3 == '') type3 = 'M'//media

    if (reqtype != 'X') {
        if (type3 != reqtype) return neosysinvalid(neosysquote(gvalue) + ' is not a ' + typename + ' type')
    }

    //check if has account number
    if (withacno && (withacno == 'INCOME' && !type[5])
 || (withacno == 'COST' && !type[6])
 || (withacno && !type[5] && !type[6])) return neosysinvalid(neosysquote(gvalue) + ' type does not have an ' + (typeof withacno == 'string' ? withacno.toLowerCase() : '') + ' account number so cannot be used here.')

    //check if a group code
    if (onlygroups && type[10]) return neosysinvalid(neosysquote(gvalue) + ' type has a group code itself\rand cannot be used here.')

    return true

}

//validate a key and get a record (user may choose alternative records)
function agency_findrecord(filename, key0, cols, returncolid, media_nonmedia, allownew, many) {

    //check allowed to create new
    if (allownew) allownew = neosyssecurity(filename.neosyssingular() + ' CREATE')

    //use gvalue if no key
    var key = key0
    if (!key) key = gvalue

    var proxy
    var request
    switch (filename) {
        case 'SUPPLIERS':
            {
                proxy = 'AGENCY'
                request = 'VAL.SUPPLIER'
                if (media_nonmedia) request += '.' + media_nonmedia
                break
            }
        case 'VEHICLES':
            {
                proxy = 'AGENCY'
                request = 'VAL.VEHICLE'
                break
            }
        case 'CLIENTS':
            {
                proxy = 'AGENCY'
                request = 'VAL.CLIENT'
                break
            }
        case 'BRANDS':
            {
                proxy = 'AGENCY'
                request = 'VAL.BRAND'
                break
            }
        default:
            {
                return neosysinvalid(filename + ' not allowed in agency_findrecord()')
            }

    }

    //standardise columns and determine column names
    var colids = []
    if (typeof cols == 'string') cols = cols.split(' ')
    if (cols) {
        for (var coln = 0; coln < cols.length; coln++) {
            var col = cols[coln]

            if (typeof col == 'string') col = [col]
            if (!col[1]) col[1] = col[0].neosyscapitalise()
            cols[coln] = col

            colids[coln] = cols[coln][0]

        }
    }
    else cols = [['ID', 'Code']]

    //try to get the record out of the cache
    //otherwise get the record or list of possible records
    var cacherequest = 'READ\r' + filename + '\r' + key
    if (!(db.data = readcache(cacherequest))) {
        var record = []
        db.request = 'EXECUTE\r' + proxy + '\r' + request + '\r' + filename + '\r' + key + '\r' + colids.join(' ') + '\rXML'
        if (!db.send()) {

            //allow creation of new record
            if (allownew && db.response.indexOf('cannot be found') + 1) {
                if (neosysdecide(key + ' does not exist', 'Create new record:Cancel') == 1) return true
                return false
            }

            neosysinvalid('Error: ' + db.response)
            return false
        }
    }

    /* moved down
    //allow creation of new record OR search for existing
    if (allownew)
    {
    var reply=neosysdecide(key+' does not exist','Search for existing records containing '+key+':Create new record:Cancel')
    if (!reply||reply==3) return false
    if (reply==2) return true
    }
    */

    //got list of possible records
    if (db.data.slice(0, 8) == '<records') {

        //allow creation of new record OR search for existing
        if (allownew) {
            var reply = neosysdecide(key + ' does not exist', 'Search for existing records containing ' + key + ':Create new record:Cancel')
            if (!reply || reply == 3) return false
            if (reply == 2) return true
        }

        //user decides or fail
        if (!cols) cols = 'ID'
        if (!(key = neosysdecide2('', db.data, cols, returncolid, '', allownew || many))) return false

        //setup list of keys for browse

        if (typeof key == 'object') {
            if (many) return key
            gkeys = key
            gkeyn = 0
            key = key[0]
        }

        //get the actual record
        var record = []
        if (!record.neosysread(filename, key)) return neosysinvalid(record.neosysresponse)
    }

    //got actual record immediately
    else {

        //if entered correct key in multivalued field
        if (many) return [key]

        //save in the cache as if a normal read
        writecache(cacherequest, db.data)

        //built the normal record format
        record = (key + fm + db.data).split(fm)

    }

    //update gvalue if using it
    if (!key0) gvalue = record[0]

    return record

}

function agency_pop_analdesigns(many) {
    var sortselect = 'BY KEY1 WITH KEY2 "ANALDESIGN"'
    return neosysfilepopup('DEFINITIONS', [['F9', 'Title'], ['KEY1', 'Code'], ['F8', 'Designer']], 1, sortselect, many)
}

function validcode2(companycode, clientcode, brandcode, reqauth, action) {

    //returns 1=ok 0=fail see gmsg
    gmsg = ''

    if (typeof companycode == 'undefined') companycode = ''
    if (typeof clientcode == 'undefined') clientcode = ''
    if (typeof brandcode == 'undefined') brandcode = ''
    if (typeof reqauth == 'undefined') reqauth = true//also 2=prevent override
    if (typeof action == 'undefined') action = 'create a new document'//sometimes 'approve a document'

    //check company access
    if (companycode) {
        if (!neosyssecurity('COMPANY ACCESS ' + neosysquote(companycode))) return 0
    }

    //check brand access
    if (brandcode) {

        //check brand access
        if (!neosyssecurity('BRAND ACCESS ' + neosysquote(brandcode))) return 0

        //get brand
        var brand = []
        if (!brand.neosysread('BRANDS', brandcode)) {
            gmsg = brand.neosysresponse
            return 0
        }

        //this will trigger client validation below
        clientcode = brand[1].split(vm)[0]

        //prevent stopped brands being added to new or existing records 
        if (gKeyNodes.length) {
            //tt=brand[2].toUpperCase()
            tt = brand.join(fm).toUpperCase()
            //this will trigger on the word (STOP) in the brand name or the client name
            //(or the higher brand levels if the full brand name is added to the brand file later)
            if (tt.indexOf('<STOP>') >= 0 || tt.indexOf('(STOP)') >= 0) {
                tt = tt.split(fm)
                gmsg = tt[2] + ' (' + brandcode + ')\r' + tt[3] + ' (' + brand[1] + ')\r\rYou cannot ' + action + ' for this brand because it is stopped'
                return 0
            }
        }

    }

    if (clientcode) {

        //check client access
        if (!neosyssecurity('CLIENT ACCESS ' + neosysquote(clientcode))) return 0

        //get the client
        var client = []
        if (!client.neosysread('CLIENTS', clientcode)) {
            gmsg = client.neosysresponse
            return 0
        }

        //prevent stopped clients being added to new or existing records
        if (gKeyNodes.length) {

            //prevent use of client to all if stop in name or "stopped"
            tt = client[1].toUpperCase() + client[35].toUpperCase()
            if (tt.indexOf('<STOP>') >= 0 || tt.indexOf('(STOP)') >= 0) {
                if (gpreviouselement.id != 'CLIENT_CODE') {
                    //adding to any other field but the key field
                    gmsg = client[1] + '\r\rYou cannot ' + action + ' for this client because it is stopped'
                    if (client[35]) gmsg += '\r\r' + client[35]
                    gmsg += '\r\r(See the Client File)'
                    gmsg = gmsg.neosysswap('<', '&lt;').neosysswap('>', '&gt;')
                    return 0
                }
            }

            if (client[35]) {

                //block if not authorised to use stopped clients
                //if ((gdatafilename != 'PLANS' && !neosyssecurity('CLIENT CREATE STOPPED')) {
                if (reqauth == 2 || (reqauth && !neosyssecurity('CLIENT CREATE STOPPED'))) {
                    msg = client[1] + '\r\rYou cannot ' + action + ' for this client because'
                    msg += '\r\r' + client[35]
                    msg += '\r\r(See the Client File)'
                    if (reqauth != 2)
                        msg += '\r\ryou are not authorised to use stopped clients.\r\r' + gmsg
                    gmsg = msg
                    return 0
                }

                //otherwise warn and get confirmation to use this client
                else {
                    if (gdatafilename != 'CLIENTS' && !neosysyesno(client[1] + '\r\r<B>' + client[35] + '</B>\r\rAre you SURE that you want to ' + action + ' for this client?', 2)) {
                        gmsg = ''
                        return 0
                    }
                }

            } //of has stopped

        } //of bound form

    } //of client code

    return 1

}

function dict_AGENCYFILTER(parameters) {

    //returns an array representing the entry fields

    var dict = []//of dictrecs
    var din = -1

    var g = 0//group number

    //field 21-99 is reserved for agencyfilter
    dict = addagencyfilter(dict, parameters)

    return dict

}

function addagencyfilter(dict, parameters) {

    //field offset+21-99 is agencyfilter
    //?!field 40,42,43 overlap usage in jobsfilter and agencyfilter .. job filter could be moved to 50,52,53?

    //determine the last group no
    var g = 0
    for (var din = 0; din < dict.length; ++din) {
        if (dict[din].groupno > g) g = dict[din].groupno
    }

    var offset = parameters.offset
    if (!offset)
        offset = 0

    //determine the number of entries
    var din = dict.length - 1

    var di = dict[++din] = dictrec('MEDIA_NONMEDIA', 'F', offset + 21, 'Media/Jobs', 0, '', '', '', '', 'L', '10')
    di.radio = '1;Media and/or Jobs:2;Media only:3;Jobs only'
    di.required = true
    di.defaultvalue = '1'

    di = dict[++din] = dictrec('COMPANY_CODE', 'F', offset + 22, 'Company Code', ++g)
    general_dict_companycode(di, true)

    di = dict[++din] = dictrec('COMPANY_NAME', 'S', '', 'Company Name', g)
    general_dict_companyname(di)

    //NB CLIENT_GROUP_CODE and CLIENT_GROUP_NAME is at position 48 below
    //NB SUPPLIER_GROUP_CODE and SUPPLIER_GROUP_NAME is at position 50 below

    ++g
    di = dict[++din] = dictrec('CLIENT_CODE', 'F', offset + 23, 'Client Code', g)
    if (gdatafilename == 'INVOICEREPRINT')
        di.groupno = 0
    agency_dict_clientcode(di, '', true)

    di = dict[++din] = dictrec('CLIENT_NAME', 'S', '', 'Client Name', g, '', '', '', '', 'T', '30')
    if (gdatafilename == 'INVOICEREPRINT')
        di.groupno = 0
    agency_dict_clientname(di)

    ++g
    di = dict[++din] = dictrec('BRAND_CODE', 'F', offset + 24, 'Brand Code', g)
    agency_dict_brandcode(di, 'getvalues("CLIENT_CODE")', true)

    di = dict[++din] = dictrec('BRAND_NAME', 'S', '', 'Brand Name', g, '', '', '', '', 'T', '30')
    agency_dict_brandname(di)

    di = dict[++din] = dictrec('TYPE_CODE', 'F', offset + 25, 'Type Code', ++g, '', '', '', '', 'L', '5')
    //agency_dict_typecode(di,'getvalue("MEDIA_NONMEDIA")',true)
    agency_dict_typecode(di, 'gds.getx("MEDIA_NONMEDIA")', true)

    di = dict[++din] = dictrec('TYPE_NAME', 'S', '', 'Type Name', g, '', '', '', '', 'T', '30')
    agency_dict_typename(di)

    di = dict[++din] = dictrec('SUPPLIER_CODE', 'F', offset + 26, 'Supplier Code', ++g, '', '', '', '', 'L', '5')
    //agency_dict_suppliercode(di,'getvalue("MEDIA_NONMEDIA")',true)
    agency_dict_suppliercode(di, 'gds.getx("MEDIA_NONMEDIA")', true)

    di = dict[++din] = dictrec('SUPPLIER_NAME', 'S', '', 'Supplier Name', g, '', '', '', '', 'T', '30')
    agency_dict_suppliername(di)

    di = dict[++din] = dictrec('MARKET_CODE', 'F', offset + 27, 'Market Code', ++g, '', '', '', '', 'L', '5')
    di.defaultvalue = ''
    general_dict_marketcode(di, true)

    di = dict[++din] = dictrec('MARKET_NAME', 'S', '', 'Market Name', g, '', '', '', '', 'T', '30')
    general_dict_marketname(di)

    //media only
    di = dict[++din] = dictrec('VEHICLE_CODE', 'F', offset + 28, 'Vehicle Code', ++g, '', '', '', '', 'L', '5')
    if (typeof media_dict_vehiclename != 'undefined') media_dict_vehiclecode(di, true)
    di.popup = 'media_pop_vehicles(gds.getx("TYPE_CODE",null),true)'

    //media only
    di = dict[++din] = dictrec('VEHICLE_NAME', 'S', '', 'Vehicle Name', g, '', '', '', '', 'T', '30')
    if (typeof media_dict_vehiclename != 'undefined') media_dict_vehiclename(di)

    di = dict[++din] = dictrec('PERIODS', 'F', offset + 29, 'Period', ++g)
    neosys_dict_period(di)
    di.conversion = '[YEAR_PERIOD]'
    di.popup = 'agency_pop_periods(gds.getx("DOCUMENT_TYPE"),true)'
    //di.rowrequired=true

    di = dict[++din] = dictrec('FROM_DATE', 'F', offset + 30, 'From Date', '', '')
    neosys_dict_date(di, 'TO=UPTO_DATE')
    // di.defaultvalue='TODAY'
    //if (gdatafilename!='JOBLIST'&&gdatafilename!='MEDIADIARY'&&gdatafilename!='AGENCYSEARCH')
    if (gparameters.MODE && gparameters.MODE.indexOf('DEADLINE') >= 0)
        di.defaultvalue = 'TODAY'

    if (!['BOOKINGLIST', 'INVOICELIST', 'JOBLIST', 'MEDIADIARY', 'AGENCYSEARCH', 'AGENCYFILTER', 'INVOICEREPRINT'].neosyslocate(gdictfilename)) di.required = true

    di = dict[++din] = dictrec('UPTO_DATE', 'F', offset + 31, 'Upto Date', '')
    neosys_dict_date(di, 'FROM=FROM_DATE')
    // di.required=true
    di.defaultvalue = 'gds.getx("FROM_DATE")'

    //media only 
    di = dict[++din] = dictrec('SHOW_VEHICLES', 'F', offset + 32)
    //neosys_dict_yesno(di)
    //di.defaultvalue = '"N"'
    //di.required = true
    di.checkbox = 'Y;'
    di.display = 'return this.getx("MEDIA_NONMEDIA")!=3'

    di = dict[++din] = dictrec('INVOICE_NO', 'F', offset + 33, '', ++g)
    neosys_dict_number(di)

    di = dict[++din] = dictrec('SUPPLIER_INVOICE_NO', 'F', offset + 34, '', ++g)

    di = dict[++din] = dictrec('PERIOD', 'F', offset + 35, 'Current Period', 0, '', '', '', '', 'L', 10)
    general_dict_period(di, 'FROM', 'PERIOD_TO')
    //var temp=''
    //for (var year=2000;year<=2001;year++)
    //{
    // for (var period=0;period<=12;period++)
    // {
    //  var temp2=period+'/'+year
    //  temp+=':'+temp2+';'+temp2
    // }
    //}
    //di.conversion=temp
    di.defaultvalue = neosysgetcookie(glogincode, 'NEOSYS2', 'pd').neosysquote()
    if (['INVOICELIST', 'INVOICEREPRINT'].neosyslocate(gdatafilename)) di.defaultvalue = ''
    if (!['INVOICELIST', 'INVOICEREPRINT'].neosyslocate(gdatafilename))
        di.required = true

    di = dict[++din] = dictrec('PERIOD_TO', 'F', offset + 36)
    general_dict_period(di, 'UPTO', 'PERIOD')
    //di.conversion='[YEAR_PERIOD]'
    //neosys_dict_period(di)
    //di.defaultvalue=neosysgetcookie(glogincode,'NEOSYS2','pd').neosysquote()
    //di.required=true

    di = dict[++din] = dictrec('OUTPUT_FORMAT', 'F', offset + 37)
    di.radio = 'htm;Normal Report:xls;Excel Worksheet (must actually click the "List" button):txt;Tabbed Text File'
    di.defaultvalue = '"htm"'
    di.required = true

    di = dict[++din] = dictrec('TIME_FROM', 'F', offset + 38)
    neosys_dict_time(di, 'FROM', 'TIME_UPTO')

    di = dict[++din] = dictrec('TIME_UPTO', 'F', offset + 39)
    neosys_dict_time(di, 'UPTO', 'TIME_FROM')

    //?!field 40,42,43 overlap usage in jobsfilter and agencyfilter .. job filter could be moved to 50,52,53?

    //used in:
    //invoicelist.htm
    //invoicereprint.htm
    //agency.js x2
    //jobs.js x2
    //media_dict.js
    //ratecardprint_dict.js
    //bookinglist.htm
    di = dict[++din] = dictrec('ORDER', 'F', offset + 40)
    //di.radio='4;By Date:2;By Client:6;By Brand:7;By Market'
    di.radio = '4;By Date:9;By Company:2;By Client:6;By Brand'
    if (window.document.location.toString().indexOf('bookinglist') < 0)
        di.radio += ':7;By Market'
    //if (parameters.MODE&&parameters.MODE.indexOf('PURCHASE')>=0)
    //di.radio+=':8;By Media Type'
    if (window.document.location.toString().indexOf('media') >= 0 || (parameters.MODE && parameters.MODE.indexOf('PURCHASE') >= 0)) {
        di.radio += ':5;By Supplier'
    }
    if (window.document.location.toString().indexOf('media') >= 0) {
        //if (window.document.location.toString().indexOf('bookinglist') < 0)
        di.radio += ':10;By Vehicle'
    }
    //else
    di.radio += ':1;By Executive'
    //if (parameters.MODE&&parameters.MODE.indexOf('ESTIMATE')>=0)
    if (window.document.location.toString().indexOf('joblist') >= 0) {
        //orders/purchasae invoices, estimate and sales invoices have status but jobs do not
        if (parameters.MODE && (parameters.MODE.indexOf('PURCHASE') >= 0 || parameters.MODE.indexOf('INVOICE') >= 0 || parameters.MODE.indexOf('ESTIMATE') >= 0))
            di.radio += ':3;By Status'
    }
    di.required = true
    di.defaultvalue = '4'

    //moved from 
    di = dict[++din] = dictrec('EXECUTIVE_CODE', 'F', offset + 41, '', ++g)
    agency_dict_executivecode(di, (gparameters.MEDIA_NONMEDIA == 2) ? 'SCHEDULES' : 'JOBS', allownew = false, popmany = true)

    /* MATDEL
    di=dict[++din]=dictrec('ORDER', 'F',5)
    //di.radio='1;By Date:2;By Client:3;By Brand:4;By Supplier:5;Total only'
    di.radio='1;By Date:2;By Client:3;By Brand:4;By Supplier'
    di.required=true
    di.defaultvalue='1'
    */

    //why is 42-47 not offset+42 etc? presumably they happen not used where offset is required

    //?!field 40,42,43 overlap usage in jobsfilter and agencyfilter .. job filter could be moved to 50,52,53?

    di = dict[++din] = dictrec('SUMMARY', 'F', 42)
    di.radio = '0;Details and subtotals:1;Subtotals with drilldown to details:2;Subtotals only'
    if (parameters.MODE == 'PROGRESS' || parameters.MODE == 'FREEADS')
        di.defaultvalue = '2'
    else
        di.defaultvalue = '0'

    //?!field 40,42,43 overlap usage in jobsfilter and agencyfilter .. job filter could be moved to 50,52,53?

    di = dict[++din] = dictrec('CURRENT_COMPANY_CODE', 'F', '43')
    general_dict_companycode(di, false, true)
    if (gdatafilename == 'INVOICEREPRINT')
        di.defaultvalue = '""'
    else {
        di.required = true
        di.defaultvalue = gcompanycode.neosysquote()
    }

    //media only
    //di=dict[++din]=dictrec('PAGE_BREAKS', 'F',44,'',++g)
    di = dict[++din] = dictrec('ROW_FIELD', 'F', 45, '', ++g)
    di.rowrequired = true

    var pop = ''
    pop = 'COMPANY_NAME'
    pop += ':COMPANY_CODE'
    pop += ':EXECUTIVE_NAME'
    pop += ':'
    pop += ':PRINCIPLE_NAME'
    pop += ':PRINCIPLE_CODE'
    pop += ':CLIENT_NAME'
    pop += ':CLIENT_CODE'
    pop += ':PRODUCT_CATEGORY_NAME'
    pop += ':PRODUCT_CATEGORY_CODE'
    pop += ':BRAND1_NAME;Brand Group Name'
    pop += ':BRAND1_CODE;Brand Group Code'
    pop += ':BRAND_NAME'
    pop += ':BRAND_CODE'
    pop += ':CAMPAIGN'
    pop += ':VEHICLE_CURRENCY_NAME'
    pop += ':VEHICLE_CURRENCY_CODE'
    pop += ':INVOICE_CURRENCY_NAME'
    pop += ':INVOICE_CURRENCY_CODE'
    pop += ':'
    pop += ':MAIN_MARKET_NAME'
    pop += ':MAIN_MARKET_CODE'
    pop += ':MARKET_NAME'
    pop += ':MARKET_CODE'
    pop += ':SUPPLIER_NAME'
    pop += ':SUPPLIER_CODE'
    pop += ':MEDIA_TYPE_NAME'
    pop += ':MEDIA_TYPE_CODE'
    pop += ':VEHICLE_NAME'
    pop += ':VEHICLE_CODE'
    pop += ':'
    //SPECIFICATION
    pop += ':RATECARD_ROW'
    pop += ':RATECARD_COLUMN'
    pop += ':SIZE_DURATION'
    //REPEAT
    pop += ':'
    //pop=':DATE
    //pop=':DEADLINE
    pop += ':TIME2;Time'//(BREAK_TIME OR TIME OR RATING_TIME1)
    //TIME3 (BREAK_TIME OR TIME OR RATING_TIME_RANGE)
    //RATING_TIME
    //RATING_TIME2
    pop += ':BREAK_TIME;;Time of TV commercial break'
    pop += ':BREAK_POSITION;;Position in TV commercial break'
    pop += ':TIME2BREAK;;Time and position in TV commercial break'
    pop += ':'
    pop += ':MATERIAL_LETTER;;One letter code'
    pop += ':LETTER_X;;The letter "X"'
    pop += ':MATERIAL_DESCRIPTION'
    pop += ':MATERIAL_REQUIREMENTS;;of the vehicle'
    pop += ':DETAILS;Position/Program'
    pop += ':EXTRAS;;Discounts/commissions/fees/taxes to client'
    pop += ':COST_EXTRAS;;Discounts/commissions/fees/taxes to agency'
    pop += ':'
    pop += ':SCHEDULE_NO'
    pop += ':CLIENT_ORDER_NO'
    pop += ':ORDER_NO;Agency Order No'
    pop += ':CERTIFICATE_NO'
    pop += ':SUPP_INV_NO;Supplier Invoice No'
    pop += ':INV_NO;Agency Invoice No'

    di.conversion = pop

    di = dict[++din] = dictrec('CALENDAR_BREAK', 'F', 46, '')
    di.radio = '1;Days (page per month):2;Days:3;Weeks:4;Months'
    di.horizontal = true
    di.defaultvalue = 1

    //di=dict[++din]=dictrec('COLUMN_BREAKS', 'F',46,'',++g)
    ////////////////////////////////////////////////////////
    di = dict[++din] = dictrec('DATA_FIELD', 'F', 47, '', ++g)
    di.rowrequired = true

    //NB continues from the previous pop!
    pop += ':TOTAL_SECONDS'
    pop += ':TOTAL_SIZE_DURATION'
    pop += ':TOTAL_NUMBER_OF_ADS'
    pop += ':WEEKLY_NUMBER_OF_ADS;Number of Ads per Week'

    pop += ':TOTAL_GROSS_LOCAL_BILL_AND_CURR;Gross Cost to Client (vehicle currency)'
    pop += ':TOTAL_NET_LOCAL_BILL_AND_CURR;Net Cost to Client (vehicle currency)'
    pop += ':TOTAL_BILL_AMOUNT_AND_CURR;Net Cost to Client (invoice currency)'
    pop += ':TOTAL_PAID_BILL_AMOUNT_BASE;Net Cost to Client (in ' + gbasecurr + ')'

    pop += ':TOTAL_GROSS_LOCAL_COST_AND_CURR;Gross Cost to Agency (vehicle currency)'
    pop += ':TOTAL_NET_LOCAL_COST_AND_CURR;Net Cost to Agency (vehicle currency)'
    pop += ':TOTAL_PAID_COST_CURR;Net Cost to Agency (vehicle currency) without currency code'
    pop += ':TOTAL_PAID_COST_AMOUNT_BASE;Net Cost to Agency (in ' + gbasecurr + ')'

    pop += ':TOTAL_NET_LOCAL_MARGIN_AND_CURR;Margin (vehicle currency)'
    pop += ':TOTAL_MARGIN_AMOUNT_BASE;Margin (in ' + gbasecurr + ')'

    di.conversion = pop

    di = dict[++din] = dictrec('CURRENT_COMPANY_NAME', 'S')
    general_dict_companyname(di, 'CURRENT_COMPANY_CODE')

    di = dict[++din] = dictrec('CLIENT_GROUP_CODE', 'F', offset + 48, '', ++g)
    agency_dict_clientcode(di, 'gds.getx("MEDIA_NONMEDIA")', many = true, groups = true)

    di = dict[++din] = dictrec('CLIENT_GROUP_NAME', 'S', '', '', g)
    agency_dict_clientname(di, 'CLIENT_GROUP_CODE')

    di = dict[++din] = dictrec('CLIENT_ORDER_NO', 'F', offset + 49, '', ++g)

    di = dict[++din] = dictrec('SUPPLIER_GROUP_CODE', 'F', offset + 50, '', ++g)
    agency_dict_suppliercode(di, 'gds.getx("MEDIA_NONMEDIA")', many = true, groups = true)

    di = dict[++din] = dictrec('SUPPLIER_GROUP_NAME', 'S', '', '', g)
    agency_dict_suppliername(di, 'SUPPLIER_GROUP_CODE')

    di = dict[++din] = dictrec('PRODUCT_CATEGORY_CODE', 'F', offset + 51, '', ++g)
    agency_dict_productcategorycode(di, 'gds.getx("MEDIA_NONMEDIA")', many = true)

    di = dict[++din] = dictrec('PRODUCT_CATEGORY_NAME', 'S', '', '', g)
    agency_dict_productcategoryname(di, 'PRODUCT_CATEGORY_CODE')

    ++g

    di = dict[++din] = dictrec('USER_ID', 'F', offset + 52, '', g)
    di.popup = 'general_pop_users(many=true)'
    di.validation = 'general_val_users()'
    di.filename = 'USERS'

    di = dict[++din] = dictrec('USER_NAME', 'S', '', '', g)
    di.functioncode = 'return this.getx("USER_ID").neosysxlate("USERS",1)'
    di.align = 'T'
    di.length = 30

    //di = dict[++din] = dictrec('PAYMENT_STATUS', 'F', offset + 53)
    //di.checkbox = '1;Paid:2;Unpaid'
    di = dict[++din] = dictrec('PAYMENT_RECEIVED', 'F', offset + 53)
    di.radio = '1;Yes:2;No:12;All'
    //undefault so receipt details dont come by default ... really needs specific checkbox for that
    //di.defaultvalue='"1:2"'
    di.horizontal = true
    neosyssecurity(((!parameters.MEDIA_NONMEDIA || parameters.MEDIA_NONMEDIA == 2) ? 'MEDIA' : 'PRODUCTION') + ' INVOICE ACCESS RECEIPT')
    di.readonly = gmsg

    di = dict[++din] = dictrec('CLIENT_GROUP2_CODE', 'F', offset + 54, '', ++g)
    agency_dict_clientcode(di, 'gds.getx("MEDIA_NONMEDIA")', many = true, groups = true, members = false, 2)

    di = dict[++din] = dictrec('CLIENT_GROUP2_NAME', 'S', '', '', g)
    agency_dict_clientname(di, 'CLIENT_GROUP2_CODE')

    di = dict[++din] = dictrec('DEBIT_CREDIT_TYPE', 'F', offset + 55)
    di.checkbox = '1;Invoices:2;Credit Notes'
    di.defaultvalue = '"1:2"'
    di.horizontal = true

    //di=dict[++din]=dictrec('BILLINGS_BASED_ON', 'F',offset+56)
    //di.radio=';Default:1;Finance (Journals):2;Operations (Invoices)'

    di = dict[++din] = dictrec('MATERIAL_CODE', 'F', offset + 57, '', ++g)

    di = dict[++din] = dictrec('FIRST_APPEARANCE_DATE', 'F', offset + 58, '', ++g)
    neosys_dict_date(di)

    di = dict[++din] = dictrec('EMAIL_TO', 'F', offset + 59)
    di.checkbox = 'EXECUTIVE;Executive'

    di = dict[++din] = dictrec('WITHIN_DAYS', 'F', offset + 60)
    neosys_dict_number(di, 0)

    //look back at ads even after they have appeared
    di = dict[++din] = dictrec('AFTER_DAYS', 'F', offset + 61)
    neosys_dict_number(di, 0)

    //similar in media_dict.js and agencyfilter.hs
    di = dict[++din] = dictrec('STOPPED', 'F', offset + 62)
    di.checkbox = '1;Open:2;Stopped'
    di.horizontal = true
    di.required = true
    di.defaultvalue = '1'

    di = dict[++din] = dictrec('DELIVERY', 'F', offset + 63)
    di.radio = '0;Interactive:1;Email Body:2;Email Attachment'
    di.horizontal = true
    di.required = true
    di.defaultvalue = '0'

    dict.g = g
    return dict

}

function agency_open_invoice(mode, companycode, year, invoiceno) {

    if (!mode) mode = 1
    if (mode == 'BILL') mode = 1
    if (mode == 'COST') mode = 2
    if (mode == 'BOTH') mode = 3

    if (!companycode) {
        companycode = gds.regetx('COMPANY_CODE')
        if (!companycode) companycode = gds.getx('COMPANY_CODE')
        if (typeof companycode == 'object') companycode = companycode[0]
    }

    if (!year) year = ''

    if (!invoiceno) {
        var tt = ''
        if (mode.toString().indexOf('PROFORMA') >= 0)
            tt = 'PROFORMA_'
        invoiceno = gds.getx(tt + 'INVOICE_NO')
        if (typeof invoiceno == 'object') invoiceno = invoiceno[0]
    }

    if (!invoiceno) return neosysinvalid('This has not been invoiced')

    if (invoiceno.indexOf(',')) {
        invoiceno = invoiceno.neosysconvert(' ', '')
        var reply = neosysdecide('', invoiceno.split(','), '', 0, '', 1);
        //function neosysdecide(question,data,cols,returncoln,defaultreply,many,inverted)
        if (!reply)
            return neosysinvalid()
        invoiceno = reply.join(',')
    }

    var data = invoiceno
    //d+fm+fm+mode+fm+companycode+fm+year
    data = data.neosysreplace(3, 0, 0, mode)
    data = data.neosysreplace(4, 0, 0, year)
    data = data.neosysreplace(43, 0, 0, companycode)
    return openwindow('EXECUTE\r\AGENCY\r\INVOICEREPRINT', data)

}