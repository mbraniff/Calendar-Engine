#pragma once
#include "ui/animation/animatable_traits.hpp"
#include "ui/animation/animation_transaction.hpp"
#include "ui/animation/animator.hpp"
#include "ui/animation/motion.hpp"
#include <array>
#include <cstddef>
#include <functional>
#include <memory>

namespace Animation {
    class AnimatableBase {
    public:
        virtual bool tick(float dt) = 0;
    };

    template <typename T>
    class Animatable : public AnimatableBase {
        using Tr = AnimatableTraits<T>;
        static constexpr int N = Tr::N;
    public:
        Animatable(T v = {}) {
            Tr::split(v, model_.data());
            presented_ = model_;
        }
        ~Animatable() { Animator::shared().remove(this); }
        Animatable& operator =(T v) {
            Tr::split(v, model_.data());
            if (auto tx = Transaction::current()) {
                for (auto& m : motions_) m = tx->makeMotion();
                Animator::shared().add(this);
            } else {
                auto before = presented_;
                presented_ = model_; velocity_ = {};
                for (auto& m : motions_) m.reset();
                if (presented_ != before && onChange_) onChange_();
            }
            return *this;
        }

        template <typename F>
        Animatable& update(F&& change) {
            T v = Tr::join(model_.data());
            change(v);
            return *this = v;
        }

        Animatable& setVelocity(T v) {
            Tr::split(v, velocity_.data());
            return *this;
        }

        const T target() const {
            return Tr::join(model_.data());
        }

        operator T() const { return Tr::join(presented_.data()); }

        // Called whenever the presented value moves: on an assignment outside animate(), and on every animation
        // step after one inside it. For values read in layout, which otherwise only sees changes it's told about.
        // It runs while the Animator ticks, so it mustn't start animations.
        void onChange(std::function<void()> onChange) {
            onChange_ = std::move(onChange);
        }

        bool tick(float dt) {
            auto before = presented_;
            bool running = false;
            for (size_t i = 0; i < N; i++) {
                if (!motions_[i]) continue;
                if (motions_[i]->step(dt, presented_[i], velocity_[i], model_[i])) running = true;
                else motions_[i].reset();
            }
            if (presented_ != before && onChange_) onChange_();
            return running;
        }

    private:
        std::array<float, N> model_{}, presented_{}, velocity_{};
        std::array<std::unique_ptr<Motion>, N> motions_;
        std::function<void()> onChange_;
    };
}