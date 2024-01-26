#pragma once
#include "UIComponent.h"
#include "MatrixOS.h"

class TransBar : public UIComponent {
public:
    TransportState* state = nullptr;
    Point position= Point(0,0);

    struct TransConfig {
        string name;
        Color color = COLOR_BLANK;
        uint8_t midiCC;
        bool* state = nullptr;
    }transConfig[2][3];

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
        transConfig[0][0] = {"Play", COLOR_LIME, 80, &state->play} ;
        transConfig[0][1] = {"Record", COLOR_ORANGE, 82, &state->record} ;
        transConfig[0][2] = {"Mute", COLOR_RED, 83, &state->mute};
        transConfig[1][0] = {"Metronome", COLOR_BLUE, 86, &state->metronome};
        transConfig[1][1] = {"Auto Grouth", COLOR_LIME, 87, &state->autoGrouth};
        transConfig[1][2] = {"Undo", COLOR_YELLOW, 88, &state->undo};
    }
    
    virtual Color GetColor() { return transConfig[0][0].color; }
    virtual Dimension GetSize() { return Dimension(3, 1); }

    virtual bool Render(Point origin) {
        position = origin;
        if (state != nullptr) {
            for (int x = 0; x < 3; x++)
            {
                bool shift = Device::KeyPad::ShiftActived();
                Color color;
                if (*transConfig[shift][x].state == true){
                    color = transConfig[shift][x].color.Blink(true, 0, 1000, 3, 1);
                } else {
                    color = transConfig[shift][x].color.ToLowBrightness();
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
                MatrixOS::UIInterface::TextScroll(transConfig[shift][xy.x].name, transConfig[shift][xy.x].color);
                return true;
            }

            if (keyInfo->state == RELEASED && keyInfo->hold == false){
                *transConfig[shift][xy.x].state = !*transConfig[shift][xy.x].state;
                // MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, 0, transConfig[shift][xy.x].midiCC, *transConfig[shift][xy.x].state ? 127 : 0));
                MatrixOS::MIDI::Hold(position, SEND_CC, 0, transConfig[shift][xy.x].midiCC);
                return true;
            }
        }
        return false;
    }

};


