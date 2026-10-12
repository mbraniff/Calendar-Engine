#pragma once
#include "app/animation/motion.hpp"
#include <functional>
#include <memory>
#include <utility>

namespace Animation {
    template <typename T>
    class Transaction {
    public:
        using MotionFactory = std::function<std::unique_ptr<Motion<T>>()>;
        Transaction(MotionFactory make) : make_(std::move(make)), previous_(current_) {
            current_ = this;
        };

        ~Transaction() { current_ = previous_; }
        Transaction(const Transaction&) = delete;

        static Transaction* current() { return current_; }
        std::unique_ptr<Motion<T>> makeMotion() const { return make_(); }

    private:
        MotionFactory make_;
        Transaction* previous_;
        static inline thread_local Transaction* current_ = nullptr;
    };
}