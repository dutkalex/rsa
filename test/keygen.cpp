#include <print>

#include "rsa.hpp"
#include "bigint.hpp"

int test_native_integers() {
    int err_count = 0;

    auto [n, pub, prv] = rsa::keygen<std::int64_t>(std::int16_t{1000}, std::int16_t{10000});
    std::println("pub={} prv={} n={}", pub, prv, n);

    auto gen = rsa::impl::RandomGenerator<std::int64_t>(0, n - 1);

    auto message = gen();
    auto encrypted_message = rsa::encode(message, pub, n);
    auto decrypted_message = rsa::decode(encrypted_message, prv, n);
    std::println("forward m={} e={} d={}", message, encrypted_message, decrypted_message);

    if (decrypted_message != message) {
        ++err_count;
    }

    message = gen();
    encrypted_message = rsa::decode(message, prv, n);
    decrypted_message = rsa::encode(encrypted_message, pub, n);
    std::println("backwards m={} e={} d={}", message, encrypted_message, decrypted_message);

    if (decrypted_message != message) {
        ++err_count;
    }

    return err_count;
}

int test_bigint() {
    int err_count = 0;
    using prime_t = rsa::bigint<16>;
    using key_t = rsa::bigint<64>;

    auto [n, pub, prv] = rsa::keygen<key_t>(prime_t{1000000}, prime_t{1000000000});
    std::println("pub={} prv={} n={}", pub.to_string(), prv.to_string(), n.to_string());

    auto gen = rsa::impl::RandomGenerator<key_t>(0, n - 1);

    auto message = gen();
    auto encrypted_message = rsa::encode(message, pub, n);
    auto decrypted_message = rsa::decode(encrypted_message, prv, n);
    std::println(
        "forward m={} e={} d={}", message.to_string(), encrypted_message.to_string(), decrypted_message.to_string()
    );

    if (decrypted_message != message) {
        ++err_count;
    }

    message = gen();
    encrypted_message = rsa::decode(message, prv, n);
    decrypted_message = rsa::encode(encrypted_message, pub, n);
    std::println(
        "backwards m={} e={} d={}", message.to_string(), encrypted_message.to_string(), decrypted_message.to_string()
    );

    if (decrypted_message != message) {
        ++err_count;
    }

    return err_count;
}

int main() {
    int ec = test_native_integers();
    ec += test_bigint();
    return ec;
}
