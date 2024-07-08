//
// MUST keep this file virtually identical between "varfriends.h" and "varfriends_impl.h"
// except for commenting in/out the implementation section on the right hand side
//
// Update one of the other recommend varfriends.h and CREATE the other using one of the following:
//
// 1. varfriends.h -> varfriends_impl.h
//
//		sed 's#;//IMPL # /\*IMPL\*/ #' varfriends.h > varfriends_impl.h
//
// 2. varfriends_impl.h -> varfriends.h
//
//		sed 's# /\*IMPL\*/#;//IMPL#' varfriends_impl.h > varfriends.h
//
	//////////////////////////////
	// OPERATOR FRIENDS - pure var
	//////////////////////////////

	// Logical friends for var and var

	PUBLIC VAR_FRIEND bool operator==(CBX lhs, CBX rhs);//IMPL {return  var_eq_var(lhs, rhs );}
#if !(__GNUG__ >= 11 || __clang_major__ >= 14)
	PUBLIC VAR_FRIEND bool operator!=(CBX lhs, CBX rhs);//IMPL {return !var_eq_var(lhs, rhs );}
#endif
	PUBLIC VAR_FRIEND bool operator< (CBX lhs, CBX rhs);//IMPL {return  var_lt_var(lhs, rhs );}
	PUBLIC VAR_FRIEND bool operator>=(CBX lhs, CBX rhs);//IMPL {return !var_lt_var(lhs, rhs );}
	PUBLIC VAR_FRIEND bool operator> (CBX lhs, CBX rhs);//IMPL {return  var_lt_var(rhs, lhs );}
	PUBLIC VAR_FRIEND bool operator<=(CBX lhs, CBX rhs);//IMPL {return !var_lt_var(rhs, lhs );}

	/////////////////////////////////
	// OPERATOR FRIENDS - mixed types
	/////////////////////////////////

	// LOGICAL friends v. main types

	//== and !=
	//TODO consider replacing by operator<=> and replacing var_eq_var and var_lt by MVcmp
	//or provide more specialisations of var_eq_var and var_lt esp. for numeric types

	// == EQ friends v. main types

	//template<class RHS>
//	PUBLIC VAR_FRIEND bool operator==(CBX          lhs,   RHS          rhs   );//IMPL {return  var_eq_var(  lhs, rhs       ); }
	PUBLIC VAR_FRIEND bool operator==(CBX          lhs,   const char*  cstr2 );//IMPL {return  var_eq_var(  lhs, VAR(cstr2)); }
	PUBLIC VAR_FRIEND bool operator==(CBX          lhs,   const char   char2 );//IMPL {return  var_eq_var(  lhs, VAR(char2)); }
	PUBLIC VAR_FRIEND bool operator==(CBX          lhs,   const int    int2  );//IMPL {return  var_eq_int(  lhs, int2      ); }
	PUBLIC VAR_FRIEND bool operator==(CBX          lhs,   const double dbl2  );//IMPL {return  var_eq_dbl(  lhs, dbl2      ); }
	PUBLIC VAR_FRIEND bool operator==(CBX          lhs,   const bool   bool2 );//IMPL {return  var_eq_bool( lhs, bool2     ); }

//	template<class LHS>
//	PUBLIC VAR_FRIEND bool operator==(LHS lhs,            CBX          rhs   );//IMPL {return  var_eq_var(  rhs, lhs       ); }
	PUBLIC VAR_FRIEND bool operator==(const char*  cstr1, CBX          rhs   );//IMPL {return  var_eq_var(  rhs, VAR(cstr1)); }
	PUBLIC VAR_FRIEND bool operator==(const char   char1, CBX          rhs   );//IMPL {return  var_eq_var(  rhs, VAR(char1)); }
	PUBLIC VAR_FRIEND bool operator==(const int    int1,  CBX          rhs   );//IMPL {return  var_eq_int(  rhs, int1      ); }
	PUBLIC VAR_FRIEND bool operator==(const double dbl1,  CBX          rhs   );//IMPL {return  var_eq_dbl(  rhs, dbl1      ); }
	PUBLIC VAR_FRIEND bool operator==(const bool   bool1, CBX          rhs   );//IMPL {return  var_eq_bool( rhs, bool1     ); }

	// != NE friends v. main types
#if !(__GNUG__ >= 11 || __clang_major__ >= 14)
//	template<class RHS>
//	PUBLIC VAR_FRIEND bool operator!=(CBX          lhs,   RHS          rhs   );//IMPL {return !var_eq_var( lhs, rhs        ); }
	PUBLIC VAR_FRIEND bool operator!=(CBX          lhs,   const char*  cstr2 );//IMPL {return !var_eq_var(  lhs, VAR(cstr2)); }
	PUBLIC VAR_FRIEND bool operator!=(CBX          lhs,   const char   char2 );//IMPL {return !var_eq_var(  lhs, VAR(char2)); }
	PUBLIC VAR_FRIEND bool operator!=(CBX          lhs,   const int    int2  );//IMPL {return !var_eq_int(  lhs, int2      ); }
	PUBLIC VAR_FRIEND bool operator!=(CBX          lhs,   const double dbl2  );//IMPL {return !var_eq_dbl(  lhs, dbl2      ); }
	PUBLIC VAR_FRIEND bool operator!=(CBX          lhs,   const bool   bool2 );//IMPL {return !var_eq_bool( lhs, bool2     ); }

//	template<class LHS>
//	PUBLIC VAR_FRIEND bool operator!=(LHS lhs,            CBX          rhs   );//IMPL {return !var_eq_var(  rhs, lhs       ); }
	PUBLIC VAR_FRIEND bool operator!=(const char*  cstr1, CBX          rhs   );//IMPL {return !var_eq_var(  rhs, VAR(cstr1)); }
	PUBLIC VAR_FRIEND bool operator!=(const char   char1, CBX          rhs   );//IMPL {return !var_eq_var(  rhs, VAR(char1)); }
	PUBLIC VAR_FRIEND bool operator!=(const int    int1,  CBX          rhs   );//IMPL {return !var_eq_int(  rhs, int1      ); }
	PUBLIC VAR_FRIEND bool operator!=(const double dbl1,  CBX          rhs   );//IMPL {return !var_eq_dbl(  rhs, dbl1      ); }
	PUBLIC VAR_FRIEND bool operator!=(const bool   bool1, CBX          rhs   );//IMPL {return !var_eq_bool( rhs, bool1     ); }
#endif
	// < LT friends v. main types

//	template<class RHS>
//	PUBLIC VAR_FRIEND bool operator<(CBX           lhs,   RHS          rhs   );//IMPL {return  var_lt_var(lhs,   rhs       ); }
	PUBLIC VAR_FRIEND bool operator<(CBX           lhs,   const char*  cstr2 );//IMPL {return  var_lt_var(lhs,   VAR(cstr2)); }
	PUBLIC VAR_FRIEND bool operator<(CBX           lhs,   const char   char2 );//IMPL {return  var_lt_var(lhs,   VAR(char2)); }
	PUBLIC VAR_FRIEND bool operator<(CBX           lhs,   const int    int2  );//IMPL {return  var_lt_int(lhs,   int2      ); }
	PUBLIC VAR_FRIEND bool operator<(CBX           lhs,   const double dbl2  );//IMPL {return  var_lt_dbl(lhs,   dbl2      ); }
	//PUBLIC VAR_FRIEND bool operator<(CBX         lhs,   const bool   bool1 );//IMPL {return  bool_lt_bool(lhs, bool1     ); }

//	template<class LHS>
//	PUBLIC VAR_FRIEND bool operator<(CBX LHS,             CBX          rhs   );//IMPL {return  var_lt_var(lhs,        rhs); }
	PUBLIC VAR_FRIEND bool operator<(const char*   cstr1, CBX          rhs   );//IMPL {return  var_lt_var(VAR(cstr1), rhs); }
	PUBLIC VAR_FRIEND bool operator<(const char    char1, CBX          rhs   );//IMPL {return  var_lt_var(VAR(char1), rhs); }
	PUBLIC VAR_FRIEND bool operator<(const int     int1,  CBX          rhs   );//IMPL {return  int_lt_var(int1,       rhs); }
	PUBLIC VAR_FRIEND bool operator<(const double  dbl1,  CBX          rhs   );//IMPL {return  dbl_lt_var(dbl1,       rhs); }
	//PUBLIC VAR_FRIEND bool operator<(const bool  bool1, CBX          rhs   );//IMPL {return  bool_lt_bool(bool1,    rhs ); }

	// >= GE friends v. main types

//	template<class RHS>
//	PUBLIC VAR_FRIEND bool operator>=(CBX          lhs,   RHS          rhs   );//IMPL {return !var_lt_var(lhs,   rhs       ); }
	PUBLIC VAR_FRIEND bool operator>=(CBX          lhs,   const char*  cstr2 );//IMPL {return !var_lt_var(lhs,   VAR(cstr2)); }
	PUBLIC VAR_FRIEND bool operator>=(CBX          lhs,   const char   char2 );//IMPL {return !var_lt_var(lhs,   VAR(char2)); }
	PUBLIC VAR_FRIEND bool operator>=(CBX          lhs,   const int    int2  );//IMPL {return !var_lt_int(lhs,   int2      ); }
	PUBLIC VAR_FRIEND bool operator>=(CBX          lhs,   const double dbl2  );//IMPL {return !var_lt_dbl(lhs,   dbl2      ); }
	//PUBLIC VAR_FRIEND bool operator>=(CBX        lhs,   const bool   bool2 );//IMPL {return !bool_lt_bool(lhs, bool2     ); }

//	template<class LHS>
//	PUBLIC VAR_FRIEND bool operator>=(LHS lhs,            CBX          rhs   );//IMPL {return !var_lt_var(lhs,        rhs); }
	PUBLIC VAR_FRIEND bool operator>=(const char*  cstr1, CBX          rhs   );//IMPL {return !var_lt_var(VAR(cstr1), rhs); }
	PUBLIC VAR_FRIEND bool operator>=(const char   char1, CBX          rhs   );//IMPL {return !var_lt_var(VAR(char1), rhs); }
	PUBLIC VAR_FRIEND bool operator>=(const int    int1,  CBX          rhs   );//IMPL {return !int_lt_var(int1,       rhs); }
	PUBLIC VAR_FRIEND bool operator>=(const double dbl1,  CBX          rhs   );//IMPL {return !dbl_lt_var(dbl1,       rhs); }
	//PUBLIC VAR_FRIEND bool operator>=(const bool bool1, CBX          rhs   );//IMPL {return !bool_lt_bool(bool1,    rhs); }

	// > GT friends v. main types

//	template<class RHS>
//	PUBLIC VAR_FRIEND bool operator>(CBX           lhs,   RHS          rhs   );//IMPL {return  var_lt_var(rhs,        lhs); }
	PUBLIC VAR_FRIEND bool operator>(CBX           lhs,   const char*  cstr2 );//IMPL {return  var_lt_var(VAR(cstr2), lhs); }
	PUBLIC VAR_FRIEND bool operator>(CBX           lhs,   const char   char2 );//IMPL {return  var_lt_var(VAR(char2), lhs); }
	PUBLIC VAR_FRIEND bool operator>(CBX           lhs,   const int    int2  );//IMPL {return  int_lt_var(int2,       lhs); }
	PUBLIC VAR_FRIEND bool operator>(CBX           lhs,   const double dbl2  );//IMPL {return  dbl_lt_var(dbl2,       lhs); }
	//PUBLIC VAR_FRIEND bool operator>(CBX         lhs,   const bool   bool2 );//IMPL {return  bool_lt_bool(bool2,    lhs); }

//	template<class LHS>
//	PUBLIC VAR_FRIEND bool operator>(LHS lhs,             CBX          rhs   );//IMPL {return  var_lt_var(rhs,   lhs       ); }
	PUBLIC VAR_FRIEND bool operator>(const char*   cstr1, CBX          rhs   );//IMPL {return  var_lt_var(rhs,   VAR(cstr1)); }
	PUBLIC VAR_FRIEND bool operator>(const char    char1, CBX          rhs   );//IMPL {return  var_lt_var(rhs,   VAR(char1)); }
	PUBLIC VAR_FRIEND bool operator>(const int     int1,  CBX          rhs   );//IMPL {return  var_lt_int(rhs,   int1      ); }
	PUBLIC VAR_FRIEND bool operator>(const double  dbl1,  CBX          rhs   );//IMPL {return  var_lt_dbl(rhs,   dbl1      ); }
	//PUBLIC VAR_FRIEND bool operator>(const bool  bool1, CBX          rhs   );//IMPL {return  bool_lt_bool(rhs, bool1     ); }

	// <= LE friends v. main types

//	template<class RHS>
//	PUBLIC VAR_FRIEND bool operator<=(CBX          lhs,   RHS          rhs   );//IMPL {return !var_lt_var(rhs,        lhs); }
	PUBLIC VAR_FRIEND bool operator<=(CBX          lhs,   const char*  cstr2 );//IMPL {return !var_lt_var(VAR(cstr2), lhs); }
	PUBLIC VAR_FRIEND bool operator<=(CBX          lhs,   const char   char2 );//IMPL {return !var_lt_var(VAR(char2), lhs); }
	PUBLIC VAR_FRIEND bool operator<=(CBX          lhs,   const int    int2  );//IMPL {return !int_lt_var(int2,       lhs); }
	PUBLIC VAR_FRIEND bool operator<=(CBX          lhs,   const double dbl2  );//IMPL {return !dbl_lt_var(dbl2,       lhs); }
	//PUBLIC VAR_FRIEND bool operator<=(CBX        lhs,   const bool   bool2 );//IMPL {return !bool_lt_bool(bool2,    lhs); }

//	template<class LHS>
//	PUBLIC VAR_FRIEND bool operator<=(LHS           lhs,   CBX          rhs   );//IMPL {return !var_lt_var(rhs,   lhs       ); }
	PUBLIC VAR_FRIEND bool operator<=(const char*   cstr1, CBX          rhs   );//IMPL {return !var_lt_var(rhs,   VAR(cstr1)); }
	PUBLIC VAR_FRIEND bool operator<=(const char    char1, CBX          rhs   );//IMPL {return !var_lt_var(rhs,   VAR(char1)); }
	PUBLIC VAR_FRIEND bool operator<=(const int     int1,  CBX          rhs   );//IMPL {return !var_lt_int(rhs,   int1      ); }
	PUBLIC VAR_FRIEND bool operator<=(const double  dbl1,  CBX          rhs   );//IMPL {return !var_lt_dbl(rhs,   dbl1      ); }
	//PUBLIC VAR_FRIEND bool operator<=(const bool  bool1, CBX          rhs   );//IMPL {return !bool_lt_bool(rhs, bool1     ); }

	// Use nrvo (named return value optimisation) to ensure fastest implementation
	//
	// https://godbolt.org/z/Go6EWqr8E

	// Arithmetic friends for var and var (not templated, to prevent "error: ambiguous overload"
	PUBLIC VAR_FRIEND RETVAR  operator+(CBX  lhs, CBX rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo += rhs; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR  operator*(CBX  lhs, CBX rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo *= rhs; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR  operator-(CBX  lhs, CBX rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo -= rhs; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR  operator/(CBX  lhs, CBX rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo /= rhs; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR  operator%(CBX  lhs, CBX rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo %= rhs; return nrvo;}

	// Concatenation friends for var and var

//	PUBLIC VAR_FRIEND RETVAR operator^(CBX lhs, CBX rhs);//IMPL {return var_cat_var(lhs, rhs ); }
	//PUBLIC VAR_FRIEND RETVAR operator^(TBX lhs, CBX rhs);//IMPL {return lhs ^= rhs ; }
//	PUBLIC VAR_FRIEND RETVAR operator^(TBX lhs, CBX rhs);//IMPL {return static_cast<RETVAR>(lhs ^= rhs) ; } //var_base -> var
//	template<typename T> PUBLIC VAR_FRIEND RETVAR operator^(const T&  lhs, const T& rhs);//IMPL {return var_cat_var(lhs, rhs ); }
//	template<typename T> PUBLIC VAR_FRIEND RETVAR operator^(      T&& lhs, const T& rhs);//IMPL {return static_cast<RETVAR>(lhs ^= rhs) ; } //var_base -> va

	// TODO review all operators are doing minimal copying/constructing

#undef TBX
#define TBX var_base<var_mid<exo::var>>&&
//	PUBLIC VAR_FRIEND RETVAR operator^(CBX lhs,CBX rhs);//IMPL {return var_cat_var(VARBASE(lhs), rhs ); }
	PUBLIC VAR_FRIEND RETVAR operator^(CBX lhs,CBX rhs);//IMPL {var lhs2 = lhs.toString(); lhs2 ^= rhs; return lhs2; }
	PUBLIC VAR_FRIEND RETVAR operator^(TBX lhs,CBX rhs);//IMPL {lhs ^= rhs; return *static_cast<var*>(&lhs); }
//	PUBLIC VAR_FRIEND RETVAR operator^(TBX lhs,CBX rhs);//IMPL {return var_cat_var(lhs, rhs); }

//	//template<typename T> PUBLIC VAR_FRIEND RETVAR operator^(const T&   lhs,       CBX rhs);//IMPL {var_base<var> nrvo = lhs; return var_cat_var(nrvo, rhs ); }
//	template<typename T> PUBLIC VAR_FRIEND RETVAR operator^(const T&   lhs,       CBX rhs);//IMPL {return var_cat_var(VARBASE(lhs), rhs ); }
//
////	template<typename T> PUBLIC VAR_FRIEND RETVAR operator^(      T&&  lhs,       CBX rhs);//IMPL {return static_cast<RETVAR>(T(lhs) ^= rhs) ; } //var_base -> va
//
//	template<typename T> PUBLIC VAR_FRIEND RETVAR operator^(      T&&  lhs,       CBX rhs);//IMPL {return static_cast<RETVAR>(VARBASE(lhs) ^= rhs) ; } //var_base -> va

//	template<typename T> PUBLIC VAR_FRIEND RETVAR operator^(      CBX  lhs, const T&  rhs);//IMPL {return var_cat_var(lhs, rhs ); }
//	template<typename T> PUBLIC VAR_FRIEND RETVAR operator^(      T&&  lhs, const T&  rhs);//IMPL {return static_cast<RETVAR>(T(lhs) ^= rhs) ; } //var_base -> va

	//PUBLIC VAR_FRIEND RETVAR operator^(const std::string &  lhs, CBX rhs);//IMPL {return static_cast<RETVAR>(RETVAR(lhs) ^= rhs); }
//	PUBLIC VAR_FRIEND RETVAR operator^(std::string &&  lhs, CBX rhs);//IMPL {return static_cast<RETVAR>(RETVAR(lhs) ^= rhs); }

	// ARITHMETIC friends v. main types

	// PLUS  friends v. main types

#if 00 // Templates before var became the template var_base<var>
	template<class T> PUBLIC VAR_FRIEND RETVAR operator+(CBX lhs, const T rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo += rhs; return nrvo;}
	template<class T> PUBLIC VAR_FRIEND RETVAR operator-(CBX lhs, const T rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo -= rhs; return nrvo;}
	template<class T> PUBLIC VAR_FRIEND RETVAR operator*(CBX lhs, const T rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo *= rhs; return nrvo;}
	template<class T> PUBLIC VAR_FRIEND RETVAR operator/(CBX lhs, const T rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo /= rhs; return nrvo;}
	template<class T> PUBLIC VAR_FRIEND RETVAR operator%(CBX lhs, const T rhs);//IMPL {RETVAR nrvo = lhs.clone(); nrvo %= rhs; return nrvo;}

//	template<class T> PUBLIC VAR_FRIEND RETVAR operator+(const T lhs, CBX rhs);//IMPL {RETVAR nrvo = rhs.clone(); nrvo += lhs; return nrvo;}
//	template<class T> PUBLIC VAR_FRIEND RETVAR operator*(const T lhs, CBX rhs);//IMPL {RETVAR nrvo = rhs.clone(); nrvo *= lhs; return nrvo;}
	template<class T> PUBLIC VAR_FRIEND RETVAR operator-(const T lhs, CBX rhs);//IMPL {RETVAR nrvo = -rhs;        nrvo += lhs; return nrvo;}
	template<class T> PUBLIC VAR_FRIEND RETVAR operator/(const T lhs, CBX rhs);//IMPL {RETVAR nrvo =  lhs;        nrvo /= rhs; return nrvo;}
	template<class T> PUBLIC VAR_FRIEND RETVAR operator%(const T lhs, CBX rhs);//IMPL {RETVAR nrvo =  lhs;        nrvo %= rhs; return nrvo;}
#else
	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const char*  cstr2 );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    += cstr2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const char   char2 );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    += char2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const int    int2  );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    += int2;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const double dbl2  );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    += dbl2;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const bool   bool2 );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    += bool2; return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const bool   bool2 );//IMPL {if (bool2) return lhs + 1; else return static_cast<var>(lhs);}
//	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const bool   bool2 );//IMPL {if (bool2) return lhs + 1; else return dynamic_cast<var>(lhs);}
//	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const bool   bool2 );//IMPL {if (bool2) return lhs + 1; else return lhs + 0;}
//	PUBLIC VAR_FRIEND RETVAR operator+(CBX            lhs,   const bool   bool2 );//IMPL {if (bool2) return lhs + 1; else return 3;}

	PUBLIC VAR_FRIEND RETVAR operator+(const char*    cstr1, CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    += cstr1; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator+(const char     char1, CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    += char1; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator+(const int      int1,  CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    += int1;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator+(const double   dbl1,  CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    += dbl1;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator+(const bool     bool1, CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    += bool1; return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator+(const bool     bool1, CBX          rhs   );//IMPL {if (bool1) return rhs + 1; else return static_cast<var>(rhs);}

	// MULTIPLY  friends v. main types
	//////////////////////////////////

	PUBLIC VAR_FRIEND RETVAR operator*(CBX            lhs,   const char*  cstr2 );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    *= cstr2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator*(CBX            lhs,   const char   char2 );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    *= char2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator*(CBX            lhs,   const int    int2  );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    *= int2;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator*(CBX            lhs,   const double dbl2  );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    *= dbl2;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator*(CBX            lhs,   const bool   bool2 );//IMPL {RETVAR nrvo = lhs.clone(); nrvo    *= bool2; return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator*(CBX            lhs,   const bool   bool2 );//IMPL {if (bool2) return static_cast<var>(lhs); else return 0;}

	PUBLIC VAR_FRIEND RETVAR operator*(const char*    cstr1, CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    *= cstr1; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator*(const char     char1, CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    *= char1; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator*(const int      int1,  CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    *= int1;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator*(const double   dbl1,  CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    *= dbl1;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator*(const bool     bool1, CBX          rhs   );//IMPL {RETVAR nrvo = rhs.clone(); nrvo    *= bool1; return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator*(const bool     bool1, CBX          rhs   );//IMPL {if (bool1) return static_cast<var>(rhs); else return 0;}

	// MINUS  friends v. main types
	///////////////////////////////

	PUBLIC VAR_FRIEND RETVAR operator-(CBX            lhs,   const char*  cstr2 );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  -= cstr2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator-(CBX            lhs,   const char   char2 );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  -= char2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator-(CBX            lhs,   const int    int2  );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  -= int2;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator-(CBX            lhs,   const double dbl2  );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  -= dbl2;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator-(CBX            lhs,   const bool   bool2 );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  -= bool2; return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator-(CBX            lhs,   const bool   bool2 );//IMPL {RETVAR nrvo = lhs.clone();   if (bool2) nrvo  -= 1; return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator-(CBX            lhs,   const bool   bool2 );//IMPL {if (bool2) return lhs - 1; else return static_cast<var>(lhs);}

//	PUBLIC VAR_FRIEND RETVAR operator-(const char*    cstr1, CBX          rhs   );//IMPL {RETVAR nrvo = cstr1; nrvo  -= rhs;   return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const char     char1, CBX          rhs   );//IMPL {RETVAR nrvo = char1; nrvo  -= rhs;   return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const int      int1,  CBX          rhs   );//IMPL {RETVAR nrvo = int1;  nrvo  -= rhs;   return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const double   dbl1,  CBX          rhs   );//IMPL {RETVAR nrvo = dbl1;  nrvo  -= rhs;   return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const bool     bool1, CBX          rhs   );//IMPL {RETVAR nrvo = bool1; nrvo  -= rhs;   return nrvo;}

//	PUBLIC VAR_FRIEND RETVAR operator-(const char*    cstr1, CBX          rhs   );//IMPL {return -rhs + cstr1;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const char     char1, CBX          rhs   );//IMPL {return -rhs + char1;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const int      int1,  CBX          rhs   );//IMPL {return -rhs + int1;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const double   dbl1,  CBX          rhs   );//IMPL {return -rhs + dbl1;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const bool     bool1, CBX          rhs   );//IMPL {return -rhs + bool;}

	PUBLIC VAR_FRIEND RETVAR operator-(const char*    cstr1, CBX          rhs   );//IMPL {RETVAR nrvo = -rhs; nrvo += cstr1; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator-(const char     char1, CBX          rhs   );//IMPL {RETVAR nrvo = -rhs; nrvo += char1; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator-(const int      int1,  CBX          rhs   );//IMPL {RETVAR nrvo = -rhs; nrvo += int1;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator-(const double   dbl1,  CBX          rhs   );//IMPL {RETVAR nrvo = -rhs; nrvo += dbl1;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator-(const bool     bool1, CBX          rhs   );//IMPL {RETVAR nrvo = -rhs; nrvo += bool1; return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator-(const bool     bool1, CBX          rhs   );//IMPL {if (bool1) return (-rhs) + 1; else return -rhs;}

	// DIVIDE  friends v. main types

	PUBLIC VAR_FRIEND RETVAR operator/(CBX            lhs,   const char*  cstr2 );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  /= cstr2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator/(CBX            lhs,   const char   char2 );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  /= char2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator/(CBX            lhs,   const int    int2  );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  /= int2;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator/(CBX            lhs,   const double dbl2  );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  /= dbl2;  return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator/(CBX            lhs,   const bool   bool2 );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  /= bool2; return nrvo;} // Either does nothing or throws divide by zero

	PUBLIC VAR_FRIEND RETVAR operator/(const char*    cstr1, CBX          rhs   );//IMPL {RETVAR nrvo = cstr1; nrvo  /= rhs;   return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator/(const char     char1, CBX          rhs   );//IMPL {RETVAR nrvo = char1; nrvo  /= rhs;   return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator/(const int      int1,  CBX          rhs   );//IMPL {RETVAR nrvo = int1;  nrvo  /= rhs;   return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator/(const double   dbl1,  CBX          rhs   );//IMPL {RETVAR nrvo = dbl1;  nrvo  /= rhs;   return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator/(const bool     bool1, CBX          rhs   );//IMPL {RETVAR nrvo = bool1 ;nrvo /= rhs;   return nrvo;} // Almost meaningless

	// MODULO  friends v. main types

	PUBLIC VAR_FRIEND RETVAR operator%(CBX            lhs,   const char*  cstr2 );//IMPL {RETVAR nrvo = lhs.clone(); nrvo  %= cstr2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator%(CBX            lhs,   const char   char2 );//IMPL {RETVAR nrvo = lhs.clone(); nrvo  %= char2; return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator%(CBX            lhs,   const int    int2  );//IMPL {RETVAR nrvo = lhs.clone(); nrvo  %= int2;  return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator%(CBX            lhs,   const double dbl2  );//IMPL {RETVAR nrvo = lhs.clone(); nrvo  %= dbl2;  return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator%(CBX            lhs,   const bool   bool2 );//IMPL {RETVAR nrvo = lhs.clone();   nrvo  %= bool2; return nrvo;} // Rather useless or throws divide by zero

	PUBLIC VAR_FRIEND RETVAR operator%(const char*    cstr1, CBX          rhs   );//IMPL {RETVAR nrvo = cstr1; nrvo  %= rhs;   return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator%(const char     char1, CBX          rhs   );//IMPL {RETVAR nrvo = char1; nrvo  %= rhs;   return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator%(const int      int1,  CBX          rhs   );//IMPL {RETVAR nrvo = int1;  nrvo  %= rhs;   return nrvo;}
	PUBLIC VAR_FRIEND RETVAR operator%(const double   dbl1,  CBX          rhs   );//IMPL {RETVAR nrvo = dbl1;  nrvo  %= rhs;   return nrvo;}
//	PUBLIC VAR_FRIEND RETVAR operator%(const bool   bool1, CBX          rhs   );//IMPL {RETVAR nrvo = bool1 ;nrvo  %= rhs;   return nrvo;} // Almost meaningless

#endif

	// STRING CONCATENATE  friends v. main types

	// NB do *NOT* support concatenate with bool or vice versa!
	// to avoid compiler doing wrong precendence issue between ^ and logical operators
	//remove this to avoid some gcc ambiguous warnings although it means concat std::string will create a temp RETVAR

	PUBLIC VAR_FRIEND RETVAR operator^(      CBX     lhs, const char*   rhs);//IMPL {lhs.assertString(__PRETTY_FUNCTION__); return lhs.var_str + rhs;}
	PUBLIC VAR_FRIEND RETVAR operator^(      CBX     lhs, const char    rhs);//IMPL {lhs.assertString(__PRETTY_FUNCTION__); return lhs.var_str + rhs;}
	PUBLIC VAR_FRIEND RETVAR operator^(      CBX     lhs, const int     rhs);//IMPL {lhs.assertString(__PRETTY_FUNCTION__); return lhs.var_str + RETVAR(rhs).toString();}
	PUBLIC VAR_FRIEND RETVAR operator^(      CBX     lhs, const double  rhs);//IMPL {lhs.assertString(__PRETTY_FUNCTION__); return lhs.var_str + RETVAR(rhs).toString();}
	PUBLIC VAR_FRIEND RETVAR operator^(const char*   lhs,       CBX     rhs);//IMPL {rhs.assertString(__PRETTY_FUNCTION__); return lhs + rhs.var_str;}
	PUBLIC VAR_FRIEND RETVAR operator^(const char    lhs,       CBX     rhs);//IMPL {rhs.assertString(__PRETTY_FUNCTION__); return lhs + rhs.var_str;}
	PUBLIC VAR_FRIEND RETVAR operator^(const int     lhs,       CBX     rhs);//IMPL {rhs.assertString(__PRETTY_FUNCTION__); return RETVAR(lhs).toString() += rhs.var_str;}
	PUBLIC VAR_FRIEND RETVAR operator^(const double  lhs,       CBX     rhs);//IMPL {rhs.assertString(__PRETTY_FUNCTION__); return RETVAR(lhs).toString() += rhs.var_str;}

#undef TBX
#define TBX var_base<var_mid<exo::var>>&&
//#define TBX var_base&&
//#define TBR var_base<var_mid<var>>&&
//#define TBR var_base&
	//temporaries (rvalues) - mutate the temporary string to save a copy
	PUBLIC VAR_FRIEND RETVAR operator^(TBX    lhs, const char*  rhs);//IMPL {lhs ^= rhs; return std::move(lhs.var_str);}
	PUBLIC VAR_FRIEND RETVAR operator^(TBX    lhs, const char   rhs);//IMPL {lhs ^= rhs; return std::move(lhs.var_str);}
	PUBLIC VAR_FRIEND RETVAR operator^(TBX    lhs, const int    rhs);//IMPL {lhs ^= rhs; return std::move(lhs.var_str);}
	PUBLIC VAR_FRIEND RETVAR operator^(TBX    lhs, const double rhs);//IMPL {lhs ^= rhs; return std::move(lhs.var_str);}
