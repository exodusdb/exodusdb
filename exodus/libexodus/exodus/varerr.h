#ifndef EXODUS_LIBEXODUS_EXODUS_VARERR_H_
#define EXODUS_LIBEXODUS_EXODUS_VARERR_H_

namespace exodus {

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Winline"

// Individual exceptions type are made public so that EXODUS programmers can
// catch specific types of errors and/or stop, abort, or debug as they choose
// clang-format off
class PUBLIC VarDivideByZero    : public VarError {public: explicit VarDivideByZero(    CBR var1);};
class PUBLIC VarNonNumeric      : public VarError {public: explicit VarNonNumeric(      CBR var1);};
class PUBLIC VarNonPositive     : public VarError {public: explicit VarNonPositive(     CBR var1);};
class PUBLIC VarNumOverflow     : public VarError {public: explicit VarNumOverflow(     CBR var1);};
class PUBLIC VarNumUnderflow    : public VarError {public: explicit VarNumUnderflow(    CBR var1);};
class PUBLIC VarOutOfMemory     : public VarError {public: explicit VarOutOfMemory(     CBR var1);};
class PUBLIC VarUnassigned      : public VarError {public: explicit VarUnassigned(      CBR var1);};
class PUBLIC VarUndefined       : public VarError {public: explicit VarUndefined(       CBR var1);};
class PUBLIC VarInvalidPointer  : public VarError {public: explicit VarInvalidPointer(  CBR var1);};
class PUBLIC VarDBException     : public VarError {public: explicit VarDBException(     CBR var1);};
class PUBLIC VarNotImplemented  : public VarError {public: explicit VarNotImplemented(  CBR var1);};
class PUBLIC VarDebug           : public VarError {public: explicit VarDebug(           CBR var1);};

class PUBLIC DimDimensionedZero : public VarError {public: explicit DimDimensionedZero( CBR var1);};
class PUBLIC DimIndexOutOfBounds: public VarError {public: explicit DimIndexOutOfBounds(CBR var1);};
class PUBLIC DimNotDimensioned  : public VarError {public: explicit DimNotDimensioned(  CBR var1);};

// clang-format on

//#pragma GCC diagnostic pop

} //namespace exodus

#endif //EXODUS_LIBEXODUS_EXODUS_VARERR_H_

