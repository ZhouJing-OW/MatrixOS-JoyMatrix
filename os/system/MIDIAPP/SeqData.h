#pragma once
#include "MatrixOS.h"
// #include "MidiCenter.h"
#include <bitset>
#include <vector>
#include <map>
#include <set>

namespace MatrixOS::MidiCenter
{
  
  extern ChannelConfig*     channelConfig;
  class   SEQ_DataStore;

  #define PATTERN_MAX   32
  #define CLIP_MAX      8
  #define BAR_MAX       4
  #define STEP_MAX      16
  #define STEP_DIVISION 12
  #define NOTE_MAX      8
  #define PARAM_MAX     8
  #define AUTOM_MAX     STEP_MAX  // < 16
  #define STEP_ALL      16 * CLIP_MAX * BAR_MAX * STEP_MAX
  #define AUTOM_ALL     16 * CLIP_MAX * AUTOM_MAX

  class   SEQ_Pos
  {
    int32_t m_pos; // in the song data position
    uint8_t m_channel;
    uint8_t m_clip;
    uint8_t m_bar;
    uint8_t m_number; // step : STEP_MAX or autom:  BAR_MAX * PARAM_MAX

  public:
    bool operator< (const SEQ_Pos& other) const { return m_pos <  other.m_pos; }
    bool operator> (const SEQ_Pos& other) const { return m_pos >  other.m_pos; }
    bool operator==(const SEQ_Pos& other) const { return m_pos == other.m_pos; }

    SEQ_Pos(uint8_t channel, uint8_t clip, uint8_t bar, uint8_t number)
    { SetPos(channel, clip, bar, number); }

    SEQ_Pos(int32_t pos)
    { SetPos(pos); }

    void SetPos(uint8_t channel, uint8_t clip, uint8_t bar, uint8_t number)
    {
      if(channel >= 16 || clip >= CLIP_MAX || bar >= BAR_MAX || number >= STEP_MAX) return;
      m_channel   = channel;
      m_clip      = clip;
      m_bar       = bar;
      m_number    = number;
      m_pos       = (channel << 24) | (clip << 16) | (bar << 8) | number;
    }

    void SetPos(int32_t pos)
    {
      if(pos < 0 || pos >= 16 * CLIP_MAX * STEP_MAX) return;
      m_pos = pos;
      m_channel   = pos >> 24 & 0x0F;
      m_clip      = pos >> 16 & 0xFF;
      m_bar       = pos >> 8 & 0xFF;
      m_number    = pos & 0xFF;
    }

    int32_t PosNum()      { return m_pos; }
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
    int8_t  offset;  // half tick -5 to +5

    SEQ_Note(uint8_t note = 255, uint8_t velocity = MatrixOS::UserVar::defaultVelocity, uint8_t gate = 0, uint8_t offset = 0)
    {
      this->number = note;
      this->velocity = velocity;
      this->gate = gate;
      this->offset = offset;
    }
  };

  enum    PARAM_TYPE : uint8_t
  {
    PARAM_K01,      PARAM_K02,      PARAM_K03,      PARAM_K04,      PARAM_K05,      PARAM_K06,      PARAM_K07,      PARAM_K08,
    PARAM_K09,      PARAM_K10,      PARAM_K11,      PARAM_K12,      PARAM_K13,      PARAM_K14,      PARAM_K15,      PARAM_K16,
    PARAM_K17,      PARAM_K18,      PARAM_K19,      PARAM_K20,      PARAM_K21,      PARAM_K22,      PARAM_K23,      PARAM_K24,
    PARAM_K25,      PARAM_K26,      PARAM_K27,      PARAM_K28,      PARAM_K29,      PARAM_K30,      PARAM_K31,      PARAM_K32,
    PARAM_MOD,      PARAM_B02,      PARAM_B03,      PARAM_B04,      PARAM_B05,      PARAM_B06,      PARAM_B07,      PARAM_B08,
    PARAM_B09,      PARAM_B10,      PARAM_B11,      PARAM_B12,      PARAM_B13,      PARAM_B14,      PARAM_B15,      PARAM_B16,
    PARAM_B17,      PARAM_B18,      PARAM_B19,      PARAM_B20,      PARAM_B21,      PARAM_B22,      PARAM_B23,      PARAM_B24,     
    COMP_RETRIG,    COMP_CYCLE,     PARAM_PITCH,    COMP_RND_TRIG,  COMP_RND_PITCH, COP_RND_VEL,    COMP_RND_PARM,  COMP_RND_COMP
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
    int32_t PosNum()     { return position.PosNum(); }
    uint8_t ChannelNum() { return position.ChannelNum(); }
    uint8_t ClipNum()    { return position.ClipNum(); }
    uint8_t BarNum()     { return position.BarNum(); }
    uint8_t Number()     { return position.Number(); }
    void    SetPos(uint8_t channel, uint8_t clip, uint8_t bar, uint8_t step) { position.SetPos(channel, clip, bar, step); }
    void    SetPos(int32_t pos) { position.SetPos(pos); }
    void    SetPos(SEQ_Pos pos) { position = pos; }
  };

  class   SEQ_Autom : public SEQ_ClipItem
  {
    uint8_t number = 255;
    uint8_t value[STEP_MAX * STEP_DIVISION];

  public:
    SEQ_Autom() { memset(value, -1, sizeof(value)); }

    SEQ_Autom* CopyAutom(const SEQ_Autom& other)
    {
      this->number = other.number;
      for (uint8_t i = 0; i < STEP_MAX * STEP_DIVISION; i++)
        this->value[i] = other.value[i];
      return this;
    }
  };

  class   SEQ_Step  : public SEQ_ClipItem
  {
    SEQ_Note notes[NOTE_MAX];
    SEQ_Param params[PARAM_MAX];
    std::bitset<128> noteMark;
    std::bitset<64> paramMark;
    
   public:
    SEQ_Note noteTemplate = SEQ_Note();

    std::vector<SEQ_Note> GetNotes() const {if(NoteEmpty()) { std::vector<SEQ_Note> vec; return vec;}; return Get(notes); }
    bool FindNote(uint8_t note) const {if((note) > 127) return !noteMark.none(); return Find(noteMark, note); }
    bool AddNote(SEQ_Note note) { return Add(noteMark, notes, note); }
    bool DeleteNote(uint8_t note) { return Delete(noteMark, notes, note); }
    SEQ_Step* CopyNotes(const SEQ_Step& other)
    {
      this->noteMark = other.noteMark;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
        this->notes[i] = other.notes[i];
      this->noteTemplate = other.noteTemplate;
      return this;
    }
    void ClearNote()
    {
      for(uint8_t i = 0; i < NOTE_MAX ; i++ )
        notes[i] = SEQ_Note();
      noteMark.reset();
    }

    std::vector<SEQ_Param> GetParams() const {if(ParamEmpty()) { std::vector<SEQ_Param> vec; return vec;}; return Get(params); }
    bool FindParam(uint8_t number) const { if((number) > 127) return !noteMark.none(); return Find(paramMark, number); }
    bool AddParam(SEQ_Param param) { return Add(paramMark, params, param); }
    bool DeleteParam(uint8_t number) { return Delete(paramMark, params, number); }
    SEQ_Step*  CopyParam(const SEQ_Step& other)
    {
      this->paramMark = other.paramMark;
      for (uint8_t i = 0; i < PARAM_MAX; i++)
        this->params[i] = other.params[i];
      return this;
    }
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

    bool SetVelocity(uint8_t note, uint8_t velocity)
    {
      noteTemplate.velocity = velocity;
      if(NoteEmpty()) return false;
      
      bool set = false;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (notes[i].number <= 127 && (note > 127 || notes[i].number == note))
        { notes[i].velocity = velocity; set = true; }
      }
      return set;
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

    bool SetGate(uint8_t note, uint8_t gate)
    {
      noteTemplate.gate = gate;
      if(NoteEmpty()) return false;

      bool set = false;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (notes[i].number <= 127 && (note > 127 || notes[i].number == note))
        { notes[i].gate = gate; set = true; }
      }
      return set;
    }

    uint8_t GetOffset(uint8_t note) const
    {
      if (NoteEmpty()) return noteTemplate.offset;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if ((note > 127 && notes[i].number <= 127) || (note <= 127 && notes[i].number == note))
          return notes[i].offset;
      }
      return noteTemplate.offset;
    }

    bool SetOffset(uint8_t note, uint8_t offset)
    {
      noteTemplate.offset = offset;
      if(NoteEmpty()) return false;

      bool set = false;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (notes[i].number <= 127 && (note > 127 || notes[i].number == note))
        { notes[i].offset = offset; set = true;}
      }
      return set;
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
      for(size_t i = 0; i < N; i++)
        if(arr[i].number != 255) vec.push_back(arr[i]);
      return vec;
    }

    template <size_t M> 
    bool Find(const std::bitset<M>& map, uint8_t number) const 
    { 
      if(number >= M ) return false; 
      return map.test(number); 
    }

    template <typename T, size_t N, size_t M>
    bool Add(std::bitset<M>& map, T (&arr)[N], T item)
    {
      if(item.number >= M) return false;

      uint8_t emptyIndex = 255;
      for(uint8_t i = 0; i < N; i++)
      {
        if(arr[i].number == item.number)
        {
          arr[i] = item;
          return true;
        }
        else if(arr[i].number == 255 && emptyIndex == 255)
          emptyIndex = i;
      }

      if(emptyIndex != 255)
      {
        arr[emptyIndex] = item;
        map.set(item.number);
        return true;
      }

      return false;
    }

    template <typename T, size_t N, size_t M>
    bool Delete(std::bitset<M>& map, T (&arr)[N], uint8_t number)
    {
      if(number >= M) return false;
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

  class   SEQ_Clip
  {
  public:
    uint8_t speed           = 4;          // 4x, 3x, 2x, 1.5x, 1x, 1x, 1/2, 1/3, 1/4, 1/8
    uint8_t gate            = 50;         // 10% - 100%
    uint8_t quantize        = 0;          // 0% - 100%
    uint8_t barMax          = 1;          // 1 - 4
    uint8_t barStepMax      = STEP_MAX;   // 8 - 16
   
  private:
    std::bitset<BAR_MAX * STEP_MAX>  stepMark;
    std::bitset<BAR_MAX * AUTOM_MAX> automMark;
    int16_t stepID [BAR_MAX * STEP_MAX];
    int16_t automID[BAR_MAX * AUTOM_MAX];

  public:
    SEQ_Clip() { 
      memset(stepID,  -1, sizeof(stepID));
      memset(automID, -1, sizeof(automID));
      stepMark.reset(); automMark.reset();
    }

    void CopySettings(const SEQ_Clip& src)
    {
      this->quantize = src.quantize;
      this->barMax = src.barMax;
      this->barStepMax = src.barStepMax;
      this->speed = src.speed;
      this->gate = src.gate;
    }

    int16_t StepID (uint8_t BarNum, uint8_t stepNum) const 
    { 
      if(BarNum >= BAR_MAX || stepNum >= STEP_MAX) return -1;
      return stepID [BarNum * STEP_MAX + stepNum]; 
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
      stepMark[BarNum * STEP_MAX + stepNum] = (id != -1);
    }
    void SetAutomID(uint8_t BarNum, uint8_t automNum, int16_t id) 
    { 
      automID[BarNum * AUTOM_MAX + automNum] = id;
      automMark[BarNum * AUTOM_MAX + automNum] = (id != -1);
    }

    friend class SEQ_DataStore;
  };

  class   SEQ_Pattern
  {
    public:
    std::bitset<16> mute;
    uint8_t ClipNum[16];
    uint16_t nodesIndex[NODES_MAX_CONFIGS * NODES_PER_CHANNEL];
    uint8_t repeat;
  };

  class   SEQ_Song
  {
    public:
    uint8_t editingClip[16];
    uint8_t playingClip[16];
    int8_t patternID[PATTERN_MAX];

    SEQ_Song() 
    { 
      memset(editingClip, 0, sizeof(editingClip));
      memset(playingClip, 0, sizeof(playingClip));
      memset(patternID, -1, sizeof(patternID)); 
    }
  };

  enum    CapState : uint8_t { CAP_NORMAL, CAP_EDIT, };
  extern  std::vector<std::pair<SEQ_Pos, SEQ_Step*>>   CNTR_SeqEditStep;

  class   SEQ_Capture 
  {
    SEQ_Step capStep = SEQ_Step();
    std::set<uint8_t> inputList;
    CapState state = CAP_NORMAL;
    bool editingStepEmpty = true;

   public:
    void Capture(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      if(channel != MatrixOS::UserVar::global_channel) return;

      switch(state)
      {
        case CAP_NORMAL:
          NormalCapture(channel, byte1, byte2); break;
        case CAP_EDIT:
          EditCapture(channel, byte1, byte2); break;
      }
    }

    void ChangeState(CapState changeState , bool updateCap = false)
    {
      switch(changeState)
      {
        case CAP_NORMAL:
          inputList.clear(); 
          if(updateCap && !CNTR_SeqEditStep.empty())
            capStep.CopyNotes(*(CNTR_SeqEditStep.begin()->second));
          CNTR_SeqEditStep.clear();
          break;
        case CAP_EDIT: break;
      }
      this->state = changeState;
    }

    void Editing(SEQ_Pos pos, SEQ_Step* step)
    {
      editingStepEmpty = step->NoteEmpty();
      if (!inputList.empty())
        step->CopyNotes(capStep);
      CNTR_SeqEditStep.push_back({pos, step});
      ChangeState(CAP_EDIT);
    }

    void EditCapture(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      if (byte2 > 0) 
      {
        SEQ_Step* step = CNTR_SeqEditStep.front().second;
        if(!step) return;
        if (step->FindNote(byte1)) step->DeleteNote(byte1);
        else step->AddNote(SEQ_Note(byte1, byte2, step->noteTemplate.gate, 0));
        return;
      }
      inputList.erase(byte1);
    }

    void NormalCapture(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      if (byte2 > 0) 
      {
        if (inputList.empty()) capStep.ClearNote();
        inputList.emplace(byte1);
        capStep.AddNote(SEQ_Note(byte1, byte2, capStep.noteTemplate.gate, 0));
        return;
      }
      inputList.erase(byte1);
      capStep.DeleteNote(byte1);
    }

    void Clear(bool resetState = true)
    {
      if(resetState) state = CAP_NORMAL;
      CNTR_SeqEditStep.clear();
      capStep.ClearNote();
      inputList.clear();
      editingStepEmpty = true;
    }

    void EndEditing()
    {
      state = CAP_NORMAL;
      CNTR_SeqEditStep.clear();
    }

    bool EditingStepEmpty() const { return editingStepEmpty; }

    SEQ_Step* GetCap(){return &capStep;}
  };

  class   SEQ_DataStore
  {
    SEQ_Song song;
    std::array<SEQ_Clip, 16 * CLIP_MAX> clips;
    std::map<int16_t, SEQ_Pattern, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Pattern>>> patterns; // patternID, pattern
    std::map<int16_t, SEQ_Step,    std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Step >>>   steps;    // stepID, step
    std::map<int16_t, SEQ_Autom,   std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Autom>>>   automs;   // automID, autom
    std::set<int16_t> patternChanged, stepChanged, automChanged; // stepID, automID
    uint8_t clipCount[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    SEQ_Capture capturer;
    bool inited = false;

   public:
    void Init()
    {
      new (&song)   SEQ_Song();
      new (&clips)  std::array<SEQ_Clip, 16 * CLIP_MAX>();
      new (&steps)  std::map<int16_t, SEQ_Step,  std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Step >>>();
      new (&automs) std::map<int16_t, SEQ_Autom, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Autom>>>();
      new (&stepChanged)  std::set<int16_t>();
      new (&automChanged) std::set<int16_t>();
      new (&capturer) SEQ_Capture();
      inited = true;
    }

    void Destroy()
    {
      song.~SEQ_Song();
      clips.~array();
      steps.~map();
      automs.~map();
      stepChanged.~set();
      automChanged.~set();
      capturer.~SEQ_Capture();
    }

    ~SEQ_DataStore() { if(inited) Destroy(); }

    uint8_t EditingClip(uint8_t channel) { return song.editingClip[channel]; }

    void SetEditingClip(uint8_t channel, uint8_t clipNum) { song.editingClip[channel] = clipNum; }

    uint8_t PlayingClip(uint8_t channel) { return song.playingClip[channel]; }

    SEQ_Pattern* Pattern(uint8_t patternNum) { return &patterns[patternNum]; }

    SEQ_Pattern* InsertPattern(uint8_t patternNum) { return &patterns[patternNum];}

    void DeletePattern(uint8_t patternNum) {}

    void Capture(uint8_t channel, uint8_t byte1, uint8_t byte2) { capturer.Capture(channel, byte1, byte2); }

    void Capture_ChangeState(CapState state, bool updateCap = false) { capturer.ChangeState(state, updateCap); }

    void Capture_UpdateCap(SEQ_Pos position) { if(Step(position)) capturer.GetCap()->CopyNotes(*Step(position)); }

    void Capture_Editing(SEQ_Pos position) {
      SEQ_Step* step = Step(position, true);
      if(step) capturer.Editing(position, step);
    }

    void Capture_SaveHold(SEQ_Pos position)
    {
      SEQ_Step* step = Step(position);
      if(!step) return;

      if(step->Empty()) 
      {
        step->ClearNote();
        capturer.ChangeState(CAP_NORMAL, !capturer.EditingStepEmpty());
        if (step->Empty()) DeleteStep(position);
        return; 
      }

      capturer.ChangeState(CAP_NORMAL, true);
    }

    void Capture_SaveClick(SEQ_Pos position)
    {
      if(!capturer.EditingStepEmpty())
      {
        ClearNote(position);
        capturer.ChangeState(CAP_NORMAL);
        return;
      }

      if(capturer.GetCap()->NoteEmpty())
      {
        if(channelConfig != nullptr)
          AddNote(position, SEQ_Note(channelConfig->activeNote[position.ChannelNum()], MatrixOS::UserVar::defaultVelocity));
      }
      else
        Step(position)->CopyNotes(*(capturer.GetCap()));

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
      
      capturer.ChangeState(CAP_NORMAL);
    }

    void Capture_Clear() { capturer.Clear(); }

    uint8_t Capture_CapCount() { return capturer.GetCap()->NoteCount(); }

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

    bool    FindNote(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->FindNote(note); return false;}

    bool    FindNote(uint8_t channel, uint8_t clipNum, uint8_t barNum, uint8_t note = 255)
    {
      for(uint8_t i = 0; i < STEP_MAX; i++)
      {
        SEQ_Pos pos(channel, clipNum, barNum, i);
        if(FindNote(pos, note)) return true;
      }
      return false;
    }

    bool    ClipEmpty(uint8_t channel, uint8_t clipNum) { return Clip(channel, clipNum)->Empty(); }

    bool    NoteEmpty(SEQ_Pos position) { if(Step(position)) return Step(position)->NoteEmpty(); return true;}

    uint8_t NoteCount(SEQ_Pos position, uint8_t note = 255) 
    {
      if(!Step(position)) return 0;
      if(note > 127)return Step(position)->NoteCount();
      return Step(position)->FindNote(note);
    }

    uint8_t GetVelocity(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->GetVelocity(note); return MatrixOS::UserVar::defaultVelocity;}

    void    SetVelocity(SEQ_Pos position, uint8_t velocity, uint8_t note = 255) 
    { 
      if(Step(position)) 
        if(Step(position)->SetVelocity(note, velocity))
          stepChanged.insert(StepID(position));
    }

    uint8_t GetGate(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->GetGate(note); return 0;}

    void    SetGate(SEQ_Pos position, uint8_t gate, uint8_t note = 255) 
    {
      if(Step(position)) 
        if (Step(position)->SetGate(note, gate))
          stepChanged.insert(StepID(position));
    }

    uint8_t GetOffset(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->GetOffset(note); return 0;}

    void    SetOffset(SEQ_Pos position, uint8_t offset, uint8_t note = 255) 
    {
      if(Step(position)) 
        if(Step(position)->SetOffset(note, offset))
          stepChanged.insert(StepID(position));
    }

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
      if(src == dst) return;
      int16_t stepID = StepID(src);
      if (stepID < 0) { DeleteStep(dst); return; }

      auto it = steps.find(stepID);
      if(it == steps.end()) return;

      SEQ_Step* stepDst = Step(dst, true);
      if(!stepDst) return;

      stepDst->CopyNotes(it->second);
      stepDst->CopyParam(it->second);
      stepChanged.insert(StepID(dst));
    }

    void CopyAutom(SEQ_Pos src, SEQ_Pos dst)
    {
      if(src == dst) return;
      int16_t automID = AutomID(src);
      if (automID < 0) { DeleteAutom(dst); return; }

      auto it = automs.find(automID);
      if(it == automs.end()) return;

      SEQ_Autom* automDst = Autom(dst, true);
      if(!automDst) return;

      automDst->CopyAutom(it->second);
      automChanged.insert(AutomID(dst));
    }

    void CopyBar(SEQ_Pos src, SEQ_Pos dst)
    {
      // MLOGD("SEQ Copy From","Channel %d. Clip %d. Bar %d.", src.ChannelNum(), src.ClipNum(), src.BarNum());
      // MLOGD("SEQ Copy To  ","Channel %d. Clip %d. Bar %d.", dst.ChannelNum(), dst.ClipNum(), dst.BarNum());
      if(src == dst) return;
      for (uint8_t s = 0; s < STEP_MAX;  s++)
      { CopyStep (SEQ_Pos(src.ChannelNum(), src.ClipNum(), src.BarNum(), s), SEQ_Pos(dst.ChannelNum(), dst.ClipNum(), dst.BarNum(), s)); }
      for (uint8_t a = 0; a < AUTOM_MAX; a++)
      { CopyAutom(SEQ_Pos(src.ChannelNum(), src.ClipNum(), src.BarNum(), a), SEQ_Pos(dst.ChannelNum(), dst.ClipNum(), dst.BarNum(), a)); }
    }

    void ClearBar(SEQ_Pos pos) 
    { 
      for (uint8_t s = 0; s < STEP_MAX;  s++) 
      { DeleteStep (SEQ_Pos(pos.ChannelNum(), pos.ClipNum(), pos.BarNum(), s)); }
      for (uint8_t a = 0; a < AUTOM_MAX; a++)
      { DeleteAutom(SEQ_Pos(pos.ChannelNum(), pos.ClipNum(), pos.BarNum(), a)); }
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
      if(channel_src == channel_dst && clip_src == clip_dst) return;
      for (uint8_t bar = 0; bar < BAR_MAX; bar++) {
        for (uint8_t s = 0; s < STEP_MAX; s++)
          CopyStep (SEQ_Pos(channel_src, clip_src, bar, s), SEQ_Pos(channel_dst, clip_dst, bar, s));
        for (uint8_t a = 0; a < PARAM_MAX; a++)
          CopyAutom(SEQ_Pos(channel_src, clip_src, bar, a), SEQ_Pos(channel_dst, clip_dst, bar, a));
      }
      Clip(channel_dst, clip_dst)->CopySettings(*Clip(channel_src, clip_src));
    }

  private:
    inline int16_t StepID (SEQ_Pos position) { return Clip(position.ChannelNum(), position.ClipNum())->StepID (position.BarNum(), position.Number()); }
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

      string type = changedItems == stepChanged ? "Step" : "Autom";
      MLOGD("SEQ Add " + type, "channel %d. clip %d. Bar %d. Step %d. ID %d.", 
            position.ChannelNum(), position.ClipNum(), position.BarNum(),position.Number(), ID);
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

      string type = changedItems == stepChanged ? "Step" : "Autom";
      MLOGD("SEQ Delete " + type, "channel %d. clip %d. Bar %d. Step %d. ID %d.", 
            position.ChannelNum(), position.ClipNum(), position.BarNum(),position.Number(), ID);
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

    void DeleteNote(SEQ_Pos position, uint8_t note, bool deleteEmptyStep = true) { DeleteSub(position, note, &SEQ_Step::DeleteNote, deleteEmptyStep); }
    void DeleteParam(SEQ_Pos position, uint8_t number, bool deleteEmptyStep = true) { DeleteSub(position, number, &SEQ_Step::DeleteParam, deleteEmptyStep); }
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