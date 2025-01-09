#include "Sequencer.h"

namespace MatrixOS::MidiCenter
{
  std::map<uint16_t, uint32_t> CNTR_Seq; // midiID, offTime
  std::vector<std::pair<SEQ_Pos, SEQ_Step*>> CNTR_SeqEditStep;

  void Sequencer::Scan()
  {
    if (!transportState.play) 
    {
      if(!end) End();
      return;
    }

    clipNum = seqData->EditingClip(channel);
    quarterTickPerStep = speedToQuarterTick[seqData->Clip(channel, clipNum)->speed];
    stepInterval = quarterInterval * quarterTickPerStep;
    scale = channelConfig->padType[channel] == DRUM_PAD ? 0x0FFF : notePadConfig[channelConfig->activePadConfig[channel][channelConfig->padType[channel]]].scale;

    end = false;
    if (!firstStepBuff) FirstStepBuff();
    Trigger();
    
    if((quarterTick % quarterTickPerStep == (quarterTickPerStep / 2)) && !noteBuff)
    {
      // if(channel == 0) MLOGD("Sequencer", "tickCount: %d, PlayHead: %d", tickCount, playHead);
      MoveHead(buffHead);
      GetNoteQueue();
      noteBuff = true;
    }
    else if((quarterTick % quarterTickPerStep != (quarterTickPerStep / 2)) && noteBuff)
      noteBuff = false;
    
    if(quarterTick % quarterTickPerStep == 4 && playHead != buffHead) playHead = buffHead; 
  }

  void Sequencer::Record(uint8_t channel, uint8_t byte1, uint8_t byte2)
  {
    if (channel != MatrixOS::UserVar::global_channel)
    {
        if(!recNotes.empty()) SetRecNoteGate();
        return;
    } 

    SEQ_Clip* clip = seqData->Clip(this->channel, clipNum);
    if (clip->HasLoop()) {
        // 确保录音在 loop 范围内
        uint16_t loopStart = clip->loopStart * STEP_MAX;
        uint16_t loopEnd = (clip->loopEnd + 1) * STEP_MAX - 1;
        
        if (playHead > loopEnd || playHead < loopStart) {
            // 如果录音位置在 loop 范围外，不进行录音
            return;
        }
    }

    if(byte2 > 0)
    {
        SEQ_Step* recStep = seqData->Step(SEQ_Pos(channel, clipNum, buffHead / STEP_MAX, buffHead % STEP_MAX), true);
        uint32_t now = MatrixOS::SYS::Millis();
        int8_t offset = ((now - stepTime) * 120) / stepInterval;
        offset = std::clamp(offset, int8_t(-60), int8_t(60));
        recStep->AddNote(SEQ_Note(byte1, byte2, 0, offset));
        recNotes.emplace(byte1, std::make_pair(stepTime, recStep));
        recording = true;
        return;
    }
    else
        SetRecNoteGate(byte1);
  }

  bool Sequencer::AutoGrouth() { return transportState.record && transportState.autoGrouth && recording; }

  void Sequencer::End()
  {
    std::queue<std::pair<uint32_t, SEQ_Note>>().swap(notesQueue);
    SetRecNoteGate(); 
    cycleStep.clear();
    
    // 根据 loop 状态设置初始播放位置
    SEQ_Clip* clip = seqData->Clip(channel, clipNum);
    if (clip->HasLoop()) {
        playHead = clip->loopStart * STEP_MAX;
        buffHead = playHead;
    } else {
        playHead = 0;
        buffHead = 0;
    }
    
    firstStepBuff = false;
    recording = false;
    end = true;
  }

  void Sequencer::MoveHead(int16_t& head)
  {
    head++;
    SEQ_Clip* clip = seqData->Clip(channel, clipNum);

    if (clip->HasLoop()) {
        // 在 loop 范围内循环
        uint16_t loopStart = clip->loopStart * STEP_MAX;
        uint16_t loopEnd = (clip->loopEnd + 1) * STEP_MAX - 1;
        
        // 如果超出 loop 范围，回到 loop 起始位置
        if (head > loopEnd) {
            head = loopStart;
        }
        // 如果在 loop 范围外，移动到 loop 起始位置
        else if (head < loopStart) {
            head = loopStart;
        }
        
        // 处理每个 bar 内的 step 限制
        if (head % STEP_MAX >= clip->barStepMax) {
            head = (head / STEP_MAX + 1) * STEP_MAX;
            if (head > loopEnd) {
                head = loopStart;
            }
        }
    } else {
        // 原有的非 loop 逻辑
        uint8_t stepMax = clip->barStepMax + (clip->barMax - 1) * STEP_MAX;
        if(head >= stepMax && AutoGrouth())
        {
            clip->barMax = std::min(int(clip->barMax + 1), BAR_MAX);
            stepMax = clip->barStepMax + (clip->barMax - 1) * STEP_MAX;
        }
        if(head % STEP_MAX >= clip->barStepMax)
            head = head + STEP_MAX - (head % STEP_MAX);
        if(head >= stepMax)
            head = head % STEP_MAX;
    }
    stepTime = stepInterval / 2 + MatrixOS::SYS::Millis();
  }

  void Sequencer::GetNoteQueue(bool firstStep)
  {
    SEQ_Clip* clip = seqData->Clip(channel, clipNum);
    SEQ_Step* step = seqData->Step(SEQ_Pos(channel, clipNum, buffHead / STEP_MAX, buffHead % STEP_MAX));
    if(!step || step->Empty()) return;
    if(!Chance(step) || !Cycle(step)) return;

    int8_t  octaveShift = 0,  pitchShift  = 0;
    uint8_t retrig      = 1,  retrigDecay = 0;  
    uint8_t flamVel     = 0,  flamTime    = 0;
    PitchShift(step, &octaveShift, &pitchShift);
    Retrig(step, &retrig, &retrigDecay);
    Flam(step, &flamTime, &flamVel);

    vector<const SEQ_Note*> notes = step->GetNotes();
    uint32_t now = MatrixOS::SYS::Millis();
    for(auto note : notes)
    {
      if(!step->FindStepComp(COMP_CHANCE) && !Chance(note)) continue;
      if(!step->FindStepComp(COMP_CYCLE)  && !Cycle(note) ) continue;

      SEQ_Note tempNote = *note;
      uint32_t time = now + quarterInterval * (firstStep ? 0 : quarterTickPerStep / 2 + 4);
      int32_t offset = (stepInterval * tempNote.offset * (100 - clip->quantize)) / 12000;
      PitchShift(&tempNote, octaveShift, pitchShift);
      if(!step->FindStepComp(COMP_RETRIG)) Retrig(note, &retrig, &retrigDecay);
      if(!step->FindStepComp(COMP_FLAM)) Flam(note, &flamTime, &flamVel);

      if(flamTime > 0)
      {
        SEQ_Note flamNote = tempNote;
        flamNote.velocity = flamVelocity[flamVel];
        notesQueue.push({time + offset - flamTime, flamNote});
      }

      for(uint8_t i = 0; i < retrig; i++)
      {
        if(i > 0) tempNote.velocity = tempNote.velocity * retrigDecayRatio[retrigDecay];
        notesQueue.push({time + offset + i * stepInterval / retrig, tempNote});
      }
    }
  }

  void Sequencer::FirstStepBuff()
  {
    std::queue<std::pair<uint32_t, SEQ_Note>>().swap(notesQueue);
    GetNoteQueue(true);
    firstStepBuff = true;
    stepTime = MatrixOS::SYS::Millis();
    
    // 确保初始播放位置在 loop 范围内
    SEQ_Clip* clip = seqData->Clip(channel, clipNum);
    if (clip->HasLoop()) {
        if (playHead < clip->loopStart * STEP_MAX || 
            playHead > (clip->loopEnd + 1) * STEP_MAX - 1) {
            playHead = clip->loopStart * STEP_MAX;
            buffHead = playHead;
        }
    }
  }

  void Sequencer::Trigger()
  {
    if(notesQueue.empty()) return;

    uint32_t now = MatrixOS::SYS::Millis();
    SEQ_Clip* clip = seqData->Clip(channel, clipNum);
    int gateTail = stepInterval * clip->tair / 100;

    while(!notesQueue.empty() && notesQueue.front().first <= now)
    {
      auto it = notesQueue.front();
      int tail = gateTail;
      int offset = (stepInterval * it.second.offset * (100 - clip->quantize)) / 12000;
      if(it.second.tair <= 100)
        tail = (gateTail * clip->quantize + (stepInterval * it.second.tair * (100 - clip->quantize) / 100 )) / 100;
      
      int gate = int(it.second.gate * stepInterval) + tail - offset;
      if (it.second.offset > 0 && it.second.tair > 0 && it.second.gate > 0) gate = gate - stepInterval * clip->quantize / 100; // if offset and tair both positive, gate will be shorter 1 step
      gate = std::max(gate, 5);
      uint32_t offTime = now + gate;
      uint8_t note = it.second.note;
      uint8_t velocity = it.second.velocity;
      uint16_t midiID = MidiID(SEND_NOTE, channel, note);
      auto it2 = CNTR_Seq.find(midiID);
      if (it2 != CNTR_Seq.end())
        MidiRouter(seqInOut[channel], SEND_NOTE, channel, note, 0);
      MidiRouter(seqInOut[channel], SEND_NOTE, channel, note, velocity);
      CNTR_Seq.insert_or_assign(midiID, offTime);
      notesQueue.pop();
      // MLOGD("Sequencer", "Channel: %d, Note: %d, Velocity: %d, Offset: %d, Gate: %d, Tail: %d, Step: %d  OffTime: %d", channel, note, velocity, offset, gate, tail, buffHead, offTime);
    }
  }

  void Sequencer::SetRecNoteGate(uint8_t note)
    {
      while(note <= 127 ? recNotes.find(note) != recNotes.end() : !recNotes.empty())
      {
        auto it = note <= 127 ? recNotes.find(note) : recNotes.begin();
        SEQ_Step* step = it->second.second;
        if(step && step->FindNote(it->first))
        {
          int64_t now = MatrixOS::SYS::Millis();
          double gateRatio = (now - it->second.first) / stepInterval;
          uint8_t gate = std::min(int(gateRatio), STEP_MAX * BAR_MAX - 1);
          double tairRatio = gateRatio < 0 ? gateRatio : gateRatio - int(gateRatio);
          int8_t tair  = std::clamp(int(tairRatio * 100), -50, 100);

          step->SetGate(it->first, gate, tair);
          // MLOGD("Seq Record", "Channel: %d, Note: %d, Gate: %d, Tair: %d, Time: %d", channel, it->first, gate, tair, now);
        }
        recNotes.erase(it);
      }
    }

  bool Sequencer::Chance(const SEQ_Step* step)
  {
    if(step && step->FindStepComp(COMP_CHANCE))
      return esp_random() % 101 < step->noteTemplate.chance;
    return true;
  }

  bool Sequencer::Chance(const SEQ_Note* note)
  {
    if(note && note->FindComp(COMP_CHANCE))
      return sys_random() % 101 < note->chance;
    return true;
  }

  bool Sequencer::Cycle(const SEQ_Step* step)
  {
    const SEQ_Note* note = &step->noteTemplate;
    if(step && step->FindStepComp(COMP_CYCLE))
    {
      if (cycleStep.find(note) != cycleStep.end())
      {
        cycleStep[note]++;
        if (cycleStep[note] >= step->noteTemplate.cycleLength) cycleStep[note] = 0;
      }
      else
        cycleStep[note] = 0;
      return (step->noteTemplate.cycleStep >> cycleStep[note]) & 1;
    }
    return true;
  }

  bool Sequencer::Cycle(const SEQ_Note* note)
  {
    if(note && note->FindComp(COMP_CYCLE))
    {
      if (cycleStep.find(note) != cycleStep.end())
      {
        cycleStep[note]++;
        if (cycleStep[note] >= note->cycleLength) cycleStep[note] = 0;
      }
      else
        cycleStep[note] = 0;
      return (note->cycleStep >> cycleStep[note]) & 1;
    }
    return true;
  }

  void Sequencer::Retrig(const SEQ_Step* step, uint8_t* retrig, uint8_t* retrigDecay)
  {
    if(step && step->FindStepComp(COMP_RETRIG))
    {
      *retrig = step->noteTemplate.retrig;
      *retrigDecay = step->noteTemplate.retrigDecay;
    }
  }

  void Sequencer::Retrig(const SEQ_Note* note, uint8_t* retrig, uint8_t* retrigDecay)
  {
    if(note && note->FindComp(COMP_RETRIG))
    {
      *retrig = note->retrig;
      *retrigDecay = note->retrigDecay;
      return;
    }
    *retrig = 1;
    *retrigDecay = 0;
  }

  void Sequencer::Flam(const SEQ_Step* step, uint8_t* flamTime, uint8_t* flamVel)
  {
    if(step && step->FindStepComp(COMP_FLAM))
    {
      *flamTime = step->noteTemplate.flamTime;
      *flamVel = step->noteTemplate.flamVel;
    }
  }

  void Sequencer::Flam(const SEQ_Note* note, uint8_t* flamTime, uint8_t* flamVel)
  {
    if(note && note->FindComp(COMP_FLAM))
    {
      *flamTime = note->flamTime;
      *flamVel = note->flamVel;
    }
  }

  void Sequencer::PitchShift(const SEQ_Step* step, int8_t* octaveShift, int8_t* pitchShift)
  {
    if(step && step->FindStepComp(COMP_PITCH))
    {
      *octaveShift = step->noteTemplate.octaveShift;
      *pitchShift = step->noteTemplate.pitchShift;
      if (step->noteTemplate.randomPitch) 
      {
        uint8_t octaveRand = sys_random() % (std::abs(*octaveShift) + 1);
        *octaveShift = *octaveShift < 0 ? -octaveRand : octaveRand;
        uint8_t pitchRand = sys_random() % (std::abs(*pitchShift) + 1);
        *pitchShift = *pitchShift < 0 ? -pitchRand : pitchRand;
      }
    }
  }

  void Sequencer::PitchShift(SEQ_Note* note, int8_t octaveShift, int8_t pitchShift)
  {
    if(note && octaveShift == 0 && pitchShift == 0 && note->FindComp(COMP_PITCH))
    {
      octaveShift = note->octaveShift;
      pitchShift = note->pitchShift;
      if (note->randomPitch) 
      {
        uint8_t octaveRand = sys_random() % (std::abs(octaveShift) + 1);
        octaveShift = octaveShift < 0 ? -octaveRand : octaveRand;
        uint8_t pitchRand = sys_random() % (std::abs(pitchShift) + 1);
        pitchShift = pitchShift < 0 ? -pitchRand : pitchRand;
      }
    }

    int16_t pitch = note->note;

    if (pitchShift > 0)
    {
      std::bitset<12> tempScale = (scale >> pitch % 12 | scale << (12 - pitch % 12)) & 0x0FFF;
      for (uint8_t i = 0; i < 12 && pitchShift > 0; i++)
      {
        if (tempScale[i] & 1) pitchShift--;
        pitch++;
      }
    }
    else if (pitchShift < 0)
    {
      std::bitset<12> tempScale = (scale >> pitch % 12 | scale << (12 - pitch % 12)) & 0x0FFF;
      for (uint8_t i = 0; i < 12 && pitchShift < 0; i++)
      {
        if (tempScale[11 - i] & 1) pitchShift++;
        pitch--;
      }
    }

    note->note = std::clamp(pitch + octaveShift * 12, pitch % 12, 120 + pitch % 12 > 127 ? 108 + pitch % 12 : 120 + pitch % 12);
  }

  void Sequencer::Update()
  {
    // 更新播放头位置
    playHead++;
    
    SEQ_Clip* clip = seqData->Clip(channel, clipNum);
    if (clip->HasLoop()) {
        // 在 loop 范围内循环
        uint16_t loopStart = clip->loopStart * STEP_MAX;
        uint16_t loopEnd = (clip->loopEnd + 1) * STEP_MAX - 1;
        
        if (playHead > loopEnd) {
            playHead = loopStart;
        }
        // 如果播放头在 loop 范围外，移动到 loop 起始位置
        else if (playHead < loopStart) {
            playHead = loopStart;
        }
    } else {
        // 在整个 clip 范围内循环
        if (playHead >= clip->barMax * STEP_MAX) {
            playHead = 0;
        }
    }
  }
}