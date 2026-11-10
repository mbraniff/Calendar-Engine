#include "calendar/views/calendar_view.hpp"
#include "calendar/views/calendar_cell_view.hpp"
#include "calendar/colors.hpp"
#include "ui/graphics.hpp"
#include "ui/containers/hstack.hpp"
#include "ui/containers/vstack.hpp"
#include "ui/shapes/shape.hpp"
#include "ui/text/font.hpp"
#include "ui/text/text.hpp"
#include "ui/view/view.hpp"
#include <chrono>
#include <memory>

std::string monthNames[12] = {
    "Januarary", "Feburary", "March",
    "April", "May", "June",
    "July", "August", "September",
    "October", "November", "December"
};

CalendarView::CalendarView() {
    background = Rectangle().fill(Colors::background);
    auto now = std::chrono::system_clock::now();
    std::chrono::year_month_day currentDay = std::chrono::floor<std::chrono::days>(now);
    std::chrono::year_month_day firstOfMonth = currentDay.year() / currentDay.month() / 1;
    auto vstack = make<VStack>(0.0f);
    auto title = vstack->add<Text>(monthNames[static_cast<unsigned int>(currentDay.month()) - 1]);
    title->fontSize = 72;
    title->fontWeight = Font::Weight::Bold;
    title->color = Colors::text;
    std::chrono::weekday firstDay{firstOfMonth};

    bool hitFirstDay = false;
    int firstDayIndex = firstDay.c_encoding();
    int index = 0;
    int daysCount = 1;
    for (int i = 0; i<5; i++) {
        auto hstack = vstack->add<HStack>(0.0f);
        for (int j = 0; j<7; j++) {
            if (!hitFirstDay && firstDayIndex == index) {
                hitFirstDay = true;
            }
            auto v = hstack->add<CalendarCellView>();
            // Columns start on Sunday, as c_encoding counts weekdays.
            v->weekend = j == 0 || j == 6;
            v->setup();
            if (hitFirstDay) {
                v->highlighted = static_cast<unsigned int>(currentDay.day()) == daysCount;
                v->date = std::to_string(daysCount++);
                v->setup();
            }
            index++;
        }
    }
    addSubview(vstack);
}