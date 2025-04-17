#ifndef EXO_VARTYP_CPP
#define EXO_VARTYP_CPP

// Module
//
#define EXO_EXPORT export
#	ifdef EXO_EXPORT
//	EXO_EXPORT module vartyp;
	export module vartyp;
#endif

//EXO_EXPORT
export namespace exo {
//namespace exo {

#define ND [[nodiscard]]

class VARTYP {

 private:

	// bit 1 string
	// bit 2 int
	// bit 3 double
	// bit 4 nan
	// bit 5 osfile
	unsigned int flags_ = 0;

 public:

	// default constructor
	constexpr
	VARTYP() = default;
	//VARTYP() : flags(0) {
	//    std::cout<<"def ctor2"
	//    << std::endl;
	//};

	// copy constructor
	// warning: definition of implicit copy assignment operator for 'VARTYP' is deprecated because it has a user-declared copy constructor [-Wdeprecated-copy]
	constexpr
	VARTYP(const VARTYP& rhs) = default;
	//VARTYP(const VARTYP& rhs) : flags_(rhs.flags_) {
	//    std::cout<<"copy ctor "<< rhs.flags
	//    << std::endl;
	//};

	// copy assignment
	constexpr
	VARTYP& operator=(const VARTYP& rhs) = default;

	// constructor from unsigned int
	constexpr
	VARTYP(const unsigned int rhs)
		: flags_(rhs){}

	// assign from unsigned int
	constexpr
	VARTYP& operator=(const unsigned int rhs) {
		flags_ = rhs;
		return *this;
	}

	// bitwise mutators: xor, or, and
	constexpr
	VARTYP& operator^=(const unsigned int rhs) {flags_ ^= rhs; return *this; }
	constexpr
	VARTYP& operator|=(const unsigned int rhs) {flags_ |= rhs; return *this; }
	constexpr
	VARTYP& operator&=(const unsigned int rhs) {flags_ &= rhs; return *this; }

	// logical comparison with int and self
	ND constexpr bool operator==(const unsigned int rhs) const { return flags_ == rhs; }
	ND constexpr bool operator!=(const unsigned int rhs) const { return flags_ != rhs; }
	ND constexpr bool operator==(const VARTYP rhs) const { return flags_ == rhs.flags_; }
	ND constexpr bool operator!=(const VARTYP rhs) const { return flags_ != rhs.flags_; }

	// bitwise accessors
	constexpr VARTYP operator&(const unsigned int rhs) const { return flags_ & rhs; }
	constexpr VARTYP operator|(const unsigned int rhs) const { return flags_ | rhs; }
	constexpr VARTYP operator~() const { return VARTYP(~flags_); }

	// int - not explicit so we can do "if (var_typ)"
	constexpr operator int() const { return flags_; }

}; // class VARTYP

// WARNING these VARTYP constants must be initialised before any var variables are
// NOT they are declared inline which presumably makes them the same in all compilation units
// but it also seems to ensure that they are initialised BEFORE any var variables
// theoretically they should be because C++ says
// this is mandatory because initialising var variables REQUIRES these constants to be available
// with their correct values otherwise any such var, when used later on, will throw Unassigned
// Variable Used since its var_typ will be zero

// throw an exception if used an unassigned variable
constexpr unsigned int VARTYP_UNA {0x0};

// assigned string - unknown if numeric or not
constexpr unsigned int VARTYP_STR {0x1};

// following indicate that the var is numeric
constexpr unsigned int VARTYP_INT {0x2};
constexpr unsigned int VARTYP_DBL {0x4};

// indicates known non-numeric string
constexpr unsigned int VARTYP_NAN {0x8};

// following indicates that the int is an os file handle
constexpr unsigned int VARTYP_OSFILE {0x10};
//constexpr unsigned int VARTYP_DBCONN {0x20};

// various useful flag combinations
constexpr unsigned int VARTYP_INTDBL {VARTYP_INT | VARTYP_DBL};
constexpr unsigned int VARTYP_INTSTR {VARTYP_INT | VARTYP_STR};
constexpr unsigned int VARTYP_DBLSTR {VARTYP_DBL | VARTYP_STR};
constexpr unsigned int VARTYP_NANSTR {VARTYP_NAN | VARTYP_STR};
constexpr unsigned int VARTYP_NOTNUMFLAGS {~(VARTYP_INT | VARTYP_DBL | VARTYP_NAN)};

constexpr unsigned int VARTYP_NANSTR_OSFILE {VARTYP_NANSTR | VARTYP_OSFILE};
//constexpr unsigned int VARTYP_NANSTR_DBCONN {VARTYP_NANSTR | VARTYP_DBCONN};

//constexpr unsigned int VARTYP_DESTRUCTED {0xFFFFF0};

constexpr unsigned int VARTYP_MASK {~(VARTYP_STR | VARTYP_NAN | VARTYP_INT | VARTYP_DBL | VARTYP_OSFILE | VARTYP_OSFILE)};

} // namespace exo

#endif // EXO_VARTYP_CPP