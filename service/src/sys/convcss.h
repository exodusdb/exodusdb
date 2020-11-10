//generated by exodus "compile convcss"
//#ifndef EXODUSDLFUNC_CONVCSS_H
#define EXODUSDLFUNC_CONVCSS_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_convcss;
class efb_convcss : public ExodusFunctorBase
{
public:

efb_convcss(MvEnvironment& mv) : ExodusFunctorBase("convcss", "exodusprogrambasecreatedelete_", mv) {}

efb_convcss& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in mode, in tableid, in in0, out outx)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,out);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_convcss.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_convcss.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (mode,tableid,in0,outx);

}

var operator() () {
 var mode_in;
 var tableid_in;
 var in0_in;
 var outx_out;
 return operator()(mode_in, tableid_in, in0_in, outx_out);
}

var operator() (in mode) {
 var tableid_in;
 var in0_in;
 var outx_out;
 return operator()(mode, tableid_in, in0_in, outx_out);
}

var operator() (in mode, in tableid) {
 var in0_in;
 var outx_out;
 return operator()(mode, tableid, in0_in, outx_out);
}

var operator() (in mode, in tableid, in in0) {
 var outx_out;
 return operator()(mode, tableid, in0, outx_out);
}

};
efb_convcss convcss{mv};
//#endif