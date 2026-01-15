#include "threadsafequeue.h"

#if EXO_MODULE
    import var;
#else
#   include "var.h"
#endif

#define PUBLIC __attribute__((visibility("default")))

#include "exoenv.h"
//#include <iostream>
//#include <iomanip>
//#include <chrono>
//#include <thread>

//// Logging macro with relative timestamp from program start
////#define ENABLE_LOGGING
//#ifdef ENABLE_LOGGING
//    // Static variable to capture initial time (initialized once at program start)
//static const auto start_time = std::chrono::system_clock::now();
//#define LOG std::cerr << "[" << std::fixed << std::setprecision(3) \
//                      << (std::chrono::duration_cast<std::chrono::microseconds>( \
//                          std::chrono::system_clock::now().time_since_epoch()).count() - \
//                          std::chrono::duration_cast<std::chrono::microseconds>( \
//                          start_time.time_since_epoch()).count()) / 1000.0 \
//                      << "ms, T" << std::this_thread::get_id() << "] Q "
////                          << "ms, T" << (start_time%1000000) << "] "
//#else
//    #define LOG if (0) std::cerr // No-op, optimized out
//#endif

namespace exo {

//template<typename T>
//void ThreadSafeQueue<T>::push(const T& value) {
//    std::lock_guard<std::mutex> lock(mutex_);
//    LOG << "Pushing value to queue, size before: " << queue_.size() << std::endl;
//    queue_.push(value);
//    LOG << "Pushed value, size after: " << queue_.size() << ", notifying one" << std::endl;
//    cond_var_.notify_one();
//}

template<typename T>
template<typename U, typename>
void ThreadSafeQueue<T>::push(const U& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    //LOG << "Pushing value to queue, size before: " << queue_.size() << std::endl;
    queue_.push(value);
    //LOG << "Pushed value, size after: " << queue_.size() << ", notifying one" << std::endl;
    cond_var_.notify_one();
}

template <typename T>
bool ThreadSafeQueue<T>::pop(T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    //LOG << "Attempting pop, size: " << queue_.size() << std::endl;
    if (queue_.empty()) {
        //LOG << "Pop failed: queue empty" << std::endl;
        return false;
    }
    value = std::move(queue_.front());
    queue_.pop();
    //LOG << "Popped value, size after: " << queue_.size() << std::endl;
    return true;
}

template <typename T>
bool ThreadSafeQueue<T>::try_pop(T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    //LOG << "Attempting try_pop, size: " << queue_.size() << std::endl;
    if (queue_.empty()) {
        //LOG << "Try_pop failed: queue empty" << std::endl;
        return false;
    }
    value = std::move(queue_.front());
    queue_.pop();
    //LOG << "Try_pop succeeded, size after: " << queue_.size() << std::endl;
    return true;
}

template <typename T>
bool ThreadSafeQueue<T>::empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    bool is_empty = queue_.empty();
    //LOG << "Checked empty: " << (is_empty ? "true" : "false") << ", size: " << queue_.size() << std::endl;
    return is_empty;
}

template <typename T>
bool ThreadSafeQueue<T>::wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lock(mutex_);
    //LOG << "Waiting to pop, size: " << queue_.size() << std::endl;
    cond_var_.wait(lock, [this] { 
        bool not_empty = !queue_.empty();
        //LOG << "Condition check: queue " << (not_empty ? "not empty" : "empty") << std::endl;
        return not_empty; 
    });
    value = std::move(queue_.front());
    queue_.pop();
    //LOG << "Wait_and_pop succeeded, size after: " << queue_.size() << std::endl;
    return true;
}

template<typename T>
T ThreadSafeQueue<T>::pop() {
    std::lock_guard<std::mutex> lock(mutex_);
    //LOG << "Attempting pop (return value), size: " << queue_.size() << std::endl;
    if (queue_.empty()) {
        //LOG << "Pop failed: throwing runtime_error (queue empty)" << std::endl;
        throw std::runtime_error("Queue is empty");
    }
    T value = std::move(queue_.front());
    queue_.pop();
    //LOG << "Popped value (return), size after: " << queue_.size() << std::endl;
    return value;
}

template<typename T>
void ThreadSafeQueue<T>::push(T&& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    //LOG << "Pushing rvalue to queue, size before: " << queue_.size() << std::endl;
    queue_.push(std::move(value));
    //LOG << "Pushed rvalue, size after: " << queue_.size() << ", notifying one" << std::endl;
    cond_var_.notify_one();
}

// Explicit instantiations for specific types
template class PUBLIC ThreadSafeQueue<var>;
template class PUBLIC ThreadSafeQueue<std::string>;
template class PUBLIC ThreadSafeQueue<ExoEnv>;

template void PUBLIC ThreadSafeQueue<var>::push<var>(const var&);
template void PUBLIC ThreadSafeQueue<var>::push<int>(const int&);
template void PUBLIC ThreadSafeQueue<var>::push<double>(const double&);

} // namespace exo