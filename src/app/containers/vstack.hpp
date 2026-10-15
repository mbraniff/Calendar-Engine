#pragma once
#include "app/containers/stack.hpp"
#include "app/graphics.hpp"
#include <optional>

class VStack : public Stack {
public:
    using Stack::Stack;
protected:
    void layoutSubviews() override;
    Size measure(std::optional<float> w, std::optional<float> h) override;
};