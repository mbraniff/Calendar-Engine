#pragma once
#include "ui/view/view.hpp"
#include "ui/graphics.hpp"
#include <functional>
#include <memory>

class View;

class GestureRecognizer : public std::enable_shared_from_this<GestureRecognizer> {
public:
    using Callback = std::function<void(GestureRecognizer&)>;
    enum State { Possible, Began, Changed, Ended, Cancelled, Failed, };

    const State state() { return state_; }

    bool isLive() const {
        return state_ == Possible
            || state_ == Began
            || state_ == Changed;
    }

    bool hasClaimed() const {
        return state_ == Began
            || state_ == Changed
            || state_ == Ended;
    }

    virtual void pointerDown(int, Point, double) {}
    virtual void pointerMoved(int, Point, double) {}
    virtual void pointerUp(int, Point, double) {}

    void forceStop() {
        if (state_ == Possible) state_ = Failed;
        else if (state_ == Began || state_ == Changed) state_ = Cancelled;
    }

    void reset() { state_ = Possible; }
    void setView(std::weak_ptr<View> view) { view_ = view; }

    Callback onStateChange;
protected:
    void setState(State state) {
        state_ = state;
        if (state != Possible && state != Failed && onStateChange) {
            onStateChange(*this);
        }
    }
    virtual void onReset() {}
private:
    std::weak_ptr<View> view_;
    State state_ = Possible;
};