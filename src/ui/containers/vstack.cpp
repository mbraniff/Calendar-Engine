#include "ui/containers/vstack.hpp"
#include "ui/graphics.hpp"
#include <cstddef>

Size VStack::measure(std::optional<float> w, std::optional<float> h) {
    auto sizes = childSizes(w, h, Axis::Vertical);
    Size total{ 0, 0 };
    for (auto& s : sizes) {
        total.width = std::max(total.width, s.width);
        total.height += s.height;
    }
    if (!sizes.empty()) total.height += spacing * (sizes.size() - 1);
    return total;
}

void VStack::layoutSubviews() {
    Rect content = contentFrame();
    auto sizes = childSizes(content.width, content.height, Axis::Vertical);
    float y = content.origin.y;
    for (size_t i = 0; i < subviews_.size(); ++i) {
        Size s = sizes[i];
        float x = content.origin.x + (content.width - s.width) * layoutAlignment.x;
        subviews_[i]->layout({ { x, y }, s.width, s.height });
        y += s.height + spacing;
    }
}