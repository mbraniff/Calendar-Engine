#pragma once

#include <algorithm>
#include <functional>
#include <memory>
namespace Animation {
    template <typename T>
    struct Motion {
    public:
        virtual ~Motion() = default;
        virtual bool step(float dt, T& value, T& velocity, T target) = 0;
    };

    template <typename T>
    struct Linear : Motion<T> {
        float time_ = 0.3f, elapsed_ = 0.0f;
        bool started_ = false;
        T start_;
    public:
        Linear(float time = 0.3f) : time_(time) {}

        bool step(float dt, T& value, T& velocity, T target) override {
            if (!started_) { started_ = true; start_ = value; }
            elapsed_ += dt;
            float t = std::min(elapsed_ / time_, 1.0f);
            T next = start_ + (target - start_) * t;
            velocity = dt > 0 ? (next - value) / dt : T{};
            value = next;
            return t < 1.0f;
        }
    };

      template <typename T>
  struct AutoReverse : Motion<T> {
      using Factory = std::function<std::unique_ptr<Motion<T>>()>;

      // cycles: number of there-and-back trips; 0 = forever.
      AutoReverse(Factory make, int cycles = 1) : make_(std::move(make)), remaining_(cycles) {}

      bool step(float dt, T& value, T& velocity, T target) override {
          if (!inner_) { origin_ = value; inner_ = make_(); }

          T goal = forward_ ? target : origin_;
          if (inner_->step(dt, value, velocity, goal)) return true;

          // This leg finished. Coming back completes a cycle.
          if (!forward_ && remaining_ > 0 && --remaining_ == 0) return false;
          forward_ = !forward_;
          inner_ = make_();   // fresh motion for the next leg, since motions keep state
          return true;
      }

  private:
      Factory make_;
      std::unique_ptr<Motion<T>> inner_;
      T origin_{};
      bool forward_ = true;
      int remaining_;
  };

  template <typename T>
  static constexpr auto linear(float duration = 0.3f) {
    return [=] { return std::make_unique<Linear<T>>(duration); };
  }

  template <typename T, typename F>
  static constexpr auto autoreverse(F&& make, int count = 0) {
    return [=] { return std::make_unique<AutoReverse<T>>(make, count); };
  }
}