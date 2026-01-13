#include <iostream>

#include "App/Equation.hpp"

#include "Vendor/tinyexpr.h"

void TrimInPlace(std::string& s) {
    while (!s.empty() && std::isspace(s.front())) {
        s.erase(s.begin());
    }

    while (!s.empty() && std::isspace(s.back())) {
        s.pop_back();
    }
}

bool UsesStandaloneSymbol(const std::string& s, char symbol) {
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] != symbol) continue;

        bool leftOk = (i == 0) || !std::isalnum(s[i - 1]);
        bool rightOk = (i + 1 == s.size()) || !std::isalnum(s[i + 1]);

        if (leftOk && rightOk) {
            return true;
        }
    }
    return false;
}

std::string ReplaceSymbol(const std::string& expr, char from, char to) {
    std::string out;
    out.reserve(expr.size());

    for (std::size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];

        if (c == from) {
            bool leftOk = (i == 0) || !std::isalnum(expr[i - 1]);
            bool rightOk = (i + 1 == expr.size()) || !std::isalnum(expr[i + 1]);

            if (leftOk && rightOk) {
                out += to;
                continue;
            }
        }

        out += c;
    }

    return out;
}

void ReplaceAll(std::string& string, const std::string& a, const std::string& b) {
    std::size_t pos = 0;
    while ((pos = string.find(a, pos)) != std::string::npos) {
        string.replace(pos, a.size(), b);
    }
}

System::Error::ResultWrapper<std::pair<double, double>> parseDomain(const std::string& src) {
    std::string t;
    for (char c : src) {
        if (!std::isspace((unsigned char)c)) {
            t += c;
        }
    }

    ReplaceAll(t, "<=", "<");
    ReplaceAll(t, ">=", ">");

    auto p1 = t.find('<');
    auto p2 = t.rfind('<');

    if (p1 == std::string::npos || p1 == p2) {
        return System::Error::failure<std::pair<double, double>>("Invalid domain");
    }

    std::string mid = t.substr(p1 + 1, p2 - p1 - 1);

    if (mid != "x" && mid != "t") {
        return System::Error::failure<std::pair<double, double>>("Unknown domain identifier");
    }

    std::string left = t.substr(0, p1);
    std::string right = t.substr(p2 + 1);

    int err = 0;

    double domainLeft = te_interp(left.c_str(), &err);
    if (err) {
        return System::Error::failure<std::pair<double, double>>("Invalid domain");
    }

    double domainRight = te_interp(right.c_str(), &err);
    if (err) {
        return System::Error::failure<std::pair<double, double>>("Invalid domain");
    }

    if (domainRight < domainLeft) {
        return System::Error::success<std::pair<double, double>>({domainRight, domainLeft});
    } else {
        return System::Error::success<std::pair<double, double>>({domainLeft, domainRight});
    }
}

bool IsFullyWrapped(const std::string& s) {
    if (s.size() < 2 || s.front() != '(' || s.back() != ')')
        return false;

    int depth = 0;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '(') depth++;
        else if (s[i] == ')') depth--;

        if (depth == 0 && i != s.size() - 1) {
            return false;
        }
    }
    return depth == 0;
}

inline void StripOuterParentheses(std::string& s) {
    TrimInPlace(s);

    while (IsFullyWrapped(s)) {
        s.erase(s.begin());
        s.pop_back();
    }
}

std::pair<std::string, std::pair<double, double>> SplitExpressionsAndDomain(const std::string& raw, int& error) {
    auto l = raw.find('{');
    auto r = raw.find('}');

    if (l == std::string::npos || r == std::string::npos || r < l) {
        return {raw, {-1.0, 1.0}}; // default domain
    }

    std::string expression = raw.substr(0, l);
    std::string domainText = raw.substr(l + 1, r - l - 1);
    auto domainResult = parseDomain(domainText);

    if (domainResult) {
        error = 0;
        return {expression, domainResult.value()};
    }

    error = 1;
    return {};
}

System::Error::ResultWrapper<Equation> Equation::Parse(const std::string& raw) {
    Equation eq;

    if (raw.empty()) {
        return System::Error::failure<Equation>("Input string is empty");
    }

    /* ---------- 1. Extract domain ---------- */

    int error = 0;
    auto [expr, domain] = SplitExpressionsAndDomain(raw, error);

    if (error) {
        return System::Error::failure<Equation>("Failed to parse domain");
    }

    eq.DomainLeft = domain.first;
    eq.DomainRight = domain.second;

    /* ---------- 2. Parametric detection ---------- */

    if (expr.find(',') != std::string::npos) {
        StripOuterParentheses(expr);

        const std::size_t commaPos = expr.find(',');

        std::string lhs = expr.substr(0, commaPos);
        std::string rhs = expr.substr(commaPos + 1);

        TrimInPlace(lhs);

        if (lhs.empty()) {
            return System::Error::failure<Equation>("f(t) is empty for parametric equation");
        }

        TrimInPlace(rhs);

        if (rhs.empty()) {
            return System::Error::failure<Equation>("g(t) is empty for parametric equation");
        }

        eq.Type = EquationType::Parametric;
        eq.Expression_1 = lhs;
        eq.Expression_2 = rhs;

        return System::Error::success(eq);
    }

    /* ---------- 3. Explicit detection ---------- */

    const bool usesY = UsesStandaloneSymbol(expr, 'y');

    if (usesY) {
        // x = f(y) → normalize to x = f(x)
        eq.Type = EquationType::Explicit_X;
        eq.Expression_1 = ReplaceSymbol(expr, 'y', 'x');
    } else {
        // y = f(x)
        eq.Type = EquationType::Explicit_Y;
        eq.Expression_1 = expr;
    }

    return System::Error::success(eq);
}