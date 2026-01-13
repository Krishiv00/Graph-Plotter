#include <iostream>
#include <cmath>

#include "System/Color.hpp"

#include "App/Application.hpp"

#pragma region Constants

namespace Settings {
    constexpr float Damping = 10.f;
    constexpr float MinZoom = 10.f;
    constexpr float ZoomImpulse = 2.75f;
    constexpr float MoveImpulse = 2.f;
    constexpr float Ellipson = 0.0001f;
    constexpr float DefaultGizmoScale = 200.f;
}

namespace Theme {
    constexpr sf::Color BackgroundColor = sf::Color(34u, 34u, 40u);
    constexpr sf::Color GizmoBaseColor(180u, 180u, 200u);
    constexpr uint8_t GizmoColorFalloff = 3u;
}

#pragma region Resources

Application::Application() {
    m_GizmoScale = Settings::DefaultGizmoScale;

    constexpr double PI = 3.1415926535;

    // m_Graphs.emplace_back().Generate(Equation::Parse("(0.05*16*sin(t)*sin(t)*sin(t), 0.05*(13*cos(t)-5*cos(2*t)-2*cos(3*t)-cos(4*t))) { -pi <= t <= pi }").value());
    // m_Graphs.emplace_back().Generate(Equation::Parse("(0.1*cos(2*t)*cos(t), 0.1*cos(2*t)*sin(t)) { 0 <= t <= 2*pi }").value());
    // m_Graphs.emplace_back().Generate(Equation::Parse("(sin(2*t), cos(3*t)) { -5 <= t <= 5 }").value());
    // m_Graphs.emplace_back().Generate(Equation::Parse("x*sin(1/x) { -pi <= x <= pi }").value());
    // m_Graphs.emplace_back().Generate(Equation::Parse("x - 1/x^2 { -pi <= x <= pi }").value());
    // m_Graphs.emplace_back().Generate(Equation::Parse("0.25*(2-x)/(1+sin(x)) { -10 <= x <= 10 }").value());
    // m_Graphs.emplace_back().Generate(Equation::Parse("(x+2)/(x^2+3*x+2) { -10 <= x <= 10 }").value());
    // m_Graphs.emplace_back().Generate(Equation::Parse("15*((sin(x)^3*15)-15)/((sin(x)^2*15)^-1-15) { -100 <= x <= 100 }").value());
}

bool Application::LoadResources(const char* root) {
    if (!m_Font.openFromFile(root + std::string("Fonts/JetBrainsMono-SemiBold.ttf"))) {
        return false;
    }

    return true;
}

#pragma region Events

void Application::HandleKeyPress(sf::Keyboard::Scancode key) {
    if (key == sf::Keyboard::Scancode::Insert) {
        m_GettingUserInput ^= true;
        m_ShowPreview = false;
    }

    else if (
        key == sf::Keyboard::Scancode::P &&
        sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)
    ) {
        m_ShowPreview ^= true;
    }

    else if (m_GettingUserInput) {
        if (key == sf::Keyboard::Scancode::Enter) {
            m_GettingUserInput = false;
            m_RestoringDefaultView = true;
            m_Grabbed = false;

            auto equation = Equation::Parse(m_Textbox.Consume());
            if (equation) {
                if (auto error = m_Graphs.emplace_back(!m_ShowPreview).Generate(equation.value())) {
                    invokeError(error.value());
                }
            } else {
                invokeError(equation.error());
            }
        } else {
            m_Textbox.HandleKeyPress(key);
        }
    }

    else if (key == sf::Keyboard::Scancode::C) {
        m_RestoringDefaultView = true;
        m_Grabbed = false;
    }

    else if (key == sf::Keyboard::Scancode::R) {
        m_Graphs.clear();
    }
}

void Application::HandleCharTyped(char c) {
    if (m_GettingUserInput) {
        m_Textbox.Type(c);
    }
}

void Application::HandleMouseButtonPress(sf::Mouse::Button button) {
    if (button == sf::Mouse::Button::Left) {
        m_Grabbed = true;
        m_RestoringDefaultView = false;
        m_LastMousePosition = State.MousePosition;
    }
}

void Application::HandleMouseButtonRelease(sf::Mouse::Button button) {
    if (button == sf::Mouse::Button::Left) {
        m_Grabbed = false;
    }
}

void Application::HandleMouseWheelScroll(float delta) {
    m_ZoomMomentum = delta * Settings::ZoomImpulse * m_GizmoScale;
    m_RestoringDefaultView = false;
}

void Application::invokeError(const std::string& errorMessage) {
    std::cerr << "ERROR: " << errorMessage << std::endl;
}

#pragma region Update

void Application::Update(float deltaTime) {
    updatePanning();
    updateZoom(deltaTime);
    updateViewport(deltaTime);

    for (Graph& graph : m_Graphs) {
        graph.Update(deltaTime);
    }
}

void Application::updatePanning() {
    if (m_Grabbed) {
        const sf::Vector2i delta = State.MousePosition - m_LastMousePosition;

        m_LastMousePosition = State.MousePosition;
        m_Position += sf::Vector2f(delta) * Settings::MoveImpulse;
    }
}

void Application::updateZoom(float deltaTime) {
    if (m_ZoomMomentum) {
        const float oldScale = m_GizmoScale;

        m_GizmoScale = std::max<float>(m_GizmoScale + m_ZoomMomentum * deltaTime, Settings::MinZoom);

        if (oldScale) {
            m_Position *= m_GizmoScale / oldScale;
        }

        m_ZoomMomentum *= std::exp(-Settings::Damping * deltaTime);

        if (std::fabs(m_ZoomMomentum) < Settings::Ellipson) {
            m_ZoomMomentum = 0.f;
        }
    }
}

void Application::updateViewport(float deltaTime) {
    if (m_RestoringDefaultView) {
        const float factor = -Settings::Damping * std::min(4.f, m_GizmoScale / 100.f) * deltaTime;

        m_Position *= std::exp(factor);
        m_GizmoScale = Settings::DefaultGizmoScale + (m_GizmoScale - Settings::DefaultGizmoScale) * std::exp(factor * 0.5f);

        if (std::abs(m_Position.x) < 1.f && std::abs(m_Position.y) < 1.f && std::abs(Settings::DefaultGizmoScale - m_GizmoScale) < 1.f) {
            m_Position = sf::Vector2f(0.f, 0.f);
            m_RestoringDefaultView = false;
        }
    }
}

#pragma region Rendering

void Application::renderGizmo(sf::RenderTarget& target) {
    const float secondaryZoomFactor = std::clamp((m_GizmoScale - Settings::MinZoom) / (Settings::DefaultGizmoScale - Settings::MinZoom), 0.2f, 1.f);
    const float tertiaryZoomFactor = std::clamp((m_GizmoScale - Settings::DefaultGizmoScale) / Settings::DefaultGizmoScale, 0.f, 1.f);

    constexpr sf::Color PrimaryColor = sf::Color(Theme::GizmoBaseColor.r, Theme::GizmoBaseColor.g, Theme::GizmoBaseColor.b, 255u / Theme::GizmoColorFalloff);

    const uint8_t secondaryAlpha = static_cast<uint8_t>(255u / Theme::GizmoColorFalloff * secondaryZoomFactor * secondaryZoomFactor);
    const sf::Color SecondaryColor = sf::Color(Theme::GizmoBaseColor.r, Theme::GizmoBaseColor.g, Theme::GizmoBaseColor.b, secondaryAlpha);

    const uint8_t tertiaryAlpha = static_cast<uint8_t>(255u / Theme::GizmoColorFalloff * tertiaryZoomFactor * tertiaryZoomFactor * tertiaryZoomFactor);
    const sf::Color TertiaryColor = sf::Color(Theme::GizmoBaseColor.r, Theme::GizmoBaseColor.g, Theme::GizmoBaseColor.b, tertiaryAlpha);

    const float primaryFrequency = 1.f / m_GizmoScale;
    const float secondaryFrequency = 2.f * primaryFrequency;
    const float tertiaryFrequency = 2.f * secondaryFrequency;

    const float secondaryScale = m_GizmoScale * 0.5f;
    const float tertiaryScale = secondaryScale * 0.5f;

    const sf::Vector2u targetSize = target.getSize();
    const sf::Vector2f screenCenter = sf::Vector2f(targetSize) * 0.5f;
    const sf::Vector2f worldCenter = screenCenter + m_Position;
    const sf::Vector2f gridOrigin = screenCenter + sf::Vector2f(std::fmod(m_Position.x, m_GizmoScale), std::fmod(m_Position.y, m_GizmoScale));

    const int halfLinesX = static_cast<int>(std::ceil(targetSize.x / (2.f * m_GizmoScale))) + 2;
    const int halfLinesY = static_cast<int>(std::ceil(targetSize.y / (2.f * m_GizmoScale))) + 2;
    const int halfLinesX2 = static_cast<int>(std::ceil(targetSize.x / (2.f * secondaryScale))) + 2;
    const int halfLinesY2 = static_cast<int>(std::ceil(targetSize.y / (2.f * secondaryScale))) + 2;
    const int halfLinesX3 = static_cast<int>(std::ceil(targetSize.x / (2.f * tertiaryScale))) + 2;
    const int halfLinesY3 = static_cast<int>(std::ceil(targetSize.y / (2.f * tertiaryScale))) + 2;

    sf::VertexArray vertices(sf::PrimitiveType::Lines, 2 * ((2 * halfLinesX3 + 1) + (2 * halfLinesY3 + 1)) + 2 * ((2 * halfLinesX2 + 1) + (2 * halfLinesY2 + 1)) + 2 * ((2 * halfLinesX + 1) + (2 * halfLinesY + 1)) + 4);
    unsigned int currentIndex = 0u;

    // --- Tertiary grid ---
    if (tertiaryAlpha) {
        for (int i = -halfLinesX3; i <= halfLinesX3; ++i) {
            const float x = gridOrigin.x + i * tertiaryScale;
            vertices[currentIndex++] = sf::Vertex(sf::Vector2f(x, 0.f), TertiaryColor);
            vertices[currentIndex++] = sf::Vertex(sf::Vector2f(x, static_cast<float>(targetSize.y)), TertiaryColor);
        }
        for (int i = -halfLinesY3; i <= halfLinesY3; ++i) {
            const float y = gridOrigin.y + i * tertiaryScale;
            vertices[currentIndex++] = sf::Vertex(sf::Vector2f(0.f, y), TertiaryColor);
            vertices[currentIndex++] = sf::Vertex(sf::Vector2f(static_cast<float>(targetSize.x), y), TertiaryColor);
        }
    }

    // --- Secondary grid ---
    for (int i = -halfLinesX2; i <= halfLinesX2; ++i) {
        const float x = gridOrigin.x + i * secondaryScale;
        vertices[currentIndex++] = sf::Vertex(sf::Vector2f(x, 0.f), SecondaryColor);
        vertices[currentIndex++] = sf::Vertex(sf::Vector2f(x, static_cast<float>(targetSize.y)), SecondaryColor);
    }
    for (int i = -halfLinesY2; i <= halfLinesY2; ++i) {
        const float y = gridOrigin.y + i * secondaryScale;
        vertices[currentIndex++] = sf::Vertex(sf::Vector2f(0.f, y), SecondaryColor);
        vertices[currentIndex++] = sf::Vertex(sf::Vector2f(static_cast<float>(targetSize.x), y), SecondaryColor);
    }

    // --- Primary grid ---
    for (int i = -halfLinesX; i <= halfLinesX; ++i) {
        const float x = gridOrigin.x + i * m_GizmoScale;
        vertices[currentIndex++] = sf::Vertex(sf::Vector2f(x, 0.f), PrimaryColor);
        vertices[currentIndex++] = sf::Vertex(sf::Vector2f(x, static_cast<float>(targetSize.y)), PrimaryColor);
    }
    for (int i = -halfLinesY; i <= halfLinesY; ++i) {
        const float y = gridOrigin.y + i * m_GizmoScale;
        vertices[currentIndex++] = sf::Vertex(sf::Vector2f(0.f, y), PrimaryColor);
        vertices[currentIndex++] = sf::Vertex(sf::Vector2f(static_cast<float>(targetSize.x), y), PrimaryColor);
    }

    // --- Axes ---
    vertices[currentIndex++] = sf::Vertex(sf::Vector2f(worldCenter.x, 0.f), Theme::GizmoBaseColor);
    vertices[currentIndex++] = sf::Vertex(sf::Vector2f(worldCenter.x, static_cast<float>(targetSize.y)), Theme::GizmoBaseColor);
    vertices[currentIndex++] = sf::Vertex(sf::Vector2f(0.f, worldCenter.y), Theme::GizmoBaseColor);
    vertices[currentIndex++] = sf::Vertex(sf::Vector2f(static_cast<float>(targetSize.x), worldCenter.y), Theme::GizmoBaseColor);

    target.draw(vertices);
}

void Application::renderColorRect(sf::RenderTarget& target, sf::Color color) {
    const sf::Vector2u size = target.getSize();

    const sf::Vertex vertices[] = {
        sf::Vertex(sf::Vector2f(0.f, 0.f), color),
        sf::Vertex(sf::Vector2f(static_cast<float>(size.x), 0.f), color),
        sf::Vertex(sf::Vector2f(0.f, static_cast<float>(size.y)), color),
        sf::Vertex(sf::Vector2f(static_cast<float>(size.x), static_cast<float>(size.y)), color)
    };

    target.draw(vertices, 4u, sf::PrimitiveType::TriangleStrip);
}

void Application::Render(sf::RenderTarget& target) {
    target.clear(Theme::BackgroundColor);

    renderGizmo(target);

    for (std::size_t i = 0u; i < m_Graphs.size(); ++i) {
        const float hue = static_cast<float>(i) / static_cast<float>(m_Graphs.size());
        const sf::Color graphColor = System::Color::HSLtoRGB(hue, 0.9f, 0.5f);

        m_Graphs[i].Render(target, graphColor, m_Position, m_GizmoScale);
    }

    if (m_GettingUserInput) {
        if (m_ShowPreview) {
            const std::string textboxString = m_Textbox.GetString();

            if (!textboxString.empty()) {
                Graph graph(false);

                auto equation = Equation::Parse(textboxString);
                if (equation && !graph.Generate(equation.value())) {
                    const float hue = m_Graphs.size() / static_cast<float>(m_Graphs.size() + 1);
                    const sf::Color color = System::Color::HSLtoRGB(hue, 0.9f, 0.5f);
                    graph.Render(target, color, m_Position, m_GizmoScale);
                }
            }
        }

        renderColorRect(target, sf::Color(25u, 25u, 35u, 150));
        m_Textbox.Render(target, m_Font);
    }
}

#pragma region Signals

std::optional<Application::SignalType> Application::ConsumeSignal() {
    if (m_SystemSignal == SignalType::None) {
        return std::nullopt;
    }

    const SignalType signal = m_SystemSignal;
    m_SystemSignal = SignalType::None;

    return std::make_optional(signal);
}