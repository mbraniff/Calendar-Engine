#include "pointer_dispatch.hpp"
#include "ui/graphics.hpp"
#include "ui/view/view.hpp"
#include <memory>

PointerDispatch::PointerDispatch(std::shared_ptr<View> rootView) {
    root_ = rootView;
}

void PointerDispatch::pointerDown(int channel, Point p, double time) {
    if (startChannel == channel || startChannel == -1) {
        recognizers_.clear();
        startChannel = channel;
        hitView_ = root_->hitTest(p);
        for(auto v = hitView_.get(); v; v = v->parent()) {
            for(auto& r : v->gestureRecognizers()) {
                recognizers_.push_back(r);
            }
        }
    }
    for(auto& r : recognizers_) {
        r->pointerDown(channel, p, time);
    }
    updateClaimed();
    deliver([&](View& v){ v.pointerDown(channel, p, time); });
}

void PointerDispatch::pointerMove(int channel, Point p, double time) {
    for(auto& r : recognizers_) {
        if (r->isLive()) {
            r->pointerMoved(channel, p, time);
        }
    }
    updateClaimed();
    deliver([&](View& v){ v.pointerMoved(channel, p, time); });
}

void PointerDispatch::pointerUp(int channel, Point p, double time) {
    for(auto& r : recognizers_) {
        if (r->isLive()) {
             r->pointerUp(channel, p, time);
        }
    }
    updateClaimed();
    deliver([&](View& v){ v.pointerUp(channel, p, time); });
    if (startChannel == channel) {
        claimed_ = nullptr;
        hitView_ = nullptr;
        startChannel = -1;
    }
    resetRecognizers();
}

void PointerDispatch::resetRecognizers() {
    for (auto& r : recognizers_) {
        r->reset();
    }
}