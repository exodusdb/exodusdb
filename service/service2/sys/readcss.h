#ifndef EXODUSDLFUNC_READCSS_H
#define EXODUSDLFUNC_READCSS_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_readcss;

//a member function with the right arguments, returning a var
var readcss(io css)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_readcss.pmemberfunction_==NULL)
  efb_readcss.init("readcss","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_readcss.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_readcss.pmemberfunction_)))
  (css);

}
#endif
