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

void Mask::update(const Path& path, const Mask* parent) {
    uint64_t pv = parent ? parent->version : 0;
    if (version != 0 && path == path_ && pv == parentVersion_) return;

    static uint64_t nextVersion = 1;
    path_ = path;
    parentVersion_ = pv;
    version = nextVersion++;
    int oldW = w, oldH = h;

    Rect b = path.bounds();
    int left   = (int)std::floor(b.origin.x);
    int top    = (int)std::floor(b.origin.y);
    int right  = (int)std::ceil(b.origin.x + b.width);
    int bottom = (int)std::ceil(b.origin.y + b.height);

    if (parent) {
        left   = std::max(left,   parent->x0);
        top    = std::max(top,    parent->y0);
        right  = std::min(right,  parent->x0 + parent->w);
        bottom = std::min(bottom, parent->y0 + parent->h);
    }

    x0 = left;
    y0 = top;
    w = std::max(0, right - left);
    h = std::max(0, bottom - top);
    if (w == 0 || h == 0) {
        if (tex) glDeleteTextures(1, &tex);
        tex = 0;
        coverage.clear();
        return;
    }

    coverage = rasterize(path, x0, y0, w, h);

    if (parent) {
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                int px = x0 + i - parent->x0;
                int py = y0 + j - parent->y0;
                uint8_t& c = coverage[j * w + i];
                c = uint8_t(c * parent->coverage[py * parent->w + px] / 255);
            }
        }
    }

    if (tex && w == oldW && h == oldH) {
        glBindTexture(GL_TEXTURE_2D, tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, coverage.data());
    } else {
        if (tex) glDeleteTextures(1, &tex);
        tex = uploadMask(coverage, w, h);
    }

}

std::vector<uint8_t> rasterize(const Path &path, int x0, int y0, int w, int h) {
    FT_Library lib = getLibrary();
    std::vector<FT_Vector> pts;
    std::vector<unsigned char> tags;
    std::vector<unsigned short> contours;

    auto add = [&](Point p, unsigned char tag) {
        pts.push_back({ FT_Pos(std::lround((p.x - x0) * 64.0f)), FT_Pos(std::lround((y0 + h - p.y) * 64.0f))} );
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