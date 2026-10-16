#pragma once
#include "ui/text/text.hpp"
#include "ui/view_controller.hpp"
#include <memory>

// The test views: shapes, text, buttons, gestures and a scroll view, with the red label bouncing around the screen.
class PlaygroundViewController : public ViewController {
public:
    PlaygroundViewController();

protected:
    // Moves the bouncing label.
    void tick(float dt) override;

private:
    std::shared_ptr<Text> bouncer_;
    std::shared_ptr<Text> counterView_;
    int counter_ = 0;
    float dirx_ = 1, diry_ = 1;
};
