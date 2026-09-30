#pragma once
#include "app/graphics.hpp"
#include "app/shapes/path.hpp"

struct Shape {
    virtual ~Shape() = default;
    virtual Path pathIn(Rect rect) const = 0;
};

struct Rectangle : public Shape {
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