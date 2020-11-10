//generated by exodus "compile shell2"
//#ifndef EXODUSDLFUNC_SHELL2_H
#define EXODUSDLFUNC_SHELL2_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_shell2;
class efb_shell2 : public ExodusFunctorBase
{
public:

efb_shell2(MvEnvironment& mv) : ExodusFunctorBase("shell2", "exodusprogrambasecreatedelete_", mv) {}

efb_shell2& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in cmd,out errors)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,out);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_shell2.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_shell2.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (cmd,errors);

}

var operator() () {
 var cmd_in;
 var errors_out;
 return operator()(cmd_in, errors_out);
}

var operator() (in cmd) {
 var errors_out;
 return operator()(cmd, errors_out);
}

};
efb_shell2 shell2{mv};
//#endif
