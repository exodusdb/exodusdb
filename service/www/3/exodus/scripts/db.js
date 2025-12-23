//Copyright NEOSYS All Rights Reserved.

var glocktimeoutinmins=5
var trailingspaces=/\s*$/g

function exodus_dict_dow(di,many) {

 if (many) {

  di.checkbox='1;Mon:2;Tue:3;Wed:4;Thu:5;Fri:6;Sat:7;Sun'
  di.horizontal=true 
 }
 else
  di.conversion='1;Monday:2;Tuesday:3;Wednesday:4;Thursday:5;Friday:6;Saturday:7;Sunday'
 
}

function exodus_dict_url(di,protocol) {

 exodus_dict_text(di)
 di.validation='yield* exodus_val_url("'+protocol+'")'
}

function* exodus_val_url(protocol) {

 //protocol should be http or https
 if (protocol=='http'||protocol=='https') {

  var regexp=new RegExp(protocol+'\:\/\/.','i')
  if (!gvalue.match(regexp)) {
   return exodusinvalid('Should start with '+protocol+'://')
  }
  return true
 }
  
 var regexp=new RegExp('((http)|(https))'+'\:\/\/.','i')
 if (!gvalue.match(regexp)) {
   return exodusinvalid('Should start with http:// or https://')
 }
 
 return true
}

function* exodus_val_html() {
    var element=document.createElement('span')
    element.innerHTML=gvalue
    var normalisedhtml=element.innerHTML
    if (normalisedhtml!=gvalue) {
        if ((yield* exodusokcancel('Something is not exactly right with that code.\nConvert to standardised HTML?\n(required)',1))!=1)
            return yield* exodusinvalid()
        gvalue=element.innerHTML
    }
    return true
}

function exodus_dict_ipno(di,many,wildcard,plus) {

 exodus_dict_text(di)
 var tt='.0123456789'
 if (many) tt+=' ;,'
 if (wildcard) tt+='*'
 if (plus) tt+='+'
 di.validcharacters=tt
 //TODO validate true ip number patterns
}

function exodus_dict_emailaddress(di,sepchar) {

 var length=30
 if (!sepchar)
  sepchar=''
 if (sepchar&&!di.groupno)
  length=60
 if (!sepchar.slice(0,1).match(/[\'\"]/))
  sepchar='"'+sepchar+'"'
 exodus_dict_text(di,length)
 di.validation='yield* exodus_val_emailaddress('+sepchar+')'
}

function* exodus_val_diskdrive() {

 if (!gvalue) return true

 gvalue=gvalue.slice(0,1).toUpperCase()
 //if (!gvalue.match(/^[ABCDEFGHIJKLMNOPQRSTUVWXYZ](:)?$/))
  //return yield* exodusinvalid('Please enter a letter A-Z')

 if (gvalue.length==1&&gvalue!='0') gvalue+=':'

 return true
}

function exodus_dict_diskdrive(di) {

 //di.conversion+='C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:S:T:U:V:W:X:Y:Z'
 di.validcharacters='CDEFGHIJKLMNOPQRSTUVWXYZ:'
 di.validation='yield* exodus_val_diskdrive()'
 di.length=1
 di.maxlength=1
}

function* exodus_val_emailaddress(sepchar) {

 if (!gvalue) return true
 //Description: Most email validation regexps are outdated and ignore the fact that domain names can contain any foreign character these days,
 // as well as the fact that anything before @ is acceptable.
 // The only roman alphabet restriction is in the TLD, which for a long time has been more than 2 or 3 chars (.museum, .aero, .info).
 // The only dot restriction is that . cannot be placed directly after @. This pattern captures any valid, reallife email adress. 
 //Matches: whatever@somewhere.museum|||foreignchars@myforeigncharsdomain.nu|||me+mysomething@mydomain.com 
 //Non-Matches: a@b.c|||me@.my.com|||a@b.comFOREIGNCHAR 
 // if (!gvalue.match(/[-_.abcdefghijklmnopqrstuvwxyz1234567890]+@[-.abcdefghijklmnopqrsptuvwxyz1234567890]+\.[-_.abcdefghijklmnopqrstuvwxyz]+/i))
 var values
 if (sepchar) {

  gvalue=gvalue.replace(/[, ;]+/g,sepchar).replace(/^;+/,'').replace(/;+$/,'')
  values=gvalue.split(';')
 }
 else
  values=[gvalue]
 for (var ii=0;ii<values.length;ii++) {

  var value=values[ii]
  if (value&&!value.match(/^[\w.-]+@[^\W_][\w.-]*\.[\w]{2,}$/)) {

   var msg=value+'\rEmail address must be in the format xxxxxx@xxxxxx.xxx'
   if (sepchar)
    msg+=' '+sepchar+'...'
   return yield* exodusinvalid(msg)
  }
 }
 gvalue=values.join(sepchar)
 return true
}

/////////////////
//INDEX FUNCTIONS
/////////////////

function exodus_dict_index(di,filename,fieldname,many,allownew,warnnew) {

 if (!filename) filename=''
 if (!fieldname) fieldname=''
 if (typeof many == 'undefined') many=di.groupno
 if (typeof allownew == 'undefined') allownew=true
 if (typeof warnnew == 'undefined') warnnew=true

 if (!di.validation) di.validation='yield* form_val_index('+filename.exodusquote()+','+fieldname.exodusquote()+','+allownew+','+warnnew+')'

 if (!di.popup) di.popup='yield* form_pop_index('+filename.exodusquote()+','+fieldname.exodusquote()+','+many+')'
 
}

////////////////////
//DATABASE FUNCTIONS
////////////////////

function exodus_dict_year(dicti,from,to,defaultyear) {

 if (!from) from=0
 if (!to) to=0
 exodusassertobject(dicti,'exodus_dict_year','dicti')
 dicti.conversion='[NUMBER,0]'
 //dicti.align='R'
 // see exodus_dict_date() why switch to L
 dicti.align='L'
 dicti.length=4
 var curryear=exodusdate().exodusoconv('[DATE,YEAR]')
 var years=''
 for (var year=curryear+to;year>=curryear+from;year--) {

  if (years) years+=':'
  years+=year+';'+year
 }
 dicti.conversion=years
 if (typeof defaultyear!='undefined') dicti.defaultvalue='"'+(curryear+defaultyear)+'"'
}

function exodus_dict_datetimeupdated(di) {

 exodus_dict_datetime(di)
 di.readonly=true
 di.copyable=false
 //di.align='R'
 // see exodus_dict_date() why switch to L
 di.align='L'
 di.lowercase=true
}

function exodus_dict_datetime(di,params) {

 if (!params) params=''
 di.conversion='[DATE_TIME,'+params+']'
 //di.align='R'
 // see exodus_dict_date() why switch to L
 di.align='L'
 di.lowercase=true
}

function exodus_dict_time(di,mode,otherid,params) {
 
 if (!di.conversion) {

  if (!params) params=''
  else params=','+params
  di.conversion='[TIME'+params+']'
 }
 
 di.align='R'
 di.length=5 
 if (mode&&otherid) {

  di.defaultvalue='yield* exodus_def_time("'+mode+'","'+otherid+'")'
  di.validation='yield* exodus_val_time("'+mode+'","'+otherid+'")'
 }
 
}

function* exodus_def_time(mode,othertimeid) {

 if (!othertimeid) return ''
 
 othertime=yield* gds.getx(othertimeid)
 if (typeof othertime=='object') othertime=othertime[0]

 return othertime
}

function* exodus_val_time(mode,otherid) {
 
 if (!gvalue||!mode||!otherid) return true
 
 var time=+gvalue
 var oldtime=+goldvalue
 
 var othertime=yield* gds.getx(otherid)
 if (typeof othertime=='object') othertime=othertime[0]
 var othertime2=+othertime

 //prevent toperiod less than fromperiod
 if (mode=='FROM'&&time>othertime2) othertime2=time
 if (mode=='UPTO'&&time<othertime2) othertime2=time
    
 //if change fromperiod and toperiod blank or same as from goldvalue
 if (mode=='FROM'&&(othertime==''||othertime==oldtime)) othertime2=time

 //update the otherperiod
 if (othertime2!=(+othertime)) yield* gds.setx(otherid,grecn,othertime.toString())

 //gvalue=time
  
 return true
 
}

// exodus_dict_period(di,'FROM','PERIOD_TO','[FINANCIAL_PERIOD]')
function exodus_dict_period(di,mode,otherperiodid,conversion) {

 if (!mode) mode=''
 if (!otherperiodid) otherperiodid=''
 if (!conversion) conversion=''
 exodusassertobject(di,'exodus_dict_period','di')
 di.conversion='[PERIOD_OF_YEAR]'
 //di.align='R'
 // see exodus_dict_date() why switch to L
 di.align='L'
 di.length=5
 if (di.type=='F') di.validation='yield* exodus_val_period("'+mode+'","'+otherperiodid+'")'
}

function exodus_dict_year_period(di,mode) {

 if (!mode) mode=''
 exodusassertobject(di,'exodus_dict_yearperiod','di')
 di.conversion='[YEAR_PERIOD,'+mode+']'
 //di.align='R'
 // see exodus_dict_date() why switch to L
 di.align='L'
 di.length=5
}

function* exodus_val_period(mode,otherperiodid) {

 if (!gvalue) return true
 
 var value=gvalue.split('/')
 
 var oldvalue=goldvalue.split('/')
 
 if (mode&&otherperiodid) {
 
  otherperiod=yield* gds.getx(otherperiodid)
  if (typeof otherperiod=='object') otherperiod=otherperiod[0]
  var othervalue=otherperiod.split('/')
  
  //get year from the other period
  if (value.length==1||!value[1]) {
  
   //missing year is taken from other period or current period
   if (otherperiod) {

    value[1]=othervalue[1]
    if (mode=='FROM') {

     //make sure to month is not less than from month
     if (value[0]>othervalue[0]) othervalue[0]=value[0]
    }
    else {

     //make sure from month is not less than to month
     if (value[0]<othervalue[0]) othervalue[0]=value[0]
    }
   }
   else {

    value[1]=gperiod.split('/')[1]
   }
  }

  var otherjulianperiod=exodusjulianperiod(othervalue.join('/'),gmaxperiodno)
  var julianperiod=exodusjulianperiod(value.join('/'),gmaxperiodno)

  //prevent toperiod less than fromperiod
  if (mode=='FROM'&julianperiod>otherjulianperiod) othervalue=value
  if (mode=='UPTO'&julianperiod<otherjulianperiod) othervalue=value
    
  //if change fromperiod and toperiod blank or same as from goldvalue
  if (mode=='FROM'&&(otherperiod==''||otherperiod==goldvalue)) othervalue=value

  //update the otherperiod
  if (otherperiod!=othervalue.join('/')) yield* gds.setx(otherperiodid,grecn,othervalue.join('/'))

  gvalue=value.join('/')
  
 }
 
 return true
 
}

function exodus_dict_yesno(dicti,defaultvalue,type) {

 exodusassertobject(dicti,'exodus_dict_yesno','dicti')
 if (!type) type='dropdown' 
 if (typeof defaultvalue=='undefined') defaultvalue='"Y"'
 defaultvalue=defaultvalue.toString()
 if (type=='dropdown') dicti.conversion='Y;Yes:N;No'
 else if (type=='radio'||type=='hradio') {

  if (defaultvalue=='1'||defaultvalue=='0') dicti.radio='1;Yes:0;No'
  else dicti.radio='Y;Yes:N;No'
  if (type=='hradio') dicti.horizontal=true
 }
 
 if (typeof defaultvalue!='undefined') dicti.defaultvalue=defaultvalue
 
}

function exodus_dict_date(dicti,params) {

 if (typeof params=='undefined') params=''

 exodusassertobject(dicti,'exodus_dict_date','dicti')
 dicti.conversion='[DATE,'+params+']'
 //dicti.align='R' causes funny columns data entry (where?)
 //dicti.align='R' //put back so reports.htm shows neatly
 // dbform.js fixed to actaully do align = 'R', previously only did left
 // decided best to preserve the current but technically wrong left alignment
 // as to not disrupt users and in case aligning to right creates some bug
 dicti.align = 'L'


 dicti.length=10
 dicti.popup='yield* form_pop_calendar()'
 if (!gcalendarscript) gcalendarscript=1
 if (params) dicti.lowercase=true
}

function exodus_dict_number(dicti,params,minimum,maximum) {

 if (typeof params=='undefined') params=''
 exodusassertobject(dicti,'exodus_dict_number','dicti')
 
 //params can be
 //a digit for the number of decimals
 //BASE which indicated base format
 //NDECS in which case gndecs or getrecord('NDECS') used
 //append a Z for suppression of zeroes

 //minimum can be "POSITIVE" or a number
 if (typeof minimum=='undefined') minimum=''
 if (typeof maximum=='undefined') maximum=''
 params+=','+minimum+','+maximum
 dicti.conversion='[NUMBER,'+params+']'
 //dicti.align='R'
 // see exodus_dict_date() why switch to L
 dicti.align = 'L'

}

function exodus_dict_text(dicti,size,rows) {

 exodusassertobject(dicti,'exodus_dict_text','dicti')
 if (!dicti.readonly) {

  if (typeof size=='undefined') size=30
  if (typeof rows=='undefined') rows=1
 }
 dicti.align='T'
 if (typeof size!='undefined' && size)
  dicti.length=size
 if (typeof rows!='undefined' && rows)
  dicti.rows=rows
 if (typeof dicti.lowercase=='undefined')
  dicti.lowercase=true
}

function exodusrecord(dictarray,filename) {

 this.dict=dictarray
 this.filename=filename

 this.key=''
 this.revstr=''
 this.defaultrevstr=''
 this.data=null
 this.readenvironment=''
 this.dictitem=exodusrecord_dictitem
 this.dict.groups=[]
 this.dict.fieldandwordns=[]
 this.dict.fieldnos=new Object//not [] otherwise dictitem() finds array properties
   
 var response
 var sessionid
 var ngroups=0
 var nfields=0

 var validpropnames = /(^sequence$)|(^openfunction$)|(^dropdown$)|(^test$)|(^name$)|(^type$)|(^fieldno$)|(^title$)|(^groupno$)|(^keypart$)|(^conversion$)|(^functioncode$)|(^length$)|(^align$)|(^wordsep$)|(^wordno$)|(^nwords$)|(^validation$)|(^popup$)|(^conversion$)|(^checkbox$)|(^radio$)|(^horizontal$)|(^required$)|(^rowrequired$)|(^unique$)|(^nonuniquewarning$)|(^defaultvalue$)|(^validcharacters$)|(^invalidcharacters$)|(^display$)|(^lowercase$)|(^readonly$)|(^maxlength$)|(^printfunction$)|(^listfunction$)|(^filename$)|(^rows$)|(^noinsertrow$)|(^nodeleterow$)|(^allowemptyrows$)|(^copyable$)|(^link$)|(^nochangeswarning$)|(^allowduplicatefieldno$)|(^allowcursor$)|(^afterupdate$)|(^image$)/
 
 //parse the dict array to get the number of fields and number of groups etc.
 for (var dictn=0;dictn<this.dict.length;dictn++) {
 
  //delete missing array elements
  while (!this.dict[dictn]&&dictn<this.dict.length) this.dict.splice(dictn,1)
  if (!this.dict[dictn]) continue
  
  //with is a rather horrible way of defaulting object property access
  //it should replaced with a variable x and get access via x.a x.b etc
  //instead of a, b, c
  with (this.dict[dictn]) {

   //check all properties valid
   temp=this.dict[dictn]
   for (var propname in temp) {

    if (!propname.match(validpropnames))
        alert(exodusquote(propname)+' invalid dictionary property in '+name+' ignored')
    
    //check sequence A for ascending (D for descending not implemented yet)
    if (propname=='sequence'&&!['A'].exoduslocate(sequence)) {

     alert(exodusquote(propname)+' invalid value '+sequence+' in '+name+' ignored')
     sequence=''
    }
   }
     
   //check for duplicate dict ids
   if (typeof this.dict[name]!='undefined') {

    throw(new Error(0,exodusquote(name)+' duplicate dictionary id not allowed'))
   }

   //save a pointer
   this.dict[name]=dictn
     
   //standardise group numbers
   if(groupno=='S'||groupno==''||groupno=='0'||fieldno=='0') {

    groupno=0
   }
   else {

    if (groupno=='M') {

     groupno=1
    }
    else {

     if (!(isNaN(parseInt(groupno,10)))) {

      groupno=parseInt(groupno,10)
     }
     else {

      groupno=parseInt(groupno.toString().slice(1),10)
      if (isNaN(groupno)) {

       groupno=''
       var msg=groupno+' group number is not numeric for '+name
       throw(new Error(0, msg))
      }
     }
    }
    if (groupno+1>ngroups) ngroups=groupno+1 
   }

   //check S/F options
   if (type=='F'&&fieldno==='') {

    throw(new Error(0, fieldno+' field number must be numeric for '+name))
   }
   
   //data fields
   if (fieldno!='') {

    fieldno=parseInt(fieldno,10)
    if (isNaN(fieldno)) {

     throw(new Error(0, fieldno+' field number is not numeric for '+name))
    }
    if (fieldno>nfields) nfields=fieldno
    
    //save a pointer
    this.dict.fieldnos[fieldno]=this.dict[dictn]
     
   }
   
   //subfields
   if (typeof wordno!='undefined') {

    if (typeof nwords=='undefined') nwords=1
    wordno=parseInt(wordno,10)
   }
   if (typeof nwords!='undefined') {

    nwords=parseInt(nwords,10)
   }
   
   //force keys to be required ... done in formfunctions now
   //done in formfunctions
   //if (fieldno==0) required=true
   
   if (!title)
    title=name.replace(/_/gi,' ').exoduscapitalise()
   
   //save the dictitems by groupno to avoid scanning the whole dict
   //zzz this should be used in rev2obj and obj2rev
   if (!this.dict.groups[groupno]) this.dict.groups[groupno]=[]
   var groupfieldlist=this.dict.groups[groupno]
   groupfieldlist[groupfieldlist.length]=this.dict[dictn]

   //
   if (functioncode&&type!='S') {

     throw(new Error(0, 'functioncode parameter is only valid for "S" type in '+name))    
   }
      
   if (typeof validation!='undefined'&&type!='F') {

     throw(new Error(0, 'validation parameter is only valid for "F" type in '+name))
   }
    
   //function code
   if(typeof functioncode=='string'&&functioncode) {
    try {

        /* yield */ functioncode='return function *(){'+functioncode+'}'//wrap source code to create a generator
        functioncode=new Function(functioncode)
        /* yield */ functioncode=functioncode()//call function to get a generator
    } catch (e) {
        var msg='Invalid function in '+filename+' '+name+': '
        if (e.description)
            msg+=e.description
        msg+='\n\n'+functioncode
        return systemerror('exodusrecord()',msg)
    }
   }
   
   if (typeof validation=='function') {

    var globalfunctionname=filename+'_'+name+'_function'
    window[globalfunctionname]=validation
    validation=globalfunctionname+'()'
   }
 
   //display or not function code
   if (typeof display=='string'&&display) {

    //cant store functions in elements now so store expression as string
    //tempfunction=new Function(display)
    //display=tempfunction
   }

  if (typeof maxlength!='undefined'&&(typeof length=='undefined'||!length||maxlength<length)) this.dict[dictn].length=maxlength
  
  }//end with
  
 }

//READU
////////
exodusrecord.prototype.readu=function* exodusrecord_readu() {

 //gets lock if possible and reads
 return /**/ yield * this.read(true)
}

//READ
//////
exodusrecord.prototype.read=function* exodusrecord_read(withlock) {
 
 this.sessionid=''
 this.data=null
 
 //if read with empty key then return empty record
 //now empty key means get the next sequential key from the server
 //if (this.key=='')
 //{
 // this.revstr=this.defaultrevstr
 // this.response='OK'
 //}
 //else
 
 //get a record from the database or fail
 db.request=withlock?'READU':'READ'
 db.request+='\r'+this.filename+'\r'+this.key+'\r'+glocktimeoutinmins
 if (this.readenvironment)
    db.request+='\r'+this.readenvironment
 if (!(yield* db.send())
 &&(db.response.toUpperCase().slice(0,16)!="ERROR: NO RECORD")
 &&(db.response.toUpperCase().slice(0,25)!="ERROR: CANNOT LOCK RECORD")
 ) {

  this.response=db.response
  return 0
 }

 //extract sessionid, key, dataobj
 return this.postread()
 
}

//used after read (and write with a new record)
//to process the response and data into key and obj
exodusrecord.prototype.postread=function exodusrecord_postread() {

 //save the record string
 this.revstr=db.data

 //split the response for analysis
 this.response=db.response
 var response=db.response
 //cannot convert to uppercase as recordkey may be lowercase
 //.toUpperCase()
 response=response.split(' ')
 
 //get sessionid from response
 var temp=response.exoduslocate('SESSIONID')
 if (temp) {

  this.sessionid=response[temp]
  response[temp-1]=''
  response[temp]=''
 }

 //get the sequential key provided by the server
 if ((temp=response.exoduslocate('RECORDKEY'))>0) {

  this.key=response[temp].replace(/\{20\}/g,' ')
  response[temp-1]=''
  response[temp]=''
 }

 this.response=response.join(' ').exodustrim()
 //to prevent RECORDKEY message popup on opening (company) versions
 db.response=this.response
 
 //convert the schedule rev str to an object or fail
 if (!(this.rev2obj())) {

  this.response='Error: Internal failure to convert revstr to object'
  return 0
 }
 
 if (response[0]=='OK'&&this.sessionid!='') return 1
 else return 0
   
}
 
//WRITEU
////////
exodusrecord.prototype.writeu=function* exodusrecord_writeu() {

 //writes and unlocks
 return /**/ yield * this.writex(true)
}

//WRITE
////////
exodusrecord.prototype.write=function* exodusrecord_write() {

 //writes and leaves lock (zzz is this implemented in DOS LISTEN yet?)
 return /**/ yield * this.writex(false)
}

//WRITEX
////////
exodusrecord.prototype.writex=function* exodusrecord_writex(withunlock) {

 if (no(this.data)) {
  this.response="data not defined in writex"
  return 0
 }
  
 if (no(this.key)) {
  this.response="key not defined in writex"
  return 0
 }

 //convert the object to a rev string
 if (!(this.obj2rev())) {
  this.response='Error: Internal failure to convert object to revstr in writex'
  return 0
 }
 
 if (typeof gmaxstrsize == 'undefined') gmaxstrsize=60000
 if (gmaxstrsize && this.revstr.length>gmaxstrsize) {
  this.response='Error: Document is too large to save ('+this.revstr.length+' bytes/max '+maxbytes+')\rEdit some characters from the document and try again)'
  return 0
 }
 
 //put the converted rev string back into the database and unlock
 if (withunlock) db.request='WRITEU'
 else db.request='WRITE'
 db.request+='\r'+this.filename+'\r'+this.key+'\r\r'+this.sessionid
 if (this.readenvironment)
    db.request+='\r'+this.readenvironment
 if (!(yield* db.send(this.revstr))) {

  this.response=db.response
  return 0
 }
 
 //extract sessionid, key, dataobj
 gro.data=null
 if (db.data)
    return this.postread()
 
// this.response='OK'
 this.response=db.response
 return 1
  
}


 //REV2OBJ
 /////////
 exodusrecord.prototype.rev2obj=function exodusrecord_rev2obj() {

  //returns this.data and 1 ... or 0
  
  //split the fields
  var rev0=(this.key+fm+this.revstr).split(fm)
  var rev=[]// of nfields
  
  //split the values
  for (var fn=0;fn<=nfields;fn++) {

   if (fn<=rev0.length-1) {

    rev1=(rev0[fn]).split(vm)
   }
   else {

    rev1=['']
   }
   rev[fn]=rev1
  }

  //find maximum number of values in each group
  var nvalues=[]// of ngroups
  nvalues[0]=1;for (var groupn=1;groupn<ngroups;groupn++)nvalues[groupn]=0
  for (var dictn=0;dictn<this.dict.length;dictn++) {

   with (this.dict[dictn]) {

    if (groupno>0&&fieldno!='') {

     if (fieldno<rev.length&&rev[fieldno].length>nvalues[groupno]) {

      nvalues[groupno]=rev[fieldno].length
     }
    }
   }
  }

  if (1) {
   
   var datanode=new Object

   //convert the rec data into object
   for (var groupn=0;groupn<=ngroups;groupn++) {
      
    if (groupn==0) {

     group=datanode
    }
    else {

     group=datanode['group'+groupn]=[]
    }
      
    var nv=nvalues[groupn]
    for (var valuen=0;valuen<nv;valuen++) {
     
     //add a row to a group
     if (groupn==0) {

      var rec=group
     }
     else {

      var rec=group[valuen]=new Object
     }
      
     //add a field for DB_ORDINAL
     addfield(rec,'db_ordinal',valuen)
       
     //add a field for DB_TIMEDATE
     if (groupn==0) {

      addfield(rec,'db_timedate',(new Date()))
     }
     
     for (var dictn=0;dictn<this.dict.length;dictn++) {
       
      with (this.dict[dictn]) {

       if (groupno==groupn) {
              
        //add empty field for calculated fields
        if (typeof(fieldno)!='number') {

         //addfield(rec,name,'&nbsp;')
         addfield(rec,name,'')
        }

        else {
        
         var temp=''
         if (fieldno<rev.length) {

          if (groupn) temp=rev[fieldno][valuen]
          else temp=rev[fieldno].join(vm)
         }
         
         //extract keypart
         if (keypart) temp=temp.exodusfield('*',keypart)
         
         if(typeof(temp)=='undefined'||temp==null) {

          temp=""
         }
         else {

          //check for four ascii zero characters being passed. caused by bug in DOS 2.0
          //no EXODUS data should contain char(0). Usually presents as four character zeros.
          //if (temp.charCodeAt(0)==0)
          if (temp.indexOf(String.fromCharCode(0))>=0) {

           systemerror('exodusrecord_rev2obj()','char(0) in data from server in '+name+' length:'+temp.length)
           return 0
          }
         
         }
         
         //extract subfield or substring
         if (typeof wordno!='undefined') {

          if (wordsep!='') {

           temp=temp.exodusfield(wordsep,wordno,nwords)
          }
          else {

           temp=temp.substr(wordno-1,nwords).replace(trailingspaces,'')
          }
         }
         
         //convert any tm to cr
         //mac if (groupn==0) temp.replace(/TMre,'\r')
         temp=temp.replace(TMre,'\n')

         //add a field into the object
         addfield(rec,name,temp)

        }
       }
      }
     }
    }
   }
 
  }

  this.data=datanode
  
  return 1
 
function addfield(rec,fieldname,fieldtext) {

   var field
   field=rec[fieldname]=new Object
   field.text=fieldtext
  }

 }

 //OBJ2REV
 /////////
 exodusrecord.prototype.obj2rev=function exodusrecord_obj2rev() {
 
  //uses the dictionary to convert the object into a single revelation string in revstr

  //returns this.revstr and 1 or 0
  
  //var rev=new Array// of nfields
  //for (fieldno=0;fieldno<nfields;fieldno++) rev[fieldno]=""

  //copy the original string to a new array and clear only fields that are defined
  // in the dictionary. That means that any fields not in the dictionary will not be lost.
  var rev=this.revstr.split(fm)
  //make sure that there are at least nfields elements in the array
  for (var fieldno=rev.length;fieldno<nfields;fieldno++) rev[fieldno]=''
  for (var dictn=0;dictn<this.dict.length;dictn++) {

   with (this.dict[dictn]) {

    if (fieldno!=0) rev[fieldno-1]=''
   }
  }
  
  var numberdateconvs=/(^\[DATE)|(^\[NUMBER).*/
  
  //split all old fields into arrays of multivalues (will be re-joined later)
  for (var fieldno=0;fieldno<nfields;fieldno++) rev[fieldno]=rev[fieldno].split()
  
  for (var groupn=0;groupn<=ngroups;groupn++) {
 
   //locate the main group or subordinate group  
   if (groupn==0) {

    var group=this.data
    var nvalues=1
   }
   else {

    var group=this.data['group'+groupn]
    var nvalues=0
    //allow for missing groups (ngroups can be messed up during copy schedule to plan)
    if (group) nvalues=group.length
   }
   
   for (var valuen=0;valuen<nvalues;valuen++) {
    
	//get access to one record in the group
    if (groupn==0) {

     var record=group
    }
    else {

     var record=group[valuen]
    }
    
    //pass through all dicts looking for fields that belong to the current mv group
    //and copy them into the rev array
    for (var name in record) {
     
     //locate the data item
     var child=record[name]
     
     //locate dictionary id then process
     if (dictitem=this.dictitem(name)) {

      with (dictitem) {
       
       //redundant check
       if (groupno==groupn) {

        //skip duplicate fields if they are invisible
        //to avoid updating fields that have been entered
        if (dictitem.allowduplicatefieldno) {

         if (!child.element||!child.element.offsetWidth) continue
        }
        
        //if first time for the current fieldn then prepare empty multivalue array
        if (valuen==0 && fieldno>=1&&typeof rev['init'+fieldno]=='undefined') {

         var emptyvalues=new Array//of (nvalues)
         for (var i=0;i<nvalues;i++) emptyvalues[i]=''
         rev[fieldno-1]=emptyvalues
         
         //flag that this field has been inited (to prevent split fields being inited more than once)
         //nb any parts of the field that are not in the dictionary will be lost on update
         rev['init'+fieldno]=''
         
        }

        temp=child.text

        if (temp||typeof wordno!='undefined') {
         
         //convert numbers to strings
         if (typeof(temp)=='number') temp=temp+''
          
         //convert any crlf to tms
         //temp=temp.replace(/\r/gi,String.fromCharCode(251)).replace(/\n/gi,'')
         //internet explorer lines seem to be separated by crlf \r\n whereas firefox/chrome are separated by lf (\n) only
         temp=temp.replace(/\n/g,tm).replace(/\r/g,'')

         //check dates and numbers are numeric
         if (typeof conversion=='string') {

          if (conversion.match(numberdateconvs)) {

           if (!(exodusnum(temp))) {

            alert(name+' '+exodusquote(temp)+' is not a '+conversion.slice(1,-1).toLowerCase()+'.')
            return 0
           }
          }
         }

         //insert subfield or substring
         if (typeof wordno!='undefined') {

          var temp2=rev[fieldno-1][valuen]
          //if (wordsep)
          //{
          //NB noextend=true
           temp=temp2.exodusfieldstore(wordsep,wordno,nwords,temp,true)
          //}
          //else
          //{
          // var temp=temp2.substr(wordno-1,nwords)
          //}
         }
         
         //update the key or rev str
         if (fieldno<1) {

          //this.key=temp
         }
         else {

          rev[fieldno-1][valuen]=temp
         }

        }
       
       
       }
      }
     }
    }
   
   }
  
  }

  //for each field, join the multivalues into a vm separated string
  for (var fieldn=0;fieldn<nfields;fieldn++) {

   if (rev[fieldn]!="") {

    rev[fieldn]=rev[fieldn].join(vm).exodustrimr(vm)
   }
  }

  //join the fields into a single fm delimited string
  //zzz should remove excess fm and vm etc
  this.revstr=rev.join(fm).exodustrimr(fm)
  
  return 1
 
 }

}

function exodusrecord_dictitem(dictitemname)
//given a dictionary item name, returns a single dictionary item
// if not found then return ''
{
 //if present as number then return it
 var temp=this.dict.fieldnos[dictitemname]
 //doesnt work on Mac IE5.23
 //if (temp!=undefined) return temp
 if (temp) return temp

 //if present as name then return it
 if (typeof this.dict[dictitemname]!='undefined') return this.dict[this.dict[dictitemname]]

 //otherwise scan for it
 for (var dictn=0;dictn<this.dict.length && this.dict[dictn].name!=dictitemname;dictn++){}
 
 //save it and return it if found
 if (dictn<this.dict.length) {

  this.dict[dictitemname]=dictn
  return(this.dict[dictn])
 }
 //or return ''
 else {

  return('')
 }
 
}
 
function dictrec(code0,type1,fieldno2,title3,group4,keypart5,x6,conversion7,functioncode8,align9,length10) {

 if (!group4) group4=''
  
 exodusassertstring(code0,dictrec,code0)
// if (typeof code0!='string'||code0.match(/(^NUMBER$)|(^DATE$)|(^TIME$)|(^PERIOD_OF_YEAR$)|(^PERIOD_OF_TIME$)|(^TIME_OF_DAY$)/))
// {
//  alert(exodusquote(code0)+' invalid dictionary code')
// }

 //check not used elsewhere as a function
 var typeofcode0
 //try {typeofcode0=typeof eval(code0)}
 try {typeofcode0=typeof code0}
 catch(e){}
// if (typeof typeofcode0!='undefined')
 if (typeof typeofcode0=='function') {

  alert(exodusquote(code0)+' dictionary ids must be unique but this is used elsewhere as '+typeofcode0)
 }
 
 var newdictitem=new Object
 
 //field number must be blank except for real fields
 if (type1!='F') fieldno2=''

 //key fields
 if (type1=='F'&&fieldno2=='0') {

  //key fields are required
  //newdictitem.required=true
  
  //subfield must be an integer
  if (!keypart5) keypart5=1
  keypart5=parseInt(keypart5,10)
  
  //most punctuation except slash dash (hash removed 2018/3/21) are invalid
  newdictitem.invalidcharacters='`~!@$%^&*()_+={}[]|\\:;"\'<>,. #?'

  //cannot be multivalued
  group4=''
  
 }
  
 if (!title3) {

  //dict[dictn].title=name.replace(/_/gi,' ')
  title3=code0.replace(/_/gi,' ').exoduscapitalise()
 }
 
 newdictitem.name=code0
 newdictitem.type=type1
 newdictitem.fieldno=fieldno2
 newdictitem.title=title3
 newdictitem.groupno=group4
 newdictitem.keypart=keypart5
 
 if (typeof conversion7=='undefined')
  conversion7=''
 if (typeof functioncode8=='undefined')
  functioncode8=''
 if (typeof align9=='undefined')
  align9=''
 if (typeof length10=='undefined')
  length10=''
 
 newdictitem.conversion=conversion7
 newdictitem.functioncode=functioncode8
 newdictitem.align=align9
 newdictitem.length=length10
 
 /* moved to dbform.js because this function cant call yielding functions
 if (gtasks) {

  var words=code0.split('_')
  for (var wordn=0;wordn<words.length;++wordn) {

   if (!(yield* exodussecurity(gdatafilename.exodussingular()+' UPDATE '+words.slice(0,wordn+1).join(' ').exodusquote()))) {

    newdictitem.readonly=gmsg
    break;
   }
  }
 }
 */
  
 return newdictitem
 
}

function* reado(filename,key,fieldno) {

 if (no(key)) {

  this.response="Error: key not specified in reado"
  return ''
 }

 //array of keys return array of results
 if (key.push) {
    var results=[]
    for (var ii=0;ii<key.length;++ii)
        results[ii]=yield* reado(filename,key[ii],fieldno)
    return results
 }
 
 db.request='CACHE\rREAD\r'+filename+'\r'+key
 if (!(yield* db.send()))
  return ""
  
 if (!fieldno)
      return db.data
      
 var field=db.data.split(fm)[fieldno-1]
 if (!field)
    return ''
 return field
 
}

 //var group=this.data['group'+groupn]
 //var nvalues=group.length

//function to clear all values that are not copyable (ie status fields)
exodusrecord.prototype.initialise=function exodusrecord_initialise(row) {

 if (!row) row=this.data
 
 var dictid
 for (dictid in row) {

  var dictitem=this.dictitem(dictid)
  if (dictitem&&typeof dictitem.copyable!='undefined'&&!dictitem.copyable) {

   row[dictid].text=''
  }
 }
 
 return row
 
}
