#pragma once
#include "app/view/view.hpp"
#include <memory>

class ScrollView : public View {
public:
    enum class ScrollAxis { Vertical, Horizontal, Both };
    ScrollView(ScrollAxis axis = ScrollAxis::Both);

    void addSubview(std::shared_ptr<View> view) override;
    void layoutSubviews() override;
    void pointerDown(int, Point, double) override;
private:
    ScrollAxis axis_;
    std::shared_ptr<View> container_;
    std::shared_ptr<View> verticalScrollIndicator_;
    std::shared_ptr<View> horizontalScrollIndicator_;
};