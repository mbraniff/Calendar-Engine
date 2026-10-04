#include "touch_libinput.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <unistd.h>

namespace {
    int openRestricted(const char* path, int flags, void*) {
        int fd = open(path, flags);
        return fd < 0 ? -errno : fd;
    }

    void closeRestricted(int fd, void*) { close(fd); }

    const libinput_interface kInterface = { openRestricted, closeRestricted };
}

LibinputTouch::~LibinputTouch() {
    if (li_) libinput_unref(li_);
    if (udev_) udev_unref(udev_);
}

bool LibinputTouch::init(const char* seat) {
    udev_ = udev_new();
    if (!udev_) {
        std::fprintf(stderr, "libinput: udev_new failed\n");
        return false;
    }
    li_ = libinput_udev_create_context(&kInterface, nullptr, udev_);
    if (!li_) {
        std::fprintf(stderr, "libinput: failed to create context\n");
        return false;
    }
    if (libinput_udev_assign_seat(li_, seat) != 0) {
        std::fprintf(stderr, "libinput: failed to assign seat %s\n", seat);
        return false;
    }
    return true;
}

void LibinputTouch::poll(int width, int height, const PointerCallback& onPointer) {
    if (!li_) return;
    libinput_dispatch(li_);
    while (libinput_event* ev = libinput_get_event(li_)) {
        auto type = libinput_event_get_type(ev);
        switch (type) {
        case LIBINPUT_EVENT_TOUCH_DOWN:
        case LIBINPUT_EVENT_TOUCH_MOTION: {
            auto* touch = libinput_event_get_touch_event(ev);
            int slot = libinput_event_touch_get_seat_slot(touch);
            double x = libinput_event_touch_get_x_transformed(touch, width);
            double y = libinput_event_touch_get_y_transformed(touch, height);
            last_[slot] = { x, y };
            if (onPointer) onPointer(slot, x, y, type == LIBINPUT_EVENT_TOUCH_DOWN ? PointerAction::Down : PointerAction::Move);
            break;
        }
        case LIBINPUT_EVENT_TOUCH_UP:
        case LIBINPUT_EVENT_TOUCH_CANCEL: {
            auto* touch = libinput_event_get_touch_event(ev);
            int slot = libinput_event_touch_get_seat_slot(touch);
            auto it = last_.find(slot);
            if (it == last_.end()) break;
            if (onPointer) onPointer(slot, it->second.x, it->second.y, PointerAction::Up);
            last_.erase(it);
            break;
        }
        default:
            break;
        }
        libinput_event_destroy(ev);
    }
}
