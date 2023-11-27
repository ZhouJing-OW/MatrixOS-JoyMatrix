#include "HIDtest.h"
#include "UIKeyboardKey.h"
#include "UIDPad.h"
#include "UIGamepadKey.h"
#include "UIGamepadAxis.h"

void HIDtest::Setup() {
    UI keypadUI("", Color(0xFFFFFF));

    UIKeyboardKey Dkey(Color(0xFF0000), KEY_D);
    keypadUI.AddUIComponent(Dkey, Point(9, 0));

    UIKeyboardKey Fkey(Color(0xFF0000), KEY_F);
    keypadUI.AddUIComponent(Fkey, Point(10, 0));

    UIKeyboardKey Jkey(Color(0xFF0000), KEY_J);
    keypadUI.AddUIComponent(Jkey, Point(13, 0));

    UIKeyboardKey Kkey(Color(0xFF0000), KEY_K);
    keypadUI.AddUIComponent(Kkey, Point(14, 0));

    UIDPad Dpad(Color(0x00FF00));
    keypadUI.AddUIComponent(Dpad, Point(0, 1));

    UIGamepadKey Akey(Color(0x00FF00), GAMEPAD_A);
    keypadUI.AddUIComponent(Akey, Point(6, 3));

    UIGamepadKey Bkey(Color(0x00FF00), GAMEPAD_B);
    keypadUI.AddUIComponent(Bkey, Point(7, 2));

    UIGamepadKey Xkey(Color(0x00FF00), GAMEPAD_X);
    keypadUI.AddUIComponent(Xkey, Point(5, 2));

    UIGamepadKey Ykey(Color(0x00FF00), GAMEPAD_Y);
    keypadUI.AddUIComponent(Ykey, Point(6, 1));

    UIGamepadKey L1key(Color(0x00FFFF), GAMEPAD_L1);
    keypadUI.AddUIComponent(L1key, Point(0, 0));

    UIGamepadKey R1key(Color(0x00FFFF), GAMEPAD_R1);
    keypadUI.AddUIComponent(R1key, Point(7, 0));

    UIGamepadAxis L2pad(Color(0x00FFFF), GAMEPAD_AXIS_LEFT_TRIGGER, 127);
    keypadUI.AddUIComponent(L2pad, Point(1, 0));

    UIGamepadAxis R2pad(Color(0x00FFFF), GAMEPAD_AXIS_RIGHT_TRIGGER, 127);
    keypadUI.AddUIComponent(R2pad, Point(6, 0));

    UIGamepadKey Startkey(Color(0x0000FF), GAMEPAD_START);
    keypadUI.AddUIComponent(Startkey, Point(11, 3));

    UIGamepadKey Selectkey(Color(0x0000FF), GAMEPAD_SELECT);
    keypadUI.AddUIComponent(Selectkey, Point(12, 3));

    UIButtonWithColorFunc LX("mainView", [&]() -> Color { return Color(0xFFFF00).Scale(Device::Rocker::GetX(0) + 128); }, [&]() -> void {}, [&]() -> void {});
    keypadUI.AddUIComponent(LX, Point(8, 1));

    UIButtonWithColorFunc LY("mainView", [&]() -> Color { return Color(0xFFFF00).Scale(Device::Rocker::GetY(0) + 128); }, [&]() -> void {}, [&]() -> void {});
    keypadUI.AddUIComponent(LY, Point(9, 1));

    UIButtonWithColorFunc LB(
        "mainView", [&]() -> Color { return Color(0xFFFF00).Scale(Device::Rocker::GetButton(0) * 80); }, [&]() -> void {},
        [&]() -> void {});
    keypadUI.AddUIComponent(LB, Point(10, 1));

    UIButtonWithColorFunc LP("mainView", [&]() -> Color { return Color(0xFFFF00).Scale(Device::Rocker::GetPressure(0) * 2); }, [&]() -> void {}, [&]() -> void {});
    keypadUI.AddUIComponent(LP, Point(11, 1));

    UIButtonWithColorFunc RX("mainView", [&]() -> Color { return Color(0x00FFFF).Scale(Device::Rocker::GetX(1) + 128); }, [&]() -> void {}, [&]() -> void {});
    keypadUI.AddUIComponent(RX, Point(12, 1));

    UIButtonWithColorFunc RY("mainView", [&]() -> Color { return Color(0x00FFFF).Scale(Device::Rocker::GetY(1) + 128); }, [&]() -> void {}, [&]() -> void {});
    keypadUI.AddUIComponent(RY, Point(13, 1));

    UIButtonWithColorFunc RB(
        "mainView", [&]() -> Color { return Color(0x00FFFF).Scale(Device::Rocker::GetButton(1) * 60); }, [&]() -> void {},
        [&]() -> void {});
    keypadUI.AddUIComponent(RB, Point(14, 1));

    UIButtonWithColorFunc RP("mainView", [&]() -> Color { return Color(0x00FFFF).Scale(Device::Rocker::GetPressure(1) * 2); }, [&]() -> void {}, [&]() -> void {});
    keypadUI.AddUIComponent(RP, Point(15, 1));        

    keypadUI.Start();
    Exit();



}

void HIDtest::Loop() {
    // Do nothing
}