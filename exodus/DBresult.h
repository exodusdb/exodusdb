#ifndef DBRESULT_H
#define DBRESULT_H

#ifdef EXO_MODULE
	import std;
#else
#	include <string>
#endif

#include <libpq-fe.h> // For PGresult and PQclear

namespace exo {

// Wrapper for PGresult plus additional data.
// RAII smart pointer with PQclear in dtor.
class DBresult {
public:
	PGresult* pgresult_ = nullptr;	// Owned PGresult pointer
	int rown_ = 0;					// Current row index
	std::string pqerrmsg = "";		// Error message storage

public:
	// Default constructor
	DBresult() noexcept = default;

	// Constructor from PGresult*
	DBresult(PGresult* pgresult) noexcept;

	// No copy constructor or assignment
	DBresult(const DBresult&) = delete;
	DBresult& operator=(const DBresult&) = delete;

	// Move constructor
	DBresult(DBresult&& other) noexcept;

	// Move assignment
	DBresult& operator=(DBresult&& other) noexcept;

	// Assignment from PGresult*
	DBresult& operator=(PGresult* pgresult) noexcept;

	// Destructor
	~DBresult();

	// Implicit conversion to PGresult*
	operator PGresult*() const noexcept { return pgresult_; }

	// Accessor for raw pointer
	PGresult* get() const noexcept { return pgresult_; }

	// Release ownership without clearing
	PGresult* release() noexcept;

	// Check if holding a valid pointer
	explicit operator bool() const noexcept { return pgresult_ != nullptr; }

};

} // namespace exo

#endif // DBRESULT_H