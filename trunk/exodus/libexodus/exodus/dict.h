#include <exodus/exodus.h>

//define dict here and not exodus macros to keep dict a keyword only for dictionary libraries
//repeat subroutine details just in case we use dict in a library
//because library functions are redefined to be plain member functions i.e. without EXODUS_IMPORTEXPORT
#define dict(DICTID) EXODUSMACRO_IMPORTEXPORT var DICTID (MvEnvironment& mv)
#define dictinit() //do nothing for now and are global funcs
#define dictexit() //ditto
