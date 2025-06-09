#ifndef EXO_MODULE // exclude the whole thing since we have generator in our std module

//#include <version>
//#ifndef __cpp_lib_generator // 202207L
#if ! __has_include(<generator>)

#	include <memory> // std::shared_ptr
#	include <stdexcept>

#	include "result_range.h"
#	include "job_manager.h"
#	include "task_manager.h"
#	include "task_manager_impl.h"

//#define LOG std::cerr
#define LOG if (0) std::cerr // No-op, optimized out

#define PUBLIC __attribute__((visibility("default")))

namespace exo {

// Generic implementation
template<typename U, typename T>
ResultRange<U, T>::ResultRange(U* manager)
    : manager_(manager) {}

template<typename U, typename T>
ResultRange<U, T>::Iterator::Iterator(ResultRange* range)
    : range_(range) {}

template<typename U, typename T>
T& ResultRange<U, T>::Iterator::operator*() const {
    return range_->value_;
}

template<typename U, typename T>
bool ResultRange<U, T>::Iterator::operator!=(const Iterator& other) const {
    return range_ != nullptr;
}

template<typename U, typename T>
typename ResultRange<U, T>::Iterator ResultRange<U, T>::end() const {
    return Iterator(nullptr);
}

// Specialization for JobManager, ExoEnv
template<>
ResultRange<JobManager, ExoEnv>::Iterator
ResultRange<JobManager, ExoEnv>::begin() {
    if (manager_->run_count() <= 0) {
        return Iterator(nullptr);
    }
    manager_->result_queue()->wait_and_pop(value_);
    manager_->decrement_run_count();
    return Iterator(const_cast<ResultRange*>(this));
}

template<>
ResultRange<JobManager, ExoEnv>::Iterator&
ResultRange<JobManager, ExoEnv>::Iterator::operator++() {
    if (range_ && range_->manager_->run_count() > 0) {
        range_->manager_->result_queue()->wait_and_pop(range_->value_);
        range_->manager_->decrement_run_count();
    } else {
        range_ = nullptr;
    }
    return *this;
}

// Specialization for TaskManager, AsyncResult
template<>
ResultRange<TaskManager, AsyncResult>::Iterator
ResultRange<TaskManager, AsyncResult>::begin() {
    if (manager_->async_count_ <= 0) {
        LOG << "Expecting 0 results" << std::endl;
        return Iterator(nullptr);
    }
    LOG << "Expecting " << manager_->async_count_ << " results" << std::endl;
    while (manager_->async_count_ > 0) {
        --(manager_->async_count_);
        std::shared_ptr<AsyncResult> result_ptr;
        if (manager_->impl_->result_channel.pop(result_ptr) == boost::fibers::channel_op_status::success) {
            LOG << "Popped result: data=" << result_ptr->data << ", message=" << result_ptr->message << std::endl;
            if (manager_->err_ptr) {
                std::rethrow_exception(manager_->err_ptr);
            }
            value_ = std::move(*result_ptr);
            return Iterator(const_cast<ResultRange *>(this));
        } else {
            LOG << "Channel pop failed" << std::endl;
        }
    }
    return Iterator(nullptr);
}

template<>
ResultRange<TaskManager, AsyncResult>::Iterator&
ResultRange<TaskManager, AsyncResult>::Iterator::operator++() {
    if (range_ && range_->manager_->async_count_ > 0) {
        --(range_->manager_->async_count_);
        std::shared_ptr<AsyncResult> result_ptr;
        if (range_->manager_->impl_->result_channel.pop(result_ptr) == boost::fibers::channel_op_status::success) {
            LOG << "Popped result: data=" << result_ptr->data << ", message=" << result_ptr->message << std::endl;
            if (range_->manager_->err_ptr) {
                std::rethrow_exception(range_->manager_->err_ptr);
            }
            range_->value_ = std::move(*result_ptr);
        } else {
            LOG << "Channel pop failed" << std::endl;
        }
    } else {
        range_ = nullptr;
    }
    return *this;
}

// Explicit instantiations
template class PUBLIC ResultRange<JobManager, ExoEnv>;
template class PUBLIC ResultRange<TaskManager, AsyncResult>;

// Declare TaskManager specialization (defined in task_manager.cpp)
template<>
PUBLIC ResultRange<exo::TaskManager, exo::AsyncResult>::Iterator ResultRange<exo::TaskManager, exo::AsyncResult>::begin();
template<>
PUBLIC ResultRange<exo::TaskManager, exo::AsyncResult>::Iterator& ResultRange<exo::TaskManager, exo::AsyncResult>::Iterator::operator++();

} // namespace exo

#endif // doesnt have <generator>
#endif // doesnt have EXO_MODULE