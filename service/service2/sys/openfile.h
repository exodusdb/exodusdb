//#ifndef EXODUSDLFUNC_OPENFILE_H
#define EXODUSDLFUNC_OPENFILE_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_openfile;

//a member function with the right arguments, returning a var or void
var openfile(in filename, io file, in similarfilename="")
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_openfile.pmemberfunction_==NULL)
  efb_openfile.init("openfile","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,io,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_openfile.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_openfile.pmemberfunction_)))
  (filename,file,similarfilename);

}
//#endif
