#include "UI.h"

UI::UI(string name, Color color, bool newLedLayer) {
  this->name = name;
  this->nameColor = color;
  this->newLedLayer = newLedLayer;
}



// TODO, make new led layer
void UI::Start() {
  status = 0;
  if (MatrixOS::UserVar::brightness.value > 0)
  {
    for (; MatrixOS::UserVar::brightness.value > 0; MatrixOS::UserVar::brightness.value--)
    {
      uint32_t ms = std::max(FADE_INOUT_TIME / MatrixOS::UserVar::currentBrightness.value, 1);
      MatrixOS::SYS::DelayMs(ms);
    }
  }
  
  if (newLedLayer)    
    MatrixOS::LED::CreateLayer();
  MatrixOS::KEYPAD::Clear();
  Setup();

  // ----------- common bar ---------- //

  UIButtonWithColorFunc playBtn(
      "Play / Metronome", [&]() -> Color { return !(Device::rightShift | Device::leftShift) ? Color(0x00FF00).Scale(Device::playState ? 255 : 36) : Color(0x0000FF).Scale(Device::metronomeState ? 255 : 36); },
      [&]() -> void {},
      []() -> void {},
      [&]() -> void {
        if((Device::rightShift | Device::leftShift))
        {
          Device::metronomeState = !Device::metronomeState;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 124, Device::metronomeState ? 127 : 0));
        }
        else
        {
          Device::playState = !Device::playState;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 121, Device::playState ? 127 : 0));
        };
      },
      [&]() -> void {
        if((Device::rightShift | Device::leftShift))
        {
          Device::metronomeState = false;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 124, 0));
        }
        else
        {
          Device::playState = false;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 121, 0));
        };
      }
      );
  AddUIComponent(playBtn, Point(0, 4));

  UIButtonWithColorFunc recordBtn(
      "Record / AutoGrouth", [&]() -> Color { return !(Device::rightShift | Device::leftShift) ? Color(0xFF3000).Scale(Device::recordState ? 255 : 36) : Color(0xFF9900).Scale(Device::autoGrouthState ? 255 : 36); }, 
      [&]() -> void {
        if((Device::rightShift | Device::leftShift))
        {
          Device::autoGrouthState = false;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 125, 0));
        }
      },[]() -> void {},
      [&]() -> void {
        if((Device::rightShift | Device::leftShift))
        {
          Device::autoGrouthState = true;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 125, 127));
        }
        else
        {
          Device::recordState = !Device::recordState;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 122, Device::recordState ? 127:0)); 
        }
      },
      [&]() -> void {
        if((Device::rightShift | Device::leftShift))
        {
          Device::autoGrouthState = false;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 125, 0));
        }
        else
        {
          Device::recordState = false;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 122, 0)); 
        }
      }
      );
  AddUIComponent(recordBtn, Point(1, 4));

  UIButtonWithColorFunc muteBtn(
      "Mute / Undo",  [&]() -> Color { return !(Device::rightShift | Device::leftShift) ? Color(0xFF0000).Scale(Device::muteState ? 255 : 36) : Color(0xFF900FF).Scale(Device::undoState ? 255 : 36); }, 
      [&]() -> void {
        if((Device::rightShift | Device::leftShift))
        {
          Device::undoState = false;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 127, 0));
        }
      },[]() -> void {},
      [&]() -> void { 
        if((Device::rightShift | Device::leftShift))
        {
          Device::undoState = true;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 127, 127));
        }
        else
        {
          Device::muteState = !Device::muteState;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 126, Device::muteState ? 127 : 0)); 
        }
      },
      [&]() -> void {
        if((Device::rightShift | Device::leftShift))
        {
          Device::undoState = false;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 127, 0));
        }
        else
        {
          Device::muteState = false;
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, 126, 0)); 
        }
      }
      );
  AddUIComponent(muteBtn, Point(2, 4));

  // ---------- common bar ---------- //

  while (status != -1)
  {
    LoopTask();
    Loop();
    RenderUI();

  }
  End();
  UIEnd();
}

void UI::Exit() {
  MLOGD("UI", "UI Exit signaled");
  status = -1;
}

void UI::LoopTask() {
  GetKey();
  if (!disableExit && MatrixOS::SYS::FNExit == true) Exit();
  if (MatrixOS::UserVar::brightness.value < MatrixOS::UserVar::currentBrightness.value) {
    MatrixOS::UserVar::brightness.value ++;
    uint32_t ms = std::max(FADE_INOUT_TIME / MatrixOS::UserVar::currentBrightness.value, 1);
    MatrixOS::SYS::DelayMs(ms);
  }
}

void UI::RenderUI() {
  if (uiTimer.Tick(uiUpdateMS) || needRender)
  {
    needRender = false;
    // MatrixOS::LED::Fill(0);
    for (auto const& uiComponentPair : uiComponentMap)
    {
      Point xy = uiComponentPair.first;
      UIComponent* uiComponent = uiComponentPair.second;
      uiComponent->Render(xy);
    }
    Render();
    MatrixOS::LED::Update();
  }
}

void UI::GetKey() {
  struct KeyEvent keyEvent;
  while (MatrixOS::KEYPAD::Get(&keyEvent))
  {
    // MLOGD("UI", "Key Event %d %d", keyEvent.id, keyEvent.info.state);
    if (!CustomKeyEvent(&keyEvent)) //Run Custom Key Event first. Check if UI event is blocked
      UIKeyEvent(&keyEvent);
    else
      MLOGD("UI", "KeyEvent Skip: %d", keyEvent.id);
  }
}

void UI::UIKeyEvent(KeyEvent* keyEvent) {
  // MLOGD("UI Key Event", "%d - %d", keyID, keyInfo->state);
  if (keyEvent->id == FUNCTION_KEY)
  {
    if (!disableExit && keyEvent->info.state == RELEASED)
    {
      MLOGD("UI", "Function Key Exit");
      Exit();
      return;
    }
    if (!disableExit && keyEvent->info.state == HOLD)
    {
      MLOGD("UI", "Function Key ExitAPP");
      MatrixOS::SYS::FNExit = true;
      Exit();
      return;
    }
  }
  Point xy = MatrixOS::KEYPAD::ID2XY(keyEvent->id);
  if (xy)
  {
    // MLOGD("UI", "UI Key Event X:%d Y:%d", xy.x, xy.y);
    bool hasAction = false;
    for (auto const& uiComponentPair : uiComponentMap)
    {
      Point relative_xy = xy - uiComponentPair.first;
      UIComponent* uiComponent = uiComponentPair.second;
      if (uiComponent->GetSize().Contains(relative_xy))  // Key Found
      { hasAction |= uiComponent->KeyEvent(relative_xy, &keyEvent->info); }
    }
    // if(hasAction)
    // { needRender = true; }
    if (this->name.empty() == false && hasAction == false && keyEvent->info.state == HOLD && Dimension(Device::x_size, Device::y_size).Contains(xy))
    { MatrixOS::UIInterface::TextScroll(this->name, this->nameColor);
    }
  }
}

void UI::AddUIComponent(UIComponent* uiComponent, Point xy) {
  // ESP_LOGI("Add UI Component", "%d %d %s", xy.x, xy.y, uiComponent->GetName().c_str());
  // uiComponents.push_back(uiComponent);
  uiComponentMap[xy] = uiComponent;
}

void UI::AddUIComponent(UIComponent* uiComponent, uint16_t count, ...) {
  // uiComponents.push_back(uiComponent);
  va_list valst;
  va_start(valst, count);
  for (uint8_t i = 0; i < count; i++)
  { uiComponentMap[(Point)va_arg(valst, Point)] = uiComponent;
  }
}

void UI::RemoveUIComponent(UIComponent* uiComponent, Point xy){
  Dimension dimension = uiComponent->GetSize();
  for (uint16_t x = 0; x < dimension.x; x++)
  {
    for (uint16_t y = 0; y < dimension.y; y++)
    { MatrixOS::LED::SetColor(xy + Point(x, y), 0x000000);
    }
  }
  uiComponentMap.erase(xy);
}

void UI::AllowExit(bool allow) {
  disableExit = !allow;
}

void UI::SetSetupFunc(std::function<void()> setup_func) {
  UI::setup_func = &setup_func;
}

void UI::SetLoopFunc(std::function<void()> loop_func) {
  UI::loop_func = &loop_func;
}

void UI::SetEndFunc(std::function<void()> end_func) {
  UI::end_func = &end_func;
}

void UI::SetKeyEventHandler(std::function<bool(KeyEvent*)> key_event_handler){
  UI::key_event_handler = &key_event_handler;
}

void UI::ClearUIComponents() {
  uiComponentMap.clear();
}

void UI::UIEnd() {
  MLOGD("UI", "UI Exited");
  for (; MatrixOS::UserVar::brightness.value > 0; MatrixOS::UserVar::brightness.value--){
    uint32_t ms = std::max(FADE_INOUT_TIME / MatrixOS::UserVar::currentBrightness.value, 1);
    MatrixOS::SYS::DelayMs(ms);
  }
  
  if (newLedLayer)
  { 
    MatrixOS::LED::DestoryLayer(); 
  }
  else
  { MatrixOS::LED::Fill(0); }

  MatrixOS::KEYPAD::Clear();
  // MatrixOS::LED::Update();
}

void UI::SetFPS(uint16_t fps)
{
  if (fps == 0)
    uiUpdateMS = UINT32_MAX;
  else
    uiUpdateMS = 1000 / fps;
}

