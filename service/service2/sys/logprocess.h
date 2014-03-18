#ifndef EXODUSDLFUNC_LOGPROCESS_H
#define EXODUSDLFUNC_LOGPROCESS_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_logprocess;

//a member function with the right arguments, returning a var
var logprocess(io processid, in processcategory, in processparameters="", in processresult="", in processcomments="")
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_logprocess.pmemberfunction_==NULL)
  efb_logprocess.init("logprocess","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io,in,in,in,in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_logprocess.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_logprocess.pmemberfunction_)))
  (processid,processcategory,processparameters,processresult,processcomments);

}
#endif
