#include "app/containers/frame.hpp"
#include "app/graphics.hpp"
#include <algorithm>

Size Frame::measure(Size proposal) {
    float childWidth = 0;
    float childHeight = 0;
    for (auto child : subviews_) {
        Size cS = child->sizeThatFits(proposal);
        childWidth = std::max(cS.width, childWidth);
        childHeight = std::max(cS.height, childHeight);
    } 
    Size s{0, 0};
    s.width = options.width.has_value() ? options.width.value() : childWidth;
    s.height = options.height.has_value() ? options.height.value() : childHeight;

    if (options.width.has_value() || options.height.has_value()) return s;
    s.width = options.minWidth.has_value() ? std::max(s.width, options.minWidth.value()) : s.width;
    s.height = options.minHeight.has_value() ? std::max(s.height, options.minHeight.value()) : s.height;
    s.width = options.maxWidth.has_value() ? std::min(proposal.width, options.maxWidth.value()) : s.width;
    s.height = options.maxHeight.has_value() ? std::min(proposal.height, options.maxHeight.value()) : s.height;
    return s;
}

void Frame::layoutSubviews() {
    Rect box = contentFrame();
    for (auto& child : subviews_) {
        Size s = child->sizeThatFits({ box.width, box.height });
        s.width = std::min(s.width, box.width);
        s.height = std::min(s.height, box.height);
        child->layout({ { box.origin.x + (box.width - s.width) * options.alignment.x,
                        box.origin.y + (box.height - s.height) * options.alignment.y },
                        s.width, s.height });
    }
}