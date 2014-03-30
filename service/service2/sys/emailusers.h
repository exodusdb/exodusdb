#ifndef EXODUSDLFUNC_EMAILUSERS_H
#define EXODUSDLFUNC_EMAILUSERS_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_emailusers;

//a member function with the right arguments, returning a var or void
var emailusers(in mode, in subject0, in body0, in groupids0, in jobids0, in userids0, in options, io emaillog)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_emailusers.pmemberfunction_==NULL)
  efb_emailusers.init("emailusers","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,in,in,in,in,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_emailusers.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_emailusers.pmemberfunction_)))
  (mode,subject0,body0,groupids0,jobids0,userids0,options,emaillog);

}
#endif
