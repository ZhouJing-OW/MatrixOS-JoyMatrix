#include "FactoryMenu.h"

void BurnEFuse();  // This is in device folder, a custom BurnEFuse will be provided

void FactoryMenu::EFuseBurner() {
#ifdef EFUSE_BURNER
  if (true)
  {
    UI efuseConfirm("eFuse Burn Confirmation", Color(0xFFFFFF));

    UIButtonLarge confirmBtn("Confirm", Color(0x00FF00), Dimension(4, 2), [&]() -> void {
      BurnEFuse();
      efuseConfirm.Exit();
    });
    efuseConfirm.AddUIComponent(confirmBtn, Point(2, 1));

    UIButtonLarge cancelBtn("Cancel", Color(0xFF0000), Dimension(4, 2), [&]() -> void { efuseConfirm.Exit(); });
    efuseConfirm.AddUIComponent(cancelBtn, Point(10, 1));

    efuseConfirm.Start();
  }
  else
  {
    MatrixOS::LED::Fill(0);
    MatrixOS::LED::SetColor(Point(6, 0), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(7, 0), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(8, 0), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(9, 0), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(6, 1), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(9, 1), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(6, 2), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(9, 2), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(6, 3), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(7, 3), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(8, 3), Color(0x00FF00));
    MatrixOS::LED::SetColor(Point(9, 3), Color(0x00FF00));
    MatrixOS::LED::Update();
    MatrixOS::SYS::DelayMs(2000);
    MatrixOS::LED::Fill(0);
  }
#else  // Not in factory mode or not ESP32
  MatrixOS::LED::Fill(0);
  MatrixOS::LED::SetColor(Point(6, 0), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(7, 0), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(8, 0), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(9, 0), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(6, 1), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(9, 1), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(6, 2), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(9, 2), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(6, 3), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(7, 3), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(8, 3), Color(0xFF00FF));
  MatrixOS::LED::SetColor(Point(9, 3), Color(0xFF00FF));
  MatrixOS::LED::Update();
  MatrixOS::SYS::DelayMs(2000);
  MatrixOS::LED::Fill(0);
#endif
}