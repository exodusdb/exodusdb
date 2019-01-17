//generated by exodus "compile ../job/prodordersubs2"
//#ifndef EXODUSDLFUNC_PRODORDERSUBS2_H
#define EXODUSDLFUNC_PRODORDERSUBS2_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_prodordersubs2;
class efb_prodordersubs2 : private ExodusFunctorBase
{
public:

efb_prodordersubs2(MvEnvironment& mv) : ExodusFunctorBase("prodordersubs2", "exodusprogrambasecreatedelete_", mv) {}

efb_prodordersubs2& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in mode)
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
 //return CALLMEMBERFUNCTION(*(efb_prodordersubs2.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_prodordersubs2.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (mode);

}

};
efb_prodordersubs2 prodordersubs2{mv};
//#endif
