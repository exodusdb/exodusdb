#ifndef LIBEXODUS_VARERR_H_
#define LIBEXODUS_VARERR_H_

//#include <stddef> // std::size_t

#if EXO_MODULE
	import std;
#else
#	include <string>
#endif

namespace exo {

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Winline"

/////////////////////////
// A base exception class - Provide stack tracing
/////////////////////////
class PUBLIC VarError {
 public:

	VarError(std::string message);

	// Note: "message" is not const so that an exception handler (catch block)
	// can add any context (additional info only known in the handler)
	// to the error message and rethrow the exception up to
	// a higher exception handler.
	// (using plain "throw;")
	// Otherwise the only the stack track captured by the exception site will be available. 
	std::string message;

	// function to convert stack addresses to source code if available
	std::string stack(const std::size_t limit = 0) const;

 private:

	mutable void* stack_addresses_[BACKTRACE_MAXADDRESSES];
	mutable std::size_t stack_size_ = 0;

};

// Individual exceptions type are made public so that EXODUS programmers can
// catch specific types of errors and/or stop, abort, or debug as they choose
// clang-format off
class PUBLIC VarUnconstructed   : public VarError {public: explicit VarUnconstructed(   std::string message);};
class PUBLIC VarUnassigned      : public VarError {public: explicit VarUnassigned(      std::string message);};
class PUBLIC VarDivideByZero    : public VarError {public: explicit VarDivideByZero(    std::string message);};
class PUBLIC VarNonNumeric      : public VarError {public: explicit VarNonNumeric(      std::string message);};
class PUBLIC VarNonPositive     : public VarError {public: explicit VarNonPositive(     std::string message);};
class PUBLIC VarNumOverflow     : public VarError {public: explicit VarNumOverflow(     std::string message);};
class PUBLIC VarNumUnderflow    : public VarError {public: explicit VarNumUnderflow(    std::string message);};
class PUBLIC VarOutOfMemory     : public VarError {public: explicit VarOutOfMemory(     std::string message);};
class PUBLIC VarInvalidPointer  : public VarError {public: explicit VarInvalidPointer(  std::string message);};
class PUBLIC VarDBException     : public VarError {public: explicit VarDBException(     std::string message);};
class PUBLIC VarNotImplemented  : public VarError {public: explicit VarNotImplemented(  std::string message);};
class PUBLIC VarDebug           : public VarError {public: explicit VarDebug(           std::string message);};

class PUBLIC DimUndimensioned : public VarError {public: explicit DimUndimensioned( std::string message);};
class PUBLIC DimIndexOutOfBounds: public VarError {public: explicit DimIndexOutOfBounds(std::string message);};

// clang-format on

//#pragma GCC diagnostic pop

} //namespace exo

#endif //LIBEXODUS_VARERR_H_

