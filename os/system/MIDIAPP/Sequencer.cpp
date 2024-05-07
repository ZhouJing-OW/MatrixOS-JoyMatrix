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
    speedTick = speedToHalfTick[seqData->Clip(channel, clipNum)->speed];

    end = false;
    if (!firstStepBuff) FirstStepBuff();
    Trigger();
    
    if((halfTick % speedTick == (speedTick / 2)) && !noteBuff)
    {
      // if(channel == 0) MLOGD("Sequencer", "tickCount: %d, PlayHead: %d", tickCount, playHead);
      MoveHead(buffHead);
      GetNoteQueue();
      noteBuff = true;
    }
    else if((halfTick % speedTick != (speedTick / 2)) && noteBuff)
      noteBuff = false;
    
    if(halfTick % speedTick == 2 && playHead != buffHead) playHead = buffHead; 
  }

  void Sequencer::GetNoteQueue(bool firstStep)
  {
    SEQ_Step* step = seqData->Step(SEQ_Pos(channel, clipNum, buffHead / STEP_MAX, buffHead % STEP_MAX));
    if(!step || step->Empty()) return;
    
    vector<SEQ_Note> notes = step->GetNotes();
    uint32_t now = MatrixOS::SYS::Millis();
    for(auto note : notes)
    {
      uint32_t time = now + (tickInterval / 2) * (firstStep ? 0 : (speedTick / 2 - speedTick % 2 * 2 + 2));
      notesQueue.push({time, note});
    }
  }

  void Sequencer::FirstStepBuff()
  {
    std::queue<std::pair<uint32_t, SEQ_Note>>().swap(notesQueue);
    GetNoteQueue(true);
    firstStepBuff = true;
  }

  void Sequencer::End()
  {
    std::queue<std::pair<uint32_t, SEQ_Note>>().swap(notesQueue);
    playHead = 0;
    buffHead = 0;
    firstStepBuff = false;
    end = true;
  }

  void Sequencer::MoveHead(int16_t& head)
  {
    head++;
    SEQ_Clip* clip = seqData->Clip(channel, clipNum);
    if(head % STEP_MAX >= clip->barStepMax)
      head = head + STEP_MAX - (head % STEP_MAX);
    if(head >= clip->barStepMax + (clip->barMax - 1) * STEP_MAX)
      head = head % STEP_MAX;
  }

  void Sequencer::Trigger()
  {
    if(notesQueue.empty()) return;

    uint32_t now = MatrixOS::SYS::Millis();
    uint16_t  stepInterval = tickInterval / 2 * speedTick;
    uint16_t gateTail = stepInterval * seqData->Clip(channel, clipNum)->gate / 100;
    if (gateTail == stepInterval) gateTail = stepInterval - 1;
    if (gateTail < 5) gateTail = 5;

    while(!notesQueue.empty() && notesQueue.front().first <= now)
    {
      uint16_t gate = notesQueue.front().second.gate * stepInterval + gateTail;
      uint32_t offTime = now + gate;
      uint8_t note = notesQueue.front().second.number;
      uint8_t velocity = notesQueue.front().second.velocity;
      uint16_t midiID = MidiID(SEND_NOTE, channel, note);
      MidiRouter(seqInOut[channel], SEND_NOTE, channel, note, velocity);
      CNTR_Seq.insert({midiID, offTime});
      notesQueue.pop();
      // MLOGD("Sequencer", "Channel: %d, Note: %d, Velocity: %d, Gate: %d, OffTime: %d", channel, note, velocity, gate, offTime);
    }
  }

  void Sequencer::Jump()
  {

  }

  void Sequencer::Chance()
  {

  }

  void Sequencer::Random()
  {

  }

  void Sequencer::Retrig()
  {

  }
}