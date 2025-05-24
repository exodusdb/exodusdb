#ifndef STREAM_HANDLE_H
#define STREAM_HANDLE_H

namespace exo {

// Manages a Boost ASIO posix stream_descriptor and its cleanup.
class StreamPtr {
public:
    StreamPtr() : stream_ptr_(nullptr), cleanup_(nullptr) {}

    ~StreamPtr() {
        if (stream_ptr_ && cleanup_) {
            cleanup_(stream_ptr_);
        }
    }

    // Sets the stream_descriptor pointer and its cleanup function.
    void set_stream(void* stream_ptr, void (*cleanup)(void*)) {
        stream_ptr_ = stream_ptr;
        cleanup_ = cleanup;
    }

    // Gets the stream_descriptor pointer.
    void* get_stream() const { return stream_ptr_; }

private:
    void* stream_ptr_;        // Pointer to the stream_descriptor.
    void (*cleanup_)(void*);  // Cleanup function for the stream_descriptor.
};

} // namespace exo

#endif