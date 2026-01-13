#pragma once

#include "SFML/Graphics.hpp"

class Textbox {
private:
    std::string processPastedString(std::string str) const;
    std::size_t findWordBoundary(std::size_t pos, bool dir) const;
    std::pair<std::size_t, std::size_t> getSelectionRange() const;
    void clearSelection();

    std::string m_Buffer;
    std::size_t m_CursorPosition{0u};
    std::size_t m_SelectingAnchor{0u};

public:
    void HandleKeyPress(sf::Keyboard::Scancode key);
    void Type(char c);

    void Render(sf::RenderTarget& target, const sf::Font& font);

    std::string Consume() noexcept;

    inline const std::string& GetString() const noexcept {
        return m_Buffer;
    }
};