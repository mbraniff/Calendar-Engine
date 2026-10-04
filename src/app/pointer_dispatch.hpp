#pragma once
#include "app/graphics.hpp"
#include "app/view/view.hpp"
#include <memory>
#include <vector>

class PointerDispatch {
public:
    PointerDispatch(std::shared_ptr<View> rootView);
    
    void pointerDown(int channel, Point p);
    void pointerMove(int channel, Point p);
    void pointerUp(int channel, Point p);
private:
    void updateClaimed() {
        if (claimed_ != nullptr) return;
        for (auto& r : recognizers_) {
            if (r->hasClaimed()) {
                claimed_ = r;
                cancelProspects();
                return;
            }
        }

    }

    void cancelProspects() {
        for (auto& r : recognizers_) {
            if (r != claimed_) {
                r->forceStop();
            }
        }
    }

    template <typename F>
    void deliver(F&& send) {
        if (!hitView_) return;
        if (claimed()) {
            hitView_->pointerCancelled();
            hitView_.reset();
            return;
        }
        send(*hitView_);
    }

    bool claimed() const {
        return claimed_ != nullptr;
    }

    std::vector<std::shared_ptr<GestureRecognizer>> recognizers_;
    std::shared_ptr<View> hitView_;
    std::shared_ptr<View> root_;
    std::shared_ptr<GestureRecognizer> claimed_;
    int startChannel = -1;
};