#include "ui/view/view.hpp"
#include "ui/containers/frame.hpp"
#include "ui/gestures/gesture.hpp"
#include "ui/graphics.hpp"
#include <memory>
#include <vector>

std::shared_ptr<View> View::frame(FrameOptions options) {
    auto frame = std::make_shared<Frame>(options);
    frame->addSubview(shared_from_this());
    return frame;
}

std::shared_ptr<View> View::hitTest(Point p) {
    // Frames don't include offsets, which are applied when drawing; undo ours so p is in our frame's space.
    p = p - offset;
    bool inside = frame_.contains(p);
    if (clipShape && !inside) { return nullptr; }
    for (auto it = subviews_.rbegin(); it != subviews_.rend(); it++) {
        if (auto hit = (*it)->hitTest(p)) { return hit; }
    }
    return inside ? shared_from_this() : nullptr;
}

const std::vector<std::shared_ptr<GestureRecognizer>> View::gestureRecognizers() {
    return gestureRecognizers_;
}

View* View::parent() { return parent_; }

Point View::fromScreen(Point p) {
    for (auto v = this; v; v = v->parent_)
        p = p - v->offset;
    return p;
}

bool View::hasSubview(std::shared_ptr<View> view) {
    return view->hasParent(this);
}

bool View::hasParent(View* parent) {
    for (auto current = this->parent(); current != nullptr; current = current->parent()) {
        if (current == parent) return true;
    }
    return false;
}