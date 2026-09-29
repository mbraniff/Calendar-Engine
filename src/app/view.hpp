#pragma once
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <vector>
#include "app/graphics.hpp"
#include "renderer.hpp"

class View {
public:
    float opacity = 1.0f;

    void layout(Rect frame) {
        frame_ = frame;
        layoutSubviews();
        for (auto& child : subviews_)
            child->layout(child->frame_);
    }

    void render(Renderer& r) {
        // r.pushClip(clipPath_);
        r.pushOpacity(opacity);
        drawSelf(r);
        for (auto& child : subviews_)
            child->render(r);
        // r.popClip();
        r.popOpacity();
    }
protected:
    virtual Size measure(Size proposed) { return proposed; }
    virtual void layoutSubviews() {}
    virtual void drawSelf(Renderer&) {}

    Rect frame_;
    std::vector<std::unique_ptr<View>> subviews_;
};