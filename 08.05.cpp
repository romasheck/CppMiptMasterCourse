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

// Problem: 08.05

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

template <typename D = std::chrono::duration<double>>
class Timer {
public:
    using duration_t = D;
    using clock_t = std::chrono::steady_clock;

    explicit Timer(std::string scope) : scope_(std::move(scope)) {
    }

    void start() {
        if (running_) {
            throw std::runtime_error("timer is already running");
        }

        begin_ = clock_t::now();
        running_ = true;
    }

    void stop() {
        if (!running_) {
            throw std::runtime_error("timer is not running");
        }

        measurements_.push_back(std::chrono::duration_cast<duration_t>(clock_t::now() - begin_));
        running_ = false;
    }

    double average() const {
        if (measurements_.empty()) {
            return 0.0;
        }

        duration_t sum{};
        for (const duration_t measurement : measurements_) {
            sum += measurement;
        }

        return (sum / measurements_.size()).count();
    }

    const std::string& scope() const {
        return scope_;
    }

    std::size_t size() const {
        return measurements_.size();
    }

private:
    std::string scope_;
    bool running_ = false;
    typename clock_t::time_point begin_{};
    std::vector<duration_t> measurements_;
};

double calculate(std::size_t size) {
    double result = 0.0;

    for (std::size_t i = 0; i < size; ++i) {
        result += std::pow(std::sin(static_cast<double>(i)), 2.0) +
                  std::pow(std::cos(static_cast<double>(i)), 2.0);
    }

    return result;
}

bool equal(double lhs, double rhs, double epsilon = 1e-6) {
    return std::abs(lhs - rhs) < epsilon;
}

// global variable to prevent compiler from optimizing away the calculation
volatile double g_sink = 0.0;

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

    {
        Timer<> timer("empty");
        log.require(equal(timer.average(), 0.0), "average is zero without measurements");
        log.require(timer.size() == 0, "empty timer has zero measurements");
    }

    {
        Timer<> timer("series");
        timer.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timer.stop();

        timer.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        timer.stop();

        log.require(timer.size() == 2, "stop stores each measurement");
        log.require(timer.average() > 0.0, "average is positive after measurements");
    }

    {
        Timer<> timer("double-start");
        bool caught = false;
        try {
            timer.start();
            timer.start();
        } catch (const std::runtime_error&) {
            caught = true;
        }
        log.require(caught, "double start is rejected");
    }

    {
        Timer<> timer("stop-without-start");
        bool caught = false;
        try {
            timer.stop();
        } catch (const std::runtime_error&) {
            caught = true;
        }
        log.require(caught, "stop without start is rejected");
    }

    {
        log.require(equal(calculate(1000), 1000.0), "calculation from example still works");
    }

    std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
    if (log.failed != 0) {
        std::exit(1);
    }
}

}  // namespace tests

int main() {
    tests::run_all();

    Timer<> timer("calculate");

    for (int i = 0; i < 3; ++i) {
        timer.start();
        
        const double value = calculate(1000000);
        // prevent compiler from optimizing away the calculation
        g_sink = value;
        
        timer.stop();
        
        std::cout << timer.scope() << " run " << (i + 1) << ": " << value << '\n';
    }

    std::cout << timer.scope() << " average: " << timer.average() << " s\n";
    return 0;
}
