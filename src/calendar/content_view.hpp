#include "calendar/views/calendar_view.hpp"
#include "ui/view_controller.hpp"
#include <memory>

class ContentViewViewController : public ViewController {
public:
    ContentViewViewController() : ViewController(std::make_shared<CalendarView>()) {}
};