// RSA: R. Rivest, A. Shamir, L. Adleman

#include <optional>
#include <random>
#include <tuple>
#include <utility>

namespace rsa::impl {

// Returns res op (elm pow n) where pow is the power operator associated to op
// Example: if op is '*' then the result is 'res * elm^n'
// Example: if op is '+' then the result is 'res + elm*n'
template<std::regular T, std::integral I, typename SemiGroupOperator>
T power_accumulate_semigroup(T res, T elm, I n, SemiGroupOperator op){
    // assumes n >= 0
    if (n == I{0}){
        return res;
    }

    auto is_odd = [](T a) -> bool { return a % I{2} != 0; };
    auto half = [](T a) -> T { return a / I{2}; };

    while (true){
        if (is_odd(n)){
            res = op(res, elm);
            if (n == I{1}){
                return res;
            }
        }
        n = half(n);
        elm = op(elm, elm);
    }
}

// Returns elm pow n where pow is the power operator associated to op
template<std::regular T, std::integral I, typename SemiGroupOperator>
T power_semigroup(T elm, I n, SemiGroupOperator op){
    // assumes n > 0
    auto is_even = [](T a) -> bool { return a % I{2} == 0; };
    auto half = [](T a) -> T { return a / I{2}; };

    T res = elm;
    I i = n;

    while (is_even(i)){
        res = op(res, res);
        i = half(i);
    }

    if (i == I{1}){
        return res;
    }

    return power_accumulate_semigroup(res, op(res, res), half(i), op);
}

template<std::regular T, std::integral I, typename MonoidOperator>
T power_monoid(T elm, I n, MonoidOperator op){
    // assumes n >=0
    if (n == I{0}){
        return identity_element(op);
    }

    return power_semigroup(elm, n, op);
}

// Computes the greatest common divisor of a and b
// T must be a Euclidian domain for the algorithm to terminate
template<std::regular T>
T gcd(T a, T b){
    if (a < b){
        std::swap(a, b);
    }

    if (b == T{0}){
        return T{1};
    }

    while (true){
        a = a - b*(a/b);
        if (a == T{0}){
            return b;
        }

        b = b - a*(b/a);
        if (b == T{0}){
            return a;
        }
    }
}

// Computes the greatest common divisor of a and b and the multiplicative inverse of a modulo b if it exists
// T must be a Euclidian domain for the algorithm to terminate
template<std::regular T>
std::pair<T, T> extended_gcd(T a, T b){
    if (b == T{0}){
        return std::make_pair(T{1}, T{0});
    }

    T u = T{1};
    T v = T{0};
    while (true){
        T q = a/b;
        a = a - q*b;
        if (a == T{0}){
            return std::make_pair(b, v);
        }
        u = u - q*v;

        q = b/a;
        b = b - q*a;
        if (b == T{0}){
            return std::make_pair(a, u);
        }
        v = v - q*u;
    }
}

// Returns the multiplicative inverse of a modulo n if it exists, 0 otherwise
template<std::integral I>
std::optional<I> multiplicative_inverse(I a, I n){
    auto [gcd, inverse] = extended_gcd(a, n);
    if (gcd != I{1}){
        return std::nullopt;
    }
    return (inverse + n) % n;
}

template<std::integral I>
I smallest_divisor(I n){
    // assumes n > 0
    if (n % I{2} == 0){
        return I{2};
    }

    for (auto i = I{3}; i*i <= n; i += I{2}){
        if (n % i == 0){
            return i;
        }
    }

    return n;
}

template<std::integral I>
bool is_prime(I n){
    return n > I{1} and smallest_divisor(n) == n;
}

template<std::integral I>
struct modulo_multiply {
    I modulus;

    I operator()(I lhs, I rhs) const {
        return (lhs * rhs) % modulus;
    }
};

template<std::integral I>
I identity_element(modulo_multiply<I>){
    return I{1};
}

// Returns the multiplicative inverse of elm modulo p, with p a prime
template<std::integral I>
I multiplicative_inverse_prime(I elm, I p){
    // assumes e > 0 and p is prime
    // relies on the fact that (e^(p-1)) % p = 1
    auto op = modulo_multiply<I>{p};
    return power_monoid(elm, p-2, op);
}

template<std::integral I>
bool fermat_test(I n, I witness){
    // assumes 0 < witness < n
    auto op = modulo_multiply<I>{n};
    return power_semigroup(witness, n-1, op) == I{1};
}

// Determines whether n is prime with at least 75% accuracy
// Repeating the test with 100 different witness values brings the error
// probability below 1/2^200, which renders the test deterministic for practical
// puporses (see Knuth).
// n must be odd and greater than 1
// q and k must be such that n-1 = q * 2^k, with q odd
template<std::integral I>
bool miller_rabin_test(I n, I q, I k, I witness){
    auto op = modulo_multiply<I>{n};
    I x = power_semigroup(witness, q, op);
    I i = I{1};
    while (x != I{1} and x != n - I{1}){
        // invariant: x = w^(q * 2^i)
        ++i;
        if (i >= k){
            return false;
        }
        x = op(x, x);
    }

    return x != 1 or i == 1;
}

template<std::integral I>
class RandomGenerator {
  private:
    std::mt19937 gen_;
    std::uniform_int_distribution<I> distrib_;

  public:
    RandomGenerator(I floor, I ceiling, std::optional<int> seed = std::nullopt):
        gen_{seed ? *seed : std::random_device{}()},
        distrib_{floor, ceiling} {}

    I operator()(){
        return distrib_(gen_);
    }
};


// Generates a triplet (n, q, k) where n is the prime candidate
// n is guaranteed to be odd
// q and k are such that n-1 = q * 2^k with q odd
template<std::integral I>
std::tuple<I, I, I> generate_prime_candidate(RandomGenerator<I>& distrib){
    while (true){
        I n = distrib();
        if (n % 2 == 0){
            continue;
        }

        I q = n-1;
        I k = 0;
        while (q % 2 == 0){
            ++k;
            q = q/2;
        }

        return {n, q, k};
    }
}

// Generates a random prime number within the [floor, ceiling] range
template<std::integral I>
I generate_random_prime(I floor, I ceiling, std::optional<int> seed = std::nullopt){
    auto distrib = RandomGenerator<I>{floor, ceiling, seed};

    while (true){
        auto [n, q, k] = generate_prime_candidate(distrib);

        // Apply 100 times the Miller-Rabin test
        auto witness_distrib = RandomGenerator<I>{2, n-2, seed.transform([](int s){ return s+1; })};
        bool n_is_prime = true;
        for (int i = 0; i < 100 and n_is_prime; ++i){
            n_is_prime = miller_rabin_test(n, q, k, witness_distrib());
        }

        if (n_is_prime){
            return n;
        }
    }
}

// Generates a pair of distinct random prime numbers within the [floor, ceiling] range
// This function is useful for ensuring that the two primes are distinct, which is
// crucial for the RSA key generation algorithm
template<std::integral I>
std::pair<I, I> generate_random_pair_of_distinct_primes(I floor, I ceiling){
    I prime1 = rsa::impl::generate_random_prime(floor, ceiling);

    I prime2 = prime1;
    while (prime2 == prime1){
        prime2 = rsa::impl::generate_random_prime(floor, ceiling);
    }

    return std::pair{prime1, prime2};
}

template<std::integral I>
I generate_random_coprime(I n, std::optional<int> seed = std::nullopt){
    if (not seed){
        std::random_device dev;
        seed = dev();
    }

    std::mt19937 gen(seed.value());
    std::uniform_int_distribution<> distr(2, n-1);

    while (true){
        I candidate = distr(gen);
        if (gcd(n, candidate) == 1){
            return candidate;
        }
    }
}

} // namespace rsa::impl

namespace rsa {

template<std::integral I>
std::tuple<I, I, I> keygen(){
    I floor = 10;
    I ceiling = static_cast<I>(std::pow(std::numeric_limits<I>::max(), 0.25)); //
    auto [prime1, prime2] = impl::generate_random_pair_of_distinct_primes(floor, ceiling);
    I n = prime1*prime2;
    I phi_n = (prime1-1)*(prime2-1);
    I pub = impl::generate_random_coprime(phi_n);
    I prv = impl::multiplicative_inverse(pub, phi_n).value();
    return {n, pub, prv};
}

template<std::integral I>
auto encode(I plaintext, I pub, I n){
    return impl::power_semigroup(plaintext, pub, impl::modulo_multiply<I>{n});
}

template<std::integral I>
auto decode(I cyphertext, I prv, I n){
    return impl::power_semigroup(cyphertext, prv, impl::modulo_multiply<I>{n});
}

} // namespace rsa
