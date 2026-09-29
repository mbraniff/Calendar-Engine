#include "app/text/text.hpp"
#include <string>

Text::Text(std::string text) {
    this->text = text;
}

void Text::drawSelf(Renderer &r) {
    r.drawText(font.get(fontSize, fontWeight), text, frame_.origin, glm::vec3(1.0f, 1.0f, 1.0f));
}
