#ifndef VARERR_H
#define VARERR_H

namespace exodus {

// clang-format off

//individual exceptions are made public so exodus programmers can catch specific errors or even stop/abort/debug if they want
class PUBLIC MVDivideByZero         : public MVError {public: explicit MVDivideByZero         (CVR var1    );};
class PUBLIC MVNonNumeric           : public MVError {public: explicit MVNonNumeric           (CVR var1    );};
class PUBLIC MVIntOverflow          : public MVError {public: explicit MVIntOverflow          (CVR var1    );};
class PUBLIC MVIntUnderflow         : public MVError {public: explicit MVIntUnderflow         (CVR var1    );};
class PUBLIC MVOutOfMemory          : public MVError {public: explicit MVOutOfMemory          (CVR var1    );};
class PUBLIC MVUnassigned           : public MVError {public: explicit MVUnassigned           (CVR var1    );};
class PUBLIC MVUndefined            : public MVError {public: explicit MVUndefined            (CVR var1    );};
class PUBLIC MVInvalidPointer       : public MVError {public: explicit MVInvalidPointer       (CVR var1    );};
class PUBLIC MVDBException          : public MVError {public: explicit MVDBException          (CVR var1    );};
class PUBLIC MVNotImplemented       : public MVError {public: explicit MVNotImplemented       (CVR var1    );};
class PUBLIC MVDebug                : public MVError {public: explicit MVDebug                (CVR var1 DEFAULT_EMPTY);};
class PUBLIC MVArrayDimensionedZero : public MVError {public: explicit MVArrayDimensionedZero (            );};
class PUBLIC MVArrayIndexOutOfBounds: public MVError {public: explicit MVArrayIndexOutOfBounds(CVR var1    );};
class PUBLIC MVArrayNotDimensioned  : public MVError {public: explicit MVArrayNotDimensioned  (            );};

// clang-format on

} //namespace exodus

#endif //VARERR_H

