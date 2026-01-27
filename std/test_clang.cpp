import std;
#include <version>

int main() {
#	if __cpp_lib_print
	std::println("std::println: Hello modules from Clang-{} c++{} + libc++! π ≈ {:.6}", __clang_major__, __cplusplus/100%100, std::numbers::pi);
#	else
	std::printf("std::printf: Hello modules from Clang-%d c++%ld + libc++! π ≈ %.6f\n", __clang_major__, __cplusplus/100%100, std::numbers::pi);
#	endif
	std::printf("std::string SSO capacity is %ld\n", std::string().capacity());
	return 0;
}
