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

// Problem: 10.01
// Description: Track std::vector capacity growth and estimate the growth factor.

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct CapacityEvent {
    std::size_t size = 0;
    std::size_t previous_capacity = 0;
    std::size_t new_capacity = 0;

    double factor() const {
        if (previous_capacity == 0) {
            return 0.0;
        }
        return static_cast<double>(new_capacity) / static_cast<double>(previous_capacity);
    }
};

template <typename T>
std::vector<CapacityEvent> track_capacity_growth(std::size_t push_count) {
    std::vector<T> data;
    std::vector<CapacityEvent> events;

    std::size_t previous_capacity = data.capacity();
    for (std::size_t i = 0; i < push_count; ++i) {
        data.push_back(static_cast<T>(i));

        if (data.capacity() != previous_capacity) {
            events.push_back({data.size(), previous_capacity, data.capacity()});
            previous_capacity = data.capacity();
        }
    }

    return events;
}

double estimate_growth_factor(const std::vector<CapacityEvent>& events) {
    double sum = 0.0;
    std::size_t count = 0;

    for (const CapacityEvent& event : events) {
        if (event.previous_capacity == 0) {
            continue;
        }
        sum += event.factor();
        ++count;
    }

    return count == 0 ? 0.0 : sum / static_cast<double>(count);
}

bool almost_equal(double lhs, double rhs, double epsilon = 1e-9) {
    return std::abs(lhs - rhs) < epsilon;
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

void test_capacity_changes_are_recorded(Logger& log) {
    const auto events = track_capacity_growth<int>(128);

    require(!events.empty(), "expected capacity growth events");
    require(events.front().previous_capacity == 0, "first event should start from zero capacity");
    require(events.front().new_capacity >= 1, "first growth should create capacity");
    log.require(true, "capacity changes are tracked");
}

void test_capacity_strictly_grows(Logger& log) {
    const auto events = track_capacity_growth<int>(512);

    for (const CapacityEvent& event : events) {
        require(event.new_capacity > event.previous_capacity, "capacity must strictly increase");
        require(event.size <= event.new_capacity, "size must fit in current capacity");
    }

    log.require(true, "recorded capacities strictly increase");
}

void test_growth_factor_is_consistent(Logger& log) {
    const auto events = track_capacity_growth<int>(4096);

    require(events.size() >= 3, "expected multiple growth steps");

    for (std::size_t i = 2; i < events.size(); ++i) {
        require(almost_equal(events[i].factor(), events[1].factor()),
                "expected stable growth factor after initial step");
    }

    log.require(true, "growth factor is stable across reallocations");
}

void test_observed_factor_for_int(Logger& log) {
    const auto events = track_capacity_growth<int>(4096);
    const double factor = estimate_growth_factor(events);

    require(almost_equal(factor, 2.0), "expected average growth factor 2.0 for this STL build");
    log.require(true, "observed growth factor is 2.0");
}

void test_other_type_has_same_pattern(Logger& log) {
    const auto int_events = track_capacity_growth<int>(1024);
    const auto double_events = track_capacity_growth<double>(1024);

    require(int_events.size() == double_events.size(), "expected same number of reallocations");
    for (std::size_t i = 0; i < int_events.size(); ++i) {
        require(int_events[i].new_capacity == double_events[i].new_capacity,
                "expected same capacity sequence for int and double");
    }

    log.require(true, "capacity growth pattern matches for another type");
}

void run_all() {
    Logger log;

    try {
        test_capacity_changes_are_recorded(log);
        test_capacity_strictly_grows(log);
        test_growth_factor_is_consistent(log);
        test_observed_factor_for_int(log);
        test_other_type_has_same_pattern(log);
    } catch (const std::exception& ex) {
        log.require(false, ex.what());
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main() {
    tests::run_all();

    const auto events = track_capacity_growth<int>(128);
    const double factor = estimate_growth_factor(events);

    std::cout << "\nstd::vector<int> capacity growth:\n";
    std::cout << " size | old cap | new cap | factor\n";
    for (const CapacityEvent& event : events) {
        std::cout << std::setw(5) << event.size << " | " << std::setw(7) << event.previous_capacity << " | "
                  << std::setw(7) << event.new_capacity << " | ";

        if (event.previous_capacity == 0) {
            std::cout << "n/a\n";
        } else {
            std::cout << std::fixed << std::setprecision(2) << event.factor() << '\n';
        }
    }

    std::cout << "Estimated growth factor: " << std::fixed << std::setprecision(2) << factor << '\n';
    return 0;
}
