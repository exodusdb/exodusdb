//Copyright NEOSYS All Rights Reserved.

//designed to have no async code. no function * nor yield *

//Should not be dependent on gds/gds.getx/gds.setx or be related to screen/dom
//There is currently one exception that we use gds.data[].text and timeout to gds.setx to get
// and set "other date" when validating iconv dates since defaulting depends on the other date
// when updating date ranges. perhaps should not be using iconv for such validations
// also perhaps dates ranges should be entered into a single field instead of two.

var gdatedaypos
var gdatemonthpos
var gdateyearpos
var gdateformat
var gfirstdayofweek

//returns an array of unique array elements from an array (removes duplicates)
Array.prototype.exodusunique = function exodusunique(removeempty) {
    var uniquecodes = []
    for (var ii = 0; ii < this.length; ii++) {
        if ((this[ii] == '' && removeempty) || !uniquecodes.exoduslocate(this[ii]))
            uniquecodes[uniquecodes.length] = this[ii]
    }
    return uniquecodes
}

//not used anywhere?
function exodusamountinwords(amount) {

    //if blank return blank
    if (!amount) return ''

    //if zero return 'Zero'
    if (! +amount) return 'Zero'

    amount = amount.toString().exodusconvert('-', '')

    var words = ''

    //millions
    var millions = exodusint(amount / 1000000)

    if (millions) words = exodusamountinwords(millions) + ' Million'

    //thousands
    var thousands = exodusint(exodusmod(amount, 1000000) / 1000)
    if (thousands) {
        if (words) words += ', '
        words += exodusamountinwords(thousands) + ' Thousand'
    }

    //hundreds
    var hundreds = exodusint(exodusmod(amount, 1000) / 100)
    if (hundreds) {
        if (words) words += ', '
        words += exodusamountinwords(hundreds) + ' Hundred'
    }

    //units
    var units = '00' + exodusint(exodusmod(amount, 100))
    var text = ['', 'Zero', 'One', 'Two', 'Three', 'Four', 'Five', 'Six', 'Seven', 'Eight', 'Nine', 'Ten', 'Eleven', 'Twelve', 'Thirteen', 'Fourteen', 'Fifteen', 'Sixteen', 'Seventeen', 'Eighteen', 'Nineteen', 'Twenty', 'Thirty', 'Forty', 'Fifty', 'Sixty', 'Seventy', 'Eighty', 'Ninety'].join(vm)
    if (+units) {
        if (words) words += ' and '
        if (units <= 20) words += text[+units + 1]
        else {
            words += text[19 + (+units.slice(-2, -1))]
            if (units.toString().exodusright(1) != '0') words += '-' + text[(+units.slice(-1)) + 1]
        }
    }

    //subunits
    if (exodusint(amount) != amount) {
        words += ' .' + amount.toString().exodusfield('.', 2)
    }

    return words.exodustrim()

}

function exodusint(arg1) {
    if (typeof arg1 != 'number')
        exodusassertnumeric(arg1, 'int', 'arg1')
    return Math.floor(arg1)
}

function exodusmod(number1, number2) {
    return number1 % number2
}

function exodusjulianperiod(period, maxperiodno) {
    //given a period in format pp/YY or pp/YYYY
    //return the julian period (eg 9/2002 is 2002*maxperiod+9)
    if (!period) return period
    period = period.split('/')
    if (period.length < 2) period[1] = ''
    //if (!maxperiodno) maxperiodno=gcompany.maxperiodno
    if (!maxperiodno) maxperiodno = 12
    return (period[1].exodusaddcent() - 0) * maxperiodno + (period[0] - 0)
}

function exodusrnd(max, min) {
    //return a random integer between 0 (or min) and max-1
    if (!min) min = 0
    max = max - 1
    return (Math.floor(min + Math.random() * (max - min + 1)))
}

function exoduscloneobj(obj) {

    if (typeof obj != 'object') return obj

    var result = []
    for (var propname in obj) {
        var prop = obj[propname]
        if (typeof prop == 'object') prop = exoduscloneobj(prop)
        result[propname] = prop
    }
    return result
}

/*
function badchars(input,badchars) {

//prevent certain characters in key fields
//returns TRUE if bad chars found otherwise false

if (!input) return false

if (!badchars) badchars="'"+'~!@#$%^&*()_+|\\{}[]:";,?'
var input2=input.exodusconvert(badchars)
if (input2!=input) {

return !yield* exodusinvalid('Sorry, you cannot use\n'+input.exodusconvert(input2)+' characters in key fields')
}

return false

}
*/

Number.prototype.exodusdivide0isblank = function (divider) {

    if (!this) return 0

    //coerce divider to numeric else raise an error
    if (typeof divider == 'string') {
        var divider2 = +divider
        if (isNaN(divider2)) {
            return systemerror('exodusdivide0isblank()', 'divider "' + divider + '" is not an number')
            //return divider2
        }
        divider = divider2
    }

    if (!divider) return ''

    return this / divider

}

String.prototype.exoduscapitalise = function exoduscapitalise() {
    //converts BRAND_CODE to Brand code
    var string = this.toString()
    return string.toLowerCase().replace(/\b[a-z]|_[a-z]/g, function (s) { return s.toUpperCase(); });
    //string=string.replace(/_/g,' ').toLowerCase()
    //string=string.slice(0,1).toUpperCase()+string.slice(1)
    //return string
}

function joinarrays(array1, array2) {

    if (typeof array1 == 'object') {
        if (typeof array2 == 'object') {
            var result = []
            var n = (array1.length > array2.length) ? array1.length : array2.length
            for (var i = 0; i < n; i++) {
                var v1 = array1[i]
                if (typeof v1 == 'undefined') v1 = ''
                var v2 = array2[i]
                if (typeof v2 == 'undefined') v2 = ''
                result[i] = v1.toString() + v2.toString()
            }
            return result
        }
        else {
            array2 = [array2.toString()]
            for (var i = 1; i < array1.length; i++) array2[i] = array2[0]
            return joinarrays(array1, array2)
        }
    }
    else {
        if (typeof array2 == 'object') {
            array1 = [array1.toString()]
            for (var i = 1; i < array2.length; i++) array1[i] = array1[0]
            return joinarrays(array1, array2)
        }
        else {
            return array1.toString() + array2.toString()
        }
    }

}

Array.prototype.tostatement = tostatement
function tostatement() {
    var xml = []
    var xmlbit
    loop:
    for (var propname in this) {
        var prop = this[propname]
        switch (typeof (prop)) {
            case 'function': {

                    //xmlbit='<'+propname+'>...</'+propname+'>'
                    continue loop
                }
            case 'object': {

                    //xmlbit=prop.toXML()//recursive
                    xmlbit = prop.tostatement()//recursive
                    break
                }
            default: {

                    //xmlbit='<'+propname+'>'+prop.toString()+'</'+propname+'>'
                    xmlbit = prop.toString()
                }
        }
        xml[xml.length] = xmlbit
    }

    return '[' + xml.join(',') + ']'

}

/////////////////
//ARRAY FUNCTIONS
/////////////////

//function exodusadd()
Array.prototype.exodusadd = function exodusadd(array2) {
    var results = []
    var nn = this.length
    var isarray = typeof array2 == 'object'
    if (isarray && array2.length > nn) nn = array2.length
    for (var ii = 0; ii < nn; ii++) {
        //results[ii]=this[ii]+array2[ii]
        if (typeof this[ii] == 'undefined') this[ii] = 0
        results[ii] = +this[ii] + Number(isarray ? array2[ii] : array2)
    }
    return results
}

//function exodussubtract()
Array.prototype.exodussubtract = function (array2) {
    var results = []
    var nn = this.length
    var isarray = typeof array2 == 'object'
    if (isarray && array2.length > nn) nn = array2.length
    for (var ii = 0; ii < nn; ii++) {
        if (typeof this[ii] == 'undefined') this[ii] = 0
        results[ii] = this[ii] - Number(isarray ? array2[ii] : array2)
    }
    return results
}

//function exodusmultiply()
Array.prototype.exodusmultiply = function (array2) {
    var results = []
    var nn = this.length
    var isarray = typeof array2 == 'object'
    if (isarray && array2.length > nn) nn = array2.length
    for (var ii = 0; ii < nn; ii++) {
        var multiplier = isarray ? array2[ii] : array2
        //should really test for numeric multiplier and in exodusdivide etc
        if (typeof this[ii] == 'undefined') this[ii] = 0
        if (this[ii]) results[ii] = this[ii] * Number(multiplier)
        else results[ii] = this[ii]
    }
    return results
}

//function exodusdivide()
Array.prototype.exodusdivide = arrayexodusdivide
function arrayexodusdivide(array2, divzero) {
    var results = []
    var nn = this.length
    var isarray = typeof array2 == 'object'
    if (isarray && array2.length > nn) nn = array2.length
    for (var ii = 0; ii < nn; ii++) {
        if (typeof this[ii] == 'undefined') this[ii] = 0
        var divisor = (Number(isarray ? array2[ii] : array2))
        if (Number(this[ii])) {
            if (!divisor) {
                if (divzero == 'divideby1') {
                    results[ii] = Number(this[ii])
                }
                else {
                    if (typeof divzero == 'undefined' || divzero == 'error') {
                        systemerror('exodusdivide', this[i] + ' cannot be divided by "' + divisor + '"')
                        results[ii] = ''
                    }
                    else {
                        results[ii] = divzero
                    }
                }
            }
            else {
                results[ii] = this[ii] / divisor
            }
        }
        else {
            results[ii] = this[ii]
        }
    }
    return results
}

Number.prototype.exodussum = function exodussumnumber() { return this }

String.prototype.exodussum = function exodussumstring(sepchar) {

    if (this == '')
        return 0

    //convert multivalued string to array
    if (!sepchar)
        sepchar = vm//this should be ',' really
    if (this.indexOf(sepchar) >= 0)
        return this.split(sepchar).exodussum()

    return exodusnumber(this)

}

Array.prototype.exodussum = function exodussumarray(sepchar) {
    //adds up all array elements including subarrays recursively
    //any "string arrays" (with the given sepchar) are handled as well

    if (!this.length)
        return 0

    //add 'em up (recursively)
    var total = 0
    for (var i = 0; i < this.length; i++)
        total += this[i].exodussum(sepchar)

    //got a small number error of 1.5e-10 once adding up just 11 2 digit decimal amounts
    if (Math.abs(total) < 0.00000001)
        total = 0
    return Number(total.toPrecision(14))+0
    //return (Math.round(total,14)+0)

}

//////////////////
//STRING FUNCTIONS
//////////////////

//convert &amp; to & etc
function decodehtmlcodes(htmlcodes) {
    // htmlcodes=htmlcodes.replace(/\x26amp;/g,String.fromCharCode(38))
    htmlcodes = htmlcodes.replace(/&amp;/g, '&')
    htmlcodes = htmlcodes.replace(/&quot;/g, "'")
    return htmlcodes
}

//convert & to &amp; etc
function encodehtmlcodes(htmlcodes) {
    // htmlcodes=htmlcodes.replace(/\x26amp;/g,String.fromCharCode(38))
    htmlcodes = htmlcodes.replace(/\&/g, '&amp;').replace(/\</g, '&lt;').replace(/\>/g, '&gt;')
    return htmlcodes
}

Array.prototype.exodussplit = function exodussplitarray(seps, invert) {
    var newarray = []
    for (var i = 0; i < this.length; i++) {
        var cell=this[i]
        //if (!cell)
        if (typeof cell == 'undefined')
            systemerror('this['+i+']='+this[i]+' undefined','exodussplit('+this+')')
        if (!cell.exodussplit)
            systemerror('this['+i+']='+this[i]+' missing exodussplit','exodussplit('+this+')')
        newarray[i] = cell.exodussplit(seps, invert)
    }
    return newarray
}

Array.prototype.exodusinvert = function exodusinvert(defaultvalue) {
    if (typeof defaultvalue == 'undefined')
        defaultvalue = ''
    var invertedarray=[]
    var array = this
    //if first element of array has the maximum number of subelements then no need to go back afterwards and fill any missing elements
    var minjj = array[0].length
    var maxjj = minjj
    for (var ii = 0; ii < array.length; ii++) {
        var arrayii = array[ii]
        if (arrayii.length>maxjj)
            maxjj=arrayii.length
        for (var jj = 0; jj < maxjj; jj++) {
            var cell = arrayii[jj]
            if (typeof cell == 'undefined')
                cell = defaultvalue
            if (!invertedarray[jj])
                invertedarray[jj] = []
            invertedarray[jj][ii] = cell
        }
    }
    //go back and fill any missing elements
    if (maxjj>minjj) {
        for (var ii = 0; ii < invertedarray.length; ii++) {
            var arrayii = invertedarray[ii]
            var lenii = arrayii.length
            if (lenii<maxjj) {
                for (var jj = lenii-1; jj < maxjj; jj++) {
                    var cell = arrayii[jj]
                    if (typeof cell == 'undefined')
                        cell = defaultvalue
                    invertedarray[jj][ii] = cell
                }
            }
        }
    }
    return invertedarray
}

String.prototype.exodussplit = function exodussplit(seps, invert) {

    //return exodussplit(this.toString(),seps)

    //exodusassertstring(this,'exodussplit','string')
    var str1 = this.toString()

    if (str1 == '') return []

    // returns array length 1 if passed ''
    //should really return array of 0 but this would break

    //split the fields
    var array = str1.split(seps.slice(0, 1))

    //split the values
    if (seps.length > 1) {
        //seps=seps.slice(1)
        for (var i = 0; i < array.length; i++) {
            array[i] = array[i].exodussplit(seps.slice(1))
        }
    }

    //option to invert the columns and rows (only if two dimensional)
    if (invert && seps.length == 2) {
        /*
        var invertedarray = []
        for (var ii = 0; ii < array.length; ii++) {
            var arrayii = array[ii]
            var maxjj = array[0].length
            for (var jj = 0; jj < maxjj; jj++) {
                var cell = arrayii[jj]
                if (!cell) cell = ''
                if (!invertedarray[jj]) invertedarray[jj] = []
                invertedarray[jj][ii] = cell
            }
        }
        array = invertedarray
        */
        array = array.exodusinvert()
    }

    return array

}

Array.prototype.exodusjoin = function exodusjoin(seps, invert) {

    //this is a multilevel join suited for converting 2d arrays to fm/vm separated strings
    //invert not implemented yet

    if (!this.length) return ''

    if (seps.length <= 1) return this.join(seps)

    var toparray = []
    var otherseps = seps.slice(1)
    for (var i = 0; i < this.length; i++) {
        var typeofthisi = typeof this[i]
        if (typeofthisi == 'string') toparray[i] = this[i]
        else if (typeofthisi == 'number') toparray[i] = this[i].toString()
        else toparray[i] = this[i].exodusjoin(otherseps)
    }

    return toparray.join(seps.slice(0, 1))

}

Array.prototype.exodustrim = function exodustrimarray(trimchar) {
    var newarray = []
    var jj = -1
    for (var ii = 0; ii < this.length; ii++) {
        var cell = this[ii]
        if (cell) {
            cell=cell.exodustrim(trimchar)
            if (cell.length)
                newarray[++jj] = cell
        }
    }
    return newarray
}

Array.prototype.exodustrimr = function exodustrimrarray() {
    var jj = this.length - 1
    for (; jj >= 0; jj--) {
        if (this[jj]) break;
    }
    return this.slice(0, jj + 1)
}

String.prototype.exodustrimf = function exodustrimf(char0) {
    return this.exodustrim(char0, 'L')
}

String.prototype.exodustrimb = function exodustrimb(char0) {
    return this.exodustrim(char0, 'R')
}

String.prototype.exodustriml = function exodustriml(char0) {
    return this.exodustrim(char0, 'L')
}

String.prototype.exodustrimr = function exodustrimr(char0) {
    return this.exodustrim(char0, 'R')
}

String.prototype.exodustrim = function exodustrim(char0, mode) {
    //char0 can be a string list of characters to trim
    
    //mode undefined or A trim all characters
    //mode L (Left)  ... trim leading (leftmost or forward) characters
    //mode R (Right) ... trim trailing (rightmost or backward) characters

    //default trim space and non-breaking spaces and zero width spaces
    if (typeof char0 == 'undefined')
        char0 = ' \xA0\u200B'
        
    if (char0 == '')
        return this.toString()
    temp = this.toString()

    if (typeof mode == 'undefined')
        mode = 'A'

    //escape regexp special characters
    var char1 = char0
    if ('\\^$*+?.()|{}[]'.indexOf(char0) >= 0)
        char1 = '\\' + char1

    //remove leading characters
    if (mode != 'R') {
        var regexp = new RegExp('^[' + char1 + ']*')
        temp = temp.replace(regexp, '')
    }

    //remove trailing characters
    if (mode != 'L') {
        var regexp = new RegExp('['+char1 + ']*$')
        temp = temp.replace(regexp, '')
        //while(temp.slice(-1)==char0) temp=temp.slice(0,-1)
        //while(temp.slice(0,1)==char0) temp=temp.slice(1)
    }

    //remove inner duplicates
    if (mode == 'A') {
        var regexp = new RegExp('['+ char1 + ']+', 'gi')
        temp = temp.replace(regexp, char0.slice(0,1))
    }

    return temp.toString()

}

/*
String.prototype.exodustrimr=exodustrimr

//trim trailing characters
function exodustrimr(char0) {

if (typeof char0=='undefined') char0=' '
if (char0=='') return this.toString()
temp=this.toString()
//while(temp.substr(temp.length-1,1)==char0) temp=temp.substr(0,temp.length-1)
while(temp.slice(-1)==char0) temp=temp.slice(0,-1)
return temp.toString()
}
*/

//return string of N spaces
function exodusspace(length) {
    return exodusstr(' ', length)
}

//return string of N strings
function exodusstr(str1, length) {
    var str2 = ''
    for (var i = 0; i < length; i++) str2 += str1
    return str2
}

//convert anything to string
/*
function ctext(arg1) {

try {

if(typeof(arg)=="string") return(arg1)
return(arg1.toString())
}
catch(e) {

return("")
}
}
*/

Number.prototype.exodusquote = exodusquotemethod
String.prototype.exodusquote = exodusquotemethod
function exodusquotemethod(char0, char1) {
    if (typeof char0 == 'undefined') char0 = '"'
    if (typeof char1 == 'undefined') char1 = char0

    var value = this.toString()

    //do not quote if already quoted
    if (value.slice(0, char0.length) == char0) return value

    //quote
    return char0 + value + char1

}

String.prototype.exodusxmlquote = exodusxmlquotemethod
function exodusxmlquotemethod(char0, char1) {

    return this.toString().replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;').exodusquote(char0, char1)

}

Array.prototype.exodusquote = exodusquotearray
function exodusquotearray(char0, char1) {

    if (typeof char0 == 'undefined') char0 = '"'
    if (typeof char1 == 'undefined') char1 = char0

    //quote the elements of the array
    var results = []
    for (var i = 0; i < this.length; i++) {
        results[i] = ((typeof this[i] == 'undefined') ? 'undefined' : this[i]).exodusquote(char0, char1)
    }

    //return as a single string
    return results.join(' ')

}

//add double quotes
function exodusquote(arg1, arg2) {
    if (!arg2) arg2 = '"'
    return '"' + arg1 + '"'
}

/////////////////
// MATH FUNCTIONS
/////////////////

//round a number to n decimal places
function exodusround(number, ndecimals) {
    //return Math.round(number*Math.pow(10,ndecimals))/Math.pow(10,ndecimals)
    var number2 = Math.round(Math.abs(number) * Math.pow(10, ndecimals)) / Math.pow(10, ndecimals)
    if (number < 0) return -number2
    return number2
}

//convert to number
function exodusnumber(arg1) {

    //must a number if numeric
    if (typeof arg1 == 'numeric') return arg1

    var result = (+arg1)

    //fail if not numeric as defined by exodusnum()
    if (!(exodusnum(arg1) || isNaN(result))) {
        systemerror('exodusnumber()', '"' + arg1 + '" is not numeric')
        result = 0
    }

    return result

    /*
    // exodusassertnumeric(arg1,'exodusnumber','arg1')
    if (arg1=='') return 0
    if (typeof arg1=="number") return arg1
    try {

    return parseFloat(arg1)+0
    }
    catch(e) {

    return 0
    }
    */
}

//check if numeric
//maximum 14 before decimal place
// otherwise numbers resort to E exponent form when converted to strings
//disallow numbers that are too large (or small TODO) to be represented accurately and without exponentials
gnumericregexp = RegExp('(^[-+]?\\d{1,14}[.]?\\d*$)|(^[-+]?\\d*[.]?\\d{1,14}$)|(^$)')//put most likely first
function exodusnum(arg1) {

    if (typeof arg1 == 'string') {
        if (!arg1.length) return true//'' is considered numeric in exodus/pick
    } else if (typeof arg1 == 'number')
        arg1 = arg1.toString()
    else {
        try {
            arg1 = arg1.toString()//strings sometimes present as objects
        }
        catch (e) {
            return false
        }
    }
    //ensure is plain number
    //1. optional +/- prefix
    //2. at least one digit before or after an optional decimal point
    //3. max 14 digits before decimal point otherwise too big to be represented without exponential
    //4. OR no characters at all
    return arg1.match(gnumericregexp) != null

}

function exodusassertobject(object, funcname, varname) {
    if (!object || typeof (object) != 'object') {
        return systemerror(funcname, varname + ' is not an object')
    }
    return true
}

function exodusassertfunctionname(arg1, funcname, varname) {
    try {
        var func = eval(arg1)
        if (typeof (func) == 'function')
            return true
    }
    catch (e) {
    }
    return systemerror(funcname, varname + ' ' + exodusquote(arg1) + ' is not a function name')
}

function exodusassertstring(arg1, funcname, varname) {
    if (typeof (arg1) != 'string')
        return systemerror(funcname, varname + ' ' + exodusquote(arg1) + ' is not a string')
}

function exodusassertnumeric(arg1, funcname, varname) {
    if (!(exodusnum(arg1)))
        return systemerror(funcname, varname + ' ' + exodusquote(arg1) + ' is not numeric')
}

function exodusmod(num, div) {
    var result = num % div
    if (result >= 0)
        return result
    else
        return result + div
}

/////////////////////////
//TIME AND DATE FUNCTIONS
/////////////////////////

function exodusconvarray(functionx, mode, value, params) {
    var result = []
    for (var ii = 0; ii < value.length; ii++) {
        result[i] = functionx(mode, value[ii], params)
        //if any conversion fails return complete failure
        if (result[i] == null)
            return null
    }
    return result
}

function NUMBER(mode, value, params) {

    gmsg=''
    
    //can handle an array of values
    if (typeof value == 'object')
        return exodusconvarray(NUMBER, mode, value, params)

    if (typeof value == 'string') {

        //can handle records, multivalues and subvalues
        if (value.indexOf(fm) + 1)
            return exodusconvarray(NUMBER, mode, value.split(fm), params).join(fm)
        else if (value.indexOf(vm) + 1)
            return exodusconvarray(NUMBER, mode, value.split(vm), params).join(vm)
        else if (value.indexOf(sm) +1)
            return exodusconvarray(NUMBER, mode, value.split(sm), params).join(sm)

    }

    //empty in, empty out
    if (value == '')
        return value

    //accept comma as decimal point - use exceptions for speed since usually string but might not be
    if (mode == 'ICONV') {
        if (gbasefmt.substr(0, 2) != 'MC') {
            try {
                value = value.replace(gthousands_regex,'')
                value = value.replace(/,/gi, '.')
            }
            catch (e) { }
        }
    }

    params = params.split(',')

    //prevent reformatting
    if (params[0] == '')
        params[0] = value.toString().replace(/[^0-9.]/gi, '').exodusfield('.', 2, 1).length.toString()

    //if cannot parseFloat then is deemed not a number
    //value = parseFloat(value)
    //if (isNaN(value)) return null
    if (!(exodusnum(value))) {
        gmsg=value+' cannot be understood as a number'
        return null
    }
    value = exodusnumber(value)

    //format to N decimal places
    if (value !== '') {

        //check to remove trailing zeros
        var nozero = params[0].slice(-1) == 'Z'
        if (nozero) params[0] = params[0].slice(0, -1)

        if (params[0] == 'BASE' && gbasefmt) {
            params[0] = gbasefmt.substr(2, 1)
        }
        if (params[0] == 'BASE') params = ['4']
        //if (params=='NDECS') params=['2']
        if (params[0] == 'NDECS') {
            //params[0]=(typeof gndecs=='undefined')?getrecord('NDECS'):gndecs.toString()
            //params[0] = (typeof gndecs == 'undefined') ? yield* gds.getx('NDECS') : gndecs.toString()
            //params[0] = (typeof gndecs == 'undefined') ? getvalue('NDECS') : gndecs.toString()
            if (typeof gndecs=='undefined')
                params[0]=gds.data['NDECS'].text
            else
                params[0]=gndecs.toString()

        }

        //do formatting
        if (params[0].match(/^\d+$/)) {
            //value=exodusformatnumber(value,params[0])
            var ndecimals = exodusnumber(params[0])
            value = exodusround(value, ndecimals)
            if (ndecimals > 0) {
                var temp = value.toString().split(".")
                if (temp.length == 1)
                    temp[1] = ''
                value = temp.join('.') + '00000000000000000000'.substr(0, ndecimals - temp[1].length)
            }

        }

        //remove training zeros
        if (nozero && value == 0)
            value = ''
        // value=(parseFloat(value)+0).toString()

    }

    //input conversion
    var result
    if (mode == 'ICONV') {

        //prevent negative
        if (value < 0 && params[1] == 'POSITIVE') {
            gmsg = value + ' is negative but must be positive'
            return null
        }

        //minimum
        if (params[1] != '' && exodusnum(params[1]) && value < +params[1]) {
            gmsg = value+' must not be less than ' + params[1]
            return null
        }

        //maximum
        //if (params[2]!=''&&exodusnum(params[2])&&gvalue>+params[2])
        if (params[2] && exodusnum(params[2]) && value > +params[2]) {
            gmsg = value+' must not be more than ' + params[2]
            return null
        }

        //   var result=parseFloat(value)
        result = value.toString()
        if (!(exodusnum(result))) {
            gmsg = value+' number is too large'
            return null
        }

    }

    //output conversion
    else {
        //zzz should format it with params?
        result = value.toString()
    }

    return result

}

var gdatedaypos
var gdatemonthpos
var gdateyearpos

function invaliddatemsg() {

    var tt
    /*
    var year4 = new Date().getFullYear().toString()
    var year2 = year4.substr(2)
    var mth = new Date().getMonth().toString()
    var mth2 = (mth.length == 1) ? ('0' + mth) : mth
    if (gdatedaypos == 0)
    tt = '1/' + mth + '/' + year4 + ' or just 1, 1/' + mth + ', 1/' + mth + '/' + Number(year2) + ', 01' + mth2 + year2 + ', 01' + mth2 + year4 + ' etc.'
    else if (gdatedaypos == 1)
    tt = mth + '/1/' + year4 + ' or just 1, ' + mth + '/1, ' + mth + '/1/' + Number(year2) + ', ' + mth2 + '01' + year2 + ', ' + mth2 + '01' + year4 + ' etc.'
    else
    tt = year4 + '/' + mth + '/1 or just 1, ' + mth + '/1, ' + Number(year2) + '/' + mth + '/1, ' + year2 + mth2 + '01, ' + year4 + mth2 + '01 etc.'
    tt='eg for 1st December ' + year4 + ' use\r' + tt
    */
    tt = 'Valid date formats are\r'
    if (gdatedaypos == 0)
        tt += 'd/m/y, ddmmyy or ddmmyyyy'
    else if (gdatedaypos == 1)
        tt += 'm/d/y, mmddyy or mmddyyyy'
    else
        tt += 'y/m/d, yymmdd or yyyymmdd'
    tt += '\r\ror just '
    if (gdatedaypos == 0)
        tt += 'd, d/m, ddmm'
    else
        tt += 'd, m/d, mmdd'
    tt += '\rfor current month or year'
    tt += '\r\rseparator is / - . space or none'
    return tt
}

//object eg x=new exodusamount('123USD')
var gleadingnumberregex = /(^[-+]?\d{1,14}[.]?\d*)|(^[-+]?\d*[.]?\d{1,14})|(^$)/
function exodusamount(string) {
    if (typeof string=='number')
        string=string.toString()
    exodusassertstring(string, 'exodusamount')
    this.string = string
    //this.amount=string.replace(/([A-Z]*$)/,'')
    //this.amount = string.replace(/([^\+\-\.0123456789]*$)/, '')
    //this.unit = RegExp.$1

    //var result = string.match(gleadingnumberregex)
    //this.amount = string.slice(0, result.lastIndex)
    //this.unit = string.slice(result.lastIndex)
    //lastIndex not working in standard browsers like MSIE so use a more reliable method
    this.unit = string.replace(gleadingnumberregex,'')
    if (this.unit.length)
        this.amount = string.slice(0,-this.unit.length)
    else
        this.amount=this.string
}

function exodusaddunits(a, b) {

    //add a to b
    //a and b can be a mv list or js array of amounts with unit codes eg 200STG','300YEN','100USD
    //b must be ascii alphabetic order

    if (a == '') return b
    if (b == '') return a

    //quick calc and exit if both are plain numeric
    if (exodusnum(a) && exodusnum(b)) return Number(a) + Number(b)

    if (typeof a == 'string') a = a.split(vm)
    else if (typeof a == 'number') a = [a.toString()]
    if (typeof b == 'string') b = b.split(vm)
    else if (typeof b == 'number') b = [b.toString()]

    for (var an = 0; an < a.length; an++) {

        var ax = new exodusamount(a[an])

        var bx = new Object
        bx.amount = ''
        bx.unit = ''

        for (var bn = 0; bn < b.length; bn++) {
            var bx = new exodusamount(b[bn])
            if (bx.unit == ax.unit) {
                if (bx.amount.length || ax.amount.length) {
                    ndecs = ax.amount.exodusfield('.', 2).length
                    bndecs = bx.amount.exodusfield('.', 2).length
                    if (bndecs > ndecs) ndecs = bndecs
                    if (exodusnum(ax.amount) && exodusnum(bx.amount)) b[bn] = (Number(bx.amount) + Number(ax.amount)).exodusoconv('[NUMBER,' + ndecs + ']') + ax.unit
                }
                else {
                    b[bn] = ax.unit
                }
                break
            }
            else if (bx.unit >= ax.unit) break
        } // next bn
        if (bx.unit != ax.unit) b = b.exodusinsert(bn + 1, ax.amount + ax.unit)

    } // next an

    return b.join(vm)

}

//exodustime
function exodustime(mode) {
    //returns the current LOCAL time as exodustime (ms since midnight) an integer
    //or converted to a given external format
    var now=new Date()
    //var now = Number(new Date()) - (new Date()).getTimezoneOffset() * 60000
    var result = now.getHours() * 60 * 60 + now.getMinutes() * 60 + now.getSeconds()

    if (mode) result = result.exodusoconv('[DATE,' + mode + ']')

    return result

}

function exodusdate(year, month, day) {

    //returns todays date (LOCAL timezone date) as an integer 1=1/1/1967
    //or converted to external format with a parameter
    //result=parseInt((new Date())/24/60/60/1000,10)
    //result=parseInt((Number(new Date())-(new Date()).getTimezoneOffset()*60000)/24/60/60/1000,10)
    var newdatel
    if (year && month && day) {
        newdate = new Date(Date.UTC(Number(year), Number(month) - 1, Number(day)))
        var result = parseInt((Number(newdate)) / 24 / 60 / 60 / 1000, 10)
    }
    else {
        newdate = new Date()
        var result = parseInt((Number(newdate) - (new Date()).getTimezoneOffset() * 60000) / 24 / 60 / 60 / 1000, 10)
    }

    //convert to PICK dates which start on 31/12/1967
    result -= parseInt(Date.UTC(1967, 11, 31) / 24 / 60 / 60 / 1000, 10)

    return result

}

String.prototype.exodusiconv = stringexodusiconv
function stringexodusiconv(conversion) {
    return exodusiconv(this.toString(), conversion)
}

function exodusiconv(value, conversion) {
    return exodusconv('ICONV', value, conversion)
}

String.prototype.exodusoconv = stringexodusoconv
Number.prototype.exodusoconv = stringexodusoconv
function stringexodusoconv(conversion) {
    return exodusoconv(this.toString(), conversion)
}

function exodusoconv(value, conversion) {
    return exodusconv('OCONV', value, conversion)
}

Array.prototype.exodusoconv = exodusoconvarray
function exodusoconvarray(conversion) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].toString().exodusoconv(conversion)
    return result
}

Array.prototype.exodusiconv = exodusiconvarray
function exodusiconvarray(conversion) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].toString().exodusiconv(conversion)
    return result
}

function exodusconv(mode, value, conversion) {

    //convert everything including 0 but not ''
    if (conversion === '')
        return value

    if (typeof (conversion) == 'undefined') {
        alert('Conversion for ' + exodusquote(value) + ' is undefined\nNo conversion performed.')
        return value
    }

    //can handle an array of values
    if (typeof value == 'object') {
        var result = []
        for (var i = 0; i < value.length; i++) {
            result[i] = exodusconv(mode, value[i], conversion)

            //if any conversion fails return complete failure
            if (result[i] == null) return null

        }
        return result
    }

    //can handle an array of conversions
    if (typeof (conversion) == 'object') {
        var result = []
        for (var i = 0; i < value.length; i++) {
            result[i] = exodusconv(mode, value, conversion[i])

            //if any conversion fails return complete failure
            if (result[i] == null) return null

        }
        return result
    }

    //function call
    if (conversion.slice(0, 1) == '[' && conversion.slice(-1) == ']') {

        //slice off surrounding [] and split
        var temp = conversion.slice(1, -1).split(',')

        if (temp.length < 2) temp[1] = ''
        temp[1] = temp.slice(1).join(',')
        //VERY strange once off error in the line below where if value was '' not if anything else
        // value.toString().replace(/\\/g,'\\\\')
        // produced a "" string that could have nothing appended to it
        // was repeatable in two debug sessions but disappeared with a reboot
        //UPDATE !!!
        //this appears to be caused when the server return a string containing x00 characters
        //usually caused by a random bug in AREV 2.0 where inserting into a string
        //after on oconv/iconv conversion and garbagecollect workaround not called
        //results in four ascii zero characters
        //value.length is four

        //if not a function then return unconverted (if pass assertion)
        //if (!(exodusassertfunctionname(temp[0],' mode="'+mode+'" conversion="'+temp[0]+'" value="'+value+'" exodusconv','temp[0]')))
        //{
        // return value
        //}
        exodusassertfunctionname(temp[0], ' mode="' + mode + '" conversion="' + temp[0] + '" value="' + value + '" exodusconv', 'temp[0]')

        //var temp2=temp[0]+'('+"'"+mode+"','"+value.toString().replace(/\\/g,'\\\\')+"','"+temp[1]+"')"
        //temp=eval(temp2)
        temp = eval(temp[0])(mode, value, temp[1])

        // alert('mode:'+mode+' value:'+value+' conversion:'+conversion+' result:'+temp)

        return temp

    }

    var matches
    if (conversion.match(/^([RL])\((.)\)\#(\d)+$/)) {
        var align = RegExp.$1
        var padchar = RegExp.$2
        var length = RegExp.$3
        if (padchar == '') padchar = ' '
        switch (align) {
            case 'R': return (exodusstr(padchar, length) + value).slice(-length)
            case 'L': return (value + exodusstr(padchar, length)).slice(0, length)
        }
    }

    systemerror('exodusconv()', exodusquote(conversion) + 'conversion is unknown, value=' + value)

    return value

}

//replace
String.prototype.exodusreplace = function exodusreplace(fieldno, valueno, subvalueno, replacestring) {

    if (subvalueno) {
        if (!fieldno) fieldno = 1
        if (!valueno) valueno = 1
        var value = this.exodusextract(fieldno, valueno)
        replacestring = value.exodusfieldstore(sm, subvalueno, 1, replacestring)
    }

    if (valueno) {
        var field = this.exodusextract(fieldno)
        replacestring = field.exodusfieldstore(vm, valueno, 1, replacestring)
        if (!fieldno) fieldno = 1
    }

    return this.exodusfieldstore(fm, fieldno, 1, replacestring)

}

//fieldstore

String.prototype.exodusfieldstore = exodusfieldstore
function exodusfieldstore(sep, start, length, insertstring, noextend) {

    //standardise sep to one character
    sep = sep.toString().substr(0, 1)

    //if sep is '' then do characterwise insertion
    if (sep == '') {

        //get, extend first part of string
        var temp = this.substr(0, start - 1)
        while (temp.length < start - 1) temp += ' '

        //add insertion
        temp += insertstring

        //add tail
        temp += this.substr(start + length - 1)

        return temp

    }

    //ensure length is set
    if (length == null) length = 1

    var temp = this.split(sep)

    insertstring = insertstring.toString()

    //option to not extend
    if (!insertstring.length && noextend && start > temp.length) return this

    var temp2 = insertstring.split(sep)

    //ensure string is filled
    for (var i = 0; i < start - 1 + length; i++) {
        if (length > temp2.length && i > temp.length - 1) break
        if (typeof temp[i] == 'undefined') temp[i] = ''
    }

    //copy insert into the main string
    for (var i = 0; i < temp2.length; i++) temp[start - 1 + i] = temp2[i]

    //convert array into string
    return temp.join(sep)

}

Array.prototype.exodusfieldstore = exodusfieldstorearray
function exodusfieldstorearray(sep, start, length, insertstring) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].exodusfieldstore(sep, start, length, insertstring)
    return result
}

//field on array returns array of fields
//can work recursively on arrays of arrays
Array.prototype.exodusfield = function exodusfieldarray(sep, start, length) {

    var newarray = []
    for (var i = 0; i < this.length; i++)
        newarray[i] = this[i].toString().exodusfield(sep, start, length)
    return newarray

}

//field on string returns string
String.prototype.exodusfield = function exodusfieldstring(sep, start, length) {

    //nb "typeof this" is "object" even for strings!

    //start is 1 based, zero returns whole string.
    //negative counts back from end of string
    if (start == 0) return this

    //quick return
    if (this == null || this == '') return ''

    //standardise sep and length
    sep = sep.toString()
    if (sep == '') return ''
    if (length == null) length = 1

    start = Number(start)

    var result = ''
    var temp = this.toString().split(sep)
    if (start < 0) start = temp.length + start + 1
    for (var i = start; i <= start + length - 1; i++) {
        if (i > temp.length) break
        if (temp.length >= i) result += sep + temp[i - 1]
    }
    return result.slice(1)
}

//locate

String.prototype.exoduslocate = exoduslocatestring
function exoduslocatestring(value, order, sep, startfrom) {

    //quick return
    if (this == value) return 1

    if (typeof sep == 'undefined') sep = vm

    return this.split(sep).exoduslocate(value, order, startfrom)

}

Array.prototype.exoduslocate = exoduslocatearray
function exoduslocatearray(value, order, startfrom) {

    //returns position (1 based!) in array (0 based!) or zero if not found

    //order null ... exact locate
    //order D ... best position in descending order
    //order A ... best position in ascending order
    //order is alphabetic or numeric depending on string/numeric type of data

    //quick exit
    if (value == this && !startfrom) return 1

    if (!startfrom) startfrom = 1
    startfrom -= 1

    //no order, exact match
    if (!order) {
        for (i = startfrom; i < this.length; i++) {
            if (this[i] == value) return (i + 1)
        }
        return 0
    }

    //descending
    if (order.substr(0, 1) == 'D') {
        for (i = this.length - 1; i >= startfrom; i--) {
            if (this[i] >= value) {
                if (this[i] == value) return i + 1
                return (i + 2)
            }
        }
        return 1
    }

    //ascending
    if (order.substr(0, 1) == 'A') {
        for (i = startfrom; i < this.length; i++) {
            if (this[i] >= value) return (i + 1)
        }
        return (this.length + 1)
    }

}

//extract

String.prototype.exodusextract = exodusextractstring
function exodusextractstring(fieldn, valuen, subvaluen) {
    return this.split(fm).exodusextract(fieldn, valuen, subvaluen)
}

Array.prototype.exodusextract = exodusextractarray
function exodusextractarray(fieldn, valuen, subvaluen) {

    //extracts the n'th field (1 based!) or '' if not defined
    //if fieldn is undefined then returns input as output
    //if fieldn is 0 then returns an array (string split by fm if necessary)
    //and so on for the values and subvalues

    var field = this
    if (typeof fieldn == 'undefined') return field

    // if (typeof field=='string') field=field.split(fm)
    if (fieldn == 0) return field

    //allow that the array may not be zero indexed
    if (!field.base) fieldn -= 1

    field = field[fieldn]
    if (typeof field == 'undefined') return ''
    if (typeof valuen == 'undefined') return field

    if (typeof field == 'string') field = field.split(vm)
    if (valuen == 0) return field
    field = field[valuen - 1]
    if (typeof field == 'undefined') return ''
    if (typeof subvaluen == 'undefined') return field

    if (typeof field == 'string') field = field.split(sm)
    if (subvaluen == 0) return field
    field = field[subvaluen - 1]
    if (typeof field == 'undefined') return ''
    return field

}

//delete (index 1 based!)
Array.prototype.exodusdelete = exodusdeletearray
function exodusdeletearray(index) {
    var newarray = this.slice(0, index - 1)
    var remainder = this.slice(index)
    for (var i = 0; i < remainder.length; i++) newarray[newarray.length] = remainder[i]
    return newarray
}

//insert (1 based!)
Array.prototype.exodusinsert = exodusinsertarray
function exodusinsertarray(index, values) {

    if (typeof values != 'object') values = [values]

    var newarray = this.slice(0, index - 1)

    for (var i = 0; i < values.length; i++) newarray[newarray.length] = values[i]

    var remainder = this.slice(index - 1)
    for (var i = 0; i < remainder.length; i++) newarray[newarray.length] = remainder[i]

    return newarray

}

/* not used anywhere?
//locate
function exoduslocate(value,values) {

if (values==null) return false
if (typeof(values)!='object'&&typeof(values)!='string') return value==values
return values.exoduslocate(value)
}
*/

//plural

//given a number!=1 or boolean=true return the string with (s) appended (or a given suffix)
String.prototype.exodusplural = function exodusplural(n, suffix) {
    return this + (((typeof n == 'boolean' && !n) || n == 1) ? '' : (suffix ? suffix : 's'))
}

String.prototype.exodussingular = exodussingular
function exodussingular() {
    temp = this.toString()

    if (temp.exodusright(2) == 'ES') {

        //COMPANIES=COMPANY
        //ADDRESSES=ADDRESS
        if (temp.exodusright(3) == 'IES') temp = temp.slice(0, -3) + 'Y'
        else if (temp.exodusright(4) == 'SSES') temp = temp.slice(0, -2)
        else if (temp.exodusright(4) == 'SHES') temp = temp.slice(0, -2)
        else if (temp.exodusright(4) == 'CHES') temp = temp.slice(0, -2)
        else temp = temp.slice(0, -1)

    }
    else {

        if (temp.exodusright(1) == 'S') {
            //ANALYSIS, DOS
            if (temp.exodusright(2) != 'IS' && temp.exodusright(2) != 'OS') temp = temp.slice(0, -1)
        }

    }

    return temp

}

//right

String.prototype.exodusright = exodusstringright
function exodusstringright(len) {
    var str = this.toString()
    return str.slice(str.length - len)
}

function no(value) {
    return !is(value)
}

function isnt(value) {
    return !is(value)
}

function is(value) {
    if (value == null) return (0)

    switch (typeof (value)) {

        case "undefined": {

                return (0)
            }

        case "string": {

                if (value.length == 0) return (0)
                return (value != 0)
            }

        case "number": {

                return (value != 0)
            }

        case "boolean": {

                return (value)
            }

        case "object": {

                //not an array
                if (typeof (value.length) == 'undefined') return 1

                //array
                if (value.length == 0) return (0)

                for (var i = 0; i < value.length; i++) {
                    if (is(value[i])) return (1)
                }
                return (0)
            }

        default:
            return (0)

    }
}

//swap
Array.prototype.exodusswap = function exodusswaparray(swapfrom, swapto) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].exodusswap(swapfrom, swapto)
    return result
}

String.prototype.exodusswap = function exodusswapstring(swapfrom, swapto) {
    //should this be made case sensitive or an option added?

    if (!swapfrom) return this

    //make sure all regexpression characters are escaped
    //if (swapfrom) swapfrom='\\'+swapfrom.split('').join('\\')
    swapfrom = swapfrom.replace(/([\\,\^,\$,\*,\+,\?,\.,\(,\),\|,\{,\},\[,\]])/g, "\\$1")

    swapfrom = new RegExp(swapfrom, 'gi')
    return this.replace(swapfrom, swapto)

}

//convert
Array.prototype.exodusconvert = function exodusconvertarray(convertfrom, convertto) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].exodusconvert(convertfrom, convertto)
    return result
}

String.prototype.exodusconvert = function exodusconvertstring(str1, str2) {

    if (!str2) str2 = ''

    str1 = str1.toString()
    str2 = str2.toString()
    var result = this.toString()
    for (var i = 0; i < str1.length; i++) {
        var index = 0
        var char1 = str1.substr(i, 1)
        var char2 = str2.substr(i, 1)

        //escape regexp special characters
        if ('\\^$*+?.()|{}[]'.indexOf(char1) >= 0) char1 = '\\' + char1

        result = result.replace(new RegExp(char1, 'gi'), char2)
    }
    return result
}

Array.prototype.exodusdcount = function exodusdcountarray(str2) {
    var result = []
    for (var ii = 0; ii < this.length; ii++) {
        result[ii] = this[ii].exodusdcount(str2)
    }
    return result
}

String.prototype.exodusdcount = function exodusdcountstring(str2) {
    return this.exoduscount(str2) + (this.length ? 1 : 0)
}

Array.prototype.exoduscount = function exoduscountarray(str2) {
    var result = []
    for (var ii = 0; ii < this.length; ii++) {
        result[ii] = this[ii].exoduscount(str2)
    }
    return result
}

String.prototype.exoduscount = function exoduscount(str2) {
    var str1 = this.toString()
    if (typeof (str1) == 'undefined' || typeof (str2) == '') return 0
    if (str1 == '' || str2 == '') return 0
    str1 = str1.toString()
    str2 = str2.toString()
    var count = 0
    var index = -1
    while ((index = str1.indexOf(str2, index + 1)) >= 0) count++
    return count
}

String.prototype.exodusaddcent = exodusaddcentmethod
Number.prototype.exodusaddcent = exodusaddcentmethod
function exodusaddcentmethod(type) {
    if (!type) type = 'OCONV'
    var result = ADDCENT(type, this.toString())
    return result
}

//no longer handles conversion of period conversions YYMM <-> YYYYMM
//since 2001 could be YYMM or YYYY
function ADDCENT(type, input, mode) {

    //00-89 <-> 2000-2089
    //90-99 <-> 1990-1999
    var centuryyear = 90

    //cater for direct call (not via oconv)
    if (type != 'OCONV' && type != 'ICONV') {
        input = type
        type = 'OCONV'
    }

    //NINO nothing in nothing out
    if (input == '' || input == null) return input

    //make sure that it is a string
    input = input.toString()

    //convert -1 -2 to 1999 1998 etc
    if (parseFloat(input) < 0) input = (parseFloat(input) + 2000).toString()

    //standardise the separators
    input = input.exodusconvert('-/', '  ')

    //two numbers means 99/99
    if (input.exoduscount(' ') == 1) input = input.exodusconvert(' ', '/')

    //four numbers means 99/99-99/99
    if (input.exoduscount(' ') == 3) {
        input = input.split(' ')
        input = input[0] + '/' + input[1] + '-' + input[2] + '/' + input[3]
    }

    var out = ''

    //range of periods 1/98-3/98
    if (input.indexOf('-') >= 0) {
        var out1 = ADDCENT(type, input.exodusfield('-', 1), mode)
        if (out1 == null) return null
        var out2 = ADDCENT(type, input.exodusfield('-', 2), mode)
        if (out2 == null) return null
        return out1 + '-' + out2
    }

    //remove any prefixed period number '1/99'
    var prefix = ''
    if (input.indexOf('/') >= 0) {
        prefix = input.exodusfield('/', 1) + '/'
        input = input.exodusfield('/', 2)
    }

    //if not all digits then error
    if (!input.match(/^\d+$/)) {
        gmsg = input + ' is not numeric'
        return null
    }

    //ensure at least two digits
    if (input.length == 1) input = '0' + input

    input2 = input.substr(0, 2)

    if (input.length < 4) {
        out = ((parseInt(input2, 10) < centuryyear) ? '20' : '19') + input
    }
    else {
        out = input
    }

    if (type == 'ICONV') out = out.substr(2, 2)

    return prefix + out

}

function CENTURY(type, input, mode) {

    var centuryyear = 90

    //cater for direct call (not via oconv)
    if (type != 'OCONV' && type != 'ICONV') {
        input = type
        type = 'OCONV'
    }

    //NINO nothing in nothing out
    if (input == '' || input == null) return input

    //make sure that it is a string
    input = input.toString()

    //convert -1 -2 to 1999 1998 etc
    if (parseFloat(input) < 0) input = (parseFloat(input) + 2000).toString()

    var out = input

    //if not all digits then error
    if (!input.match(/^\d+$/)) {
        gmsg = input + ' is not numeric'
        return null
    }

    //ensure at least two digits
    if (input.length == 1) input = '0' + input

    input2 = input.substr(0, 2)

    if (input.length < 4) {
        out = ((parseInt(input2, 10) <= centuryyear) ? '20' : '19') + input
    }

    if (type == 'ICONV' && mode == '2') out = out.substr(2, 2)

    return out

}

//similar code YEAR_PERIOD and YEARPERIOD
function PERIOD_OF_YEAR(type, input, mode) {

    maxmth = 12
    gmsg = 'e.g. month "1" or month/year "1/02" or "1/2002"\r'

    input = input.toString()
    if (input == '') return input

    input = input.split('/')

    //check in format 99 or 99/99
    if (input.length > 2) return null

    //default to current year
    if (input.length == 1 || input[1] == '') {
        if (mode) {
            //mode can be the current period eg 7/02
            mode = mode.split('/')

            //default to the mode year
            input[1] = mode[1]

            //if the entered month is after the mode month then default to the previous year
            if (input[0].exodusaddcent() > mode[0].exodusaddcent()) {
                input[1] = (input[1] - 1).toString()
            }
        }
        else {
            input[1] = new Date().getFullYear()
        }
    }

    var month = input[0]
    var year = input[1]

    //trim leading zero
    if (month.substr(0, 1) == '0') month = month.substr(1)

    //determine maxmonth number
    //T=period[1,'/']
    //if (year<1||month>journal.maxperiod||!(T MATCHES '0N'))
    //{
    // gmsg='PLEASE ENTER A PERIOD, OR PERIOD/YEAR|EG. '+exodusquote(gcompany.currperiod)+' or '+exodusquote(gcompany.currperiodyear)
    // return null
    //}

    //check month
    var mths = []
    for (var i = 1; i <= maxmth; i++) { mths[i - 1] = i.toString() }
    if (!mths.exoduslocate(month)) return null

    //check year ... ICONV converts to 2 digits, OCONV converts to 4 digits
    year = ADDCENT(type, year)
    if (year == null) return null

    return month + '/' + year

}

//similar code YEAR_PERIOD and YEARPERIOD
function YEAR_PERIOD(type, input, mode) {

    //same as YEARPERIOD but 09.12 <-> 12/09
    //or if mode is 4 then 2009.12 <-> 12/2009

    //can handle an array of values but what about strings that appear as objects
    if (typeof input == 'object')
        return exodusconvarray(YEAR_PERIOD, type, input, mode)

    input = input.toString()
    if (input == '')
        return input

    //iconv
    if (type == 'ICONV') {

        input = input.split('/')

        //check in format 99 or 99/99
        if (input.length > 2)
            return null

        //default to current year
        if (input.length == 1 || input[1] == '')
            input[1] = new Date().getFullYear()

        var month = input[0]
        var year = input[1]

        //trim leading zero
        if (month.substr(0, 1) == '0')
            month = month.substr(1)

        //check month
        if (!['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12'].exoduslocate(month))
            return null

        //check year ... ICONV converts to 2 digits, OCONV converts to 4 digits
        if (mode == '4')
            year = ADDCENT('OCONV', year, mode)
        else
            year = ADDCENT(type, year, mode)
        if (year == null)
            return null

        return year + '.' + ('00' + month).exodusright(2)

    }

    //oconv

    input = input.split('.')
    return (parseInt(input[1], 10) + 0) + '/' + ADDCENT(type, input[0], '')

}

//similar code YEAR_PERIOD and YEARPERIOD
function YEARPERIOD(type, input, mode) {

    //same as YEARPERIOD but 0912 <-> 12/09

    //can handle an array of values
    if (typeof input == 'object')
        return exodusconvarray(YEARPERIOD, type, input, mode)

    input = input.toString()
    if (input == '') return input

    //iconv
    if (type == 'ICONV') {

        input = input.split('/')

        //check in format 99 or 99/99
        if (input.length > 2) return null

        //default to current year
        if (input.length == 1) input[1] = new Date().getFullYear()

        var month = input[0]
        var year = input[1]

        //trim leading zero
        if (month.substr(0, 1) == '0') month = month.substr(1)

        //check month
        if (!['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12'].exoduslocate(month)) {
            return null
        }

        //check year ... ICONV converts to 2 digits, OCONV converts to 4 digits
        year = ADDCENT(type, year, mode)
        if (year == null) return null

        //return year+'.'+('00'+month).exodusright(2)
        return year + ('00' + month).exodusright(2)

    }

    //oconv

    //input=input
    return (parseInt(input.slice(2), 10) + 0) + '/' + ADDCENT(type, input.slice(0, 2), '')

}

function exodustimers(ntimers) {
    this.timers = []
    for (var i = 0; i < ntimers; i++) {
        this.timers[i] = new exodustimer()
    }

    this.timers[0].start()

    this.start = function (i) { this.timers[i].start() }
    this.stop = function (i) { this.timers[i].stop() }

    this.display = function () {

        this.timers[0].stop()
        var msg = []
        for (var i = 0; i < this.timers.length; i++) {
            msg[i] = i + '=' + (this.timers[i].elapsedtime() / 10)
        }
        alert(msg.join('\n'))
    }

}

function exodustimer() {
    this.time = 0
    this.timestarted = new Date()

    this.start = function () {
        return this.timestarted = new Date()
    }

    this.stop = function () {
        return this.time += new Date() - this.timestarted
    }

    this.elapsedtime = function () {
        return this.time
    }

}

//ensure hashtags go on line breaks
//hashtag is a (space followed by a # followed by NOT a space)
//eg ..... #XXX will be broken onto a new line
function exodus_HASHTAGS(type, input, mode) {

    //can handle an array of values but what about strings that appear as objects
    if (typeof input == 'object')
        return exodusconvarray(exodus_HASHTAGS, type, input, mode)

    input = input.toString()
    if (input == '')
        return input

    //BE CAREFUL OF GETVALUE/SETVALUE on SPANS MESSING WITH \r\n ALSO

    //iconv
    //remove line breaks
    if (type == 'ICONV') {
//        return input.replace(/\r#([^ ])/g,' #$1')
        var internalx=input.replace(/\r\n#([^ ])/g,' #$1').replace(/\n#([^ ])/g,' #$1').replace(/\r#([^ ])/g,' #$1')
//alert(internalx.replace(/\r/g,'\\r').replace(/\n/g,'\\n') + ' <hashtag< ' + input.replace(/\r/g,'\\r').replace(/\n/g,'\\n'))
        return internalx
    }

    //oconv
    //insert line breaks
    //var external = input.replace(/ #/g,'\r#')
    var external = input.replace(/ #([^ ])/g,'\n#$1')
//alert(input.replace(/\r/g,'\\r').replace(/\n/g,'\\n') + ' >hashtag> ' + external.replace(/\r/g,'\\r').replace(/\n/g,'\\n'))
    return external

}
