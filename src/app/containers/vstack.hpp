#pragma once
#include "app/graphics.hpp"
#include "app/view/view.hpp"
#include <vector>

class VStack : public View {
public:
    float spacing = 8;
    Alignment layoutAlignment = Alignment::center();
    
    VStack(const Alignment alignment = Alignment::center(), float spacing = 8)
        : spacing(spacing), layoutAlignment(alignment) {}
    
protected:
    void layoutSubviews() override;
    Size measure(Size proposed) override;

private:
    std::vector<Size> childSizes(Size proposal);
};