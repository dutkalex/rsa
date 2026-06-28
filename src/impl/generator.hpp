#pragma once

#include <random>

#include "concepts.hpp"

namespace rsa::impl {
    // Uniform distribution positive number generator
    template<rsa::integral I>
    class RandomGenerator {
      private:
        std::mt19937 gen_;
        std::uniform_int_distribution<std::uint8_t> distrib_;
        I floor_;
        I ceiling_;

      public:
        // Precondition: floor and ceiling must be positive, with ceiling > floor
        RandomGenerator(I floor, I ceiling, std::optional<int> seed = std::nullopt)
            : gen_{seed ? *seed : std::random_device{}()}, distrib_{0x00, 0xff}, floor_{floor}, ceiling_{ceiling} {}

        I operator()() {
            I result = 0;
            result += static_cast<I>(distrib_(gen_) / 2);  // ensures result > 0
            for (std::size_t i = 1; i < sizeof(I); ++i) {
                result *= I{256};
                result += I{distrib_(gen_)};
            }

            result %= result % (ceiling_ - floor_);
            result += floor_;
            return result;
        }
    };
}  // namespace rsa::impl
