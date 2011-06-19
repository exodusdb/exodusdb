%module(docstring="An interface to Exodus") exodus

%include "swig.swg"

#ifdef SWIGCSHARP
//if you change "mvar" then it also need to be changed in build scripts prebuildcsharp.cmd and config.sh
//(until exodus.i somehow implements "override" keyword on mvar ToString)
%rename(mvar) var;
%include "csharp/csharp.swg"
%include "csharp/std_wstring.i"
%include "csharp/std_string.i" 
%include "csharp/typemaps.i"
%include "csharp/wchar.i"
%include "exception.i"
%include <std_string.i>
%ignore exodus::var::var(wchar_t const *);
%ignore exodus::var::var(char const *);
%rename (ToString) toString;
#endif

#ifdef SWIGJAVA
%include "java/java.swg"
%include "java/javakw.swg"
%include "java/typemaps.i"
%include "java/std_wstring.i"
%include "java/std_string.i" 
%include "php/utils.i"
%include "exception.i"
%include <std_string.i>
//%rename (tostring) toString;
#endif

#ifdef SWIGPYTHON

%implicitconv var;
%include "python/pywstrings.swg"
%include "python/pystrings.swg"
%include "python/std_string.i"
%include "python/typemaps.i"

%fragment("SWIG_AsVal_wchar_t", "header", fragment="<wchar.h>") {
    SWIGINTERN int SWIG_AsVal_wchar_t(PyObject* p, wchar_t* c) {
        return SWIG_OK;
    }
}
%fragment("SWIG_From_wchar_t", "header", fragment="<wchar.h>") {
    SWIGINTERNINLINE PyObject* SWIG_From_wchar_t(wchar_t c) {
        return SWIG_Py_Void();
    }
} 

/*
// Python -> C
%typemap(in) wchar_t const * {
  $1 = PyString_to_wchar_t($input);
}

// C -> Python
%typemap(out) wchar_t * {
  $result = wchar_t_to_PyObject($1);
}
*/

//%rename(__repr__) toString;
%rename(__str__) toString;
#endif

#ifdef SWIGPHP
%include "php/php.swg"

//%include "php/phpinit.swg"
//%include "php/phprun.swg"
//%include "php/phpkw.swg"

%include "php/typemaps.i"
%include "php/std_string.i"

%include "exception.i"
%include "php/utils.i"

%rename(mvar) var;
%rename(__toString) toString;

#endif

#ifdef SWIGPERL
%include "perl5/perl5.swg"
//%include "perl5/perlmain.i"
%include "perl5/typemaps.i"
%include "perl5/std_string.i"
//%include "std/std_basic_string.i"
//%include "std/std_wstring.i"

//http://perldoc.perl.org/overload.html

//swig interface is currently converting undefined var to string in some cases
//for now, return "" in that case




/*
%include "perl5/attribute.i"
%include "perl5/carrays.i"
%include "perl5/cdata.i"
%include "perl5/cmalloc.i"
//%include "perl5/cni.i"
%include "perl5/cpointer.i"
%include "perl5/cstring.i"
%include "perl5/exception.i"
%include "perl5/factory.i"
%include "perl5/jstring.i"
//%include "perl5/Makefile.pl"
//%include "perl5/noembed.h"
%include "perl5/perl5.swg"
//%include "perl5/perlerrors.swg"
%include "perl5/perlfragments.swg"
%include "perl5/perlhead.swg"
%include "perl5/perlinit.swg"
%include "perl5/perlkw.swg"
%include "perl5/perlmacros.swg"
%include "perl5/perlmain.i"
%include "perl5/perlopers.swg"
%include "perl5/perlprimtypes.swg"
//%include "perl5/perlrun.swg"
%include "perl5/perlruntime.swg"
%include "perl5/perlstrings.swg"
%include "perl5/perltypemaps.swg"
%include "perl5/perluserdir.swg"
%include "perl5/reference.i"
%include "perl5/std_common.i"
%include "perl5/std_deque.i"
%include "perl5/std_except.i"
%include "perl5/std_list.i"
%include "perl5/std_map.i"
%include "perl5/std_pair.i"
%include "perl5/std_string.i"
%include "perl5/std_vector.i"
%include "perl5/stl.i"
%include "perl5/typemaps.i"
*/

%rename(__str__) toString2;

//linking of swig gets confused and wants var:win32_connect and var:win32_select?!
//solution seems to be to undefine some macros (from where) in top of mv.h during swig generation
//%rename (connect) connect_renamed;
//%rename (select) select_renamed;
//%rename (abort) abort_renamed;

#endif

%{
#include "exodus/mv.h"
//#include "exodus/mvenvironment.h"
//#include "exodus/exodusfuncs.h"
%}

%include "exodus/mv.h"
//%include "exodus/mvenvironment.h"
//%include "exodus/exodusfuncs.h"
