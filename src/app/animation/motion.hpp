#pragma once

#include <algorithm>
#include <cmath>
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

  struct EaseIn : Linear {
    using Linear::Linear;

    bool step(float dt, float &value, float &velocity, float target) override {
        if (!started_) { started_ = true; start_ = value; }
        elapsed_ += dt;
        float t = std::min(elapsed_ / time_, 1.0f);
        t *= t;
        float next = start_ + (target - start_) * t;
        velocity = dt  > 0 ? (next - value) / dt : float{};
        value = next;
        return t < 1.0f;
    }
  };

  struct EaseOut : Linear {
    using Linear::Linear;

    bool step(float dt, float &value, float &velocity, float target) override {
        if (!started_) { started_ = true; start_ = value; }
        elapsed_ += dt;
        float t = std::min(elapsed_ / time_, 1.0f);
        t = std::sqrt(t);
        float next = start_ + (target - start_) * t;
        velocity = dt  > 0 ? (next - value) / dt : float{};
        value = next;
        return t < 1.0f;
    }
  };

  // Slows from its starting velocity at the rate that comes to rest exactly on the target.
  struct Decay : Motion {
    bool started_ = false;
    float lambda_ = 0;
    bool step(float dt, float& x, float& v, float target) override {
        if (!started_) {
            started_ = true;
            float d = target - x;
            // Moving away from the target, or nowhere to go: there's no decay that lands there.
            if (d * v <= 0) { x = target; v = 0; return false; }
            lambda_ = v / d;
        }
        v *= std::exp(-lambda_ * dt);
        x += v * dt;
        if (std::abs(v) < 5.0f || (target - x) * v <= 0) { x = target; v = 0; return false; }
        return true;
    }
  };

  struct Spring : Motion {
    float stiffness = 300, damping = 30;
    Spring(float stiffness = 300, float damping = 30) : stiffness(stiffness), damping(damping) {}
    bool step(float dt, float& x, float& v, float target) override {
        v += (-stiffness * (x - target) - damping * v) * dt;
        x += v * dt;
        return std::abs(v) > 0.01f || std::abs(x - target) > 0.01f;
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

  static constexpr auto eastOut(float duration = 0.3f) {
    return [=] { return std::make_unique<EaseOut>(duration); };
  }

  static constexpr auto decay() {
    return []{ return std::make_unique<Decay>(); };
  }

  static constexpr auto spring(float stiffness = 300, float damping = 30) {
    return [=] { return std::make_unique<Spring>(stiffness, damping); };
  }
}