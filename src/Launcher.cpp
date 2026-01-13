#include "System/Launcher.hpp"

Launcher::Launcher(Config&& config) : m_Config(std::move(config)) {
    // size in case the window begin with fullscreen mode
    // in that case program has no idea about the state before fullscreen
    const sf::Vector2u defaultSize = sf::Vector2u(1280u, 720u);
    const sf::Vector2u desktopSize = sf::VideoMode::getDesktopMode().size;

    m_LastWindowState.Size = defaultSize;
    m_LastWindowState.Position = sf::Vector2i((desktopSize - defaultSize) / 2u);

    applyConfig(m_Config);
}

void Launcher::Update() {
    static sf::Clock deltaClock;

    const float deltaTime = deltaClock.restart().asSeconds();

    m_Application.State.MousePosition = sf::Mouse::getPosition(m_Window);
    m_Application.Update(deltaTime);

    if (auto signal = m_Application.ConsumeSignal()) {
        processApplicationSignal(signal.value());
    }
}

void Launcher::processApplicationSignal(Application::SignalType signal) {
    if (signal == Application::SignalType::Exit) {
        m_Window.close();
    }
}

void Launcher::Render() {
    m_Window.clear();

    m_Application.Render(m_Window);

    m_Window.display();
}

void Launcher::HandleEvents() {
    while (const std::optional<sf::Event> event = m_Window.pollEvent()) {
        onEvent(event.value());
    }
}

bool Launcher::LoadResources(const char* root) {
    Render(); // render before loading resources to make loading time less obvious.

    if (!m_Application.LoadResources(root)) [[unlikely]] {
        m_Window.close();

        return false;
    }

    return true;
}

void Launcher::applyConfig(const Config& config) {
    m_Window.create(
        sf::VideoMode(sf::Vector2u(config.WindowWidth, config.WindowHeight)),
        config.WindowTitle,
        config.WindowStyle,
        config.Fullscreen ? sf::State::Fullscreen : sf::State::Windowed
    );

    m_Window.setFramerateLimit(config.Fps);
}

void Launcher::toggleFullscreen() {
    const bool wasFullscreen = m_Config.Fullscreen;

    m_Config.Fullscreen ^= true;

    if (wasFullscreen) {
        m_Config.WindowWidth = m_LastWindowState.Size.x;
        m_Config.WindowHeight = m_LastWindowState.Size.y;
    } else {
        const sf::Vector2u desktopSize = sf::VideoMode::getDesktopMode().size;

        m_Config.WindowWidth = desktopSize.x;
        m_Config.WindowHeight = desktopSize.y;

        m_LastWindowState.Position = m_Window.getPosition();
        m_LastWindowState.Size = m_Window.getSize();
    }

    applyConfig(m_Config);

    if (wasFullscreen) {
        m_Window.setPosition(m_LastWindowState.Position);
    }
}

void Launcher::onEvent(const sf::Event& event) {
    if (event.is<sf::Event::Closed>()) {
        m_Window.close();
    }

    else if (const auto* resize = event.getIf<sf::Event::Resized>()) {
        m_Window.setView(sf::View(sf::FloatRect(sf::Vector2f(), sf::Vector2f(resize->size))));
    }

    else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->scancode == sf::Keyboard::Scancode::Escape) {
            m_Window.close();
        }

        else if (key->scancode == sf::Keyboard::Scancode::F11) {
            toggleFullscreen();
        }

        else {
            m_Application.HandleKeyPress(key->scancode);
        }
    }

    else if (const auto* button = event.getIf<sf::Event::MouseButtonPressed>()) {
        m_Application.HandleMouseButtonPress(button->button);
    }

    else if (const auto* button = event.getIf<sf::Event::MouseButtonReleased>()) {
        m_Application.HandleMouseButtonRelease(button->button);
    }

    else if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>()) {
        m_Application.HandleMouseWheelScroll(scroll->delta);
    }

    else if (const auto* text = event.getIf<sf::Event::TextEntered>()) {
        m_Application.HandleCharTyped(static_cast<char>(text->unicode));
    }
}