#pragma once
#include "ui/text/text.hpp"
#include <chrono>
#include <cmath>
#include <string>

// Debug FPS counter, averaged over half a second so it's readable. Counts the frames it's drawn in.
class FPSView : public Text {
public:
    FPSView() : Text("FPS --") {
        alignment = Alignment::topTrailing();
        offset = { -8.0f, 8.0f };
        color = Color::green();
    }

    void drawSelf(Renderer& r) override {
        auto now = std::chrono::steady_clock::now();
        frames_++;
        float elapsed = std::chrono::duration<float>(now - windowStart_).count();
        if (elapsed >= 0.5f) {
            text = "FPS " + std::to_string((int)std::round(frames_ / elapsed));
            frames_ = 0;
            windowStart_ = now;
        }
        Text::drawSelf(r);
    }

private:
    int frames_ = 0;
    std::chrono::steady_clock::time_point windowStart_ = std::chrono::steady_clock::now();
};
