#include "calendar/playground_view_controller.hpp"
#include "ui/animation/animator.hpp"
#include "ui/animation/motion.hpp"
#include "ui/button/button.hpp"
#include "ui/containers/frame.hpp"
#include "ui/containers/hstack.hpp"
#include "ui/containers/vstack.hpp"
#include "ui/gestures/drag_gesture.hpp"
#include "ui/gestures/tap_gesture.hpp"
#include "ui/shapes/shape.hpp"
#include "ui/shapes/shape_view.hpp"
#include "ui/view/scroll_view.hpp"
#include <algorithm>
#include <string>

PlaygroundViewController::PlaygroundViewController() : ViewController(std::make_shared<View>()) {
    bouncer_ = std::make_shared<Text>("This is some text under the original text");
    bouncer_->alignment = Alignment::centerLeading();
    bouncer_->offset.update([](Point& p) {
        p.x = -100.0f;
    });
    bouncer_->color = Color::red();
    bouncer_->fontWeight = Font::Weight::Heavy;
    bouncer_->padding(8);
    bouncer_->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(5.0f), Color::blue());

    counterView_ = std::make_shared<Text>("0");
    counterView_->alignment = Alignment::bottomTrailing();
    bouncer_->addGesture(std::make_shared<TapGestureRecognizer>([this](std::shared_ptr<GestureRecognizer> sender) {
        counter_ += 1;
        counterView_->text = std::to_string(counter_);
        counterView_->setNeedsLayout();
    }));

    std::shared_ptr<Text> bigTextLabel = std::make_shared<Text>("Text that is too large for its frame, something that is biggy: like way too biggy");
    bigTextLabel->color = Color::white();
    bigTextLabel->setTextAlignment(TextAlignment::Right);
    auto bigText = bigTextLabel->frame({.maxWidth=100, .maxHeight=100});
    bigText->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(5.0f), Color::blue());
    auto btg = std::make_shared<DragGestureRecognizer>();
    btg->onChange([weak = std::weak_ptr(bigText)](DragGestureRecognizer::DragEvent event) {
        auto view = weak.lock();
        ((Frame*)view.get())->options.maxWidth.value() += event.translation.width;
        ((Frame*)view.get())->options.maxHeight.value() += event.translation.height;
        view->setNeedsLayout();
    });
    bigText->addGesture(btg);

    auto containerHStack = std::make_shared<HStack>();
    auto scrollView = std::make_shared<ScrollView>();
    scrollView->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(20.0f), Color::black().withAlpha(0.75f));
    auto scrollViewFrame = scrollView->frame({ .width = 300, .height = 500 });
    auto vstack = std::make_shared<VStack>();
    auto hstack = std::make_shared<HStack>(0);
    auto frame = vstack->frame({.maxHeight = infinity});
    auto swapAlignment = std::make_shared<Button>(std::make_shared<Text>("Swap"), [weak = std::weak_ptr(bigTextLabel)]{
        animate(Animation::spring(), [&]{
            auto view = weak.lock();
            if (!view) return;
            auto text = static_cast<Text*>(view.get());
            switch (text->getTextAlignment()) {
            case TextAlignment::Left:
                text->setTextAlignment(TextAlignment::Centered);
                break;
            case TextAlignment::Centered:
                text->setTextAlignment(TextAlignment::Right);
                break;
            case TextAlignment::Right:
                text->setTextAlignment(TextAlignment::Left);
                break;
            }
            view->setNeedsLayout();
        });
    });

    auto scrollVStack = std::make_shared<VStack>();
    scrollView->addSubview(scrollVStack);

    std::shared_ptr<Button> b = std::make_shared<Button>(std::make_shared<Text>("Add To ScrollView"), [weak = std::weak_ptr(scrollVStack)]() {
        auto view = weak.lock();
        if (!view) return;
        auto text = std::make_shared<Text>("This is some text yippee yippee doo, this is some text, won't you let me coo! I will go dance and eat the breakfast!");
        text->fontSize = 72;
        text->fontWeight = Font::Weight::Heavy;
        text->color = Color::white();
        view->addSubview(text);
    });
    b->padding(8);
    b->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(5.0f), Color(0.8f, 0.0f, 0.8f));
    b->clipShape = std::make_unique<Capsule>();
    b->borderWidth = 2;
    std::shared_ptr<DragGestureRecognizer> dragGesture = std::make_shared<DragGestureRecognizer>();
    dragGesture->onChange([weak = std::weak_ptr<View>(b)](DragGestureRecognizer::DragEvent event){
        auto view = weak.lock();
        Point offset = view->offset;
        view->offset = { offset.x + event.translation.width, offset.y + event.translation.height };
    }).onEnd([this, weak = std::weak_ptr<View>(b)](DragGestureRecognizer::DragEvent event) {
        auto view = weak.lock();
        if (!view) return;

        Point anchor = view->anchor();
        Point offset = view->offset;   // current, including this poll's moves
        Size s = view->size();
        Size bounds = view_->size();
        // How much farther the flick carries the pointer, applied to where the view is now.
        Size flick = { event.projectedEnd.x - event.location.x, event.projectedEnd.y - event.location.y };

        float x = std::clamp(anchor.x + offset.x + event.translation.width + flick.width,
                            0.0f, bounds.width - s.width);
        float y = std::clamp(anchor.y + offset.y + event.translation.height + flick.height,
                            0.0f, bounds.height - s.height);

        animate(Animation::spring(), [&] {
            view->offset.setVelocity({ event.velocity.width, event.velocity.height });
            view->offset = { x - anchor.x, y - anchor.y };
        });
    });
    b->addGesture(dragGesture);

    vstack->padding(8);
    vstack->addSubview(b);
    vstack->addSubview(bouncer_);
    vstack->addSubview(bigText);
    vstack->addSubview(swapAlignment);

    containerHStack->addSubview(frame);
    containerHStack->addSubview(scrollViewFrame);

    hstack->clipShape = std::make_unique<Rectangle>();
    hstack->addSubview(std::make_shared<ShapeView>(std::make_shared<Circle>(), Color::blue())->frame({.width = 100}));
    hstack->addSubview(std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(25), Color::green())->frame({.width = 200, .height = 200}));
    hstack->padding(8);
    view_->addSubview(hstack);
    view_->addSubview(containerHStack);
    view_->addSubview(counterView_);

    animate(Animation::autoreverse(Animation::timed(Animation::Easing::EaseOut, 1.0f), 0), [&] {
        bouncer_->background->opacity = 0.2f;
    });
}

void PlaygroundViewController::tick(float dt) {
    float speed = 25.0f;
    bouncer_->offset.update([&](Point& p) {
        p.x += dirx_ * speed * dt;
        p.y += diry_ * speed * dt;
    });
    // The frame is where layout put it; the bounce moves it with the offset on top.
    Rect f = bouncer_->frame();
    f.origin = bouncer_->anchor() + bouncer_->offset;
    Size bounds = view_->size();
    if (f.origin.y + f.height >= bounds.height) diry_ = -1;
    if (f.origin.y <= 0)                        diry_ =  1;
    if (f.origin.x + f.width >= bounds.width)   dirx_ = -1;
    if (f.origin.x <= 0)                        dirx_ =  1;
}
