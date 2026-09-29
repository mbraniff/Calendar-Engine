#include "app/font.hpp"
#include "app/GL.h"
#include "app/paths.hpp"
#include <freetype/fttypes.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H

Font::Font(const std::string &path) {
    if (FT_Init_FreeType(&ft_)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    if (FT_New_Face(ft_, path.c_str(), 0, &face_)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    FT_MM_Var* mm = nullptr;
    if (FT_Get_MM_Var(face_, &mm) == 0) {
        numAxes_ = mm->num_axis;
        for (FT_UInt i = 0; i < mm->num_axis; ++i) {
            FT_ULong tag = mm->axis[i].tag;
            if (tag == FT_MAKE_TAG('w', 'g', 'h', 't')) wghtAxis_ = i;
            if (tag == FT_MAKE_TAG('o', 'p', 's', 'z')) opszAxis_ = i;
            defaults_.push_back(mm->axis[i].def);
        }
        FT_Done_MM_Var(ft_, mm);
    }
}

Font::~Font() {
    for (auto& [key, set] : cache_)
        for (auto& [c, ch] : set.chars) glDeleteTextures(1, &ch.TextureID);
    FT_Done_Face(face_);
    FT_Done_FreeType(ft_);
}

Font::GlyphSet& Font::get(int size, Weight weight) {
    if (!face_) {
        std::cout << "FACE NOT INITIALIZED" << std::endl;
        exit(1);
    }
    Font::Key key{size, static_cast<int>(weight)};
    auto it = cache_.find(key);
    if (it != cache_.end()) { return it->second; }
    return BuildGlyphSet(key);
}

Font::GlyphSet& Font::BuildGlyphSet(Font::Key key) {
    FT_Set_Pixel_Sizes(face_, 0, key.pixelSize);

    std::vector<FT_Fixed> coords = defaults_;

    if (wghtAxis_ != -1) {
        coords[wghtAxis_] = static_cast<FT_Fixed>(key.weight << 16);
    }

    if (opszAxis_ != -1) {
        coords[opszAxis_] = static_cast<FT_Fixed>(key.pixelSize << 16);
    }

    if (!coords.empty())
        FT_Set_Var_Design_Coordinates(face_, numAxes_, coords.data());

    GlyphSet& set = cache_[key];
    set.lineHeight = face_->size->metrics.height / 64.0f;
    set.ascender = face_->size->metrics.ascender / 64.0f;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 32; c < 128; c++) {
        if (FT_Load_Char(face_, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
        0,
        GL_R8,
        face_->glyph->bitmap.width,
        face_->glyph->bitmap.rows,
        GL_FALSE,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face_->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face_->glyph->bitmap.width, face_->glyph->bitmap.rows),
            glm::ivec2(face_->glyph->bitmap_left, face_->glyph->bitmap_top),
            static_cast<unsigned int>(face_->glyph->advance.x)
        };

        set.chars.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return set;
}

Font& Font::Default() {
    static Font* font = new Font(paths::font("Inter.ttf"));
    return *font;
}