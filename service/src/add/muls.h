//generated by exodus "compile ../add/muls"
//#ifndef EXODUSDLFUNC_MULS_H
#define EXODUSDLFUNC_MULS_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_muls;
class efb_muls : public ExodusFunctorBase
{
public:

efb_muls(MvEnvironment& mv) : ExodusFunctorBase("muls", "exodusprogrambasecreatedelete_", mv) {}

efb_muls& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in multipliers1, in multipliers2, in sep)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_muls.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_muls.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (multipliers1,multipliers2,sep);

}

var operator() () {
 var multipliers1_in;
 var multipliers2_in;
 var sep_in;
 return operator()(multipliers1_in, multipliers2_in, sep_in);
}

var operator() (in multipliers1) {
 var multipliers2_in;
 var sep_in;
 return operator()(multipliers1, multipliers2_in, sep_in);
}

var operator() (in multipliers1, in multipliers2) {
 var sep_in;
 return operator()(multipliers1, multipliers2, sep_in);
}

};
efb_muls muls{mv};
//#endif
