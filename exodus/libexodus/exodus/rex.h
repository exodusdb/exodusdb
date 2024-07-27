#ifndef EXODUS_VARREX_H
#define EXODUS_VARREX_H

#include <exodus/var.h>

//warning: inlining failed in call to ‘exo::rex::~rex() noexcept’: call is unlikely and code size would grow [-Winline]
#pragma GCC diagnostic push
#if __GNUC__ <= 10
#	pragma GCC diagnostic ignored "-Winline"
#endif

namespace exo {

//class rex final
class PUBLIC rex final {

friend class var;

 private:
	mutable var   regex_str_ = "";
	mutable var   options_   = "";
	mutable void* pimpl_     = nullptr;

	// Default constructor
	rex();

 public:

	// Constructor
	rex(SV regex_str, SV options = "");

	// Destructor - deletes pimpl_
	~rex();

	// User defined literal "_rex" e.g. "[a-z]"_rex
	friend rex operator""_rex(const char* cstr, std::size_t size);

}; // class rex

PUBLIC rex operator""_rex(const char* cstr, std::size_t size);

} // namespace exo

#pragma GCC diagnostic pop

#endif //EXODUS_VARREX_H
