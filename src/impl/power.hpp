#pragma once

#include <concepts>

namespace rsa::impl {
    // Returns res op (elm pow n) where pow is the power operator associated to op
    // Example: if op is '*' then the result is 'res * elm^n'
    // Example: if op is '+' then the result is 'res + elm*n'
    template<std::regular T, std::integral I, typename SemiGroupOperator>
    T power_accumulate_semigroup(T res, T elm, I n, SemiGroupOperator op) {
        // assumes n >= 0
        if (n == I{0}) {
            return res;
        }

        auto is_odd = [](T a) -> bool { return a % I{2} != 0; };
        auto half = [](T a) -> T { return a / I{2}; };

        while (true) {
            if (is_odd(n)) {
                res = op(res, elm);
                if (n == I{1}) {
                    return res;
                }
            }
            n = half(n);
            elm = op(elm, elm);
        }
    }

    // Returns elm pow n where pow is the power operator associated to op
    template<std::regular T, std::integral I, typename SemiGroupOperator>
    T power_semigroup(T elm, I n, SemiGroupOperator op) {
        // assumes n > 0
        auto is_even = [](T a) -> bool { return a % I{2} == 0; };
        auto half = [](T a) -> T { return a / I{2}; };

        T res = elm;
        I i = n;

        while (is_even(i)) {
            res = op(res, res);
            i = half(i);
        }

        if (i == I{1}) {
            return res;
        }

        return power_accumulate_semigroup(res, op(res, res), half(i), op);
    }

    template<std::regular T, std::integral I, typename MonoidOperator>
    T power_monoid(T elm, I n, MonoidOperator op) {
        // assumes n >=0
        if (n == I{0}) {
            return identity_element(op);
        }

        return power_semigroup(elm, n, op);
    }
}  // namespace rsa::impl
