#pragma once
#include "app/graphics.hpp"
#include "app/view/view.hpp"

class Frame : public View {
public:
    Frame(FrameOptions options) : options_(options) {}
protected:
    Size measure(Size proposed) override;
    void layoutSubviews() override;
    FrameOptions options_;
};