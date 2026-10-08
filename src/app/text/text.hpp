#pragma once
#include "app/text/font.hpp"
#include "app/text/text_layout.hpp"
#include "app/view/view.hpp"
#include "app/rendering/renderer.hpp"
#include <string>
#include <vector>

class Text: public View {
public:
    int fontSize = 16;
    Font::Weight fontWeight = Font::Weight::Regular;
    Font& font = Font::Default();
    std::string text;
    Color color;
    Text(std::string text);
    void drawSelf(Renderer &) override;
    Size measure(Size proposed) override;
    void setTextAlignment(TextAlignment alignment);
private:
    std::vector<TextEntry> text_;
    TextAlignment textAlignment_ = TextAlignment::Left;
};