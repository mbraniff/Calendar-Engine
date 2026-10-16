#pragma once
#include "ui/text/text.hpp"
#include "ui/view/view.hpp"
#include <memory>

class CalendarCellView : public View {
public:
    bool highlighted = false;
    std::string date = "";

    CalendarCellView(const std::string& date = "", bool highlighted = false);
    void setup();

private:
    std::weak_ptr<Text> date_;
};