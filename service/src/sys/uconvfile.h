//generated by exodus "compile uconvfile"
//#ifndef EXODUSDLFUNC_UCONVFILE_H
#define EXODUSDLFUNC_UCONVFILE_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_uconvfile;
class efb_uconvfile : public ExodusFunctorBase
{
public:

efb_uconvfile(MvEnvironment& mv) : ExodusFunctorBase("uconvfile", "exodusprogrambasecreatedelete_", mv) {}

efb_uconvfile& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in inputfilename, in encoding1i, in encoding2i, out result, out msg)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,out,out);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_uconvfile.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_uconvfile.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (inputfilename,encoding1i,encoding2i,result,msg);

}

var operator() () {
 var inputfilename_in;
 var encoding1i_in;
 var encoding2i_in;
 var result_out;
 var msg_out;
 return operator()(inputfilename_in, encoding1i_in, encoding2i_in, result_out, msg_out);
}

var operator() (in inputfilename) {
 var encoding1i_in;
 var encoding2i_in;
 var result_out;
 var msg_out;
 return operator()(inputfilename, encoding1i_in, encoding2i_in, result_out, msg_out);
}

var operator() (in inputfilename, in encoding1i) {
 var encoding2i_in;
 var result_out;
 var msg_out;
 return operator()(inputfilename, encoding1i, encoding2i_in, result_out, msg_out);
}

var operator() (in inputfilename, in encoding1i, in encoding2i) {
 var result_out;
 var msg_out;
 return operator()(inputfilename, encoding1i, encoding2i, result_out, msg_out);
}

var operator() (in inputfilename, in encoding1i, in encoding2i, out result) {
 var msg_out;
 return operator()(inputfilename, encoding1i, encoding2i, result, msg_out);
}

};
efb_uconvfile uconvfile{mv};
//#endif
