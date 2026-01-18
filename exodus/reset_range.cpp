#if EXO_MODULE > 1
	import std;
//#	include <cstddef>
#else
#	include <cstddef>
#	include <new>
#	include <type_traits>
#	include <stdexcept>
#	include <utility>
#endif

#if EXO_MODULE
	import var;
#else
#	include <exodus/var.h>
#	include <exodus/dim.h>
#endif

#define PUBLIC __attribute__((visibility("default")))

namespace exo {

// Template to reset a range of objects to their default-constructed state
template<typename T>
PUBLIC void reset_range(T& first, T& last) {
    static_assert(std::is_destructible<T>::value, "Type must be destructible");
    static_assert(std::is_default_constructible<T>::value, "Type must be default constructible");

    // Get pointers to the objects
    T* start = &first;
    T* end = &last;

    // Ensure valid range
    if (start > end) {
        std::swap(start, end);
    }

    // Calculate the size of the range in bytes (inclusive)
    std::ptrdiff_t byte_diff = reinterpret_cast<char*>(end) - reinterpret_cast<char*>(start) + static_cast<std::ptrdiff_t>(sizeof(T));

    // Sanity check: Ensure the range is a whole number of T objects
    if (byte_diff % static_cast<std::ptrdiff_t>(sizeof(T)) != 0) {
        throw std::invalid_argument("Range size is not a multiple of object size");
    }

    // Calculate the number of objects (inclusive)
    size_t count = static_cast<size_t>(byte_diff / static_cast<std::ptrdiff_t>(sizeof(T)));

    // Destroy all objects in the range
    for (size_t i = 0; i < count; ++i) {
        (start + i)->~T();
    }

    // Reconstruct all objects with default construction
    for (size_t i = 0; i < count; ++i) {
        new (start + i) T();
    }
}

// Explicit instantiations for var and dim
template void reset_range<var>(var& first, var& last);
template void reset_range<dim>(dim& first, dim& last);

} // namespace exo
