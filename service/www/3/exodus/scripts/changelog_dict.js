//Copyright NEOSYS All Rights Reserved.

async function changelog_pop_number(many) {

 //no way to return say the last 1000 records, so we request to return all
 return await exodusfilepopup('CHANGELOG',[['NUMBER'],['DATE'],['KEYWORDS'],['TEXT']],0,'BY-DSND NUMBER',many,'',0)
}

async function changelog_list() {

 // Same folder as changelog.htm (not general/ — that path 404s)
 await windowopen('whatsnew.htm')
 return true
}

async function dict_CHANGELOG() {

 //returns an array representing the currency dictionary
 
 var dict=[]//of dictrecs
 var din=-1
 
 //single valued fields
 
 di=dict[++din]=dictrec('CHANGELOG_NUMBER','F',0)
 //di.required=true
 exodus_dict_number(di,2)
 di.validcharacters='1234567890.'
 di.defaultvalue=exodusdate().exodusquote()
 di.popup='await changelog_pop_number(true)'
 di.listfunction='await changelog_list()'
 
 di=dict[++din]=dictrec('CHANGELOG_DATE','F',1)
 di.required=true
 exodus_dict_date(di)
 di.defaultvalue='TODAY'
 
 di=dict[++din]=dictrec('KEYWORD','F',2,'',1)
 //causes popup of/validation versus previous entries
 exodus_dict_index(di)
 di.rowrequired=true
 di.lowercase=true
 
 di=dict[++din]=dictrec('CHANGELOG_TEXT','F',3)
 di.required=true
 exodus_dict_text(di)
 di.rows=20
 di.length=100
 
 di=dict[++din]=dictrec('DATE_TIME','F',4)
 exodus_dict_datetimeupdated(di)
 
 di=dict[++din]=dictrec('DISTRIBUTION','F',5)
 di.checkbox='User;User:Support;Support:Developer;Developer'
 di.defaultvalue='"User:Support:Developer"'
 di.horizontal=true
 //causes popup of/validation versus previous entries
 //exodus_dict_index(di)
 //di.rowrequired=true
 //di.lowercase=true
 
 return dict
 
}