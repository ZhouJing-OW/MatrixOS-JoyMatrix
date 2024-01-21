#include "MatrixOS.h"
#include "UI/UI.h"

namespace MatrixOS::UIInterface
{
  // Three digit max
  int32_t NumberSelector16x4(int16_t value, Color color, string name, int16_t lower_limit = INT16_MIN,
                            int16_t upper_limit = INT16_MAX) {
    // Point origin = Point((Device::x_size - 1) / 2, (Device::y_size - 1));

    UI numberSelector(name, color);

    KnobConfig knobConfig = {
        .min = lower_limit,
        .max = upper_limit,
        .color = color,
    };

    Device::Encoder::DisableAll();
    Device::Encoder::Setup(&value, &knobConfig, 0);

    UIButtonWithColorFunc knobColor(
    "", [&]() -> Color { return color.Scale(value * 239 / upper_limit  + 16); }, [&]() -> void {}, [&]() -> void {});
    numberSelector.AddUIComponent(knobColor, Point(8, 4));
    
    numberSelector.AddUIComponent(new UI4pxNumber(color, 3, (int32_t*)&value, Color(0xFFFFFF)), Point(5, 0));

    int32_t modifier1[1] = {10};
    uint8_t gradient1[1] = {255};
    numberSelector.AddUIComponent(new UINumberModifier(color, 1, (int32_t*)&value, modifier1, gradient1, lower_limit,
                                                       upper_limit),
                                  Point(1, 0));

    int32_t modifier2[1] = {-10};
    uint8_t gradient2[1] = {255};
    numberSelector.AddUIComponent(new UINumberModifier(color, 1, (int32_t*)&value, modifier2, gradient2, lower_limit,
                                                       upper_limit),
                                  Point(1, 2));
    
    
    int32_t modifier3[1] = {-1};
    uint8_t gradient3[1] = {127};
    numberSelector.AddUIComponent(new UINumberModifier(color, 1, (int32_t*)&value, modifier3, gradient3, lower_limit,
                                                       upper_limit),
                                  Point(0, 1));

    int32_t modifier4[1] = {1};
    uint8_t gradient4[1] = {127};
    numberSelector.AddUIComponent(new UINumberModifier(color, 1, (int32_t*)&value, modifier4, gradient4, lower_limit,
                                                       upper_limit),
                                  Point(2, 1));

    UIButton min("min", color.Scale(32), [&]() -> void { value = lower_limit; });
    numberSelector.AddUIComponent(min, Point(0, 3));

    UIButton middle("middle", color.Scale(143), [&]() -> void { value = (lower_limit + upper_limit) / 2; });
    numberSelector.AddUIComponent(middle, Point(1, 3));

    UIButton max("max", color, [&]() -> void { value = upper_limit; });
    numberSelector.AddUIComponent(max, Point(2, 3));


    numberSelector.Start();

    Device::Encoder::DisableAll();
    return value;
  }
}