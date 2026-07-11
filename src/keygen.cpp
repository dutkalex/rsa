#include <filesystem>
#include <fstream>
#include <algorithm>
#include <charconv>
#include <optional>
#include <span>
#include <string_view>
#include <vector>
#include <print>

#include "rsa.hpp"
#include "bigint.hpp"

std::span<std::string_view> cmdline_args(int argc, const char* argv[]) {
    static std::vector<std::string_view> args(argv, argv + argc);
    return args;
}

std::optional<std::size_t> parse_input(std::span<std::string_view> args) {
    auto iter = std::find(args.begin(), args.end(), "-s");
    if (iter == args.end()) {
        std::println("Error! A prime size in bits must be specified with the -s flag");
        return std::nullopt;
    }

    ++iter;
    if (iter == args.end()) {
        std::println("Error! Expected an integral value after the -s flag");
        return std::nullopt;
    }

    std::size_t value = 0;

    auto str_begin = iter->data();
    auto str_end = str_begin + iter->size();
    auto [ptr, ec] = std::from_chars(str_begin, str_end, value);
    if (ec != std::errc{} or ptr != str_end) {
        std::println("Error! Could not parse {} as a valid prime size", *iter);
        return std::nullopt;
    }

    return value;
}

void dump_key_file(std::string_view n, std::string_view key, std::filesystem::path file) {
    std::ofstream out(file);
    if (!out) {
        std::println("Error! Could not write file {}", std::string{file.filename()});
        return;
    }

    out << n << '\n';
    out << key << '\n';
}

template<std::size_t N>
bool try_keygen(std::size_t size) {
    auto opt_key = rsa::keygen<rsa::bigint<N>>(size);
    if (not opt_key) {
        return false;
    }

    std::string n = opt_key->n.to_string();
    std::string pub = opt_key->pub.to_string();
    std::string prv = opt_key->prv.to_string();
    std::println("n={} pub={} prv={}", n, pub, prv);
    dump_key_file(n, pub, "rsa.pub");
    dump_key_file(n, prv, "rsa.prv");
    return true;
}

int main(int argc, const char* argv[]) {
    auto args = cmdline_args(argc, argv);
    auto opt_size = parse_input(args);
    if (not opt_size) {
        return 1;
    }

    bool ok = try_keygen<32>(*opt_size);
    if (not ok) {
        ok = try_keygen<64>(*opt_size);
    }
    if (not ok) {
        ok = try_keygen<128>(*opt_size);
    }
    if (not ok) {
        std::println("Error! Requested prime size is too large");
        return 2;
    }
    return 0;
}
