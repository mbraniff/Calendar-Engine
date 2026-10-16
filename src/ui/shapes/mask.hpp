#pragma once
#include "ui/GL.h"
#include "ui/shapes/path.hpp"
#include <cstdint>
#include <vector>

  struct Mask {
      GLuint tex = 0;
      int x0 = 0, y0 = 0, w = 0, h = 0;
      std::vector<uint8_t> coverage;
      uint64_t version = 0;

      Mask() = default;
      ~Mask() { if (tex) glDeleteTextures(1, &tex); }
      Mask(const Mask&) = delete;             // owns a GL texture: no copies
      Mask& operator=(const Mask&) = delete;

      void update(const Path& path, const Mask* parent);

  private:
      Path path_;
      uint64_t parentVersion_ = 0;
  };


std::vector<uint8_t> rasterize(const Path& path, int x0, int y0, int w, int h);
GLuint uploadMask(const std::vector<uint8_t>& coverage, int w, int h);