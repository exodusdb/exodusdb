module;

#include <exodus/var.h>
#include <exodus/dim.h>
#include <exodus/rex.h>

export module var;

export namespace exo {

	using exo::var_base;
	using exo::var_mid;
	using exo::var;
	using exo::var_iter;

	using exo::dim;
	using exo::dim_iter;
	using exo::rex;
	using exo::VarError;
	using exo::operator""_var;
	using exo::operator""_rex;

	using exo::var_proxy1;
	using exo::var_proxy2;
	using exo::var_proxy3;

	// Not actually required to export since they will be exported along with all var members anyway
	// Only available for specific export if specifically listed in var.h
	// Not sufficient if only declared via friendship
//	using exo::begin;
//	using exo::end;

	// Exported automatically since they are friended in class var
	// and cannot be reexported since they are not actually declared outside of the friend declarations?
//	using exo::operator==;
//	using exo::operator!=;
//	using exo::operator<;
//	using exo::operator>;
//	using exo::operator<=;
//	using exo::operator>=;
//	using exo::operator<=>;

	using exo::varint_t;

	using exo::VarDBException;
	using exo::VarDebug;
	using exo::VarDivideByZero;
	using exo::VarError;
	using exo::VarInvalidPointer;
	using exo::VarNonNumeric;
	using exo::VarNonPositive;
	using exo::VarNotImplemented;
	using exo::VarNumOverflow;
	using exo::VarNumUnderflow;
	using exo::VarOutOfMemory;
	using exo::VarUnassigned;
	using exo::VarUnconstructed;

	using exo::DimUndimensioned;
	using exo::DimIndexOutOfBounds;

	using exo::CVR;
	using exo::VARREF;
	using exo::SV;

//	using exo::EOL;
//	using exo::OSSLASH;
//	using exo::OSSLASH_IS_BACKSLASH;

//	using exo::_CPP_STANDARD;
//	using exo::PLATFORM;

	using exo::_OS_NAME;
	using exo::_OS_VERSION;

	using exo::RM;
	using exo::FM;
	using exo::VM;
	using exo::SM;
	using exo::TM;
	using exo::STM;

	using exo::BS;
	using exo::DQ;
	using exo::SQ;

	using exo::BASEFMT;
	using exo::DATEFMT;
	using exo::TZ;

// Generated using
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+ -oP|awk '{print "\tusing " $1 ";"}'|sort|uniq

// See also for all exo:: classes
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+::[\\w]+ -oP|sort|uniq|awk -F: '{print $3}'|uniq

// See also for member functions of classes
// nm -D -S -C /usr/local/lib/libexodus.so|\grep exo::[\\w]+::[\\w]+ -oP|sort|uniq

// Member functions do not need to be exported
//	using exo::abs;

}
