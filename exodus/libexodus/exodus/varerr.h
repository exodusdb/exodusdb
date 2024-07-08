#ifndef EXODUS_LIBEXODUS_EXODUS_VARERR_H_
#define EXODUS_LIBEXODUS_EXODUS_VARERR_H_

namespace exo {

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Winline"

/////////////////////////
// A base exception class - Provide stack tracing
/////////////////////////
class PUBLIC VarError {
 public:

	//VarError(CBR description) = delete;
	//explicit
	VarError(std::string description);

	// Note: "description" is not const so that an exception handler (catch block)
	// can add any context (additional info only known in the handler)
	// to the error description and rethrow the exception up to
	// a higher exception handler.
	// (using plain "throw;")
	// Otherwise the only the stack track captured by the exception site will be available. 
	std::string description;

	// function to convert stack addresses to source code if available
	std::string stack(const size_t limit = 0) const;

 private:

	mutable void* stack_addresses_[BACKTRACE_MAXADDRESSES];
	mutable size_t stack_size_ = 0;

};

// Individual exceptions type are made public so that EXODUS programmers can
// catch specific types of errors and/or stop, abort, or debug as they choose
// clang-format off
class PUBLIC VarDivideByZero    : public VarError {public: explicit VarDivideByZero(    std::string message);};
class PUBLIC VarNonNumeric      : public VarError {public: explicit VarNonNumeric(      std::string message);};
class PUBLIC VarNonPositive     : public VarError {public: explicit VarNonPositive(     std::string message);};
class PUBLIC VarNumOverflow     : public VarError {public: explicit VarNumOverflow(     std::string message);};
class PUBLIC VarNumUnderflow    : public VarError {public: explicit VarNumUnderflow(    std::string message);};
class PUBLIC VarOutOfMemory     : public VarError {public: explicit VarOutOfMemory(     std::string message);};
class PUBLIC VarUnassigned      : public VarError {public: explicit VarUnassigned(      std::string message);};
class PUBLIC VarUndefined       : public VarError {public: explicit VarUndefined(       std::string message);};
class PUBLIC VarInvalidPointer  : public VarError {public: explicit VarInvalidPointer(  std::string message);};
class PUBLIC VarDBException     : public VarError {public: explicit VarDBException(     std::string message);};
class PUBLIC VarNotImplemented  : public VarError {public: explicit VarNotImplemented(  std::string message);};
class PUBLIC VarDebug           : public VarError {public: explicit VarDebug(           std::string message);};

class PUBLIC DimDimensionedZero : public VarError {public: explicit DimDimensionedZero( std::string message);};
class PUBLIC DimIndexOutOfBounds: public VarError {public: explicit DimIndexOutOfBounds(std::string message);};
class PUBLIC DimNotDimensioned  : public VarError {public: explicit DimNotDimensioned(  std::string message);};

// clang-format on

//#pragma GCC diagnostic pop

} //namespace exo

#endif //EXODUS_LIBEXODUS_EXODUS_VARERR_H_

