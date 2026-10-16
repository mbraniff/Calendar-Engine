#include "ui/containers/hstack.hpp"
#include "ui/graphics.hpp"
#include <cstddef>
#include <optional>

Size HStack::measure(std::optional<float> w, std::optional<float> h) {
    auto sizes = childSizes(w, h, Axis::Horizontal);
    Size total{ 0, 0 };
    for (auto& s : sizes) {
        total.height = std::max(total.height, s.height);
        total.width += s.width;
    }
    if (!sizes.empty()) total.width += spacing * (sizes.size() - 1);
    return total;
}

void HStack::layoutSubviews() {
    Rect content = contentFrame();
    auto sizes = childSizes(content.width, content.height, Axis::Horizontal);
    float x = content.origin.x;
    for (size_t i = 0; i < subviews_.size(); i++) {
        Size s = sizes[i];
        float y = content.origin.y + (content.height - s.height) * layoutAlignment.y;
        subviews_[i]->layout({ { x, y }, s.width, s.height });
        x += s.width + spacing;
    }
}