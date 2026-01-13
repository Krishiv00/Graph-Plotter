#pragma once

#include "SFML/Graphics/Color.hpp"

namespace System {
    namespace Color {
        constexpr float hue2rgb(float p, float q, float t) {
            if (t < 0.f) t += 1.f;
            if (t > 1.f) t -= 1.f;
            if (t < 1.f / 6.f) return p + (q - p) * 6.f * t;
            if (t < 1.f / 2.f) return q;
            if (t < 2.f / 3.f) return p + (q - p) * (2.f / 3.f - t) * 6.f;
            
            return p;
        }

        constexpr sf::Color HSLtoRGB(float h, float s, float l) {
            float r, g, b;

            if (s == 0.f) {
                r = g = b = l;
            } else {
                float q = l < 0.5f ? l * (1.f + s) : l + s - l * s;
                float p = 2.f * l - q;

                r = hue2rgb(p, q, h + 1.f / 3.f);
                g = hue2rgb(p, q, h);
                b = hue2rgb(p, q, h - 1.f / 3.f);
            }

            return sf::Color(static_cast<uint8_t>(r * 255.f), static_cast<uint8_t>(g * 255.f), static_cast<uint8_t>(b * 255.f));
        }
    }
}
