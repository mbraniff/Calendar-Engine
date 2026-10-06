#include "app/gestures/tap_gesture.hpp"
#include "app/gestures/gesture.hpp"
#include "app/graphics.hpp"

void TapGestureRecognizer::pointerDown(int channel, Point p, double time) {
    if (channel != 0) {
        setState(Failed);
        return;
    }
    startPoint = p;
}

void TapGestureRecognizer::pointerMoved(int channel, Point p, double time) {
    if (p.squareDistance(startPoint) > maxDist * maxDist) {
        setState(Failed);
    }
}

void TapGestureRecognizer::pointerUp(int channel, Point p, double time) {
    if (state() != Possible) { return; }
    setState(Ended);
    onTap_(shared_from_this());
}