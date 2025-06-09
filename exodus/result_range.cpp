#include "result_range.h"
#include "job_manager.h" // Assumed to define JobManager, ExoEnv, and queue

namespace exo {

template<class T>
ResultRange<T>::ResultRange(JobManager* manager)
	: jm(manager), value_() {}

template<class T>
ResultRange<T>::Iterator::Iterator(ResultRange* range)
	: range_(range) {}

template<class T>
T& ResultRange<T>::Iterator::operator*() const {
	return range_->value_;
}

template<class T>
typename ResultRange<T>::Iterator& ResultRange<T>::Iterator::operator++() {
	if (range_->jm->run_count() > 0) {
		range_->jm->result_queue()->wait_and_pop(range_->value_);
		range_->jm->decrement_run_count();
	}
	return *this;
}

template<class T>
bool ResultRange<T>::Iterator::operator!=(const Iterator& other) const {
	return range_ && range_->jm->run_count() > 0;
}

template<class T>
typename ResultRange<T>::Iterator ResultRange<T>::begin() {
	if (jm->run_count() == 0) {
		return Iterator(this);
	}
	jm->result_queue()->wait_and_pop(value_);
	jm->decrement_run_count();
	return Iterator(this);
}

template<class T>
typename ResultRange<T>::Iterator ResultRange<T>::end() const {
	return Iterator(const_cast<ResultRange*>(this));
}

template class ResultRange<ExoEnv>;

} // namespace exo