#include "app/containers/vstack.hpp"
#include "app/graphics.hpp"
#include <cstddef>

void VStack::layoutSubviews() {
    float y = frame_.origin.y;
    for (auto& child : subviews_) {
        Size s = child->sizeThatFits({frame_.width, frame_.height});

        float x = frame_.origin.x + (frame_.width - s.width) * layoutAlignment.x;
        
        child->layout(Rect { { x, y }, s.width, s.height});
        y += s.height + spacing;
    }
}

Size VStack::measure(Size proposal) {
    Size size{0, 0};
    float remaining = proposal.height;
    for (size_t i = 0; i < subviews_.size(); i++) {
        Size s = subviews_[i]->sizeThatFits({ proposal.width, remaining });
        size.width = std::max(s.width, size.width);
        size.height += s.height;
        if (i + 1 < subviews_.size()) size.height += spacing;
        remaining = std::max(0.0f, proposal.height - size.height);
    }
    return size;
}