#include <print>

#include "rsa.hpp"
#include "bigint.hpp"

int main(){
    auto [n, pub, prv] = rsa::keygen<rsa::bigint<16>, rsa::bigint<4>>();
    std::println("pub={} prv={} n={}", pub.to_string(), prv.to_string(), n.to_string());

    auto gen = rsa::impl::RandomGenerator<rsa::bigint<16>>(0, n-1);

    auto message = gen();
    auto encrypted_message = rsa::encode(message, pub, n);
    auto decrypted_message = rsa::decode(encrypted_message, prv, n);
    std::println("forward m={} e={} d={}", message.to_string(), encrypted_message.to_string(), decrypted_message.to_string());

    if (decrypted_message != message){
        return 1;
    }

    message = gen();
    encrypted_message = rsa::decode(message, prv, n);
    decrypted_message = rsa::encode(encrypted_message, pub, n);
    std::println("backwards m={} e={} d={}", message.to_string(), encrypted_message.to_string(), decrypted_message.to_string());

    if (decrypted_message != message){
        return 2;
    }

    return 0;
}
