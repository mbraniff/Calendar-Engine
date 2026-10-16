#pragma once
#include "ui/animation/animation_transaction.hpp"
#include <unordered_set>

namespace Animation {class AnimatableBase; }

class Animator {
    Animator() {}
public:
    static Animator& shared() {
        static Animator value = Animator();
        return value;
    }

    void tick(float dt);
    void add(Animation::AnimatableBase* a);
    void remove(Animation::AnimatableBase* a);

private:
    std::unordered_set<Animation::AnimatableBase*> animations_;
};

template <typename F>
static void animate(typename Animation::MotionFactory make, F&& changes) {
    auto tx = Animation::Transaction(make);
    changes();
}