#pragma once

#include <concepts>
#include <type_traits>

namespace rsa {
    template<std::size_t N>
    class bigint;

    namespace impl {
        template<typename T>
        struct is_bigint : std::false_type {};

        template<std::size_t N>
        struct is_bigint<rsa::bigint<N>> : std::true_type {};

        template<typename T>
        inline constexpr bool is_bigint_v = is_bigint<T>::value;
    }  // namespace impl

    template<typename T>
    concept integral = std::integral<T> or impl::is_bigint_v<T>;

    template<typename T>
    concept signed_integral = std::signed_integral<T> or impl::is_bigint_v<T>;

    template<typename T>
    concept unsigned_integral = std::unsigned_integral<T>;
}  // namespace rsa
