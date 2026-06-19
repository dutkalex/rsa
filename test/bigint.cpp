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


    auto j = rsa::bigint{uint64_t{92753}};
    if (j.to_string() != "92753"){
        std::println("j={}", j.to_string());
        return 4;
    }

    auto k = rsa::bigint{int16_t{-48}};
    if (k.to_string() != "-48"){
        std::println("k={}", k.to_string());
        return 5;
    }

    return 0;
}
