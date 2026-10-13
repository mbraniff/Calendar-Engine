#pragma once
#include "app/animation/animatable.hpp"
#include "app/view/view.hpp"
#include <utility>

class ShapeView : public View {
public:
    std::shared_ptr<Shape> shape;
    Animation::Animatable<Color> fill = Color::white();

    ShapeView(std::shared_ptr<Shape> shape, Color fill = Color::white())
        : shape(std::move(shape)), fill(fill) {}

protected:
    void drawSelf(Renderer& r) override;

private:
    Mask fillMask_;
};