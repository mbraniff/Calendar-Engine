#pragma once
#include "app/animation/animatable_traits.hpp"
#include "app/animation/animation_transaction.hpp"
#include "app/animation/animator.hpp"
#include "app/animation/motion.hpp"
#include <array>
#include <cstddef>
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
            if (auto tx = Transaction<T>::current()) {
                for (auto& m : motions_) m = tx->makeMotion();
                Animator::shared().add(this);
            } else {
                presented_ = model_; velocity_ = {};
                for (auto& m : motions_) m.reset();
            }
            return *this;
        }

        template <typename F>
        Animatable& update(F&& change) {
            T v = Tr::join(model_.data());
            change(v);
            return *this = v;
        }

        const T target() const {
            return Tr::join(model_.data());
        }

        operator T() const { return Tr::join(presented_.data()); }

        bool tick(float dt) {
            bool running = false;
            for (size_t i = 0; i < N; i++) {
                if (!motions_[i]) continue;
                if (motions_[i]->step(dt, presented_[i], velocity_[i], model_[i])) running = true;
                else motions_[i].reset();
            }
            return running;
        }

    private:
        std::array<float, N> model_{}, presented_{}, velocity_{};
        std::array<std::unique_ptr<Motion>, N> motions_;
    };
}