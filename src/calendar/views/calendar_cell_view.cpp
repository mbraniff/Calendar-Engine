#include "calendar/views/calendar_cell_view.hpp"
#include "calendar/colors.hpp"
#include "ui/graphics.hpp"
#include "ui/shapes/shape.hpp"
#include "ui/text/text.hpp"
#include <memory>
#include <string>

CalendarCellView::CalendarCellView(const std::string& date, bool highlighted) : date(date), highlighted(highlighted) {
    setup();
}

void CalendarCellView::setup() {
    borderColor = Colors::grid;
    borderWidth = 1;
    background = Rectangle().fill(weekend ? Colors::weekendCell : Colors::cell);
    if (!date_.lock()) {
        auto v = add<Text>(date);
        v->padding(8.0f);
        v->color = Colors::text;
        v->alignment = Alignment::topTrailing();
        date_ = std::weak_ptr(v);
    }
    auto v = date_.lock();
    v->text = date;
    if (highlighted) {
        v->background = Circle().fill(Colors::accent);
        v->background->padding(4.0f);
    } else {
        v->background.reset();
    }
}