//Copyright NEOSYS All Rights Reserved.

//////////////////
function neosysdatasource() {

    this.data = null
    this.onreadystatechange = null
    this.readystate = 'initialised'

    //async methods (capitalised only to evade yield conversion programs)
    //require yield * in call like "abc=YIELD* GDS.GETX(..."
    this.xlate = gds_xlate
    this.rexlate = gds_rexlate
    this.evaluate = gds_evaluate
    this.regetx = gds_regetx
    this.get1 = gds_get1
    this.getall = gds_getall
    this.getx = gds_getx
    this.setx = gds_setx
    this.setx2 = gds_setx2
    this.bind = gds_bind
    this.bindgroup = gds_bindgroup
    this.rebind = gds_rebind
    this.load = gds_load
    //set in dbform.js if required
    //this.onreadystatechange=gds_onreadystatechange
    
    //sync methods
    this.getcells = gds_getcells
    this.setdefaulted = gds_setdefaulted
    this.updaterow = gds_updaterow
    this.appendrow = gds_appendrow
    this.createrow = gds_createrow
    this.addrow = gds_addrow
    this.insertrow = gds_insertrow
    this.blankrow = gds_blankrow
    this.blankrowx = gds_blankrowx
    this.deleterow = gds_deleterow
}

//NB arguments are fieldname, filename not filename, fieldname as in server side xlate ATM
//this is because rapid readability is vastly better due to sequence of sourcefieldname->filename/fieldno
function* gds_xlate(sourcefieldname,targetfilename,targetfieldno,mode){
    var keys=yield* this.getx(sourcefieldname)
    return yield* keys.neosysxlate(targetfilename,targetfieldno,mode)
}

function* gds_rexlate(sourcefieldname,targetfilename,targetfieldno,mode){
    var keys=yield* this.regetx(sourcefieldname)
    return yield* keys.neosysxlate(targetfilename,targetfieldno,mode)
}

function* gds_evaluate(functionx) {

    if (typeof functionx == 'boolean')
        return functionx

    if (functionx == 'true')
        return true

    if (functionx == 'false')
        return false

    if (!functionx)
        return true

    //this=whatever gds object the evaluate method has been called on
    //because of .apply() in neosysevaluate3
    var result=yield* neosysevaluate(
        functionx,   //function string or object
        'gds_evaluate',//for debugging
        'gds',  //any "gds" variable in the function code will actually access
        this,   //this (the current default gds object)
        this)   //any "this" variable in the function code will access this
                // (the local gds object) because of neosysevaluate3's .apply()

    return result
}

function* gds_regetx(dictitemorid, recn) {

    //actually is screenelement not dictitem!

    //convert id into dictitem
    var dictitem = dictitemorid
    if (typeof dictitem == 'string') {
        dictitem = this.dictitem(dictitem)
        if (!dictitem) {
            systemerror('yield* gds_regetx()', neosysquote(dictitemorid) + ' is not in the ' + gdatafilename + ' dictionary.')
            return ''
        }
    }

    this.tempfunction = dictitem.functioncode ? dictitem.functioncode : (dictitem.getAttribute ? dictitem.getAttribute('neosysfunctioncode') : '')
    //this should always be a function not source code
    if (!this.tempfunction) {

        //recalc doesnt need to be called for F items but if it is should work like getx
        if (dictitem.type == 'F')
            return yield* this.getx(dictitem.name, recn)

        return ''
    }

    var storegrecn = grecn
    if (typeof recn != 'unassigned') grecn = recn
    if (!dictitem.groupno) grecn = null

    //login('regetx ' + dictitem.name + ' ' + grecn)
    //login('regetx '+dictitem.id+' '+grecn)

    //calculate the answer (possibly multivalued)
    /* yield */ var results = /**/ yield * this.tempfunction.apply(this)
    // noyield // var results = this.tempfunction()
    if (typeof results == 'undefined') systemerror('gds_recalx', (dictitem.name ? dictitem.name : dictitem.id) + ' function returned undefined')

    //and update the record and display
    /*if (dictitem.type!='S') */
    yield* this.setx(dictitem, !dictitem.groupno ? 0 : grecn, results)

    grecn = storegrecn

    //logout('regetx ' + dictitem.name + ' ' + grecn)
    ////logout('regetx '+dictitem.id+' '+grecn)

    return results

}

//getall
function* gds_getall(name, oldtext) {
    return yield* this.getx(name,null,oldtext)
}

//get1 - should only be called on multivalued fields and only with specific recn (or null for grecn)
//purpose is to return a value when given a specific recn ... not a 1 item array like getx does
function* gds_get1(name, recn, oldtext) {
    if (recn)
        neosysassertnumeric(recn,'gds_get1','recn')
    return (yield* this.getx(name,recn,oldtext))[0]
}

//getx - NB returns an array if multivalued field even for one recn
//recn can be -1 to get the last one
function* gds_getx(name, recn, oldtext) {

    //if no data section then return nothing
    //eg in postinit setting expressions based on gds before there is any data section
    //neosyssetexpression('currencycode_span','innerText','yield* gds.getx("CURRENCY_CODE")')
    //(not necessary now that cleardoc is *always* called in forminit
    //if (!this.data) return ''

    if (!ginitok)
        return ''

    if (typeof recn == 'undefined')
        recn = grecn

    var results = []

    elements = this.getcells(name, recn)

    if (elements.length == 0) {
        systemerror('yield* gds.getx()', name + ' is not in the dictionary\ror is not in the correct group.')
        results[0] = ''
        return results
    }

    for (var ii = 0; ii < elements.length; ii++) {
        var element = elements[ii]
        results[ii] = (oldtext && (typeof element.oldtext != 'undefined')) ? element.oldtext : element.text
    }

    //if not multivalued then return single value
    if (elements.groupno == 0)
        results = results[0]

    return results

}

//GETCELLS
//////////
//recn can be negative to get elements from the end
function gds_getcells(dbelementordbelementid, recn) {

    if (!this.data) {
        systemerror('gds_getcells', 'this.data is unassigned')
        return []
    }

    //return an array of objects that have text properties containing the data
    //for a specific field (all or one row - array with one element)
    //return emptyarray if dbelementid does not exist

    //given a dbelement, get target cell, or cells if recn=null


    //this seems to solve a problem (at least 
    // where onfocus events are happening at the same time as
    //client_onload thread ... who knows why you can put anything instead of return
    //alert('x') puts up an alert but you cannot seem force into debugger eg xxx() is allowed!!!
    if (!ginitok)
        return

    //login('getcells ' + dbelementordbelementid + ' ' + recn)

    var dictid
    var dbelement
    var dbelement2
    var groupno

    //if passed an object assume that it is a neosys HTML data element
    //or a dictitem
    if (typeof (dbelementordbelementid) == 'object') {
        if (dbelementordbelementid.tagName) {
            dictid = dbelementordbelementid.id
            groupno = Number(dbelementordbelementid.getAttribute('neosysgroupno'))
        }
        else {
            dictid = dbelementordbelementid.name
            groupno = dbelementordbelementid.groupno
        }
        if (!dictid)
            return systemerror('gds_getcells', 'dbelement does not have a name or id\r')
    }
    //if given a dbelementid, convert to (first) matching dbelement
    else {

        //gets all elements if multivalued
        //gets first element only (on PC (mac?) dbelement2[0]==dbelement is true)
        //dbelement2=document.all(dbelementordbelementid)

        //find dbelement in dom
        //dbelement=document.getElementsByName(dbelementordbelementid)[0]
        //if (dbelement)
        //{
        // dictid=dbelementordbelementid
        // groupno=Number(dbelement.getAttribute('neosysgroupno'))
        //}
        //else

        //otherwise try to find it in the dictionary
        {
            var dictitem = this.dictitem(dbelementordbelementid)
            if (!dictitem) {
                systemerror('getcells', neosysquote(dbelementordbelementid) + ' db element is missing')
                //logout('getcells - no db element - returning []')
                return []
            }

            dictid = dictitem.name
            groupno = dictitem.groupno

        }

        //NB mac SELECT tags are arrays of options as well
        //if (dbelement.length>1) dbelement=dbelement[0]

    }

    //get the SELECT node of OPTION elements
    //if (dbelement.tagName=='OPTION') dbelement=dbelement.parentNode

    if (groupno == 0)
        cells = [this.data[dictid]]
    else {

        var datasource
        datasource = this.data['group' + groupno]
        if (!datasource)
            systemerror('getcells()', dictid + '  group:' + groupno + ' doesnt exist')

        if (recn<0) {
            recn=datasource.length+recn
            if (recn<0)
                recn=0
        }
        
        var startrown
        var endrown
        if (recn == null) {
            startrown = 0
            endrown = datasource.length - 1
        }
        else {
            startrown = recn
            endrown = recn
        }
        var cells = []
        for (var rown = startrown; rown <= endrown; rown++) {
            //some cases like calling getx for one grouped dictid from a dictid belonging to a different group number
            //cause errors when the line numbers dont agree. this should be treated as a bug in the application code
            try {
                cells[cells.length] = datasource[rown][dictid]
            }
            catch (e) {

                var errmsg = dictid + ' group:' + groupno + ' rown:' + rown + ' rown doesnt exist?'
                if (e.description)
                    errmsg + '\n' + e.description
                systemerror('getcells()', errmsg)

                //handle beyond end of rows as an programming error since is rare to requires this and is usually just the result of an error
                if (rown >= datasource.length) {
                    var errmsg = 'Maximum row is No. ' + datasource.length
                    systemerror('getcells()', errmsg)
                }

                //otherwise guess is a programming error due to mismatching group
                //eg cases like calling getx for one grouped dictid from a dictid belonging to a different group number
                else {
                    //cause errors when the line numbers dont agree. this should be treated as a bug in the application code
                    var errmsg = dictid + ' rown ' + rown + ' but nrows ' + datasource.length
                    if (gpreviouselement)
                        errmsg += '\r(maybe different group from ' + gpreviouselement.id + ')'
                    if (e.description)
                        errmsg + '\n' + e.description
                    systemerror('getcells()', errmsg)
                }

            }
        }
    }

    if (!cells[0])
        systemerror('getcells()', dictid + '  has no cells')

    //save the groupno and dictid
    cells.groupno = groupno
    cells.dictid = dictid

    //logout('getcells')

    return cells

}

//setx
///////////
function* gds_setx(element, recn, values) {

    //given a screen element or dbelement or element name
    //and values, update the screen and record

    //called from:
    //validateupdate
    //setdefault
    //gds_regetx
    //earlyupdate
    //insertallrows
    //general user code

    //done in getcells now
    //if (typeof element=='string') element=document.getElementsByName(element)[0]
    //assertelement(element,'setx','element')

    //nb select looks like an array
    //if (element.length>1) element=element[0]

    //check values is defined
    if (typeof values == 'undefined') {
        systemerror('setx()', 'values is undefined')
        return
    }

    //check recn is null or numeric
    if (recn != null)
        neosysassertnumeric(recn, 'setx', 'recn')

    //get the cells to be updated 
    var cells = this.getcells(element, recn)
    if (!cells[0])
        return

    if (values == null)
        values = ''

    //convert single value into an array
    if (typeof (values) != 'object') {
        values = [values]
        if (recn == null) {
            var value = values[0]
            for (var ii = cells.length - 1; ii > 0; --ii)
                values[ii] = value
        }
    }

    //result = /**/ yield * this.setx2(cells, values)
    return /**/ yield * this.setx2(cells, values)
}

//SETDEFAULTED
//////////////
function gds_setdefaulted(element, recn, trueorfalse) {

    //nb setting a value to '' also clears the "defaulted" property

    //check recn is null or numeric
    if (recn != null) neosysassertnumeric(recn, 'setx', 'recn')

    //get the cells to be updated 
    var cells = this.getcells(element, recn)
    if (!cells[0]) return

    //set the cells to defaulted trueorfalse
    for (var rown = 0; rown < cells.length; rown++) {
        cells[rown].defaulted = trueorfalse
    }

    return

}

//TODO convert this and gds_setx to _sync
//make it do the async oconvertvalue (and setvalue) in a timeout
//since nothing depends on screen values except visual appearance
//only afterupdate() might be synchronous and that is only used by schedule_upd_extra which could be refactored
function* gds_setx2(cells, values, forced) {

    var conversion = false

    // for (var rown=0;rown<values.length;rown++)
    //sometimes more values than cells!
    for (var rown = 0; rown < values.length && rown < cells.length; rown++) {

        //this is cut out because
        //could enter 999.999 when the old data is 1000.00 and
        //although the internal data does not need to be updated,
        //the screen value MIGHT need updating (in this case from 999.999 to 1000.00
        //  if (cells[rown].text!=values[rown])
		if (true) {

            //prevent NaN or undefined
            var value = values[rown]
            if (typeof value != 'string' && (isNaN(value) || typeof value == 'undefined')) {
                systemerror('setx()', 'Attempting to set ' + cells.dictid + ' rown ' + (rown + 1) + ' to ' + (typeof value == 'undefined' ? 'undefined' : 'NaN') + ' is disallowed')
                continue
            }

            var rowcell = cells[rown]

            //prevent very small numbers
            if (typeof value == 'number' && Math.abs(value) < 0.0000000001) value = 0

            //save original value if not present
            if (typeof rowcell.oldtext == 'undefined') rowcell.oldtext = rowcell.text

            //get the screen element
            var screenelement = rowcell.element

            //update the recs
            if (rowcell.text != value) {
                rowcell.text = value
                cells.changed = true
            }
            //if (gro.data.group1[0].BILL.text=='0') xxxx

            //reset the defaulted property
            if (!value && rowcell.defaulted) {
                //clearing a checkbox does not allow redefaulting
                if (!screenelement || 'checkbox radio'.indexOf(screenelement.type) < 0) {
                    cells[rown].defaulted = false
                }
            }
            //update the screen if possible

            //if no screen element then
            if (!screenelement || typeof screenelement != 'object') {

                //if null then maybe same group number in different tables
                //or trying to set a field before it is bound

                //if no screen element, display an error message
                //get the id
                //if (gds.readystate!='loading')
                //{    
                // if (typeof element=='string')
                //  var elementid=element
                // else
                //  var elementid=element.id
                // alert('setx cannot set "'+elementid+'" because it has not been bound.\nMaybe dictitem has same group number in different tables.')
                //}

            }

            //if there is a screen element then update the screen
            else {

                //work out if conversion required and do any conversion
                if (rown == 0)
                    conversion = screenelement.getAttribute('neosysconversion')
                if (conversion) {
                    var ivalue = value
                    value = yield* oconvertvalue(value, conversion)
                    if (value == null) {
                        systemerror('yield* gds_setx2("' + screenelement.id + '") ' + neosysquote(ivalue) + ' is not a ' + conversion)
                        return
                    }
                }

                //update the screen value (fast unprotected)
                setvalue2(screenelement, value)

                //add new dependents to list if not already there
                if (rown == 0) {
                    var newdependents = screenelement.getAttribute('neosysdependents')
                    if (newdependents) {
                        newdependents = newdependents.split(';')
                        for (var depn = 0; depn < newdependents.length; depn++) {
                            if (!gdependents.neosyslocate(newdependents[depn]))
                                gdependents[gdependents.length] = newdependents[depn]
                        }
                    }
                }
                //alert(screenelement.name+' : '+screenelement.name+' : '+newdependents.join()+' : '+gdependents.join())
            }

        }

    } //next rown

    //call any afterupdate function
    //forced comes from rebind eg when inserting rows from another record due to data being already set
    //could be moved to end of setx( but wouldnt be called during rebind
    if (forced || cells.changed) {
        var afterupdate = this.dictitem(cells.dictid).afterupdate
        if (afterupdate) {
            try {
                var result=afterupdate(cells, values)
                /* yield */ if (result.next) result.next()
            }
            catch (e) {
                systemerror(e.description + ' ' + e.number, 'in afterupdate() in yield* gds_setx(' + cells.dictid + ')')
            }
        }
    }

    return true

}


//LOAD
//////
function* gds_load(neosysrecordobject1) {
    //alert('gds_load')
    //login('load')

    //trigger loading event
    this.loading = true
    this.readystate = 'loading'
    if (typeof (this.onreadystatechange) == 'function')
        /**/ yield * this.onreadystatechange()

    //grab the dict and dictitem method
    this.dict = neosysrecordobject1.dict
    this.dictitem = neosysrecordobject1.dictitem
    
    //bind newdatasource to the HTML screen
    this.data = neosysrecordobject1.data
    /**/ yield * this.bind(this.data, '', 0)

    // alert(neosysobj2xml(gro.revstr))
    //gstepping=true
    //yield* neosysbreak('x')

    //trigger completion event
    this.loading = false
    this.readystate = 'complete'
    if (typeof (this.onreadystatechange) == 'function')
        /**/ yield * this.onreadystatechange()

    //logout('load')

}

//UPDATEROW
///////////
function gds_updaterow(mode, row, copyrow) {

    for (dictid in copyrow) {

        //only copy non-key fields
        var dictitem = this.dictitem(dictid)
        if (dictitem && dictitem.fieldno.toString() != '0') {
            var element = row[dictid].element
            //new data cell
            var cell = row[dictid] = new Object
            //but restore the binding
            cell.element = element
            if (mode != 'copy' || dictitem.copyable != false) {
                cell.text = copyrow[dictid].text
                //if (gro.data.group1[0].BILL.text=='0') xxxx
            }
            else {
                cell.text = ''
            }
        }
    }

    return row

}

//APPENDROW
///////////
function gds_appendrow(mode, group, copyrow) {

    //if mode='copy' then do not copy fields with .copyable==false

    //add a new row
    var row = group[group.length] = this.createrow(group)

    //update the row
    this.updaterow(mode, row, copyrow)

    return row

}

//CREATEROW
///////////
function gds_createrow(group) {
    var newrow = new Object
    var oldrow = group[0]
    for (var propname in oldrow) {
        var cell = newrow[propname] = new Object
        cell.text = ''
    }
    return newrow
}

//ADDROW
////////
function gds_addrow(groupno, newdatarow) {

    var tablex = $$('neosysgroup' + groupno)
    //if (!tablex) return false

    //get the group
    var group = this.data['group' + groupno]

    //append a new row (copy first row)
    if (tablex) {
        var tbody = tablex.tBodies[0]
        var firstrow = tbody.getElementsByTagName('TR')[0]

        var newrow = firstrow.cloneNode(true)
        tbody.insertBefore(newrow, null)

        setupnewrow(newrow)

    }

    //create a new row
    group[group.length] = this.createrow(group)

    //and blank it/bind it
    var datarow = group[group.length - 1]
    if (newdatarow) {
        //not tested/debugged yet
        /**/ yield * this.rebind(datarow, newdatarow)
    }
    else {
        this.blankrowx(datarow, newrow, true)
    }

    var rown = group.length - 1

    return datarow = group[rown]

}

function setupnewrow(newrow) {

    //faster than using the .all collection? TR.all is not available in ff3
    newrow.neosysfields = []
    var elements = newrow.getElementsByTagName('*')
    for (var ii = 0; ii < elements.length; ii++) {
        var id = elements[ii].id
        if (id)
            newrow.neosysfields[id] = elements[ii]
    }

}

//INSERTROW
///////////
function gds_insertrow(groupno, rown) {

    var tablex = $$('neosysgroup' + groupno)
    if (!tablex) return

    //get the group
    var group = this.data['group' + groupno]

    //insert a new row (copy first row)
    var tbody = tablex.tBodies[0]
    var firstrow = tbody.getElementsByTagName('TR')[0]

    var newrow = firstrow.cloneNode(true)
    if (typeof tbody.getElementsByTagName('TR')[rown] != 'undefined')
        tbody.insertBefore(newrow, tbody.getElementsByTagName('TR')[rown])//insert
    else
        tbody.insertBefore(newrow, null)//append

    setupnewrow(newrow)

    //ripple up the following data rows (backwards from last)
    for (var rown2 = group.length - 1; rown2 >= rown; rown2--) {
        group[rown2 + 1] = group[rown2]
    }

    //create a new row
    group[rown] = this.createrow(group)

    //and blank it/bind it
    this.blankrowx(group[rown], newrow, true)

    return group[rown]

}

function gds_blankrow(groupno, rown) {

    //get the group
    var group = this.data['group' + groupno]

    //get the table and row
    var tablex = $$('neosysgroup' + groupno)
    if (!tablex) return false
    var formrow = tablex.tBodies[0].getElementsByTagName('TR')[rown]
    if (!formrow) return false

    //blank it
    this.blankrowx(group[rown], formrow, true)

    return group[rown]

}

//BLANKROWX
///////////
function gds_blankrowx(datarow, formrow, initdata) {

    //initdata=false is only used in bind
    //to clear form and NOT do any binding (actually it unbinds)
    if (!formrow)
        return

    if (!formrow.neosysfields)
        setupnewrow(formrow)

    //blank all the columns found in the data and form rows
    for (var propname in datarow) {

        //blank the data element
        if (initdata) {
            var dataitem = datarow[propname] = new Object
            dataitem.text = ''
            dataitem.oldtext = ''
            dataitem.defaulted = false
        }

        //blank the form element
        try {
            var element = null
            //element=formrow.all[propname]
            element = formrow.neosysfields[propname]

            if (initdata) {
                datarow[propname].element = element
                if (element) element.dataitem = datarow[propname]
            }
            else {
                datarow[propname].element = null
                if (element) element.dataitem = null
            }
        }
        catch (e) {
            continue
        }

        if (element && (element.name == propname || element.id == propname))
            setvalue(element, '')

    }

}

//DELETEROW
///////////
function gds_deleterow(groupno, rown1, rown2) {
    //get the group
    var group = this.data['group' + groupno]

    if (typeof rown2 == 'undefined') rown2 = rown1

    //get the table and row
    var tablex = $$('neosysgroup' + groupno)
    if (!tablex)
        return

    var formrow = tablex.tBodies[0].getElementsByTagName('TR')[rown1]
    if (!formrow)
        return

    //do not delete first and only row, blank it instead (but leaves formatting)
    if (rown1 == 0 && group.length == 1) {
        this.blankrowx(group[rown1], formrow, true)
        return
    }

    //Delete the datasource row

    //zzz?how to avoid deleting all rows?

    //copy nondeleted rows to a new group
    var newgroup = []
    for (var rown = 0; rown < group.length; rown++) {
        if (rown < rown1 || rown > rown2) newgroup[newgroup.length] = group[rown]
    }
    //set the group
    this.data['group' + groupno] = newgroup

    //Delete the form row
    var formrowparentnode = formrow.parentNode
    //remove following rows if any
    for (rown = rown1 + 1; rown <= rown2; rown++) formrowparentnode.removeChild(formrow.nextSibling)
    //remove firstrow
    formrowparentnode.removeChild(formrow)

}


//BIND
//////
function* gds_bind(datasource, elements, rownx) {

    //login('bind')

    for (var propname in datasource) {

        //many rows
        if (propname.slice(0, 5) == 'group') {
            /**/ yield * this.bindgroup(datasource, propname)
        }
        //one row or the header
        else {

            //locate the first element with the same name
            var element
            if (!elements) {
                element = $$(propname)
                if (element && element[0] && element.tagName != 'SELECT')
                    element = element[0]
            }
            else {
                element = elements[propname]
            }
            if (!element)
                continue
            //log('start')
            //for checkbox and radio buttons, bind to the first child
            if (element.length > 0 && (element[0].type == 'radio' || element[0].type == 'checkbox'))
                element = element[0]

            //if the field is used in the form then
            //bind the element to the datasource and set the default
            if (element.id != propname)
                continue

            //    element.name=element.id//fix mac bug?

            //bind the element to the datasource
            //and viceversa
            var dataitem = datasource[propname]
            dataitem.element = element
            element.dataitem = dataitem

            //set as defaulted already if editing an existing record and in initial bind
            if (glocked && !this.isnewrecord && this.loading)
                dataitem.defaulted=true
            else
                dataitem.defaulted=false
            //alert(element.id+' '+dataitem.defaulted)
            
            //default immediately for radio/checkbox and dropdown
            //or if literal default (starts with " or ')
            //do not default immediately for lines to avoid problem described in yield* validateall()
            if (!(Number(element.getAttribute('neosysgroupno'))) && element.getAttribute('neosysdefaultvalue')) {
                var ischeckbox
                if (dataitem.text == ''
                    &&
                    (
                     (ischeckbox = element.type == 'checkbox')
                     ||
                     element.type == 'radio'
                     ||
                     element.tagName == 'SELECT'
                     ||
                     (
                      typeof element.getAttribute('neosysdefaultvalue') == 'string'
                      &&
                      '\'\"'.indexOf(element.getAttribute('neosysdefaultvalue').slice(0, 1)) >= 0
                     )
                    )
                   ) {
                    //if element.getAttribute('neosysdefaultvalue')
                    dataitem.text = yield* getdefault(element)

                    //allow : as separators of checkbox multiple default values
                    if (ischeckbox)
                        dataitem.text = dataitem.text.replace(/:/g, sm)

                    //if (gro.data.group1[0].BILL.text=='0') xxxx
                    dataitem.defaulted = true
                }

                //initialise not defaulted unless editing an existing record
                //already done above to include data in groups
                //and no need to initialise to false
                else {
                    //dataitem.defaulted = (glocked && !this.isnewrecord && this.loading)
                }

            }
            else {
                //1st select option is the data
                if (!dataitem.text && element.tagName == 'SELECT')
                    //should this be element.getElementsByTagName('OPTION')?
                    if (dataitem.text = element.childNodes.length)
                        dataitem.text = element.childNodes[0].value
                }

                //log('started')

                //save the original value
                var value = dataitem.text
                dataitem.oldtext = value

                //put the value into the screen
                //log('value=')

                //conversions
                var conversion = element.getAttribute('neosysconversion')
                if (value != '' && typeof (conversion) == 'string' && conversion.substr(0, 1) == '[') {
                    value2 = value.neosysoconv(conversion)
                    if (value2 == null) {
                        return systemerror('bind()', 'bind("' + element.id + '") ' + neosysquote(value) + ' is not a valid internal ' + conversion.slice(1, -1) + ' value')
                    }
                    value = value2
                }
                //log('converted')

                //update the screen
                //increase speed by not setting values emptyvalues
                if (value || rownx <= 1) setvalue2(element, value)

            }

        } //next propname

        //logout('bind')

    }


    //BINDGROUP
    ///////////
    function* gds_bindgroup(datasource, propname) {
        //alert('bindgroup')
        //locate the table element otherwise it is not required to be bound on screen
        var tablex = $$('neosys' + propname)
        if (!tablex)
            return

        //log('deleting existing rows')
        //delete existing rows except first (backwards from last)
        //var rows=tablex.getElementsByTagName('TBODY')[0].childNodes
        //!!!watch out the above will include any tables included in the header of the table
        var rows = tablex.tBodies[0].getElementsByTagName('TR')
        var rown = 0
        while (rows.length > 1) {
            if (rown > 0 || rows[rown].nodeType != 1)
                rows[rown].parentNode.removeChild(rows[rown])
            else
                ++rown
        }
        //log('deleted existing rows')

        //insert and bind new rows   
        var rows = datasource[propname]
        var tbody = tablex.tBodies[0]

        //blank the first row and get a clone of it
        var firstrow = tbody.getElementsByTagName('TR')[0]
        this.blankrowx(rows[0], firstrow, false)
        var emptyrow = firstrow.cloneNode(true)

        //gtimers=new neosystimers(10)
        //alert('nrows:'+rows.length)
        for (var rown = 0; rown < rows.length; rown++) {
            //alert('rown:'+rown)
            //gtimers.start(1)
            //for second row onwards: append a new row (copy first row)
            var rowx
            if (rown == 0) {
                rowx = firstrow
            }
            else {
                //alert('emptyrow:'+emptyrow)
                var newrow = emptyrow.cloneNode(true)
                //alert('newrow:'+newrow)
                //alert('tbody.insertBefore:'+tbody.insertBefore)
                tbody.insertBefore(newrow, null)
                //alert('inserted')     
                rowx = newrow

            }
            //gtimers.stop(1)

            //gtimers.start(2)
            //setup even the first row to avoid a bug in mac ie5 where column widths are excessive
            setupnewrow(rowx)
            //alert('rows:'+rows)
            //alert('rown:'+rown)
            //alert('rownx:'+rowx)
            //alert('rowx.neosysfields:'+rowx.neosysfields)
            //call bind data recursively to the new row
            //function* gds_bind(datasource,elements,rownx)
            /**/ yield * this.bind(rows[rown], rowx.neosysfields, rown)
            //alert('bound:'+rown)
            //gtimers.stop(3)

        } //rown

        //gtimers.display()

    }


    //REBIND
    ////////
    function* gds_rebind(newdatasource, olddatasource, forced) {

        //will fail a) if newdatasource has more group rows than the olddatasource

        //login('rebind')

        if (!olddatasource) {
            olddatasource = this.data
            for (var groupno = 1; groupno < 100; groupno++) {
                var newrows = newdatasource['group' + groupno]
                var oldrows = olddatasource['group' + groupno]
                if (newrows && oldrows) {
                    for (var rown = 0; newrows[rown]; rown++) {
                        //append a new row if necessary
                        if (!oldrows[rown])
                            this.addrow(groupno)

                        /**/ yield * gds.rebind(newrows[rown], oldrows[rown])

                    }

                    //delete any excess rows
                    //for (var rown=oldrows.length-1;rown>=newrows.length;rown--) this.deleterow(groupno,rown)
                    if (oldrows.length > newrows.length)
                        this.deleterow(groupno, newrows.length, oldrows.length - 1)

                }
            }
        }

        for (var propname in newdatasource) {

            //recursive for groups
            if (propname.slice(0, 5) == 'group') return

            //update any changed values
            var newvalue = newdatasource[propname].text
            var oldcell = olddatasource[propname]
            if (forced || newvalue != oldcell.text) {
                //alert(propname+' '+newvalue+' '+oldcell.text)
                //if (oldcell.element&&oldcell.element.getAttribute('neosysfieldno'))
                // /**/ yield * this.setx2([oldcell], [newvalue])
                var cells = [oldcell]
                cells.dictid = propname//needed to enable afterupdate function
                /**/ yield * this.setx2(cells, [newvalue], forced)
            }
            //initialise "oldtext"
            oldcell.oldtext = newvalue

        } //next propname

        //logout('rebind')

    }
