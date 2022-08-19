#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <vector>
 
struct id { int i; explicit id(int i) : i{i} {} };
 
void print(id i, const auto& v) {
    std::cout << "@" << i.i << ": ";
    std::ranges::for_each(v, [](auto const& e) { std::cout << e << ' '; });
    std::cout << '\n';
}
 
int main()
{
    // a vector containing several duplicate elements
    std::vector<int> v{1, 2, 1, 1, 3, 3, 3, 4, 5, 4};
 
    print(id{1}, v);
 
    // remove consecutive (adjacent) duplicates
    auto ret = std::ranges::unique(v);
    // v now holds {1 2 1 3 4 5 4 x x x}, where 'x' is indeterminate
    v.erase(ret.begin(), ret.end());
    print(id{2}, v);
 
    // sort followed by unique, to remove all duplicates
    std::ranges::sort(v); // {1 1 2 3 4 4 5}
    print(id{3}, v);
 
    auto [first, last] = std::ranges::unique(v.begin(), v.end());
    // v now holds {1 2 3 4 5 x x}, where 'x' is indeterminate
    v.erase(first, last);
    print(id{4}, v);
 
 
    // unique with custom comparison and projection
    std::vector<std::complex<int>> vc{ {1, 1}, {-1, 2}, {-2, 3}, {2, 4}, {-3, 5} };
    print(id{5}, vc);
 
    auto ret2 = std::ranges::unique(vc,
        [](int x, int y) { return std::abs(x) == std::abs(y); }, // comp
        [](std::complex<int> z) { return z.real(); }             // proj
    );
    vc.erase(ret2.begin(), ret2.end());
    print(id{6}, vc);
}
