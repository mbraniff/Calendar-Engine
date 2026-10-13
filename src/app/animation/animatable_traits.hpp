#pragma once
#include "app/graphics.hpp"
#include "app/text/text_layout.hpp"

template <typename T> struct AnimatableTraits;

template <> struct AnimatableTraits<float> {
    static constexpr int N = 1;
    static void split(const float& v, float* out) { out[0] = v; }
    static float join(const float* in) { return in[0]; }
};

template <> struct AnimatableTraits<Point> {
    static constexpr int N = 2;
    static void split(const Point& p, float* out) { out[0] = p.x; out[1] = p.y; }
    static Point join(const float* in) { return { in[0], in[1] }; }
};

template <> struct AnimatableTraits<Rect> {
    static constexpr int N = 4;
    static void split(const Rect& r, float* out) {
        AnimatableTraits<Point>::split(r.origin, out);
        out[2] = r.width; out[3] = r.height;
    }
    static Rect join(const float* in) { return { AnimatableTraits<Point>::join(in), in[2], in[3] }; }
};

template <> struct AnimatableTraits<Color> {
    static constexpr int N = 4;
    static void split(const Color& c, float* out) { out[0] = c.r; out[1] = c.g; out[2] = c.b; out[3] = c.a; }
    static Color join(const float* in) { return { in[0], in[1], in[2], in[3] }; }
};