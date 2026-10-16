#include "calendar/views/calendar_view.hpp"
#include "calendar/views/calendar_cell_view.hpp"
#include "ui/graphics.hpp"
#include "ui/containers/hstack.hpp"
#include "ui/containers/vstack.hpp"
#include "ui/shapes/shape.hpp"
#include "ui/shapes/shape_view.hpp"
#include "ui/text/font.hpp"
#include "ui/text/text.hpp"
#include <chrono>
#include <memory>

std::string monthNames[12] = {
    "Januarary", "Feburary", "March",
    "April", "May", "June",
    "July", "August", "September",
    "October", "November", "December"
};

CalendarView::CalendarView() {
    auto now = std::chrono::system_clock::now();
    std::chrono::year_month_day currentDay = std::chrono::floor<std::chrono::days>(now);
    std::chrono::year_month_day firstOfMonth = currentDay.year() / currentDay.month() / 1;
    auto vstack = std::make_shared<VStack>(0.0f);
    auto title = std::make_shared<Text>(monthNames[static_cast<unsigned int>(currentDay.month()) - 1]);
    title->fontSize = 72;
    title->fontWeight = Font::Weight::Bold;
    vstack->addSubview(title);
    std::chrono::weekday firstDay{firstOfMonth};

    bool hitFirstDay = false;
    int firstDayIndex = firstDay.c_encoding();
    int index = 0;
    int daysCount = 1;
    for (int i = 0; i<5; i++) {
        auto hstack = std::make_shared<HStack>(0.0f);
        for (int j = 0; j<7; j++) {
            if (!hitFirstDay && firstDayIndex == index) {
                hitFirstDay = true;
            }
            auto v = std::make_shared<CalendarCellView>();
            hstack->addSubview(v);
            if (hitFirstDay) {
                v->highlighted = static_cast<unsigned int>(currentDay.day()) == daysCount;
                v->date = std::to_string(daysCount++);
                v->setup();
            }
            index++;
        }
        vstack->addSubview(hstack);
    }
    addSubview(vstack);
}