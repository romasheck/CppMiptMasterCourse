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

// Problem: 07.03 

#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <utility>
#include <vector>

enum class Status {
    success,
    failure
};

struct PersonConfig {
    std::string name = "Alice";
    int grade = 0;
    int salary = 0;
    int id = 0;

    bool throw_on_name = false;
    bool throw_on_grade = false;
    bool throw_on_salary = false;
    bool throw_on_id = false;
};

struct SaveConfig {
    bool throw_on_save = false;
};

struct CallLog {
    bool name_called = false;
    bool grade_called = false;
    bool salary_called = false;
    bool id_called = false;
    bool save_called = false;
    Status saved_status = Status::failure;
    int saved_id = -1;
};

class Person {
public:
    Person(PersonConfig config, CallLog& log) : config_(std::move(config)), log_(log) {
    }

    const std::string& name() const {
        log_.name_called = true;
        if (config_.throw_on_name) {
            throw std::runtime_error("name() failed");
        }
        return config_.name;
    }

    int grade() const {
        log_.grade_called = true;
        if (config_.throw_on_grade) {
            throw std::runtime_error("grade() failed");
        }
        return config_.grade;
    }

    int salary() const {
        log_.salary_called = true;
        if (config_.throw_on_salary) {
            throw std::runtime_error("salary() failed");
        }
        return config_.salary;
    }

    int id() const {
        log_.id_called = true;
        if (config_.throw_on_id) {
            throw std::runtime_error("id() failed");
        }
        return config_.id;
    }

private:
    PersonConfig config_;
    CallLog& log_;
};

void save(Status status, int id, const SaveConfig& config, CallLog& log) {
    log.save_called = true;
    log.saved_status = status;
    log.saved_id = id;

    if (config.throw_on_save) {
        throw std::runtime_error("save() failed");
    }
}

void test(const Person& person, std::ostream& out, const SaveConfig& save_config, CallLog& log) {
    out << "test: " << person.name() << '\n';

    if (person.grade() == 10 || person.salary() > 1000000) {
        save(Status::success, person.id(), save_config, log);
    } else {
        save(Status::failure, person.id(), save_config, log);
    }
}

void test(const Person& person, const SaveConfig& save_config, CallLog& log) {
    test(person, std::cout, save_config, log);
}

struct AnalysisPoint {
    const char* expression;
    const char* normal_flow;
    const char* exception_source;
};

const std::vector<AnalysisPoint>& analysis_points() {
    static const std::vector<AnalysisPoint> points{
        {"out << \"test: \"", "writes prefix to stream", "ostream insertion may fail and throw"},
        {"person.name()", "provides name for output", "user code may throw"},
        {"out << '\\n'", "completes the log line", "ostream insertion may fail and throw"},
        {"person.grade() == 10", "may send control to success branch", "grade() may throw"},
        {"||", "short-circuits when grade() == 10", "right operand is skipped or evaluated"},
        {"person.salary() > 1000000", "may send control to success branch", "salary() may throw"},
        {"if (...) / else", "chooses success or failure branch", "condition evaluation may throw"},
        {"person.id()", "provides identifier for save()", "id() may throw"},
        {"save(Status::success, ...)", "stores success result", "save() may throw"},
        {"save(Status::failure, ...)", "stores failure result", "save() may throw"},
    };
    return points;
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

class ThrowingBuffer : public std::streambuf {
protected:
    int overflow(int) override {
        throw std::runtime_error("ostream write failed");
    }
};

void run_all() {
    Logger log;

    {
        CallLog calls;
        Person person(PersonConfig{.name = "Alice", .grade = 10, .salary = 10, .id = 7}, calls);
        std::ostringstream out;
        test(person, out, SaveConfig{}, calls);

        log.require(calls.name_called, "name() is used for output");
        log.require(calls.grade_called, "grade() participates in condition");
        log.require(!calls.salary_called, "salary() is skipped by short-circuit when grade == 10");
        log.require(calls.id_called, "id() is used before save()");
        log.require(calls.save_called, "save() is called on success branch");
        log.require(calls.saved_status == Status::success, "success branch stores success status");
    }

    {
        CallLog calls;
        Person person(PersonConfig{.name = "Bob", .grade = 5, .salary = 2000000, .id = 9}, calls);
        std::ostringstream out;
        test(person, out, SaveConfig{}, calls);

        log.require(calls.grade_called && calls.salary_called,
                    "salary() is evaluated when grade != 10");
        log.require(calls.saved_status == Status::success,
                    "high salary reaches success branch");
    }

    {
        CallLog calls;
        Person person(PersonConfig{.name = "Carl", .grade = 5, .salary = 100, .id = 11}, calls);
        std::ostringstream out;
        test(person, out, SaveConfig{}, calls);

        log.require(calls.saved_status == Status::failure,
                    "false condition reaches failure branch");
    }

    {
        CallLog calls;
        Person person(PersonConfig{.throw_on_name = true}, calls);
        std::ostringstream out;
        bool caught = false;

        try {
            test(person, out, SaveConfig{}, calls);
        } catch (const std::runtime_error& error) {
            caught = std::string(error.what()) == "name() failed";
        }

        log.require(caught, "name() exception propagates");
        log.require(!calls.grade_called && !calls.save_called,
                    "execution stops before condition and save when name() throws");
    }

    {
        CallLog calls;
        Person person(PersonConfig{.throw_on_grade = true}, calls);
        std::ostringstream out;
        bool caught = false;

        try {
            test(person, out, SaveConfig{}, calls);
        } catch (const std::runtime_error& error) {
            caught = std::string(error.what()) == "grade() failed";
        }

        log.require(caught, "grade() exception propagates");
        log.require(!calls.salary_called && !calls.save_called,
                    "grade() throw prevents later evaluation");
    }

    {
        CallLog calls;
        Person person(PersonConfig{.grade = 5, .throw_on_salary = true}, calls);
        std::ostringstream out;
        bool caught = false;

        try {
            test(person, out, SaveConfig{}, calls);
        } catch (const std::runtime_error& error) {
            caught = std::string(error.what()) == "salary() failed";
        }

        log.require(caught, "salary() exception propagates");
        log.require(!calls.save_called, "save() is not reached when salary() throws");
    }

    {
        CallLog calls;
        Person person(PersonConfig{.grade = 10, .throw_on_id = true}, calls);
        std::ostringstream out;
        bool caught = false;

        try {
            test(person, out, SaveConfig{}, calls);
        } catch (const std::runtime_error& error) {
            caught = std::string(error.what()) == "id() failed";
        }

        log.require(caught, "id() exception propagates");
        log.require(!calls.save_called, "save() is not entered when id() throws");
    }

    {
        CallLog calls;
        Person person(PersonConfig{.grade = 10, .id = 17}, calls);
        std::ostringstream out;
        bool caught = false;

        try {
            test(person, out, SaveConfig{.throw_on_save = true}, calls);
        } catch (const std::runtime_error& error) {
            caught = std::string(error.what()) == "save() failed";
        }

        log.require(caught, "save() exception propagates");
        log.require(calls.save_called, "save() was entered before throwing");
    }

    {
        CallLog calls;
        Person person(PersonConfig{.name = "Dana", .grade = 10, .id = 5}, calls);
        ThrowingBuffer buffer;
        std::ostream out(&buffer);
        out.exceptions(std::ios::badbit | std::ios::failbit);
        bool caught = false;

        try {
            test(person, out, SaveConfig{}, calls);
        } catch (const std::runtime_error& error) {
            caught = std::string(error.what()) == "ostream write failed";
        }

        log.require(caught, "stream output may throw");
        log.require(!calls.grade_called && !calls.save_called,
                    "output failure stops execution before condition");
    }

    {
        const auto& points = analysis_points();
        log.require(points.size() == 10, "analysis checklist contains all identified points");
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
