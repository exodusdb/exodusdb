// Copyright NEOSYS All Rights Reserved.

//used in authorisation.htm and users.htm

var gtasks_newpassword
function* user_setpassword(savemode,element) {

 if (!gkey)
  return yield* neosysinvalid('Please select a user first')
 
 if (!glocked)
  return yield* neosysinvalid('This record is currently read-only')
 
 //grecn=element?getrecn(element):null
 
 var userid=yield* gds.getx('USER_ID',grecn)
 if (typeof userid!='string') userid=userid[0]
 if (!userid)
  //return yield* neosysinvalid('Please select a user first')
  return yield* neosysinvalid('This is a group separator line\rYou can only set the password on user lines')
 
 if (userid!=gusername&&!(yield* neosyssecurity('AUTHORISATION UPDATE')))
  return yield* neosysinvalid()
 
 var newpassword=''
 
 var minpasslen=4
 var reply=1
 if (yield* neosyssecurity('AUTHORISATION INVENT OWN PASSWORDS')) reply=yield* neosysdecide('','Generate random password:Enter your own password')
 if (!reply)
    return false

 while (true) {
 
  if (reply==2) {
  
   //user inputs password
   while (true) {

    newpassword=yield* neosysinput('Please enter a new case-INSENSITIVE password for '+userid+'\rMinimum '+minpasslen+' alphanumeric characters.)',newpassword,true)
    if (!newpassword)
        return false
    newpassword=newpassword.toUpperCase()
    if (newpassword.length>=minpasslen)
     break
    yield* neosysinvalid('The minimum password length is '+minpasslen)
   }
   
  }
  else {
 
   while (true) {
   
    newpassword=''
    
    //generate six letter password CVCVCV
    var consonants='BCDFGHJKLMNPRSTVWZ'//except AEIOUY and QX
    var vowels='AEIOUY'
    for (var i=0;i<(minpasslen/2);i++) {

     newpassword+=consonants.substr(neosysrnd(consonants.length),1)
     newpassword+=vowels.substr(neosysrnd(vowels.length),1)
    }
    
    if (!(reply=yield* neosysconfirm('The new password for '+userid+' will be\r\r'+newpassword+'\r\rRemember the new password!\r\rPassword is case-insensitive',1,'OK','No, generate another','Cancel')))
        return false
    if (reply==1)
        break
   }
   
  }
  
  var newpassword2=''
  newpassword2=yield* neosysinput('Enter your new password to confirm\ror press Esc to Cancel\r\rRemember the new password!\r\rPassword is case-INSENSITIVE',newpassword2,true)

  if (!newpassword2)
   return false
  
  newpassword2=newpassword2.toUpperCase()
  if (newpassword2==newpassword)
    break

  yield* neosysinvalid('You did not enter the same password\rPlease try again')
  
 }

 yield* gds.setx('PASSWORD',grecn,newpassword.toUpperCase())
 setchangesmade(true)
 
 //login with new password after writing if you change your own password
 if (userid==gusername)
  gtasks_newpassword=newpassword

 if (savemode) {

  if (!(yield* neosysokcancel('Change password?',1)))
   return yield* neosysinvalid()
  yield* saverecord_onclick()
 }
 else {

  yield* neosysnote('The new password will not be effective\runless and until you save this document')
 }

 return true
 
}
