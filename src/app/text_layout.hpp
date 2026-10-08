#pragma once
#include "app/graphics.hpp"
#include "app/text/font.hpp"

struct TextEntry {
    const Font::Character* ch;
    Point pos;
};
