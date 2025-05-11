//#include <iostream>
//#include <string>
//#include <vector>
//#include <random>
//#include <sstream>
#include <chrono>
#include <cassert>

#include <exodus/program.h>
programinit()

func main() {

	var ntests = COMMAND.f(2);
	if (not ntests)
		ntests = 1'000'000;

    // Run repeatable fuzz tests
//    fuzzTest(ntests, 12345, true);
    fuzzTest(ntests, 12345, false);

    return 0;
}

// Simpler, slower algorithm: Split s2 by commas and check each token
auto simpleFindSubstring(const std::string& s1, const std::string& s2, bool indexed) -> size_t {
    if (s1.empty() || s2.empty()) return false;

    std::stringstream ss(s2);
    std::string token;
	size_t tokenn = 0;
	size_t index1 = 1;
    while (std::getline(ss, token, ',')) {
		tokenn++;
        if (token == s1) {
            return indexed ? index1 : tokenn;
        }
		index1 += token.size() + 1;
    }
    return 0;
}

// Random string generator with {',', 'a', 'b'}
std::string generateRandomString(std::mt19937& rng, int minLen, int maxLen, bool allowCommas) {
    std::uniform_int_distribution<int> lenDist(minLen, maxLen);
    std::uniform_int_distribution<int> charDist(0, allowCommas ? 2 : 1); // 0: a, 1: b, 2: comma
    int len = lenDist(rng);
    std::string result;

    for (int i = 0; i < len; ++i) {
        if (allowCommas && i > 0 && i < len - 1) {
            std::uniform_int_distribution<int> commaDist(0, 4); // 20% chance of comma
            if (commaDist(rng) == 0) {
                result += ',';
                continue;
            }
        }
        result += (charDist(rng) == 0 ? 'a' : 'b');
    }
    return result;
}

// Fuzz testing function
void fuzzTest(size_t numTests, unsigned int seed, bool indexed) {
    std::mt19937 rng(seed);
    int failures = 0;
    int foundCases = 0; // Counter for found cases

    // Explicit edge cases
    std::vector<std::pair<std::string, std::string>> edgeCases = {
        {"", ""},           // Empty strings
        {"a", ""},          // Empty s2
        {"", "a"},          // Empty s1
        {"a", "a"},         // Single-char match
        {"a", "b"},         // Single-char no match
        {"ab", "ab"},       // Exact match, no commas
        {"ab", "a,b"},      // Comma-separated no match
        {"ab", "ab,a"},     // Match with trailing comma
        {"ab", "a,ab"},     // Match with leading comma
        {"a", ",,a,,"},     // Multiple consecutive commas
        {"abc", "a,b,c"},   // No match, individual chars
        {"aaa", "aaa,,b"},  // Match with empty token
        {"b", ",b,,b,"}     // Multiple matches with commas
    };

	std::cout << (indexed ? "Indexed" : "Counted") << std::endl;

    std::cout << "Testing " << edgeCases.size() << " edge cases...\n";
    for (size_t i = 0; i < edgeCases.size(); ++i) {
        const auto& [s1, s2] = edgeCases[i];
        auto resultSlow = simpleFindSubstring(s1, s2, indexed);
#ifdef RAWCPP
        auto resultFast = findSubstringInCommaSeparated(s1, s2, indexed);
        if (resultFast != resultSlow) {
#else
		var resultFast;
		if (var(s1).listed(s2, resultFast)) {}
        if (resultFast.toInt64() != resultSlow) {
#endif
            failures++;
            std::cout << "Edge case " << i + 1 << " FAILED:\n";
            std::cout << "  s1 = \"" << s1 << "\"\n";
            std::cout << "  s2 = \"" << s2 << "\"\n";
            std::cout << "  Fast algorithm: " << resultFast << "\n";
            std::cout << "  Slow algorithm: " << resultSlow << "\n";
        } else if (resultFast && resultSlow) {
            foundCases++; // Count edge cases where s1 is found
        }
    }

    std::cout << "\nRunning " << numTests << " fuzz tests...\n";

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < numTests; ++i) {
        // Generate random strings (1-10 chars)
        std::string s1 = generateRandomString(rng, 1, 10, false); // s1: no commas
        std::string s2 = generateRandomString(rng, 1, 10, true);  // s2: allow commas

        // Run both algorithms
        auto resultSlow = simpleFindSubstring(s1, s2, indexed);
#ifdef RAWCPP
        auto resultFast = findSubstringInCommaSeparated(s1, s2, indexed);
        // Compare results
        if (resultFast != resultSlow) {
#else
		var resultFast;
		if (var(s1).listed(s2, resultFast)) {}
        // Compare results
        if (resultFast.toInt64() != resultSlow) {
#endif
            failures++;
            std::cout << "Test " << i + 1 << " FAILED:\n";
            std::cout << "  s1 = \"" << s1 << "\"\n";
            std::cout << "  s2 = \"" << s2 << "\"\n";
            std::cout << "  Fast algorithm: " << resultFast << "\n";
            std::cout << "  Slow algorithm: " << resultSlow << "\n";
        } else if (resultFast && resultSlow) {
            foundCases++; // Count cases where s1 is found
        }

        // Progress indicator
        if ((i + 1) % (numTests / 10) == 0) {
            std::cout << "\r" << (i + 1) << std::flush;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "\nFuzz testing complete!\n";
	std::cout << (indexed ? "Indexed" : "Counted") << std::endl;
    std::cout << "Total tests: " << size_t(numTests) + edgeCases.size() << " (including " << edgeCases.size() << std::endl;
    std::cout << "Found cases: " << foundCases << "\n";
    std::cout << "Failures: " << failures << "\n";
    std::cout << "Time taken: " << duration.count() << " seconds\n";

	if (!failures)
		std::cout << "Test passed." << std::endl;
}

}; // programexit()
