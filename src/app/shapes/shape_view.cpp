#include "app/shapes/shape_view.hpp"

void ShapeView::drawSelf(Renderer& r) {
    r.drawShape(shape->pathIn(frame_), fill, fillMask_);
}