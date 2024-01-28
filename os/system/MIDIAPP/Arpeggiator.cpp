#include "MidiCenter.h"

namespace MatrixOS::MidiCenter
{
  std::map<uint8_t, Arpeggiator*> arpeggiators; // channel, arpConfig
  std::map<uint16_t, uint32_t> arp; // midiID, length

  void Arpeggiator::Scan()
  {
    arpInterval = rateToRatio[config->rate] * tickInterval * 24;
    if(!SyncBeat()) return;
    CheckVarChange();
    gateLength = arpInterval * gateToRatio[config->gate];
    gateLength = gateLength < 10 ? 10 : gateLength;
    uint32_t currentTime = MatrixOS::SYS::Millis();
    if(currentTime >= arpTimer + arpInterval - intervalDelta)
    {
      if(currentArpNote >= arpArrange.size())
      currentArpNote = 0;
      intervalDelta =  currentTime - (arpTimer + arpInterval - intervalDelta);
      // MLOGD("Arpeggiator", "Time Interval : %f, Interval Delta: %f", arpInterval, intervalDelta);
      arpTimer = currentTime;
      Trigger();
    }
  }

  void Arpeggiator::CheckVarChange()
  {
    // monitor knob change. if changed, restart arp, do not restart step
    bool needChange = false; 
    bool needReset = false;
    if (syncBeatPrev != config->syncBeat) { syncBeatPrev = config->syncBeat; needChange = true;}
    if (skipPrev != config->skip) { skipPrev = config->skip; needChange = true;}
    if (forBackwardPrev != config->forBackward) { forBackwardPrev = config->forBackward; needChange = true; needReset = true;}
    if (repeatEndsPrev != config->repeatEnds) { repeatEndsPrev = config->repeatEnds; needChange = true; needReset = true;}
    for (uint8_t i = 0; i < 8; i++) 
    { 
      if(configPrv[i] != *configPtr[i])
      {
        configPrv[i] = *configPtr[i];
        activeLabel = i;
        needChange = true;
        if(i == 4 || i == 5 || i == 6) // type, repeat, octaveRange
          needReset = true;
      }
    }
    if (needChange) 
    {
      ArpStart(needReset);
      // MLOGD("Arpeggiator", "Arp Config %d Changed.", activeLabel);
    }

    // monitor inputList change. if changed, restart arp, do not restart step
    if (inputList != inputListPrv) 
    { 
      if(inputList.size() > inputListPrv.size())
        decayNow = 0;
      inputListPrv = inputList;
      ArpStart(true);
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
        arp.insert({noteID, MatrixOS::SYS::Millis() + gateLength});
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

    if (currentArpNote < config->octaveRange * inputList.size())
      currentOctave = currentArpNote / inputList.size();
    else
      currentOctave = config->octaveRange - 1 - (currentArpNote + config->forBackward * !config->repeatEnds - config->octaveRange * inputList.size()) / inputList.size();
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
  }

  void Arpeggiator::ArpStart(bool reset)
  {
    ArpEnd(reset);
    GenerateNoteList();
    NoteArrange();
    OctaveExpansion();
    if (config->forBackward)
      forBackward();
  }

  bool Arpeggiator::SyncBeat()
  {
    if(inputList.empty())
    {
      currentArpNote = 0;
      decayNow = 0;
      synced = false;
      ArpEnd(true);
      return false;
    }

    if (synced) return true;

    if(!config->syncBeat || !transportState.play)
    {
      arpTimer = MatrixOS::SYS::Millis() - arpInterval;
      synced = true;
      ArpStart(true);
      return true;
    }
    else if(!beatTimer.IsLonger(20))
    {
      arpTimer = MatrixOS::SYS::Millis() - beatTimer.SinceLastTick() - arpInterval;
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
    for (auto it = inputList.begin(); it != inputList.end(); it++)
      arpList.push_back(it->first);
  } 

  void Arpeggiator::NoteArrange() 
  {
    std::vector<uint8_t> tempArrange;
    uint8_t tempHigh = 0;
    uint8_t tempLow = 0;
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
        tempHigh = *arpList.rbegin() + (config->octaveRange - 1)  * 12;
        for(auto it = arpList.begin(); it != arpList.end() - 1; it++)
        {
          arpArrange.push_back(tempHigh);
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
        tempLow = *arpList.begin();
        for(auto it = arpList.begin() + 1; it != arpList.end(); it++)
        {
          arpArrange.push_back(tempLow);
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
      {
        std::map<uint8_t, uint8_t> tempList; // noteCount, noteID
        for(auto it = inputList.begin(); it != inputList.end(); it++)
        {
          tempList.insert({it->second, it->first});
        }
        for(auto it = tempList.begin(); it != tempList.end(); it++)
        {
          arpArrange.push_back(it->second);
        }
        break;
      }
    }
    if(config->type == ARP_Down || config->type == ARP_Diverge)
      std::reverse(arpArrange.begin(), arpArrange.end());
  }

  void Arpeggiator::OctaveExpansion()
  {
    size_t size = arpArrange.size();
    for(uint8_t octave = 1; octave < config->octaveRange; octave++)
    {
      for(auto it = arpArrange.begin(); it != arpArrange.begin() + size; it++)
      {
        if((config->type == ARP_PinkUp || config->type == ARP_ThumbUp || config->type == ARP_PinkDown || config->type == ARP_ThumbDown) && *it == *arpArrange.begin())
          arpArrange.push_back(*arpArrange.begin());
        else if (*it + octave * 12 <= 127)
          arpArrange.push_back(*it + octave * 12);
        else
          return;
      }
    }
    if(config->type == ARP_PinkDown || config->type == ARP_ThumbDown)
    {
      std::reverse(arpArrange.begin(), arpArrange.end());
      arpArrange.insert(arpArrange.begin(), *arpArrange.rbegin());
      arpArrange.pop_back();
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