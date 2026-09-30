#include "app/shapes/mask.hpp"
#include "app/GL.h"
#include <GL/gl.h>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <freetype/freetype.h>
#include <freetype/ftimage.h>
#include <stdexcept>
#include <vector>
#include FT_OUTLINE_H

FT_Library getLibrary() {
    static FT_Library lib = nullptr;
    if (lib != nullptr) return lib;
    if (FT_Init_FreeType(&lib)) throw std::runtime_error("FT_Init_FreeType failed");
    return lib;
}

GLuint uploadMask(const std::vector<uint8_t>& coverage, int w, int h) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, GL_FALSE, GL_RED, GL_UNSIGNED_BYTE, coverage.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

Mask Mask::from(const Path &path, const Mask* parent) {
    Rect b = path.bounds();
    int x0 = (int)std::floor(b.origin.x);
    int y0 = (int)std::floor(b.origin.y);
    int x1 = (int)std::floor(b.origin.x + b.width);
    int y1 = (int)std::floor(b.origin.y + b.height);

    if (parent) {
        x0 = std::max(x0, parent->x0);
        y0 = std::max(y0, parent->y0);
        x1 = std::min(x1, parent->x0 + parent->w);
        y1 = std::min(y1, parent->y0 + parent->h);
    }

    Mask m;
    m.x0 = x0; m.y0 = y0;
    m.w = std::max(0, x1 - x0);
    m.h = std::max(0, y1 - y0);
    if (m.w == 0 || m.h == 0) return m;

    m.coverage = rasterize(path, m.x0, m.y0, m.w, m.h);

    if (parent) {
        for (int j = 0; j < m.h; j++) {
            for (int i = 0; i < m.w; i++) {
                int px = m.x0 + i - parent->x0;
                int py = m.y0 + j - parent->y0;
                uint8_t& c = m.coverage[j * m.w + i];
                c = uint8_t(c * parent->coverage[py * parent->w + px] / 255);
            }
        }
    }
    m.tex = uploadMask(m.coverage, m.w, m.h);
    return m;
}

std::vector<uint8_t> rasterize(const Path &path, int x0, int y0, int w, int h) {
    FT_Library lib = getLibrary();
    std::vector<FT_Vector> pts;
    std::vector<unsigned char> tags;
    std::vector<unsigned short> contours;

    auto add = [&](Point p, unsigned char tag) {
        pts.push_back({ FT_Pos(std::lround(p.x - x0) << 6), FT_Pos(std::lround(y0 + h - p.y) << 6)} );
        tags.push_back(tag);
    };

    auto endContour = [&] {
        if (!pts.empty() && (contours.empty() || contours.back() != pts.size() - 1))
            contours.push_back(static_cast<unsigned short>(pts.size() - 1));
    };

    size_t i = 0;
    for (Path::Verb verb : path.verbs) {
        switch (verb) {
        case Path::Verb::Move:
            endContour();
        case Path::Verb::Line:
            add(path.points[i++], FT_CURVE_TAG_ON);
            break;
        case Path::Verb::Quad:
            add(path.points[i++], FT_CURVE_TAG_CONIC);
            add(path.points[i++], FT_CURVE_TAG_ON);
            break;
        case Path::Verb::Cubic:
            add(path.points[i++], FT_CURVE_TAG_CUBIC);
            add(path.points[i++], FT_CURVE_TAG_CUBIC);
            add(path.points[i++], FT_CURVE_TAG_ON);
            break;
        case Path::Verb::Close:
            endContour();
        }
    }
    endContour();

    FT_Outline outline{};
    outline.n_points = (unsigned short)pts.size();
    outline.points = pts.data();
    outline.tags = tags.data();
    outline.n_contours = (unsigned short)contours.size();
    outline.contours = contours.data();
    outline.flags = 0;

    std::vector<uint8_t> coverage(w * h, 0);
    FT_Bitmap bm{};
    bm.rows = h;
    bm.width = w;
    bm.pitch = w;
    bm.buffer = coverage.data();
    bm.num_grays = 256;
    bm.pixel_mode = FT_PIXEL_MODE_GRAY;
    FT_Outline_Get_Bitmap(lib, &outline, &bm);
    return coverage;
}