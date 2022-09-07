#ifndef EXODUS_LIBEXODUS_EXODUS_VARERR_H_
#define EXODUS_LIBEXODUS_EXODUS_VARERR_H_

namespace exodus {

// clang-format off

//individual exceptions are made public so exodus programmers can catch specific errors or even stop/abort/debug if they want
class PUBLIC VarDivideByZero    : public VarError {public: explicit VarDivideByZero(    CVR var1);};
class PUBLIC VarNonNumeric      : public VarError {public: explicit VarNonNumeric(      CVR var1);};
class PUBLIC VarIntOverflow     : public VarError {public: explicit VarIntOverflow(     CVR var1);};
class PUBLIC VarIntUnderflow    : public VarError {public: explicit VarIntUnderflow(    CVR var1);};
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

} //namespace exodus

#endif //EXODUS_LIBEXODUS_EXODUS_VARERR_H_

