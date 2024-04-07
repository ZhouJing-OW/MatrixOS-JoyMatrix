#include "FactoryMenu.h"

class KeyTestUI : public UIComponent 
{
  public:
  Dimension dimension;
  uint8_t** keypad_tested;
  Color** keyColor;
  bool colorSelectMode = false;
  Point settingPoint = {0, 0};

  KeyTestUI(Dimension dimension = Dimension(Device::x_size, Device::y_size))
  {
    this->dimension = dimension; 

    keypad_tested = new uint8_t*[dimension.x];
    keyColor = new Color*[dimension.x];
    for (uint8_t x = 0; x < dimension.x; x++) {
      keypad_tested[x] = new uint8_t[dimension.y];
      keyColor[x] = new Color[dimension.y];
    }

    for (uint8_t x = 0; x < dimension.x; x++) {
      for (uint8_t y = 0; y < dimension.y; y++) {
        keypad_tested[x][y] = 0;
        keyColor[x][y] = Color(0xFFFFFF);
      }
    }
  }
  
  ~KeyTestUI() {
    for (uint8_t x = 0; x < dimension.x; x++) {
      delete[] keypad_tested[x];
      delete[] keyColor[x];
    }
    delete[] keypad_tested;
    delete[] keyColor;
  }

  virtual Color GetColor(uint8_t x, uint8_t y) { return 0xFFFFFF; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin)
  {
    if(!colorSelectMode)
    {
      for (uint8_t x = 0; x < dimension.x; x++) {
        for (uint8_t y = 0; y < dimension.y; y++) {
          if (keypad_tested[x][y])
            MatrixOS::LED::SetColor(Point(x, y) + origin, keyColor[x][y].ToLowBrightness(keypad_tested[x][y] == 2));
          else
            MatrixOS::LED::SetColor(Point(x, y) + origin, Color(0x000000));
        }
      }
      return true;
    }
    else
    {
      for (uint8_t x = 0; x < dimension.x; x++) {
        for (uint8_t y = 0; y < dimension.y; y++)
            MatrixOS::LED::SetColor(Point(x, y), COLOR_BLANK);
        MatrixOS::LED::SetColor(Point(x, 0), COLOR_CONFIG[x]);
      }

      MatrixOS::LED::SetColor(Point(0, 1) + origin, COLOR_WHITE);
      MatrixOS::LED::SetColor(Point(15, 1) + origin, Color(0xFFFFFF).ToLowBrightness(keypad_tested[settingPoint.x][settingPoint.y] == 2));

      for (uint8_t x = 6; x < 10; x++) {
        for (uint8_t y = 2; y < 4; y++)
          MatrixOS::LED::SetColor(Point(x, y) + origin, keyColor[settingPoint.x][settingPoint.y].ToLowBrightness(keypad_tested[settingPoint.x][settingPoint.y] == 2));
      }
      return true;
    }
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
  {
    if(!colorSelectMode)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        if(Device::KeyPad::Shift())
        {
          keypad_tested[xy.x][xy.y] = 0;
          return true;
        }
        switch(keypad_tested[xy.x][xy.y])
        {
          case 0:
            keypad_tested[xy.x][xy.y] = 2;
            keyColor[xy.x][xy.y] = keyColor[settingPoint.x][settingPoint.y];
            break;
          case 1:
            keypad_tested[xy.x][xy.y] = 0;
            break;
          case 2:
            keypad_tested[xy.x][xy.y] = 1;
            break;
        }
        return true;
      }

      if(keyInfo->state == HOLD)
      {
        settingPoint = xy;
        if(keypad_tested[xy.x][xy.y] == 0) keypad_tested[xy.x][xy.y] = 2;
        colorSelectMode = true;
        return true;
      }
    }
    else 
    {
      if(keyInfo->state == PRESSED)
      {
        if (xy.y == 0)
          keyColor[settingPoint.x][settingPoint.y] = COLOR_CONFIG[xy.x];
        else if (xy.x == 0 && xy.y == 1)
          keyColor[settingPoint.x][settingPoint.y] = COLOR_WHITE;
        else if (xy.x == 15 && xy.y == 1)
        {
          keypad_tested[settingPoint.x][settingPoint.y] = keypad_tested[settingPoint.x][settingPoint.y] == 2 ? 1 : 2;
          return true;
        }
        colorSelectMode = false;
        Device::KeyPad::Clear();
        return true;
      }
    }
    return false;
  }
};

void FactoryMenu::KeyPadTester() {
  MatrixOS::KEYPAD::Clear();
  UI keyTest("");
  KeyTestUI keyUI;

  keyTest.AddUIComponent(keyUI, Point(0, 0));
  keyTest.Render();
  keyTest.Start();

  MatrixOS::KEYPAD::Clear();
  MatrixOS::LED::Fill(0);
}