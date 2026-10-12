#pragma once
#include "app/animation/animation_transaction.hpp"
#include "app/animation/animator.hpp"
#include "app/animation/motion.hpp"
#include <memory>

namespace Animation {
    class AnimatableBase {
    public:
        virtual bool tick(float dt) = 0;
    };

    template <typename T>
    class Animatable : public AnimatableBase {
    public:
        Animatable(T v = {}) : model_(v), presented_(v) {}
        ~Animatable() { Animator::shared().remove(this); }
        Animatable& operator =(T v) {
            model_ = v;
            if (auto tx = Transaction<T>::current()) {
                motion_ = tx->makeMotion();
                Animator::shared().add(this);
            } else {
                presented_ = v; velocity_ = {};
                motion_.reset();
            }
            return *this;
        }

        operator T() const { return presented_; }

        bool tick(float dt) {
            if (motion_ == nullptr) { return false; }
            return motion_->step(dt, presented_, velocity_, model_);
        }

    private:
        T model_, presented_, velocity_{};
        std::unique_ptr<Motion<T>> motion_;
    };
}