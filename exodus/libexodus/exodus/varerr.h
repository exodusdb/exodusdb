#ifndef EXODUS_LIBEXODUS_EXODUS_VARERR_H_
#define EXODUS_LIBEXODUS_EXODUS_VARERR_H_

namespace exodus {

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Winline"

// Individual exceptions type are made public so that EXODUS programmers can
// catch specific types of errors and/or stop, abort, or debug as they choose
// clang-format off
class PUBLIC VarDivideByZero    : public VarError {public: explicit VarDivideByZero(    CVR var1);};
class PUBLIC VarNonNumeric      : public VarError {public: explicit VarNonNumeric(      CVR var1);};
class PUBLIC VarNonPositive     : public VarError {public: explicit VarNonPositive(     CVR var1);};
class PUBLIC VarNumOverflow     : public VarError {public: explicit VarNumOverflow(     CVR var1);};
class PUBLIC VarNumUnderflow    : public VarError {public: explicit VarNumUnderflow(    CVR var1);};
class PUBLIC VarOutOfMemory     : public VarError {public: explicit VarOutOfMemory(     CVR var1);};
class PUBLIC VarUnassigned      : public VarError {public: explicit VarUnassigned(      CVR var1);};
class PUBLIC VarUndefined       : public VarError {public: explicit VarUndefined(       CVR var1);};
class PUBLIC VarInvalidPointer  : public VarError {public: explicit VarInvalidPointer(  CVR var1);};
class PUBLIC VarDBException     : public VarError {public: explicit VarDBException(     CVR var1);};
class PUBLIC VarNotImplemented  : public VarError {public: explicit VarNotImplemented(  CVR var1);};
class PUBLIC VarDebug           : public VarError {public: explicit VarDebug(           CVR var1);};

class PUBLIC DimDimensionedZero : public VarError {public: explicit DimDimensionedZero( CVR var1);};
class PUBLIC DimIndexOutOfBounds: public VarError {public: explicit DimIndexOutOfBounds(CVR var1);};
class PUBLIC DimNotDimensioned  : public VarError {public: explicit DimNotDimensioned(  CVR var1);};

// clang-format on

//#pragma GCC diagnostic pop

} //namespace exodus

#endif //EXODUS_LIBEXODUS_EXODUS_VARERR_H_

