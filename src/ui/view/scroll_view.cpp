#include "ui/view/scroll_view.hpp"
#include "ui/animation/animator.hpp"
#include "ui/animation/motion.hpp"
#include "ui/gestures/drag_gesture.hpp"
#include "ui/graphics.hpp"
#include "ui/shapes/shape.hpp"
#include "ui/view/view.hpp"
#include <algorithm>
#include <memory>
#include <optional>

// How far past the edge of the content a drag can pull before it stops moving.
static constexpr float maxScrollOverflow = 100.0f;

// Offsets run negative as content scrolls, so the content's edge is at 0 and the far edge at
// viewport - content, or 0 too when the content is smaller than the viewport.
static float clampOffset(float value, float viewport, float content) {
    return std::clamp(value, std::min(viewport - content, 0.0f), 0.0f);
}

// Moves value by delta, resisting more the further past the content's edge it is, until it stops at maxScrollOverflow.
static float rubberBand(float value, float delta, float viewport, float content) {
    float over = value - clampOffset(value, viewport, content);
    // Only pulling further out is resisted; heading back toward the content moves freely.
    if (over * delta > 0) delta *= std::max(1.0f - std::abs(over) / maxScrollOverflow, 0.0f);
    return value + delta;
}

ScrollView::ScrollView(ScrollView::ScrollAxis axis) : axis_(axis) {
    container_ = std::make_shared<View>();
    auto weak = std::weak_ptr(container_);
    auto scroller = std::make_shared<DragGestureRecognizer>();
    // The gesture is owned by this view, so it can't outlive this.
    scroller->onChange([this, weak] (DragGestureRecognizer::DragEvent e){
        auto view = weak.lock();
        if (!view) return;
        Rect viewport = contentFrame();
        Size content = view->size();
        view->offset.update([&](Point& p){
            if (axis_ != ScrollAxis::Vertical) p.x = rubberBand(p.x, e.translation.width, viewport.width, content.width);
            if (axis_ != ScrollAxis::Horizontal) p.y = rubberBand(p.y, e.translation.height, viewport.height, content.height);
        });
        // Update indicators
    })
    .onEnd([this, weak] (DragGestureRecognizer::DragEvent e){
        auto view = weak.lock();
        if (!view) return;
        Rect viewport = contentFrame();
        Size content = view->size();
        Point p = view->offset.target();
        Point edge = { clampOffset(p.x, viewport.width, content.width), clampOffset(p.y, viewport.height, content.height) };
        if (edge != p) {
            // Let go while overscrolled: spring back to the edge of the content.
            view->offset.setVelocity({ 0, 0 });
            animate(Animation::spring(), [&]() { view->offset = edge; });
            return;
        }
        // Dragging set the offset directly, which zeroes its velocity; the decay needs the flick's.
        view->offset.setVelocity({ axis_ != ScrollAxis::Vertical ? e.velocity.width : 0,
            axis_ != ScrollAxis::Horizontal ? e.velocity.height : 0 });
        animate(Animation::decay(), [&]() {
            auto distance = e.projectedEnd - e.location;
            view->offset.update([&] (Point& p){
                if (axis_ != ScrollAxis::Vertical) p.x = clampOffset(p.x + distance.x, viewport.width, content.width);
                if (axis_ != ScrollAxis::Horizontal) p.y = clampOffset(p.y + distance.y, viewport.height, content.height);
            });
        });
    });
    addGesture(scroller);
    clipShape = std::make_unique<Rectangle>();
}

void ScrollView::layoutSubviews() {
    // Content is unbounded along the scrolling axis, and matches our size across it.
    Rect box = contentFrame();
    std::optional<float> w = axis_ == ScrollAxis::Vertical ? std::optional(box.width) : std::nullopt;
    std::optional<float> h = axis_ == ScrollAxis::Horizontal ? std::optional(box.height) : std::nullopt;
    container_->layout({ box.origin, w, h });
}

void ScrollView::addSubview(std::shared_ptr<View> view) {
    // Attached here rather than in the constructor, where weak_from_this() is still empty
    // and the container would be left without a parent for gestures to walk up through.
    if (subviews_.empty()) View::addSubview(container_);
    container_->addSubview(view);
}

void ScrollView::pointerDown(int channel, Point p, double time) {
    container_->offset.setVelocity({0, 0} );
    Point pos = container_->offset;
    container_->offset = pos;
}

void ScrollView::scrollTo(std::shared_ptr<View> view) {
    if (!container_->hasSubview(view)) { return; }
    auto viewCenter = view->frame().center();
    centerAt(viewCenter);
}

void ScrollView::centerAt(Point& p) {
    auto viewport = contentFrame();
    auto viewportCenter = viewport.center();
    auto offset = viewportCenter - p;
    animate(Animation::spring(), [&]{
        container_->offset.update([&](Point& p) {
            if (axis_ != ScrollAxis::Vertical) p.x = clampOffset(offset.x, viewport.width, container_->size().width);
            if (axis_ != ScrollAxis::Horizontal) p.y = clampOffset(offset.y, viewport.height, container_->size().height);
        });
    });
}