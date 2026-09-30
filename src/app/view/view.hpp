#pragma once
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <vector>
#include "app/graphics.hpp"
#include "app/rendering/renderer.hpp"

class View {
public:
    Alignment alignment = Alignment::center();
    float opacity = 1.0f;
    Point offset;

    void layout(Rect frame) {
        Size want = measure({frame.width, frame.height});
        want.width = std::min(want.width, frame.width);
        want.height = std::min(want.height, frame.height);

        frame_.origin = {
            frame.origin.x + (frame.width - want.width) * alignment.x + offset.x,
            frame.origin.y + (frame.height - want.height) * alignment.y + offset.y,
        };
        frame_.width = want.width;
        frame_.height = want.height;

        layoutSubviews();
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

    Size size() {
        return Size{frame_.width, frame_.height};
    }

    void addSubview(std::unique_ptr<View> view) {
        subviews_.push_back(std::move(view));
    }
protected:
    virtual Size measure(Size proposed) { return proposed; }
    virtual void layoutSubviews() {
        for (auto& child : subviews_)
            child->layout(frame_);
    }
    virtual void drawSelf(Renderer&) {}

    Rect frame_;
    std::vector<std::unique_ptr<View>> subviews_;
};