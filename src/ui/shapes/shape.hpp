#pragma once
#include "ui/graphics.hpp"
#include "ui/shapes/path.hpp"
#include <algorithm>
#include <memory>

class View;

// Distance from a quarter circle's end point to its cubic control point, per unit of radius.
inline constexpr float kCircleKappa = 0.5522847498f;

struct Shape {
    virtual ~Shape() = default;
    virtual Path pathIn(Rect rect) const = 0;
    virtual std::unique_ptr<Shape> clone() const = 0;

    // A view of this shape filled with color, e.g. background = Rectangle().fill(Colors::cell).
    std::shared_ptr<View> fill(Color color) const;
    // A view of this shape's outline, width wide and inside its frame.
    std::shared_ptr<View> stroke(Color color, float width) const;
};

struct Rectangle : public Shape {
    std::unique_ptr<Shape> clone() const override { return std::make_unique<Rectangle>(*this); }
    Path pathIn(Rect rect) const override {
        Path result;
        result.moveTo({ rect.origin.x, rect.origin.y });
        result.lineTo({rect.origin.x, rect.origin.y + rect.height});
        result.lineTo({rect.origin.x + rect.width, rect.origin.y + rect.height});
        result.lineTo({rect.origin.x + rect.width, rect.origin.y});
        result.close();
        return result;
    }
};

struct Circle : public Shape {
    std::unique_ptr<Shape> clone() const override { return std::make_unique<Circle>(*this); }
    Path pathIn(Rect rect) const override {
        Point c = rect.center();
        float r = std::min(rect.width, rect.height) / 2;
        float k = kCircleKappa * r;   // how far each control point sits from its end point

        Path p;
        p.moveTo({ c.x, c.y - r });                                                // top
        p.cubicTo({ c.x - k, c.y - r }, { c.x - r, c.y - k }, { c.x - r, c.y });   // → left
        p.cubicTo({ c.x - r, c.y + k }, { c.x - k, c.y + r }, { c.x, c.y + r });   // → bottom
        p.cubicTo({ c.x + k, c.y + r }, { c.x + r, c.y + k }, { c.x + r, c.y });   // → right
        p.cubicTo({ c.x + r, c.y - k }, { c.x + k, c.y - r }, { c.x, c.y - r });   // → top
        p.close();
        return p;
    }
};

struct RoundedRectangle : public Shape {
    std::unique_ptr<Shape> clone() const override { return std::make_unique<RoundedRectangle>(*this); }
    float cornerRadius;

    explicit RoundedRectangle(float cornerRadius) : cornerRadius(cornerRadius) {}

    Path pathIn(Rect rect) const override {
        float l = rect.origin.x, t = rect.origin.y;
        float r = l + rect.width, b = t + rect.height;
        float rad = std::clamp(cornerRadius, 0.0f, std::min(rect.width, rect.height) / 2);
        float k = kCircleKappa * rad;

        Path p;
        p.moveTo({ l + rad, t });
        p.lineTo({ r - rad, t });                                                  // top edge
        p.cubicTo({ r - rad + k, t }, { r, t + rad - k }, { r, t + rad });        // top-right
        p.lineTo({ r, b - rad });                                                  // right edge
        p.cubicTo({ r, b - rad + k }, { r - rad + k, b }, { r - rad, b });        // bottom-right
        p.lineTo({ l + rad, b });                                                  // bottom edge
        p.cubicTo({ l + rad - k, b }, { l, b - rad + k }, { l, b - rad });        // bottom-left
        p.lineTo({ l, t + rad });                                                  // left edge
        p.cubicTo({ l, t + rad - k }, { l + rad - k, t }, { l + rad, t });        // top-left
        p.close();
        return p;
    }
};

struct Capsule : public Shape {
    std::unique_ptr<Shape> clone() const override { return std::make_unique<Capsule>(*this); }
    Path pathIn(Rect rect) const override {
        return RoundedRectangle(std::min(rect.width, rect.height) / 2).pathIn(rect);
    }
};
