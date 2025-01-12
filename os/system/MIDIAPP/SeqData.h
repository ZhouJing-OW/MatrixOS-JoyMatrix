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
  #define BAR_MAX       16
  #define STEP_MAX      16
  #define STEP_DIVISION 12
  #define NOTE_MAX      8
  #define PARAM_MAX     8
  #define AUTOM_MAX     STEP_MAX  // < 16
  #define STEP_ALL      16 * CLIP_MAX * BAR_MAX * STEP_MAX
  #define AUTOM_ALL     16 * CLIP_MAX * AUTOM_MAX
  #define UNDO_POINT_MAX 20

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

    void            SetRetrig(uint8_t retrig, uint8_t note = 255) { SetComp(&SEQ_Note::retrig, retrig, uint8_t(1), note); }

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
    int8_t  loopStart       = -1;         // -1 表示未设置 loop
    int8_t  loopEnd         = -1;
    uint8_t activeBar       = 0;          // 添加activeBar变量
    
    bool HasLoop() const { return loopStart >= 0 && loopEnd >= 0; }
    void ClearLoop() { loopStart = -1; loopEnd = -1; }
    void SetLoop(int8_t start, int8_t end)
    {
        // 确保start和end在有效范围内
        if (start < 0 || end < 0 || start >= BAR_MAX || end >= BAR_MAX || start > end) {
            return;
        }
        
        // 如果loop范围超出当前barMax，则更新barMax
        if (end >= barMax) {
            barMax = end + 1;
        }
        
        loopStart = start;
        loopEnd = end;

        checkActiveBar();
    }
    void checkActiveBar() {
      if (!HasLoop())
      {
        if (activeBar >= barMax) {
          activeBar = barMax - 1;
        }
      } else {
        if (activeBar > loopEnd || activeBar < loopStart) {
          activeBar = loopStart;
        }
      }
    }

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
      this->speed = src.speed;
      this->quantize = src.quantize;
      this->direction = src.direction;
      this->tair = src.tair;
      this->barMax = src.barMax;
      this->barStepMax = src.barStepMax;
      this->loopStart = src.loopStart;
      this->loopEnd = src.loopEnd;
      this->activeBar = src.activeBar;
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

    bool            Empty(int8_t bar = -1) const { 
      if(bar == -1) return stepMark.none() && automMark.none();
      return stepMark[bar * STEP_MAX + 0] == 0 && automMark[bar * AUTOM_MAX + 0] == 0;
    }

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
    bool changed = false;

   public:
    PickBlock         Pick(uint8_t channel, uint8_t byte1, uint8_t byte2)
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

    PickBlock         EditPick(uint8_t channel, uint8_t byte1, uint8_t byte2)
    {
      if (byte2 > 0) 
      {
        
        SEQ_Step* step = CNTR_SeqEditStep.front().second;
        if(!step) return PickBlock(transportState.play); // when play, block input
        changed = true;
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

    PickBlock         NormalPick(uint8_t channel, uint8_t byte1, uint8_t byte2)
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

    void              ChangeState(PickState changeState , bool updatePick = false)
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

    void              Editing(SEQ_Pos pos, SEQ_Step* step)
    {
      editingStepEmpty = step->NoteEmpty();
      if (!inputList.empty())
        step->CopyNotes(capStep);
      CNTR_SeqEditStep.push_back({pos, step});
      ChangeState(PICK_EDIT);
    }

    void              Clear(bool resetState = true)
    {
      if(resetState) state = PICK_NORMAL;
      CNTR_SeqEditStep.clear();
      capStep.ClearNote();
      inputList.clear();
      editingStepEmpty = true;
      changed = false;
    }

    void              EndEditing()
    {
      state = PICK_NORMAL;
      CNTR_SeqEditStep.clear();
    }

    bool              EditingStepEmpty() const { return editingStepEmpty; }

    bool              Changed() const { return changed; }

    SEQ_Step*         GetPick(){return &capStep;}
  };

  struct  SEQ_Snapshot {
    int16_t undoPoint = -1;
    SEQ_Pos position = SEQ_Pos(0);
    SEQ_Clip clip;                      // 保存整个clip
    std::map<int16_t, SEQ_Step> steps;  // 保存clip引用的所有steps
    std::map<int16_t, SEQ_Autom> automs;// 保存clip引用的所有automs
  };

  struct  SEQ_History
  {
    std::vector<SEQ_Snapshot, PSRAMAllocator<SEQ_Snapshot>> snapshots; 
    SEQ_Snapshot tempSnapshot;
    uint32_t timestamp = 0;
    int16_t currentUndoPoint = 0;
    int16_t lastUndoPoint = 0;
    int16_t firstUndoPoint = 0;
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
    SEQ_History history;
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
      new (&history) SEQ_History();
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
      history.~SEQ_History();
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

    void            CopyNote(SEQ_Pos src, SEQ_Pos dst, uint8_t note)
    {
        // 1. 检查源位置是否有指定音符
        if (!FindNote(src, note)) return;

        // 2. 获取源位置的 step
        SEQ_Step* srcStep = Step(src);
        if (!srcStep) return;

        // 3. 获取音符列表并找到目标音符
        std::vector<const SEQ_Note*> notes = srcStep->GetNotes();
        for (const SEQ_Note* srcNote : notes) {
            if (srcNote->note == note) {
                AddNote(dst, *srcNote);
                break;
            }
        }
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

    void            ShiftStep(SEQ_Pos position, int8_t distance, uint8_t note = 255)
    {
        SEQ_Clip* clip = Clip(position.ChannelNum(), position.ClipNum());
        if (!clip || distance == 0) return;

        // 获取当前step
        SEQ_Step* step = Step(position);
        if (!step) return;

        // 确定移动范围
        uint16_t startBar = clip->HasLoop() ? clip->loopStart : 0;
        uint16_t endBar = clip->HasLoop() ? clip->loopEnd : clip->barMax - 1;
        uint16_t totalSteps = (endBar - startBar + 1) * clip->barStepMax;

        // 计算当前位置相对于范围起点的偏移
        int32_t relativePos = (position.BarNum() - startBar) * clip->barStepMax + position.Number();
        
        // 计算新位置
        int32_t newRelativePos = relativePos + distance;
        while (newRelativePos < 0) newRelativePos += totalSteps;
        newRelativePos %= totalSteps;
        
        // 转换回绝对位置
        uint16_t newBar = startBar + (newRelativePos / clip->barStepMax);
        uint16_t newStep = newRelativePos % clip->barStepMax;
        SEQ_Pos newPos(position.ChannelNum(), position.ClipNum(), newBar, newStep);

        if (note != 255) {
            // 移动特定音符
            if (step->FindNote(note)) {
                std::vector<const SEQ_Note*> notes = step->GetNotes();
                for (const SEQ_Note* stepNote : notes) {
                    if (stepNote->note == note) {
                        // 先添加到新位置
                        AddNote(newPos, *stepNote);
                        // 再从原位置删除
                        DeleteNote(position, note, stepNote->offset, true);
                    }
                }
            }
        } else {
            // 移动整个step
            SEQ_Step tempStep = *step;
            DeleteStep(position);
            AddStep(tempStep, newPos);
        }
    }

    void            TransposeStep(SEQ_Pos position, int8_t interval, uint16_t scale = 0b111111111111, uint8_t root = 0, uint8_t note = 255)
    {
        SEQ_Clip* clip = Clip(position.ChannelNum(), position.ClipNum());
        if (!clip) return;

        // 获取当前step
        SEQ_Step* step = Step(position);
        if (!step) return;

        // 根据根音转位音阶
        uint16_t shiftedScale = ((scale << root) | (scale >> (12 - root))) & 0xFFF;

        // 收集并排序音符
        struct NoteInfo {
            SEQ_Note note;
            int8_t newNote;
        };
        std::vector<NoteInfo> notesToMove;

        // 收集音符并计算新音符值
        std::vector<const SEQ_Note*> notes = step->GetNotes();
        for (const SEQ_Note* stepNote : notes) {
            // 如果指定了音符且不匹配，则跳过
            if (note != 255 && stepNote->note != note) continue;

            // 计算新的音符值
            int8_t newNote = stepNote->note + interval;
            
            // 如果不是八度转调，需要找到最近的音阶音
            if (interval % 12 != 0) {
                // 计算相对音阶位置
                int8_t notePos = (newNote + 12) % 12;
                
                // 如果新位置不在音阶内，根据转调方向寻找最近的音阶音
                if (!(shiftedScale & (1 << notePos))) {
                    if (interval > 0) {
                        // 向上找最近的音阶音
                        for (int8_t i = 0; i < 12; i++) {
                            int8_t testPos = (notePos + i) % 12;
                            if (shiftedScale & (1 << testPos)) {
                                newNote += i;
                                break;
                            }
                        }
                    } else {
                        // 向下找最近的音阶音
                        for (int8_t i = 0; i < 12; i++) {
                            int8_t testPos = (notePos - i + 12) % 12;
                            if (shiftedScale & (1 << testPos)) {
                                newNote -= i;
                                break;
                            }
                        }
                    }
                }
            }

            // 确保音符在有效范围内
            if (newNote >= 0) {
                notesToMove.push_back({*stepNote, newNote});
            }
        }

        // 根据转调方向排序
        if (interval > 0) {
            // 向上转调时从高音到低音处理
            std::sort(notesToMove.begin(), notesToMove.end(), 
                [](const NoteInfo& a, const NoteInfo& b) { return a.note.note > b.note.note; });
        } else {
            // 向下转调时从低音到高音处理
            std::sort(notesToMove.begin(), notesToMove.end(), 
                [](const NoteInfo& a, const NoteInfo& b) { return a.note.note < b.note.note; });
        }

        // 按排序顺序移动音符
        for (const auto& noteInfo : notesToMove) {
            DeleteNote(position, noteInfo.note.note, noteInfo.note.offset, true);
            SEQ_Note newNote = noteInfo.note;
            newNote.note = noteInfo.newNote;
            AddNote(position, newNote);
        }
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

    void            ClearBar(SEQ_Pos pos, uint8_t note = 255) 
    { 
        SEQ_Clip* clip = Clip(pos.ChannelNum(), pos.ClipNum());
        if (!clip) return;

        // 如果指定了音符，则只处理该音符
        if (note != 255) {
            bool hasOtherNotes = false;
            for (uint8_t s = 0; s < STEP_MAX; s++) {
                SEQ_Pos stepPos(pos.ChannelNum(), pos.ClipNum(), pos.BarNum(), s);
                SEQ_Step* step = Step(stepPos);
                if (step) {
                    if (step->FindNote(note)) {
                        // 获取所有音符并逐个检查删除
                        std::vector<const SEQ_Note*> notes = step->GetNotes();
                        for (const SEQ_Note* stepNote : notes) {
                            if (stepNote->note == note) {
                                DeleteNote(stepPos, note, stepNote->offset, true);
                            }
                        }
                    }
                    if (!step->NoteEmpty()) {
                        hasOtherNotes = true;
                    }
                }
            }
            // 如果是最后一个bar且没有任何音符，则删除整个bar
            if (pos.BarNum() == clip->barMax - 1 && !hasOtherNotes) {
                clip->barMax--;
                clip->checkActiveBar();
            }
            return;
        }

        // 清空当前bar的所有内容
        for (uint8_t s = 0; s < STEP_MAX; s++) {
            DeleteStep(SEQ_Pos(pos.ChannelNum(), pos.ClipNum(), pos.BarNum(), s));
        }
        for (uint8_t a = 0; a < AUTOM_MAX; a++) {
            DeleteAutom(SEQ_Pos(pos.ChannelNum(), pos.ClipNum(), pos.BarNum(), a));
        }

        // 如果是最后一个bar且不是第一个bar，则删除这个bar
        if (pos.BarNum() > 0 && pos.BarNum() == clip->barMax - 1) {
            clip->barMax--;
            clip->checkActiveBar();
        }
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
            DeleteStep(SEQ_Pos(channel, clipNum,bar, s));
        }
        for (uint8_t a = 0; a < AUTOM_MAX; a++) {
          int16_t automID = Clip(channel, clipNum)->AutomID(bar, a);
          if (automID != -1)
            DeleteAutom(SEQ_Pos(channel, clipNum,bar, a));
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

    void            ShiftClip(SEQ_Pos position, int8_t distance, uint8_t note = 255)
    {
        SEQ_Clip* clip = Clip(position.ChannelNum(), position.ClipNum());
        if (!clip || distance == 0) return;

        // 确定移动范围
        uint16_t startBar = clip->HasLoop() ? clip->loopStart : 0;
        uint16_t endBar = clip->HasLoop() ? clip->loopEnd : clip->barMax - 1;
        uint16_t totalSteps = (endBar - startBar + 1) * clip->barStepMax;

        // 使用map存储所有需要移动的steps，键为原始位置
        std::map<SEQ_Pos, SEQ_Step> tempSteps;
        
        // 1. 收集所有需要移动的steps
        for (uint16_t bar = startBar; bar <= endBar; bar++) {
            for (uint16_t step = 0; step < clip->barStepMax; step++) {
                SEQ_Pos curPos(position.ChannelNum(), position.ClipNum(), bar, step);
                SEQ_Step* curStep = Step(curPos);
                if (curStep) {
                    if (note != 255) {
                        // 只移动特定音符
                        if (curStep->FindNote(note)) {
                            SEQ_Step newStep;
                            std::vector<const SEQ_Note*> notes = curStep->GetNotes();
                            for (const SEQ_Note* stepNote : notes) {
                                if (stepNote->note == note) {
                                    newStep.AddNote(*stepNote);
                                }
                            }
                            if (!newStep.Empty()) {
                                tempSteps[curPos] = newStep;
                            }
                        }
                    } else {
                        // 移动整个step
                        tempSteps[curPos] = *curStep;
                    }
                }
            }
        }

        // 2. 删除原位置的steps/notes
        for (const auto& pair : tempSteps) {
            SEQ_Pos oldPos = pair.first;
            if (note != 255) {
                SEQ_Step* step = Step(oldPos);
                if (step) {
                    step->DeleteNote(note);
                    if (step->Empty()) {
                        DeleteStep(oldPos);
                    }
                }
            } else {
                DeleteStep(oldPos);
            }
        }

        // 3. 将steps写入新位置
        for (const auto& pair : tempSteps) {
            SEQ_Pos oldPos = pair.first;
            SEQ_Step step = pair.second;
            
            // 计算相对位置
            int32_t relativePos = (oldPos.BarNum() - startBar) * clip->barStepMax + oldPos.Number();
            
            // 计算新位置
            int32_t newRelativePos = relativePos + distance;
            while (newRelativePos < 0) newRelativePos += totalSteps;
            newRelativePos %= totalSteps;
            
            // 转换回绝对位置
            uint16_t newBar = startBar + (newRelativePos / clip->barStepMax);
            uint16_t newStep = newRelativePos % clip->barStepMax;
            
            // 添加到新位置
            SEQ_Pos newPos(position.ChannelNum(), position.ClipNum(), newBar, newStep);
            if (note != 255) {
                // 如果是移动特定音符，需要将音符添加到目标位置的step中
                std::vector<const SEQ_Note*> notes = step.GetNotes();
                for (const SEQ_Note* stepNote : notes) {
                    AddNote(newPos, *stepNote);
                }
            } else {
                // 如果是移动整个step，直接添加step
                AddStep(step, newPos);
            }
        }
    }

    void            TransposeClip(SEQ_Pos position, int8_t interval, uint16_t scale = 0b111111111111, uint8_t root = 0, uint8_t note = 255)
    {
        SEQ_Clip* clip = Clip(position.ChannelNum(), position.ClipNum());
        if (!clip) return;

        // 确定转调范围
        uint16_t startBar = clip->HasLoop() ? clip->loopStart : 0;
        uint16_t endBar = clip->HasLoop() ? clip->loopEnd : clip->barMax - 1;

        // 遍历范围内的所有steps
        for (uint16_t bar = startBar; bar <= endBar; bar++) {
            for (uint16_t step = 0; step < clip->barStepMax; step++) {
                SEQ_Pos stepPos(position.ChannelNum(), position.ClipNum(), bar, step);
                SEQ_Step* curStep = Step(stepPos);
                if (curStep) {
                    // 对当前step进行转调
                    TransposeStep(stepPos, interval, scale, root, note);
                }
            }
        }
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

    bool            Pick_SaveHold(SEQ_Pos position)
    {
      bool changed = pick.Changed();
      SEQ_Step* step = Step(position);
      if(!step) return changed;

      if(step->Empty()) 
      {
        step->ClearNote();
        pick.ChangeState(PICK_NORMAL, !pick.EditingStepEmpty());
        if (step->Empty()) DeleteStep(position);
        return changed; 
      }

      pick.ChangeState(PICK_NORMAL, true);
      return changed;
    }

    bool            Pick_SaveClick(SEQ_Pos position)
    {
      if(!pick.EditingStepEmpty())
      {
        ClearNote(position);
        pick.ChangeState(PICK_NORMAL);
        return true;
      }

      if(pick.GetPick()->NoteEmpty())
      {
        if(channelConfig != nullptr)
          AddNote(position, SEQ_Note(channelConfig->activeNote[position.ChannelNum()], MatrixOS::UserVar::defaultVelocity));
      }
      else
        Step(position)->CopyNotes(*(pick.GetPick()));

      pick.ChangeState(PICK_NORMAL);
      return true;
    }

    bool            Pick_SaveSingle(SEQ_Pos position)
    {
      uint8_t channel = MatrixOS::UserVar::global_channel;
      uint8_t activeNote = channelConfig->activeNote[channel];
      if(!FindNote(position, activeNote))
        AddNote(position, SEQ_Note{activeNote, (uint8_t)MatrixOS::UserVar::defaultVelocity});
      else 
        DeleteNote(position, activeNote);
      
      pick.ChangeState(PICK_NORMAL);
      return true;
    }

    void            Pick_Clear() { pick.Clear(); }

    uint8_t         Pick_PickCount() { return pick.GetPick()->NoteCount(); }

    void            Pick_EndEditing() { pick.EndEditing(); }

    //----------------------------------UNDO REDO------------------------------------//

    void            CreateTempSnapshot(SEQ_Pos position, bool ignoreTimeStamp = false)
    {
      uint32_t timestamp = MatrixOS::SYS::Millis();
      SEQ_Pos lastPosition = SEQ_Pos(0);
      if(!history.snapshots.empty()) {
        lastPosition = history.snapshots.back().position;
      }
      // 如果当前快照是同一个clip且时间间隔小于1秒，则不创建新快照
      if (!ignoreTimeStamp && timestamp - history.timestamp < 1000 &&
          lastPosition.ChannelNum() == position.ChannelNum() &&
          lastPosition.ClipNum() == position.ClipNum()) {
          history.timestamp = timestamp;
          return;
      }
      history.timestamp = ignoreTimeStamp ? 0 : timestamp;

      // 暂存clip
      SEQ_Clip* clip = Clip(position.ChannelNum(), position.ClipNum());
      history.tempSnapshot.clip = *clip;
      history.tempSnapshot.undoPoint = history.currentUndoPoint;
      history.tempSnapshot.position = position;

      // 保存clip引用的所有steps
      for (uint8_t bar = 0; bar < clip->barMax; bar++) {
          for (uint8_t step = 0; step < clip->barStepMax; step++) {
              int16_t stepID = clip->StepID(bar, step);
              if (stepID >= 0) {history.tempSnapshot.steps[stepID] = steps[stepID];}
          }
      }

      // 保存clip引用的所有automs
      for (uint8_t bar = 0; bar < clip->barMax; bar++) {
          for (uint8_t autom = 0; autom < AUTOM_MAX; autom++) {
              int16_t automID = clip->AutomID(bar, autom);
              if (automID >= 0) {history.tempSnapshot.automs[automID] = automs[automID];}
          }
      }
      //MLOGD("SEQ", "CreateTempSnapshot. undoPoint %d.", history.currentUndoPoint);
    }

    void            EnableTempSnapshot() {
      if(history.tempSnapshot.undoPoint == -1) return;

      RemoveSnapshotFrom(history.currentUndoPoint);
      history.snapshots.push_back(history.tempSnapshot);
      history.lastUndoPoint = history.currentUndoPoint;
      history.currentUndoPoint++;
      //MLOGD("SEQ", "EnableTempSnapshot. lastUndoPoint %d. currentUndoPoint %d.", history.lastUndoPoint, history.currentUndoPoint);
      
      if(history.snapshots.size() > UNDO_POINT_MAX) {
        history.snapshots.erase(history.snapshots.begin());
        history.firstUndoPoint++;
        //MLOGD("SEQ", "Move . firstUndoPoint %d.", history.firstUndoPoint);
      }
      
      history.tempSnapshot = SEQ_Snapshot();
    }

    void            RestoreSnapshot(int16_t undoPoint) {
        // 查找对应还原点的快照
        if(undoPoint < history.firstUndoPoint || undoPoint > history.lastUndoPoint)  return;
        
        auto it = std::find_if(history.snapshots.begin(), history.snapshots.end(),
            [undoPoint](const SEQ_Snapshot& snap) { return snap.undoPoint == undoPoint; });
        if (it == history.snapshots.end()) return;
        ClearClip(it->position.ChannelNum(), it->position.ClipNum());
        SEQ_Clip* clip = Clip(it->position.ChannelNum(), it->position.ClipNum());
        if(!clip) return;
        clip->CopySettings(it->clip);
        clip->checkActiveBar();

        // 3. 还原所有steps
        for (const auto& step : it->steps) {
            // 添加新的step，会自动分配新的ID
            SEQ_Step newStep = step.second;
            SEQ_Pos stepPos = newStep.Position();
            AddStep(newStep, stepPos);
        }

        // 4. 还原所有automs
        for (const auto& autom : it->automs) {
            // 添加新的autom，会自动分配新的ID
            SEQ_Autom newAutom = autom.second;
            SEQ_Pos automPos = newAutom.Position();
            AddAutom(newAutom, automPos);
        }

        history.tempSnapshot = SEQ_Snapshot();
        history.timestamp = 0;
        //MLOGD("SEQ", "Restorechannel %d. clip %d.", it->position.ChannelNum(), it->position.ClipNum());
        //MLOGD("SEQ", "lastUndoPoint %d. currentUndoPoint %d.", history.lastUndoPoint, history.currentUndoPoint);
    }
  
    void            RemoveSnapshotFrom(int16_t undoPoint) {
        if (history.snapshots.empty()) return;
        if(undoPoint > history.lastUndoPoint || undoPoint < history.firstUndoPoint) return;

        history.snapshots.erase(
            history.snapshots.begin() + (undoPoint - history.firstUndoPoint),
            history.snapshots.end()
        );
        history.lastUndoPoint = undoPoint - 1;
        history.currentUndoPoint = undoPoint;
    }
    
    bool            CanUndo() const { 
        return history.currentUndoPoint > history.firstUndoPoint; 
    }

    bool            CanRedo() const { 
        return history.currentUndoPoint < history.lastUndoPoint; 
    }

    void            Undo() {
        if (!CanUndo()) return;

        // 如果当前undo点大于lastUndo点，则需要创建Redo快照
        if(history.lastUndoPoint < history.currentUndoPoint){
            CreateTempSnapshot(history.snapshots.back().position, true);
            EnableTempSnapshot();
            history.currentUndoPoint--;
        }

        history.currentUndoPoint--;
        RestoreSnapshot(history.currentUndoPoint);
    }

    void            Redo() {
        if (!CanRedo()) return;

        history.currentUndoPoint++;
        RestoreSnapshot(history.currentUndoPoint);

        // 如果当前undo点等于lastUndo点，则需要删除Redo快照
        if(history.lastUndoPoint == history.currentUndoPoint){
            RemoveSnapshotFrom(history.lastUndoPoint);
        }
    }

    //-------------------------------------------------------------------------------//

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
      // MLOGD("SEQ Add " + type, "channel %d. clip %d. Bar %d. Step %d. ID %d.", 
      //       position.ChannelNum(), position.ClipNum(), position.BarNum(),position.Number(), ID);
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

      // string type = changedItems == stepChanged ? "Step" : "Autom";
      // MLOGD("SEQ Delete " + type, "channel %d. clip %d. Bar %d. Step %d. ID %d.", 
      //       position.ChannelNum(), position.ClipNum(), position.BarNum(),position.Number(), ID);
    }

  };
}
