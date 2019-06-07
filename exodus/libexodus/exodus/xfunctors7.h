#ifndef ExodusFunctorS7_H
#define ExodusFunctorS7_H

// generated by genfunctors.cpp

#include <exodus/mvfunctor.h>

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
class ExodusFunctorS7 : private ExodusFunctorBase
{
      public:
	ExodusFunctorS7(const std::string libname, const std::string funcname)
	    : ExodusFunctorBase(libname, funcname)
	{
	}

	void operator()()
	{
		checkload();
		typedef void (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
					      T7 arg7);
		((ExodusDynamic)pfunction_)(T1(), T2(), T3(), T4(), T5(), T6(), T7());
		return;
	}

	void operator()(T1 arg1)
	{
		checkload();
		typedef void (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
					      T7 arg7);
		((ExodusDynamic)pfunction_)(arg1, T2(), T3(), T4(), T5(), T6(), T7());
		return;
	}

	void operator()(T1 arg1, T2 arg2)
	{
		checkload();
		typedef void (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
					      T7 arg7);
		((ExodusDynamic)pfunction_)(arg1, arg2, T3(), T4(), T5(), T6(), T7());
		return;
	}

	void operator()(T1 arg1, T2 arg2, T3 arg3)
	{
		checkload();
		typedef void (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
					      T7 arg7);
		((ExodusDynamic)pfunction_)(arg1, arg2, arg3, T4(), T5(), T6(), T7());
		return;
	}

	void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4)
	{
		checkload();
		typedef void (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
					      T7 arg7);
		((ExodusDynamic)pfunction_)(arg1, arg2, arg3, arg4, T5(), T6(), T7());
		return;
	}

	void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
	{
		checkload();
		typedef void (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
					      T7 arg7);
		((ExodusDynamic)pfunction_)(arg1, arg2, arg3, arg4, arg5, T6(), T7());
		return;
	}

	void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
	{
		checkload();
		typedef void (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
					      T7 arg7);
		((ExodusDynamic)pfunction_)(arg1, arg2, arg3, arg4, arg5, arg6, T7());
		return;
	}

	void operator()(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
	{
		checkload();
		typedef void (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6,
					      T7 arg7);
		((ExodusDynamic)pfunction_)(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
		return;
	}
};
#endif
