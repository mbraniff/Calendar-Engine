#include "app/view/view.hpp"
#include "app/containers/frame.hpp"
#include "app/gestures/geature.hpp"
#include "app/graphics.hpp"
#include <memory>
#include <vector>

std::shared_ptr<View> View::frame(FrameOptions options) {
    auto frame = std::make_shared<Frame>(options);
    frame->addSubview(shared_from_this());
    return frame;
}

std::shared_ptr<View> View::hitTest(Point p) {
    if (!frame_.contains(p)) { return nullptr; }
    for (auto it = subviews_.rbegin(); it != subviews_.rend(); it++) {
        if (auto hit = (*it)->hitTest(p)) { return hit; }
    }
    return shared_from_this();
}

const std::vector<std::shared_ptr<GestureRecognizer>> View::gestureRecognizers() {
    return gestureRecognizers_;
}

std::weak_ptr<View> View::parent() { return parent_; }