#pragma once
#include "app/containers/stack.hpp"
#include "app/graphics.hpp"

class VStack : public Stack {
public:
    using Stack::Stack;
protected:
    void layoutSubviews() override;
    Size measure(Size proposed) override;
};