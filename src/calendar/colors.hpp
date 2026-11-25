#pragma once
#include "ui/graphics.hpp"
#include <array>

// The calendar's palette, named by role. Dark, since it's on a wall all day.
namespace Colors {
    inline constexpr Color background    = Color::hex(0x121417);
    inline constexpr Color cell          = Color::hex(0x1A1D21);
    inline constexpr Color weekendCell   = Color::hex(0x16191C);
    // Buttons, sheets and popovers above the grid; close to cell in tone, so give it a grid border.
    inline constexpr Color raisedSurface = Color::hex(0x2C2C2E);
    inline constexpr Color grid          = Color::hex(0x2A2E34);
    inline constexpr Color text          = Color::hex(0xECEEF1);
    inline constexpr Color secondaryText = Color::hex(0x8B929C);
    inline constexpr Color mutedText     = Color::hex(0x4A5059);
    // Only for "now": today and the current time.
    inline constexpr Color accent        = Color::hex(0xFF5A4E);
    // Today's day number on the accent disc.
    inline constexpr Color onAccent      = Color::hex(0x000000);
    // Ring around the selected day or a pressed control.
    inline constexpr Color focusRing     = text;

    // Event categories, picked by index; similar brightness so none dominates.
    inline constexpr std::array<Color, 6> events = {
        Color::hex(0x4C8DFF), Color::hex(0x3DBE8B), Color::hex(0xF2B33D),
        Color::hex(0xA27BFF), Color::hex(0xF06BA8), Color::hex(0x2EC4C9),
    };
    // Text on any event chip.
    inline constexpr Color onEvent       = Color::hex(0x000000);
}
