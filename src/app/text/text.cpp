#include "app/text/text.hpp"
#include "app/text_layout.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>

Text::Text(std::string text) {
    this->text = text;
}

void Text::drawSelf(Renderer &r) {
    Rect frame = contentFrame();
    r.drawText(text_, font.get(fontSize, fontWeight), frame.origin, color);
}

Size Text::measure(Size proposal) {
    auto& set = font.get(fontSize, fontWeight);
    text_.clear();

    float x = 0, y = 0;
    size_t lineStart = 0, wordStart = 0;
    auto advanceOf = [](const Font::Character& ch) { return float(ch.Advance >> 6); };

    // Moves down a line, or returns false if that line wouldn't fit. The caller resets x.
    auto nextLine = [&]() {
        // Slack so re-measuring at exactly our own height doesn't lose the last line to float rounding.
        if (y + 2 * set.lineHeight > proposal.height + 0.01f) return false;
        y += set.lineHeight;
        return true;
    };

    // Ends the current line with "...", dropping glyphs from `end` on and as many before it as needed to make room.
    auto truncate = [&](size_t end) {
        const Font::Character& dot = set.chars['.'];
        float dotsWidth = 2 * advanceOf(dot) + dot.Bearing.x + dot.Size.x;
        text_.resize(end);
        float pen = 0;
        while (text_.size() > lineStart) {
            pen = text_.back().pos.x + advanceOf(*text_.back().ch);
            if (pen + dotsWidth <= proposal.width) break;
            text_.pop_back();
            pen = 0;
        }
        for (int k = 0; k < 3; k++) {
            text_.push_back({ &dot, { pen, y } });
            pen += advanceOf(dot);
        }
    };

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '\n') {
            if (!nextLine()) { truncate(text_.size()); break; }
            x = 0;
            lineStart = wordStart = text_.size();
            continue;
        }

        const Font::Character& ch = set.chars[c];
        float advance = advanceOf(ch);

        // Spaces aren't drawn; a trailing one may hang past the edge, and the next word wraps instead.
        if (c == ' ') {
            x += advance;
            wordStart = text_.size();
            continue;
        }

        // The last glyph of a word only needs its ink to fit, not its full advance.
        bool endOfWord = i + 1 == text.size() || text[i + 1] == ' ' || text[i + 1] == '\n';
        float right = endOfWord ? x + ch.Bearing.x + ch.Size.x : x + advance;

        if (right > proposal.width && text_.size() > lineStart) {
            // Move the whole word down, or break mid-word if it's the only thing on the line.
            size_t from = wordStart > lineStart ? wordStart : text_.size();
            if (!nextLine()) { truncate(from); break; }
            float shift = from < text_.size() ? text_[from].pos.x : x;
            for (size_t j = from; j < text_.size(); j++) {
                text_[j].pos.x -= shift;
                text_[j].pos.y = y;
            }
            x -= shift;
            lineStart = wordStart = from;
        }

        text_.push_back({ &ch, { x, y } });
        x += advance;
    }

    float width = 0;
    for (auto& e : text_) width = std::max(width, e.pos.x + e.ch->Bearing.x + e.ch->Size.x);
    return { std::ceil(width), text_.empty() ? 0 : y + set.lineHeight };
}
