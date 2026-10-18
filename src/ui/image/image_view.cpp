#include "ui/image/image_view.hpp"
#include "ui/GL.h"
#include "ui/graphics.hpp"
#include "ui/image/stb_image.h"
#include "ui/paths.hpp"
#include <algorithm>
#include <glm/ext/vector_float4.hpp>
#include <optional>
#include <string_view>

ImageView::ImageView(std::string_view name) {
    aspectRatio.onChange([this] { setNeedsLayout(); });
    auto imagePath = paths::asset(name);
    int channels = 0;
    // Always 4 channels, so grayscale and RGB images upload the same way as RGBA.
    unsigned char* data = stbi_load(imagePath.string().c_str(), &width_, &height_, &channels, 4);
    if (!data) {
        width_ = height_ = 0;
        return;
    }
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    // Scaled, unlike masks: linear when enlarged, blended mip levels when shrunk so it doesn't shimmer.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    aspectRatio = naturalAspectRatio();
}

ImageView::~ImageView() {
    if (texture_) glDeleteTextures(1, &texture_);
}

float ImageView::naturalAspectRatio() const {
    return height_ > 0 ? (float)width_ / (float)height_ : 1.0f;
}

Size ImageView::measure(std::optional<float> w, std::optional<float> h) {
    if (!texture_) return { w.value_or(0), h.value_or(0) };
    float W = (float)width_, H = (float)height_;

    if (contentMode == ContentMode::Stretch) return { w.value_or(W), h.value_or(H) };

    float r = aspectRatio;
    if (!w && !h) return { W, W / r };
    if (!h) return { *w, *w / r };
    if (!w) return { *h * r, *h };
    // Fill takes the whole proposal and crops the image to it, rather than overflowing into its neighbours.
    if (contentMode == ContentMode::Fill) return { *w, *h };
    Rect fit = placed({ {}, *w, *h });
    return { fit.width, fit.height };
}

Rect ImageView::placed(Rect box) const {
    if (contentMode == ContentMode::Stretch) return box;
    float r = aspectRatio;
    // The height of the largest r-shaped rect inside box, or of the smallest one covering it.
    float s = contentMode == ContentMode::Fit
        ? std::min(box.width / r, box.height)
        : std::max(box.width / r, box.height);
    Size size = { s * r, s };
    return { { box.origin.x + (box.width - size.width) / 2, box.origin.y + (box.height - size.height) / 2 },
        size.width, size.height };
}

void ImageView::drawSelf(Renderer& r) {
    Rect box = contentFrame();
    if (!texture_ || box.width <= 0 || box.height <= 0) return;
    Rect image = placed(box);
    glm::vec4 uv(0, 0, 1, 1);
    if (contentMode == ContentMode::Fill) {
        float fx = box.width / image.width, fy = box.height / image.height;
        uv = { (1 - fx) / 2, (1 - fy) / 2, fx, fy };
        image = box;
    }
    r.drawImage(texture_, image, uv, color, templatedMode);
}
