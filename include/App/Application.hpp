#pragma once

#include <vector>

#include "SFML/Graphics.hpp"

#include "App/Graph.hpp"
#include "App/Textbox.hpp"
#include "App/Equation.hpp"

class Application final {
public:
    enum class SignalType : uint8_t {
        None,
        Exit
    };

private:
    struct State {
        sf::Vector2i MousePosition;
    };

    void invokeError(const std::string& errorMessage);

    void updatePanning();
    void updateZoom(float deltaTime);
    void updateViewport(float deltaTime);

    void renderGizmo(sf::RenderTarget& target);
    void renderColorRect(sf::RenderTarget& target, sf::Color color);

    float m_GizmoScale;
    float m_ZoomMomentum{0.f};

    sf::Vector2f m_Position{0.f, 0.f};

    std::vector<Graph> m_Graphs;

    bool m_Grabbed{false};
    bool m_RestoringDefaultView{false};
    bool m_GettingUserInput{false};
    bool m_ShowPreview{false};

    sf::Font m_Font;
    Textbox m_Textbox;

    sf::Vector2i m_LastMousePosition;

    SignalType m_SystemSignal{SignalType::None};

public:
    Application();

    [[nodiscard]] bool LoadResources(const char* root);

    void Update(float deltaTime);
    void Render(sf::RenderTarget& target);

    void HandleKeyPress(sf::Keyboard::Scancode key);
    void HandleCharTyped(char c);
    void HandleMouseButtonPress(sf::Mouse::Button button);
    void HandleMouseButtonRelease(sf::Mouse::Button button);
    void HandleMouseWheelScroll(float delta);

    [[nodiscard]] std::optional<SignalType> ConsumeSignal();

    State State;
};