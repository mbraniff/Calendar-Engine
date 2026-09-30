#pragma once
#include "app/rendering/text_renderer.hpp"
#include "app/text/font.hpp"
#include <glm/fwd.hpp>
#include <string_view>
#include <vector>

class Renderer {
public:
    void drawText(Font::GlyphSet &set, const std::string_view text, Point pos, const Color &color) {
        float opacity = 1.0f;
        for (float o : opacity_) opacity *= o;
        textRenderer_.draw(set, text, pos, color, opacity);
    }

    void pushOpacity(float opacity) {
        opacity_.push_back(opacity);
    }

    void popOpacity() {
        opacity_.pop_back();
    }

    void setFrameBuffer(float w, float h) {
        textRenderer_.setFrameBuffer(w, h);
    }
private:
    std::vector<float> opacity_;
    TextRenderer textRenderer_;
};