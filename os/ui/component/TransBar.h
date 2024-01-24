#pragma once
#include "UIComponent.h"
#include "MatrixOS.h"

class TransBar : public UIComponent {
public:
    TransportState* state = nullptr;
    Point position= Point(0,0);

    struct Transport {
        string name;
        Color color = COLOR_BLANK;
        uint8_t midiCC;
        bool* state = nullptr;
    }transport[2][3];

    TransBar(){};
    TransBar(TransportState* state){
        this->state = state;
        TransInit();
    }
    void Setup(TransportState* state){
        this->state = state;
        TransInit();
    }

    void TransInit(){
        transport[0][0] = {"Play", COLOR_LIME, 121, &state->play} ;
        transport[0][1] = {"Record", COLOR_ORANGE, 122, &state->record} ;
        transport[0][2] = {"Mute", COLOR_RED, 124, &state->mute};
        transport[1][0] = {"Metronome", COLOR_BLUE, 125, &state->metronome};
        transport[1][1] = {"Auto Grouth", COLOR_LIME, 126, &state->autoGrouth};
        transport[1][2] = {"Undo", COLOR_YELLOW, 127, &state->undo};
    }
    
    virtual Color GetColor() { return transport[0][0].color; }
    virtual Dimension GetSize() { return Dimension(3, 1); }

    virtual bool Render(Point origin) {
        position = origin;
        if (state != nullptr) {
            for (int x = 0; x < 3; x++)
            {
                bool shift = Device::KeyPad::ShiftActived();
                Color color;
                if (*transport[shift][x].state == true){
                    color = transport[shift][x].color.Blink(true, 0, 1000, 3, 1);
                } else {
                    color = transport[shift][x].color.ToLowBrightness();
                }
                Point xy = origin + Point(x, 0);
                MatrixOS::LED::SetColor(xy, color);
            }                         
            return true;
        }
        return false;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
        if (state != nullptr) {
            bool shift = Device::KeyPad::ShiftActived();
        
            if (keyInfo->state == HOLD){
                MatrixOS::UIInterface::TextScroll(transport[shift][xy.x].name, transport[shift][xy.x].color);
                return true;
            }

            if (keyInfo->state == RELEASED && keyInfo->hold == false){
                *transport[shift][xy.x].state = !*transport[shift][xy.x].state;
                // MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, transport[shift][xy.x].midiCC, *transport[shift][xy.x].state ? 127 : 0));
                MatrixOS::MIDI::Hold(position, SEND_CC, 0, transport[shift][xy.x].midiCC);
                return true;
            }
        }
        return false;
    }
};
