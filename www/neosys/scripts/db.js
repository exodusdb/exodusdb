//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

var glocktimeoutinmins=10
var trailingspaces=/\s*$/g

function neosys_dict_dow(di,many)
{
 if (many)
 {
  di.checkbox='1;Mon:2;Tue:3;Wed:4;Thu:5;Fri:6;Sat:7;Sun'
  di.horizontal=true 
 }
 else
  di.conversion='1;Monday:2;Tuesday:3;Wednesday:4;Thursday:5;Friday:6;Saturday:7;Sunday'
 
}

function neosys_dict_url(di,protocol)
{
 neosys_dict_text(di)
 di.validation='neosys_val_url("'+protocol+'")'
}

function neosys_val_url(protocol)
{
 //protocol should be http or https
 if (protocol=='http'||protocol=='https')
 {
  var regexp=new RegExp(protocol+'\:\/\/.','i')
  if (!gvalue.match(regexp))
   return neosysinvalid('Should start with '+protocol+'://')
  return true
 }
  
 var regexp=new RegExp('((http)|(https))'+'\:\/\/.','i')
 if (!gvalue.match(regexp))
   return neosysinvalid('Should start with http:// or https://')
   
 return true
}

function neosys_dict_ipno(di,many,wildcard)
{
 neosys_dict_text(di)
 var tt='.0123456789'
 if (many) tt+=' ;,'
 if (wildcard) tt+='*'
 di.validcharacters=tt
 //TODO validate true ip number patterns
}

function neosys_dict_emailaddress(di,sepchar)
{
 var length=30
 if (!sepchar)
  sepchar=''
 if (sepchar&&!di.groupno)
  length=60
 if (!sepchar.slice(0,1).match(/[\'\"]/))
  sepchar='"'+sepchar+'"'
 neosys_dict_text(di,length)
 di.validation='neosys_val_emailaddress('+sepchar+')'
}

function neosys_val_diskdrive()
{
 if (!gvalue) return true

 gvalue=gvalue.slice(0,1).toUpperCase()
 //if (!gvalue.match(/^[ABCDEFGHIJKLMNOPQRSTUVWXYZ](:)?$/))
  //return neosysinvalid('Please enter a letter A-Z')

 if (gvalue.length==1&&gvalue!='0') gvalue+=':'

 return true
}

function neosys_dict_diskdrive(di)
{
 //di.conversion+='C:D:E:F:G:H:I:J:K:L:M:N:O:P:Q:R:S:T:U:V:W:X:Y:Z'
 di.validcharacters='CDEFGHIJKLMNOPQRSTUVWXYZ:'
 di.validation='neosys_val_diskdrive()'
 di.length=1
 di.maxlength=1
}

function neosys_val_emailaddress(sepchar)
{
 if (!gvalue) return true
 //Description: Most email validation regexps are outdated and ignore the fact that domain names can contain any foreign character these days,
 // as well as the fact that anything before @ is acceptable.
 // The only roman alphabet restriction is in the TLD, which for a long time has been more than 2 or 3 chars (.museum, .aero, .info).
 // The only dot restriction is that . cannot be placed directly after @. This pattern captures any valid, reallife email adress. 
 //Matches: whatever@somewhere.museum|||foreignchars@myforeigncharsdomain.nu|||me+mysomething@mydomain.com 
 //Non-Matches: a@b.c|||me@.my.com|||a@b.comFOREIGNCHAR 
 // if (!gvalue.match(/[-_.abcdefghijklmnopqrstuvwxyz1234567890]+@[-.abcdefghijklmnopqrsptuvwxyz1234567890]+\.[-_.abcdefghijklmnopqrstuvwxyz]+/i))
 var values
 if (sepchar)
 {
  gvalue=gvalue.replace(/[, ;]+/g,sepchar).replace(/^;+/,'').replace(/;+$/,'')
  values=gvalue.split(';')
 }
 else
  values=[gvalue]
 for (var ii=0;ii<values.length;ii++)
 {
  var value=values[ii]
  if (value&&!value.match(/^[\w.-]+@[^\W_][\w.-]*\.[\w]{2,}$/))
  {
   var msg=value+'\rEmail address must be in the format xxxxxx@xxxxxx.xxx'
   if (sepchar)
    msg+=' '+sepchar+'...'
   return neosysinvalid(msg)
  }
 }
 gvalue=values.join(sepchar)
 return true
}

/////////////////
//INDEX FUNCTIONS
/////////////////

function neosys_dict_index(di,filename,fieldname,many,allownew,warnnew)
{
 if (!filename) filename=''
 if (!fieldname) fieldname=''
 if (!many) many=di.groupno
 if (!allownew) allownew=true
 if (!warnnew) warnnew=true

 if (!di.validation) di.validation='form_val_index('+filename.neosysquote()+','+fieldname.neosysquote()+','+allownew+','+warnnew+')'

 if (!di.popup) di.popup='form_pop_index('+filename.neosysquote()+','+fieldname.neosysquote()+','+many+')'
 
}

////////////////////
//DATABASE FUNCTIONS
////////////////////

function neosys_dict_year(dicti,from,to,defaultyear)
{
 if (!from) from=0
 if (!to) to=0
 neosysassertobject(dicti,'neosys_dict_year','dicti')
 dicti.conversion='[NUMBER,0]'
 dicti.align='R'
 dicti.length=4
 var curryear=neosysdate().neosysoconv('[DATE,YEAR]')
 var years=''
 for (var year=curryear+to;year>=curryear+from;year--)
 {
  if (years) years+=':'
  years+=year+';'+year
 }
 dicti.conversion=years
 if (typeof defaultyear!='undefined') dicti.defaultvalue='"'+(curryear+defaultyear)+'"'
}

function neosys_dict_datetimeupdated(di)
{
 neosys_dict_datetime(di)
 di.readonly=true
 di.copyable=false
 di.align='R'
 di.lowercase=true
}

function neosys_dict_datetime(di,params)
{
 if (!params) params=''
 di.conversion='[DATE_TIME,'+params+']'
 di.align='R'
 di.lowercase=true
}

function neosys_dict_time(di,mode,otherid,params)
{
 
 if (!di.conversion)
 {
  if (!params) params=''
  else params=','+params
  di.conversion='[TIME'+params+']'
 }
 
 di.align='R'
 di.length=5 
 if (mode&&otherid)
 {
  di.defaultvalue='neosys_def_time("'+mode+'","'+otherid+'")'
  di.validation='neosys_val_time("'+mode+'","'+otherid+'")'
 }
 
}

function neosys_def_time(mode,othertimeid)
{
 if (!othertimeid) return ''
 
 othertime=gds.getx(othertimeid)
 if (typeof othertime=='object') othertime=othertime[0]

 return othertime
}

function neosys_val_time(mode,otherid)
{
 
 if (!gvalue||!mode||!otherid) return true
 
 var time=+gvalue
 var oldtime=+goldvalue
 
 var othertime=gds.getx(otherid)
 if (typeof othertime=='object') othertime=othertime[0]
 var othertime2=+othertime

 //prevent toperiod less than fromperiod
 if (mode=='FROM'&&time>othertime2) othertime2=time
 if (mode=='UPTO'&&time<othertime2) othertime2=time
    
 //if change fromperiod and toperiod blank or same as from goldvalue
 if (mode=='FROM'&&(othertime==''||othertime==oldtime)) othertime2=time

 //update the otherperiod
 if (othertime2!=(+othertime)) gds.setx(otherid,grecn,othertime.toString())

 //gvalue=time
  
 return true
 
}

// neosys_dict_period(di,'FROM','PERIOD_TO','[FINANCIAL_PERIOD]')
function neosys_dict_period(di,mode,otherperiodid,conversion)
{
 if (!mode) mode=''
 if (!otherperiodid) otherperiodid=''
 if (!conversion) conversion=''
 neosysassertobject(di,'neosys_dict_period','di')
 di.conversion='[PERIOD_OF_YEAR]'
 di.align='R'
 di.length=5
 if (di.type=='F') di.validation='neosys_val_period("'+mode+'","'+otherperiodid+'")'
}

function neosys_dict_year_period(di,mode)
{
 if (!mode) mode=''
 neosysassertobject(di,'neosys_dict_yearperiod','di')
 di.conversion='[YEAR_PERIOD,'+mode+']'
 di.align='R'
 di.length=5
}

function neosys_val_period(mode,otherperiodid)
{

 if (!gvalue) return true
 
 var value=gvalue.split('/')
 
 var oldvalue=goldvalue.split('/')
 
 if (mode&&otherperiodid)
 {
 
  otherperiod=gds.getx(otherperiodid)
  if (typeof otherperiod=='object') otherperiod=otherperiod[0]
  var othervalue=otherperiod.split('/')
  
  //get year from the other period
  if (value.length==1||!value[1])
  {
  
   //missing year is taken from other period or current period
   if (otherperiod)
   {
    value[1]=othervalue[1]
    if (mode=='FROM')
    {
     //make sure to month is not less than from month
     if (value[0]>othervalue[0]) othervalue[0]=value[0]
    }
    else
    {
     //make sure from month is not less than to month
     if (value[0]<othervalue[0]) othervalue[0]=value[0]
    }
   }
   else
   {
    value[1]=gperiod.split('/')[1]
   }
  }

  var otherjulianperiod=neosysjulianperiod(othervalue.join('/'),gmaxperiodno)
  var julianperiod=neosysjulianperiod(value.join('/'),gmaxperiodno)

  //prevent toperiod less than fromperiod
  if (mode=='FROM'&julianperiod>otherjulianperiod) othervalue=value
  if (mode=='UPTO'&julianperiod<otherjulianperiod) othervalue=value
    
  //if change fromperiod and toperiod blank or same as from goldvalue
  if (mode=='FROM'&&(otherperiod==''||otherperiod==goldvalue)) othervalue=value

  //update the otherperiod
  if (otherperiod!=othervalue.join('/')) gds.setx(otherperiodid,grecn,othervalue.join('/'))

  gvalue=value.join('/')
  
 }
 
 return true
 
}

function neosys_dict_yesno(dicti,defaultvalue,type)
{
 neosysassertobject(dicti,'neosys_dict_yesno','dicti')
 if (!type) type='dropdown' 
 if (typeof defaultvalue=='undefined') defaultvalue='"Y"'
 defaultvalue=defaultvalue.toString()
 if (type=='dropdown') dicti.conversion='Y;Yes:N;No'
 else if (type=='radio'||type=='hradio')
 {
  if (defaultvalue=='1'||defaultvalue=='0') dicti.radio='1;Yes:0;No'
  else dicti.radio='Y;Yes:N;No'
  if (type=='hradio') dicti.horizontal=true
 }
 
 if (typeof defaultvalue!='undefined') dicti.defaultvalue=defaultvalue
 
}

function neosys_dict_date(dicti,params)
{
 if (typeof params=='undefined') params=''
 neosysassertobject(dicti,'neosys_dict_date','dicti')
 dicti.conversion='[DATE,'+params+']'
 //dicti.align='R' causes funny columns data entry (where?)
 dicti.align='R'//put back so reports.htm shows neatly
 dicti.length=10
 dicti.popup='form_pop_calendar()'
 if (!gcalendarscript) gcalendarscript=1
 if (params) dicti.lowercase=true
}

function neosys_dict_number(dicti,params,minimum,maximum)
{
 if (typeof params=='undefined') params=''
 neosysassertobject(dicti,'neosys_dict_number','dicti')
 
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
 dicti.align='R'
}

function neosys_dict_text(dicti,size,rows)
{
 neosysassertobject(dicti,'neosys_dict_text','dicti')
 if (!dicti.readonly)
 {
  if (!size) size=30
  if (!rows) rows=1
 }
 dicti.align='T'
 if (typeof size!='undefined')
  dicti.length=size
 if (typeof rows!='undefined')
  dicti.rows=rows
 if (typeof dicti.lowercase=='undefined')
  dicti.lowercase=true
}

function neosysrecord(dictarray,filename)
{
 this.dict=dictarray
 this.filename=filename

 this.key=''
 this.revstr=''
 this.defaultrevstr=''
 this.data=null
 this.readenvironment=''
 this.dictitem=neosysrecord_dictitem
 this.dict.groups=[]
 this.dict.fieldandwordns=[]
 this.dict.fieldnos=new Object//not [] otherwise dictitem() finds array properties
   
 var response
 var sessionid
 var ngroups=0
 var nfields=0

 var validpropnames = /(^sequence$)|(^openfunction$)|(^dropdown$)|(^test$)|(^name$)|(^type$)|(^fieldno$)|(^title$)|(^groupno$)|(^keypart$)|(^conversion$)|(^functioncode$)|(^length$)|(^align$)|(^wordsep$)|(^wordno$)|(^nwords$)|(^validation$)|(^popup$)|(^conversion$)|(^checkbox$)|(^radio$)|(^horizontal$)|(^required$)|(^rowrequired$)|(^unique$)|(^nonuniquewarning$)|(^defaultvalue$)|(^validcharacters$)|(^invalidcharacters$)|(^display$)|(^lowercase$)|(^readonly$)|(^maxlength$)|(^printfunction$)|(^listfunction$)|(^filename$)|(^rows$)|(^noinsertrow$)|(^nodeleterow$)|(^allowemptyrows$)|(^copyable$)|(^link$)|(^nochangeswarning$)|(^allowduplicatefieldno$)|(^allowcursor$)|(^afterupdate$)/
 
 //parse the dict array to get the number of fields and number of groups etc.
 for (var dictn=0;dictn<this.dict.length;dictn++)
 {
 
  //delete missing array elements
  while (!this.dict[dictn]&&dictn<this.dict.length) this.dict.splice(dictn,1)
  if (!this.dict[dictn]) continue
    
  with (this.dict[dictn])
  {

   //check all properties valid
   temp=this.dict[dictn]
   for (var propname in temp)
   {
    if (!propname.match(validpropnames)) alert(neosysquote(propname)+' invalid dictionary property in '+name+' ignored')
    
    //check sequence A for ascending (D for descending not implemented yet)
    if (propname=='sequence'&&!['A'].neosyslocate(sequence))
    {
     alert(neosysquote(propname)+' invalid value '+sequence+' in '+name+' ignored')
     sequence=''
    }
   }
     
   //check for duplicate dict ids
   if (typeof this.dict[name]!='undefined')
   {
    throw(new Error(0,neosysquote(name)+' duplicate dictionary id not allowed'))
   }

   //save a pointer
   this.dict[name]=dictn
     
   //standardise group numbers
   if(groupno=='S'||groupno==''||groupno=='0'||fieldno=='0')
   {
    groupno=0
   }
   else
   {
    if (groupno=='M')
    {
     groupno=1
    }
    else
    {
     if (!isNaN(parseInt(groupno,10)))
     {
      groupno=parseInt(groupno,10)
     }
     else
     {
      groupno=parseInt(groupno.toString().slice(1),10)
      if (isNaN(groupno))
      {
       groupno=''
       var msg=groupno+' group number is not numeric for '+name
       throw(new Error(0, msg))
      }
     }
    }
    if (groupno+1>ngroups) ngroups=groupno+1 
   }

   //check S/F options
   if (type=='F'&&fieldno==='')
   {
    throw(new Error(0, fieldno+' field number must be numeric for '+name))
   }
   
   //data fields
   if (fieldno!='')
   {
    fieldno=parseInt(fieldno,10)
    if (isNaN(fieldno))
    {
     throw(new Error(0, fieldno+' field number is not numeric for '+name))
    }
    if (fieldno>nfields) nfields=fieldno
    
    //save a pointer
    this.dict.fieldnos[fieldno]=this.dict[dictn]
     
   }
   
   //subfields
   if (typeof wordno!='undefined')
   {
    if (typeof nwords=='undefined') nwords=1
    wordno=parseInt(wordno,10)
   }
   if (typeof nwords!='undefined')
   {
    nwords=parseInt(nwords,10)
   }
   
   //force keys to be required ... done in formfunctions now
   //done in formfunctions
   //if (fieldno==0) required=true
   
   if (!title)
    title=name.replace(/_/gi,' ').neosyscapitalise()
   
   //save the dictitems by groupno to avoid scanning the whole dict
   //zzz this should be used in rev2obj and obj2rev
   if (!this.dict.groups[groupno]) this.dict.groups[groupno]=[]
   var groupfieldlist=this.dict.groups[groupno]
   groupfieldlist[groupfieldlist.length]=this.dict[dictn]

   //
   if (functioncode&&type!='S')
   {
     throw(new Error(0, 'functioncode parameter is only valid for "S" type in '+name))    
   }
      
   if (typeof validation!='undefined'&&type!='F')
   {
     throw(new Error(0, 'validation parameter is only valid for "F" type in '+name))
   }
      
   //function code
   if(typeof functioncode=='string'&&functioncode)
   {
    tempfunction=new Function(functioncode)
    functioncode=tempfunction
   }
   
   if (typeof validation=='function')
   {
    var globalfunctionname=filename+'_'+name+'_function'
    window[globalfunctionname]=validation
    validation=globalfunctionname+'()'
   }
 
   //display or not function code
   if (typeof display=='string'&&display)
   {
    tempfunction=new Function(display)
    display=tempfunction
   }

  if (typeof maxlength!='undefined'&&(typeof length=='undefined'||!length||maxlength<length)) this.dict[dictn].length=maxlength
  
  }//end with
  
 }

//READU
////////
neosysrecord.prototype.readu=function neosysrecord_readu()
{
 //gets lock if possible and reads
 return this.read(true)
}

//READ
//////
neosysrecord.prototype.read=function neosysrecord_read(withlock)
{
 
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
 db.request+='\r'+this.filename+'\r'+this.key+'\r'+glocktimeoutinmins+'\r'+this.readenvironment
 if (!db.send()
 &&db.response.toUpperCase().slice(0,16)!="ERROR: NO RECORD"
 &&db.response.toUpperCase().slice(0,25)!="ERROR: CANNOT LOCK RECORD")
 {
  this.response=db.response
  return 0
 }

 //extract sessionid, key, dataobj
 return this.postread()
 
}

//used after read (and write with a new record)
//to process the response and data into key and obj
neosysrecord.prototype.postread=function neosysrecord_postread()
{

 //save the record string
 this.revstr=db.data

 //split the response for analysis
 this.response=db.response
 var response=db.response
 //cannot convert to uppercase as recordkey may be lowercase
 //.toUpperCase()
 response=response.split(' ')
 
 //get sessionid from response
 var temp=response.neosyslocate('SESSIONID')
 if (temp)
 {
  this.sessionid=response[temp]
  response[temp-1]=''
  response[temp]=''
 }

 //get the sequential key provided by the server
 if ((temp=response.neosyslocate('RECORDKEY'))>0)
 {
  this.key=response[temp].replace(/\{20\}/g,' ')
  response[temp-1]=''
  response[temp]=''
 }

 this.response=response.join(' ').neosystrim()
 //to prevent RECORDKEY message popup on opening (company) versions
 db.response=this.response
 
 //convert the schedule rev str to an object or fail
 if (!this.rev2obj())
 {
  this.response='Error: Internal failure to convert revstr to object'
  return 0
 }
 
 if (response[0]=='OK'&&this.sessionid!='') return 1
 else return 0
   
}
 
//WRITEU
////////
neosysrecord.prototype.writeu=function neosysrecord_writeu()
{
 //writes and unlocks
 return this.writex(true)
}

//WRITE
////////
neosysrecord.prototype.write=function neosysrecord_write()
{
 //writes and leaves lock (zzz is this implemented in AREV LISTEN yet?)
 return this.writex(false)
}

//WRITEX
////////
neosysrecord.prototype.writex=function neosysrecord_writex(withunlock)
{
 if (no(this.data))
 {
  this.response="data not defined in writex"
  return 0
 }
  
 if (no(this.key))
 {
  this.response="key not defined in writex"
  return 0
 }

 //convert the object to a rev string
 if (!(this.obj2rev()))
 {
  this.response='Error: Internal failure to convert object to revstr in writex'
  return 0
 }
 
 var maxbytes=60000
 if (this.revstr.length>maxbytes)
 {
  this.response='Error: Document is too large to save ('+this.revstr.length+' bytes/max '+maxbytes+')\rEdit some characters from the document and try again)'
  return 0
 }
 
 //put the converted rev string back into the database and unlock
 if (withunlock) db.request='WRITEU'
 else db.request='WRITE'
 db.request+='\r'+this.filename+'\r'+this.key+'\r\r'+this.sessionid
 if (!db.send(this.revstr))
 {
  this.response=db.response
  return 0
 }
 
 //extract sessionid, key, dataobj
 gro.data=null
 if (db.data) return this.postread()
 
// this.response='OK'
 this.response=db.response
 return 1
  
}


 //REV2OBJ
 /////////
 neosysrecord.prototype.rev2obj=function neosysrecord_rev2obj()
 {

  //returns this.data and 1 ... or 0
  
  //split the fields
  var rev0=(this.key+fm+this.revstr).split(fm)
  var rev=[]// of nfields
  
  //split the values
  for (var fn=0;fn<=nfields;fn++)
  {
   if (fn<=rev0.length-1)
   {
    rev1=(rev0[fn]).split(vm)
   }
   else
   {
    rev1=['']
   }
   rev[fn]=rev1
  }

  //find maximum number of values in each group
  var nvalues=[]// of ngroups
  nvalues[0]=1;for (var groupn=1;groupn<ngroups;groupn++)nvalues[groupn]=0
  for (var dictn=0;dictn<this.dict.length;dictn++)
  {
   with (this.dict[dictn])
   {
    if (groupno>0&&fieldno!='')
    {
     if (fieldno<rev.length&&rev[fieldno].length>nvalues[groupno])
     {
      nvalues[groupno]=rev[fieldno].length
     }
    }
   }
  }

  if (1)
  {
   
   var datanode=new Object

   //convert the rec data into object
   for (var groupn=0;groupn<=ngroups;groupn++)
   {
      
    if (groupn==0)
    {
     group=datanode
    }
    else
    {
     group=datanode['group'+groupn]=[]
    }
      
    var nv=nvalues[groupn]
    for (var valuen=0;valuen<nv;valuen++)
    {  
     
     //add a row to a group
     if (groupn==0)
     {
      var rec=group
     }
     else
     {
      var rec=group[valuen]=new Object
     }
      
     //add a field for DB_ORDINAL
     addfield(rec,'db_ordinal',valuen)
       
     //add a field for DB_TIMEDATE
     if (groupn==0)
     {
      addfield(rec,'db_timedate',(new Date))
     }
     
     for (var dictn=0;dictn<this.dict.length;dictn++)
     {
       
      with (this.dict[dictn])
      {
       if (groupno==groupn)
       {
              
        //add empty field for calculated fields
        if (typeof(fieldno)!='number')
        {
         //addfield(rec,name,'&nbsp;')
         addfield(rec,name,'')
        }

        else
        {
        
         var temp=''
         if (fieldno<rev.length)
         {
          if (groupn) temp=rev[fieldno][valuen]
          else temp=rev[fieldno].join(vm)
         }
         
         //extract keypart
         if (keypart) temp=temp.neosysfield('*',keypart)
         
         if(typeof(temp)=='undefined'||temp==null)
         {
          temp=""
         }
         else
         {

          //check for four ascii zero characters being passed
          //caused by bug in AREV 2.0
          if (temp.charCodeAt(0)==0)
          {
           systemerror('neosysrecord_rev2obj()',temp.length+' char(0) in data from server')
           return 0
          }
         
         }
         
         //extract subfield or substring
         if (typeof wordno!='undefined')
         {
          if (wordsep!='')
          {
           temp=temp.neosysfield(wordsep,wordno,nwords)
          }
          else
          {
           temp=temp.substr(wordno-1,nwords).replace(trailingspaces,'')
          }
         }
         
         //convert any tm to cr
         //mac if (groupn==0) temp.replace(/TMre,'\r')
         temp=temp.replace(TMre,'\r')

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
 
  function addfield(rec,fieldname,fieldtext)
  {
   var field
   field=rec[fieldname]=new Object
   field.text=fieldtext
  }

 }

 //OBJ2REV
 /////////
 neosysrecord.prototype.obj2rev=function neosysrecord_obj2rev()
 {
 
  //uses the dictionary to convert the object into a single revelation string in revstr

  //returns this.revstr and 1 or 0
  
  //var rev=new Array// of nfields
  //for (fieldno=0;fieldno<nfields;fieldno++) rev[fieldno]=""

  //copy the original string to a new array and clear only fields that are defined
  // in the dictionary. That means that any fields not in the dictionary will not be lost.
  var rev=this.revstr.split(fm)
  //make sure that there are at least nfields elements in the array
  for (var fieldno=rev.length;fieldno<nfields;fieldno++) rev[fieldno]=''
  for (var dictn=0;dictn<this.dict.length;dictn++)
  {
   with (this.dict[dictn])
   {
    if (fieldno!=0) rev[fieldno-1]=''
   }
  }
  
  var numberdateconvs=/(^\[DATE)|(^\[NUMBER).*/
  
  //split all old fields into arrays of multivalues (will be re-joined later)
  for (var fieldno=0;fieldno<nfields;fieldno++) rev[fieldno]=rev[fieldno].split()
  
  for (var groupn=0;groupn<=ngroups;groupn++)
  {
 
   //locate the main group or subordinate group  
   if (groupn==0)
   {
    var group=this.data
    var nvalues=1
   }
   else
   {
    var group=this.data['group'+groupn]
    var nvalues=0
    //allow for missing groups (ngroups can be messed up during copy schedule to plan)
    if (group) nvalues=group.length
   }
   
   for (var valuen=0;valuen<nvalues;valuen++)
   {
    
	//get access to one record in the group
    if (groupn==0)
    {
     var record=group
    }
    else
    {
     var record=group[valuen]
    }
    
    //pass through all dicts looking for fields that belong to the current mv group
    //and copy them into the rev array
    for (var name in record)
    {
     
     //locate the data item
     var child=record[name]
     
     //locate dictionary id then process
     if (dictitem=this.dictitem(name))
     {
      with (dictitem)
      {
       
       //redundant check
       if (groupno==groupn)
       {

        //skip duplicate fields if they are invisible
        //to avoid updating fields that have been entered
        if (dictitem.allowduplicatefieldno)
        {
         if (!child.element||!child.element.offsetWidth) continue
        }
        
        //if first time for the current fieldn then prepare empty multivalue array
        if (valuen==0 && fieldno>=1&&typeof rev['init'+fieldno]=='undefined')
        {
         var emptyvalues=new Array//of (nvalues)
         for (var i=0;i<nvalues;i++) emptyvalues[i]=''
         rev[fieldno-1]=emptyvalues
         
         //flag that this field has been inited (to prevent split fields being inited more than once)
         //nb any parts of the field that are not in the dictionary will be lost on update
         rev['init'+fieldno]=''
         
        }

        temp=child.text

        if (temp||typeof wordno!='undefined')
        {
         
         //convert numbers to strings
         if (typeof(temp)=='number') temp=temp+''
          
         //convert any crlf to tms
         //temp=temp.replace(/\r/gi,String.fromCharCode(251)).replace(/\n/gi,'')
         temp=temp.replace(/\r/g,tm).replace(/\n/g,'')

         //check dates and numbers are numeric
         if (typeof conversion=='string')
         {
          if (conversion.match(numberdateconvs))
          {
           if (!neosysnum(temp))
           {
            alert(name+' '+neosysquote(temp)+' is not a '+conversion.slice(1,-1).toLowerCase()+'.')
            return 0
           }
          }
         }

         //insert subfield or substring
         if (typeof wordno!='undefined')
         {
          var temp2=rev[fieldno-1][valuen]
          //if (wordsep)
          //{
          //NB noextend=true
           temp=temp2.neosysfieldstore(wordsep,wordno,nwords,temp,true)
          //}
          //else
          //{
          // var temp=temp2.substr(wordno-1,nwords)
          //}
         }
         
         //update the key or rev str
         if (fieldno<1)
         {
          //this.key=temp
         }
         else
         {

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
  for (var fieldn=0;fieldn<nfields;fieldn++)
  {
   if (rev[fieldn]!="")
   {
    rev[fieldn]=rev[fieldn].join(vm).neosystrimr(vm)
   }
  }

  //join the fields into a single fm delimited string
  //zzz should remove excess fm and vm etc
  this.revstr=rev.join(fm).neosystrimr(fm)
  
  return 1
 
 }

}

function neosysrecord_dictitem(dictitemname)
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
 if (dictn<this.dict.length)
 {
  this.dict[dictitemname]=dictn
  return(this.dict[dictn])
 }
 //or return ''
 else
 {
  return('')
 }
 
}
 
function dictrec(code0,type1,fieldno2,title3,group4,keypart5,x6,conversion7,functioncode8,align9,length10)
{

 if (!group4) group4=''
  
 neosysassertstring(code0,dictrec,code0)
// if (typeof code0!='string'||code0.match(/(^NUMBER$)|(^DATE$)|(^TIME$)|(^PERIOD_OF_YEAR$)|(^PERIOD_OF_TIME$)|(^TIME_OF_DAY$)/))
// {
//  alert(neosysquote(code0)+' invalid dictionary code')
// }

 //check not used elsewhere as a function
 var typeofcode0
 //try {typeofcode0=typeof eval(code0)}
 try {typeofcode0=typeof code0}
 catch(e){}
// if (typeof typeofcode0!='undefined')
 if (typeof typeofcode0=='function')
 {
  alert(neosysquote(code0)+' dictionary ids must be unique but this is used elsewhere as '+typeofcode0)
 }
 
 var newdictitem=new Object
 
 //field number must be blank except for real fields
 if (type1!='F') fieldno2=''

 //key fields
 if (type1=='F'&&fieldno2=='0')
 {
  //key fields are required
  //newdictitem.required=true
  
  //subfield must be an integer
  if (!keypart5) keypart5=1
  keypart5=parseInt(keypart5,10)
  
  //most punctuation except slash dash hash are invalid
  newdictitem.invalidcharacters='`~!@$%^&*()_+={}[]|\\:;"\'<>,. '

  //cannot be multivalued
  group4=''
  
 }
  
 if (!title3)
 {
  //dict[dictn].title=name.replace(/_/gi,' ')
  title3=code0.replace(/_/gi,' ').neosyscapitalise()
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
 
 if (gtasks)
 {
  var words=code0.split('_')
  for (var wordn=0;wordn<words.length;++wordn)
  {
   if (!neosyssecurity(gdatafilename.neosyssingular()+' UPDATE '+words.slice(0,wordn+1).join(' ').neosysquote()))
   {
    newdictitem.readonly=gmsg
    break;
   }
  }
 }
 
 return newdictitem
 
}

//not used anywhere at the moment
function reado(filename,key)
{

 if (no(key))
 {
  this.response="Error: key not specified in reado"
  return(0)
 }

 db.request='CACHE\rREAD\r'+filename+'\r'+key
 if (db.send())
  return db.data
 else
  return ""

}

 //var group=this.data['group'+groupn]
 //var nvalues=group.length

//function to clear all values that are not copyable (ie status fields)
neosysrecord.prototype.initialise=function neosysrecord_initialise(row)
{

 if (!row) row=this.data
 
 var dictid
 for (dictid in row)
 {
  var dictitem=this.dictitem(dictid)
  if (dictitem&&typeof dictitem.copyable!='undefined'&&!dictitem.copyable)
  {
   row[dictid].text=''
  }
 }
 
 return row
 
}
