#include "ui/shapes/shape_view.hpp"

void ShapeView::drawSelf(Renderer& r) {
    Rect box = contentFrame();
    Color f = fill;
    if (f.a > 0) r.drawShape(shape->pathIn(box), f, fillMask_);
    // Insetting by half the width keeps the whole stroke inside the frame, so a parent's clip can't shave it.
    if (strokeWidth) r.drawShape(shape->pathIn(box.inset(*strokeWidth / 2)), strokeColor, strokeMask_, strokeWidth);
}

std::shared_ptr<View> Shape::fill(Color color) const {
    return std::make_shared<ShapeView>(clone(), color);
}

std::shared_ptr<View> Shape::stroke(Color color, float width) const {
    auto view = std::make_shared<ShapeView>(clone(), Color::clear());
    view->strokeWidth = width;
    view->strokeColor = color;
    return view;
}
