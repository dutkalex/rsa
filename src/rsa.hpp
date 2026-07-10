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
    // Represents an RSA key
    // pub and prv are multiplicative inverses modulo n
    template<rsa::integral I>
    struct Key {
        I n;
        I pub;
        I prv;
    };

    // Generates an RSA key using primes with prime_size binary digits
    // The template parameter I represents the integral type to use
    // If I is not big enough to safely perform the RSA arithmetic, an std::nullopt is returned.
    template<rsa::integral I>
    std::optional<Key<I>> keygen(std::size_t prime_size) {
        if (prime_size + 1 > sizeof(I) * 2) {  // the generated values can't be safely raised to the 4th power
            std::println("{}+1 > {}*2", prime_size, sizeof(I));
            return std::nullopt;
        }

        I floor = I{1} << (prime_size - 1);
        I ceiling = (floor << 1) - 1;

        I prime1 = rsa::impl::generate_random_prime(floor, ceiling);
        I prime2 = prime1;
        while (prime2 == prime1) {
            prime2 = rsa::impl::generate_random_prime(floor, ceiling);
        }

        I n = prime1 * prime2;
        I phi_n = (prime1 - 1) * (prime2 - 1);
        I pub = impl::generate_random_coprime(phi_n);
        I prv = impl::multiplicative_inverse(pub, phi_n).value();
        return Key<I>{n, pub, prv};
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
