#include "app/shapes/path.hpp"
#include "app/graphics.hpp"
#include <limits>

void Path::moveTo(Point p) {
    points.push_back(p);
    verbs.push_back(Verb::Move);
}

void Path::lineTo(Point p) {
    points.push_back(p);
    verbs.push_back(Verb::Line);
}

void Path::quadTo(Point ctrl, Point end) {
    points.push_back(ctrl);
    points.push_back(end);
    verbs.push_back(Verb::Quad);
}

void Path::cubicTo(Point c1, Point c2, Point end) {
    points.push_back(c1);
    points.push_back(c2);
    points.push_back(end);
    verbs.push_back(Verb::Cubic);
}

void Path::close() {
    verbs.push_back(Verb::Close);
}

Rect Path::bounds() const {
    float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::lowest();
    float minY = minX, maxY = maxX;
    for (Point point : points) {
        minX = std::min(point.x, minX);
        maxX = std::max(point.x, maxX);
        minY = std::min(point.y, minY);
        maxY = std::max(point.y, maxY);
    }

    return Rect{{minX, minY}, maxX - minX, maxY - minY};
}