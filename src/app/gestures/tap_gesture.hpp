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

    void pointerDown(int, Point, double) override;
    void pointerMoved(int, Point, double) override;
    void pointerUp(int, Point, double) override;
private:
    TapCallback onTap_;
    float maxDist = 10.0f;
    Point startPoint;
};