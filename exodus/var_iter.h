#ifndef VAR_ITER_H
#define VAR_ITER_H

/////////////////////////////////
// var_iter - iterate over fields
/////////////////////////////////

#include <exodus/varb.h>

namespace exo {

class PUBLIC var_iter {

	using in = const RETVAR&;

 private:
	const RETVAR* pvar_;
	mutable std::size_t startpos_ = 0;
	// No longer storing endpos so the user can AMEND the current field
	// and even perhaps decrement an iter after deleting a field.
//	mutable std::size_t endpos_ = std::string::npos;

 public:
	using value_type = var*;

	// Default constructor
	var_iter() = default;

	// Construct from var
	explicit var_iter(in v);

//	var_iter(var_iter&) = delete;
//	var_iter(var_iter&&) = delete;

	// Check iter != iter (i.e. iter != string::npos)
	   bool operator!=(const var_iter& vi);

	// Convert to var (copy)
	RETVAR operator*() const;

//	// Convert to var reference
//	var&  operator*();

	// ++Iter prefix
	var_iter& operator++();

	// Iter++ postfix
	var_iter operator++(int);

	// --Iter prefix
	var_iter& operator--();

	// iter++ postfix
	var_iter operator--(int);

	// assign
	//void operator=(SV replacement);

}; // class var_iter

} // namespace exo

#endif // VAR_ITER_H