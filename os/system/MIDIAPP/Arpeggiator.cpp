#include "MidiCenter.h"

namespace MatrixOS::MidiCenter
{
  std::map<uint16_t, uint32_t> CNTR_Arp; // midiID, length

  void Arpeggiator::Scan()
  {
    arpInterval = rateToRatio[config->rate] * tickInterval * 24;

    if(!StartCheck()) {
      if (configPrv != *config) {
        configPrv = *config;
        MatrixOS::FATFS::MarkChanged(configRoot, configNum);
      }
      return;
    }

    uint32_t currentTime = MatrixOS::SYS::Millis();
    if(currentTime >= arpTimer + arpInterval - intervalDelta)
    {
      CheckVarChange();
      gateLength = arpInterval * gateToRatio[config->gate];
      gateLength = (gateLength > arpInterval && gateLength > 15) ? gateLength - 10 : gateLength;
      gateLength = gateLength < 5 ? 5 : gateLength;
      if(currentArpNote >= arpArrange.size())
      currentArpNote = 0;
      currentOctave = (arpArrange[currentArpNote] - inputList.begin()->first) / 12;
      intervalDelta = currentTime - (arpTimer + arpInterval - intervalDelta);
      // MLOGD("Arpeggiator", "Time Interval : %f, Interval Delta: %f", arpInterval, intervalDelta);
      arpTimer = currentTime;
      Trigger();
    }
  }

  void Arpeggiator::SetActiveConfig(uint8_t num)
  {
    if (num >= NODES_MAX_CONFIGS) num = NODES_MAX_CONFIGS - 1;
    configNum = num;
    config = &configRoot[num];
    configPtr = { &config->type,           &config->rate,           
                  &config->octaveRange,    &config->noteRepeat,        
                  &config->patternLength,  &config->chance,         
                  &config->gate,           &config->velDecay};
    configPrv = *config;
    nodesConfigNum[channel].insert({NODE_ARP, num});
  }

  void Arpeggiator::CheckVarChange()
  {
    // monitor inputList change. 
    if (inputList != inputListPrv) 
    { 
      if(inputList.size() >= inputListPrv.size())
        decayNow = 0;
      inputListPrv = inputList;
      ArpStart(true);
    }
    
    // monitor config change. 
    if (configPrv != *config) {
      bool needReset = configPrv.NeedReset(*config);
      configPrv = *config;
      MatrixOS::FATFS::MarkChanged(configRoot, configNum);
      ArpStart(needReset);
    }

  };

  void Arpeggiator::Trigger()
  {
    if(GetStep())
    {
      uint8_t velocity = config->velocity[currentStep] > decayNow ? config->velocity[currentStep] - decayNow : 0;
      
      if (velocity > 0)
      {
        auto it = arpArrange.begin() + currentArpNote;
        uint8_t note = *it;
        float velRatio = inputVelocity / 127.0;
        velocity = (uint8_t)(velocity * velRatio);
        uint16_t noteID = SEND_NOTE << 12 | channel << 8 | note;
        CNTR_Arp.insert_or_assign(noteID, MatrixOS::SYS::Millis() + gateLength);
        MidiRouter(NODE_ARP, SEND_NOTE, channel, note, velocity);
      }
      if(currentRepeat >= config->noteRepeat - 1)
      {
        currentArpNote += 1;
        decayNow = decayNow + config->velDecay > 127 ? 127 : decayNow + config->velDecay;
        if(config->type == ARP_Random)
          random_shuffle(arpArrange.begin(), arpArrange.end(), MatrixOS::SYS::RandSeed());
        if(config->type == ARP_RandomOther && currentArpNote < arpArrange.size() - 1)
          random_shuffle(arpArrange.begin() + currentArpNote, arpArrange.end(), MatrixOS::SYS::RandSeed());
      }
    }
    else if(config->skip)
    {
      if(currentRepeat >= config->noteRepeat - 1)
      {
        currentArpNote += 1;
        decayNow = decayNow + config->velDecay > 127 ? 127 : decayNow + config->velDecay;
        if(config->type == ARP_Random)
          random_shuffle(arpArrange.begin(), arpArrange.end(), MatrixOS::SYS::RandSeed());
        if(config->type == ARP_RandomOther && currentArpNote < arpArrange.size() - 1)
          random_shuffle(arpArrange.begin() + currentArpNote, arpArrange.end(), MatrixOS::SYS::RandSeed());
      }
    }

    currentRepeat++;
    if(currentRepeat >= config->noteRepeat)
      currentRepeat = 0;
  }

  void Arpeggiator::ArpEnd(bool reset)
  {
    if(reset)
    {
      currentStep = -1;
      currentOctave = 0;
      currentRepeat = 0;
    }
    arpList.clear();
    arpArrange.clear();
    intervalDelta = 0;
    inputVelocity = 0;
  }

  void Arpeggiator::ArpStart(bool reset)
  {
    ArpEnd(reset);
    GenerateNoteList();
    NoteArrange();
    Reverse();
    if (config->forBackward)
      forBackward();
    
    if(inputVelocity == 0) {
      uint32_t time = 0xFFFFFFFF;
      for(auto it = inputList.begin(); it != inputList.end(); it++) {
        if ( it ->second.time < time) {
          time = it->second.time;
          inputVelocity = it->second.velocity;
        }
      }
    }
  }

  bool Arpeggiator::StartCheck()
  {
    
    if(inputList.empty())
    {
      if(empty) return false;
      currentArpNote = 0;
      decayNow = 0;
      synced = false;
      empty = true;
      ArpEnd(true);
      return false;
    } else empty = false;

    if(synced) return true;

    bool syncNow = false;
    uint32_t timeForSync;
    switch(config->timeSync & timeReceived)
    {
      case 0: 
        syncNow = true; 
        timeForSync = MatrixOS::SYS::Millis();
        break;
      case 1:
        syncNow =!stepTimer.IsLonger(tickInterval * 3);
        timeForSync = MatrixOS::SYS::Millis() - stepTimer.SinceLastTick();
        break;
      // case 2:
      //   syncNow = !beatTimer.IsLonger(20);
      //   timeForSync = MatrixOS::SYS::Millis() - beatTimer.SinceLastTick();
      //   break;
    }

    if(syncNow)
    {
      arpTimer = timeForSync - arpInterval;
      synced = true;
      ArpStart(true);
      return true;
    }
    else
      return false;
  }

  bool Arpeggiator::GetStep()
  {
    currentStep ++;
      if (currentStep >= config->patternLength)
        currentStep = 0;
    bool pattern = bitRead(config->pattern, currentStep);
    srand((unsigned)MatrixOS::SYS::Millis());
    bool random = rand() % 101 <= config->chance;
    return pattern && random;
  }

  void Arpeggiator::GenerateNoteList()
  {
    arpList.clear();
    if (config -> type == ARP_ByOrder)
    {
      std::multimap<uint32_t, uint8_t> tempList; // time, noteID
      for(auto it = inputList.begin(); it != inputList.end(); it++)
        tempList.insert({it->second.time, it->first});

      for(auto it = tempList.begin(); it != tempList.end(); it++)
        arpList.push_back(it->second);
    }
    else
    {
    for (auto it = inputList.begin(); it != inputList.end(); it++)
      arpList.push_back(it->first);
    }

    size_t size = arpList.size();
    for(uint8_t octave = 1; octave < config->octaveRange; octave++) // OctaveExpansion
    {
      for(auto it = arpList.begin(); it != arpList.begin() + size; it++)
      {
        uint8_t note = *it + octave * 12;
        if (note <= *(arpList.end() - 1))
          continue;
        if (note > 127)
          return;
        arpList.push_back(note);
      }
    }
    
  }

  void Arpeggiator::NoteArrange() 
  {
    std::vector<uint8_t> tempArrange;
    uint8_t tempNote = 0;
    arpArrange.clear();
    arpArrange.reserve(arpList.size());
    switch (config->type)
    {
      case ARP_Up: 
      case ARP_Down: 
        for(auto it = arpList.begin(); it != arpList.end(); it++)
          arpArrange.push_back(*it);
        break;
      case ARP_Converge: 
      case ARP_Diverge: 
        tempArrange.reserve(arpList.size());
        for(auto it = arpList.begin(); it != arpList.end(); it++)
          tempArrange.push_back(*it);
        while(tempArrange.size() > 0)
        {
          arpArrange.push_back(tempArrange.front());
          tempArrange.erase(tempArrange.begin());
          if(tempArrange.size() > 0)
          {
            arpArrange.push_back(tempArrange.back());
            tempArrange.pop_back();
          }
        }
        break;
      case ARP_PinkUp: 
      case ARP_PinkDown:
        if(inputList.size() < 2)
        {
          arpArrange.push_back(*arpList.begin());
          break;
        }
        tempNote = *arpList.rbegin();
        for(auto it = arpList.begin(); it != arpList.end() - 1; it++)
        {
          arpArrange.push_back(tempNote);
          arpArrange.push_back(*it);
        }
        break;
      case ARP_ThumbUp: 
      case ARP_ThumbDown:
        if(inputList.size() < 2)
        {
          arpArrange.push_back(*arpList.begin());
          break;
        }
        tempNote = *arpList.begin();
        for(auto it = arpList.begin() + 1; it != arpList.end(); it++)
        {
          arpArrange.push_back(tempNote);
          arpArrange.push_back(*it);
        }
        break;
      case ARP_Random:
      case ARP_RandomOther:
      case ARP_RandomOnce:
        random_shuffle(arpList.begin(), arpList.end(), MatrixOS::SYS::RandSeed()); 
        for(auto it = arpList.begin(); it != arpList.end(); it++)
          arpArrange.push_back(*it);
        break;
      case ARP_ByOrder:
        for(auto it = arpList.begin(); it != arpList.end(); it++)
          arpArrange.push_back(*it);
        break;
    }
  }

  void Arpeggiator::Reverse()
  {
    switch(config->type)
    {
      case ARP_Down:
      case ARP_Diverge:
        std::reverse(arpArrange.begin(), arpArrange.end());
        break;
      case ARP_PinkDown:
      case ARP_ThumbDown:
        std::reverse(arpArrange.begin(), arpArrange.end());
        arpArrange.insert(arpArrange.begin(), *arpArrange.rbegin());
        arpArrange.pop_back();
        break;
      default:
        break;
    }
  }
  
  void Arpeggiator::forBackward()
  {
    if(arpArrange.size() <= (config->repeatEnds ? 1 : 2 ) || config->type == ARP_Random || config->type == ARP_RandomOther) return;

    auto it = config->repeatEnds ? arpArrange.rbegin() : arpArrange.rbegin() + 1;
    auto end = config->repeatEnds ? arpArrange.rend() : arpArrange.rend() - 1;
    for (; it != end; it++)
      arpArrange.push_back(*it);
  }
  
};