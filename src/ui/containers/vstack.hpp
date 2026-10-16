#pragma once
#include "ui/containers/stack.hpp"
#include "ui/graphics.hpp"
#include <optional>

class VStack : public Stack {
public:
    using Stack::Stack;
protected:
    void layoutSubviews() override;
    Size measure(std::optional<float> w, std::optional<float> h) override;
};