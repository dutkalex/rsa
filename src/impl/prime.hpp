#pragma once

#include "modulo_multiply.hpp"
#include "power.hpp"

namespace rsa::impl {
    // Computes the smallest divisor of n by brute force
    // Assumes n > 0
    template<rsa::integral I>
    I smallest_divisor(I n) {
        if (n % I{2} == 0) {
            return I{2};
        }

        for (auto i = I{3}; i * i <= n; i += I{2}) {
            if (n % i == 0) {
                return i;
            }
        }

        return n;
    }

    // Determines whether n is prime by brute force
    template<rsa::integral I>
    bool is_prime(I n) {
        return n > I{1} and smallest_divisor(n) == n;
    }

    // Probabilistic test for determining if n is prime
    // The witness is a random value which will be used to test the identity w^(n-1) = 1 mod n
    // for any w if n is prime
    // Assumes 0 < witness < n
    template<rsa::integral I>
    bool fermat_test(I n, I witness) {
        return power_semigroup(witness, n - 1, modulo_multiply<I>{n}) == I{1};
    }

    // Determines whether n is prime with at least 75% accuracy
    // This test is a variation on the Fermat test which relies on the fact that only 1 and -1 (i.e. n-1)
    // are their own multiplicative inverses modulo n.
    // Repeating the test with 100 different witness values brings the error
    // probability below 1/2^200, which renders the test deterministic for practical
    // puporses (see Knuth).
    // n must be odd and greater than 1
    // q and k must be such that n-1 = q * 2^k, with q odd
    // Assumes 0 < witness < n
    template<rsa::integral I>
    bool miller_rabin_test(I n, I q, I k, I witness) {
        auto op = modulo_multiply<I>{n};
        I x = power_semigroup(witness, q, op);
        I i = I{1};
        while (x != I{1} and x != n - I{1}) {
            // invariant: x = w^(q * 2^i)
            ++i;
            if (i >= k) {
                return false;
            }
            x = op(x, x);
        }

        return x != 1 or i == 1;
    }
}  // namespace rsa::impl
