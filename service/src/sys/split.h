//generated by exodus "compile split"
//#ifndef EXODUSDLFUNC_SPLIT_H
#define EXODUSDLFUNC_SPLIT_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_split;
class efb_split : public ExodusFunctorBase
{
public:

efb_split(MvEnvironment& mv) : ExodusFunctorBase("split", "exodusprogrambasecreatedelete_", mv) {}

efb_split& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in input0, out unitx)
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
 //return CALLMEMBERFUNCTION(*(efb_split.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_split.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (input0,unitx);

}

var operator() () {
 var input0_in;
 var unitx_out;
 return operator()(input0_in, unitx_out);
}

var operator() (in input0) {
 var unitx_out;
 return operator()(input0, unitx_out);
}

};
efb_split split{mv};
//#endif
