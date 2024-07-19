//module;
//export module exo;
//export import :var;
//export import :dim;

module;

#include <exodus/var.h>
#include <exodus/dim.h>
#include <exodus/rex.h>

export module var;

namespace exo {
    export using exo::var;
    export using exo::var_iter;
//    export using exo::begin;
//    export using exo::end;
    export using exo::dim;
//    export using exo::dim_iter;
    export using exo::rex;
    export using exo::VarError;
	export using exo::operator""_var;
    export using exo::operator""_rex;
	export using exo::SMALLEST_NUMBER;

	export using exo::varint_t;

	export using exo::VarDivideByZero;
	export using exo::VarNonNumeric;
	export using exo::VarNonPositive;
	export using exo::VarNumOverflow;
	export using exo::VarNumUnderflow;
	export using exo::VarOutOfMemory;
	export using exo::VarUnassigned;
	export using exo::VarUndefined;
	export using exo::VarInvalidPointer;
	export using exo::VarDBException;
	export using exo::VarNotImplemented;
	export using exo::VarDebug;

	export using exo::DimDimensionedZero;
	export using exo::DimIndexOutOfBounds;
	export using exo::DimNotDimensioned;

    export using exo::CVR;
    export using exo::VARREF;
    export using exo::SV;

//	export using exo::EOL;
//	export using exo::OSSLASH;
//	export using exo::OSSLASH_IS_BACKSLASH;

//	export using exo::_CPP_STANDARD;
//	export using exo::PLATFORM;

	export using exo::_OS_NAME;
	export using exo::_OS_VERSION;

	export using exo::RM;
	export using exo::FM;
	export using exo::VM;
	export using exo::SM;
	export using exo::TM;
	export using exo::ST;

	export using exo::BS;
	export using exo::DQ;
	export using exo::SQ;

}
