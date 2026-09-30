#include "app/view/view.hpp"
#include "app/containers/frame.hpp"

std::shared_ptr<View> View::frame(FrameOptions options) {
    auto frame = std::make_shared<Frame>(options);
    frame->addSubview(shared_from_this());
    return frame;
}