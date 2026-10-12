#include "app/animation/animator.hpp"
#include "app/animation/animatable.hpp"
#include <unordered_set>

void Animator::add(Animation::AnimatableBase* a) {
    animations_.insert(a);
}

void Animator::remove(Animation::AnimatableBase* a) {
    animations_.erase(a);
}

void Animator::tick(float dt) {
    std::unordered_set<Animation::AnimatableBase*> remove;
    for (auto animation : animations_) {
        if (!animation->tick(dt)) {
            remove.insert(animation);
        };
    }
    for (auto r : remove) {
        animations_.erase(r);
    }
}