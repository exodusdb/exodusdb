#ifndef EXODUSDLFUNC_CHANGELOGSUBS_H
#define EXODUSDLFUNC_CHANGELOGSUBS_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_changelogsubs;

//a member function with the right arguments, returning a var
var changelogsubs(in mode0)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_changelogsubs.pmemberfunction_==NULL)
  efb_changelogsubs.init("changelogsubs","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_changelogsubs.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_changelogsubs.pmemberfunction_)))
  (mode0);

}
#endif
