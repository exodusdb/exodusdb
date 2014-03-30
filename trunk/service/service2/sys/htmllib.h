//#ifndef EXODUSDLFUNC_HTMLLIB_H
#define EXODUSDLFUNC_HTMLLIB_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_htmllib;

//a member function with the right arguments, returning a var or void
void htmllib(in mode, io datax, in params="", in params20="")
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_htmllib.pmemberfunction_==NULL)
  efb_htmllib.init("htmllib","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef void (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,io,in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 call CALLMEMBERFUNCTION(*(efb_htmllib.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_htmllib.pmemberfunction_)))
  (mode,datax,params,params20);

}
//#endif
