#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra -Wpedantic"
SCRIPT_NAME=$(basename "$0" .cpp)
# Boost.DLL requires boost_filesystem & boost_system, link them plus libdl
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out" \
    -lboost_filesystem -lboost_system -ldl
./"${SCRIPT_NAME}.out" "$@"
rm "${SCRIPT_NAME}.out"
exit
#endif

// Problem: 06.05 - Dynamic library version selection
// Description: allow a user to choose which shared library to load at runtime
//              and import a function named "test" from it using Boost.DLL.
//              Two separate dynamic libraries are provided with identical
//              signatures but different implementations.  The executable does
//              not need to be rebuilt when switching between them.

#include <iostream>
#include <string>
#include <functional>
#include <boost/dll.hpp>
#include <filesystem>

// forward declaration of test runner (defined later)
namespace tests { void run_all(); }

// Helper that loads the symbol "test" from a shared library and calls it.
// boost::dll::import_symbol returns a callable object (boost::function) which
// holds a reference to the library.  It throws if the library or symbol
// cannot be loaded, so the caller can handle errors.
void invoke_test_from_library(const std::string & path)
{
    auto test = boost::dll::import_symbol<void()>(path, "test");
    test();
}

int main(int argc, char * argv[])
{
    // allow running built-in tests via a flag so that CI can execute them
    if (argc > 1 && std::string(argv[1]) == "--test") {
        tests::run_all();
        return 0;
    }

    std::string lib_path;
    if (argc > 1) {
        lib_path = argv[1];
    } else {
        std::cout << "Enter path to shared library: ";
        if (!(std::cin >> lib_path))
            return 0;
    }

    try {
        invoke_test_from_library(lib_path);
    } catch (std::exception const & e) {
        std::cerr << "Error loading library: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

//================================================================================
// tests
//================================================================================

namespace tests {

    struct Logger {
        int passed = 0, failed = 0;

        void require(bool condition, const char * message) {
            if (condition) {
                passed++;
                std::cout << "passed: " << message << "\n";
            } else {
                failed++;
                std::cout << "failed: " << message << "\n";
            }
        }
    };

    // helper that captures std::cout output from invoke_test_from_library
    std::string capture_output(const std::string & lib)
    {
        std::ostringstream oss;
        auto * oldbuf = std::cout.rdbuf(oss.rdbuf());
        try {
            invoke_test_from_library(lib);
        } catch (...) {
            // ignore; output may already be captured or nothing to capture
        }
        std::cout.rdbuf(oldbuf);
        return oss.str();
    }

    // write and compile two simple shared libraries for testing
    bool build_test_libraries()
    {
        const char * code_v1 =
            "#include <iostream>\n"
            "extern \"C\" void test() { std::cout << \"library version 1\\n\"; }\n";
        const char * code_v2 =
            "#include <iostream>\n"
            "extern \"C\" void test() { std::cout << \"library version 2\\n\"; }\n";

        std::ofstream out;
        out.open("libv1.cpp");
        out << code_v1;
        out.close();
        out.open("libv2.cpp");
        out << code_v2;
        out.close();

        int ret1 = std::system("g++ -std=c++23 -fPIC -shared libv1.cpp -o libv1.so");
        int ret2 = std::system("g++ -std=c++23 -fPIC -shared libv2.cpp -o libv2.so");
        return ret1 == 0 && ret2 == 0 &&
               std::filesystem::exists("libv1.so") &&
               std::filesystem::exists("libv2.so");
    }

    void run_all()
    {
        Logger log;

        log.require(build_test_libraries(), "build two shared libraries");

        std::string out1 = capture_output("libv1.so");
        log.require(out1 == "library version 1\n", "load v1 and run test");

        std::string out2 = capture_output("libv2.so");
        log.require(out2 == "library version 2\n", "load v2 and run test");

        // also verify that specifying wrong name produces an error message
        bool threw = false;
        try {
            invoke_test_from_library("no_such_lib.so");
        } catch (...) {
            threw = true;
        }
        log.require(threw, "loading nonexistent library throws");

        std::cout << "\n" << log.passed << " passed, " << log.failed << " failed\n";
        if (log.failed > 0) std::exit(1);
    }
}


