#pragma once
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <vector>
#include "app/gestures/gesture.hpp"
#include "app/graphics.hpp"
#include "app/rendering/renderer.hpp"
#include "app/shapes/shape.hpp"

class GestureRecognizer;

class View : public std::enable_shared_from_this<View> {
public:
    Alignment alignment = Alignment::center();
    float opacity = 1.0f;
    Point offset;
    std::unique_ptr<Shape> clipShape;
    std::shared_ptr<View> background;

    Size sizeThatFits(Size proposal) {
        float padW = padding_.left + padding_.right;
        float padH = padding_.top + padding_.bottom;
        Size inner = measure({ std::max(0.0f, proposal.width - padW),
            std::max(0.0f, proposal.height - padH) });
        return { inner.width + padW, inner.height + padH };
    }

    void layout(Rect frame) {
        Size want = sizeThatFits({frame.width, frame.height});
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

    const Rect frame() {
        return frame_;
    }

    void addSubview(std::shared_ptr<View> view) {
        view->parent_ = std::weak_ptr<View>(weak_from_this());
        subviews_.push_back(std::move(view));
    }

    void padding(float value, Padding::Edge edge = Padding::Edge::All) {
        padding_.set(edge, value);
    }

    std::shared_ptr<View> frame(FrameOptions options);
    virtual std::shared_ptr<View> hitTest(Point p);
    const std::vector<std::shared_ptr<GestureRecognizer>> gestureRecognizers();
    std::weak_ptr<View> parent();
    void addGesture(std::shared_ptr<GestureRecognizer> recognizer) {
        gestureRecognizers_.push_back(recognizer);
    }

    virtual void pointerDown(int, Point) {}
    virtual void pointerMoved(int, Point) {}
    virtual void pointerUp(int, Point) {}
    virtual void pointerCancelled() {}

protected:
    virtual Size measure(Size proposed) { return proposed; }
    virtual void layoutSubviews() {
        for (auto& child : subviews_)
            child->layout(contentFrame());
    }
    virtual void drawSelf(Renderer&) {}

    Rect contentFrame() const {
        return { { frame_.origin.x + padding_.left, frame_.origin.y + padding_.top},
            frame_.width - padding_.left - padding_.right,
            frame_.height - padding_.top - padding_.bottom };
    }

    Rect frame_;
    std::vector<std::shared_ptr<View>> subviews_;
    Mask clipMask_;
    Padding padding_ = Padding(0);
    std::weak_ptr<View> parent_;
    std::vector<std::shared_ptr<GestureRecognizer>> gestureRecognizers_;
};