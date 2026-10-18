#pragma once
#include "ui/GL.h"
#include "ui/rendering/image_renderer.hpp"
#include "ui/rendering/shape_renderer.hpp"
#include "ui/rendering/text_renderer.hpp"
#include "ui/shapes/mask.hpp"
#include "ui/text/font.hpp"
#include "ui/text/text_layout.hpp"
#include <cmath>
#include <glm/fwd.hpp>
#include <utility>
#include <vector>

class Renderer {
public:
    Renderer() { whiteTex_ = uploadMask({255}, 1, 1); }

    void drawText(const std::vector<TextEntry>& text, Font::GlyphSet& set, Point pos, const Color &color) {
        GLuint tex = whiteTex_;
        glm::vec4 rect(0, 0, width_, height_);
        const auto o = offset();
        if(!getMask(tex, rect)) return;
        textRenderer_.setMask(rect, tex);
        float opacity = 1.0f;
        for (float o : opacity_) opacity *= o;
        textRenderer_.setOpacity(opacity);
        textRenderer_.setColor(color);
        textRenderer_.draw(text, pos + o, set);
    }

    void drawShape(const Path& path, Color color, Mask& fill, std::optional<float> stroke = std::nullopt) {
        fill.update(path, nullptr, stroke);
        if (fill.tex == 0) return;

        GLuint tex = whiteTex_;
        glm::vec4 rect(0, 0, width_, height_);
        if (!getMask(tex, rect)) return;
        shapeRenderer_.setMask(rect, tex);

        float opacity = 1.0f;
        for (float o : opacity_) opacity *= o;
        shapeRenderer_.setOpacity(opacity);
        shapeRenderer_.setColor(color);
        // Whole pixels: the coverage texture is sampled nearest, so a fractional shift would shimmer.
        Point o = offset();
        shapeRenderer_.draw(fill, { std::round(o.x), std::round(o.y) });
    }

    // Draws uv's part of image over rect, which is in frame space like the rects views lay out.
    void drawImage(GLuint image, const Rect& rect, const glm::vec4& uv, Color color, float templateMode) {
        GLuint tex = whiteTex_;
        glm::vec4 mask(0, 0, width_, height_);
        if (!getMask(tex, mask)) return;
        imageRenderer_.setMask(mask, tex);

        float opacity = 1.0f;
        for (float o : opacity_) opacity *= o;
        imageRenderer_.setOpacity(opacity);
        // No rounding, unlike shapes: the image is filtered, so a fractional offset moves it smoothly.
        Point o = offset();
        Rect screen = { { rect.origin.x + o.x, rect.origin.y + o.y }, rect.width, rect.height };
        imageRenderer_.draw(image, screen, uv, color, templateMode);
    }

    void pushClip(const Path& path, Mask& mask) {
        const Mask* parent = masks_.empty() ? nullptr : masks_.back().first;
        mask.update(path, parent);
        masks_.push_back({ &mask, offset() });
    }

    void popClip() {
        masks_.pop_back();
    }

    void pushOpacity(float opacity) {
        opacity_.push_back(opacity);
    }

    void popOpacity() {
        opacity_.pop_back();
    }

    void pushTranslation(Point p) {
        auto back = offsets_.empty() ? Point{} : offsets_.back();
        offsets_.push_back(p + back);
    }

    void popTranslation() {
        offsets_.pop_back();
    }

    void setFrameBuffer(float w, float h) {
        width_ = w; height_ = h;
        textRenderer_.setFrameBuffer(w, h);
        shapeRenderer_.setFrameBuffer(w, h);
        imageRenderer_.setFrameBuffer(w, h);
    }

    bool isClippedOut(const Rect& r) const {
        // No clip active: nothing to be clipped out of. getMask wouldn't fill rect in this case.
        if (masks_.empty()) return false;
        glm::vec4 rect;
        GLuint _ = 0;
        // An empty clip hides everything.
        if (!getMask(_, rect)) return true;
        // Both in screen space: the mask carries the offset it was pushed under, the frame takes the current one.
        Point o = offset();
        float x = r.origin.x + o.x, y = r.origin.y + o.y;
        return (x + r.width < rect.x || x > rect.x + rect.z)
            || (y + r.height < rect.y || y > rect.y + rect.w);
    }
private:
    std::vector<float> opacity_;
    std::vector<std::pair<Mask*, Point>> masks_;
    std::vector<Point> offsets_;
    GLuint whiteTex_ = 0;
    TextRenderer textRenderer_;
    ShapeRenderer shapeRenderer_;
    ImageRenderer imageRenderer_;
    float width_ = 0, height_ = 0;

    bool getMask(GLuint& tex, glm::vec4& rect) const {
        if (masks_.empty()) return true;
        const auto clip = masks_.back();
        const auto& m = *clip.first;
        if (m.tex == 0) return false; 
        tex = m.tex;
        rect = glm::vec4(m.x0 + clip.second.x, m.y0 + clip.second.y, m.w, m.h);
        return true;
    }

    Point offset() const {
        return offsets_.empty() ? Point{} : offsets_.back();
    }
};