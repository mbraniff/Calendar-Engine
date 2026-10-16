#pragma once
#include "ui/view/view.hpp"
#include <algorithm>
#include <memory>
#include <vector>

class ViewController {
public:
    ViewController(std::shared_ptr<View> view) : view_(view) {
        view_->willAppear_ = [this] { willAppear(); };
        view_->didAppear_ = [this] { onScreen_ = true; didAppear(); };
        view_->willLayout_ = [this] { willLayout(); };
        view_->didLayout_ = [this] { didLayout(); };
        view_->didDisappear_ = [this] { onScreen_ = false; didDisappear(); };
        live().push_back(this);
    }

    virtual ~ViewController() {
        // The view can outlive us in the tree; its hooks must not call back into a destroyed controller.
        view_->willAppear_ = nullptr;
        view_->didAppear_ = nullptr;
        view_->willLayout_ = nullptr;
        view_->didLayout_ = nullptr;
        view_->didDisappear_ = nullptr;
        std::erase(live(), this);
    }

    // The hooks capture this, so a controller can't move.
    ViewController(const ViewController&) = delete;
    ViewController& operator=(const ViewController&) = delete;

    std::shared_ptr<View> view() const { return view_; }

    // Ticks every controller whose view is on screen, between didAppear and didDisappear. Call once a frame.
    static void tickAll(float dt) {
        // A tick may create or destroy controllers, so walk a copy and skip any that are gone.
        auto controllers = live();
        for (auto* c : controllers) {
            if (std::find(live().begin(), live().end(), c) == live().end()) continue;
            if (c->onScreen_) c->tick(dt);
        }
    }

protected:
    std::shared_ptr<View> view_;

    virtual void willAppear() {}
    virtual void didAppear() {}
    virtual void willLayout() {}
    virtual void didLayout() {}
    virtual void didDisappear() {}
    virtual void tick(float dt) {}

private:
    bool onScreen_ = false;

    static std::vector<ViewController*>& live() {
        static std::vector<ViewController*> controllers;
        return controllers;
    }
};
