#include <iostream>
#include <cmath>

#include "App/Graph.hpp"

#include "Vendor/tinyexpr.h"

#include "System/Error.hpp"

std::vector<sf::Vector2f> Graph::genratePoints(sampler_t sampler, double domainLeft, double domainRight) {
    constexpr double IncrementSteps = 0.005;

    std::vector<sf::Vector2f> points;
    points.reserve(1u + static_cast<size_t>((domainRight - domainLeft) / IncrementSteps));

    for (double t = domainLeft; t <= domainRight; t += IncrementSteps) {
        points.emplace_back(sampler(t));
    }

    return points;
}

System::Error::ResultWrapper<Graph::sampler_t> makeExplicitSampler(const std::string& equation, Axis axis) {
    auto value = std::make_shared<double>();

    te_variable vars[] = {
        {"x", value.get(), TE_VARIABLE}
    };

    int error = 0;
    te_expr* raw = te_compile(equation.c_str(), vars, 1, &error);

    if (!raw) {
        te_free(raw);
        return System::Error::failure<Graph::sampler_t>("Could't parse the expression, error at position: " + std::to_string(error));
    }

    auto expr = std::shared_ptr<te_expr>(raw, te_free);

    return System::Error::success<Graph::sampler_t>(
        [expr, value, axis](double t) -> sf::Vector2f {
            *value = t;
            double r = te_eval(expr.get());

            return axis == Axis::Y
                ? sf::Vector2f((float)t, (float)-r)
                : sf::Vector2f((float)r, (float)t);
            }
    );
}

System::Error::ResultWrapper<Graph::sampler_t> makeParametricSampler(const std::string& eqX, const std::string& eqY) {
    auto value = std::make_shared<double>();

    te_variable vars[] = {
        {"t", value.get(), TE_VARIABLE}
    };

    int error = 0;
    te_expr* rawX = te_compile(eqX.c_str(), vars, 1, &error);

    if (!rawX) {
        return System::Error::failure<Graph::sampler_t>("Could't parse the expression g(t), error at position: " + std::to_string(error));
    }

    te_expr* rawY = te_compile(eqY.c_str(), vars, 1, &error);

    if (!rawY) {
        return System::Error::failure<Graph::sampler_t>("Could't parse the expression f(t), error at position: " + std::to_string(error));
    }

    auto ex = std::shared_ptr<te_expr>(rawX, te_free);
    auto ey = std::shared_ptr<te_expr>(rawY, te_free);

    return System::Error::success<Graph::sampler_t>(
        [ex, ey, value](double t) -> sf::Vector2f {
            *value = t;
            return {
                (float)te_eval(ex.get()),
                (float)-te_eval(ey.get())
            };
            }
    );
}

std::optional<std::string> Graph::Generate(const Equation& equation) {
    if (equation.Type == EquationType::Parametric) {
        auto result = makeParametricSampler(equation.Expression_1, equation.Expression_2);

        if (result) {
            m_Points = genratePoints(result.value(), equation.DomainLeft, equation.DomainRight);
            return std::nullopt;
        } else {
            return result.error();
        }
    } else {
        auto result = makeExplicitSampler(equation.Expression_1, equation.Type == EquationType::Explicit_X ? Axis::X : Axis::Y);

        if (result) {
            m_Points = genratePoints(result.value(), equation.DomainLeft, equation.DomainRight);
            return std::nullopt;
        } else {
            return result.error();
        }
    }
}

void Graph::SetExplicitCallback(func_explicit_t function, double domainLeft, double domainRight, Axis axis) {
    m_Points = genratePoints(
        [function, axis](double t) -> sf::Vector2f {
            const double r = function(t);
            return axis == Axis::Y ? sf::Vector2f(static_cast<float>(t), static_cast<float>(-r)) : sf::Vector2f(static_cast<float>(r), static_cast<float>(t));
        },
        domainLeft, domainRight
    );
}

void Graph::SetParametricCallback(func_parametric_t function, double domainLeft, double domainRight) {
    m_Points = genratePoints(
        [function](double t) -> sf::Vector2f {
            sf::Vector2f p = function(t);
            return sf::Vector2f(p.x, -p.y);
        },
        domainLeft, domainRight
    );
}

void Graph::Update(float deltaTime) {
    constexpr float AnimationDuration = 1.f;

    if (m_Progress < 1.f) {
        const float t = deltaTime / AnimationDuration;
        m_Progress = std::min<float>(1.f, m_Progress + t);
    }
}

void Graph::Render(sf::RenderTarget& target, sf::Color color, sf::Vector2f offset, float zoom) {
    const unsigned int numLines = static_cast<unsigned int>((m_Points.size() / 2u) * m_Progress);

    if (!numLines) {
        return;
    }

    const unsigned int numPoints = numLines * 2u;

    const sf::Vector2u targetSize = target.getSize();
    const sf::Vector2f center = sf::Vector2f(targetSize) * 0.5f + offset;

    constexpr float Thickness = 4.f;

    sf::VertexArray vertices(sf::PrimitiveType::Triangles, (numPoints - 1) * 6u);
    unsigned int currentIndex = 0u;

    for (unsigned int i = 0u; i + 1u < numPoints; ++i) {
        const sf::Vector2f p0 = m_Points[i] * zoom + center;
        const sf::Vector2f p1 = m_Points[i + 1u] * zoom + center;

        const sf::Vector2f p01 = p1 - p0;
        const float lenSquare = p01.x * p01.x + p01.y * p01.y;

        if (lenSquare < 1e-6f) [[unlikely]] {
            continue;
        }

        const sf::Vector2f dir = p01 / std::sqrt(lenSquare);

        const sf::Vector2f normalOffset = sf::Vector2f(-dir.y, dir.x) * (Thickness * 0.5f);

        vertices[currentIndex++] = sf::Vertex(p0 + normalOffset, color);
        vertices[currentIndex++] = sf::Vertex(p1 + normalOffset, color);
        vertices[currentIndex++] = sf::Vertex(p1 - normalOffset, color);

        vertices[currentIndex++] = sf::Vertex(p0 + normalOffset, color);
        vertices[currentIndex++] = sf::Vertex(p1 - normalOffset, color);
        vertices[currentIndex++] = sf::Vertex(p0 - normalOffset, color);
    }

    target.draw(vertices);
}