#pragma once

struct Size {
    float width = 0;
    float height = 0;
};

struct Point {
    float x = 0, y = 0;
};

struct Rect {
    Point origin;
    float width = 0, height = 0;

    Point center() { return { origin.x + width / 2, origin.y + height / 2 }; }
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