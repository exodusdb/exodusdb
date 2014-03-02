//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

var gdatedaypos
var gdatemonthpos
var gdateyearpos
var gdateformat
var gfirstdayofweek

//returns an array of unique array elements from an array (removes duplicates)
Array.prototype.neosysunique = function neosysunique(removeempty) {
    var uniquecodes = []
    for (var ii = 0; ii < this.length; ii++) {
        if ((this[ii] == '' && removeempty) || !uniquecodes.neosyslocate(this[ii]))
            uniquecodes[uniquecodes.length] = this[ii]
    }
    return uniquecodes
}

//not used anywhere?
function neosysamountinwords(amount) {

    //if blank return blank
    if (!amount) return ''

    //if zero return 'Zero'
    if (! +amount) return 'Zero'

    amount = amount.toString().neosysconvert('-', '')

    var words = ''

    //millions
    var millions = neosysint(amount / 1000000)

    if (millions) words = neosysamountinwords(millions) + ' Million'

    //thousands
    var thousands = neosysint(neosysmod(amount, 1000000) / 1000)
    if (thousands) {
        if (words) words += ', '
        words += neosysamountinwords(thousands) + ' Thousand'
    }

    //hundreds
    var hundreds = neosysint(neosysmod(amount, 1000) / 100)
    if (hundreds) {
        if (words) words += ', '
        words += neosysamountinwords(hundreds) + ' Hundred'
    }

    //units
    var units = '00' + neosysint(neosysmod(amount, 100))
    var text = ['', 'Zero', 'One', 'Two', 'Three', 'Four', 'Five', 'Six', 'Seven', 'Eight', 'Nine', 'Ten', 'Eleven', 'Twelve', 'Thirteen', 'Fourteen', 'Fifteen', 'Sixteen', 'Seventeen', 'Eighteen', 'Nineteen', 'Twenty', 'Thirty', 'Forty', 'Fifty', 'Sixty', 'Seventy', 'Eighty', 'Ninety'].join(vm)
    if (+units) {
        if (words) words += ' and '
        if (units <= 20) words += text[+units + 1]
        else {
            words += text[19 + (+units.slice(-2, -1))]
            if (units.toString().neosysright(1) != '0') words += '-' + text[(+units.slice(-1)) + 1]
        }
    }

    //subunits
    if (neosysint(amount) != amount) {
        words += ' .' + amount.toString().neosysfield('.', 2)
    }

    return words.neosystrim()

}

function neosysint(arg1) {
    if (typeof arg1 != 'number')
        neosysassertnumeric(arg1, 'int', 'arg1')
    return Math.floor(arg1)
}

function neosysmod(number1, number2) {
    return number1 % number2
}

function neosysjulianperiod(period, maxperiodno) {
    //given a period in format pp/YY or pp/YYYY
    //return the julian period (eg 9/2002 is 2002*maxperiod+9)
    if (!period) return period
    period = period.split('/')
    if (period.length < 2) period[1] = ''
    //if (!maxperiodno) maxperiodno=gcompany.maxperiodno
    if (!maxperiodno) maxperiodno = 12
    return (period[1].neosysaddcent() - 0) * maxperiodno + (period[0] - 0)
}

function neosysrnd(max, min) {
    //return a random integer between 0 (or min) and max-1
    if (!min) min = 0
    max = max - 1
    return (Math.floor(min + Math.random() * (max - min + 1)))
}

function neosyscloneobj(obj) {

    if (typeof obj != 'object') return obj

    var result = []
    for (var propname in obj) {
        var prop = obj[propname]
        if (typeof prop == 'object') prop = neosyscloneobj(prop)
        result[propname] = prop
    }
    return result
}

/*
function badchars(input,badchars)
{

//prevent certain characters in key fields
//returns TRUE if bad chars found otherwise false
 
if (!input) return false

if (!badchars) badchars="'"+'~!@#$%^&*()_+|\\{}[]:";,?' 
var input2=input.neosysconvert(badchars)
if (input2!=input)
{
return !neosysinvalid('Sorry, you cannot use\n'+input.neosysconvert(input2)+' characters in key fields')
}

return false
 
}
*/

Number.prototype.neosysdivide0isblank = function (divider) {

    if (!this) return 0

    //coerce divider to numeric else raise an error
    if (typeof divider == 'string') {
        var divider2 = +divider
        if (isNaN(divider2)) {
            raiseerror(1000, 'In neosysdivide0isblank, divider "' + divider + '" is not an number')
            return divider2
        }
        divider = divider2
    }

    if (!divider) return ''

    return this / divider

}

String.prototype.neosyscapitalise = function neosyscapitalise() {
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
            result = []
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
            case 'function':
                {
                    //xmlbit='<'+propname+'>...</'+propname+'>'
                    continue loop
                }
            case 'object':
                {
                    //xmlbit=prop.toXML()//recursive
                    xmlbit = prop.tostatement()//recursive
                    break
                }
            default:
                {
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

//function neosysadd()
Array.prototype.neosysadd = function neosysadd(array2) {
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

//function neosyssubtract() 
Array.prototype.neosyssubtract = function (array2) {
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

//function neosysmultiply()
Array.prototype.neosysmultiply = function (array2) {
    var results = []
    var nn = this.length
    var isarray = typeof array2 == 'object'
    if (isarray && array2.length > nn) nn = array2.length
    for (var ii = 0; ii < nn; ii++) {
        var multiplier = isarray ? array2[ii] : array2
        //should really test for numeric multiplier and in neosysdivide etc
        if (typeof this[ii] == 'undefined') this[ii] = 0
        if (this[ii]) results[ii] = this[ii] * Number(multiplier)
        else results[ii] = this[ii]
    }
    return results
}

//function neosysdivide()
Array.prototype.neosysdivide = arrayneosysdivide
function arrayneosysdivide(array2, divzero) {
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
                        systemerror('neosysdivide', this[i] + ' cannot be divided by "' + divisor + '"')
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

Number.prototype.neosyssum = function neosyssumnumber() { return this }

String.prototype.neosyssum = function neosyssumstring(sepchar) {

    if (this == '')
        return 0

    //convert multivalued string to array
    if (!sepchar)
        sepchar = vm//this should be ',' really
    if (this.indexOf(sepchar) >= 0)
        return this.split(sepchar).neosyssum()

    return neosysnumber(this)

}

Array.prototype.neosyssum = function neosyssumarray(sepchar) {
    //adds up all array elements including subarrays recursively
    //any "string arrays" (with the given sepchar) are handled as well

    if (!this.length)
        return 0

    //add 'em up (recursively)
    var total = 0
    for (var i = 0; i < this.length; i++)
        total += this[i].neosyssum(sepchar)

    //got a small number error of 1.5e-10 once adding up just 11 2 digit decimal amounts
    if (Math.abs(total) < 0.00000001)
        total = 0

    return total

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

Array.prototype.neosyssplit = function neosyssplitarray(seps, invert) {
    var newarray = []
    for (var i = 0; i < this.length; i++) {
        newarray[i] = this[i].neosyssplit(seps, invert)
    }
    return newarray
}

String.prototype.neosyssplit = function neosyssplit(seps, invert) {

    //return neosyssplit(this.toString(),seps)

    //neosysassertstring(this,'neosyssplit','string')
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
            array[i] = array[i].neosyssplit(seps.slice(1))
        }
    }

    //option to invert the columns and rows
    if (invert && seps.length == 2) {

        var invertedarray = []
        /* 
        for (var i=0;i<array[0].length;i++)
        {
        for (var j=0;j<array.length;j++) invertedarray[j]=[]
        for (var j=0;j<array.length;j++)
        {
        var cell=array[i]
        if (cell)cell=cell[j]
        if (!cell) cell=''
        invertedarray[j][i]=cell
        }
        }
        */
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
    }

    return array

}

/* obsolete?
Array.prototype.neosysjoin=function neosysjoinarray!!!(seps,invert)
{
var newarray=[]
for (var i=0;i<this.length;i++)
{
newarray[i]=this[i].neosysjoin(seps,invert)
}
return newarray
}
*/

Array.prototype.neosysjoin = function neosysjoin(seps, invert) {

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
        else toparray[i] = this[i].neosysjoin(otherseps)
    }

    return toparray.join(seps.slice(0, 1))

}

Array.prototype.neosystrim = function neosystrimarray() {
    var newarray = []
    var jj = -1
    for (var ii = 0; ii < this.length; ii++) {
        var cell = this[ii].neosystrim()
        if (cell.length) newarray[++jj] = cell
    }
    return newarray
}

Array.prototype.neosystrimr = function neosystrimrarray() {
    var jj = this.length - 1
    for (; jj >= 0; jj--) {
        if (this[jj]) break;
    }
    return this.slice(0, jj + 1)
}

String.prototype.neosystrimf = function neosystrimf(char0) {
    return this.neosystrim(char0, 'L')
}

String.prototype.neosystrimb = function neosystrimb(char0) {
    return this.neosystrim(char0, 'R')
}

String.prototype.neosystriml = function neosystriml(char0) {
    return this.neosystrim(char0, 'L')
}

String.prototype.neosystrimr = function neosystrimr(char0) {
    return this.neosystrim(char0, 'R')
}

String.prototype.neosystrim = function neosystrim(char0, mode) {

    //mode undefined or A trim all characters
    //mode L ... trim leading (leftmost or forward) characters
    //mode R ... trim trailing (rightmost or backward) characters

    if (typeof char0 == 'undefined') char0 = ' '
    if (char0 == '') return this.toString()
    temp = this.toString()

    if (typeof mode == 'undefined') mode = 'A'

    //escape regexp special characters
    var char1 = char0
    if ('\\^$*+?.()|{}[]'.indexOf(char0) >= 0) char1 = '\\' + char1

    //remove duplicates
    if (mode == 'A') {
        var regexp = new RegExp(char1 + '+', 'gi')
        temp = temp.replace(regexp, char0)
    }

    //remove leading characters
    if (mode != 'R') {
        var regexp = new RegExp('^' + char1 + '*')
        temp = temp.replace(regexp, '')
    }

    //remove trailing characters
    if (mode != 'L') {
        var regexp = new RegExp(char1 + '*$')
        temp = temp.replace(regexp, '')
        //while(temp.slice(-1)==char0) temp=temp.slice(0,-1)
        //while(temp.slice(0,1)==char0) temp=temp.slice(1)
    }

    return temp.toString()

}

/*
String.prototype.neosystrimr=neosystrimr

//trim trailing characters
function neosystrimr(char0)
{
if (typeof char0=='undefined') char0=' '
if (char0=='') return this.toString()
temp=this.toString()
//while(temp.substr(temp.length-1,1)==char0) temp=temp.substr(0,temp.length-1)
while(temp.slice(-1)==char0) temp=temp.slice(0,-1)
return temp.toString()
}
*/

//return string of N spaces
function neosysspace(length) {
    return neosysstr(' ', length)
}

//return string of N strings
function neosysstr(str1, length) {
    var str2 = ''
    for (var i = 0; i < length; i++) str2 += str1
    return str2
}

//convert anything to string
/*
function ctext(arg1)
{
try
{
if(typeof(arg)=="string") return(arg1)
return(arg1.toString())
}
catch(e)
{
return("")
}
}
*/

Number.prototype.neosysquote = neosysquotemethod
String.prototype.neosysquote = neosysquotemethod
function neosysquotemethod(char0, char1) {
    if (typeof char0 == 'undefined') char0 = '"'
    if (typeof char1 == 'undefined') char1 = char0

    var value = this.toString()

    //do not quote if already quoted
    if (value.slice(0, char0.length) == char0) return value

    //quote
    return char0 + value + char1

}

String.prototype.neosysxmlquote = neosysxmlquotemethod
function neosysxmlquotemethod(char0, char1) {

    return this.toString().replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;').replace(/>/g, '&gt;').neosysquote(char0, char1)

}

Array.prototype.neosysquote = neosysquotearray
function neosysquotearray(char0, char1) {

    if (typeof char0 == 'undefined') char0 = '"'
    if (typeof char1 == 'undefined') char1 = char0

    //quote the elements of the array
    var results = []
    for (var i = 0; i < this.length; i++) {
        results[i] = ((typeof this[i] == 'undefined') ? 'undefined' : this[i]).neosysquote(char0, char1)
    }

    //return as a single string
    return results.join(' ')

}

//add double quotes
function neosysquote(arg1, arg2) {
    if (!arg2) arg2 = '"'
    return '"' + arg1 + '"'
}

/////////////////
// MATH FUNCTIONS
/////////////////

//round a number to n decimal places
function neosysround(number, ndecimals) {
    //return Math.round(number*Math.pow(10,ndecimals))/Math.pow(10,ndecimals)
    var number2 = Math.round(Math.abs(number) * Math.pow(10, ndecimals)) / Math.pow(10, ndecimals)
    if (number < 0) return -number2
    return number2
}

//convert to number
function neosysnumber(arg1) {

    //must a number if numeric
    if (typeof arg1 == 'numeric') return arg1

    var result = (+arg1)

    //fail if not numeric as defined by neosysnum()
    if (!neosysnum(arg1) || isNaN(result)) {
        systemerror('neosysnumber()', '"' + arg1 + '" is not numeric')
        result = 0
    }

    return result

    /* 
    // neosysassertnumeric(arg1,'neosysnumber','arg1')
    if (arg1=='') return 0
    if (typeof arg1=="number") return arg1
    try
    {
    return parseFloat(arg1)+0
    }
    catch(e)
    {
    return 0
    }
    */
}

//check if numeric
//maximum 14 before decimal place
// otherwise numbers resort to E exponent form when converted to strings
//disallow numbers that are too large (or small TODO) to be represented accurately and without exponentials
gnumericregexp = RegExp('(^[-+]?\\d{1,14}[.]?\\d*$)|(^[-+]?\\d*[.]?\\d{1,14}$)|(^$)')//put most likely first
function neosysnum(arg1) {

    if (typeof arg1 == 'string') {
        if (!arg1.length) return true//'' is considered numeric in neosys/pick
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

/* 
function neosysnum(arg1)
//return true if arg1 consists only of digits and optionally one "." 
//and one prefixed "-"

try
{
arg1=arg1.toString()
  
//plain dot or dash is not allowed
if (arg1=='.'||arg1=='-') return false

//all digits are allowed
arg1=arg1.replace(/[0123456789]/g,'')

//one decimal point is allowed
if (arg1==''||arg1=='.') return true
  
//one leading minus sign is allowed
if (arg1=='-'||arg1=='-.') return true
  
}
catch (e)
{
}
return false

} 
*/

function raiseerror(number, description) {

    //determine our standard error message for the given error number
    var msg
    switch (number) {
        case 100:
            {
                msg = 'Non-numeric data'
                break
            }
        default: msg = '-'
    }

    var err = '\r\rNEOSYS Error No:' + number + '\r' + msg + '\r' + description + '\r\r'
    //if (isMac)
    //{ 
    // alert(err)
    // throw 1
    //}
    //else
    //{
    //throw new Error(number,'\r\rNEOSYS Error No:'+number+'\r'+msg+'\r'+description+'\r\r')
    systemerror('', 'NEOSYS Error No:' + number + '\r' + msg + '\r' + description)
    //}
}

function neosysassertobject(object, funcname, varname) {
    if (!object || typeof (object) != 'object') {
        raiseerror(1000, 'In ' + funcname + ', ' + varname + ' is not an object.')
        return false
    }
    return true
}

function neosysassertfunctionname(arg1, funcname, varname) {
    try {
        var func = eval(arg1)
        if (typeof (func) == 'function') return true
    }
    catch (e) {
    }
    raiseerror(1000, 'In ' + funcname + ', ' + varname + ' ' + neosysquote(arg1) + ' is not a function name.')
    return
}

function neosysassertstring(arg1, funcname, varname) {
    if (typeof (arg1) != 'string') raiseerror(1000, 'In ' + funcname + ', ' + varname + ' ' + neosysquote(arg1) + ' is not a string.')
}

function neosysassertnumeric(arg1, funcname, varname) {
    //non numeric error
    if (!neosysnum(arg1)) raiseerror(100, 'In ' + funcname + ', ' + varname + ' ' + arg1 + ' is not numeric.')
}

function neosysmod(num, div) {
    result = num % div
    if (result >= 0)
        return result
    else
        return result + div
}

/////////////////////////
//TIME AND DATE FUNCTIONS
/////////////////////////

function neosysconvarray(functionx, mode, value, params) {
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

    //can handle an array of values
    if (typeof value == 'object')
        return neosysconvarray(NUMBER, mode, value, params)

    if (typeof value == 'string') {

        //can handle records, multivalues and subvalues
        if (value.indexOf(fm) + 1)
            return neosysconvarray(NUMBER, mode, value.split(fm), params).join(fm)
        else if (value.indexOf(vm) > +1)
            return neosysconvarray(NUMBER, mode, value.split(vm), params).join(vm)
        else if (value.indexOf(sm) > +1)
            return neosysconvarray(NUMBER, mode, value.split(sm), params).join(sm)

    }

    //empty in, empty out
    if (value == '')
        return value

    //accept comma as decimal point - use exceptions for speed since usually string but might not be
    try {
        value = value.replace(/,/gi, '.')
    }
    catch (e) { }

    params = params.split(',')

    //prevent reformatting
    if (params[0] == '')
        params[0] = value.toString().replace(/[^0-9.]/gi, '').neosysfield('.', 2, 1).length.toString()

    //if cannot parseFloat then is deemed not a number
    //value = parseFloat(value)
    //if (isNaN(value)) return null
    if (!neosysnum(value))
        return null
    value = neosysnumber(value)

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
            params[0] = (typeof gndecs == 'undefined') ? gds.getx('NDECS') : gndecs.toString()
        }

        //do formatting
        if (params[0].match(/^\d+$/)) {
            //value=neosysformatnumber(value,params[0])
            var ndecimals = neosysnumber(params[0])
            value = neosysround(value, ndecimals)
            if (ndecimals > 0) {
                var temp = value.toString().split(".")
                if (temp.length == 1) temp[1] = ''
                value = temp.join('.') + '00000000000000000000'.substr(0, ndecimals - temp[1].length)
            }

        }

        //remove training zeros
        if (nozero && value == 0) value = ''
        // value=(parseFloat(value)+0).toString()

    }

    //input conversion 
    if (mode == 'ICONV') {

        //prevent negative
        if (value < 0 && params[1] == 'POSITIVE') {
            gmsg = value + ' is negative but must be positive'
            return null
        }

        //minimum
        if (params[1] != '' && neosysnum(params[1]) && gvalue < +params[1]) {
            gmsg = 'must not be less than ' + params[1]
            return null
        }

        //maximum
        //if (params[2]!=''&&neosysnum(params[2])&&gvalue>+params[2])
        if (params[2] && neosysnum(params[2]) && gvalue > +params[2]) {
            gmsg = 'must not be more than ' + params[2]
            return null
        }

        //   var result=parseFloat(value)
        result = value.toString()
        if (!neosysnum(result)) {
            gmsg = 'number is too large'
            return null
        }

    }

    //output conversion
    else {
        //zzz should format it with params?
        var result = value.toString()
    }

    return result

}

var gdatedaypos
var gdatemonthpos
var gdateyearpos

function setdateformat() {

    gfirstdayofweek = neosysgetcookie(glogincode, 'NEOSYS2', 'fd')
    if (!gfirstdayofweek) gfirstdayofweek = 1
    gfirstdayofweek = Number(gfirstdayofweek)

    var dateformat = neosysgetcookie(glogincode, 'NEOSYS2', 'df')
    if (dateformat.slice(0, 2) == '01') {
        gdatedaypos = 1
        gdatemonthpos = 0
        gdateyearpos = 2
        gdateformat = 'M/d/yyyy'
    }
    else if (dateformat.slice(0, 2) == '31') {
        gdatedaypos = 0
        gdatemonthpos = 1
        gdateyearpos = 2
        gdateformat = 'd/M/yyyy'
    }
    else {
        gdatedaypos = 2
        gdatemonthpos = 1
        gdateyearpos = 0
        gdateformat = 'yyyy/M/d'
    }
    return
}

function invaliddatemsg() {
    if (typeof gdatedaypos == 'undefined')
        setdateformat()
    var tt
    /*
    var year4 = new Date().getYear().toString()
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

function DATE(mode, value, params) {

    if (typeof gdatedaypos == 'undefined') setdateformat()

    //gmsg='e.g. 31, 31/1, 31/1/01 or 31/1/2001'

    //can handle an array of values
    if (typeof value == 'object')
        return neosysconvarray(DATE, mode, value, params)
    /*
    //can handle an array of values
    if (typeof(value)=='object')
    {
    var result=[]
    for (var i=0;i<value.length;i++)
    {
    result[i]=DATE(mode,value[i],params)
    
    //if any conversion fails return complete failure
    if (result[i]==null)
    {
    gmsg=invaliddatemsg()
    return null
    }
    
    }
    return result
    }
    */

    if (value == '') return ''

    //input conversion
    if (mode == 'ICONV') {

        //four digits are assumed to be in ddmm or mmdd format
        if (value.match(/^\d{4}$/))
            value = value.substr(0, 2) + '/' + value.substr(2, 2)

        //five digits are assumed to be already in internal format
        if (value.match(/^\d{5}$/))
            return value

        //six/eight digits are assumed to be in DDMMYY/MMDDYY/DDMMYYYY/MMDDYYYY format
        if (value.match(/^(\d{6})|(\d{8})$/))
            value = value.substr(0, 2) + '/' + value.substr(2, 2) + '/' + value.substr(4)

        //assume in the format 31x1x2000
        value = value.replace(/\W/g, ' ').split(' ').slice(0, 3)

        //check day is one or two digits and force to three values (blanks if necessary)
        if (value.length == 1) {
            value[gdatedaypos] = value[0]
            value[gdatemonthpos] = ''
            value[gdateyearpos] = ''
        }
        else if (value.length == 2)
            value[2] = ''

        //default params
        if (typeof params == 'undefined') params = ''

        //option to not iconv (re
        //if (params=='NOICONV')
        //{
        // if (value.length==1)
        // return value[0]
        //}

        var fromto = ''
        if (params.indexOf('=') + 1) {
            params = params.split('=')
            fromto = params[0]
            var otherdateid = params[1]
            otherdate = gds.getx(otherdateid, grecn)
            if (typeof otherdate == 'object') otherdate = otherdate[0]

            //params=otherdate.neosysoconv('[DATE]').neosysfield('/',2,2)
            //get month/year
            params = otherdate.neosysoconv('[DATE,MONTH]') + '/' + otherdate.neosysoconv('[DATE,YEAR]')

        }

        //split the params
        params = params.replace(/\//g, ' ').split(' ')

        //default to month parameter
        //if (!value[1]&&!!params[0]) value[1]=params[0]
        if (!value[gdatemonthpos] && !!params[0])
            value[gdatemonthpos] = params[0]

        //otherwise default to current month
        if (!value[gdatemonthpos])
            value[gdatemonthpos] = new Date().getMonth() + 1

        //convert month names to month numbers
        var monthno = parseInt(value[gdatemonthpos], 10)
        if (isNaN(monthno)) {
            var mthname = value[gdatemonthpos].toUpperCase()
            var mthnamelen = mthname.length
            for (var ii = 0; ii < 12; ++ii) {
                if (gmonthnames[ii].slice(0, mthnamelen).toUpperCase() == mthname) {
                    monthno = ii + 1
                    value[gdatemonthpos] = monthno
                    break
                }
            }
        }

        //default to year parameter
        if (!value[gdateyearpos] && !!params[1])
            value[gdateyearpos] = params[1]

        //otherwise default to current year
        if (value[gdateyearpos])
            value[gdateyearpos] = ADDCENT(value[gdateyearpos])
        else
            value[gdateyearpos] = new Date().getFullYear()

        //check integers
        if (isNaN(parseInt(value[gdatedaypos], 10)) || isNaN(monthno) || isNaN(parseInt(value[gdateyearpos], 10))) {
            gmsg = invaliddatemsg()
            return null
        }

        //check day is one or two digits
        if (!value[gdatedaypos].match(/^\d{1,2}$/)) {
            gmsg = invaliddatemsg()
            return null
        }

        /* convert to ms date
        //try to convert to internal date format
        var result=new Date(value[2],value[1]-1,value[0])
        if (isNaN(result))
        {
        gmsg=invaliddatemsg()
        return null
        }
        */

        //convert dates from MS base to base zero=31/12/67
        var result = (Date.UTC(value[gdateyearpos], value[gdatemonthpos] - 1, value[gdatedaypos]) - Date.UTC(1967, 11, 31)) / 24 / 60 / 60 / 1000

        //prevent 6 digit years ie ge 15 oct 2241
        if (result.toString().length > 5) {
            gmsg = invaliddatemsg()
            return null
        }

        //check that the oconv matches the iconv
        //the above algorithm allows dom <=99 and is calculated into next months
        var result2 = DATE('OCONV', result).split('/')
        if (parseInt(result2[gdatedaypos], 10) != parseInt(value[gdatedaypos], 10) || parseInt(result2[gdatemonthpos], 10) != parseInt(value[gdatemonthpos], 10)) {
            gmsg = invaliddatemsg()
            return null
        }

        //check versus/amend other date

        if (fromto) {

            var otherdate0 = otherdate

            //prevent todate less than fromdate
            if (fromto == 'TO' && result > otherdate) otherdate = result
            if (fromto == 'FROM' && result < otherdate) otherdate = result

            //if change fromdate and todate blank or same as from goldvalue
            if (fromto == 'UPTO' && (otherdate == '' || otherdate == goldvalue)) otherdate = result

            //update the otherdate
            if (otherdate != otherdate0) gds.setx(otherdateid, grecn, otherdate)

        }
    }

    //output conversion
    else {
        //  if (value=='"') alert(value)
        //if not digits then return unconverted
        if (typeof value == 'string' && !value.match(/^\d*$/)) return value
        //  if (value=='"') alert('x'+value)

        /* convert to ms date
        value=new Date(value)
        if (isNaN(value))
        {
        gmsg=invaliddatemsg()
        return null
        }
        //zzz should format it with params?
        var result=value.getDate()+'/'+(value.getMonth()+1)+'/'+value.getFullYear()
        */

        //convert from 1=1/1/67 to text DD/MM/YYYY format
        //result=new Date(1967,11,31+parseInt(value,10))
        result = new Date
        result.setTime(Date.UTC(1967, 11, 31 + parseInt(value, 10)))

        switch (params) {
            case 'DOW':
                {
                    result = ((value - 1) % 7) + 1
                    break
                }
            case 'DAYNAME':
                {
                    result = ((value - 1) % 7) + 1
                    result = ['Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday'][result - 1]
                    break
                }
            case 'DAYNAME3':
                {
                    result = ((value - 1) % 7) + 1
                    result = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'][result - 1]
                    break
                }
            case 'DOM':
                {
                    result = result.getUTCDate()
                    break
                }
            case 'MONTH':
                {
                    result = result.getUTCMonth() + 1
                    break
                }
            case 'DOM2':
                {
                    result = ('0'+result.getUTCDate()).slice(-2)
                    break
                }
            case 'MONTH2':
                {
                    result = ('0'+(result.getUTCMonth() + 1)).slice(-2)
                    break
                }
            case 'YEAR':
                {
                    result = result.getUTCFullYear()
                    break
                }
            case 'YEAR2':
                {
                    result = parseInt(result.getUTCFullYear().toString().slice(-2), 10)
                    break
                }
            case 'PERIOD':
                {
                    //returns the current month/current year eg 1/2000
                    var result = (result.getUTCMonth() + 1) + '/' + result.getUTCFullYear()
                    break
                }
            case 'PERIOD2':
                {
                    //returns the current month/current year eg 1/00
                    var result = (result.getUTCMonth() + 1) + '/' + result.getUTCFullYear().toString().slice(-2)
                    break
                }
            case 'YEARPERIOD':
                {
                    //returns the current current year.current month eg 2001.01
                    var result = result.getUTCFullYear() + '.' + ('00' + (result.getUTCMonth() + 1)).slice(-2)
                    break
                }
            case 'MINIMAL':
                {
                    var result2 = []
                    result2[gdatedaypos] = result.getUTCDate()
                    result2[gdatemonthpos] = result.getUTCMonth() + 1
                    result2[gdateyearpos] = result.getUTCFullYear()
                    result = result2

                    //trim same year and month
                    var curryear = (new Date().getFullYear()).toString()
                    if (result[gdateyearpos] == curryear) {
                        result[gdateyearpos] = ''
                        var currmonth = (new Date().getMonth() + 1).toString()
                        if (result[gdatemonthpos] == currmonth) {
                            result[gdatemonthpos] = ''
                        }
                    }
                    result = result.neosysjoin('/').neosystrim('/')

                    //add day of week
                    var dow = ((value - 1) % 7) + 1
                    result = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'][dow - 1] + ' ' + result

                    break
                }
            default:
                {
                    var result2 = []
                    result2[gdatedaypos] = result.getUTCDate()
                    result2[gdatemonthpos] = result.getUTCMonth() + 1
                    result2[gdateyearpos] = result.getUTCFullYear()
                    result = result2.join('/')
                }
        }

    }

    return result

}

function TIME(mode, value, params) {

    gmsg = 'e.g. 12:45, 1245 12.75, 00:00, 23:59, 11:59:59, 24:00, 25:00 etc.'

    //can handle an array of values
    if (typeof value == 'object')
        return neosysconvarray(TIME, mode, value, params)
    /*
    //can handle an array of values
    if (typeof(value)=='object')
    {
    var result=[]
    for (var i=0;i<value.length;i++)
    {
    result[i]=TIME(mode,value[i],params)
    
    //if any conversion fails return complete failure
    if (result[i]==null) return null
    
    }
    return result
    }
    */

    //blank in .. blank out
    if (value == '') return ''

    if (mode == 'OCONV') {

        //times by themselves for now are considered to be "local time" of data entry person tz could be added info
        //DATE_TIME uses gtz
        //value=Number(value)+gtz[0]

        //also in decide.htm
        var secs = value % 60
        value -= secs
        var mins = (value / 60) % 60
        value -= 60 * mins
        //var hours=(value/60/60)%24
        //allow 24:00 and 25:00 etc
        var hours = (value / 60 / 60)
        result = ('0' + hours).slice(-2) + ':' + ('0' + mins).slice(-2)
        //result+=':'+('0'+secs).slice(-2)
    }
    else {

        //format must be HH:MM or HH:MM:SS (":" may also be space or missing)
        var temp = value
        if (temp.match(/^\d{4}$/))
            temp = temp.substr(0, 2) + ':' + temp.substr(2, 2)
        if (temp.match(/^\d{6}$/))
            temp = temp.substr(0, 2) + ':' + temp.substr(2, 2) + ':' + temp.substr(4, 2)
        temp = temp.neosysconvert('. ', '::').split(':')
        if (!temp[1]) temp[1] = '00'
        if (!temp.join(':').match(/(^\d{1,2}:\d{1,2}$)|(^\d{1,2}:\d{1,2}:\d{1,2}$)/)) return null

        //if (!temp[1]) temp[1]=0
        if (!temp[2]) temp[2] = 0
        temp[0] = +temp[0]
        temp[1] = +temp[1]
        temp[2] = +temp[2]
        //if (temp[0]>23) return null
        //allow up to two days eg 47:59
        if (temp[0] > 47) return null
        if (temp[1] > 59) return null
        if (temp[2] > 59) return null

        result = temp[0] * 60 * 60 + temp[1] * 60 + temp[2]
        //times by themselves for now are considered to be "local time" of data entry person tz could be added info
        //DATE_TIME uses gtz
        //result-=gtz[0]

    }

    return result

}

function DATE_TIME(mode, value, params) {
    if (value == '') return ''
    if (mode == 'OCONV') {
        if (!params) params = ''
        params = (params + '!').split('!')
        value = value.toString().split('.')
        var datebit = Number(value[0])
        var timebit
        if (value.length < 1)
            timebit = ''
        else {
            timebit = Number(value[1]) + gtz[0]
            //assume -86400<gtz[0]<86400
            if (timebit < 0) {
                datebit -= 1
                timebit += 86400
            }
            else if (timebit > 86400) {
                datebit -= 1
                timebit += 86400
            }
        }
        return datebit.neosysoconv('[DATE,' + params[0] + ']') + ' ' + timebit.neosysoconv('[TIME,' + params[1] + ']')
    }
    else {
        //iconv not implemented yet
        return value
    }

}

function PERIOD_OF_TIME(mode, value, params) {
    if (value == '') return ''

    var result

    //can handle an array of values
    if (typeof value == 'object')
        return neosysconvarray(PERIOD_OF_TIME, mode, value, params)
    /*
    //can handle an array of values
    if (typeof(value)=='object')
    {
    result=[]
    for (var i=0;i<value.length;i++)
    {
    result[i]=PERIOD_OF_TIME(mode,value[i],params)
    
    //if any conversion fails return complete failure
    if (result[i]==null) return null
    
    }
    return result
    }
    */

    if (mode == 'ICONV') {

        //if already numeric then simply return it
        if (neosysnum(value)) return value

        //allow slash, dash, space and comma as well as ":" for separator
        value = value.neosysconvert('/- ,', '::::')

        value = value.split(':')
        if (value.length < 2) value[1] = 0

        hours = value[0]
        mins = value[1]

        //check numeric otherwise return undefined
        if (!neosysnum(hours) || !neosysnum(mins)) {
            return null
        }

        result = +hours + (+mins) / 60

    }
    else {

        if (!neosysnum(value)) {
            //out=''
            //status()=2
            return null
        }

        //nothing in nothing out
        if (value == '') return ''

        value = value.toString()
        value = value.split('.')
        if (value[0] == '') value[0] = '0'
        if (value.length < 2) value[1] = '0'

        var result = value[0] + ':'
        temp = value[1]
        if (temp) temp = parseFloat('.' + temp)
        temp = neosysround(temp * 60, 0)
        temp = '00' + temp
        result += temp.slice(temp.length - 2)

    }

    return result

}

//object eg x=new neosysamount('123USD')
var gleadingnumberregex = /(^[-+]?\d{1,14}[.]?\d*)|(^[-+]?\d*[.]?\d{1,14})|(^$)/
function neosysamount(string) {
    neosysassertstring(string, 'neosysamount')
    this.string = string
    //this.amount=string.replace(/([A-Z]*$)/,'')
    //this.amount = string.replace(/([^\+\-\.0123456789]*$)/, '')
    //this.unit = RegExp.$1
    var result = string.match(gleadingnumberregex)
    this.amount = string.slice(0, result.lastIndex)
    this.unit = string.slice(result.lastIndex)
}

function neosysaddunits(a, b) {

    //add a to b
    //a and b can be a mv list or js array of amounts with unit codes eg 200STG','300YEN','100USD
    //b must be ascii alphabetic order

    if (a == '') return b
    if (b == '') return a

    //quick calc and exit if both are plain numeric
    if (neosysnum(a) && neosysnum(b)) return Number(a) + Number(b)

    if (typeof a == 'string') a = a.split(vm)
    else if (typeof a == 'number') a = [a.toString()]
    if (typeof b == 'string') b = b.split(vm)
    else if (typeof b == 'number') b = [b.toString()]

    for (var an = 0; an < a.length; an++) {

        var ax = new neosysamount(a[an])

        var bx = new Object
        bx.amount = ''
        bx.unit = ''

        for (var bn = 0; bn < b.length; bn++) {
            var bx = new neosysamount(b[bn])
            if (bx.unit == ax.unit) {
                if (bx.amount.length || ax.amount.length) {
                    ndecs = ax.amount.neosysfield('.', 2).length
                    bndecs = bx.amount.neosysfield('.', 2).length
                    if (bndecs > ndecs) ndecs = bndecs
                    if (neosysnum(ax.amount) && neosysnum(bx.amount)) b[bn] = (Number(bx.amount) + Number(ax.amount)).neosysoconv('[NUMBER,' + ndecs + ']') + ax.unit
                }
                else {
                    b[bn] = ax.unit
                }
                break
            }
            else if (bx.unit >= ax.unit) break
        } // next bn
        if (bx.unit != ax.unit) b = b.neosysinsert(bn + 1, ax.amount + ax.unit)

    } // next an

    return b.join(vm)

}

//neosystime
function neosystime(mode) {
    //returns the current LOCAL time as neosystime (ms since midnight) an integer
    //or converted to a given external format
    //var now=new Date()
    var now = Number(new Date) - (new Date).getTimezoneOffset() * 60000
    var result = now.getHours() * 60 * 60 + now.getMinutes() * 60 + now.getSeconds()

    if (mode) result = result.neosysoconv('[DATE,' + mode + ']')

    return result

}

function neosysdate(year, month, day) {

    //returns todays date (LOCAL timezone date) as an integer 1=1/1/1967
    //or converted to external format with a parameter
    //result=parseInt((new Date)/24/60/60/1000,10)
    //result=parseInt((Number(new Date)-(new Date).getTimezoneOffset()*60000)/24/60/60/1000,10)
    var newdatel
    if (year && month && day) {
        newdate = new Date(Date.UTC(Number(year), Number(month) - 1, Number(day)))
        result = parseInt((Number(newdate)) / 24 / 60 / 60 / 1000, 10)
    }
    else {
        newdate = new Date
        result = parseInt((Number(newdate) - (new Date).getTimezoneOffset() * 60000) / 24 / 60 / 60 / 1000, 10)
    }

    //convert to PICK dates which start on 31/12/1967
    result -= parseInt(Date.UTC(1967, 11, 31) / 24 / 60 / 60 / 1000, 10)

    return result

}

String.prototype.neosysiconv = stringneosysiconv
function stringneosysiconv(conversion) {
    return neosysiconv(this.toString(), conversion)
}

function neosysiconv(value, conversion) {
    return neosysconv('ICONV', value, conversion)
}

String.prototype.neosysoconv = stringneosysoconv
Number.prototype.neosysoconv = stringneosysoconv
function stringneosysoconv(conversion) {
    return neosysoconv(this.toString(), conversion)
}

function neosysoconv(value, conversion) {
    return neosysconv('OCONV', value, conversion)
}

Array.prototype.neosysoconv = neosysoconvarray
function neosysoconvarray(conversion) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].toString().neosysoconv(conversion)
    return result
}

Array.prototype.neosysiconv = neosysiconvarray
function neosysiconvarray(conversion) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].toString().neosysiconv(conversion)
    return result
}

function neosysconv(mode, value, conversion) {

    if (conversion == '') return value

    if (typeof (conversion) == 'undefined') {
        alert('Conversion for ' + neosysquote(value) + ' is undefined\nNo conversion performed.')
        return value
    }

    //can handle an array of values
    if (typeof value == 'object') {
        var result = []
        for (var i = 0; i < value.length; i++) {
            result[i] = neosysconv(mode, value[i], conversion)

            //if any conversion fails return complete failure
            if (result[i] == null) return null

        }
        return result
    }

    //can handle an array of conversions
    if (typeof (conversion) == 'object') {
        var result = []
        for (var i = 0; i < value.length; i++) {
            result[i] = neosysconv(mode, value, conversion[i])

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
        //if (!neosysassertfunctionname(temp[0],' mode="'+mode+'" conversion="'+temp[0]+'" value="'+value+'" neosysconv','temp[0]'))
        //{
        // return value
        //}
        neosysassertfunctionname(temp[0], ' mode="' + mode + '" conversion="' + temp[0] + '" value="' + value + '" neosysconv', 'temp[0]')

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
            case 'R': return (neosysstr(padchar, length) + value).slice(-length)
            case 'L': return (value + neosysstr(padchar, length)).slice(0, length)
        }
    }

    systemerror('neosysconv()', neosysquote(conversion) + 'conversion is unknown, value=' + value)

    return value

}

//replace
String.prototype.neosysreplace = function neosysreplace(fieldno, valueno, subvalueno, replacestring) {

    if (subvalueno) {
        if (!fieldno) fieldno = 1
        if (!valueno) valueno = 1
        var value = this.neosysextract(fieldno, valueno)
        replacestring = value.neosysfieldstore(sm, subvalueno, 1, replacestring)
    }

    if (valueno) {
        var field = this.neosysextract(fieldno)
        replacestring = field.neosysfieldstore(vm, valueno, 1, replacestring)
        if (!fieldno) fieldno = 1
    }

    return this.neosysfieldstore(fm, fieldno, 1, replacestring)

}

//fieldstore

String.prototype.neosysfieldstore = neosysfieldstore
function neosysfieldstore(sep, start, length, insertstring, noextend) {

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

Array.prototype.neosysfieldstore = neosysfieldstorearray
function neosysfieldstorearray(sep, start, length, insertstring) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].neosysfieldstore(sep, start, length, insertstring)
    return result
}

//field on array returns array of fields
//can work recursively on arrays of arrays
Array.prototype.neosysfield = function neosysfieldarray(sep, start, length) {

    var newarray = []
    for (var i = 0; i < this.length; i++) newarray[i] = this[i].neosysfield(sep, start, length)
    return newarray

}

//field on string returns string
String.prototype.neosysfield = function neosysfieldstring(sep, start, length) {

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

String.prototype.neosyslocate = neosyslocatestring
function neosyslocatestring(value, order, sep, startfrom) {

    //quick return
    if (this == value) return 1

    if (typeof sep == 'undefined') sep = vm

    return this.split(sep).neosyslocate(value, order, startfrom)

}

Array.prototype.neosyslocate = neosyslocatearray
function neosyslocatearray(value, order, startfrom) {

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

String.prototype.neosysextract = neosysextractstring
function neosysextractstring(fieldn, valuen, subvaluen) {
    return this.split(fm).neosysextract(fieldn, valuen, subvaluen)
}

Array.prototype.neosysextract = neosysextractarray
function neosysextractarray(fieldn, valuen, subvaluen) {

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
Array.prototype.neosysdelete = neosysdeletearray
function neosysdeletearray(index) {
    var newarray = this.slice(0, index - 1)
    var remainder = this.slice(index)
    for (var i = 0; i < remainder.length; i++) newarray[newarray.length] = remainder[i]
    return newarray
}

//insert (1 based!)
Array.prototype.neosysinsert = neosysinsertarray
function neosysinsertarray(index, values) {

    if (typeof values != 'object') values = [values]

    var newarray = this.slice(0, index - 1)

    for (var i = 0; i < values.length; i++) newarray[newarray.length] = values[i]

    var remainder = this.slice(index - 1)
    for (var i = 0; i < remainder.length; i++) newarray[newarray.length] = remainder[i]

    return newarray

}

/* not used anywhere?
//locate
function neosyslocate(value,values)
{
if (values==null) return false
if (typeof(values)!='object'&&typeof(values)!='string') return value==values
return values.neosyslocate(value)
}
*/

//plural

//given a number!=1 or boolean=true return the string with (s) appended (or a given suffix)
String.prototype.neosysplural = function neosysplural(n, suffix) {
    return this + (((typeof n == 'boolean' && !n) || n == 1) ? '' : (suffix ? suffix : 's'))
}

String.prototype.neosyssingular = neosyssingular
function neosyssingular() {
    temp = this.toString()

    if (temp.neosysright(2) == 'ES') {

        //COMPANIES=COMPANY
        //ADDRESSES=ADDRESS
        if (temp.neosysright(3) == 'IES') temp = temp.slice(0, -3) + 'Y'
        else if (temp.neosysright(4) == 'SSES') temp = temp.slice(0, -2)
        else if (temp.neosysright(4) == 'SHES') temp = temp.slice(0, -2)
        else if (temp.neosysright(4) == 'CHES') temp = temp.slice(0, -2)
        else temp = temp.slice(0, -1)

    }
    else {

        if (temp.neosysright(1) == 'S') {
            //ANALYSIS, DOS
            if (temp.neosysright(2) != 'IS' && temp.neosysright(2) != 'OS') temp = temp.slice(0, -1)
        }

    }

    return temp

}

//right

String.prototype.neosysright = neosysstringright
function neosysstringright(len) {
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

        case "undefined":
            {
                return (0)
            }

        case "string":
            {
                if (value.length == 0) return (0)
                return (value != 0)
            }

        case "number":
            {
                return (value != 0)
            }

        case "boolean":
            {
                return (value)
            }

        case "object":
            {

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
Array.prototype.neosysswap = function neosysswaparray(swapfrom, swapto) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].neosysswap(swapfrom, swapto)
    return result
}

String.prototype.neosysswap = function neosysswapstring(swapfrom, swapto) {
    //should this be made case sensitive or an option added?

    if (!swapfrom) return this

    //make sure all regexpression characters are escaped
    //if (swapfrom) swapfrom='\\'+swapfrom.split('').join('\\')
    swapfrom = swapfrom.replace(/([\\,\^,\$,\*,\+,\?,\.,\(,\),\|,\{,\},\[,\]])/g, "\\$1")

    swapfrom = new RegExp(swapfrom, 'gi')
    return this.replace(swapfrom, swapto)

}

//convert
Array.prototype.neosysconvert = function neosysconvertarray(convertfrom, convertto) {
    var result = []
    for (i = 0; i < this.length; i++) result[i] = this[i].neosysconvert(convertfrom, convertto)
    return result
}

String.prototype.neosysconvert = function neosysconvertstring(str1, str2) {

    if (!str2) str2 = ''

    str1 = str1.toString()
    str2 = str2.toString()
    result = this.toString()
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

Array.prototype.neosysdcount = function neosysdcountarray(str2) {
    var result = []
    for (var ii = 0; ii < this.length; ii++) {
        result[ii] = this[ii].neosysdcount(str2)
    }
    return result
}

String.prototype.neosysdcount = function neosysdcountstring(str2) {
    return this.neosyscount(str2) + (this.length ? 1 : 0)
}

Array.prototype.neosyscount = function neosyscountarray(str2) {
    var result = []
    for (var ii = 0; ii < this.length; ii++) {
        result[ii] = this[ii].neosyscount(str2)
    }
    return result
}

String.prototype.neosyscount = function neosyscount(str2) {
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

String.prototype.neosysaddcent = neosysaddcentmethod
Number.prototype.neosysaddcent = neosysaddcentmethod
function neosysaddcentmethod(type) {
    if (!type) type = 'OCONV'
    var result = ADDCENT(type, this.toString())
    return result
}

function ADDCENT(type, input, mode) {

    //
    var centuryyear = 50

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
    input = input.neosysconvert('-/', '  ')

    //two numbers means 99/99
    if (input.neosyscount(' ') == 1) input = input.neosysconvert(' ', '/')

    //four numbers means 99/99-99/99
    if (input.neosyscount(' ') == 3) {
        input = input.split(' ')
        input = input[0] + '/' + input[1] + '-' + input[2] + '/' + input[3]
    }

    var out = ''

    //range of periods 1/98-3/98
    if (input.indexOf('-') >= 0) {
        var out1 = ADDCENT(type, input.neosysfield('-', 1), mode)
        if (out1 == null) return null
        var out2 = ADDCENT(type, input.neosysfield('-', 2), mode)
        if (out2 == null) return null
        return out1 + '-' + out2
    }

    //remove any prefixed period number '1/99'
    var prefix = ''
    if (input.indexOf('/') >= 0) {
        prefix = input.neosysfield('/', 1) + '/'
        input = input.neosysfield('/', 2)
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

        //convert year period YYMM to YYYYMM except 19XX and 20XX
        if (input.length >= 4) {
            if (input2 != '19' && input2 != '20') {
                out = ((parseInt(input2, 10) < centuryyear) ? '20' : '19') + input
            }
            else {
                out = input
            }
        }
    }

    if (type == 'ICONV') out = out.substr(2, 2)

    return prefix + out

}

function CENTURY(type, input, mode) {
    // return input
    //
    var centuryyear = 50

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
        out = ((parseInt(input2, 10) < centuryyear) ? '20' : '19') + input
    }

    if (type == 'ICONV' && mode == '2') out = out.substr(2, 2)

    return out

}

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
            if (input[0].neosysaddcent() > mode[0].neosysaddcent()) {
                input[1] = (input[1] - 1).toString()
            }
        }
        else {
            input[1] = new Date().getYear()
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
    // gmsg='PLEASE ENTER A PERIOD, OR PERIOD/YEAR|EG. '+neosysquote(gcompany.currperiod)+' or '+neosysquote(gcompany.currperiodyear)
    // return null
    //}

    //check month
    var mths = []
    for (var i = 1; i <= maxmth; i++) { mths[i - 1] = i.toString() }
    if (!mths.neosyslocate(month)) return null

    //check year ... ICONV converts to 2 digits, OCONV converts to 4 digits
    year = ADDCENT(type, year)
    if (year == null) return null

    return month + '/' + year

}

function YEAR_PERIOD(type, input, mode) {

    //same as YEARPERIOD but 09.12 <-> 12/09
    //or if mode is 4 then 2009.12 <-> 12/2009

    //can handle an array of values but what about strings that appear as objects
    if (typeof input == 'object')
        return neosysconvarray(YEAR_PERIOD, type, input, mode)
    /*
    //can work on arrays (recursively)
    //nb "typeof this" is "object" even for strings!
    if (typeof input[0]!='undefined')
    {
    for (var i=0;i<input.length;i++) input[i]=YEARPERIOD(type,input[i],mode)
    return input
    }
    */

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
            input[1] = new Date().getYear()

        var month = input[0]
        var year = input[1]

        //trim leading zero
        if (month.substr(0, 1) == '0')
            month = month.substr(1)

        //check month
        if (!['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12'].neosyslocate(month))
            return null

        //check year ... ICONV converts to 2 digits, OCONV converts to 4 digits
        if (mode == '4')
            year = ADDCENT('OCONV', year, mode)
        else
            year = ADDCENT(type, year, mode)
        if (year == null)
            return null

        return year + '.' + ('00' + month).neosysright(2)

    }

    //oconv

    input = input.split('.')
    return (parseInt(input[1], 10) + 0) + '/' + ADDCENT(type, input[0], '')

}

function YEARPERIOD(type, input, mode) {

    //same as YEARPERIOD but 0912 <-> 12/09

    //can handle an array of values
    if (typeof input == 'object')
        return neosysconvarray(YEARPERIOD, type, input, mode)

    //can work on arrays (recursively)
    //nb "typeof this" is "object" even for strings!
    if (typeof input[0] != 'undefined') {
        for (var i = 0; i < input.length; i++) input[i] = YEARPERIOD(type, input[i], mode)
        return input
    }

    input = input.toString()
    if (input == '') return input

    //iconv
    if (type == 'ICONV') {

        input = input.split('/')

        //check in format 99 or 99/99
        if (input.length > 2) return null

        //default to current year
        if (input.length == 1) input[1] = new Date().getYear()

        var month = input[0]
        var year = input[1]

        //trim leading zero
        if (month.substr(0, 1) == '0') month = month.substr(1)

        //check month
        if (!['1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12'].neosyslocate(month)) {
            return null
        }

        //check year ... ICONV converts to 2 digits, OCONV converts to 4 digits
        year = ADDCENT(type, year, mode)
        if (year == null) return null

        //return year+'.'+('00'+month).neosysright(2)
        return year + ('00' + month).neosysright(2)

    }

    //oconv

    //input=input
    return (parseInt(input.slice(2), 10) + 0) + '/' + ADDCENT(type, input.slice(0, 2), '')

}

function neosystimers(ntimers) {
    this.timers = []
    for (var i = 0; i < ntimers; i++) {
        this.timers[i] = new neosystimer()
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

function neosystimer() {
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