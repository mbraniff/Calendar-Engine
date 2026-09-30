#pragma once
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <vector>
#include "app/graphics.hpp"
#include "app/rendering/renderer.hpp"
#include "app/shapes/shape.hpp"

class View {
public:
    Alignment alignment = Alignment::center();
    float opacity = 1.0f;
    Point offset;
    std::unique_ptr<Shape> clipShape;
    std::shared_ptr<View> background;

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

        if (background) background->layout(frame_);

        layoutSubviews();
    }

    void render(Renderer& r) {
        if (clipShape) r.pushClip(clipShape->pathIn(frame_), clipMask_);
        r.pushOpacity(opacity);
        if (background) background->render(r);
        drawSelf(r);
        for (auto& child : subviews_)
            child->render(r);
        if (clipShape) r.popClip();
        r.popOpacity();
    }

    Size size() {
        return Size{frame_.width, frame_.height};
    }

    void addSubview(std::shared_ptr<View> view) {
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
    std::vector<std::shared_ptr<View>> subviews_;
    Mask clipMask_;
};