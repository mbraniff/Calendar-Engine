#pragma once
#include "ui/graphics.hpp"
#include "ui/animation/animatable.hpp"
#include "ui/view/view.hpp"
#include <optional>
#include <string_view>

class ImageView : public View {
public:
    ContentMode contentMode = ContentMode::Fit;
    Animation::Animatable<float> aspectRatio = 1.0f;
    Animation::Animatable<Color> color = Color::white();
    Animation::Animatable<float> templatedMode = 0;

    ImageView(std::string_view name);
    ~ImageView();

    float naturalAspectRatio() const;

protected:
    Size measure(std::optional<float> w, std::optional<float> h) override;
    void drawSelf(Renderer& r) override;

private:
    int width_ = 0;
    int height_ = 0;
    unsigned int texture_ = 0;

    Rect placed(Rect box) const;
};
