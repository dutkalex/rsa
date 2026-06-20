#include <print>

#include "bigint.hpp"

int main(){
    auto i = rsa::bigint{42};
    if (i.to_string() != "42"){
        std::println("i={}", i.to_string());
        return 1;
    }

    auto i2 = rsa::bigint{300};
    if (i2.to_string() != "300"){
        std::println("i2={}", i2.to_string());
        return 2;
    }

    auto i3 = rsa::bigint{-48};
    if (i3.to_string() != "-48"){
        std::println("i3={}", i3.to_string());
        return 3;
    }

    i = i2 + i3;
    if (i.to_string() != "252"){
        std::println("i={}", i.to_string());
        return 4;
    }

    i += i2;
    if (i.to_string() != "552"){
        std::println("i={}", i.to_string());
        return 5;
    }

    auto j = rsa::bigint{uint64_t{92753}};
    if (j.to_string() != "92753"){
        std::println("j={}", j.to_string());
        return 6;
    }

    auto k = rsa::bigint{int16_t{-48}};
    if (k.to_string() != "-48"){
        std::println("k={}", k.to_string());
        return 7;
    }

    i = i + j;
    if (i.to_string() != "93305"){
        std::println("i={}", i.to_string());
        return 8;
    }

    i = j - rsa::bigint{305};
    if (i.to_string() != "92448"){
        std::println("i={}", i.to_string());
        return 9;
    }

    k -= rsa::bigint{52};
    if (k.to_string() != "-100"){
        std::println("k={}", k.to_string());
        return 10;
    }

    return 0;
}
