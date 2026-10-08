#pragma once
#include "app/graphics.hpp"
#include "app/view/view.hpp"

class Frame : public View {
public:
    FrameOptions options;
    Frame(FrameOptions options) : options(options) {}
protected:
    Size measure(Size proposed) override;
    void layoutSubviews() override;
};