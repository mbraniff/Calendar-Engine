#pragma once
#include "ui/graphics.hpp"
#include "ui/view/view.hpp"
#include <algorithm>
#include <numeric>
#include <vector>

class Stack: public View {
public:
    enum Axis { Vertical, Horizontal };
    float spacing = 8;
    Alignment layoutAlignment = Alignment::center();

    Stack(float spacing = 8, const Alignment alignment = Alignment::center())
        : spacing(spacing), layoutAlignment(alignment) {}
protected:
    std::vector<Size> childSizes(std::optional<float> w, std::optional<float> h, Axis axis) {
        size_t n = subviews_.size();
        std::vector<Size> sizes(n);
        if (n == 0) return sizes;

        const float inf = std::numeric_limits<float>::infinity();
        std::vector<float> flex(n);
        for (size_t i = 0; i < n; ++i) {
            Size lo = subviews_[i]->sizeThatFits(axis == Axis::Vertical ? w : 0, axis == Axis::Vertical ? 0 : h);
            Size hi = subviews_[i]->sizeThatFits(axis == Axis::Vertical ? w : inf, axis == Axis::Vertical ? inf : h);
            flex[i] = axis == Axis::Vertical ? hi.height - lo.height : hi.width - lo.width;
        }

        std::vector<size_t> order(n);
        std::iota(order.begin(), order.end(), 0);
        std::stable_sort(order.begin(), order.end(),
                        [&](size_t a, size_t b) { return flex[a] < flex[b]; });

        float remaining = std::max(0.0f, (axis == Axis::Vertical ? h.value_or(infinity) : w.value_or(infinity)) - spacing * (n - 1));
        size_t left = n;
        for (size_t i : order) {
            float width = axis == Axis::Vertical ? w.value_or(infinity) : remaining / left;
            float height = axis == Axis::Vertical ? remaining / left : h.value_or(infinity);
            Size s = subviews_[i]->sizeThatFits(width, height);
            sizes[i] = s;
            remaining = std::max(0.0f, remaining - (axis == Axis::Vertical ? s.height : s.width));
            left--;
        }
        return sizes;
    }
};