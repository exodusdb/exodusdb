//Copyright© 2000 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

function company_val_closedperiod()
{

 //nb using gorigvalue (from the old record) instead of goldvalue (from the old field)
 //allows them to change their minds and change/revert before saving

 //always allow closing from nothing or returning to original value
 if (!gorigvalue||gvalue==gorigvalue) return true
 
 //removing the closed period is the same as reopening all years
 var periodsyears='YEARS'
 if (gvalue)
 {

  var oldperiod=gorigvalue.split('/')
  var newperiod=gvalue.split('/')
  
  //closing years is always allowed at the moment
  if (Number(newperiod[1])>Number(oldperiod[1])) return true
  
  //if years are the same and the period has been reduced then check for re-opening periods
  if (newperiod[1]==oldperiod[1]&&Number(newperiod[0])<Number(oldperiod[0])) periodsyears='PERIODS'

 }
 
 if (!neosyssecurity('LEDGER RE-OPEN CLOSED '+periodsyears)) return neosysinvalid(gmsg)

 return true
 
}

function dict_COMPANIES()
{

 //returns an array representing the currency dictionary
 
 var dict=[]//of dictrecs
 var din=-1
 
 //single valued fields
 
 di=dict[++din]=dictrec('COMPANY_CODE','F',0)
 general_dict_companycode(di,true)
 di.required=true
 di.validation=null
 //di.defaultvalue='gcompanycode'
  
 di=dict[++din]=dictrec('COMPANY_NAME','F',1)
 general_dict_companyname(di)
 di.required=true
 
 di=dict[++din]=dictrec('CURRENT_PERIOD','F',2)
 di.wordsep='/'
 di.wordno=1
 di.defaultvalue='1'
 di.required=true
  
 di=dict[++din]=dictrec('CURRENT_YEAR','F',2)
 di.wordsep='/'
 di.wordno=2
 di.required=true
 di.conversion='[CENTURY,2]'
 //di.validation=
   
 di=dict[++din]=dictrec('CLOSED_PERIOD_FINANCE','F',16)
 neosys_dict_period(di,'','','[FINANCIAL_PERIOD]')
 di.validation='company_val_closedperiod()'
 
 di=dict[++din]=dictrec('FINANCIAL_YEAR','F',6)
 di.required=true
 di.defaultvalue=1
 
 di=dict[++din]=dictrec('LANGUAGE','F',14)
 di.required=true
 di.defaultvalue='"ENGLISH"'
 
 di=dict[++din]=dictrec('DATE_STYLE','F',10)
 di.conversion='31/01/2002:01/31/2002:2002/01/31'
 di.required=true
 di.defaultvalue='"31/01/2002"'
  
 di=dict[++din]=dictrec('NUMBER_STYLE','F',22)
 di.conversion='1,000.00:1.000,00'
 di.required=true
 di.defaultvalue='"1,000.00"'
  
 di=dict[++din]=dictrec('BASE_CURRENCY_CODE','F',3)
 general_dict_currencycode(di)
 di.required=true
 
 di=dict[++din]=dictrec('MAIN_CURRENCY_CODE','F',15)
 general_dict_currencycode(di)
 di.defaultvalue=''
 
 di=dict[++din]=dictrec('COMPANY_PREFIX','F',28)
 di.maxlength=3
 di.validcharacters='ABCDEFGHIJKLMNOPQRSTUVWXYZ'
 di.validation=function()
 {
  if (neosysyesno('This may affect the invoice number sequence|Are you SURE that you want to change this?')!=1) return neosysinvalid()
 }
  
 di=dict[++din]=dictrec('EXCHANGE_ACCOUNT_NO','F',4)
 general_dict_acno(di,'','COMPANY_CODE')

 di=dict[++din]=dictrec('EXCHANGE_ACCOUNT_NO2','F',5)
 general_dict_acno(di,'','COMPANY_CODE')

 di=dict[++din]=dictrec('REVALUATION_METHOD','F',11)
 di.conversion=''
  
 di=dict[++din]=dictrec('CONVERSION_ACCOUNT_NO','F',12)
 general_dict_acno(di,'','COMPANY_CODE')
 
 di=dict[++din]=dictrec('TAX_ACCOUNT_NO','F',19)
 general_dict_acno(di,'','COMPANY_CODE','-')
 
 di=dict[++din]=dictrec('TAX_REGISTRATION_NO','F',21)
 di.length=20
 
 di=dict[++din]=dictrec('DATE_TIME', 'F', '26')
 neosys_dict_datetimeupdated(di)
 
 di=dict[++din]=dictrec('TERMS_OF_PAYMENT','F',32)
 neosys_dict_text(di,80)
 
 di=dict[++din]=dictrec('MARKET_CODE','F',30)
 general_dict_marketcode(di)
 di.defaultvalue=''
 if (gsystem=='ADAGENCY') di.required=true
 
 di=dict[++din]=dictrec('PAYMENT_INSTRUCTIONS','F',29)
 neosys_dict_text(di,80,5)
 
 di=dict[++din]=dictrec('MARKET_NAME', 'S')
 general_dict_marketname(di)
 
 var g=1 
 di=dict[++din]=dictrec('OTHER_COMPANY_CODE','F',7,'',g)
 general_dict_companycode(di)
 di.required=true
 di.unique=true
 
 di=dict[++din]=dictrec('OTHER_COMPANY_ACCOUNT_NO','F',8,'',g)
 general_dict_acno(di,'','COMPANY_CODE')
 di.required=true

 //calculated fields
  
 di=dict[++din]=dictrec('OTHER_COMPANY_NAME','S','','',g)
 general_dict_companyname(di,'OTHER_COMPANY_CODE')
 
 di=dict[++din]=dictrec('OTHER_COMPANY_ACCOUNT_NAME','S','','',g)
// general_dict_acname(di,'OTHER_COMPANY_ACCOUNT_NO')

 di=dict[++din]=dictrec('STOPPED','F', 35)
 di=dict[++din]=dictrec('VERSION','F', 36)
 
 di=dict[++din]=dictrec('CLOSED_PERIOD_OPERATIONS','F',37)
 neosys_dict_period(di,'','','[FINANCIAL_PERIOD]')
 di.validation='company_val_closedperiod()'
 
 var groupn=3
 general_dict_addlogfields(dict,groupn,40,'general_open_version()')
 din=dict.length-1

 return dict
 
}
