#include <print>

#include "rsa.hpp"

int main(){
    int p = rsa::impl::generate_random_prime(100, 1000);
    std::println("generated number: {}", p);
    if (not rsa::impl::is_prime(p)){
        return 1;
    }
    return 0;
}
