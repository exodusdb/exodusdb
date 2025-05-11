#include <string>
#include <sstream>

namespace exo {

template<int N>
auto findSubstringInCommaSeparated(const std::string_view s1, const std::string_view s2) -> size_t {
    static_assert(N == 1 || N == 2 || N == 3, "Template parameter must be 1 for indexed, 2 for counted progressive, 3 for counted backtracking");

    if (s1.empty() || s2.empty() || s1.size() > s2.size()) return 0;

    const char* p1 = s1.data();
    const char* p2 = s2.data();
    const char* s1_end = s1.data() + s1.size();
    const char* s2_end = s2.data() + s2.size();
    const char* s2_max = s2_end - s1.size() + 1;

//	if constexpr(N == 2) {
	    size_t token_count = 1; // Token count for N=2
//	}

    while (p2 < s2_max) {
        if constexpr (N == 2) {
            if (*p2 == ',') {
                token_count++;
            }
        }
        if (*p2 == *p1) {
            if (p2 == s2.data() || *(p2 - 1) == ',') {
                const char* temp1 = p1;
                const char* start = p2; // Match start for N=1
                while (temp1 < s1_end) {
                    if constexpr (N == 2) {
                        if (*p2 == ',') {
                            token_count++;
                        }
                    }
                    if (*temp1 != *p2) {
                        goto mismatch;
                    }
                    temp1++;
                    p2++;
                    if (p2 == s2_end) {
                        if (!(temp1 < s1_end)) {
                            goto matched;
                        }
                    }
                }
                if (p2 == s2_end || *p2 == ',') {
                matched:
                    if constexpr (N == 1) {
                        return size_t(start - s2.data()) + 1; // Position for N=1
                    } else if constexpr (N == 2) {
                        return token_count; // Token count for N=2
                    } else {
                        size_t count = 1;
                        p2 -= s1.size();
                        while (p2-- >= s2.data()) {
                            count += *p2 == ',';
                        }
                        return count; // Backtracked count for N=3
                    }
                }
            mismatch:;
            }
        }
        p2++;
    }
    return 0;
}

template auto findSubstringInCommaSeparated<1>(const std::string_view s1, const std::string_view s2) -> size_t;
//template auto findSubstringInCommaSeparated<2>(const std::string_view s1, const std::string_view s2) -> size_t;
//template auto findSubstringInCommaSeparated<3>(const std::string_view s1, const std::string_view s2) -> size_t;

auto findSubstringInCommaSeparated(const std::string_view s1, const std::string_view s2, bool indexed) -> size_t {
	if (indexed) return findSubstringInCommaSeparated<1>(s1, s2);
	return findSubstringInCommaSeparated<2>(s1, s2);
	//return findSubstringInCommaSeparated<3>(s1, s2);
}

//auto findSubstringInCommaSeparated(const std::string_view s1, const std::string_view s2, bool indexed) -> size_t {
//    if (s1.empty() || s2.empty() || s1.size() > s2.size()) return false;
//
//    const char* p1 = s1.data();
//    const char* p2 = s2.data();
//    const char* s1_end = s1.data() + s1.size();
//    const char* s2_end = s2.data() + s2.size();
//    const char* s2_max = s2_end - s1.size() + 1;
//
////#ifndef NDEBUG
////    assert(p1 == s1.data() && p1 < s1_end && "p1 out of s1 bounds");
////    assert(p2 == s2.data() && p2 < s2_end && "p2 out of s2 bounds");
////#endif
//
//    while (p2 < s2_max) {
////#ifndef NDEBUG
////        assert(p2 < s2_end && "p2 exceeds s2 upper bound");
////#endif
//        if (*p2 == *p1) {
//            if (p2 == s2.data() || *(p2 - 1) == ',') {
////#ifndef NDEBUG
////                assert(p2 > s2.data() || p2 == s2.data() && "p2 underflow on prev char access");
////#endif
//                const char* temp1 = p1;
//
////#ifndef NDEBUG
////                assert(temp1 < s1_end && "temp1 exceeds s1 upper bound");
////                assert(p2 < s2_end && "p2 exceeds s2 upper bound");
////#endif
//
//                while (temp1 < s1_end) {
////#ifndef NDEBUG
////                    assert(temp1 < s1_end && "temp1 exceeds s1 upper bound in loop");
////                    assert(p2 < s2_end && "p2 exceeds s2 upper bound in loop");
////#endif
//                    if (*temp1 != *p2) {
//                        goto mismatch;
//                    }
//                    temp1++;
//                    p2++;
//                    if (p2 == s2_end) {
////                        if (! (temp1 < s1_end)) {
//							goto matched;
////						}
//                    }
//                }
//
////#ifndef NDEBUG
////                assert(temp1 <= s1_end && "temp1 exceeds s1 end after loop");
////                assert(p2 < s2_end && "p2 exceeds s2 upper bound after loop");
////#endif
//
//                if (*p2 == ',') {
//matched:
//					if (indexed)
//						return size_t(p2 - s2.data()) - s1.size() + 1;
//					// Count commas and add 1
//					size_t count = 1;
//					p2 -= s1.size();
//					while (p2-- >= s2.data()) {
//						count += *p2 == ',';
//					}
//					return count;
//                }
//mismatch:;
//            }
//        }
//        p2++;
//    }
//
////#ifndef NDEBUG
////    assert(p2 <= s2_end && "p2 exceeds s2 end after loop");
////#endif
//    return 0;
//}
//
//// Simpler, slower algorithm: Split s2 by commas and check each token
//auto simpleFindSubstring(const std::string& s1, const std::string& s2, bool counted = false) -> size_t {
//    if (s1.empty() || s2.empty()) return false;
//
//    std::stringstream ss(s2);
//    std::string token;
//	size_t tokenn = 0;
//	size_t index1 = 1;
//    while (std::getline(ss, token, ',')) {
//		tokenn++;
//        if (token == s1) {
//            return counted ? tokenn : index1;
//        }
//		index1 += token.size() + 1;
//    }
//    return 0;
//}

} //namespace exo
