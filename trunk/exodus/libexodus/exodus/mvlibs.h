//functionality replaced by mvfunctor - scheduled for deletion
#if 0
/*
Copyright (c) 2009 Stephen John Bush

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef MVLIBS_H
#define MVLIBS_H 1

#include <vector>
#include <map>

namespace exodus
{

class DLL_PUBLIC MvLib
{
public:

	//declare pure virtual to force provision of the main standard access method for all
	//classes derived from this class
    virtual var operator()(const var&)=0;

	//intended to allow "if (MvLib)" syntax but does it really work
	//since how can the function be called if it doesnt exist??
    operator bool() const;

	//virtual destructor essential for virtual functions otherwise wrong destructors called causing chaos
    virtual ~MvLib();
};

//MvLibsMap type is a pointer to a hash table of pointers to MvLibs
typedef std::map<std::wstring,MvLib*> MvLibMap;

//TODO how to configure this?
#define NTHREADMVLIBS 99

//global_mvlibs is used as a thread specific collection of MapOfMVCommands (see above)
#ifdef EXO_MVLIBS_CPP
	//plus 1 to allow for main thread to have its own
	std::vector<MvLibMap> global_mvlibmaps(NTHREADMVLIBS+1);
#else
	extern std::vector<MvLibMap> global_mvlibmaps;
#endif

/*
class MvEnvironment;
#ifdef EXO_MVENVIRONMENT_CPP
	//plus 1 to allow for main thread to have its own
	std::vector<MvEnvironment*> global_environments(NTHREADS+1);
#else
	extern std::vector<MvEnvironment*> global_environments;
#endif
*/

class DLL_PUBLIC MvLibs
{
public:

	//CONSTRUCTORS

	MvLibs(){}

	//DESTRUCTOR

	~MvLibs(){
		//TODO delete all the mvlibs for all the threads
		}

	//MUTATORS

	void reset();

    void set(const var& mvlibrarynamename, MvLib* mvlib);

	//ACCESSORS

    MvLib* get(const var& mvlibraryname);

private:

	MvLibMap* mvlibs_;

};

//global_mvlibs is used as a thread specific collection of MapOfMVCommands (see above)
#ifndef EXO_MVLIBS_CPP
extern
#endif
DLL_PUBLIC
MvLibs mvlibs;

} //namespace exodus

#endif /*MVLIBS_H*/
#endif