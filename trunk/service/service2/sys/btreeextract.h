//#ifndef EXODUSDLFUNC_BTREEEXTRACT_H
#define EXODUSDLFUNC_BTREEEXTRACT_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_btreeextract;

//a member function with the right arguments, returning a var or void
var btreeextract(in cmd, in filename, in dictfile, out hits)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_btreeextract.pmemberfunction_==NULL)
  efb_btreeextract.init("btreeextract","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,out);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_btreeextract.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_btreeextract.pmemberfunction_)))
  (cmd,filename,dictfile,hits);

}
//#endif
