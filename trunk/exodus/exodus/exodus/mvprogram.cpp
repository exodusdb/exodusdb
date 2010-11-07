#include <exodus/mvprogram.h>

namespace exodus {

DLL_PUBLIC ExodusProgramBase::ExodusProgramBase(MvEnvironment& inmv)
: mv(inmv)
{};

//virtual var main();
DLL_PUBLIC ExodusProgramBase::~ExodusProgramBase()
{};

inline
DLL_PUBLIC var ExodusProgramBase::calculate(const var& dictid) const
{
	return mv.calculate(dictid);
}

}//of namespace exodus