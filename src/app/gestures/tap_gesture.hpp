#pragma once
#include "app/gestures/geature.hpp"
#include "app/view/view.hpp"
#include <functional>
#include <memory>

class TapGestureRecognizer : public GestureRecognizer {
public:
    TapGestureRecognizer();
    void setView(std::weak_ptr<View> view);
private:
    using TapCallback = std::function<void(std::shared_ptr<TapGestureRecognizer> sender)>;
    TapCallback onTap_;
};