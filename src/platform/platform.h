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

    std::function<void(float x, float y, bool down)> onPointer;
    std::function<void(int key)> onKey;
};

std::unique_ptr<Platform> createPlatform();
std::filesystem::path exeDir();