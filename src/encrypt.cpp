#include <span>
#include <string_view>
#include <vector>
#include <print>

#include "rsa.hpp"

std::span<std::string_view> cmdline_args(int argc, const char* argv[]){
    static std::vector<std::string_view> args(argv, argv + argc);
    return args;
}

int main(int argc, const char* argv[]){
    auto args = cmdline_args(argc, argv);
    for (auto const& a: args){
        std::println("{}", a);
    }
    return 0;
}
