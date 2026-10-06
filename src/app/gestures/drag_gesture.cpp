#include "app/gestures/drag_gesture.hpp"
#include "app/gestures/gesture.hpp"
#include "app/graphics.hpp"

void DragGestureRecognizer::pointerDown(int channel, Point p, double time) {
    if (channel != 0) {
        setState(Cancelled);
        return;
    }
    startPoint_ = p;
    lastEvent_ = PointerEvent{channel, p, Down, time};
    samples_.clear();
    addSample(p, time);
}

void DragGestureRecognizer::pointerMoved(int channel, Point p, double time) {
    addSample(p, time);
    float distance = p.squareDistance(startPoint_);
    if (state() == Possible && distance <= slop_ * slop_) {
        lastEvent_ = PointerEvent{channel, p, Move, time};
        return;
    }
    if (state() == Possible) {
        setState(Began);
    } else if (state() == Began) {
        setState(Changed);
    }
    float xDist = p.x - lastEvent_.p.x, yDist = p.y - lastEvent_.p.y;
    onChange_(DragEvent{p,
         { xDist, yDist },
        startPoint_,
        velocity(),
    time});
    lastEvent_ = PointerEvent{channel, p, Move, time};
}

void DragGestureRecognizer::pointerUp(int channel, Point p, double time) {
    if (state() == Possible) {
        setState(Failed);
        return;
    }
    addSample(p, time);
    float xDist = p.x - lastEvent_.p.x, yDist = p.y - lastEvent_.p.y;
    onEnd_(DragEvent{p,
         { xDist, yDist },
        startPoint_,
        velocity(),
    time});
}

void DragGestureRecognizer::addSample(Point p, double time) {
    samples_.push_back({p, time});
    // Keep about 100ms of history, but always at least two samples.
    while (samples_.size() > 2 && time - samples_.front().time > 0.1) {
        samples_.pop_front();
    }
}

Size DragGestureRecognizer::velocity() const {
    if (samples_.size() < 2) return {0, 0};
    const auto& first = samples_.front();
    const auto& last = samples_.back();
    double elapsed = last.time - first.time;
    // Events delivered in the same poll can be microseconds apart; dividing by that explodes.
    if (elapsed < 0.005) return {0, 0};
    return { float((last.p.x - first.p.x) / elapsed), float((last.p.y - first.p.y) / elapsed) };
}