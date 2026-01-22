#if EXO_MODULE > 1
	import std;
#else
#	include <utility>
#	include <iostream>
#endif

#include "DBresult.h"

namespace exo {

#define LOG if (0) std::cerr

// Constructor from PGresult*
DBresult::DBresult(PGresult* pgresult) noexcept : pgresult_(pgresult) {
	if (pgresult_)
		LOG << "DBresult (c) own " << pgresult_ << std::endl;
}

// Move constructor
DBresult::DBresult(DBresult&& other) noexcept
	: pgresult_(std::exchange(other.pgresult_, nullptr)),
	  rown_(std::exchange(other.rown_, 0)),
	  pqerrmsg(std::move(other.pqerrmsg)) {
	if (pgresult_)
		LOG << "DBresult (m) own " << pgresult_ << std::endl;
}

// Move assignment
DBresult& DBresult::operator=(DBresult&& other) noexcept {
	if (this != &other) {
		if (pgresult_) {
			LOG << "DBresult (=m) PQC " << pgresult_ << std::endl;
			PQclear(pgresult_);
		}
		pgresult_ = std::exchange(other.pgresult_, nullptr);
		rown_ = std::exchange(other.rown_, 0);
		pqerrmsg = std::move(other.pqerrmsg);
	}
	return *this;
}

// Assignment from PGresult*
DBresult& DBresult::operator=(PGresult* pgresult) noexcept {
	if (pgresult_ != pgresult) {
		if (pgresult_) {
			LOG << "DBresult (=) PQC " << pgresult_ << std::endl;
			PQclear(pgresult_);
		}
		pgresult_ = pgresult;
		rown_ = 0;
		pqerrmsg = "";
		if (pgresult_)
			LOG << "DBresult (=) own " << pgresult_ << std::endl;
	}
	return *this;
}

// Destructor
DBresult::~DBresult() {
	if (pgresult_) {
		LOG << "DBresult (~) PQC " << pgresult_ << std::endl;
		PQclear(pgresult_);
	}
}

// Release ownership
PGresult* DBresult::release() noexcept {
	rown_ = 0;
	pqerrmsg = "";
	return std::exchange(pgresult_, nullptr);
}

} // namespace exo
