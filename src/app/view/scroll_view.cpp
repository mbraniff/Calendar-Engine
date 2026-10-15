#include "app/view/scroll_view.hpp"
#include "app/animation/animator.hpp"
#include "app/animation/motion.hpp"
#include "app/gestures/drag_gesture.hpp"
#include "app/graphics.hpp"
#include "app/shapes/shape.hpp"
#include "app/view/view.hpp"
#include <memory>
#include <optional>

ScrollView::ScrollView(ScrollView::ScrollAxis axis) : axis_(axis) {
    container_ = std::make_shared<View>();
    auto weak = std::weak_ptr(container_);
    auto scroller = std::make_shared<DragGestureRecognizer>();
    scroller->onChange([weak] (DragGestureRecognizer::DragEvent e){
        auto view = weak.lock();
        if (!view) return;
        view->offset.update([&](Point& p){
            p.x += e.translation.width;
            p.y += e.translation.height;
        });
        // Update indicators
    })
    .onEnd([weak] (DragGestureRecognizer::DragEvent e){
        auto view = weak.lock();
        if (!view) return;
        // Dragging set the offset directly, which zeroes its velocity; the decay needs the flick's.
        view->offset.setVelocity({ e.velocity.width, e.velocity.height });
        animate(Animation::decay(), [&]() {
            auto distance = e.projectedEnd - e.location;
            view->offset.update([&] (Point& p){
                p.x += distance.x;
                p.y += distance.y;
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