#pragma once
#include <glm/ext/vector_float2.hpp>
#include <memory>
#include <optional>
#include <vector>
#include "app/animation/animatable.hpp"
#include "app/gestures/gesture.hpp"
#include "app/graphics.hpp"
#include "app/rendering/renderer.hpp"
#include "app/shapes/shape.hpp"

class GestureRecognizer;

class View : public std::enable_shared_from_this<View> {
public:
    Alignment alignment = Alignment::center();
    Animation::Animatable<float> opacity = 1.0f;
    Animation::Animatable<Point> offset;
    std::unique_ptr<Shape> clipShape;
    std::shared_ptr<View> background;

    Size sizeThatFits(std::optional<float> w, std::optional<float> h) {
        float padW = padding_.left + padding_.right;
        float padH = padding_.top + padding_.bottom;
        Size inner = measure( w.has_value() ? std::max(0.0f, w.value() - padW) : w,
            h.has_value() ? std::max(0.0f, h.value() - padH) : h);
        return { inner.width + padW, inner.height + padH };
    }

    void layout(Proposal p) {
        // setNeedsLayout() only marks ancestors, so a parent that changed size reaches us as a new proposal.
        if (!needsLayout_ && p == lastProposal_) { return; }
        lastProposal_ = p;
        Size want = sizeThatFits(p.width, p.height);
        want.width = std::min(want.width, p.width.value_or(infinity));
        want.height = std::min(want.height, p.height.value_or(infinity));

        // An unspecified dimension leaves no slack to align within, so the view sits at the origin.
        anchor_ = {
            p.origin.x + (p.width.value_or(want.width) - want.width) * alignment.x,
            p.origin.y + (p.height.value_or(want.height) - want.height) * alignment.y,
        };
        Point offset = this->offset;
        frame_.origin = { anchor_.x, anchor_.y };


        frame_.width = want.width;
        frame_.height = want.height;

        if (background) background->layout(frame_.to_proposal());

        needsLayout_ = false;
        layoutSubviews();
    }

    void render(Renderer& r) {
        r.pushTranslation(offset);
        if (clipShape) r.pushClip(clipShape->pathIn(frame_), clipMask_);
        r.pushOpacity(opacity);
        if (!r.isClippedOut(frame_)) {
            if (background) background->render(r);
            drawSelf(r);
        }
        for (auto& child : subviews_)
            child->render(r);
        if (clipShape) r.popClip();
        r.popOpacity();
        r.popTranslation();
    }

    Size size() {
        return Size{frame_.width, frame_.height};
    }

    const Rect frame() {
        return frame_;
    }

    Point anchor() const { return anchor_; }

    virtual void addSubview(std::shared_ptr<View> view) {
        view->parent_ = std::weak_ptr<View>(weak_from_this());
        subviews_.push_back(std::move(view));
        setNeedsLayout();
    }

    void padding(float value, Padding::Edge edge = Padding::Edge::All) {
        padding_.set(edge, value);
    }

    std::shared_ptr<View> frame(FrameOptions options);
    virtual std::shared_ptr<View> hitTest(Point p);
    const std::vector<std::shared_ptr<GestureRecognizer>> gestureRecognizers();
    std::weak_ptr<View> parent();
    // Maps a screen point into the space frames are laid out in, by taking off our offset and our ancestors'.
    Point fromScreen(Point p);
    void addGesture(std::shared_ptr<GestureRecognizer> recognizer) {
        gestureRecognizers_.push_back(recognizer);
    }

    virtual void pointerDown(int c, Point p, double t)  { if (auto v = parent_.lock()) v->pointerDown(c, p, t); }
    virtual void pointerMoved(int c, Point p, double t) { if (auto v = parent_.lock()) v->pointerMoved(c, p, t); }
    virtual void pointerUp(int c, Point p, double t)    { if (auto v = parent_.lock()) v->pointerUp(c, p, t); }
    virtual void pointerCancelled()                     { if (auto v = parent_.lock()) v->pointerCancelled(); }

    void setNeedsLayout() {
        needsLayout_ = true;
        if (background) background->setNeedsLayout();
        auto p = parent_.lock();
        if (p) p->setNeedsLayout();
    }
protected:
    virtual Size measure(std::optional<float> w, std::optional<float> h) {
        if (w.has_value() && h.has_value()) return { w.value(), h.value() };
        if (subviews_.empty()) return { w.value_or(0), h.value_or(0) };

        Size out;
        for (auto& v : subviews_) {
            auto ideal = v->sizeThatFits(w, h);
            out.width = std::max(out.width, ideal.width);
            out.height = std::max(out.height, ideal.height);
        }
        return out;
    }
    virtual void layoutSubviews() {
        for (auto& child : subviews_)
            child->layout(contentFrame().to_proposal());
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
    Point anchor_;
    std::vector<std::shared_ptr<GestureRecognizer>> gestureRecognizers_;
    bool needsLayout_ = true;
    Proposal lastProposal_;
};