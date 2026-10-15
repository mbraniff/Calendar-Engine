#pragma once
#include "app/animation/animatable.hpp"
#include "app/graphics.hpp"
#include "app/text/font.hpp"
#include "app/text/text_layout.hpp"
#include "app/view/view.hpp"
#include "app/rendering/renderer.hpp"
#include <limits>
#include <optional>
#include <string>
#include <vector>

class Text: public View {
public:
    int fontSize = 16;
    Font::Weight fontWeight = Font::Weight::Regular;
    Font& font = Font::Default();
    Observed<std::string> text;
    Animation::Animatable<Color> color;
    Text(std::string text);
    void drawSelf(Renderer &) override;
    Size measure(std::optional<float> w, std::optional<float> h) override;
    void setTextAlignment(TextAlignment alignment);
    TextAlignment getTextAlignment();
protected:
    void layoutSubviews() override;
private:
    struct Line { size_t end; float width; };
    void align(float factor);

    // Parents probe measure() at sizes we may never take, so it lays glyphs out here;
    // layoutSubviews() keeps the final measure's glyphs in text_.
    std::vector<TextEntry> measured_;
    std::vector<TextEntry> text_;
    std::vector<Line> lines_;
    float textWidth_ = 0;
    // text_ shifted for alignment, rebuilt when the (animating) factor moves.
    std::vector<TextEntry> aligned_;
    float alignedFactor_ = std::numeric_limits<float>::quiet_NaN();
    Animation::Animatable<float> alignmentFactor_ = alignmentFactor(TextAlignment::Left);
};