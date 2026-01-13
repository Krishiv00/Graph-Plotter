#include <cmath>

#include "App/Textbox.hpp"

#pragma region Utils

constexpr inline bool IsWordChar(char c) noexcept {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

constexpr inline char GetClosingParan(char c) {
    if (c == '{') {
        return '}';
    } else if (c == '(') {
        return ')';
    }

    return 0;
}

constexpr inline bool IsOpeningParanChar(char c) noexcept {
    return c == '{' || c == '(';
}

constexpr inline bool IsClosingParanChar(char c) noexcept {
    return c == '}' || c == ')';
}

std::string AddSpacesAroundOperators(const std::string& expr) {
    const std::string ops = "+-*/%^";
    std::string out;

    for (std::size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];

        if (ops.find(c) != std::string::npos) {
            // Avoid double space
            if (!out.empty() && out.back() != ' ')
                out += ' ';

            out += c;

            // Add space after if next char is not already space
            if (i + 1 < expr.size() && expr[i + 1] != ' ')
                out += ' ';
        } else {
            out += c;
        }
    }

    // Collapse multiple spaces (from operators + pasted text)
    out.erase(
        std::unique(out.begin(), out.end(),
            [](char a, char b) { return a == ' ' && b == ' '; }),
        out.end()
    );

    return out;
}

std::string Textbox::processPastedString(std::string str) const {
    for (char& c : str) {
        if (c == '\n' || c == '\r') {
            c = ' ';
        }
    }

    const std::string ops = "+-*/%^";
    std::string result;
    result.reserve(str.size() * 3);

    for (std::size_t i = 0; i < str.size(); ++i) {
        char c = str[i];

        if (ops.find(c) != std::string::npos) {
            if (!result.empty() && result.back() != ' ') {
                result += ' ';
            }

            result += c;

            if (i + 1 < str.size() && str[i + 1] != ' ') {
                result += ' ';
            }
        } else {
            result += c;
        }
    }

    result.erase(
        std::unique(result.begin(), result.end(),
            [](char a, char b) { return a == ' ' && b == ' '; }),
        result.end()
    );

    const auto first = result.find_first_not_of(' ');
    if (first == std::string::npos) {
        return "";
    }

    const auto last = result.find_last_not_of(' ');
    return result.substr(first, last - first + 1);
}

std::size_t Textbox::findWordBoundary(std::size_t pos, bool dir) const {
    if (dir) {
        std::size_t end = pos;
        std::size_t sz = m_Buffer.size();

        if (end >= sz) {
            return sz;
        }

        while (end < sz && std::isspace(static_cast<unsigned char>(m_Buffer[end]))) {
            ++end;
        }

        if (end == sz) {
            return sz;
        }

        char current = m_Buffer[end];

        if (IsWordChar(current)) {
            while (end < sz && IsWordChar(m_Buffer[end])) {
                ++end;
            }
        } else {
            while (end < sz && !IsWordChar(m_Buffer[end]) && !std::isspace(static_cast<unsigned char>(m_Buffer[end])))
                ++end;
        }

        return end;
    } else {
        if (pos == 0u) {
            return 0u;
        }

        std::size_t start = pos;

        while (start && std::isspace(static_cast<unsigned char>(m_Buffer[start - 1]))) {
            --start;
        }

        if (start == 0u) {
            return 0u;
        }

        char current = m_Buffer[start - 1];

        if (IsWordChar(current)) {
            while (start && IsWordChar(m_Buffer[start - 1])) {
                --start;
            }
        } else {
            while (start && !IsWordChar(m_Buffer[start - 1]) && !std::isspace(static_cast<unsigned char>(m_Buffer[start - 1]))) {
                --start;
            }
        }

        return start;
    }
}

std::pair<std::size_t, std::size_t> Textbox::getSelectionRange() const {
    return {std::min(m_CursorPosition, m_SelectingAnchor), std::max(m_CursorPosition, m_SelectingAnchor)};
}

void Textbox::clearSelection() {
    if (m_CursorPosition != m_SelectingAnchor) {
        auto [start, end] = getSelectionRange();
        m_Buffer.erase(m_Buffer.begin() + start, m_Buffer.begin() + end);
        m_CursorPosition = start;
        m_SelectingAnchor = start;
    }
}

std::string Textbox::Consume() noexcept {
    std::string buffer = m_Buffer;

    m_Buffer.clear();
    m_CursorPosition = 0u;
    m_SelectingAnchor = 0u;

    return buffer;
}

#pragma region Key Press

void Textbox::HandleKeyPress(sf::Keyboard::Scancode key) {
    if (key == sf::Keyboard::Scancode::Backspace) {
        if (m_CursorPosition != m_SelectingAnchor) {
            clearSelection();
        } else if (m_CursorPosition) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)) {
                const std::size_t newPos = findWordBoundary(m_CursorPosition, false);

                m_Buffer.erase(m_Buffer.begin() + newPos, m_Buffer.begin() + m_CursorPosition);
                m_CursorPosition = newPos;
            } else {
                m_Buffer.erase(m_Buffer.begin() + --m_CursorPosition);
            }

            m_SelectingAnchor = m_CursorPosition;
        }
    }

    else if (key == sf::Keyboard::Scancode::Delete) {
        if (m_CursorPosition != m_SelectingAnchor) {
            clearSelection();
        } else if (m_CursorPosition < m_Buffer.size()) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)) {
                m_Buffer.erase(m_Buffer.begin() + m_CursorPosition, m_Buffer.begin() + findWordBoundary(m_CursorPosition, true));
            } else {
                m_Buffer.erase(m_Buffer.begin() + m_CursorPosition);
            }
        }
    }

    else if (
        key == sf::Keyboard::Scancode::Left ||
        key == sf::Keyboard::Scancode::Right
    ) {
        const bool shiftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift);

        if (!shiftPressed && m_CursorPosition != m_SelectingAnchor) {
            m_CursorPosition = key == sf::Keyboard::Scancode::Left ? std::min(m_CursorPosition, m_SelectingAnchor) : std::max(m_CursorPosition, m_SelectingAnchor);
        } else {
            if (key == sf::Keyboard::Scancode::Left) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)) {
                    m_CursorPosition = findWordBoundary(m_CursorPosition, false);
                } else if (m_CursorPosition > 0) {
                    --m_CursorPosition;
                }
            }

            else {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)) {
                    m_CursorPosition = findWordBoundary(m_CursorPosition, true);
                } else if (m_CursorPosition < m_Buffer.size()) {
                    ++m_CursorPosition;
                }
            }
        }

        if (!shiftPressed) {
            m_SelectingAnchor = m_CursorPosition;
        }
    }

    else if (
        key == sf::Keyboard::Scancode::Home ||
        key == sf::Keyboard::Scancode::End
    ) {
        if (key == sf::Keyboard::Scancode::Home) {
            m_CursorPosition = 0u;
        }

        else {
            m_CursorPosition = m_Buffer.size();
        }

        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift)) {
            m_SelectingAnchor = m_CursorPosition;
        }
    }

    else if (key == sf::Keyboard::Scancode::A) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)) {
            m_SelectingAnchor = 0u;
            m_CursorPosition = m_Buffer.size();
        }
    }

    else if (
        key == sf::Keyboard::Scancode::C ||
        key == sf::Keyboard::Scancode::X
    ) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)) {
            if (m_CursorPosition != m_SelectingAnchor) {
                auto [start, end] = getSelectionRange();

                std::string selection(m_Buffer.begin() + start, m_Buffer.begin() + end);

                sf::Clipboard::setString(selection);

                if (key == sf::Keyboard::Scancode::X) {
                    clearSelection();
                }
            }
        }
    }

    else if (key == sf::Keyboard::Scancode::V) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LControl)) {
            std::string clip = processPastedString(sf::Clipboard::getString());

            if (!clip.empty()) {
                if (m_CursorPosition != m_SelectingAnchor) {
                    clearSelection();
                }

                m_Buffer.insert(m_Buffer.begin() + m_CursorPosition, clip.begin(), clip.end());

                m_CursorPosition += clip.size();
                m_SelectingAnchor = m_CursorPosition;
            }
        }
    }
}

#pragma region Text Enter

void Textbox::Type(char c) {
    if (static_cast<unsigned char>(c) < 32u || c == 127u) {
        return;
    }

    if (m_CursorPosition != m_SelectingAnchor) {
        auto [start, end] = getSelectionRange();

        if (IsOpeningParanChar(c) || c == '\"') {
            auto [start, end] = getSelectionRange();

            m_Buffer.insert(m_Buffer.begin() + start, c);
            m_Buffer.insert(m_Buffer.begin() + end + 1, c == '\"' ? c : GetClosingParan(c));

            m_SelectingAnchor = start + 1;
            m_CursorPosition = end + 1;

            return;
        } else {
            clearSelection();
        }
    }

    if ((IsClosingParanChar(c) || c == '\"') && m_CursorPosition < m_Buffer.size() && m_Buffer[m_CursorPosition] == c) {
        ++m_CursorPosition;
        m_SelectingAnchor = m_CursorPosition;
    } else {
        m_Buffer.insert(m_Buffer.begin() + m_CursorPosition, c);
        ++m_CursorPosition;
        m_SelectingAnchor = m_CursorPosition;

        if (IsOpeningParanChar(c) || c == '\"') {
            m_Buffer.insert(m_Buffer.begin() + m_CursorPosition, c == '\"' ? c : GetClosingParan(c));
        }
    }
}

#pragma region Rendering

float ComputeScaleFalloff(float rawWidth, float maxWidth) {
    if (rawWidth <= maxWidth)
        return 1.f;

    const float excess = rawWidth - maxWidth;
    float s = 1.f - (excess / (maxWidth * 1.5f));
    return std::clamp(s, 0.85f, 1.f);
}

void Textbox::Render(sf::RenderTarget& target, const sf::Font& font) {
    constexpr float CharacterSize = 60.f;

    const sf::Vector2u targetSize = target.getSize();
    const sf::Vector2f textPosition = sf::Vector2f(targetSize) * 0.5f;

    const float borderOffset = targetSize.x * 0.05f;
    const float effectiveTargetWidth = target.getSize().x - 2.f * borderOffset;

    if (m_Buffer.empty()) {
        // render cursor and leave
        sf::RectangleShape caret(sf::Vector2f(2.f, CharacterSize * 1.3f));
        caret.setPosition(textPosition - caret.getSize() * 0.5f);

        target.draw(caret);

        return;
    }

    sf::Text text(font, m_Buffer, static_cast<unsigned int>(CharacterSize));

    const sf::FloatRect rawBounds = text.getLocalBounds();
    const float smoothScale = ComputeScaleFalloff(rawBounds.size.x, effectiveTargetWidth);

    float clampScale = 1.f;
    if (rawBounds.size.x * smoothScale > effectiveTargetWidth) {
        clampScale = effectiveTargetWidth / (rawBounds.size.x * smoothScale);
    }

    const float finalScale = smoothScale * clampScale;
    text.setScale(sf::Vector2f(finalScale, finalScale));

    const sf::FloatRect textBounds = text.getLocalBounds();

    const float charWidth = finalScale * (textBounds.size.x + CharacterSize * 0.1f) / static_cast<float>(m_Buffer.length());

    text.setOrigin(sf::Vector2f(textBounds.size.x * 0.5f, CharacterSize * 0.5f));
    text.setPosition(textPosition);

    target.draw(text);

    if (m_CursorPosition != m_SelectingAnchor) {
        const auto [selStart, selEnd] = getSelectionRange();
        const std::size_t selectionLength = selEnd - selStart;

        sf::RectangleShape selectRect(sf::Vector2f(charWidth * selectionLength / finalScale, CharacterSize * 1.3f));
        selectRect.setScale(sf::Vector2f(finalScale, finalScale));
        selectRect.setOrigin(text.getOrigin());
        selectRect.setPosition(sf::Vector2f(textPosition.x + selStart * charWidth, textPosition.y));
        selectRect.setFillColor(sf::Color(50, 100, 200, 125));

        target.draw(selectRect);
    }

    sf::RectangleShape caret(sf::Vector2f(2.f, CharacterSize * 1.3f));
    caret.setOrigin(text.getOrigin());
    caret.setPosition(sf::Vector2f(textPosition.x + m_CursorPosition * charWidth, textPosition.y));
    caret.setScale(sf::Vector2f(finalScale, finalScale));

    target.draw(caret);
}