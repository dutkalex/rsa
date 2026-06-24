#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <span>
#include <string>
#include <vector>
#include <algorithm>

#include "concepts.hpp"

namespace rsa::impl {
    // Copies the binary representation of value into dest, producing a little endian representation
    template<std::unsigned_integral U>
    void little_endian_copy(std::span<std::byte> dest, U value) {
        for (int i = 0; i < std::min(sizeof(U), dest.size()); ++i) {
            dest[i] = static_cast<std::byte>(value & 0xff);
            value >>= 8;
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
    // lhs, rhs and out are not required to be the same length. If the product does not fit, the last bytes are discarded.
    // lhs and rhs can alias or overlap, but out must have exclusive access to its bytes
    inline void le_unsigned_multiply(std::span<std::byte const> lhs, std::span<std::byte const> rhs, std::span<std::byte> out) {
        std::fill(out.begin(), out.end(), std::byte{0x00});

        int const Nl = lhs.size();
        int const Nr = rhs.size();
        int const No = out.size();
        for (int i = 0; i < Nl; ++i) {
            for (int j = 0; j < Nr; ++j) {
                auto carry = std::to_integer<unsigned>(lhs[i]) * std::to_integer<unsigned>(rhs[j]);
                int k = i+j;
                while (carry > 0 and k < No){
                    auto v = std::to_integer<unsigned>(out[k]) + carry;
                    out[k] = static_cast<std::byte>(v & 0xff);
                    carry = v >> 8;
                    ++k;
                }
            }
        }
    }
}  // namespace rsa::impl

namespace rsa {
    // A signed integer class for storing large integers
    // The size is customizable and defaults to 256 bits
    // This type is regular and has value semantics
    // Values are represented using little endian two's complement
    template<std::size_t N = 32>
    class bigint {
      private:
        std::array<std::byte, N> raw_;

        [[nodiscard]] bool is_zero() const {
            for (auto byte : raw_) {
                if (std::to_integer<unsigned>(byte) != 0) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] bool is_negative() const {
            return std::to_integer<unsigned>(raw_[N - 1]) >= 128;
        }

        [[nodiscard]] std::array<std::byte, N> magnitude() const {
            std::array<std::byte, N> mag = raw_;
            if (is_negative()) {
                impl::le_tc_negate(mag);
            }
            return mag;
        }

      public:
        bigint() = default;

        template<std::unsigned_integral U>
            requires(sizeof(U) <= N)
        bigint(U value) {
            raw_.fill(std::byte{0x00});
            impl::little_endian_copy(raw_, value);
        }

        template<std::signed_integral I>
            requires(sizeof(I) <= N)
        bigint(I value) {
            raw_.fill(value < 0 ? std::byte{0xff} : std::byte{0x00});
            impl::little_endian_copy(raw_, static_cast<std::make_unsigned_t<I>>(value));
        }

        // Only there for consistency with unary minus
        friend bigint operator+(bigint const& arg) {
            bigint copy = arg;
            return copy;
        }

        friend bigint operator-(bigint const& arg) {
            bigint copy = arg;
            impl::le_tc_negate(copy.raw_);
            return copy;
        }

        friend bigint operator+(bigint const& lhs, bigint const& rhs) {
            bigint sum;
            impl::le_tc_add(lhs.raw_, rhs.raw_, sum.raw_);
            return sum;
        }

        friend bigint operator-(bigint const& lhs, bigint const& rhs) {
            bigint diff = -rhs;
            impl::le_tc_add(lhs.raw_, diff.raw_, diff.raw_);
            return diff;
        }

        friend bigint operator*(bigint const& lhs, bigint const& rhs){
            auto lhs_mag = lhs.magnitude();
            auto rhs_mag = rhs.magnitude();

            bigint prod;
            impl::le_unsigned_multiply(lhs_mag, rhs_mag, prod.raw_);
            if (lhs.is_negative() xor rhs.is_negative()){
                impl::le_tc_negate(prod.raw_);
            }
            return prod;
        }

        bigint& operator+=(bigint const& other) {
            impl::le_tc_add(raw_, other.raw_, raw_);
            return *this;
        }

        bigint& operator-=(bigint const& other) {
            bigint tmp = -other;
            impl::le_tc_add(raw_, tmp.raw_, raw_);
            return *this;
        }

        bigint& operator*=(bigint const& other){
            bigint tmp = (*this) * other;
            *this = tmp;
            return *this;
        }

        friend std::strong_ordering operator<=>(bigint const& lhs, bigint const& rhs) {
            auto diff = lhs - rhs;
            if (diff.is_negative()) {
                return std::strong_ordering::less;
            }
            if (diff.is_zero()) {
                return std::strong_ordering::equal;
            }
            return std::strong_ordering::greater;
        }

        friend bool operator==(bigint const& lhs, bigint const& rhs) {
            return (lhs <=> rhs) == std::strong_ordering::equal;
        }

        [[nodiscard]] std::string to_string() const {
            bool const neg = is_negative();
            auto const mag = magnitude();

            std::vector<std::uint8_t> digits;  // LE decimal digit sequence
            digits.push_back(0);

            for (auto it = mag.rbegin(); it != mag.rend(); ++it) {
                int carry = std::to_integer<int>(*it);

                for (size_t i = 0; i < digits.size(); ++i) {
                    int x = digits[i] * 256 + carry;
                    digits[i] = x % 10;
                    carry = x / 10;
                }

                while (carry != 0) {
                    digits.push_back(carry % 10);
                    carry = carry / 10;
                }
            }

            while (digits.size() > 1 and digits.back() == 0) {
                digits.pop_back();
            }

            std::string retval;
            retval.reserve(digits.size() + (neg ? 1 : 0));
            if (neg) {
                retval.push_back('-');
            }
            for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
                retval.push_back(static_cast<char>('0' + *it));
            }
            return retval;
        }
    };

}  // namespace rsa

static_assert(std::regular<rsa::bigint<>>);
static_assert(rsa::integral<rsa::bigint<>>);
