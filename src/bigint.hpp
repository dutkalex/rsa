#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <ranges>
#include <span>
#include <string>
#include <vector>

namespace rsa::impl {
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
}  // namespace rsa::impl

namespace rsa {
    // A signed integer class for storing large integers
    // The size is customizable and defaults to 256 bits
    // Values are represented using little endian two's complement
    template<std::size_t N = 32>
    class bigint {
      private:
        std::array<std::byte, N> raw_;

        [[nodiscard]] bool is_negative() const {
            return std::to_integer<unsigned>(raw_[N - 1]) >= 128;
        }

        [[nodiscard]] std::array<std::byte, N> magnitude() const {
            std::array<std::byte, N> mag = raw_;
            if (is_negative()) {
                for (int i = 0; i < N; ++i) {
                    mag[i] = static_cast<std::byte>(~std::to_integer<unsigned>(mag[i]));
                }

                unsigned carry = 1;
                for (int i = 0; i < N; ++i) {
                    auto v = std::to_integer<unsigned>(mag[i]) + carry;
                    mag[i] = static_cast<std::byte>(v & 0xff);
                    carry = v >> 8;
                }
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

        bigint& operator+=(bigint const& other) {
            impl::le_tc_add(raw_, other.raw_, raw_);
            return *this;
        }

        friend bigint operator+(bigint const& lhs, bigint const& rhs) {
            bigint sum;
            impl::le_tc_add(lhs.raw_, rhs.raw_, sum.raw_);
            return sum;
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
