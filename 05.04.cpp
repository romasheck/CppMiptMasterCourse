#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out"
rm "${SCRIPT_NAME}.out"
exit
#endif

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace game {

constexpr int kSpearmanAttack = 6;
constexpr int kSpearmanDefense = 4;
constexpr int kSpearmanMorale = 3;

constexpr int kRangerAttack = 5;
constexpr int kRangerDefense = 3;
constexpr int kRangerMorale = 4;

constexpr int kKnightAttack = 8;
constexpr int kKnightDefense = 6;
constexpr int kKnightMorale = 5;

constexpr int kOutpostDefense = 9;
constexpr int kOutpostGarrison = 2;

class Unit;

class UnitState {
public:
    virtual ~UnitState() = default;
    virtual const char* name() const = 0;
    virtual void step(Unit& u) const = 0;
};

class StandbyState final : public UnitState {
public:
    const char* name() const override { return "Standby"; }
    void step(Unit& u) const override;
};

class MarchState final : public UnitState {
public:
    const char* name() const override { return "March"; }
    void step(Unit& u) const override;
};

class SkirmishState final : public UnitState {
public:
    const char* name() const override { return "Skirmish"; }
    void step(Unit& u) const override;
};

class GameObject {
public:
    virtual ~GameObject() = default;
    virtual int strength() const = 0;
    virtual void step() = 0;
    virtual void describe() const = 0;
};

class Unit final : public GameObject {
public:
    Unit(std::string name, int attack, int defense, int morale)
        : m_name(std::move(name)), m_attack(attack), m_defense(defense), m_morale(morale), m_state(&Standby()) {
    }

    const std::string& name() const noexcept { return m_name; }

    int attack() const noexcept { return m_attack; }
    int defense() const noexcept { return m_defense; }
    int morale() const noexcept { return m_morale; }

    void boost_morale(int delta) noexcept { m_morale += delta; }

    void set_state(const UnitState& s) noexcept { m_state = &s; }
    const UnitState& state() const noexcept { return *m_state; }

    int strength() const override {
        return m_attack + m_defense + m_morale;
    }

    void step() override {
        m_state->step(*this);
    }

    void describe() const override {
        std::cout << "Unit{name=" << m_name << ", atk=" << m_attack << ", def=" << m_defense
                  << ", mor=" << m_morale << ", state=" << m_state->name() << "}\n";
    }

    static const UnitState& Standby() {
        static const StandbyState s{};
        return s;
    }

    static const UnitState& March() {
        static const MarchState s{};
        return s;
    }

    static const UnitState& Skirmish() {
        static const SkirmishState s{};
        return s;
    }

private:
    std::string m_name;
    int m_attack = 0;
    int m_defense = 0;
    int m_morale = 0;
    const UnitState* m_state = nullptr;
};

class Outpost final : public GameObject {
public:
    Outpost(std::string name, int defense, int garrison)
        : m_name(std::move(name)), m_defense(defense), m_garrison(garrison) {
    }

    int strength() const override {
        return m_defense + m_garrison;
    }

    void step() override {
        std::cout << m_name << ": holding position\n";
    }

    void describe() const override {
        std::cout << "Outpost{name=" << m_name << ", def=" << m_defense << ", gar=" << m_garrison
                  << "}\n";
    }

private:
    std::string m_name;
    int m_defense = 0;
    int m_garrison = 0;
};

inline void StandbyState::step(Unit& u) const {
    std::cout << u.name() << ": standby -> march\n";
    u.set_state(Unit::March());
}

inline void MarchState::step(Unit& u) const {
    std::cout << u.name() << ": march -> skirmish\n";
    u.set_state(Unit::Skirmish());
}

inline void SkirmishState::step(Unit& u) const {
    std::cout << u.name() << ": skirmish -> standby\n";
    u.boost_morale(-1);
    u.set_state(Unit::Standby());
}

class Formation final : public GameObject {
public:
    explicit Formation(std::string name) : m_name(std::move(name)) {
    }

    void add(std::unique_ptr<GameObject> obj) {
        m_children.push_back(std::move(obj));
    }

    int strength() const override {
        int total = 0;
        for (const auto& c : m_children) {
            total += c->strength();
        }
        return total;
    }

    void step() override {
        std::cout << "Formation{" << m_name << "} step\n";
        for (auto& c : m_children) {
            c->step();
        }
    }

    void describe() const override {
        std::cout << "Formation{" << m_name << "} size=" << m_children.size() << " strength=" << strength()
                  << "\n";
        for (const auto& c : m_children) {
            c->describe();
        }
    }

private:
    std::string m_name;
    std::vector<std::unique_ptr<GameObject>> m_children;
};

enum class UnitKind { Spearman, Ranger, Knight };
enum class StructureKind { Outpost };

class GameFactory {
public:
    std::unique_ptr<GameObject> make_unit(UnitKind kind, const std::string& name) const {
        switch (kind) {
            case UnitKind::Spearman:
                return std::make_unique<Unit>(name, kSpearmanAttack, kSpearmanDefense, kSpearmanMorale);
            case UnitKind::Ranger:
                return std::make_unique<Unit>(name, kRangerAttack, kRangerDefense, kRangerMorale);
            case UnitKind::Knight:
                return std::make_unique<Unit>(name, kKnightAttack, kKnightDefense, kKnightMorale);
        }
        return std::make_unique<Unit>(name, 0, 0, 0);
    }

    std::unique_ptr<GameObject> make_structure(StructureKind kind, const std::string& name) const {
        switch (kind) {
            case StructureKind::Outpost:
                return std::make_unique<Outpost>(name, kOutpostDefense, kOutpostGarrison);
        }
        return std::make_unique<Outpost>(name, 0, 0);
    }
};

class Campaign {
public:
    explicit Campaign(std::string name) : m_name(std::move(name)) {
    }

    void set_root(std::unique_ptr<GameObject> root) {
        m_root = std::move(root);
    }

    int total_strength() const {
        return m_root ? m_root->strength() : 0;
    }

    void turn() {
        std::cout << "Campaign{" << m_name << "} turn\n";
        if (m_root) {
            m_root->step();
        }
    }

    void report() const {
        std::cout << "Campaign{" << m_name << "} report\n";
        if (m_root) {
            m_root->describe();
        }
    }

private:
    std::string m_name;
    std::unique_ptr<GameObject> m_root{};
};

}  // namespace game

namespace tests {

struct Logger {
    std::size_t passed = 0;

    void pass(const std::string& name) {
        ++passed;
        std::cout << "PASS: " << name << '\n';
    }
};

void require(bool ok) {
    assert(ok);
}

template <typename F>
std::string capture_stdout(F&& fn) {
    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    std::forward<F>(fn)();
    std::cout.rdbuf(old);
    return oss.str();
}

void test_factory_strength_math(Logger& log) {
    game::GameFactory f;

    auto spearman = f.make_unit(game::UnitKind::Spearman, "S1");
    auto outpost = f.make_structure(game::StructureKind::Outpost, "O1");

    const int expected_s = game::kSpearmanAttack + game::kSpearmanDefense + game::kSpearmanMorale;
    const int expected_o = game::kOutpostDefense + game::kOutpostGarrison;

    require(spearman->strength() == expected_s);
    require(outpost->strength() == expected_o);

    log.pass("factory_creates_objects_with_expected_strength");
}

void test_composite_sums_mixed_objects(Logger& log) {
    game::GameFactory f;

    auto formation = std::make_unique<game::Formation>("Vanguard");
    formation->add(f.make_unit(game::UnitKind::Knight, "K1"));
    formation->add(f.make_unit(game::UnitKind::Ranger, "R1"));
    formation->add(f.make_structure(game::StructureKind::Outpost, "Gate"));

    const int expected =
        (game::kKnightAttack + game::kKnightDefense + game::kKnightMorale) +
        (game::kRangerAttack + game::kRangerDefense + game::kRangerMorale) +
        (game::kOutpostDefense + game::kOutpostGarrison);

    require(formation->strength() == expected);

    log.pass("composite_strength_is_sum_of_children");
}

void test_state_cycle_and_effect(Logger& log) {
    game::Unit u("Vanguard", 1, 1, 3);

    const std::string out = capture_stdout([&] {
        u.step();
        u.step();
        u.step();
        u.describe();
    });

    require(out ==
            "Vanguard: standby -> march\n"
            "Vanguard: march -> skirmish\n"
            "Vanguard: skirmish -> standby\n"
            "Unit{name=Vanguard, atk=1, def=1, mor=2, state=Standby}\n");

    log.pass("state_cycle_changes_state_and_modifies_morale");
}

void test_campaign_scenario(Logger& log) {
    game::GameFactory f;

    auto root = std::make_unique<game::Formation>("Army");
    root->add(f.make_structure(game::StructureKind::Outpost, "ForwardBase"));

    auto squad = std::make_unique<game::Formation>("Squad");
    squad->add(f.make_unit(game::UnitKind::Spearman, "Spear-1"));
    squad->add(f.make_unit(game::UnitKind::Spearman, "Spear-2"));
    squad->add(f.make_unit(game::UnitKind::Ranger, "Ranger-1"));

    root->add(std::move(squad));

    game::Campaign c("Border");
    c.set_root(std::move(root));

    require(c.total_strength() > 0);

    const std::string out = capture_stdout([&] {
        c.report();
        c.turn();
    });

    require(out.find("Campaign{Border} report\n") != std::string::npos);
    require(out.find("Formation{Army}") != std::string::npos);
    require(out.find("Campaign{Border} turn\n") != std::string::npos);
    require(out.find("Formation{Army} step\n") != std::string::npos);

    log.pass("campaign_report_and_turn_produce_output");
}

void run_all() {
    Logger log;
    test_factory_strength_math(log);
    test_composite_sums_mixed_objects(log);
    test_state_cycle_and_effect(log);
    test_campaign_scenario(log);
    std::cout << "ALL TESTS PASSED (" << log.passed << ")\n";
}

}  // namespace tests

int main() {
    tests::run_all();
    return 0;
}
