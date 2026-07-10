#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

#include "concepts.hpp"
#include "impl/little_endian_arithmetic.hpp"

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

        template<std::size_t M>
        friend class bigint;

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

        template<std::size_t M>
            requires(M < N)
        bigint(bigint<M> const& smaller) {
            raw_.fill(smaller < 0 ? std::byte{0xff} : std::byte{0x00});
            std::copy_n(smaller.raw_.begin(), M, raw_.begin());
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

        friend bigint operator*(bigint const& lhs, bigint const& rhs) {
            auto lhs_mag = lhs.magnitude();
            auto rhs_mag = rhs.magnitude();

            bigint prod;
            impl::le_unsigned_multiply(lhs_mag, rhs_mag, prod.raw_);
            if (lhs.is_negative() xor rhs.is_negative()) {
                impl::le_tc_negate(prod.raw_);
            }
            return prod;
        }

        friend std::pair<bigint, bigint> divmod(bigint const& lhs, bigint const& rhs) {
            if (rhs.is_zero()) {
                return std::pair{bigint{0}, lhs};
            }

            auto lhs_mag = lhs.magnitude();
            auto rhs_mag = rhs.magnitude();

            bigint quotient;
            bigint remainder;
            impl::le_unsigned_divide(lhs_mag, rhs_mag, quotient.raw_, remainder.raw_);

            if (lhs.is_negative() xor rhs.is_negative()) {
                impl::le_tc_negate(quotient.raw_);
            }
            if (lhs.is_negative()) {
                impl::le_tc_negate(remainder.raw_);
            }
            return std::pair{quotient, remainder};
        }

        friend bigint operator/(bigint const& lhs, bigint const& rhs) {
            auto [quotient, _] = divmod(lhs, rhs);
            return quotient;
        }

        friend bigint operator%(bigint const& lhs, bigint const& rhs) {
            auto [_, remainder] = divmod(lhs, rhs);
            return remainder;
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

        bigint& operator*=(bigint const& other) {
            bigint tmp = (*this) * other;
            *this = tmp;
            return *this;
        }

        bigint& operator/=(bigint const& other) {
            bigint tmp = (*this) / other;
            *this = tmp;
            return *this;
        }

        bigint& operator%=(bigint const& other) {
            bigint tmp = (*this) % other;
            *this = tmp;
            return *this;
        }

        bigint& operator++() {
            *this += 1;
            return *this;
        }

        bigint& operator--() {
            *this -= 1;
            return *this;
        }

        friend bigint operator<<(bigint const& value, std::size_t n) {
            std::size_t n_bytes = n / 8;
            bigint res = 0;
            unsigned carry = 0;
            for (std::size_t i = 0; i < N - n_bytes; ++i) {
                carry += std::to_integer<unsigned>(value.raw_[i]) << (n % 8);
                res.raw_[i + n_bytes] = static_cast<std::byte>(carry & 0xff);
                carry >>= 8;
            }
            return res;
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
                    int tmp = digits[i] * 256 + carry;
                    digits[i] = tmp % 10;
                    carry = tmp / 10;
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
