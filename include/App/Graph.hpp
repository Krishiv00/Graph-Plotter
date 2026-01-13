#pragma once

#include <functional>

#include "SFML/Graphics.hpp"

#include "App/Equation.hpp"

class Graph {
public:
    typedef std::function<double(double)> func_explicit_t;
    typedef std::function<sf::Vector2f(double)> func_parametric_t;
    typedef std::function<sf::Vector2f(double)> sampler_t;

private:
    static std::vector<sf::Vector2f> genratePoints(sampler_t sampler, double domainLeft, double domainRight);

    std::vector<sf::Vector2f> m_Points;

    float m_Progress{0.f};

public:
    Graph() = default;
    Graph(bool animate) : m_Progress(static_cast<float>(!animate)) {}

    std::optional<std::string> Generate(const Equation& equation);

    void SetExplicitCallback(func_explicit_t function, double domainLeft = -1.0, double domainRight = 1.0, Axis axis = Axis::Y);
    void SetParametricCallback(func_parametric_t function, double domainLeft = -1.0, double domainRight = 1.0);

    void Update(float deltaTime);
    void Render(sf::RenderTarget& target, sf::Color color, sf::Vector2f offset, float zoom);

    [[nodiscard]] inline const std::vector<sf::Vector2f>& GetPoints() const {
        return m_Points;
    }
};