#pragma once
#include "app/view/view.hpp"
#include <functional>
#include <memory>

class Button : public View {
public:
    explicit Button(std::shared_ptr<View> content, std::function<void()> action) : action_(action) {
        subviews_.push_back(content);
    }

    Size measure(Size proposed) override;
    void pointerCancelled() override;
    void pointerDown(int, Point, double) override;
    void pointerMoved(int, Point, double) override;
    void pointerUp(int, Point, double) override;
    std::shared_ptr<View> hitTest(Point p) override;
    void setAction(std::function<void()> action) {
        action_ = action;
    }
protected:
    std::function<void()> action_;
    bool isHighlighted = false;
    int trackingChannel = -1;
    float originalOpacity = 1.0f;
};