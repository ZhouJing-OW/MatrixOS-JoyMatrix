#pragma once
#include "MatrixOS.h"

class EncoderEvent
{
public:
    std::queue<uint8_t> pin;
    KnobConfig* knob = nullptr;
    int8_t* val;
    uint8_t state;

    const uint8_t highSpeed = 3; // The encoder changes the value with each pulse in high speed mode. 

    void setup(KnobConfig* config){
        this->knob = config;
        if (knob->min <= knob->max){
            val = &knob->byte2;
            if (*val < knob->min) *val = knob->min;
            if (*val > knob->max) *val = knob->max;
            if (knob->def < knob->min) knob->def = knob->min;
            if (knob->def > knob->max) knob->def = knob->max;
            knob->enable = true;
        } else
            knob->enable = false;
    }

    void push(uint8_t pin) {
        if(this->pin.size() == 0 || (this->pin.back() != pin)){
            this->pin.push(pin); 
        }
    }

    bool decode(){
        if (pin.size() > 0)
        {
        uint8_t m = pin.front();
        int8_t min = knob->min;
        int8_t max = knob->max;
        bool shift = Device::KeyPad::ShiftActived();
        bool wide = max - min > 24;
        switch (state)
        {
            case 0:  // 00 filter
            if (m != 0){
                state = m;
            }
            break;
            case 1:  // 01
            if (m != 1){
                if (m == 0){
                state = 0;
                if (*val + highSpeed < max && wide && !shift) {
                    *val = *val + highSpeed;
                    Callback();
                }
                if (pin.size() > 16 && *val + 1 < max && shift) {
                    *val = *val + 1;
                    Callback();
                }
                } else
                state = m;
            }
            break;
            case 2:  // 10
            if (m != 2){
                if (m == 0){
                state = 0;
                if (*val - highSpeed >= min && wide && !shift) {
                    *val = *val - highSpeed;
                    Callback();
                }
                if (pin.size() > 16 && *val - 1 >= min && shift) {
                    *val = *val - 1;
                    Callback();
                }
                } else
                state = m;
            }
            break;
            case 3:  // 11
            if (m != 3){
                if (m == 1){
                state = 1;  // 读入信息为01，表示正转
                if (*val < max){
                    if (*val + highSpeed <= max && wide && !shift) *val = *val + highSpeed;
                    else if (*val + 1 <= max && shift) *val = *val + 1;
                    Callback();
                }
                }
                if (m == 2){
                state = 2;  // 读入信息为10，表示反转
                if (*val > min){
                    if (*val - highSpeed >= min && wide && !shift) *val = *val - highSpeed;
                    else if (*val - 1 >= min && shift) *val = *val - 1;
                    Callback();
                }
                }
                if (m == 0){
                state = 0;
                }
            }
            break;
        }
        pin.pop();
        return true;
        }
        return false;
    }

    void Callback() {
        MatrixOS::Component::Knob_Function(knob);
    }
};