#include "app/text/text.hpp"
#include <string>

Text::Text(std::string text) {
    this->text = text;
}

void Text::drawSelf(Renderer &r) {
    r.drawText(font.get(fontSize, fontWeight), text, frame_.origin, glm::vec3(color.r, color.g, color.b));
}

Size Text::measure(Size proposal) {
    auto& set = font.get(fontSize, fontWeight);
    float width = 0;
    for (auto& c : text) width += set.chars[c].Advance >> 6;
    return { width, set.lineHeight };
}