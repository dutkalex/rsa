#include <print>

#include "rsa.hpp"

int main(){
    int a = 136497;
    int b = rsa::impl::generate_random_coprime(a);
    int gcd = rsa::impl::gcd(a, b);
    std::println("a={}, b={}, gcd={}", a, b, gcd);
    if (gcd != 1){
        return 1;
    }
    return 0;
}
