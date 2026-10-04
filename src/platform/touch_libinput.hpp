#pragma once
#include <functional>
#include <unordered_map>
#include "platform.h"

struct libinput;
struct udev;

// Reads touchscreen events straight from the kernel through libinput.
// Each finger reports on its own channel (libinput's seat slot).
class LibinputTouch {
public:
    using PointerCallback = std::function<void(int channel, double x, double y, PointerAction action)>;

    LibinputTouch() = default;
    ~LibinputTouch();
    LibinputTouch(const LibinputTouch&) = delete;
    LibinputTouch& operator=(const LibinputTouch&) = delete;

    bool init(const char* seat = "seat0");
    // Drains pending events, mapping touches onto a width x height surface.
    void poll(int width, int height, const PointerCallback& onPointer);

private:
    struct Position { double x, y; };

    udev* udev_ = nullptr;
    libinput* li_ = nullptr;
    // libinput's up event has no position, so remember where each finger was.
    std::unordered_map<int, Position> last_;
};
