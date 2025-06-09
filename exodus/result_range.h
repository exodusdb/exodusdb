#ifndef RANGE_H
#define RANGE_H

#define PUBLIC __attribute__((visibility("default")))

namespace exo {

template<class T>
class PUBLIC ResultRange {
	T value_;
	class JobManager* jm; // Pointer to JobManager for queue and run_count access

public:
	ResultRange(JobManager* manager);

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