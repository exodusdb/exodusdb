//generated by exodus "compile reverse"
//#ifndef EXODUSDLFUNC_REVERSE_H
#define EXODUSDLFUNC_REVERSE_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_reverse;
class efb_reverse : public ExodusFunctorBase
{
public:

efb_reverse(MvEnvironment& mv) : ExodusFunctorBase("reverse", "exodusprogrambasecreatedelete_", mv) {}

efb_reverse& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in input)
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
 //return CALLMEMBERFUNCTION(*(efb_reverse.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_reverse.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (input);

}

var operator() () {
 var input_in;
 return operator()(input_in);
}

};
efb_reverse reverse{mv};
//#endif
