#include "calendar/views/calendar_cell_view.hpp"
#include "ui/graphics.hpp"
#include "ui/shapes/shape_view.hpp"
#include "ui/text/text.hpp"
#include <memory>
#include <string>

CalendarCellView::CalendarCellView(const std::string& date, bool highlighted) : date(date), highlighted(highlighted) {
    setup();
}

void CalendarCellView::setup() {
    borderColor = Color::gray();
    borderWidth = 1;
    if (!date_.lock()) {
        auto v = std::make_shared<Text>(date);
        date_ = std::weak_ptr(v);
        v->padding(8.0f);
        v->alignment = Alignment::topTrailing();
        addSubview(v);
    }
    auto v = date_.lock();
    v->text = date;
    if (highlighted) {
        v->background = std::make_shared<ShapeView>(std::make_shared<Circle>(), Color::red().withAlpha(0.7));
        v->background->padding(4.0f);
    } else {
        v->background.reset();
    }
}