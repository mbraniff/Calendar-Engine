#pragma once
#include <limits>
#include <optional>

inline constexpr float infinity = std::numeric_limits<float>::infinity();

enum PointerAction {
    Down,
    Move,
    Up
};

struct Size {
    float width = 0;
    float height = 0;
};

struct Point {
    float x = 0, y = 0;
    bool operator==(const Point&) const = default;
    float squareDistance(const Point& to) {
        float xd = x - to.x;
        float yd = y - to.y;
        return xd * xd + yd * yd;
    }

    Point operator-(const Point& r) const {
        return { x - r.x, y - r.y };
    }
};

struct Proposal {
    Point origin;
    std::optional<float> width;
    std::optional<float> height;
};

struct PointerEvent {
    int channel = -1;
    Point p;
    PointerAction action = Down;
    double time = 0;
};

struct Rect {
    Point origin;
    float width = 0, height = 0;

    Point center() { return { origin.x + width / 2, origin.y + height / 2 }; }
    bool contains(Point p) {
        return p.x >= origin.x && p.x <= origin.x + width
            && p.y >= origin.y && p.y <= origin.y + height;
    }

    Proposal to_proposal() {
        return { origin, width, height };
    }
};

struct Color {
    float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
    const Color withAlpha(float alpha) const { return {r, g, b, alpha }; }
    static constexpr Color white() { return { 1.0f, 1.0f, 1.0f }; }
    static constexpr Color black() { return { 0.0f, 0.0f, 0.0f }; }
    static constexpr Color red() { return { 1.0f, 0.0f, 0.0f }; }
    static constexpr Color green() { return { 0.0f, 1.0f, 0.0f }; }
    static constexpr Color blue() { return { 0.0f, 0.0f, 1.0f }; }
};

struct Alignment {
    // y-down: top is 0, bottom is 1.
    float x = 0.5f, y = 0.5f;
    static constexpr Alignment topLeading() { return { 0.0f, 0.0f }; }
    static constexpr Alignment topCenter() { return { 0.5f, 0.0f }; }
    static constexpr Alignment topTrailing() { return { 1.0f, 0.0f }; }
    static constexpr Alignment centerLeading() { return { 0.0f, 0.5f }; }
    static constexpr Alignment center() { return { 0.5f, 0.5f }; }
    static constexpr Alignment centerTrailing() { return { 1.0f, 0.5f }; }
    static constexpr Alignment bottomLeading() { return { 0.0f, 1.0f }; }
    static constexpr Alignment bottomCenter() { return { 0.5f, 1.0f }; }
    static constexpr Alignment bottomTrailing() { return { 1.0f, 1.0f }; }
};

struct Padding {
    enum Edge { Left, Top, Right, Bottom, Horizontal, Vertical, All};
    float left = 0, top = 0, right = 0, bottom = 0;

    Padding(float value) : left(value), top(value), right(value), bottom(value) {}

    void set(Edge edge, float value) {
        switch (edge) {
        case Edge::Left: left = value; break;
        case Edge::Top: top = value; break;
        case Edge::Right: right = value; break;
        case Edge::Bottom: bottom = value; break;
        case Edge::Horizontal: left = right = value; break;
        case Edge::Vertical: top = bottom = value; break;
        case Edge::All: left = right = top = bottom = value; break;
        }
    }
};

struct FrameOptions {
    std::optional<float> width, height;
    std::optional<float> minWidth, maxWidth, minHeight, maxHeight;
    Alignment alignment = Alignment::center();
};