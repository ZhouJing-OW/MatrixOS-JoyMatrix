#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class MainBar : public UIComponent {
public:

    struct Transport {
        string name;
        Color color = COLOR_BLANK;
        uint8_t midiCC;
        bool* state = nullptr;
    };

    Transport transport[2][3] = {
    {   Transport{"Play", COLOR_LIME, 121, &Device::playState}, 
        Transport{"Record", COLOR_ORANGE, 122, &Device::recordState}, 
        Transport{"Mute", COLOR_RED, 124, &Device::muteState}},
    {   Transport{"Metronome", COLOR_BLUE, 125, &Device::metronomeState}, 
        Transport{"Auto Grouth", COLOR_YELLOW, 126, &Device::autoGrouthState}, 
        Transport{"UNDO", COLOR_FUCHSIA, 127, &Device::undoState}}
    };

    virtual Dimension GetSize() { return Dimension(3, 1); }

    virtual bool Render(Point origin) {
      for (int x = 0; x < 3; x++)
      {
        bool shift = Device::KeyPad::ShiftActived();
        Color color;
        if (*transport[shift][x].state == true){
            color = transport[shift][x].color;
        } else {
            color = transport[shift][x].color.ToLowBrightness();
        }
        Point xy = origin + Point(x, 0);
        MatrixOS::LED::SetColor(xy, color);
      }                         
      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
        bool shift = Device::KeyPad::ShiftActived();
        if (keyInfo->state == PRESSED){
            *transport[shift][xy.x].state = !*transport[shift][xy.x].state;
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, transport[shift][xy.x].midiCC, *transport[shift][xy.x].state ? 127 : 0));
            return true;
        }

        if (keyInfo->state == HOLD){
            MatrixOS::UIInterface::TextScroll(transport[shift][xy.x].name, transport[shift][xy.x].color);
            return true;
        }
        // if (keyInfo->state == RELEASED){
        //     *transport[shift][xy.x].state = false;
        //     MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, transport[shift][xy.x].midiCC, 0));  
        //     return true;
        // }
        return false;
    }
};
