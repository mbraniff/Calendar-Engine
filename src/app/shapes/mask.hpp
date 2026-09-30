#pragma once
#include "app/GL.h"
#include "app/shapes/path.hpp"
#include <cstdint>
#include <vector>

struct Mask {
    GLuint tex = 0;
    int x0 = 0, y0 = 0, w = 0, h = 0;
    std::vector<uint8_t> coverage;

    static Mask from(const Path& path, const Mask* parent = nullptr);
};

std::vector<uint8_t> rasterize(const Path& path, int x0, int y0, int w, int h);
GLuint uploadMask(const std::vector<uint8_t>& coverage, int w, int h);