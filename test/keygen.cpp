#include <print>

#include "rsa.hpp"
#include "bigint.hpp"

int test_native_integers() {
    int err_count = 0;

    if (auto opt_key = rsa::keygen<std::int64_t>(16)) {
        rsa::Key<std::int64_t> key = *opt_key;
        std::println("pub={} prv={} n={}", key.pub, key.prv, key.n);

        auto gen = rsa::impl::RandomGenerator<std::int64_t>(0, key.n - 1);

        auto message = gen();
        auto encrypted_message = rsa::encode(message, key.pub, key.n);
        auto decrypted_message = rsa::decode(encrypted_message, key.prv, key.n);
        std::println("forward m={} e={} d={}", message, encrypted_message, decrypted_message);

        if (decrypted_message != message) {
            ++err_count;
        }

        message = gen();
        encrypted_message = rsa::decode(message, key.prv, key.n);
        decrypted_message = rsa::encode(encrypted_message, key.pub, key.n);
        std::println("backwards m={} e={} d={}", message, encrypted_message, decrypted_message);

        if (decrypted_message != message) {
            ++err_count;
        }
    } else {
        ++err_count;
    }

    return err_count;
}

int test_bigint() {
    int err_count = 0;
    if (auto opt_key = rsa::keygen<rsa::bigint<64>>(16)) {
        rsa::Key<rsa::bigint<64>> key = *opt_key;
        std::println("pub={} prv={} n={}", key.pub.to_string(), key.prv.to_string(), key.n.to_string());

        auto gen = rsa::impl::RandomGenerator<rsa::bigint<64>>(0, key.n - 1);

        auto message = gen();
        auto encrypted_message = rsa::encode(message, key.pub, key.n);
        auto decrypted_message = rsa::decode(encrypted_message, key.prv, key.n);
        std::println(
            "forward m={} e={} d={}", message.to_string(), encrypted_message.to_string(), decrypted_message.to_string()
        );

        if (decrypted_message != message) {
            ++err_count;
        }

        message = gen();
        encrypted_message = rsa::decode(message, key.prv, key.n);
        decrypted_message = rsa::encode(encrypted_message, key.pub, key.n);
        std::println(
            "backwards m={} e={} d={}", message.to_string(), encrypted_message.to_string(), decrypted_message.to_string()
        );

        if (decrypted_message != message) {
            ++err_count;
        }
    } else {
        ++err_count;
    }

    return err_count;
}

int main() {
    int ec = test_native_integers();
    ec += test_bigint();
    return ec;
}
