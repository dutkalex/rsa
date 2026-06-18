#include <utility>

namespace rsa::impl {
    // Computes the greatest common divisor of a and b
    // T must be a Euclidian domain for the algorithm to terminate
    template<std::regular T>
    T gcd(T a, T b) {
        if (a == T{0} or b == T{0}) {
            return T{1};
        }

        while (true) {
            a = a - b * (a / b);
            if (a == T{0}) {
                return b;
            }

            b = b - a * (b / a);
            if (b == T{0}) {
                return a;
            }
        }
    }

    // Computes the greatest common divisor of a and b and the multiplicative inverse of a modulo b if it exists
    // T must be a Euclidian domain for the algorithm to terminate
    // Note that the multiplicative inverse of b modulo a can easily be retrieved using Bezout's identity:
    // ax + by = gcd(a, b) with x the inverse of a mod b and y the inverse of b mod a.
    template<std::regular T>
    std::pair<T, T> extended_gcd(T a, T b) {
        if (a == T{0} or b == T{0}) {
            return std::pair{T{1}, T{0}};
        }

        T u = T{1};
        T v = T{0};
        while (true) {
            T q = a / b;
            a = a - q * b;
            if (a == T{0}) {
                return std::pair{b, v};
            }
            u = u - q * v;

            q = b / a;
            b = b - q * a;
            if (b == T{0}) {
                return std::pair{a, u};
            }
            v = v - q * u;
        }
    }
}  // namespace rsa::impl
