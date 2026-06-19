#pragma once

#include <optional>
#include <random>
#include <tuple>
#include <utility>

#include "impl/power.hpp"
#include "impl/gcd.hpp"
#include "impl/modulo_multiply.hpp"
#include "impl/prime.hpp"

namespace rsa::impl {
    // Uniform distribution number generator
    template<std::integral I>
    class RandomGenerator {
      private:
        std::mt19937 gen_;
        std::uniform_int_distribution<I> distrib_;

      public:
        RandomGenerator(I floor, I ceiling, std::optional<int> seed = std::nullopt)
            : gen_{seed ? *seed : std::random_device{}()}, distrib_{floor, ceiling} {}

        I operator()() {
            return distrib_(gen_);
        }
    };

    // Generates a triplet (n, q, k) where n is the prime candidate
    // n is guaranteed to be odd
    // q and k are such that n-1 = q * 2^k with q odd
    template<std::integral I>
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
    template<std::integral I>
    I generate_random_prime(I floor, I ceiling, std::optional<int> seed = std::nullopt) {
        auto distrib = RandomGenerator<I>{floor, ceiling, seed};

        while (true) {
            auto [n, q, k] = generate_prime_candidate(distrib);

            // Apply 100 times the Miller-Rabin test
            auto witness_distrib = RandomGenerator<I>{2, n - 2, seed.transform([](int s) { return s + 1; })};
            bool n_is_prime = true;
            for (int i = 0; i < 100 and n_is_prime; ++i) {
                n_is_prime = miller_rabin_test(n, q, k, witness_distrib());
            }

            if (n_is_prime) {
                return n;
            }
        }
    }

    // Generates a pair of distinct random prime numbers within the [floor, ceiling] range
    // This function is useful for ensuring that the two primes are distinct, which is
    // crucial for the RSA key generation algorithm
    template<std::integral I>
    std::pair<I, I> generate_random_pair_of_distinct_primes(I floor, I ceiling) {
        I prime1 = rsa::impl::generate_random_prime(floor, ceiling);

        I prime2 = prime1;
        while (prime2 == prime1) {
            prime2 = rsa::impl::generate_random_prime(floor, ceiling);
        }

        return std::pair{prime1, prime2};
    }

    // Generates a number which is coprime with n in the range [2, n-1]
    template<std::integral I>
    I generate_random_coprime(I n, std::optional<int> seed = std::nullopt) {
        if (not seed) {
            std::random_device dev;
            seed = dev();
        }

        std::mt19937 gen(seed.value());
        std::uniform_int_distribution<> distr(2, n - 1);

        while (true) {
            I candidate = distr(gen);
            if (gcd(n, candidate) == 1) {
                return candidate;
            }
        }
    }
}  // namespace rsa::impl

// RSA: R. Rivest, A. Shamir, L. Adleman
namespace rsa {
    template<std::integral I>
    std::tuple<I, I, I> keygen() {
        I floor = 10;
        I ceiling = static_cast<I>(std::pow(std::numeric_limits<I>::max(), 0.25));
        auto [prime1, prime2] = impl::generate_random_pair_of_distinct_primes(floor, ceiling);
        I n = prime1 * prime2;
        I phi_n = (prime1 - 1) * (prime2 - 1);
        I pub = impl::generate_random_coprime(phi_n);
        I prv = impl::multiplicative_inverse(pub, phi_n).value();
        return {n, pub, prv};
    }

    template<std::integral I>
    auto encode(I plaintext, I pub, I n) {
        return impl::power_semigroup(plaintext, pub, impl::modulo_multiply<I>{n});
    }

    template<std::integral I>
    auto decode(I cyphertext, I prv, I n) {
        return impl::power_semigroup(cyphertext, prv, impl::modulo_multiply<I>{n});
    }
}  // namespace rsa
