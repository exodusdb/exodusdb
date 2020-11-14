//Copyright NEOSYS All Rights Reserved.

//moved from general.js

function* system_dict_datasetcode(di, many, orcurrent, test) {
    many = many || false
    orcurrent = orcurrent || false
    //test=test||false
    //"" means no restriction, true=only TEST databases, false=not TEST databases
    if (typeof test == 'undefined')
        test = '""'
    di.popup = 'yield* system_pop_datasetcode(' + many + ',' + orcurrent + ')'
    di.validation = 'yield* system_val_datasetcode(' + many + ',' + orcurrent + ',' + test + ')'
    di.length = 8;
}

function* system_pop_datasetcode(many, orcurrent) {
    if (!(yield* system_getdatasets())) return false

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
    var datasetcode = yield* exodusdecide('Which dataset' + (many ? '(s' : '') + ' do you want?' + tt, popdata, cols, returncoln, defaultreply, many, inverted)
    return datasetcode
}

function* system_val_datasetcode(many, orcurrent, test) {

    if (gvalue == 'CURRENT') {
        if (!orcurrent) return yield* exodusinvalid('"CURRENT" is not allowed here')
        return true
    }

    if (!(yield* system_getdatasets()))
        return false

    if (gvalue != 'CURRENT' && gvalue.indexOf('CURRENT') >= 0)
        return yield* exodusinvalid('You cannot choose CURRENT and other datasets')

    if (typeof test == 'boolean') {
        if (test && gvalue && gvalue.substr(-4) != 'TEST')
            return yield* exodusinvalid('You can only choose TEST databases here')
        else if (!test && gvalue.substr(-4) == 'TEST')
            return yield* exodusinvalid('You cannot choose TEST databases here')
    }

    var values = many ? gvalue.split(':') : [gvalue]
    for (var ii = 0; ii < values.length; ii++) {
        if (!gdatasets[1].exoduslocate(values[ii]))
            return yield* exodusinvalid(values[ii].exodusquote() + ' is not a valid dataset code')
    }

    return true

}

var gdatasets
function* system_getdatasets(refresh) {
    if (refresh || !gdatasets) {
        db.request = 'EXECUTE\rGENERAL\rGETDATASETS'
        if (!(yield* db.send())) return yield* exodusinvalid(db.response)
        //split inverted
        gdatasets = db.data.exodussplit(vm + sm, true)
    }
    return gdatasets.length > 0
}

