#pragma once

#include <optional>
#include <tuple>

#include "concepts.hpp"
#include "impl/gcd.hpp"
#include "impl/modulo_multiply.hpp"
#include "impl/power.hpp"
#include "impl/prime.hpp"
#include "impl/generator.hpp"

namespace rsa::impl {
    // Generates a triplet (n, q, k) where n is the prime candidate
    // n is guaranteed to be odd
    // q and k are such that n-1 = q * 2^k with q odd
    template<rsa::integral I>
    std::tuple<I, I, I> generate_prime_candidate(RandomGenerator<I>& distrib) {
        while (true) {
            I n = distrib();
            if (n % 2 == 0) {
                continue;
            }

            I q = n - 1;
            I k = 0;
            while (q % 2 == 0) {
                ++k;
                q = q / 2;
            }

            return {n, q, k};
        }
    }

    // Generates a random prime number within the [floor, ceiling] range
    template<rsa::integral I>
    I generate_random_prime(I floor, I ceiling, std::optional<int> seed = std::nullopt) {
        auto distrib = RandomGenerator<I>{floor, ceiling, seed};

        while (true) {
            auto [n, q, k] = generate_prime_candidate(distrib);

            // Apply 100 times the Miller-Rabin test
            auto witness_distrib = RandomGenerator<I>{I{2}, static_cast<I>(n - I{2}), seed.transform([](int s) {
                                                          return s + 1;
                                                      })};
            bool n_is_prime = true;
            for (int i = 0; i < 100 and n_is_prime; ++i) {
                n_is_prime = miller_rabin_test(n, q, k, witness_distrib());
            }

            if (n_is_prime) {
                return n;
            }
        }
    }

    // Generates a number which is coprime with n in the range [2, n-1]
    template<rsa::integral I>
    I generate_random_coprime(I n, std::optional<int> seed = std::nullopt) {
        if (not seed) {
            std::random_device dev;
            seed = dev();
        }

        auto distr = rsa::impl::RandomGenerator<I>{2, n - 1, seed};

        while (true) {
            I candidate = distr();
            if (gcd(n, candidate) == 1) {
                return candidate;
            }
        }
    }
}  // namespace rsa::impl

// RSA: R. Rivest, A. Shamir, L. Adleman
namespace rsa {
    template<rsa::integral BigInt>
    struct Key {
        BigInt n;
        BigInt pub;
        BigInt prv;
    };

    template<rsa::integral BigInt>
    std::optional<Key<BigInt>> keygen(std::size_t prime_size) {
        if (sizeof(BigInt) * 2 < prime_size) {  // overflows can happen
            return std::nullopt;
        }

        BigInt floor = BigInt{1};
        for (int i = 0; i < prime_size - 1; ++i) {
            floor *= 2;
        }
        BigInt ceiling = floor * 2 - 1;

        BigInt prime1 = rsa::impl::generate_random_prime(floor, ceiling);
        BigInt prime2 = prime1;
        while (prime2 == prime1) {
            prime2 = rsa::impl::generate_random_prime(floor, ceiling);
        }
        BigInt n = prime1 * prime2;
        BigInt phi_n = (prime1 - 1) * (prime2 - 1);
        BigInt pub = impl::generate_random_coprime(phi_n);
        BigInt prv = impl::multiplicative_inverse(pub, phi_n).value();
        return Key<BigInt>{n, pub, prv};
    }

    template<rsa::integral I>
    auto encode(I plaintext, I pub, I n) {
        return impl::power_semigroup(plaintext, pub, impl::modulo_multiply<I>{n});
    }

    template<rsa::integral I>
    auto decode(I cyphertext, I prv, I n) {
        return impl::power_semigroup(cyphertext, prv, impl::modulo_multiply<I>{n});
    }
}  // namespace rsa
