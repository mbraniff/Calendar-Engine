#pragma once

#include <algorithm>
#include <functional>
#include <memory>

namespace Animation {
    struct Motion {
    public:
        virtual ~Motion() = default;
        virtual bool step(float dt, float& value, float& velocity, float target) = 0;
    };

    struct Linear : Motion {
        float time_ = 0.3f, elapsed_ = 0.0f;
        bool started_ = false;
        float start_;
    public:
        Linear(float time = 0.3f) : time_(time) {}

        bool step(float dt, float& value, float& velocity, float target) override {
            if (!started_) { started_ = true; start_ = value; }
            elapsed_ += dt;
            float t = std::min(elapsed_ / time_, 1.0f);
            float next = start_ + (target - start_) * t;
            velocity = dt > 0 ? (next - value) / dt : float{};
            value = next;
            return t < 1.0f;
        }
    };

    struct AutoReverse : Motion {
      using Factory = std::function<std::unique_ptr<Motion>()>;

      // cycles: number of there-and-back trips; 0 = forever.
      AutoReverse(Factory make, int cycles = 1) : make_(std::move(make)), remaining_(cycles) {}

      bool step(float dt, float& value, float& velocity, float target) override {
          if (!inner_) { origin_ = value; inner_ = make_(); }

          float goal = forward_ ? target : origin_;
          if (inner_->step(dt, value, velocity, goal)) return true;

          // This leg finished. Coming back completes a cycle.
          if (!forward_ && remaining_ > 0 && --remaining_ == 0) return false;
          forward_ = !forward_;
          inner_ = make_();   // fresh motion for the next leg, since motions keep state
          return true;
      }

  private:
      Factory make_;
      std::unique_ptr<Motion> inner_;
      float origin_{};
      bool forward_ = true;
      int remaining_;
  };

  struct EaseIn : Motion {
    float time_ = 0.3f, elapsed = 0.0f;
    bool started_ = false;
    float start_;

    EaseIn(float duration) : time_(duration) {}

    bool step(float dt, float &value, float &velocity, float target) override {
        if (!started_) { started_ = true; start_ = value; }
        elapsed += dt;
        float t = std::min(elapsed / time_, 1.0f);
        t *= t;
        float next = start_ + (target - start_) * t;
        velocity = dt  > 0 ? (next - value) / dt : float{};
        value = next;
        return t < 1.0f;
    }
  };

  static constexpr auto linear(float duration = 0.3f) {
    return [=] { return std::make_unique<Linear>(duration); };
  }

  template <typename F>
  static constexpr auto autoreverse(F&& make, int count = 0) {
    return [=] { return std::make_unique<AutoReverse>(make, count); };
  }

  static constexpr auto easeIn(float duration = 0.3f) {
    return [=] { return std::make_unique<EaseIn>(duration); };
  }
}