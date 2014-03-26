#ifndef EXODUSDLFUNC_ROUNDROBIN_H
#define EXODUSDLFUNC_ROUNDROBIN_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_roundrobin;

//a member function with the right arguments, returning a var
var roundrobin(in mode, in params0, io msg, io result)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_roundrobin.pmemberfunction_==NULL)
  efb_roundrobin.init("roundrobin","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,io,io);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_roundrobin.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_roundrobin.pmemberfunction_)))
  (mode,params0,msg,result);

}
#endif
