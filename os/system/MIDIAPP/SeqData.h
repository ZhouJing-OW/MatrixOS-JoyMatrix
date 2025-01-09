#pragma once
#include "MatrixOS.h"
// #include "MidiCenter.h"
#include <bitset>
#include <vector>
#include <map>
#include <set>

namespace MatrixOS::MidiCenter
{
  extern  ChannelConfig*     channelConfig;
  extern  TransportState     transportState;
  class   SEQ_DataStore;
  extern  SEQ_DataStore* seqData;

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

  enum    PARAM_TYPE : uint8_t
  {
    PARAM_K00,      PARAM_K01,      PARAM_K02,      PARAM_K03,      PARAM_K04,      PARAM_K05,      PARAM_K06,      PARAM_K07,
    PARAM_K08,      PARAM_K09,      PARAM_K10,      PARAM_K11,      PARAM_K12,      PARAM_K13,      PARAM_K14,      PARAM_K15,
    PARAM_K16,      PARAM_K17,      PARAM_K18,      PARAM_K19,      PARAM_K20,      PARAM_K21,      PARAM_K22,      PARAM_K23,
    PARAM_K24,      PARAM_K25,      PARAM_K26,      PARAM_K27,      PARAM_K28,      PARAM_K29,      PARAM_K30,      PARAM_K31,
    PARAM_K32,      PARAM_K33,      PARAM_K34,      PARAM_K35,      PARAM_K36,      PARAM_K37,      PARAM_K38,      PARAM_K39,
    PARAM_K40,      PARAM_K41,      PARAM_K42,      PARAM_K43,      PARAM_K44,      PARAM_K45,      PARAM_K46,      PARAM_K47,
    PARAM_K48,      PARAM_K49,      PARAM_K50,      PARAM_K51,      PARAM_K52,      PARAM_K53,      PARAM_K54,      PARAM_K55,
    PARAM_K56,      PARAM_K57,      PARAM_K58,      PARAM_K59,      PARAM_K60,      PARAM_K61,      PARAM_K62,      PARAM_K63,
  };

  enum    COMP_TYPE : uint8_t
  {
    COMP_CYCLE,     COMP_RETRIG,    COMP_CHANCE,    COMP_FLAM,      COMP_OCTAVE,    COMP_PITCH,     COMP_STRUM,     COMP_ARP,   
  };

  struct  SEQ_Note                     
  {
    uint8_t note;
    uint8_t velocity;
    uint8_t gate;    
    int8_t  offset;
    int8_t  tair;    // 127 : use clip setting.

    uint8_t cycleStep;
    uint8_t cycleLength;
    uint8_t retrig;
    uint8_t retrigDecay;
    uint8_t chance;
    uint8_t flamVel;
    uint8_t flamTime;
    bool    randomPitch;
    int8_t  octaveShift;
    int8_t  pitchShift;
    

    SEQ_Note(uint8_t note = 255, uint8_t velocity = MatrixOS::UserVar::defaultVelocity, uint8_t gate = 0, int8_t offset = 0, int8_t tair = 127)
    {
      CompInit();
      this->note = note;
      this->velocity = velocity;
      this->gate = gate;
      this->offset = offset;
      this->tair = tair;
    }

    void            CompInit()
    {
      this->chance = 100;
      this->cycleLength = 1;
      this->cycleStep = 0b00000001;
      this->retrigDecay = true;
      this->retrig = 1;
      this->flamVel = 0;
      this->flamTime = 0;
      this->randomPitch = false;
      this->octaveShift = 0;
      this->pitchShift = 0;
    }

    void            CopyComp (const SEQ_Note& other)
    {
      this->chance          = other.chance;
      this->cycleLength     = other.cycleLength;
      this->cycleStep       = other.cycleStep;
      this->retrig          = other.retrig;
      this->randomPitch     = other.randomPitch;
      this->octaveShift     = other.octaveShift;
      this->pitchShift      = other.pitchShift;
      this->flamTime        = other.flamTime;
    }

    bool            FindComp(COMP_TYPE type) const
    {
      switch(type)
      {
        case COMP_CYCLE:     return cycleLength != 1;
        case COMP_RETRIG:    return retrig != 1;
        case COMP_CHANCE:    return chance != 100;
        case COMP_FLAM:      return flamTime;
        case COMP_OCTAVE:    return octaveShift;
        case COMP_PITCH:     return pitchShift;
        default: break;
      }
      return false;
    }

  };

  struct  SEQ_Param
  {
    uint8_t type    = 255;  // param type
    int16_t byte2   = 0;    // value
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
    int8_t strumType = -1, strumSpeed = 0, arpConfig = -1;

    bool            Empty() const { return noteMark.none() && paramMark.none(); } 

    //------------------------------------NOTE-------------------------------------//

    std::vector
    <const SEQ_Note*>      GetNotes() 
    {
      std::vector<const SEQ_Note*> vec;
      if(NoteEmpty())  return vec;
      for(size_t i = 0; i < NOTE_MAX; i++)
        if(notes[i].note != 255) vec.push_back(notes + i);
      return vec;
    }

    bool            NoteEmpty() const { return noteMark.none(); }

    uint8_t         NoteCount() const { return noteMark.count(); }

    bool            NoteFull()  const { return noteMark.all(); }

    bool            FindNote(uint8_t note) const {if((note) > 127) return !noteMark.none(); return noteMark.test(note); }

    bool            AddNote(SEQ_Note note) 
    { 
      if(note.note >= 128) return false;

      uint8_t emptyIndex = 255;
      for(uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if(notes[i].note == note.note && note.offset == notes[i].offset)
        {
          notes[i] = note;
          return true;
        }
        else if(notes[i].note == 255 && emptyIndex == 255)
          emptyIndex = i;
      }

      if(emptyIndex != 255)
      {
        notes[emptyIndex] = note;
        noteMark.set(note.note);
        return true;
      }

      return false;
    }

    bool            DeleteNote(uint8_t note, int8_t offsetPos = 127) 
    { 
      if(note >= 128) return false;

      uint8_t count = 0;
      uint8_t deleted = 0;
      for(uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if(notes[i].note == note && (offsetPos > 60 || notes[i].offset == offsetPos))
        {
          notes[i].note = 255;
          noteMark.reset(note);
          deleted++;
        }
          count++;
      }
      if(deleted == count) noteMark.reset(note);
      return deleted > 0;
    }

    SEQ_Step*       CopyNotes(const SEQ_Step& other)
    {
      this->noteMark = other.noteMark;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
        this->notes[i] = other.notes[i];
      this->noteTemplate = other.noteTemplate;
      return this;
    }

    void            ClearNote()
    {
      for(uint8_t i = 0; i < NOTE_MAX ; i++ )
        notes[i] = SEQ_Note();
      noteMark.reset();
    }

    uint8_t         GetVelocity(uint8_t note, int8_t offsetPos) const 
    {
      if (NoteEmpty()) return noteTemplate.velocity;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (((note > 127 && notes[i].note <= 127) || (note <= 127 && notes[i].note == note)) && (offsetPos > 60 || notes[i].offset == offsetPos))
          return notes[i].velocity;
      }
      return noteTemplate.velocity;
    }

    uint8_t         GetGate(uint8_t note) const
    {
      if (NoteEmpty()) return noteTemplate.gate;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if ((note > 127 && notes[i].note <= 127) || (note <= 127 && notes[i].note == note))
          return notes[i].gate;
      }
      return noteTemplate.gate;
    }

    uint8_t         GetOffset(uint8_t note) const
    {
      if (NoteEmpty()) return noteTemplate.offset;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if ((note > 127 && notes[i].note <= 127) || (note <= 127 && notes[i].note == note))
          return notes[i].offset;
      }
      return noteTemplate.offset;
    }

    bool            SetVelocity(uint8_t note, uint8_t velocity, int8_t offsetPos)
    {
      noteTemplate.velocity = velocity;
      if(NoteEmpty()) return false;
      
      bool set = false;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if ((notes[i].note <= 127 && (note > 127 || notes[i].note == note)) && (offsetPos > 60 || notes[i].offset == offsetPos))
        { notes[i].velocity = velocity; set = true; }
      }
      return set;
    }

    bool            SetGate(uint8_t note, uint8_t gate, int8_t tair = 127)
    {
      noteTemplate.gate = gate;
      if(NoteEmpty()) return false;

      bool set = false;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (notes[i].note <= 127 && (note > 127 || notes[i].note == note))
        { 
          notes[i].gate = gate;
          notes[i].tair = tair <= 100 ? tair : 127;
          set = true; 
        }
      }
      return set;
    }

    bool            SetOffset(uint8_t note, uint8_t offset)
    {
      noteTemplate.offset = offset;
      if(NoteEmpty()) return false;
      std::set<uint8_t> exist;
      bool set = false;
      for (uint8_t i = 0; i < NOTE_MAX; i++)
      {
        if (notes[i].note <= 127 && (note > 127 || notes[i].note == note))
        { 
          if(exist.find(notes[i].note) != exist.end())
          {
            notes[i] = SEQ_Note();
            continue;
          }
          notes[i].offset = offset; 
          exist.insert(notes[i].note);
          set = true;
        }
      }
      return set;
    }

    //------------------------------------COMP-------------------------------------//

    bool            FindStepComp(COMP_TYPE type) const
    {
      switch(type)
      {
        case COMP_CYCLE:     return noteTemplate.cycleLength  != 1;
        case COMP_RETRIG:    return noteTemplate.retrig       != 1;
        case COMP_CHANCE:    return noteTemplate.chance       != 100;
        case COMP_FLAM:      return noteTemplate.flamTime     != 0;
        case COMP_OCTAVE:    return noteTemplate.octaveShift  != 0;
        case COMP_PITCH:     return noteTemplate.pitchShift   != 0;
        case COMP_STRUM:     return strumType                 >= 0;
        case COMP_ARP:       return arpConfig                 >= 0;
      }
      return false;
    }

    void            InitChance() { InitComp(&SEQ_Note::chance, uint8_t(100)); }

    void            InitCycle() { InitComp(&SEQ_Note::cycleLength, uint8_t(1)); InitComp(&SEQ_Note::cycleStep, uint8_t(0b00000001)); }

    void            InitRetrig() { InitComp(&SEQ_Note::retrig, uint8_t(1)); InitComp(&SEQ_Note::retrigDecay, uint8_t(0));}

    void            InitFlam() { InitComp(&SEQ_Note::flamTime, uint8_t(0)); InitComp(&SEQ_Note::flamVel, uint8_t(0)); }

    void            InitOctaveShift() { InitComp(&SEQ_Note::octaveShift, int8_t(0));}

    void            InitPitchShift() { InitComp(&SEQ_Note::pitchShift, int8_t(0));}

    void            SetChance(uint8_t chance, uint8_t note = 255) { SetComp(&SEQ_Note::chance, chance, uint8_t(100), note); }

    void            SetCycleLength(uint8_t length, uint8_t note = 255) { SetComp(&SEQ_Note::cycleLength, length, uint8_t(1), note); }

    void            SetCycleStep(uint8_t CycleStep, uint8_t note = 255) { SetComp(&SEQ_Note::cycleStep, CycleStep, uint8_t(0b00000001), note); }

    void            SetRetrig(uint8_t retrig, uint8_t note = 255) { SetComp(&SEQ_Note::retrig, retrig, uint8_t(1), note); };

    void            SetRetrigDecay(uint8_t retrigDecay, uint8_t note = 255) { SetComp(&SEQ_Note::retrigDecay, retrigDecay, uint8_t(0), note); }

    void            SetFlamTime(uint8_t flamTime, uint8_t note = 255) { SetComp(&SEQ_Note::flamTime, flamTime, uint8_t(0), note); }

    void            SetFlamVel(uint8_t flamVel, uint8_t note = 255) { SetComp(&SEQ_Note::flamVel, flamVel, uint8_t(0), note); }

    void            SetRandomPitch(bool randomPitch, uint8_t note = 255) { SetComp(&SEQ_Note::randomPitch, randomPitch, false, note); }

    void            SetOctaveShift(int8_t octaveShift, uint8_t note = 255) { SetComp(&SEQ_Note::octaveShift, octaveShift, int8_t(0), note); }

    void            SetPitchShift(int8_t pitchShift, uint8_t note = 255) { SetComp(&SEQ_Note::pitchShift, pitchShift, int8_t(0), note); }

    template <typename T>
    void            InitComp(T SEQ_Note::*comp, T defaultValue)
    {
      noteTemplate.*comp = defaultValue;
      for(uint8_t i = 0; i < NOTE_MAX; i++)
        notes[i].*comp = defaultValue;
    } 

    template <typename T>
    void            SetComp(T SEQ_Note::*comp, T value, T defaultValue, uint8_t note = 255)
    {
      if(note == 255) 
      {
        noteTemplate.*comp = value;
        for(uint8_t i = 0; i < NOTE_MAX; i++)
          notes[i].*comp = static_cast<T>(defaultValue);
      }
      else
      {
        for(uint8_t i = 0; i < NOTE_MAX; i++)
          if(notes[i].note == note) notes[i].*comp = value;
        noteTemplate.*comp = static_cast<T>(defaultValue);
      }
    }

    //------------------------------------PARAM------------------------------------//

    std::vector
    <SEQ_Param>     GetParams()  const 
    {
      std::vector<SEQ_Param> vec;
      if(ParamEmpty())  return vec;
      for(size_t i = 0; i < NOTE_MAX; i++)
        if(params[i].type != 255) vec.push_back(params[i]);
      return vec;
    }

    bool            ParamEmpty() const { return paramMark.none(); }

    uint8_t         ParamCount() const { return paramMark.count(); }

    bool            ParamFull()  const { return paramMark.all(); }

    bool            FindParam(uint8_t type) const { if((type) > 127) return !noteMark.none(); return paramMark.test(type);  }

    bool            AddParam(SEQ_Param param) 
    { 
      if(param.type >= 64) return false;

      uint8_t emptyIndex = 255;
      for(uint8_t i = 0; i < PARAM_MAX; i++)
      {
        if(params[i].type == param.type)
        {
          params[i] = param;
          return true;
        }
        else if(params[i].type == 255 && emptyIndex == 255)
          emptyIndex = i;
      }

      if(emptyIndex != 255)
      {
        params[emptyIndex] = param;
        paramMark.set(param.type);
        return true;
      }

      return false;
    }

    bool            DeleteParam(uint8_t type, uint8_t note) 
    { 
      if(type >= 64) return false;
      for(uint8_t i = 0; i < PARAM_MAX; i++)
      {
        if(params[i].type == type)
        {
          params[i].type = 255;
          paramMark.reset(type);
          return true;
        }
      }
      return false; 
    }

    SEQ_Step*       CopyParam(const SEQ_Step& other)
    {
      this->paramMark = other.paramMark;
      for (uint8_t i = 0; i < PARAM_MAX; i++)
        this->params[i] = other.params[i];
      return this;
    }

    void            ClearParam()
    {
      for(uint8_t i = 0; i < PARAM_MAX; i++)
        params[i] = SEQ_Param();
      paramMark.reset();
    }

  };

  class   SEQ_Clip
  {
  public:
    uint8_t speed           = 4;          // 4x, 3x, 2x, 1.5x, 1x, 1x, 1/2, 1/3, 1/4, 1/8
    uint8_t direction       = 0;          // -->, <--, <->, Random
    uint8_t tair            = 50;         // 10% - 100%
    uint8_t quantize        = 100;        // 0% - 100%
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

    void            CopySettings(const SEQ_Clip& src)
    {
      this->quantize = src.quantize;
      this->barMax = src.barMax;
      this->barStepMax = src.barStepMax;
      this->speed = src.speed;
      this->tair = src.tair;
    }

    int16_t         StepID (uint8_t BarNum, uint8_t stepNum) const 
    { 
      if(BarNum >= BAR_MAX || stepNum >= STEP_MAX) return -1;
      return stepID [BarNum * STEP_MAX + stepNum]; 
    }

    int16_t         AutomID(uint8_t BarNum, uint8_t automNum) const 
    { 
      if(BarNum >= BAR_MAX || automNum >= AUTOM_MAX) return -1;
      return automID[BarNum * AUTOM_MAX + automNum]; 
    }

    bool            Empty() const { return stepMark.none() && automMark.none(); }

  private:
    void            SetStepID(uint8_t BarNum, uint8_t stepNum, int16_t id) 
    { 
      stepID[BarNum * STEP_MAX + stepNum] = id;
      stepMark[BarNum * STEP_MAX + stepNum] = (id != -1);
    }

    void            SetAutomID(uint8_t BarNum, uint8_t automNum, int16_t id) 
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

  enum    PickState : uint8_t { PICK_NORMAL, PICK_EDIT, };
  enum    PickBlock : bool    { UNBLOCK,    BLOCK,    };
  extern  std::vector<std::pair<SEQ_Pos, SEQ_Step*>>   CNTR_SeqEditStep;
  class   SEQ_Pick 
  {
    SEQ_Step capStep = SEQ_Step();
    std::set<uint8_t> inputList;
    PickState state = PICK_NORMAL;
    bool editingStepEmpty = true;

   public:
    PickBlock        Pick(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      if(channel != MatrixOS::UserVar::global_channel) return UNBLOCK;

      switch(state)
      {
        case PICK_NORMAL:
          return NormalPick(channel, byte1, byte2);
        case PICK_EDIT:
          return EditPick(channel, byte1, byte2);
      }
      return UNBLOCK;
    }

    PickBlock        EditPick(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      if (byte2 > 0) 
      {
        SEQ_Step* step = CNTR_SeqEditStep.front().second;
        if(!step) return PickBlock(transportState.play); // when play, block input
        if (step->FindNote(byte1)) step->DeleteNote(byte1);
        else step->AddNote(SEQ_Note(byte1, byte2, step->noteTemplate.gate, 0));
        return PickBlock(transportState.play);
      }
      if (inputList.find(byte1) != inputList.end())
      {
        inputList.erase(byte1);
        return UNBLOCK;
      }
      return PickBlock(transportState.play);
    }

    PickBlock        NormalPick(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      if (byte2 > 0) 
      {
        if (inputList.empty()) capStep.ClearNote();
        inputList.emplace(byte1);
        capStep.AddNote(SEQ_Note(byte1, byte2, capStep.noteTemplate.gate, 0));
        return UNBLOCK;
      }
        inputList.erase(byte1);
        capStep.DeleteNote(byte1);
      return UNBLOCK;
    }

    void            ChangeState(PickState changeState , bool updatePick = false)
    {
      switch(changeState)
      {
        case PICK_NORMAL:
          inputList.clear(); 
          if(updatePick && !CNTR_SeqEditStep.empty())
            capStep.CopyNotes(*(CNTR_SeqEditStep.begin()->second));
          CNTR_SeqEditStep.clear();
          break;
        case PICK_EDIT: break;
      }
      this->state = changeState;
    }

    void            Editing(SEQ_Pos pos, SEQ_Step* step)
    {
      editingStepEmpty = step->NoteEmpty();
      if (!inputList.empty())
        step->CopyNotes(capStep);
      CNTR_SeqEditStep.push_back({pos, step});
      ChangeState(PICK_EDIT);
    }

    void            Clear(bool resetState = true)
    {
      if(resetState) state = PICK_NORMAL;
      CNTR_SeqEditStep.clear();
      capStep.ClearNote();
      inputList.clear();
      editingStepEmpty = true;
    }

    void            EndEditing()
    {
      state = PICK_NORMAL;
      CNTR_SeqEditStep.clear();
    }

    bool            EditingStepEmpty() const { return editingStepEmpty; }

    SEQ_Step*       GetPick(){return &capStep;}
  };

  class   SEQ_DataStore
  {
    SEQ_Song song;
    std::array<SEQ_Clip, 16 * CLIP_MAX> clips;
    std::map<int16_t, SEQ_Pattern, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Pattern>>> patterns; // patternID, pattern
    std::map<int16_t, SEQ_Step,    std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Step >>>   steps;    // stepID, step
    std::map<int16_t, SEQ_Autom,   std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Autom>>>   automs;   // automID, autom
    std::set<int16_t> patternChanged, stepChanged, automChanged; // stepID, automID
    std::set<SEQ_Step*> compEdit;
    SEQ_Pick pick;
    bool inited = false;


   public:
    void Init()
    {
      new (&song)   SEQ_Song();
      new (&clips)  std::array<SEQ_Clip, 16 * CLIP_MAX>();
      new (&steps)  std::map<int16_t, SEQ_Step,  std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Step >>>();
      new (&automs) std::map<int16_t, SEQ_Autom, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, SEQ_Autom>>>();
      new (&patternChanged) std::set<int16_t>();
      new (&stepChanged)    std::set<int16_t>();
      new (&automChanged)   std::set<int16_t>();
      new (&compEdit)       std::set<SEQ_Step*>();
      new (&pick)   SEQ_Pick();
      inited = true;
    }

    void Destroy()
    {
      song.~SEQ_Song();
      clips.~array();
      steps.~map();
      automs.~map();
      patternChanged.~set();
      stepChanged.~set();
      automChanged.~set();
      compEdit.~set();
      pick.~SEQ_Pick();
    }

    ~SEQ_DataStore() { if(inited) Destroy(); }


    //------------------------------------NOTE-------------------------------------//

    void            AddNote(SEQ_Pos position, SEQ_Note note, bool markChange = true) 
    { 
      auto step = Step(position,true);
      if(step)
      {
        (step->AddNote)(note);
        if(markChange) stepChanged.insert(StepID(position));
      }
    }

    void            AddNotes(SEQ_Pos position, std::vector<SEQ_Note> notes)
    {
      for(uint8_t i = 0; i < notes.size() && i < STEP_MAX; i++)
        AddNote(position, notes[i], false);
      stepChanged.insert(StepID(position));
    }

    void            DeleteNote(SEQ_Pos position, uint8_t note, int8_t offset = 127, bool deleteEmptyStep = true) 
    { 
      auto step = Step(position);
      if(step) 
      {
        (step->DeleteNote)(note, offset);
        if(deleteEmptyStep && step->Empty()) DeleteStep(position);
        stepChanged.insert(StepID(position));
      }
    }

    void            ClearNote(SEQ_Pos position, bool deleteEmptyStep = true) 
    { 
      SEQ_Step* step = Step(position);
      if(step) 
      {
        (step->ClearNote)(); 
        if(deleteEmptyStep && step->Empty()) DeleteStep(position);
        stepChanged.insert(StepID(position));
      }
    }

    bool            FindNoteInBar(uint8_t channel, uint8_t clipNum, uint8_t barNum, uint8_t note = 255)
    {
      for(uint8_t i = 0; i < STEP_MAX; i++)
      {
        SEQ_Pos pos(channel, clipNum, barNum, i);
        if(FindNote(pos, note)) return true;
      }
      return false;
    }

    bool            FindNote(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->FindNote(note); return false;}

    bool            NoteEmpty(SEQ_Pos position) { if(Step(position)) return Step(position)->NoteEmpty(); return true;}

    uint8_t         NoteCount(SEQ_Pos position, uint8_t note = 255) 
    {
      if(!Step(position)) return 0;
      if(note > 127)return Step(position)->NoteCount();
      return Step(position)->FindNote(note);
    }

    uint8_t         GetVelocity(SEQ_Pos position, uint8_t note = 255, int8_t offsetPos = 127) {if(Step(position)) return Step(position)->GetVelocity(note, offsetPos); return MatrixOS::UserVar::defaultVelocity;}

    uint8_t         GetGate(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->GetGate(note); return 0;}

    uint8_t         GetOffset(SEQ_Pos position, uint8_t note = 255) {if(Step(position)) return Step(position)->GetOffset(note); return 0;}

    void            SetVelocity(SEQ_Pos position, uint8_t velocity, uint8_t note = 255, int8_t offsetPos = 127) 
    { 
      if(Step(position)) 
        if(Step(position)->SetVelocity(note, velocity, offsetPos))
          stepChanged.insert(StepID(position));
    }

    void            SetGate(SEQ_Pos position, uint8_t gate, uint8_t note = 255, int8_t tair = 127) 
    {
      if(Step(position)) 
        if (Step(position)->SetGate(note, gate, tair))
          stepChanged.insert(StepID(position));
    }

    void            SetOffset(SEQ_Pos position, uint8_t offset, uint8_t note = 255) 
    {
      if(Step(position)) 
        if(Step(position)->SetOffset(note, offset))
          stepChanged.insert(StepID(position));
    }

    //------------------------------------COMP-------------------------------------//

    bool            Comp_InEditing() { return !compEdit.empty(); }

    void            Comp_AddEdit(SEQ_Pos position)
    {
      SEQ_Step* step = Step(position);
      if(step && !step->NoteEmpty())
        compEdit.insert(step);
    }

    void            Comp_DelEdit(SEQ_Pos position)
    {
      SEQ_Step* step = Step(position);
      if(step && compEdit.find(step) != compEdit.end())
      {
        stepChanged.insert(StepID(position));
        compEdit.erase(step);
      }
    }

    void            Comp_EndEditing()
    {
      for (auto step : compEdit)
        stepChanged.insert(StepID(step->Position()));
      compEdit.clear();
    } 

    void            Comp_InitChance() { Comp_Init(&SEQ_Step::InitChance); }

    void            Comp_InitCycle() { Comp_Init(&SEQ_Step::InitCycle); }

    void            Comp_InitRetrig() { Comp_Init(&SEQ_Step::InitRetrig); }

    void            Comp_InitFlam() { Comp_Init(&SEQ_Step::InitFlam); }

    void            Comp_InitOctaveShift() { Comp_Init(&SEQ_Step::InitOctaveShift); }

    void            Comp_InitPitchShift() { Comp_Init(&SEQ_Step::InitPitchShift); }

    void            Comp_SetChance(uint8_t chance, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetChance, chance, note); }

    void            Comp_SetCycleLength(uint8_t length, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetCycleLength, length, note); }

    void            Comp_SetCycleStep(uint8_t CycleStep, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetCycleStep, CycleStep, note); }

    void            Comp_SetRetrig(uint8_t retrig, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetRetrig, retrig, note); }

    void            Comp_SetRetrigDecay(uint8_t retrigDecay, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetRetrigDecay, retrigDecay, note); }

    void            Comp_SetFlamTime(uint8_t flamTime, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetFlamTime, flamTime, note); }

    void            Comp_SetFlamVel(uint8_t flamVel, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetFlamVel, flamVel, note); }

    void            Comp_SetRandomPitch(bool randomPitch, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetRandomPitch, randomPitch, note); }

    void            Comp_SetOctaveShift(int8_t octaveShift, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetOctaveShift, octaveShift, note); }

    void            Comp_SetPitchShift(int8_t pitchShift, uint8_t note = 255) { Comp_Set(&SEQ_Step::SetPitchShift, pitchShift, note); }

    template <typename F>
    void            Comp_Init(F SEQ_Step::*comp)
    {
      for (auto step : compEdit)
        (step->*comp)();
    }

    template <typename F, typename T>
    void            Comp_Set(F SEQ_Step::*comp, T value, uint8_t note = 255)
    {
      for (auto step : compEdit)
        (step->*comp)(value, note);
    }

    //------------------------------------PARAM------------------------------------//

    void            AddParam(SEQ_Pos position, SEQ_Param param, bool markChange = true) 
    { 
      auto step = Step(position,true);
      if(step)
      {
        (step->AddParam)(param);
        if(markChange) stepChanged.insert(StepID(position));
      }
    }

    void            AddParams(SEQ_Pos position, std::vector<SEQ_Param> params)
    {
      for(uint8_t i = 0; i < params.size() && i < PARAM_MAX; i++)
        AddParam(position, params[i], false);
      stepChanged.insert(StepID(position));
    }

    void            DeleteParam(SEQ_Pos position, uint8_t type, uint8_t note = 255, bool deleteEmptyStep = true) 
    { 
      auto step = Step(position);
      if(step) 
      {
        (step->DeleteParam)(type, note);
        if(deleteEmptyStep && step->Empty()) DeleteStep(position);
        stepChanged.insert(StepID(position));
      }
    }

    void            ClearParam(SEQ_Pos position, bool deleteEmptyStep = true) 
    { 
      SEQ_Step* step = Step(position);
      if(step) 
      {
        (step->ClearParam)(); 
        if(deleteEmptyStep && step->Empty()) DeleteStep(position);
        stepChanged.insert(StepID(position));
      }
    }
    
    bool            ParamEmpty(SEQ_Pos position) { if(Step(position)) return Step(position)->ParamEmpty(); return true;}

    //------------------------------------STEP-------------------------------------//

    SEQ_Step*       Step(SEQ_Pos position, bool AddNew = false) 
    {
      
      int16_t stepID = StepID(position);
      if (stepID == -1) return AddNew ? AddStep(SEQ_Step(), position) : nullptr;
      auto it = steps.find(stepID);
      if (it == steps.end()) return AddNew ? AddStep(SEQ_Step(), position) : nullptr;
      return &it->second;
    };

    SEQ_Step*       AddStep(SEQ_Step step, SEQ_Pos position) { return Add(position, step, steps, stepChanged, &SEQ_Clip::SetStepID, &SEQ_Clip::StepID);}

    void            DeleteStep(SEQ_Pos position) { Delete(position, steps, stepChanged, &SEQ_Clip::SetStepID, &SEQ_Clip::StepID); }

    void            CopyStep(SEQ_Pos src, SEQ_Pos dst)
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

    //------------------------------------AUTOM------------------------------------//

    SEQ_Autom*      Autom(SEQ_Pos position, bool AddNew = false) 
    {
      int16_t automID = StepID(position);
      auto it = automs.find(automID);
      if(it == automs.end())
        return AddNew ? AddAutom(SEQ_Autom(), position) : nullptr;
      return &it->second;
    }

    SEQ_Autom*      AddAutom(SEQ_Autom autom, SEQ_Pos position) { return Add(position, autom, automs, automChanged, &SEQ_Clip::SetAutomID, &SEQ_Clip::AutomID); }

    void            DeleteAutom(SEQ_Pos position) { Delete(position, automs, automChanged, &SEQ_Clip::SetAutomID, &SEQ_Clip::AutomID); }

    void            CopyAutom(SEQ_Pos src, SEQ_Pos dst)
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

    //------------------------------------BAR--------------------------------------//

    void            CopyBar(SEQ_Pos src, SEQ_Pos dst)
    {
      // MLOGD("SEQ Copy From","Channel %d. Clip %d. Bar %d.", src.ChannelNum(), src.ClipNum(), src.BarNum());
      // MLOGD("SEQ Copy To  ","Channel %d. Clip %d. Bar %d.", dst.ChannelNum(), dst.ClipNum(), dst.BarNum());
      if(src == dst) return;
      for (uint8_t s = 0; s < STEP_MAX;  s++)
      { CopyStep (SEQ_Pos(src.ChannelNum(), src.ClipNum(), src.BarNum(), s), SEQ_Pos(dst.ChannelNum(), dst.ClipNum(), dst.BarNum(), s)); }
      for (uint8_t a = 0; a < AUTOM_MAX; a++)
      { CopyAutom(SEQ_Pos(src.ChannelNum(), src.ClipNum(), src.BarNum(), a), SEQ_Pos(dst.ChannelNum(), dst.ClipNum(), dst.BarNum(), a)); }
    }

    void            ClearBar(SEQ_Pos pos) 
    { 
      for (uint8_t s = 0; s < STEP_MAX;  s++) 
      { DeleteStep (SEQ_Pos(pos.ChannelNum(), pos.ClipNum(), pos.BarNum(), s)); }
      for (uint8_t a = 0; a < AUTOM_MAX; a++)
      { DeleteAutom(SEQ_Pos(pos.ChannelNum(), pos.ClipNum(), pos.BarNum(), a)); }
    }

    //------------------------------------CLIP-------------------------------------//

    SEQ_Clip*       Clip(uint8_t channel, uint8_t clipNum) { 
      if(channel >= 16 || clipNum >= CLIP_MAX) return nullptr;
      return &clips[channel * CLIP_MAX + clipNum]; 
    };

    bool            ClipEmpty(uint8_t channel, uint8_t clipNum) { return Clip(channel, clipNum)->Empty(); }

    uint8_t         EditingClip(uint8_t channel) { return song.editingClip[channel]; }

    uint8_t         PlayingClip(uint8_t channel) { return song.playingClip[channel]; }

    void            SetEditingClip(uint8_t channel, uint8_t clipNum) { song.editingClip[channel] = clipNum; }

    void            ClearClip(uint8_t channel, uint8_t clipNum)
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

    void            CopyClip(uint8_t channel_src, uint8_t clip_src, uint8_t channel_dst, uint8_t clip_dst)
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

    //------------------------------------PATTERN----------------------------------//

    SEQ_Pattern*    Pattern(uint8_t patternNum) { return &patterns[patternNum]; }

    SEQ_Pattern*    InsertPattern(uint8_t patternNum) { return &patterns[patternNum];}

    void            DeletePattern(uint8_t patternNum) {}

    //--------------------------------------PICK------------------------------------//

    PickBlock       Pick(uint8_t channel, uint8_t byte1, uint8_t byte2) { return pick.Pick(channel, byte1, byte2); }

    void            Pick_ChangeState(PickState state, bool updatePick = false) { pick.ChangeState(state, updatePick); }

    void            Pick_Update(SEQ_Pos position) { if(Step(position)) pick.GetPick()->CopyNotes(*Step(position)); }

    void            Pick_Editing(SEQ_Pos position) {
      SEQ_Step* step = Step(position, true);
      if(step) pick.Editing(position, step);
    }

    void            Pick_SaveHold(SEQ_Pos position)
    {
      SEQ_Step* step = Step(position);
      if(!step) return;

      if(step->Empty()) 
      {
        step->ClearNote();
        pick.ChangeState(PICK_NORMAL, !pick.EditingStepEmpty());
        if (step->Empty()) DeleteStep(position);
        return; 
      }

      pick.ChangeState(PICK_NORMAL, true);
    }

    void            Pick_SaveClick(SEQ_Pos position)
    {
      if(!pick.EditingStepEmpty())
      {
        ClearNote(position);
        pick.ChangeState(PICK_NORMAL);
        return;
      }

      if(pick.GetPick()->NoteEmpty())
      {
        if(channelConfig != nullptr)
          AddNote(position, SEQ_Note(channelConfig->activeNote[position.ChannelNum()], MatrixOS::UserVar::defaultVelocity));
      }
      else
        Step(position)->CopyNotes(*(pick.GetPick()));

      pick.ChangeState(PICK_NORMAL);
    }

    void            Pick_SaveSingle(SEQ_Pos position)
    {
      uint8_t channel = MatrixOS::UserVar::global_channel;
      uint8_t activeNote = channelConfig->activeNote[channel];
      if(!FindNote(position, activeNote))
        AddNote(position, SEQ_Note{activeNote, (uint8_t)MatrixOS::UserVar::defaultVelocity});
      else 
        DeleteNote(position, activeNote);
      
      pick.ChangeState(PICK_NORMAL);
    }

    void            Pick_Clear() { pick.Clear(); }

    uint8_t         Pick_PickCount() { return pick.GetPick()->NoteCount(); }

    void            Pick_EndEditing() { pick.EndEditing(); }

  private:
    inline int16_t  StepID (SEQ_Pos position) { return Clip(position.ChannelNum(), position.ClipNum())->StepID (position.BarNum(), position.Number()); }
    inline int16_t  AutomID(SEQ_Pos position) { return Clip(position.ChannelNum(), position.ClipNum())->AutomID(position.BarNum(), position.Number()); }

    template<typename T>
    T*              Add(SEQ_Pos position, T item, std::map<int16_t, T, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, T>>>& items, 
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
    void            Delete(SEQ_Pos position, std::map<int16_t, T, std::less<int16_t>, PSRAMAllocator<std::pair<const int16_t, T>>>& items,
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

  };
}
