//generated by exodus "compile ../add/pendingupdates"
//#ifndef EXODUSDLFUNC_PENDINGUPDATES_H
#define EXODUSDLFUNC_PENDINGUPDATES_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_pendingupdates;
class efb_pendingupdates : public ExodusFunctorBase
{
public:

efb_pendingupdates(MvEnvironment& mv) : ExodusFunctorBase("pendingupdates", "exodusprogrambasecreatedelete_", mv) {}

efb_pendingupdates& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() ()
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 //if (efb_getlang.pmemberfunction_==NULL)
 // efb_getlang.init("getlang","exodusprogrambasecreatedelete_",mv);
 if (this->pmemberfunction_==NULL)
  this->init();

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_pendingupdates.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_pendingupdates.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  ();

}
};
efb_pendingupdates pendingupdates{mv};
//#endif
