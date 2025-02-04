#ifndef VAR_ASCII_H
#define VAR_ASCII_H

//#include <cstdint>

// Thread-safe ASCII-only isdigit function
inline bool ASCII_isdigit(char c) noexcept {
	return c >= '0' && c <= '9';
}

// Thread-safe ASCII-only isalpha function
inline bool ASCII_isalpha(char c) noexcept {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// Thread-safe ASCII-only toupper function
inline char ASCII_toupper(char c) noexcept {
	// Focus on lower case ASAP by testing >= 'a' 1st
	// 0x41 = A
	// 0x61 = a
	if (c >= 'a' && c <= 'z') {
		// LIKELY   // Stops clang 18 optimising ucase when no conversion is required
		// UNLIKELY // No change in performance
		return static_cast<char>(c - 32);
	}
	return c;
}

#endif