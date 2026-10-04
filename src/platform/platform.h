#pragma once
#include <functional>
#include <memory>
#include <filesystem>

struct PlatformConfig {
    int width, height;
    const char* title;
    bool fullscreen;
    bool hideCursor;
};

enum PointerAction {
    Down,
    Move,
    Up
};

class Platform {
public:
    virtual ~Platform() = default;
    virtual bool init(const PlatformConfig &cfg) = 0;
    virtual bool shouldClose() const = 0;
    virtual void pollEvents() = 0;
    virtual void swapBuffers() = 0;
    virtual void framebufferSize(int &w, int &h) const = 0;
    virtual double time() const = 0;
    virtual bool isKeyDown(int key) = 0;
    virtual void requestClose() = 0;

    std::function<void(int channel, double x, double y, PointerAction action)> onPointer;
    std::function<void(int key)> onKey;
    std::function<void(int w, int h)> onFramebufferResize;
};

std::unique_ptr<Platform> createPlatform();
std::filesystem::path exeDir();