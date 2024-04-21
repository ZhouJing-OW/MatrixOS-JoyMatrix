#include "Sequencer.h"

namespace MatrixOS::MidiCenter
{
  std::map<uint16_t, uint32_t> CNTR_Seq; // midiID, length
  std::map<uint16_t, NoteInfo> CNTR_SeqEdit; // midiID

  void Sequencer::Scan()
  {

  }

  void Sequencer::GetBuffer(uint8_t index)
  {
    // Buffer_Notes.clear();
    // Buffer_Params.clear();
    // int16_t until = halfTick_last + halfTick_wait;
    // int16_t stepID = dataStore->Clip(channel, clipNum[index])->StepID(playHead[index]);
    // if (stepID < 0) return;
    // SEQ_Step* step = dataStore->Step(stepID);
    // if (!step) return;

    // std::vector<SEQ_Note> notes = step->GetNotes();
    // std::vector<SEQ_Param> params = step->GetParams();
    // for(auto note : notes)
    //   Buffer_Notes.emplace(until + note.shift, note);
    // for(auto param : params)
    //   Buffer_Params.emplace(until , param);
  }

  void Sequencer::GetAutom()
  {

  }

  void Sequencer::Trigger()
  {

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