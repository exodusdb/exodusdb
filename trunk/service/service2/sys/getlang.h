#ifndef EXODUSDLFUNC_GETLANG_H
#define EXODUSDLFUNC_GETLANG_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_getlang;

//a member function with the right arguments, returning a var
var getlang(in origprogname0, in languagecode0, in origdatatype, io languagefile, io lang)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_getlang.pmemberfunction_==NULL)
  efb_getlang.init("getlang","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,io,io);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_getlang.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_getlang.pmemberfunction_)))
  (origprogname0,languagecode0,origdatatype,languagefile,lang);

}
#endif
