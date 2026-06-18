#include <print>

#include "rsa.hpp"

int main(){
    int a = 2*3*7*11;
    int b = 3*11*13;
    int gcd = rsa::impl::gcd(a, b);
    if (gcd != 3*11){
        return 1;
    }

    auto [gcd2, inv] = rsa::impl::extended_gcd(a, b);
    if (gcd2 != gcd){
        return 2;
    }
    if ((inv*a) % b != gcd){
        return 3;
    }

    return 0;
}
