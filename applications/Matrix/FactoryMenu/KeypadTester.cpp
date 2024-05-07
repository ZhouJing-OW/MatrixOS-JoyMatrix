#include "FactoryMenu.h"

class KeyTestUI : public UIComponent 
{
  public:
  Dimension dimension;
  uint8_t** keypad_tested;
  Color** keyColor;
  bool colorSelectMode = false;
  Point settingPoint = {0, 0};

  Color GetBrightness(Color color, uint8_t index)
  {
    switch (index)
    {
      case 0: return Color(0);
      case 1: return color;
      case 2: return color.ToLowBrightness();
      case 3: return color.Scale(8);
      default: return Color(0);
    }
  }

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
          Point xy = Point(x, y) + origin;
          MatrixOS::LED::SetColor(xy, GetBrightness(keyColor[x][y], keypad_tested[x][y])); 
        }
      }
      return true;
    }
    else
    {
      for (uint8_t x = 0; x < dimension.x; x++) {
        for (uint8_t y = 0; y < dimension.y; y++)
            MatrixOS::LED::SetColor(Point(x, y), Color(BLANK));
        MatrixOS::LED::SetColor(Point(x, 0), COLOR_HIGH_S[x]);
        MatrixOS::LED::SetColor(Point(x, 1), COLOR_LOW_S[x]);
        MatrixOS::LED::SetColor(Point(x, 2), COLOR_HIGH_L[x]);
      }

      MatrixOS::LED::SetColor(Point(0, 3) + origin, Color(WHITE));
      MatrixOS::LED::SetColor(Point(15, 3) + origin, GetBrightness(Color(0xFFFFFF), keypad_tested[settingPoint.x][settingPoint.y])); 

      for (uint8_t x = 6; x < 10; x++) 
      {
          MatrixOS::LED::SetColor(Point(x, 3) + origin, GetBrightness(keyColor[settingPoint.x][settingPoint.y],keypad_tested[settingPoint.x][settingPoint.y]));
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
        if (keypad_tested[xy.x][xy.y] == 0) keyColor[xy.x][xy.y] = keyColor[settingPoint.x][settingPoint.y];
        keypad_tested[xy.x][xy.y] += 1;
        if (keypad_tested[xy.x][xy.y] > 3) 
          keypad_tested[xy.x][xy.y] = 0;

        return true;
      }

      if(keyInfo->state == HOLD)
      {
        settingPoint = xy;
        if(keypad_tested[xy.x][xy.y] == 0) keypad_tested[xy.x][xy.y] = 1;
        colorSelectMode = true;
        return true;
      }
    }
    else 
    {
      if(keyInfo->state == PRESSED)
      {
        if (xy.y == 0)
          keyColor[settingPoint.x][settingPoint.y] = COLOR_HIGH_S[xy.x];
        if (xy.y == 1)
          keyColor[settingPoint.x][settingPoint.y] = COLOR_LOW_S[xy.x];
        if (xy.y == 2)
          keyColor[settingPoint.x][settingPoint.y] = COLOR_HIGH_L[xy.x];
        else if (xy.x == 0 && xy.y == 3)
          keyColor[settingPoint.x][settingPoint.y] = Color(WHITE);
        else if (xy.x == 15 && xy.y == 3)
        {
          keypad_tested[settingPoint.x][settingPoint.y] += 1;
          if (keypad_tested[settingPoint.x][settingPoint.y] > 3) 
            keypad_tested[settingPoint.x][settingPoint.y] = 1;
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