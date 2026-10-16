#include "ui/shapes/shape_view.hpp"

void ShapeView::drawSelf(Renderer& r) {
    r.drawShape(shape->pathIn(contentFrame()), fill, fillMask_);
}