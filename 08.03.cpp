#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic -O2"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 08.03

#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>

static_assert(sizeof(int) == 4);
static_assert(sizeof(float) == 4);
static_assert(std::numeric_limits<float>::is_iec559);

namespace ieee754_float32 {

constexpr unsigned int kMantissaBits = 23u;
constexpr unsigned int kExponentMask = 0xffu;
constexpr unsigned int kMantissaMask = 0x7fffffu;
constexpr int kExponentBias = 127;
constexpr int kDenormalizedLogOffset = kExponentBias + static_cast<int>(kMantissaBits) - 1;

}  // namespace ieee754_float32

int highest_bit_index(unsigned int value) {
    int result = -1;

    while (value != 0u) {
        value >>= 1u;
        ++result;
    }

    return result;
}

int floor_log2_int(int value) {
    if (value <= 0) {
        throw std::runtime_error("int value must be positive");
    }

    return highest_bit_index(static_cast<unsigned int>(value));
}

int floor_log2_float(float value) {
    if (!(value > 0.0F)) {
        throw std::runtime_error("float value must be positive");
    }

    union FloatBits {
        float value;
        unsigned int bits;
    };

    const FloatBits raw{value};
    const unsigned int exponent =
        (raw.bits >> ieee754_float32::kMantissaBits) & ieee754_float32::kExponentMask;
    const unsigned int mantissa = raw.bits & ieee754_float32::kMantissaMask;

    if (exponent == ieee754_float32::kExponentMask) {
        throw std::runtime_error("float value must be finite");
    }

    if (exponent != 0u) {
        return static_cast<int>(exponent) - ieee754_float32::kExponentBias;
    }

    return highest_bit_index(mantissa) - ieee754_float32::kDenormalizedLogOffset;
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

void run_all() {
    Logger log;

    log.require(floor_log2_int(1) == 0, "log2 for int 1");
    log.require(floor_log2_int(2) == 1, "log2 for int 2");
    log.require(floor_log2_int(3) == 1, "log2 for int 3");
    log.require(floor_log2_int(255) == 7, "log2 for int 255");
    log.require(floor_log2_int(256) == 8, "log2 for int 256");
    log.require(floor_log2_int(1023) == 9, "log2 for int 1023");

    log.require(floor_log2_float(1.0F) == 0, "log2 for float 1");
    log.require(floor_log2_float(2.0F) == 1, "log2 for float 2");
    log.require(floor_log2_float(3.5F) == 1, "log2 for float 3.5");
    log.require(floor_log2_float(0.75F) == -1, "log2 for float 0.75");
    log.require(floor_log2_float(0.5F) == -1, "log2 for float 0.5");
    log.require(floor_log2_float(0.125F) == -3, "log2 for float 0.125");
    log.require(floor_log2_float(std::numeric_limits<float>::denorm_min()) == -149,
                "log2 for minimal denormalized float");
    log.require(floor_log2_float(std::numeric_limits<float>::min()) == -126,
                "log2 for minimal normalized float");

    {
        union FloatBits {
            float value;
            unsigned int bits;
        };

        FloatBits denormalized{};
        denormalized.bits = 0x00000100u;
        log.require(floor_log2_float(denormalized.value) == -141,
                    "log2 for denormalized float with higher mantissa bit");
    }

    {
        bool caught = false;
        try {
            (void)floor_log2_int(0);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        log.require(caught, "non-positive int is rejected");
    }

    {
        bool caught = false;
        try {
            (void)floor_log2_float(0.0F);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        log.require(caught, "non-positive float is rejected");
    }

    {
        bool caught = false;
        try {
            (void)floor_log2_float(std::numeric_limits<float>::infinity());
        } catch (const std::runtime_error&) {
            caught = true;
        }
        log.require(caught, "infinity is rejected");
    }

    {
        bool caught = false;
        try {
            (void)floor_log2_float(std::numeric_limits<float>::quiet_NaN());
        } catch (const std::runtime_error&) {
            caught = true;
        }
        log.require(caught, "NaN is rejected");
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
