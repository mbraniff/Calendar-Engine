#include "app/button/button.hpp"
#include "app/animation/animator.hpp"
#include "app/animation/motion.hpp"
#include "app/graphics.hpp"
#include "app/view/view.hpp"
#include <memory>

void Button::pointerCancelled() {
    isHighlighted = false;
    trackingChannel = -1;
    animate<float>(Animation::easeIn(), [&]{
        subviews_.front()->opacity = originalOpacity;
    });
    originalOpacity = 1.0f;
}

void Button::pointerDown(int channel, Point p, double time) {
    if (trackingChannel != channel && trackingChannel != -1) return;
    isHighlighted = contentFrame().contains(p);
    trackingChannel = channel;
    originalOpacity = subviews_.front()->opacity.target();
    subviews_.front()->opacity = isHighlighted ? originalOpacity * 0.4 : originalOpacity;
}

void Button::pointerMoved(int channel, Point p, double time) {
    if (trackingChannel != channel) return;
    bool inside = contentFrame().contains(p);
    if (isHighlighted == inside) return;
    isHighlighted = inside;
    animate<float>(Animation::easeIn(), [&]{
        subviews_.front()->opacity = isHighlighted ? originalOpacity * 0.4 : originalOpacity;
    });
}

void Button::pointerUp(int channel, Point p, double time) {
    if (trackingChannel != channel) return;
    if (isHighlighted) {
        action_();
    }
    pointerCancelled();
}

Size Button::measure(Size proposal) {
    return subviews_.front()->sizeThatFits(proposal);
}

std::shared_ptr<View> Button::hitTest(Point p) {
    return frame_.contains(p) ? shared_from_this() : nullptr;
}