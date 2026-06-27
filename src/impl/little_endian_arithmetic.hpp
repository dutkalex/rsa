#pragma once

#include <concepts>
#include <span>
#include <utility>
#include <algorithm>

namespace rsa::impl {
    // Copies the binary representation of value into dest, producing a little endian representation
    template<std::unsigned_integral U>
    void little_endian_copy(std::span<std::byte> dest, U value) {
        if constexpr (sizeof(U) == 1) {
            dest[0] = static_cast<std::byte>(value);
        } else {
            for (std::size_t i = 0; i < std::min(sizeof(U), dest.size()); ++i) {
                dest[i] = static_cast<std::byte>(value & 0xff);
                value >>= 8;
            }
        }
    }

    // Computes "out = lhs + rhs" with the three parameters being little endian two's complement integers
    // lhs, rhs and out are not required to be the same length. If the sum does not fit, the last bytes are discarded.
    // lhs, rhs out out can alias to perform in-place updates and/or self-summation, but can't overlap
    inline void le_tc_add(std::span<std::byte const> lhs, std::span<std::byte const> rhs, std::span<std::byte> out) {
        auto lit = lhs.begin();
        auto rit = rhs.begin();
        auto wit = out.begin();
        unsigned carry = 0;

        while (wit != out.end()) {
            unsigned lv = 0;
            if (lit != lhs.end()) {
                lv = std::to_integer<unsigned>(*lit);
                ++lit;
            }

            unsigned rv = 0;
            if (rit != rhs.end()) {
                rv = std::to_integer<unsigned>(*rit);
                ++rit;
            }

            unsigned sum = lv + rv + carry;
            *wit = static_cast<std::byte>(sum & 0xff);
            carry = sum >> 8;
            ++wit;
        }
    }

    // Flip the sign of a little endian two's complement representation by inverting all the bits and then adding 1
    inline void le_tc_negate(std::span<std::byte> arg) {
        for (auto it = arg.begin(); it != arg.end(); ++it) {
            auto v = ~std::to_integer<unsigned>(*it);
            *it = static_cast<std::byte>(v);
        }

        unsigned carry = 1;
        for (auto it = arg.begin(); it != arg.end(); ++it) {
            auto v = std::to_integer<unsigned>(*it) + carry;
            *it = static_cast<std::byte>(v & 0xff);
            carry = v >> 8;
        }
    }

    // Computes "out = lhs * rhs" with the three parameters being little endian unsigned integers
    // lhs, rhs and out are not required to be the same length. If the product does not fit, the last bytes are
    // discarded. lhs and rhs can alias or overlap, but out must have exclusive access to its bytes
    inline void le_unsigned_multiply(
        std::span<std::byte const> lhs, std::span<std::byte const> rhs, std::span<std::byte> out
    ) {
        std::fill(out.begin(), out.end(), std::byte{0x00});

        int const Nl = lhs.size();
        int const Nr = rhs.size();
        int const No = out.size();
        for (int i = 0; i < Nl; ++i) {
            for (int j = 0; j < Nr; ++j) {
                auto carry = std::to_integer<unsigned>(lhs[i]) * std::to_integer<unsigned>(rhs[j]);
                int k = i + j;
                while (carry > 0 and k < No) {
                    auto v = std::to_integer<unsigned>(out[k]) + carry;
                    out[k] = static_cast<std::byte>(v & 0xff);
                    carry = v >> 8;
                    ++k;
                }
            }
        }
    }

    // Assumes rhs is not null
    template<size_t N>
    void le_unsigned_divide(
        std::array<std::byte, N> const& lhs, std::array<std::byte, N> const& rhs, std::array<std::byte, N>& quotient,
        std::array<std::byte, N>& remainder
    ) {
        std::fill(quotient.begin(), quotient.end(), std::byte{0x00});
        std::fill(remainder.begin(), remainder.end(), std::byte{0x00});

        constexpr size_t bits = N * 8;
        auto get_bit = [](std::array<std::byte, N> const& x, size_t bit) -> bool {
            return (std::to_integer<unsigned>(x[bit / 8]) >> (bit % 8)) & 0x01;
        };
        auto set_bit = [](std::array<std::byte, N>& x, size_t bit) -> void {
            x[bit / 8] |= std::byte(0x01 << (bit % 8));
        };
        auto shift_left = [](std::array<std::byte, N>& x) -> void {
            unsigned carry = 0;
            for (size_t i = 0; i < N; ++i) {
                unsigned v = std::to_integer<unsigned>(x[i]);
                x[i] = std::byte(((v << 1) | carry) & 0xff);
                carry = (v >> 7);
            }
        };

        std::array<std::byte, N> minus_rhs = rhs;
        le_tc_negate(minus_rhs);

        for (std::size_t i = 0; i < bits; ++i) {
            int k = bits - 1 - i;
            shift_left(remainder);

            if (get_bit(lhs, k)) {
                remainder[0] |= std::byte{0x01};
            }

            std::array<std::byte, N> test;
            le_tc_add(remainder, minus_rhs, test);
            if (std::to_integer<unsigned>(test[N - 1]) < 128) {  // i.e. remainder >= rhs
                remainder = test;
                set_bit(quotient, k);
            }
        }
    }
}  // namespace rsa::impl
