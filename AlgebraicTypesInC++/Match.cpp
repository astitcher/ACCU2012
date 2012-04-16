#include <functional>
#include <tuple>
#include <vector>

using std::function;
using std::tuple;
using std::make_tuple;
using std::tie;
using std::get;
using std::vector;

class Env {};

class Pattern;

class Matcher {
public:
    tuple<bool,Env> operator()(const Pattern& pattern);
};

template <typename R>
class MatcherAction {
    const Matcher matcher;
    const function<R (Env)> action;
public:
    MatcherAction(const Matcher& m, const function<R (Env)>& a) :
        matcher(m),
        action(a)
    {}

    tuple<bool,R> tryMatch(const Pattern& pattern) const {
        bool match;
        Env env;
        tie(match, env) = matcher(pattern);
        if (match)
            return make_tuple(match, action(env));
    }
};

// Best syntax that seems to work
template <typename R>
MatcherAction<R> operator>>(const Matcher& m, const function<R (Env)>& f) {
    return MatcherAction<R>(m, f);
}

template <typename R>
class Match {
    vector<MatcherAction<R>> matches;
public:
    Match(std::initializer_list<MatcherAction<R>> m) :
        matches(m)
    {}
    
    R match(const Pattern& pattern) const {
        for (const auto t : matches) {
            bool m;
            R r;
            tie(m, r) = t.tryMatch(pattern);
            if (m) return r;
        }
    }
};

// Test
auto m = Match<int> {
    operator>><int>(Matcher(), [](Env e) {return 1;}),
    operator>><int>(Matcher(), [](Env e) {return 2;}),
    operator>><int>(Matcher(), [](Env e) {return 3;})
};

