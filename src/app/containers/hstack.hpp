#pragma once
#include "app/containers/stack.hpp"
#include "app/graphics.hpp"
#include <optional>

class HStack : public Stack {
public:
    using Stack::Stack;
protected:
    void layoutSubviews() override;
    Size measure(std::optional<float> w, std::optional<float> h) override;
};