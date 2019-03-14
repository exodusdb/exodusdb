//generated by exodus "compile ../add/diskfreespace"
//#ifndef EXODUSDLFUNC_DISKFREESPACE_H
#define EXODUSDLFUNC_DISKFREESPACE_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_diskfreespace;
class efb_diskfreespace : public ExodusFunctorBase
{
public:

efb_diskfreespace(MvEnvironment& mv) : ExodusFunctorBase("diskfreespace", "exodusprogrambasecreatedelete_", mv) {}

efb_diskfreespace& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in path0)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_diskfreespace.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_diskfreespace.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (path0);

}

var operator() () {
 var path0_in;
 return operator()(path0_in);
}

};
efb_diskfreespace diskfreespace{mv};
//#endif
