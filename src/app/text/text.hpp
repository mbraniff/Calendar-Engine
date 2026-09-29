#pragma once
#include "app/text/font.hpp"
#include "app/view/view.hpp"
#include <string>

class Text: public View {
public:
    int fontSize = 16;
    Font::Weight fontWeight = Font::Weight::Regular;
    Font& font = Font::Default();
    std::string text;
    Text(std::string text);
    void drawSelf(Renderer &) override;
};