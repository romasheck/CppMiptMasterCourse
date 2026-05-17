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

// Problem: 12.04
// Description: Extract all email addresses and their domains from text using regex groups.

#include <iostream>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

using EmailDomainPair = std::pair<std::string, std::string>;

std::vector<EmailDomainPair> extract_emails_and_domains(std::string_view text) {
    std::string input(text);
    const std::regex pattern(R"(([A-Za-z0-9._%+-]+)@([A-Za-z0-9.-]+\.[A-Za-z]{2,}))");

    std::vector<EmailDomainPair> result;
    auto begin = std::cbegin(input);
    std::smatch matches;

    while (std::regex_search(begin, std::cend(input), matches, pattern)) {
        result.emplace_back(matches[0].str(), matches[2].str());
        begin = matches.suffix().first;
    }

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

    void run_all() {
        Logger log;

        const auto text = R"(Contact list:
            alice@example.com,
            bob.smith@sub.domain.org and
            support@company.io are available.
            Ignore invalid@address and plainword@.
        )";

        const auto result = extract_emails_and_domains(text);
        const std::vector<EmailDomainPair> expected = {
            {"alice@example.com", "example.com"},
            {"bob.smith@sub.domain.org", "sub.domain.org"},
            {"support@company.io", "company.io"}
        };

        log.require(result == expected, "Extract all valid email addresses and domains");

        log.require(extract_emails_and_domains("No emails here.").empty(), "Return empty list when no emails are present");

        const auto single = extract_emails_and_domains(R"(reply to admin@site.net for details)");
        log.require(single.size() == 1 && single[0].first == "admin@site.net" && single[0].second == "site.net",
                    "Extract a single email and its domain");

        std::cout << '\n' << log.passed << " passed, " << log.failed << " failed\n";
        if (log.failed > 0) {
            std::exit(1);
        }
    }
}

int main() {
    tests::run_all();
    return 0;
}
