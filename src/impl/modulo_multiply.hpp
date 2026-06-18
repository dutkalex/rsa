#pragma once

#include <optional>

#include "gcd.hpp"
#include "power.hpp"

namespace rsa::impl {
    // Functor representing modulo multiplication
    // Noting n the modulus, this operation is defined on the range [0, n-1]
    // Behavior for inputs outside this range is undefined
    template<std::integral I>
    class modulo_multiply {
      private:
        I modulus_;

      public:
        modulo_multiply(I modulus): modulus_{modulus} {}

        I operator()(I lhs, I rhs) const {
            return (lhs * rhs) % modulus_;
        }
    };

    template<std::integral I>
    I identity_element(modulo_multiply<I>) {
        return I{1};
    }

    // Returns the multiplicative inverse of a modulo n if it exists
    // Assumes a > 0
    template<std::integral I>
    std::optional<I> multiplicative_inverse(I a, I n) {
        auto [gcd, inverse] = extended_gcd(a, n);
        if (gcd != I{1}) {
            return std::nullopt;
        }
        return (inverse + n) % n; // to ensure the inverse is in [0, n-1]
    }

    // Returns the multiplicative inverse of elm modulo p, with p a prime
    // Assumes a > 0
    template<std::integral I>
    I multiplicative_inverse_prime(I a, I p) {
        // relies on the fact that (e^(p-1)) % p = 1 if p is prime
        return power_monoid(a, p - 2, modulo_multiply<I>{p});
    }
}
