#pragma once
#include "app/gestures/gesture.hpp"
#include "app/graphics.hpp"
#include <deque>
#include <functional>

class DragGestureRecognizer : public GestureRecognizer {
public:
    struct DragEvent {
        Point location;
        Size translation;
        Point startLocation;
        Point projectedEnd;
        Size velocity;
        double time;
    };
    using DragCallback = std::function<void(const DragEvent&)>;
    DragGestureRecognizer() {}
    void pointerDown(int, Point, double) override;
    void pointerMoved(int, Point, double) override;
    void pointerUp(int, Point, double) override;

    DragGestureRecognizer& onChange(DragCallback onChange) {
        onChange_ = onChange;
        return *this;
    }
    DragGestureRecognizer& onEnd(DragCallback onEnd) {
        onEnd_ = onEnd;
        return *this;
    }
protected:
    struct Sample { Point p; double time; };

    void addSample(Point p, double time);
    Size velocity() const;

    float slop_ = 10.0f;
    DragCallback onChange_ = [](DragEvent _){};
    DragCallback onEnd_ = [](DragEvent _){};
    PointerEvent lastEvent_;
    Point startPoint_;
    // Recent pointer positions, used to smooth out velocity.
    std::deque<Sample> samples_;
};