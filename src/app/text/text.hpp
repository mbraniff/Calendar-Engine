#pragma once
#include "app/animation/animatable.hpp"
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
    Animation::Animatable<Color> color;
    Text(std::string text);
    void drawSelf(Renderer &) override;
    Size measure(Size proposed) override;
    void setTextAlignment(TextAlignment alignment);
    TextAlignment getTextAlignment();
private:
    std::vector<TextEntry> text_;
    Animation::Animatable<float> alignmentFactor_ = alignmentFactor(TextAlignment::Left);
};