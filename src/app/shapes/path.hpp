#pragma once
#include "app/graphics.hpp"
#include <vector>

struct Path {
    enum class Verb { Move, Line, Quad, Cubic, Close };
    std::vector<Verb> verbs;
    std::vector<Point> points;

    void moveTo(Point p);
    void lineTo(Point p);
    void quadTo(Point ctrl, Point end);
    void cubicTo(Point c1, Point c2, Point end);
    void close();

    Rect bounds() const;

    bool operator==(const Path&) const = default;
};