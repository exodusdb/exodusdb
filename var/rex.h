#ifndef EXODUS_VARREX_H
#define EXODUS_VARREX_H

#include <var/var.h>

//warning: inlining failed in call to ‘exo::rex::~rex() noexcept’: call is unlikely and code size would grow [-Winline]
#pragma GCC diagnostic push
#if __GNUC__ <= 10
#	pragma GCC diagnostic ignored "-Winline"
#endif

namespace exo {

//class rex final
class PUBLIC rex final {

friend class var;
friend class var_stg;
friend class var_base;

 private:
	mutable var   regex_str_ = "";
	mutable var   options_   = "";
	mutable void* pimpl_     = nullptr;

	// Default constructor
	rex();

 public:

	// Constructor to create a regex engine on the heap
	rex(SV regex_str, SV options = "");

	// Destructor to delete the regex engine from the heap
	~rex();

	// Constructor from char* and size. Used by user defined literal _rex
	// bool arg is only to disambiguate this constructor from one above
	rex(const char* cstr, std::size_t size, bool) : regex_str_(cstr, size) {}

	// User defined literal "_rex" e.g. "[a-z]"_rex
	friend rex operator""_rex(const char* cstr, std::size_t size);

}; // class rex

PUBLIC rex operator""_rex(const char* cstr, std::size_t size);

} // namespace exo

#pragma GCC diagnostic pop

#endif //EXODUS_VARREX_H
