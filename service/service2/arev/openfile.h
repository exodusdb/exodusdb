#ifndef EXODUSDLFUNC_../AREV/OPENFILE_H
#define EXODUSDLFUNC_../AREV/OPENFILE_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_../arev/openfile;

//a member function with the right arguments, returning a var
var ../arev/openfile()
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_../arev/openfile.pmemberfunction_==NULL)
  efb_../arev/openfile.init("../arev/openfile","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_../arev/openfile.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_../arev/openfile.pmemberfunction_)))
  ();

}
#endif
