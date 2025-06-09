#ifndef RANGE_H
#define RANGE_H

// ResultRange is only used in pre Ubuntu 24.04 c++ builds where the much simpler std::generator is not available.
// It enables range based for loops over Job and Task results.

#define PUBLIC __attribute__((visibility("default")))

namespace exo {

template<class U, class T>
class PUBLIC ResultRange {
	T value_;
	U* manager_; // Pointer to JobManager/TaskManager for queue and run_count access etc.

public:
	ResultRange(U* manager);

	class Iterator {
		ResultRange* range_;
	public:
		Iterator(ResultRange* range);
		T& operator*() const;
		Iterator& operator++();
		bool operator!=(const Iterator& other) const;
	};

	Iterator begin();
	Iterator end() const;
};

} // namespace exo

#endif