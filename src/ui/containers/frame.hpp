#pragma once
#include "ui/graphics.hpp"
#include "ui/view/view.hpp"
#include <optional>

class Frame : public View {
public:
    FrameOptions options;
    Frame(FrameOptions options) : options(options) {}
protected:
    Size measure(std::optional<float> w, std::optional<float> h) override;
    void layoutSubviews() override;
};