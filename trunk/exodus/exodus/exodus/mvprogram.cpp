#include <exodus/mvprogram.h>

namespace exodus {

DLL_PUBLIC
	ExodusProgramBase::ExodusProgramBase(MvEnvironment& inmv)
		:mv(inmv)
		{};

//virtual var main();

DLL_PUBLIC
	ExodusProgramBase::~ExodusProgramBase()
		{};

}//of namespace exodus
