#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O3 -m32"
SCRIPT_NAME=$(basename "$0" .cpp)
if ! $CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"; then
    echo "warning: -m32 build failed; falling back to native -O3 build" >&2
    CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O3"
    $CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
fi
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 10.05
// Description: Collision study for nine Partow string hash functions.

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

using hash_value_t = std::uint32_t;
using hash_function_t = hash_value_t (*)(std::string_view);

hash_value_t rs_hash(std::string_view text) {
    hash_value_t hash = 0;
    hash_value_t a = 63689;
    constexpr hash_value_t b = 378551;

    for (unsigned char ch : text) {
        hash = hash * a + ch;
        a *= b;
    }

    return hash;
}

hash_value_t js_hash(std::string_view text) {
    hash_value_t hash = 1315423911u;

    for (unsigned char ch : text) {
        hash ^= ((hash << 5u) + ch + (hash >> 2u));
    }

    return hash;
}

hash_value_t pjw_hash(std::string_view text) {
    constexpr hash_value_t bits_in_unsigned_int = 32;
    constexpr hash_value_t three_quarters = (bits_in_unsigned_int * 3) / 4;
    constexpr hash_value_t one_eighth = bits_in_unsigned_int / 8;
    constexpr hash_value_t high_bits = 0xFFFFFFFFu << (bits_in_unsigned_int - one_eighth);

    hash_value_t hash = 0;
    hash_value_t test = 0;

    for (unsigned char ch : text) {
        hash = (hash << one_eighth) + ch;
        if ((test = hash & high_bits) != 0) {
            hash = (hash ^ (test >> three_quarters)) & (~high_bits);
        }
    }

    return hash;
}

hash_value_t elf_hash(std::string_view text) {
    hash_value_t hash = 0;
    hash_value_t x = 0;

    for (unsigned char ch : text) {
        hash = (hash << 4u) + ch;
        if ((x = hash & 0xF0000000u) != 0) {
            hash ^= x >> 24u;
        }
        hash &= ~x;
    }

    return hash;
}

hash_value_t bkdr_hash(std::string_view text) {
    constexpr hash_value_t seed = 131;
    hash_value_t hash = 0;

    for (unsigned char ch : text) {
        hash = hash * seed + ch;
    }

    return hash;
}

hash_value_t sdbm_hash(std::string_view text) {
    hash_value_t hash = 0;

    for (unsigned char ch : text) {
        hash = ch + (hash << 6u) + (hash << 16u) - hash;
    }

    return hash;
}

hash_value_t djb_hash(std::string_view text) {
    hash_value_t hash = 5381;

    for (unsigned char ch : text) {
        hash = ((hash << 5u) + hash) + ch;
    }

    return hash;
}

hash_value_t dek_hash(std::string_view text) {
    hash_value_t hash = static_cast<hash_value_t>(text.size());

    for (unsigned char ch : text) {
        hash = ((hash << 5u) ^ (hash >> 27u)) ^ ch;
    }

    return hash;
}

hash_value_t ap_hash(std::string_view text) {
    hash_value_t hash = 0xAAAAAAAAu;

    for (std::size_t i = 0; i < text.size(); ++i) {
        const auto ch = static_cast<unsigned char>(text[i]);
        if ((i & 1u) == 0u) {
            hash ^= ((hash << 7u) ^ ch * (hash >> 3u));
        } else {
            hash ^= (~((hash << 11u) + (ch ^ (hash >> 5u))));
        }
    }

    return hash;
}

struct HashInfo {
    std::string_view name;
    hash_function_t function;
};

constexpr std::array<HashInfo, 9> kHashFunctions{{
    {"RSHash", rs_hash},
    {"JSHash", js_hash},
    {"PJWHash", pjw_hash},
    {"ELFHash", elf_hash},
    {"BKDRHash", bkdr_hash},
    {"SDBMHash", sdbm_hash},
    {"DJBHash", djb_hash},
    {"DEKHash", dek_hash},
    {"APHash", ap_hash},
}};

struct CollisionPoint {
    std::string_view name;
    std::size_t strings = 0;
    std::size_t collisions = 0;
};

std::vector<std::string> make_random_strings(std::size_t count, std::size_t length, unsigned seed) {
    std::mt19937 engine(seed);
    std::uniform_int_distribution<int> distribution('a', 'z');
    std::unordered_set<std::string> unique;
    unique.reserve(count * 2);

    std::string current(length, 'a');
    while (unique.size() < count) {
        for (char& ch : current) {
            ch = static_cast<char>(distribution(engine));
        }
        unique.insert(current);
    }

    return {unique.begin(), unique.end()};
}

std::vector<CollisionPoint> measure_collisions(const std::vector<std::string>& strings,
                                               std::size_t step) {
    std::vector<CollisionPoint> points;

    for (const HashInfo& info : kHashFunctions) {
        std::unordered_set<hash_value_t> hashes;
        hashes.reserve(strings.size() * 2);

        for (std::size_t i = 0; i < strings.size(); ++i) {
            hashes.insert(info.function(strings[i]));

            const std::size_t processed = i + 1;
            if (processed % step == 0 || processed == strings.size()) {
                points.push_back({info.name, processed, processed - hashes.size()});
            }
        }
    }

    return points;
}

void write_csv(const std::string& path, const std::vector<CollisionPoint>& points) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("cannot open output csv");
    }

    output << "function,strings,collisions\n";
    for (const CollisionPoint& point : points) {
        output << point.name << ',' << point.strings << ',' << point.collisions << '\n';
    }
}

std::vector<CollisionPoint> final_points(const std::vector<CollisionPoint>& points,
                                         std::size_t strings_count) {
    std::vector<CollisionPoint> result;
    for (const CollisionPoint& point : points) {
        if (point.strings == strings_count) {
            result.push_back(point);
        }
    }

    std::ranges::sort(result, {}, &CollisionPoint::collisions);
    return result;
}

namespace tests {

struct Logger {
    int passed = 0;
    int failed = 0;

    void require(bool condition, const char* message) {
        if (condition) {
            ++passed;
            std::cout << "[OK]   " << message << '\n';
        } else {
            ++failed;
            std::cout << "[FAIL] " << message << '\n';
        }
    }
};

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_random_strings_are_unique(Logger& log) {
    const auto strings = make_random_strings(1024, 12, 123u);
    std::unordered_set<std::string> unique(strings.begin(), strings.end());

    require(strings.size() == 1024, "requested number of strings");
    require(unique.size() == strings.size(), "strings must be unique");
    require(std::ranges::all_of(strings, [](const std::string& value) {
        return value.size() == 12 && std::ranges::all_of(value, [](char ch) {
            return 'a' <= ch && ch <= 'z';
        });
    }), "strings should be lowercase and equal length");

    log.require(true, "random string generation follows task constraints");
}

void test_all_hashes_are_used(Logger& log) {
    require(kHashFunctions.size() == 9, "exactly nine hash functions are required");
    log.require(true, "nine Partow hash functions are present");
}

void test_collision_measurement(Logger& log) {
    const auto strings = make_random_strings(256, 8, 456u);
    const auto points = measure_collisions(strings, 64);

    require(points.size() == kHashFunctions.size() * 4, "expected one point per step per function");
    require(std::ranges::all_of(points, [](const CollisionPoint& point) {
        return point.collisions <= point.strings;
    }), "collision count cannot exceed processed string count");

    log.require(true, "collision curves are measured");
}

void run_all() {
    Logger log;

    try {
        test_random_strings_are_unique(log);
        test_all_hashes_are_used(log);
        test_collision_measurement(log);
    } catch (const std::exception& ex) {
        log.require(false, ex.what());
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main(int argc, char** argv) {
    tests::run_all();

    const std::size_t count = argc > 1 ? static_cast<std::size_t>(std::stoull(argv[1])) : (1uz << 18uz);
    const std::size_t length = argc > 2 ? static_cast<std::size_t>(std::stoull(argv[2])) : 12uz;
    const std::size_t step = argc > 3 ? static_cast<std::size_t>(std::stoull(argv[3])) : (1uz << 12uz);

    const auto strings = make_random_strings(count, length, 20260510u);
    const auto points = measure_collisions(strings, step);
    write_csv("10.05_collisions.csv", points);

    const auto finals = final_points(points, strings.size());

    std::cout << "\nFinal collision counts for " << strings.size() << " strings:\n";
    for (const CollisionPoint& point : finals) {
        std::cout << point.name << ": " << point.collisions << '\n';
    }

    std::cout << "\nBest in this run: " << finals.front().name << "; worst: " << finals.back().name << ".\n";
    std::cout << "The curves grow roughly like a birthday-process curve: with a fixed 32-bit hash space, "
              << "collisions become more likely as the number of inserted strings grows.\n";
    std::cout << "CSV data written to 10.05_collisions.csv; run python3 10.05_plot.py to build the PNG graph.\n";

    return 0;
}
