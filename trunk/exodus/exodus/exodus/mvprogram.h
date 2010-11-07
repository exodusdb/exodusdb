#ifndef MVPROGRAM_H
#define MVPROGRAM_H

#include <exodus/mvenvironment.h>
//#include <exodus/mvfunctor.h>

namespace exodus {

class ExodusProgramBase
{
public:
	DLL_PUBLIC ExodusProgramBase(MvEnvironment& inmv);
	//virtual var main();
	DLL_PUBLIC virtual ~ExodusProgramBase();
	//mv.xyz is going to be used a lot by exodus programmers for exodus "global variables"
	//it is member data so it is global to the class/object and not global to the program
	//so it is threadsafe
	//eg mv.RECORD mv.DICT
	MvEnvironment& mv;
	DLL_PUBLIC var calculate(const var& dictid) const;
};

}
#endif //MVPROGRAM_H
