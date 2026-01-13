#pragma once

#include <string>
#include <cstdint>

#include "System/Error.hpp"

enum class EquationType : uint8_t {
    Explicit_X, // x = f(y)
    Explicit_Y, // y = f(x)
    Parametric, // y = f(t), x = g(t)
};

enum class Axis : bool {
    X,
    Y
};

struct Equation {
    static System::Error::ResultWrapper<Equation> Parse(const std::string& raw);

    std::string Expression_1;
    std::string Expression_2;

    double DomainLeft = -1.0;
    double DomainRight = 1.0;

    EquationType Type = EquationType::Explicit_Y;
};