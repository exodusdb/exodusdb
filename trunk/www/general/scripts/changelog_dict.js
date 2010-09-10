//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

function changelog_pop_number(many)
{
 return neosysfilepopup('CHANGELOG',[['NUMBER'],['DATE'],['KEYWORDS'],['TEXT']],0,'BY-DSND NUMBER',many)
}

function changelog_list()
{
 //openwindow('EXECUTE\rGENERAL\rLISTCHANGELOG')
 windowopen('../general/whatsnew.htm')
 return true
}

function dict_CHANGELOG()
{

 //returns an array representing the currency dictionary
 
 var dict=[]//of dictrecs
 var din=-1
 
 //single valued fields
 
 di=dict[++din]=dictrec('CHANGELOG_NUMBER','F',0)
 //di.required=true
 neosys_dict_number(di,2)
 di.validcharacters='1234567890.'
 di.defaultvalue=neosysdate().neosysquote()
 di.popup='changelog_pop_number(true)'
 di.listfunction='changelog_list()'
 
 di=dict[++din]=dictrec('CHANGELOG_DATE','F',1)
 di.required=true
 neosys_dict_date(di)
 di.defaultvalue='TODAY'
 
 di=dict[++din]=dictrec('KEYWORD','F',2,'',1)
 //causes popup of/validation versus previous entries
 neosys_dict_index(di)
 di.rowrequired=true
 di.lowercase=true
 
 di=dict[++din]=dictrec('CHANGELOG_TEXT','F',3)
 di.required=true
 neosys_dict_text(di)
 di.rows=20
 di.length=100
 
 di=dict[++din]=dictrec('DATE_TIME','F',4)
 neosys_dict_datetimeupdated(di)
 
 di=dict[++din]=dictrec('DISTRIBUTION','F',5)
 di.checkbox='User;User:Support;Support:Developer;Developer'
 di.defaultvalue='"User:Support:Developer"'
 di.horizontal=true
 //causes popup of/validation versus previous entries
 //neosys_dict_index(di)
 //di.rowrequired=true
 //di.lowercase=true
 
 return dict
 
}