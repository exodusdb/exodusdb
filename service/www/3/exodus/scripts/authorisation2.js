// Copyright NEOSYS All Rights Reserved.

//used in authorisation.htm and users.htm

var gtasks_newpassword
async function user_setpassword(savemode,element) {

 if (!gkey)
  return await exodusinvalid('Please select a user first')
 
 if (!glocked)
  return await exodusinvalid('This record is currently read-only')
 
 //grecn=element?getrecn(element):null
 
 var userid=await gds.getx('USER_ID',grecn)
 if (typeof userid!='string') userid=userid[0]
 if (!userid)
  //return await exodusinvalid('Please select a user first')
  return await exodusinvalid('This is a group separator line\rYou can only set the password on user lines')
 
 if (userid!=gusername&&!(await exodussecurity('AUTHORISATION UPDATE')))
  return await exodusinvalid()
 
 var newpassword=''
 
 var minpasslen=4
 var reply=1
 if (await exodussecurity('AUTHORISATION INVENT OWN PASSWORDS')) reply=await exodusdecide('','Generate random password:Enter your own password')
 if (!reply)
    return false

 while (true) {
 
  if (reply==2) {
  
   //user inputs password
   while (true) {

    newpassword=await exodusinput('Please enter a new case-INSENSITIVE password for '+userid+'\rMinimum '+minpasslen+' alphanumeric characters.)',newpassword,true)
    if (!newpassword)
        return false
    newpassword=newpassword.toUpperCase()
    if (newpassword.length>=minpasslen)
     break
    await exodusinvalid('The minimum password length is '+minpasslen)
   }
   
  }
  else {
 
   while (true) {
   
    newpassword=''
    
    //generate six letter password CVCVCV
    var consonants='BCDFGHJKLMNPRSTVWZ'//except AEIOUY and QX
    var vowels='AEIOUY'
    for (var i=0;i<(minpasslen/2);i++) {

     newpassword+=consonants.substr(exodusrnd(consonants.length),1)
     newpassword+=vowels.substr(exodusrnd(vowels.length),1)
    }
    
    // default_icons false: "generate another" is not a No/reject
    if (!(reply=await exodusconfirm('The new password for '+userid+' will be\r\r'+newpassword+'\r\rRemember the new password!\r\rPassword is case-insensitive',1,'OK','No, generate another','Cancel',null,null,null,false)))
        return false
    if (reply==1)
        break
   }
   
  }
  
  var newpassword2=''
  newpassword2=await exodusinput('Enter your new password to confirm\ror press Esc to Cancel\r\rRemember the new password!\r\rPassword is case-INSENSITIVE',newpassword2,true)

  if (!newpassword2)
   return false
  
  newpassword2=newpassword2.toUpperCase()
  if (newpassword2==newpassword)
    break

  await exodusinvalid('You did not enter the same password\rPlease try again')
  
 }

 await gds.setx('PASSWORD',grecn,newpassword.toUpperCase())
 setchangesmade(true)
 
 //login with new password after writing if you change your own password
 if (userid==gusername)
  gtasks_newpassword=newpassword

 if (savemode) {

  if (!(await exodusokcancel('Change password?',1)))
   return await exodusinvalid()
  await saverecord_onclick()
 }
 else {

  await exodusnote('The new password will not be effective\runless and until you save this document')
 }

 return true
 
}
