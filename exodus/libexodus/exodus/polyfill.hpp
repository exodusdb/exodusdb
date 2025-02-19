#pragma once

#ifdef _BUILD_STD_MODULE
#define EXPORT export
#else
#define EXPORT
#endif

#include <version>

#ifndef __cpp_lib_start_lifetime_as
#include <type_traits>
namespace std {

template <typename T>
concept ImplicitLifetime = std::is_trivial_v<T> && std::is_trivially_destructible_v<T>;

EXPORT template <std::ImplicitLifetime T>
auto start_lifetime_as(void * Bytes) noexcept -> T * {
    static_assert(sizeof(T) > 0, "type T is incomplete");
    return new (Bytes) T;
}

EXPORT template <std::ImplicitLifetime T>
auto start_lifetime_as(const void * Bytes) noexcept -> const T * {
    static_assert(sizeof(T) > 0, "type T is incomplete");
    return new (Bytes) const T;
}

} // namespace std
#endif

#if !(defined(__cpp_lib_print) || __has_include(<print>))
#ifdef _WIN32
extern "C" {
    __declspec(dllimport) intptr_t __stdcall _get_osfhandle(int);
    __declspec(dllimport) int __stdcall WriteConsoleW(intptr_t, const wchar_t*, unsigned, void*, void*);
    __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned, unsigned, const char*, int, wchar_t*, int);
}
#endif
#include <format>
namespace std {

EXPORT
template <typename... T>
void println(std::format_string<T...> fmt, T&&... args) {
    auto x_Text = std::format(fmt, std::forward<T>(args)...);
    x_Text.push_back('\n');
#ifdef _WIN32
    constexpr auto CP_UTF8 = 65001;
    const int _Required =
        ::MultiByteToWideChar(CP_UTF8, 0,
            x_Text.data(), static_cast<int>(x_Text.size()),
            nullptr, 0);
    std::wstring x_WText(_Required, L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0,
        x_Text.data(), static_cast<int>(x_Text.size()),
        x_WText.data(), static_cast<int>(x_WText.size()));
    ::WriteConsoleW(_get_osfhandle(_fileno(stdout)),
        x_WText.data(), static_cast<unsigned>(x_WText.size()),
        nullptr, nullptr);
#else
    std::fwrite(x_Text.data(), sizeof(char), x_Text.size(), stdout);
#endif
}

} // namespace std
#endif

//#ifndef __cpp_lib_generator
//#include "generator.hpp"
//#endif
