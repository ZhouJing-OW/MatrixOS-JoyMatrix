#pragma once
#include "MatrixOS.h"
#include "MidiCenter.h"
#include <bitset>
#include <vector>
#include <map>
#include <set>

namespace MatrixOS::MidiCenter
{
  #define CLIP_MAX      8
  #define BAR_MAX       4
  #define STEP_MAX      16
  #define STEP_DIVISION 12
  #define NOTE_MAX      8
  #define PARAM_MAX     8
  #define AUTOM_MAX     STEP_MAX  // < 16
  #define STEP_ALL      16 * CLIP_MAX * BAR_MAX * STEP_MAX
  #define AUTOM_ALL     16 * CLIP_MAX * AUTOM_MAX

  class   SEQ_DataStore;

  class   SEQ_Clip
  {
  public:
    uint8_t quantize        = 0;
    uint8_t barMax          = 1;
    uint8_t barStepMax      = STEP_MAX;
    uint8_t speed           = 3;      // 4x, 3x, 2x, 1x, 1/2, 1/3, 1/4, 1/8
    uint8_t noteLength      = 50;     // 5% - 100%
  private:
    std::bitset<BAR_MAX * STEP_MAX> stepMark;
    std::bitset<BAR_MAX * AUTOM_MAX> automMark;
    int16_t stepID[BAR_MAX * STEP_MAX];
    int16_t automID[BAR_MAX * AUTOM_MAX];

  public:
    SEQ_Clip() { 
      memset(stepID,  -1, sizeof(stepID));
      memset(automID, -1, sizeof(automID));
      stepMark.reset(); automMark.reset();
    }

    int16_t StepID (uint8_t BarNum, uint8_t stepNum) const 
    { 
      if(BarNum >= BAR_MAX || stepNum >= STEP_MAX) return -1;
      return stepID[BarNum * STEP_MAX + stepNum]; 
    }
    int16_t AutomID(uint8_t BarNum, uint8_t automNum) const 
    { 
      if(BarNum >= BAR_MAX || automNum >= AUTOM_MAX) return -1;
      return automID[BarNum * AUTOM_MAX + automNum]; 
    }
    bool Empty() const { return stepMark.none() && automMark.none(); }

  private:
    void SetStepID(uint8_t BarNum, uint8_t stepNum, int16_t id) 
    { 
      stepID[BarNum * STEP_MAX + stepNum] = id;
      stepMark[BarNum * STEP_MAX + stepNum] = id == -1 ? false : true;
    }
    void SetAutomID(uint8_t BarNum, uint8_t automNum, int16_t id) 
    { 
      automID[BarNum * AUTOM_MAX + automNum] = id;
      automMark[BarNum * AUTOM_MAX + automNum] = id == -1 ? false : true;
    }

    friend class SEQ_DataStore;
  };

  class   SEQ_Pos
  {
    int16_t m_pos; // in the song data position
    uint8_t m_channel;
    uint8_t m_clip;
    uint8_t m_bar;
    uint8_t m_number; // step : STEP_MAX or autom:  BAR_MAX * PARAM_MAX

  public:

    SEQ_Pos(uint8_t channel, uint8_t clip, uint8_t bar, uint8_t number)
    { SetPos(channel, clip, bar, number); }

    SEQ_Pos(int16_t pos)
    { SetPos(pos); }

    void SetPos(uint8_t channel, uint8_t clip, uint8_t bar, uint8_t number)
    {
      if(channel >= 16 || clip >= CLIP_MAX || bar >= BAR_MAX || number >= STEP_MAX) return;
      m_channel = channel;
      m_clip = clip;
      m_bar = bar;
      m_number = number;
      m_pos = ((((channel * CLIP_MAX) + clip) * BAR_MAX) + bar) * STEP_MAX + number;
    }

    void SetPos(int16_t pos)
    {
      if(pos < 0 || pos >= 16 * CLIP_MAX * STEP_MAX) return;
      m_pos = pos;
      m_channel = pos / (CLIP_MAX * BAR_MAX * STEP_MAX);
      m_clip = (pos / (BAR_MAX * STEP_MAX)) % CLIP_MAX;
      m_bar = (pos / STEP_MAX) % BAR_MAX;
      m_number = pos % STEP_MAX;
    }

    int16_t Pos()         { return m_pos; }
    uint8_t ChannelNum()  { return m_channel; }
    uint8_t ClipNum()     { return m_clip; }
    uint8_t BarNum()      { return m_bar;}
    uint8_t Number()      { return m_number; }
  };
 
  struct  SEQ_Note                     
  {
    uint8_t number;  // note
    uint8_t velocity;
    uint8_t gate;    // half tick
    int8_t  shift;    // half tick -5 to +5

    SEQ_Note(uint8_t note = 255, uint8_t velocity = MatrixOS::UserVar::defaultVelocity, uint8_t gate = 0, uint8_t shift = 0)
    {
      this->number = note;
      this->velocity = velocity;
      this->gate = gate;
      this->shift = shift;
    }
  };

  enum    PARAM_TYPE : uint8_t
  {
    PARAM_K01,      PARAM_K02,      PARAM_K03,      PARAM_K04,      PARAM_K05,      PARAM_K06,      PARAM_K07,      PARAM_K08,
    PARAM_K09,      PARAM_K10,      PARAM_K11,      PARAM_K12,      PARAM_K13,      PARAM_K14,      PARAM_K15,      PARAM_K16,
    PARAM_K17,      PARAM_K18,      PARAM_K19,      PARAM_K20,      PARAM_K21,      PARAM_K22,      PARAM_K23,      PARAM_K24,
    PARAM_K25,      PARAM_K26,      PARAM_K27,      PARAM_K28,      PARAM_K29,      PARAM_K30,      PARAM_K31,      PARAM_K32,
    PARAM_MOD,      PARAM_PITCH,    COMP_RETRIG,    COMP_HOLD,      COMP_STAY,      COMP_CYCLE,     COMP_CHANCE,    COMP_RANDOM,  
  };

  struct  SEQ_Param
  {
    uint8_t number   = 255;  // param type
    int8_t byte1     = -1;   // -1 : The component is applied to every note in the step.
    int8_t byte2     = 0;
  };

  class   SEQ_ClipItem
  {
    SEQ_Pos position = SEQ_Pos(0);

  public:
    SEQ_Pos Position()   { return position; }
    int16_t Pos()        { return position.Pos(); }
    uint8_t ChannelNum() { return position.ChannelNum(); }
    uint8_t ClipNum()    { return position.ClipNum(); }
    uint8_t BarNum()     { return position.BarNum(); }
    uint8_t Number()     { return position.Number(); }
    void    SetPos(uint8_t channel, uint8_t clip, uint8_t bar, uint8_t step) { position.SetPos(channel, clip, bar, step); }
    void    SetPos(int16_t pos) { position.SetPos(pos); }
    void    SetPos(SEQ_Pos pos) { position = pos; }
  };

  class   SEQ_Autom : public SEQ_ClipItem
  {
    uint8_t number = 255;
    uint8_t value[STEP_MAX * STEP_DIVISION / BAR_MAX];

  public:
    SEQ_Autom() { memset(value, -1, sizeof(value)); }
  };

  class   SEQ_Step  : public SEQ_ClipItem
  {
    SEQ_Note noteTemplate;
    SEQ_Note notes[NOTE_MAX];
    SEQ_Param params[PARAM_MAX];
    std::bitset<128> noteMark;
    std::bitset<64> paramMark;
    

   public:
    std::vector<SEQ_Note> GetNotes() const { return Get(notes); }
    bool FindNote(uint8_t note) const { return Find(noteMark, notes, note); }
    bool AddNote(SEQ_Note note) { return Add(noteMark, notes, note, 127); }
    bool deleteNote(uint8_t note) { return Delete(noteMark, notes, note, 127); }
    void ClearNote()
    {
      for(uint8_t i = 0; i < NOTE_MAX ; i++ )
        notes[i] = SEQ_Note();
      noteMark.reset();
    }

    std::vector<SEQ_Param> GetParams() const { return Get(params); }
    bool FindParam(uint8_t number) const { return Find(paramMark, params, number); }
    bool AddParam(SEQ_Param param) { return Add(paramMark, params, param, 63); }
    bool deleteParam(uint8_t number) { return Delete(paramMark, params, number, 63); }
    void ClearParam()
    {
      for(uint8_t i = 0; i < PARAM_MAX; i++)
        params[i] = SEQ_Param();
      paramMark.reset();
    }

    uint8_t GetVelocity(uint8_t note) const 
    {
      if (NoteEmpty()) return noteTemplate.velocity;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if ((note > 127 && notes[i].number <= 127) || (note <= 127 && notes[i].number == note))
          return notes[i].velocity;
      }
      return noteTemplate.velocity;
    }

    void SetVelocity(uint8_t note, uint8_t velocity)
    {
      noteTemplate.velocity = velocity;
      if(NoteEmpty()) return;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (notes[i].number <= 127 && (note > 127 || notes[i].number == note))
          notes[i].velocity = velocity;
      }
    }

    uint8_t GetGate(uint8_t note) const
    {
      if (NoteEmpty()) return noteTemplate.gate;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if ((note > 127 && notes[i].number <= 127) || (note <= 127 && notes[i].number == note))
          return notes[i].gate;
      }
      return noteTemplate.gate;
    }

    void SetGate(uint8_t note, uint8_t gate)
    {
      noteTemplate.gate = gate;
      if(NoteEmpty()) return;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (notes[i].number <= 127 && (note > 127 || notes[i].number == note))
          notes[i].gate = gate;
      }
    }

    uint8_t GetShift(uint8_t note) const
    {
      if (NoteEmpty()) return noteTemplate.shift;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if ((note > 127 && notes[i].number <= 127) || (note <= 127 && notes[i].number == note))
          return notes[i].shift;
      }
      return noteTemplate.shift;
    }

    void SetShift(uint8_t note, uint8_t shift)
    {
      noteTemplate.shift = shift;
      if(NoteEmpty()) return;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (notes[i].number <= 127 && (note > 127 || notes[i].number == note))
          notes[i].shift = shift;
      }
    }

    bool Empty() const { return noteMark.none() && paramMark.none(); } 
    bool NoteEmpty() const { return noteMark.none(); }
    uint8_t NoteCount() const {return noteMark.count();}
    
    bool ParamEmpty() const { return paramMark.none(); }
    bool NoteFull() const { return noteMark.all(); }
    bool ParamFull() const { return paramMark.all(); }

  private:
    template <typename T, size_t N>
    std::vector<T> Get(const T (&arr)[N]) const
    {
      std::vector<T> vec;
      for(uint8_t i = 0; i < N; i++)
        if(arr[i].number != 255) vec.push_back(arr[i]);
      return vec;
    }

    template <typename T, size_t N, size_t M> 
    bool Find(const std::bitset<M>& map, T (&arr)[N], uint8_t number) const { return map.test(number); }

    template <typename T, size_t N, size_t M>
    bool Add(std::bitset<M>& map, T (&arr)[N], T item, uint8_t max)
    {
      if(item.number > max) return false;
      bool isExist = Find(map, arr, item.number);
      for(uint8_t i = 0; i < N; i++)
      {
        if(isExist)  
        {
          if(arr[i].number == item.number)
          { arr[i] = item; return true; }
        }
        else
        {
          if(arr[i].number == 255)
          { arr[i] = item; map.set(item.number); return true; }
        }
      }
      return false;
    }

    template <typename T, size_t N, size_t M>
    bool Delete(std::bitset<M>& map, T (&arr)[N], uint8_t number, uint8_t max)
    {
      if(number > max) return false;
      for(uint8_t i = 0; i < N; i++)
      {
        if(arr[i].number == number)
        {
          arr[i].number = 255;
          map.reset(number);
          return true;
        }
      }
      return false;
    }

  };

  enum    CapState : uint8_t { CAP_NORMAL, CAP_EDIT, };

  class   SEQ_Capture 
  {
    std::map<uint16_t, NoteInfo> capture;   // midiID, NoteInfo
    std::set<uint16_t> inputList;           // midiID
    uint32_t lastReleaseTime;
    uint8_t channelPrv;
    CapState state = CAP_NORMAL;

public:
    void Capture(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      if(channel != MatrixOS::UserVar::global_channel) return;

      if(channelPrv != channel)
      {
        channelPrv = channel;
        Clear(false);
      }

      switch(state)
      {
        case CAP_NORMAL:
          NormalCapture(channel, byte1, byte2); break;
        case CAP_EDIT:
          EditCapture(channel, byte1, byte2); break;
      }
    }

    void ChangeState(CapState state)
    {
      switch(state)
      {
        case CAP_NORMAL:
          inputList.clear();
          for(auto it : CNTR_SeqEdit)
            capture.insert_or_assign(it.first, NoteInfo(-(std::abs(it.second.velocity)), lastReleaseTime));
          CNTR_SeqEdit.clear(); break;
        case CAP_EDIT: break;
      }
      this->state = state;
    }

    void Editing(uint8_t channel, const SEQ_Step* step)
    {
      bool useCap = false;

      for(auto it : capture)
      {
        if (it.second.velocity > 0)
        {
          CNTR_SeqEdit.emplace(it.first, it.second);
          useCap = true;
        }
      };

      if(useCap) return;

      if(!step->NoteEmpty()) // if step has notes, set step notes to CNTR_SeqEdit
      {
        std::vector<SEQ_Note> notes = step->GetNotes();
        
        for(auto it : notes)
        {
          uint16_t midiID = MidiID(SEND_NOTE, channel, it.number);
          CNTR_SeqEdit.emplace(midiID, NoteInfo(it.velocity, 0));
        }
      }
    }

    void EditCapture(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      uint32_t time = MatrixOS::SYS::Millis();
      uint16_t midiID = MidiID(SEND_NOTE, channel, byte1);

      if(byte2 > 0)
      {
        auto it = CNTR_SeqEdit.find(midiID);
        if(it == CNTR_SeqEdit.end())
          CNTR_SeqEdit.emplace(midiID, NoteInfo(byte2,time));
        else
          CNTR_SeqEdit.erase(midiID);
        return;
      }

      inputList.erase(midiID);
      lastReleaseTime = time;
      auto it = capture.find(midiID);
      if(it != capture.end())
        capture.insert_or_assign(midiID, NoteInfo(-std::abs(it->second.velocity), time));
    }

    void NormalCapture(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      uint32_t time = MatrixOS::SYS::Millis();
      uint16_t midiID = MidiID(SEND_NOTE, channel, byte1);

      if (byte2 > 0) 
      {
        if (inputList.empty()) capture.clear();
        inputList.emplace(midiID);
        capture.emplace(midiID, NoteInfo(byte2,time));
        return;
      }

      inputList.erase(midiID);
      lastReleaseTime = time;
      auto it = capture.find(midiID);
      if (it != capture.end())
        capture.insert_or_assign(midiID, NoteInfo(-it->second.velocity, time));
    }

    void Clear(bool resetState = true)
    {
      if(resetState) state = CAP_NORMAL;
      capture.clear();
      inputList.clear();
      CNTR_SeqEdit.clear();
      lastReleaseTime = 0;
    }

    void EndEditing()
    {
      state = CAP_NORMAL;
      CNTR_SeqEdit.clear();
    }

    std::vector<SEQ_Note> GetEdit(uint8_t channel) 
    {
      std::vector<SEQ_Note> notes;
      if(CNTR_SeqEdit.empty()) return notes;
      for(auto it : CNTR_SeqEdit)
      {
        uint8_t note = ID_Byte1(it.first);
        notes.push_back(SEQ_Note(note, it.second.velocity));
      }
      return notes;
    }

    std::vector<SEQ_Note> GetCapture(uint8_t channel) 
    {
      std::vector<SEQ_Note> notes;
      if(capture.empty()) return notes;
      for(auto it : capture)
      {
        if(it.second.velocity > 0 || (it.second.velocity <= 0 && (it.second.time + 200 >= lastReleaseTime)))
          notes.push_back(SEQ_Note(ID_Byte1(it.first), std::abs(it.second.velocity)));
      }
      return notes;
    }

  };

  class   SEQ_DataStore
  {
    std::array<SEQ_Clip, 16 * CLIP_MAX> clips;
    std::map<int16_t, SEQ_Step,  std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Step>>>  steps;  // stepID, step
    std::map<int16_t, SEQ_Autom, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Autom>>> automs; // automID, autom
    std::set<int16_t> stepChanged, automChanged; // stepID, automID
    SEQ_Capture capturer;
    bool inited = false;

   public:
    void Init()
    {
      new (&clips) std::array<SEQ_Clip, 16 * CLIP_MAX>();
      new (&steps) std::map<int16_t, SEQ_Step,  std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Step>>>();
      new (&automs) std::map<int16_t, SEQ_Autom, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Autom>>>();
      new (&stepChanged) std::set<int16_t>();
      new (&automChanged) std::set<int16_t>();
      new (&capturer) SEQ_Capture();
      inited = true;
    }

    void Destroy()
    {
      clips.~array();
      steps.~map();
      automs.~map();
      stepChanged.~set();
      automChanged.~set();
      capturer.~SEQ_Capture();
    }

    ~SEQ_DataStore() { if(inited) Destroy(); }

    void Capture(uint8_t channel, uint8_t byte1, uint8_t byte2) { capturer.Capture(channel, byte1, byte2); }

    void Capture_ChangeState(CapState state) { capturer.ChangeState(state); }

    void Capture_Editing(SEQ_Pos position) 
    { 
      SEQ_Step* step = Step(position, true);
      if(step) 
      {
        capturer.Editing(position.ChannelNum(), step);
        capturer.ChangeState(CAP_EDIT);
      }
    }

    void Capture_SaveEdited(SEQ_Pos position)
    {
      std::vector<SEQ_Note> notes = capturer.GetEdit(position.ChannelNum());
      SEQ_Step* step = Step(position);
      if(!step) return;

      if(notes.empty()) 
      {
        step->ClearNote();
        if (step->Empty()) DeleteStep(position);
        capturer.ChangeState(CAP_NORMAL);
        return; 
      }

      for (uint8_t i = 0; i < notes.size(); i++)
      {
        uint8_t note = notes[i].number;
        if(step->FindNote(note))
        {
          notes[i].velocity = step->GetVelocity(note);
          notes[i].gate     = step->GetGate(note);
          notes[i].shift    = step->GetShift(note);
        }
      }

      ClearNote(position, false);
      AddNotes(position, notes);
      capturer.ChangeState(CAP_NORMAL);
    }

    void Capture_SaveNormal(SEQ_Pos position)
    {
      SEQ_Step* step = Step(position);
      if(!step || !NoteEmpty(position))
      {
        ClearNote(position, true);
        capturer.ChangeState(CAP_NORMAL);
        return;
      }

      std::vector<SEQ_Note> notes = capturer.GetCapture(position.ChannelNum());
      if(notes.empty())
      {
        if(channelConfig != nullptr)
          AddNote(position, SEQ_Note(channelConfig->activeNote[position.ChannelNum()], MatrixOS::UserVar::defaultVelocity));
      }
      else
        AddNotes(position, notes);
      capturer.ChangeState(CAP_NORMAL);
    }

    void Capture_SaveSingle(SEQ_Pos position)
    {
      uint8_t channel = MatrixOS::UserVar::global_channel;
      uint8_t activeNote = channelConfig->activeNote[channel];
      if(!FindNote(position, activeNote))
        AddNote(position, SEQ_Note{activeNote, (uint8_t)MatrixOS::UserVar::defaultVelocity});
      else 
        DeleteNote(position, activeNote);
    }

    void Capture_Clear() { capturer.Clear(); }

    void Capture_EndEditing() { capturer.EndEditing(); }

    SEQ_Clip*   Clip(uint8_t channel, uint8_t clipNum) { 
      if(channel >= 16 || clipNum >= CLIP_MAX) return nullptr;
      return &clips[channel * CLIP_MAX + clipNum]; 
    };

    SEQ_Step*   Step(SEQ_Pos position, bool AddNew = false) 
    {
      
      int16_t stepID = StepID(position);
      if (stepID == -1) return AddNew ? AddStep(SEQ_Step(), position) : nullptr;
      auto it = steps.find(stepID);
      if (it == steps.end()) return AddNew ? AddStep(SEQ_Step(), position) : nullptr;
      return &it->second;
    };

    SEQ_Step*   AddStep(SEQ_Step step, SEQ_Pos position) { return Add(position, step, steps, stepChanged, &SEQ_Clip::SetStepID, &SEQ_Clip::StepID);}

    void        DeleteStep(SEQ_Pos position) { Delete(position, steps, stepChanged, &SEQ_Clip::SetStepID, &SEQ_Clip::StepID); }

    SEQ_Autom*  Autom(SEQ_Pos position, bool AddNew = false) 
    {
      int16_t automID = StepID(position);
      auto it = automs.find(automID);
      if(it == automs.end())
        return AddNew ? AddAutom(SEQ_Autom(), position) : nullptr;
      return &it->second;
    }

    SEQ_Autom*  AddAutom(SEQ_Autom autom, SEQ_Pos position) { return Add(position, autom, automs, automChanged, &SEQ_Clip::SetAutomID, &SEQ_Clip::AutomID); }

    void        DeleteAutom(SEQ_Pos position) { Delete(position, automs, automChanged, &SEQ_Clip::SetAutomID, &SEQ_Clip::AutomID); }
    
    void    AddNotes(SEQ_Pos position, std::vector<SEQ_Note> notes)
    {
      for(uint8_t i = 0; i < notes.size() && i < STEP_MAX; i++)
        AddNote(position, notes[i], false);
      stepChanged.insert(StepID(position));
    }

    void    ClearNote(SEQ_Pos position, bool deleteEmptyStep = true) { ClearSub(position, &SEQ_Step::ClearNote, deleteEmptyStep); }

    bool    FindNote(SEQ_Pos position, uint8_t note) {if(Step(position)) return Step(position)->FindNote(note); return false;}

    bool    NoteEmpty(SEQ_Pos position) { if(Step(position)) return Step(position)->NoteEmpty(); return true;}

    uint8_t NoteCount(SEQ_Pos position) {if(Step(position)) return Step(position)->NoteCount(); return 0;}

    uint8_t GetVelocity(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->GetVelocity(note); return MatrixOS::UserVar::defaultVelocity;}

    void    SetVelocity(SEQ_Pos position, uint8_t velocity, uint8_t note = 255) {if(Step(position)) Step(position)->SetVelocity(note, velocity);}

    uint8_t GetGate(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->GetGate(note); return 0;}

    void    SetGate(SEQ_Pos position, uint8_t gate, uint8_t note = 255) {if(Step(position)) Step(position)->SetGate(note, gate);}

    uint8_t GetShift(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->GetShift(note); return 0;}

    void    SetShift(SEQ_Pos position, uint8_t shift, uint8_t note = 255) {if(Step(position)) Step(position)->SetShift(note, shift);}

    void    AddParams(SEQ_Pos position, std::vector<SEQ_Param> params)
    {
      for(uint8_t i = 0; i < params.size() && i < PARAM_MAX; i++)
        AddParam(position, params[i], false);
      stepChanged.insert(StepID(position));
    }

    void    ClearParam(SEQ_Pos position, bool deleteEmptyStep = true) { ClearSub(position, &SEQ_Step::ClearParam, deleteEmptyStep); }
    
    bool    ParamEmpty(SEQ_Pos position) { if(Step(position)) return Step(position)->ParamEmpty(); return true;}

    void CopyStep(SEQ_Pos src, SEQ_Pos dst)
    {
      auto it = steps.find(src.Pos());
      if(it == steps.end()) return;
      SEQ_Step step = it->second;
      AddStep(step, dst.Pos());
    }

    void CopyAutom(SEQ_Pos src, SEQ_Pos dst)
    {
      auto it = automs.find(src.Pos());
      if(it == automs.end()) return;
      SEQ_Autom autom = it->second;
      AddAutom(autom, dst.Pos());
    }

    void ClearClip(uint8_t channel, uint8_t clipNum)
    {
      if(channel >= 16 || clipNum >= CLIP_MAX) return;
      for (uint8_t bar = 0; bar < BAR_MAX; bar++) {
        for (uint8_t s = 0; s < STEP_MAX; s++)
        {
          int16_t stepID = Clip(channel, clipNum)->StepID(bar, s);
          if (stepID != -1)
            DeleteStep(Step(stepID)->Position());
        }
        for (uint8_t a = 0; a < AUTOM_ALL; a++) {
          int16_t automID = Clip(channel, clipNum)->AutomID(bar, a);
          if (automID != -1)
            DeleteAutom(Autom(automID)->Position());
        }
      }
    }

    void CopyClip(uint8_t channel_src, uint8_t clip_src, uint8_t channel_dst, uint8_t clip_dst)
    {
      if(channel_src >= 16 || clip_src >= CLIP_MAX) return;
      for (uint8_t bar = 0; bar < BAR_MAX; bar++) {
        for (uint8_t s = 0; s < STEP_MAX; s++)
        {
          int16_t stepID = Clip(channel_src, clip_src)->StepID(bar, s);
          if (stepID != -1)
            CopyStep(Step(stepID)->Position(), SEQ_Pos(channel_dst, clip_dst, bar, s));
        }
        for (uint8_t a = 0; a < PARAM_MAX; a++)
        {
          int16_t automID = Clip(channel_src, clip_src)->AutomID(bar, a);
          if(automID != -1)
            CopyAutom(Autom(automID)->Position(), SEQ_Pos(channel_dst, clip_dst, bar, a));
        }
      }
    }

  private:
    inline int16_t StepID(SEQ_Pos position) { return Clip(position.ChannelNum(), position.ClipNum())->StepID(position.BarNum(), position.Number()); }
    inline int16_t AutomID(SEQ_Pos position) { return Clip(position.ChannelNum(), position.ClipNum())->AutomID(position.BarNum(), position.Number()); }

    template<typename T>
    T* Add(SEQ_Pos position, T item, std::map<int16_t, T, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, T>>>& items, 
      std::set<int16_t>& changedItems, void (SEQ_Clip::*setID)(uint8_t, uint8_t, int16_t), int16_t (SEQ_Clip::*getID)(uint8_t, uint8_t) const)
    {
      item.SetPos(position);
      int16_t ID = (Clip(position.ChannelNum(), position.ClipNum())->*getID)(position.BarNum(), position.Number());
      if(ID < 0)
      {
        ID = items.empty() ? 0 : items.rbegin()->first + 1;
        items.emplace(ID, item);
        (Clip(position.ChannelNum(), position.ClipNum())->*setID)(position.BarNum(), position.Number(), ID);
      }
      else
        items.emplace(ID, item);
      changedItems.insert(ID);
      return &items[ID];
    }

    template<typename T>
    void Delete(SEQ_Pos position, std::map<int16_t, T, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, T>>>& items,
      std::set<int16_t>& changedItems, void (SEQ_Clip::*setID)(uint8_t, uint8_t, int16_t), int16_t (SEQ_Clip::*getID)(uint8_t, uint8_t) const)
    {
      int16_t ID = (Clip(position.ChannelNum(), position.ClipNum())->*getID)(position.BarNum(), position.Number());
      if(ID < 0) return;
      items.erase(ID);
      (Clip(position.ChannelNum(), position.ClipNum())->*setID)(position.BarNum(), position.Number(), -1);
      changedItems.insert(ID);
      if(items.empty() || items.rbegin()->first <= ID) return;

      //move last item to deleted ID position
      T lastItem = items.rbegin()->second;
      items.erase(items.rbegin()->first);
      items.emplace(ID, lastItem);
      (Clip(lastItem.ChannelNum(), lastItem.ClipNum())->*setID)(lastItem.BarNum(), lastItem.Number(), ID);
      
    }

    void AddNote(SEQ_Pos position, SEQ_Note note, bool markChange = true) { AddSub(position, note, &SEQ_Step::AddNote, markChange); }
    void AddParam(SEQ_Pos position, SEQ_Param param, bool markChange = true) { AddSub(position, param, &SEQ_Step::AddParam, markChange); }
    template<typename T>
    void AddSub(SEQ_Pos position, T item, bool (SEQ_Step::*addFunc)(T), bool markChange) 
    {
      auto step = Step(position,true);
      if(step)
      {
        (step->*addFunc)(item);
        if(markChange) stepChanged.insert(StepID(position));
      }
    }

    void DeleteNote(SEQ_Pos position, uint8_t note, bool deleteEmptyStep = true) { DeleteSub(position, note, &SEQ_Step::deleteNote, deleteEmptyStep); }
    void DeleteParam(SEQ_Pos position, uint8_t number, bool deleteEmptyStep = true) { DeleteSub(position, number, &SEQ_Step::deleteParam, deleteEmptyStep); }
    template<typename T>
    void DeleteSub(SEQ_Pos position, T item, bool (SEQ_Step::*deleteFunc)(T), bool deleteEmptyStep) 
    {
      auto step = Step(position);
      if(step) 
      {
        (step->*deleteFunc)(item);
        if(deleteEmptyStep && step->Empty()) DeleteStep(position);
        stepChanged.insert(StepID(position));
      }
    }

    void ClearSub(SEQ_Pos position, void (SEQ_Step::*clearFunc)(), bool deleteEmptyStep)
    {
      SEQ_Step* step = Step(position);
      if(step) 
      {
        (step->*clearFunc)(); 
        if(deleteEmptyStep && step->Empty()) DeleteStep(position);
        stepChanged.insert(StepID(position));
      }
    }
 
  };
}