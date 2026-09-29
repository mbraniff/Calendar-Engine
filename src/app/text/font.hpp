#pragma once
#include <freetype/freetype.h>
#include <glm/ext/vector_int2.hpp>
#include <map>
#include <string>
#include <vector>

class Font {
public:
    struct Character {
        unsigned int TextureID;
        glm::ivec2 Size;
        glm::ivec2 Bearing;
        unsigned int Advance;
    };

    struct Key {
        int pixelSize;
        int weight;
        auto operator<=>(const Key&) const = default;
    };

    struct GlyphSet {
        std::map<char, Character> chars;
        float lineHeight;
        float ascender;
    };

    enum class Weight : int { Light = 300, Regular = 400, SemiBold = 600, Bold = 700, Heavy = 900 };

    explicit Font(const std::string &path);
    ~Font();
    GlyphSet& get(int size, Weight weight);
    static Font& Default();
private:
    FT_Library ft_{};
    FT_Face face_{};
    FT_UInt numAxes_ = 0;
    int wghtAxis_ = -1, opszAxis_ = -1;
    std::vector<FT_Fixed> defaults_;
    std::map<Key, GlyphSet> cache_;

    GlyphSet& BuildGlyphSet(Key key);
};