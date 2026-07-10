#include <print>

#include "bigint.hpp"

int main() {
    auto i = rsa::bigint{42};
    if (i.to_string() != "42") {
        std::println("i={}", i.to_string());
        return 1;
    }

    if (not(i < 100)) {
        return 21;
    }
    if (not(i == 42)) {
        return 22;
    }
    if (not(i > 0)) {
        return 23;
    }

    auto i2 = rsa::bigint{300};
    if (i2.to_string() != "300") {
        std::println("i2={}", i2.to_string());
        return 2;
    }

    auto i3 = rsa::bigint{-48};
    if (i3.to_string() != "-48") {
        std::println("i3={}", i3.to_string());
        return 3;
    }

    i = i2 + i3;
    if (i.to_string() != "252") {
        std::println("i={}", i.to_string());
        return 4;
    }

    i += i2;
    if (i.to_string() != "552") {
        std::println("i={}", i.to_string());
        return 5;
    }

    auto j = rsa::bigint{uint64_t{92753}};
    if (j.to_string() != "92753") {
        std::println("j={}", j.to_string());
        return 6;
    }

    auto k = rsa::bigint{int16_t{-48}};
    if (k.to_string() != "-48") {
        std::println("k={}", k.to_string());
        return 7;
    }

    i = i + j;
    if (i.to_string() != "93305") {
        std::println("i={}", i.to_string());
        return 8;
    }

    i = j - rsa::bigint{305};
    if (i.to_string() != "92448") {
        std::println("i={}", i.to_string());
        return 9;
    }

    k -= rsa::bigint{52};
    if (k.to_string() != "-100") {
        std::println("k={}", k.to_string());
        return 10;
    }

    auto a = rsa::bigint{100} * rsa::bigint{4};
    if (a.to_string() != "400") {
        std::println("a={}", a.to_string());
        return 11;
    }

    a = rsa::bigint{-12} * rsa::bigint{4};
    if (a.to_string() != "-48") {
        std::println("a={}", a.to_string());
        return 12;
    }

    a *= rsa::bigint{-2};
    if (a.to_string() != "96") {
        std::println("a={}", a.to_string());
        return 13;
    }

    a = rsa::bigint{500000} * rsa::bigint{4000000};
    if (a.to_string() != "2000000000000"){
        std::println("a={}", a.to_string());
        return 14;
    }

    auto q = rsa::bigint{100} / rsa::bigint{4};
    auto r = rsa::bigint{100} % rsa::bigint{4};
    if (q.to_string() != "25" or r.to_string() != "0"){
        std::println("q={} r={}", q.to_string(), r.to_string());
        return 15;
    }

    q = rsa::bigint{10000} / rsa::bigint{300};
    r = rsa::bigint{10000} % rsa::bigint{300};
    if (q.to_string() != "33" or r.to_string() != "100"){
        std::println("q={} r={}", q.to_string(), r.to_string());
        return 16;
    }

    q = rsa::bigint{-10000} / rsa::bigint{300};
    r = rsa::bigint{-10000} % rsa::bigint{300};
    if (q.to_string() != "-33" or r.to_string() != "-100"){
        std::println("q={} r={}", q.to_string(), r.to_string());
        return 17;
    }

    q = rsa::bigint{-10000} / rsa::bigint{-300};
    r = rsa::bigint{-10000} % rsa::bigint{-300};
    if (q.to_string() != "33" or r.to_string() != "-100"){
        std::println("q={} r={}", q.to_string(), r.to_string());
        return 18;
    }

    q = rsa::bigint{10000} / rsa::bigint{-300};
    r = rsa::bigint{10000} % rsa::bigint{-300};
    if (q.to_string() != "-33" or r.to_string() != "100"){
        std::println("q={} r={}", q.to_string(), r.to_string());
        return 19;
    }

    i = rsa::bigint{1} << 3;
    j = rsa::bigint{4} << 12;
    if (i.to_string() != "8" or j.to_string() != "16384"){
        std::println("(i={} j={}", i.to_string(), j.to_string());
        return 20;
    }

    return 0;
}
