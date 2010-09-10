// © 2006 NEOSYS Software Ltd. All Rights Reserved.//**Start Encode**

//used in authorisation.htm and users.htm

var gtasks_newpassword
function user_setpassword(savemode)
{

 grecn=getrecn(event.srcElement)
 
 var userid=gds.calcx('USER_ID',grecn)
 if (typeof userid!='string') userid=userid[0]
 if (!userid) return neosysinvalid('This is a group separator line\rYou can only set the password on user lines')
 
 if (userid!=gusername&&!neosyssecurity('AUTHORISATION UPDATE')) return neosysinvalid()
 
 var newpassword=''
 
 var reply=1
 if (neosyssecurity('AUTHORISATION INVENT OWN PASSWORDS')) reply=neosysdecide('','Generate random password:Enter your own password')
 if (!reply) return false

 while (true)
 {
 
  if (reply==2)
  {
  
   //user inputs password
   newpassword=neosysinput('Please enter a new case-insensitive password for '+userid+'\rMinimum 5 alphanumeric characters.)',newpassword,true)
   if (!newpassword) return false
   newpassword=newpassword.toUpperCase()
   
  }
  else
  {
 
   while (true)
   {
   
    newpassword=''
    
    //generate six letter password CVCVCV
    var consonants='BCDFGHJKLMNPRSTVWZ'//except AEIOUY and QX
    var vowels='AEIOUY'
    for (var i=0;i<3;i++)
    {
     newpassword+=consonants.substr(neosysrnd(consonants.length),1)
     newpassword+=vowels.substr(neosysrnd(vowels.length),1)
    }
    
    if (!(reply=neosysyesnocancel('The new password for '+userid+' will be\r\r'+newpassword+'\r\rRemember the new password!\r\rPassword is case-insensitive',1,'OK','No, generate another','Cancel'))) return false
    if (reply==1) break
   }
   
  }
  
  var newpassword2=''
  newpassword2=neosysinput('Enter your new password to confirm\ror press Esc to Cancel\r\rRemember the new password!\r\rPassword is case-insensitive',newpassword2,true)
  
  if (!newpassword2) return false
  
  newpassword2=newpassword2.toUpperCase()
  if (newpassword2==newpassword) break

  neosysinvalid('You did not enter the same password\rPlease try again')
  
 }
 
 gds.setrecord('PASSWORD',grecn,newpassword.toUpperCase())
 gchangesmade=true
 
 //login with new password after writing if you change your own password
 if (userid==gusername) gtasks_newpassword=newpassword
 
 if (savemode)
 {n
  saverecord_onclick()
 }
 else
 {
  neosysnote('The new password will not be effective\runless and until you save this document')
 }

 return true
 
}
