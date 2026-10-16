#pragma once
#include "ui/animation/animatable.hpp"
#include "ui/view/view.hpp"
#include <optional>
#include <utility>

class ShapeView : public View {
public:
    std::shared_ptr<Shape> shape;
    Animation::Animatable<Color> fill = Color::white();
    // Strokes the shape this wide, inside the frame, on top of the fill.
    std::optional<float> strokeWidth;
    Animation::Animatable<Color> strokeColor = Color::black();

    ShapeView(std::shared_ptr<Shape> shape, Color fill = Color::white())
        : shape(std::move(shape)), fill(fill) {}

protected:
    void drawSelf(Renderer& r) override;

private:
    Mask fillMask_;
    Mask strokeMask_;
};