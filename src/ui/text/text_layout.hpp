#pragma once
#include "ui/graphics.hpp"
#include "ui/text/font.hpp"

enum class TextAlignment {
    Left, Centered, Right
 };

constexpr static float alignmentFactor(TextAlignment alignment) {
    switch (alignment) {
    case TextAlignment::Left:
        return 0;
    case TextAlignment::Centered:
        return 0.5f;
    case TextAlignment::Right:
        return 1.0f;
    }
    return 0;
}

struct TextEntry {
    const Font::Character* ch;
    Point pos;
};
