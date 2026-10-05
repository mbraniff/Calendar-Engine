#pragma once
#include "app/gestures/gesture.hpp"
#include "app/graphics.hpp"
#include "app/view/view.hpp"
#include <functional>
#include <memory>

class TapGestureRecognizer : public GestureRecognizer {
public:
    using TapCallback = std::function<void(std::shared_ptr<GestureRecognizer> sender)>;
    TapGestureRecognizer(TapCallback onTap) : onTap_(onTap) {}

    void setOnTap(TapCallback onTap) {
        onTap_ = onTap;
    }

    void pointerDown(int, Point) override;
    void pointerMoved(int, Point) override;
    void pointerUp(int, Point) override;
private:
    TapCallback onTap_;
    float maxDist = 5.0f;
    Point startPoint;
};