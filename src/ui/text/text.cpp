#include "ui/text/text.hpp"
#include "ui/graphics.hpp"
#include "ui/text/text_layout.hpp"
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <optional>
#include <string>

Text::Text(std::string text) {
    this->text = text;
    this->text.onChange([this]{
        setNeedsLayout();
    });
}

void Text::drawSelf(Renderer &r) {
    float factor = alignmentFactor_;
    if (factor != alignedFactor_) align(factor);
    Rect frame = contentFrame();
    r.drawText(aligned_, font.get(fontSize, fontWeight), frame.origin, color);
}

void Text::layoutSubviews() {
    // View::layout measures us at the final proposal right before this, so measured_ holds our glyphs.
    text_.swap(measured_);

    lines_.clear();
    textWidth_ = 0;
    for (size_t start = 0; start < text_.size();) {
        size_t end = start;
        while (end < text_.size() && text_[end].pos.y == text_[start].pos.y) end++;
        const TextEntry& last = text_[end - 1];
        float lineWidth = last.pos.x + last.ch->Bearing.x + last.ch->Size.x;
        lines_.push_back({ end, lineWidth });
        textWidth_ = std::max(textWidth_, lineWidth);
        start = end;
    }
    textWidth_ = std::ceil(textWidth_);
    alignedFactor_ = std::numeric_limits<float>::quiet_NaN();
}

void Text::align(float factor) {
    aligned_ = text_;
    size_t start = 0;
    for (const Line& line : lines_) {
        // Whole pixels, so glyphs don't land on half-pixels and blur.
        float shift = std::floor((textWidth_ - line.width) * factor);
        for (size_t i = start; i < line.end; i++) aligned_[i].pos.x += shift;
        start = line.end;
    }
    alignedFactor_ = factor;
}

Size Text::measure(std::optional<float> w, std::optional<float> h) {
    auto& set = font.get(fontSize, fontWeight);
    measured_.clear();

    float x = 0, y = 0;
    size_t lineStart = 0, wordStart = 0;
    auto advanceOf = [](const Font::Character& ch) { return float(ch.Advance >> 6); };

    // Moves down a line, or returns false if that line wouldn't fit. The caller resets x.
    auto nextLine = [&]() {
        if (!h.has_value()) {
            y += set.lineHeight;
            return true;
        }
        // Slack so re-measuring at exactly our own height doesn't lose the last line to float rounding.
        if (y + 2 * set.lineHeight > h.value() + 0.01f) return false;
        y += set.lineHeight;
        return true;
    };

    // Ends the current line with "...", dropping glyphs from `end` on and as many before it as needed to make room.
    auto truncate = [&](size_t end) {
        const Font::Character& dot = set.chars['.'];
        float dotsWidth = 2 * advanceOf(dot) + dot.Bearing.x + dot.Size.x;
        measured_.resize(end);
        float pen = 0;
        while (measured_.size() > lineStart) {
            pen = measured_.back().pos.x + advanceOf(*measured_.back().ch);
            if (pen + dotsWidth <= w.value_or(infinity)) break;
            measured_.pop_back();
            pen = 0;
        }
        for (int k = 0; k < 3; k++) {
            measured_.push_back({ &dot, { pen, y } });
            pen += advanceOf(dot);
        }
    };

    std::string text = this->text;
    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '\n') {
            if (!nextLine()) { truncate(measured_.size()); break; }
            x = 0;
            lineStart = wordStart = measured_.size();
            continue;
        }

        const Font::Character& ch = set.chars[c];
        float advance = advanceOf(ch);

        // Spaces aren't drawn; a trailing one may hang past the edge, and the next word wraps instead.
        if (c == ' ') {
            x += advance;
            wordStart = measured_.size();
            continue;
        }

        // The last glyph of a word only needs its ink to fit, not its full advance.
        bool endOfWord = i + 1 == text.size() || text[i + 1] == ' ' || text[i + 1] == '\n';
        float right = endOfWord ? x + ch.Bearing.x + ch.Size.x : x + advance;

        if (right > w.value_or(infinity) && measured_.size() > lineStart) {
            // Move the whole word down, or break mid-word if it's the only thing on the line.
            size_t from = wordStart > lineStart ? wordStart : measured_.size();
            if (!nextLine()) { truncate(from); break; }
            float shift = from < measured_.size() ? measured_[from].pos.x : x;
            for (size_t j = from; j < measured_.size(); j++) {
                measured_[j].pos.x -= shift;
                measured_[j].pos.y = y;
            }
            x -= shift;
            lineStart = wordStart = from;
        }

        measured_.push_back({ &ch, { x, y } });
        x += advance;
    }

    float width = 0;
    for (auto& e : measured_) width = std::max(width, e.pos.x + e.ch->Bearing.x + e.ch->Size.x);
    width = std::ceil(width);

    return { width, measured_.empty() ? 0 : y + set.lineHeight };
}

void Text::setTextAlignment(TextAlignment alignment) {
    alignmentFactor_ = alignmentFactor(alignment);
}

TextAlignment Text::getTextAlignment() {
    auto target = alignmentFactor_.target();
    if (target < 0.5f) {
        return TextAlignment::Left;
    } else if (target < 1.0f) {
        return TextAlignment::Centered;
    }
    return TextAlignment::Right;
}