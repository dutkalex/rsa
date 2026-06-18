#include <print>

#include "rsa.hpp"

int main(){
    auto [n, pub, prv] = rsa::keygen<int>();
    std::println("pub={} prv={} n={}", pub, prv, n);

    auto gen = rsa::impl::RandomGenerator<int>(0, n-1);

    int message = gen();
    int encrypted_message = rsa::encode(message, pub, n);
    int decrypted_message = rsa::decode(encrypted_message, prv, n);
    std::println("forward m={} e={} d={}", message, encrypted_message, decrypted_message);

    if (decrypted_message != message){
        return 1;
    }

    message = gen();
    encrypted_message = rsa::decode(message, prv, n);
    decrypted_message = rsa::encode(encrypted_message, pub, n);
    std::println("backwards m={} e={} d={}", message, encrypted_message, decrypted_message);

    if (decrypted_message != message){
        return 2;
    }

    return 0;
}
