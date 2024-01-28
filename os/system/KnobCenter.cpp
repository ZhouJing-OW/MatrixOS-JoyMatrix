#include "KnobCenter.h"

#define knob_center_scanrate 5

namespace MatrixOS::KnobCenter
{
  const string suffix = "knb";
  TaskHandle_t knobCenterTaskHandle;

  void KnobCenterTask(void* arg) 
  {
    for (;;)
    {
      for (uint8_t ch = 0; ch < 16; ch++)  // set Knob color
      {
        if (knobAll[ch * knobsPerChannel].color != *color[ch])
        {
          knobAll[ch * knobsPerChannel].color = *color[ch];
          MarkChanged(ch * knobsPerChannel);
        }
      }

      if (Device::AnalogInput::GetDialPtr() == nullptr && Device::Encoder::GetActEncoder() == 255)  // save changed knob
      {
        if (OpenFile(appName))
        {
          while (!changedKnobs.empty())
          {
            auto it = changedKnobs.begin();
            SaveKnob(knobAll[*it]);
            changedKnobs.erase(it);
          }
          fio.close();
        }
      }
      vTaskDelay(configTICK_RATE_HZ / knob_center_scanrate);
    }
  }

  bool RequestService(string name ,Color *channelColor) 
  {
    appName = name;
    MLOGD( appName , "Knob Center Service Requested.");
    for (uint8_t ch = 0; ch < 16; ch++)
    {
      color[ch] = channelColor + ch;
    }
    if(LoadKnobFiles(name))
    {
      xTaskCreate(KnobCenterTask, "KnobCenter", configMINIMAL_STACK_SIZE * 2, NULL, 3, &knobCenterTaskHandle);
      return true;
    }
    else
      return false;
    }

  void EndService()
  {
    while (!changedKnobs.empty()){vTaskDelay(50 / portTICK_PERIOD_MS);}
    vTaskDelete(knobCenterTaskHandle);
    appName = "";
    DisableAll();
    free(knobAll);
    knobAll = nullptr;
    knobCount = 0;
    knobsPerChannel = 0;
    pageMax = 0;
    extraPage = 0;
    MLOGD("Knob Center", "Service ended.");
  }

  bool LoadKnobFiles(string name)
  {
    knobAll = (KnobConfig*)MatrixOS::FATFS::LoadFile(sizeof(KnobConfig), knobCount, name, suffix);
    if (knobAll != nullptr)
    {
      knobsPerChannel = knobCount / 16;
      pageMax = knobsPerChannel / ENCODER_NUM - (knobsPerChannel % ENCODER_NUM == 0);
      return true;
    }
    else
      return false;
  }

  void SaveKnob(KnobConfig &knob)
  { 
    MatrixOS::FATFS::SavePart(&knob, sizeof(KnobConfig), knob.pos, fio);
  }

  bool OpenFile(string name)
  {
    return MatrixOS::FATFS::OpenFile(name, suffix, fio);
  }

  void SaveKnobContinuous(KnobConfig& knob)
  { 
    knob.pos = fio.tellp() / sizeof(KnobConfig);
    MatrixOS::FATFS::SaveContinuous(&knob, sizeof(KnobConfig), fio);
    // MLOGD("knob Center", "Init Knob pos: %d", knob.pos);
  }

  void CloseFile() { fio.close(); }

  void GetKnobPtrs(std::vector<uint16_t>& pos, std::vector<KnobConfig*>& knobPtr)
  {
    knobPtr.clear();
    knobPtr.reserve(pos.size());
    for(uint16_t i = 0; i < pos.size(); i++)
    {
      if(pos[i] < knobCount)
        knobPtr.push_back(&knobAll[pos[i]]);
      else 
        knobPtr.push_back(nullptr);
    }
  }
  
  void MarkChanged(uint16_t pos)
  {
    if(pos < knobCount)
      changedKnobs.emplace(pos);
  }

  void SetKnobBar(std::vector<uint16_t>& pos)
  {
    std::vector<KnobConfig*> tempKnob;
    tempKnob.reserve(pos.size());
    for (uint16_t i = 0; i < ENCODER_NUM; i++)
    {
      if (i < pos.size() && pos[i] < knobCount)
        tempKnob.push_back(&knobAll[pos[i]]);
    }
    knobBar.Setup(tempKnob);
    channelMode = false;
  }

  void SetKnobBar(std::vector<KnobConfig*>& knob) { knobBar.Setup(knob); channelMode = false; }

  void ChannelMode() 
  { 
    channelMode = true; 
    Device::AnalogInput::SetLeftRight(&currentPage, pageMax + extraPage, 0, 1, true);
  }

  void SetPage(uint8_t page)
  {
    if (page <= pageMax + extraPage)
      currentPage = page;
    else page = pageMax + extraPage;
    Device::AnalogInput::SetLeftRight(&currentPage, pageMax + extraPage, 0, 1, true);
  }

  void AddExtraPage(std::vector<KnobConfig*>& knob)
  {
    extraPage = knob.size() / ENCODER_NUM + (knob.size() % ENCODER_NUM > 0);
    knobBar.Setup(knob);
    Device::AnalogInput::SetLeftRight(&currentPage, pageMax + extraPage, 0, 1, true);
  }

  void DisableExtraPage()
  {
    if (extraPage == 0) return;
    extraPage = 0;
    knobBar.knobPtr.clear();
    if(currentPage > pageMax)
      currentPage = 0;
    Device::AnalogInput::SetLeftRight(&currentPage, pageMax + extraPage, 0, 1, true);
  }

  bool HaveExtraPage() { return extraPage > 0; }

  uint8_t GetPage() { return currentPage; }

  void DisableAll() { DisableExtraPage(); knobBar.DiableAll(); }

  void AddKnobBarTo(UI &ui) { ui.AddUIComponent(knobBar, knobOrigin); }

  void Knob_Function(KnobConfig* knob) 
  {
    MarkChanged(knob->pos);
    switch (knob->type) {
      case SEND_NONE: break;
      case SEND_CC: 
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, knob->channel, knob->byte1, knob->byte2)); 
        break;
      case SEND_PC: 
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, knob->channel, 0, knob->byte1));
        MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, knob->channel, knob->byte2, knob->byte2)); 
        break;
    }
  }
}

