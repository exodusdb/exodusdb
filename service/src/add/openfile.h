//generated by exodus "compile ../add/openfile"
//#ifndef EXODUSDLFUNC_OPENFILE_H
#define EXODUSDLFUNC_OPENFILE_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_openfile;
class efb_openfile : public ExodusFunctorBase
{
public:

efb_openfile(MvEnvironment& mv) : ExodusFunctorBase("openfile", "exodusprogrambasecreatedelete_", mv) {}

efb_openfile& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in filename, io file, in similarfilename="", in autocreate="")
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,io,in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_openfile.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_openfile.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (filename,file,similarfilename,autocreate);

}

var operator() () {
 var filename_in;
 var file_io;
 var similarfilename_in = "";
 var autocreate_in = "";
 return operator()(filename_in, file_io, similarfilename_in, autocreate_in);
}

var operator() (in filename) {
 var file_io;
 var similarfilename_in = "";
 var autocreate_in = "";
 return operator()(filename, file_io, similarfilename_in, autocreate_in);
}

};
efb_openfile openfile{mv};
//#endif
