#pragma once

#include "SFML/Graphics.hpp"

#include "App/Application.hpp"

class Launcher final {
public:
    struct Config {
        unsigned int WindowWidth;
        unsigned int WindowHeight;
        unsigned int Fps;
        const char* WindowTitle;
        unsigned int WindowStyle;
        bool Fullscreen;
    };

    struct WindowState {
        sf::Vector2u Size;
        sf::Vector2i Position;
    };

private:
    void applyConfig(const Config& config);
    void toggleFullscreen();

    void onEvent(const sf::Event& event);

    void processApplicationSignal(Application::SignalType signal);

    sf::RenderWindow m_Window;

    WindowState m_LastWindowState;

    Config m_Config;
    
    Application m_Application;

public:
    Launcher(Config&& config);

    [[nodiscard]] bool LoadResources(const char* root);

    void HandleEvents();
    void Update();
    void Render();

    [[nodiscard]] inline bool isRunning() const noexcept {
        return m_Window.isOpen();
    }
};