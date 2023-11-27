#include "MatrixOS.h"
#include "Device.h"
#include "timers.h"

namespace Device::Rocker
{
  #define ADS1115_SENSOR_ADDR_1       0x90
  #define ADS1115_SENSOR_ADDR_2       0x92

  StaticTimer_t rocker_timer_def;
  TimerHandle_t rocker_timer;

  uint8_t mux = 0;
  uint8_t defalutChannel = 0;
  uint8_t* ch = &defalutChannel;
  bool modWheel[5];
  bool modLock[5];
  bool pitchWheelState = false;


  //0:left 1:right
  uint16_t X[2];
  uint16_t X_Max[2] = {20000, 20000};
  uint16_t X_Min[2] = {6000, 6000};
  uint16_t X_Middle[2] = {13000, 13000};
  uint16_t X_deadZone[2]  = {1000, 1000};

  uint16_t Y[2];
  uint16_t Y_Max[2] = {20000, 20000};
  uint16_t Y_Min[2] = {6000, 6000};
  uint16_t Y_Middle[2] = {13000, 13000};
  uint16_t Y_deadZone[2] = {1000, 1000};

  uint16_t Button[2];
  uint16_t Button_Shift[2] = {15590, 15550};
  uint16_t Button_Rocker[2] = {20800, 21350};
  uint16_t Button_Both[2] = {22050, 22450};

  uint16_t Pressure[2];
  uint16_t Pressure_Max[2] = {12900, 13400};
  uint16_t Pressure_Min[2] = {10100, 12100};

  uint16_t readData[8];

  void Scan(){

    readData[mux] = Device::I2C::ADS1115_Read(ADS1115_SENSOR_ADDR_1);
    readData[mux + 4] = Device::I2C::ADS1115_Read(ADS1115_SENSOR_ADDR_2);

    switch(mux){
      case 0: { // X
        int8_t lx = GetX(0);
        int8_t ly = GetY(0);

        if (lx + ly){
          pitchWheelState = true;
          int16_t change = ly * 64 + lx * 5.3;
          if (change > 0x1FFE) change = 0x1FFE;
          if (change < - 0x1FFE) change = - 0x1FFE;

          uint16_t pitch = 0x1FFF + change;
          MatrixOS::MIDI::Send(MidiPacket(0, PitchChange, *ch, pitch));
        }
        else if (lx == 0 && ly == 0 && pitchWheelState == true)
        {
          MatrixOS::MIDI::Send(MidiPacket(0, PitchChange, *ch, 0x1FFE));
          pitchWheelState = false;
        }

        break;
      }

      case 1: { // Y
        int8_t rx = GetX(1);
        int8_t ry = GetY(1);

        ModWheelLock();

        if (ry > 0){
          ModWheelOff(2);
          modWheel[1] = true;
          if (!modLock[1]) MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, *ch, 1, ry));
        } 
        if ( ry < 0){
          ModWheelOff(1);
          modWheel[2] = true;
          if (!modLock[2]) MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, *ch, 2, -ry));
        } 
        if (rx > 0){
          ModWheelOff(4);
          modWheel[3] = true;
          if (!modLock[3]) MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, *ch, 3, rx));
        } 
        if (rx < 0){
          ModWheelOff(3);
          modWheel[4] = true;
          if (!modLock[4]) MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, *ch, 4, - rx));
        } 
        if (rx == 0 && ry == 0) {
          ModWheelOff(1);
          ModWheelOff(2);
          ModWheelOff(3);
          ModWheelOff(4);
          for (uint8_t n = 1; n < 5;n++)
          {
            modLock[n] = false;
          }
        }

        break;
      }

      case 2: { // Button

        switch(GetButton(0)){
          case 0:{
            leftShift = false;
            leftRocker = false;
            break;
          }
          case 1:{
            leftShift = true;
            leftRocker = false;
            break;
          }
          case 2:{
            leftShift = false;
            leftRocker = true;
            break;
          }
          case 3:{
            leftShift = true;
            leftRocker = true;
            break;                        
          }
        }

        switch(GetButton(1)){
          case 0:{
            rightShift = false;
            rightRocker = false;
            break;
          }
          case 1:{
            rightShift = true;
            rightRocker = false;
            break;
          }
          case 2:{
            rightShift = false;
            rightRocker = true;
            break;
          }
          case 3:{
            rightShift = true;
            rightRocker = true;
            break;   
          }
        }
      }

      case 3: { // Pressure
        uint8_t lp = GetPressure(0);
        uint8_t rp = GetPressure(1);
        uint8_t p = lp > rp ? lp : rp;
        uint8_t p2 = p * p / 128;
        Device::pressure = p2 > 1 ? p2 : 1;
        // MLOGD("Left Pressure", "Printing %d", readData[3]);
        // MLOGD("Right Pressure", "Printing %d", readData[7]);
      }
    }

    mux++;
    if (mux > 3) mux = 0; 
    Device::I2C::ADS1115_Setting(ADS1115_SENSOR_ADDR_1, 0x04 + mux);
    Device::I2C::ADS1115_Setting(ADS1115_SENSOR_ADDR_2, 0x04 + mux);
  }

  void Init(){
    rocker_timer = xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::rocker_scanrate, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(Scan), &rocker_timer_def);
    xTimerStart(rocker_timer, 0);
  }

  int8_t GetX(uint8_t LR){
    X[0] = readData[1];
    X[1] = readData[5];

    if (X[LR] <= X_Min[LR]) {
      return -127;
    } else if ((X[LR] > X_Min[LR]) && (X[LR] < X_Middle[LR] - X_deadZone[LR])) {
      return - ((X_Middle[LR] - X_deadZone[LR] - X[LR]) * 127 / (X_Middle[LR] - X_deadZone[LR] - X_Min[LR]));
    } else if ((X[LR] >= X_Middle[LR] - X_deadZone[LR]) && (X[LR] <= X_Middle[LR] + X_deadZone[LR])){
      return 0;
    } else if ((X[LR] > X_Middle[LR] + X_deadZone[LR]) && (X[LR] < X_Max[LR])){
      return (X[LR] - (X_Middle[LR] + X_deadZone[LR])) * 127 / (X_Max[LR] - (X_Middle[LR] + X_deadZone[LR]));
    } else return 127;
  }

  int8_t GetY(uint8_t LR){
    Y[0] = readData[0];
    Y[1] = readData[4];

    if (Y[LR] <= Y_Min[LR]) {
      return -127;
    } else if ((Y[LR] > Y_Min[LR]) && (Y[LR] < Y_Middle[LR] - Y_deadZone[LR])) {
      return - ((Y_Middle[LR] - Y_deadZone[LR] - Y[LR]) * 127 / (Y_Middle[LR] - Y_deadZone[LR] - Y_Min[LR]));
    } else if ((Y[LR] >= Y_Middle[LR] - Y_deadZone[LR]) && (Y[LR] <= Y_Middle[LR] + Y_deadZone[LR])){
      return 0;
    } else if ((Y[LR] > Y_Middle[LR] + Y_deadZone[LR]) && (Y[LR] < Y_Max[LR])){
      return (Y[LR] - (Y_Middle[LR] + Y_deadZone[LR])) * 127 / (Y_Max[LR] - (Y_Middle[LR] + Y_deadZone[LR]));
    } else return 127;
  }

  uint8_t GetButton(uint8_t LR){ // LR 0:L 1:R , return 0:none 1:shift 2:rocker 3:Both
    Button[0] = readData[2];
    Button[1] = readData[6];

    if (Button[LR] > Button_Shift[LR] / 2 && Button[LR] < (Button_Shift[LR] + Button_Rocker[LR]) / 2){
      return 1;
    } else if ((Button[LR] > (Button_Shift[LR] + Button_Rocker[LR]) / 2) && (Button[LR] <= (Button_Rocker[LR] + Button_Both[LR]) / 2)) {
      return 2;
    } else if ((Button[LR] > (Button_Rocker[LR] + Button_Both[LR]) / 2) && (Button[LR] < (Button_Both[LR] + 0xFFFF) / 2)){
      return 3;  
    } else return 0;
  }

  uint8_t GetPressure(uint8_t LR){
    Pressure[0] = readData[3];
    Pressure[1] = readData[7];

    if (Pressure[LR] < Pressure_Min[LR]){
      return 1;
    } else if (Pressure[LR] > Pressure_Max[LR]){
      return 127;
    } else {
      return (Pressure[LR] - Pressure_Min[LR]) * 127 / (Pressure_Max[LR] - Pressure_Min[LR]);
    }
  }

  void SetChannel(uint8_t* channel){
    ch = channel;
  }

  void ModWheelOff(uint8_t n){
    if (modWheel[n]){
      if (!modLock[n]) MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, *ch, n, 0));
      modWheel[n] = false;
    }
  }

  void ModWheelLock(){
    if ( rightRocker ){
        for (uint8_t n = 1; n < 5; n++)
        {
          if (modLock[n] == false && modWheel[n])
            modLock[n] = true;
        }
    }
  }
}