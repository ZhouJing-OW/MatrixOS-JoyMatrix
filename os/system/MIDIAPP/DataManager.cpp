#include "MidiCenter.h"
#include "MidiAppUI.h"

#define ROUTER_SUFFIX "nod"
namespace MatrixOS::MidiCenter
{
  string appName;
  MidiAppUI* midiAppUI;
  ClipSelector* clipSelector;
  MultiPad* multiPad;
  ProjectConfig* projectConfig;
  ChannelConfig* channelConfig;
  NotePadConfig* notePadConfig;
  MidiButtonConfig* drumPadConfig;
  SEQ_DataStore* seqData;

  uint16_t* nodesIndex; // NodeID << 8 | config number , for router node save and load
  extern ChorderConfig*    chordConfig;
  extern ArpConfig*        arpConfig;
  
  std::list<SaveVarInfo> saveVarList = { // for var manager
    SaveVarInfo {(void**)&projectConfig,    sizeof(ProjectConfig),      1},
    SaveVarInfo {(void**)&channelConfig,    sizeof(ChannelConfig),      1},
    SaveVarInfo {(void**)&notePadConfig,    sizeof(NotePadConfig),      4},
    SaveVarInfo {(void**)&drumPadConfig,    sizeof(MidiButtonConfig),   4 * 16},
    SaveVarInfo {(void**)&nodesIndex,       sizeof(uint16_t),           NODES_MAX_CONFIGS * NODES_PER_CHANNEL},
    SaveVarInfo {(void**)&chordConfig,      sizeof(ChorderConfig),      NODES_MAX_CONFIGS},
    SaveVarInfo {(void**)&arpConfig,        sizeof(ArpConfig),          NODES_MAX_CONFIGS},
  };

  bool ConfigInit(string name)
  {
    projectConfig = new ProjectConfig;
    channelConfig = new ChannelConfig;
    for(uint8_t ch = 0; ch < 16; ch++){
      channelConfig->color[ch] = Color(GREEN);
      channelConfig->padType[ch] = 1;
      channelConfig->bankLSB[ch] = 0;
      channelConfig->PC[ch] = 0;
      channelConfig->activeNote[ch] = 36;
      for(uint8_t n = 0; n < 3; n++){
        channelConfig->activePadConfig[ch][n] = 0;
      }
    }
    channelConfig->padType[1]  = DRUM_PAD;
    channelConfig->padType[2]  = DRUM_PAD;
    channelConfig->color[0]  = Color(PURPLE);
    channelConfig->color[1]  = Color(ORANGE);
    channelConfig->color[2]  = Color(ORANGE);
    channelConfig->color[13] = Color(CYAN);
    channelConfig->color[14] = Color(CYAN);
    channelConfig->color[15] = Color(PURPLE);

    notePadConfig = new NotePadConfig[4];
    notePadConfig[0] = {.overlap = 4, .scale = MAJOR};
    notePadConfig[1] = {.overlap = 4, .scale = MAJOR};
    notePadConfig[2] = {.overlap = 4, .scale = MAJOR};
    notePadConfig[3] = {.overlap = 4, .scale = MAJOR};

    drumPadConfig = new MidiButtonConfig[4 * 16];
    for (uint8_t i = 0; i < 4; i++)
    {
      for (uint8_t n = 0; n < 16; n++)
      {
        drumPadConfig[i * 16 + n].byte1 = n + 36;
        drumPadConfig[i * 16 + n].byte1 = n + 36;
        drumPadConfig[i * 16 + n].byte1 = n + 36;
        drumPadConfig[i * 16 + n].channel = 9;
        if(n % 12 == 0)
          drumPadConfig[i * 16 + n].color = COLOR_DRUM_PAD[1];
        else
          drumPadConfig[i * 16 + n].color = COLOR_DRUM_PAD[0];
        drumPadConfig[i * 16 + n].type = SEND_NOTE;
        drumPadConfig[i * 16 + n].globalChannel = true;
      }
    }

    nodesIndex = new uint16_t[16 * NODES_PER_CHANNEL];
    for(uint8_t ch = 0; ch < 16; ch++) {
      nodesIndex[ch * NODES_PER_CHANNEL] = NODE_SEQ << 8 | 0;
      for(uint8_t n = 1; n < NODES_PER_CHANNEL; n++) {
        nodesIndex[ch * NODES_PER_CHANNEL + n] = NODE_NONE << 8 | 0;
      }
      
    }

    chordConfig = new ChorderConfig[16];
    arpConfig   = new ArpConfig[16];

    bool ret =  MatrixOS::FATFS::ListSave(name, ROUTER_SUFFIX, saveVarList, true);
    delete   projectConfig;   delete   channelConfig;   delete[] notePadConfig;   delete[] drumPadConfig; 
    projectConfig = nullptr;  channelConfig = nullptr;  notePadConfig = nullptr;  drumPadConfig = nullptr;
    delete[] nodesIndex;      delete[] chordConfig;     delete[] arpConfig;
    nodesIndex = nullptr;     chordConfig = nullptr;     arpConfig = nullptr;
    return ret;
  }

  bool RequestService(string name, ChannelConfig*& CH_Config)
  {
    auto ServiceStart = [&](){
      midiAppUI = new MidiAppUI;
      clipSelector = new ClipSelector;
      multiPad = new MultiPad(Dimension(16, 2), 4, channelConfig, notePadConfig, drumPadConfig);
      MatrixOS::KnobCenter::SetColor(channelConfig->color);
      CH_Config = channelConfig;
      seqData = (SEQ_DataStore*)heap_caps_malloc(sizeof(SEQ_DataStore), MALLOC_CAP_SPIRAM);
      if(seqData) seqData->Init();
      
      bpm.SetPtr(&projectConfig->bpm);
      swing.SetPtr(&projectConfig->swing);
      defaultVelocity.SetValue(MatrixOS::UserVar::defaultVelocity);
      brightness.SetValue(std::sqrt((uint8_t)MatrixOS::UserVar::brightness));

      appName = name;
      for(uint8_t ch = 0; ch < 16; ch++) {
        for(uint8_t n = 0; n < NODES_PER_CHANNEL; n++) {
          uint16_t index = nodesIndex[ch * NODES_PER_CHANNEL + n];
          NodeID nodeID = NodeID(index >> 8 & 0xFF);
          uint8_t num = index & 0xFF;
          if(nodeID != NODE_NONE)
            NodeInsert(ch, nodeID, num);
        }
      }

      midiAppUI->SetUI(NODE_SEQ);
    };

    if (MatrixOS::FATFS::VarManager(name, ROUTER_SUFFIX, saveVarList) == false) {
      ConfigInit(name);
      if (MatrixOS::FATFS::VarManager(name, ROUTER_SUFFIX, saveVarList))
      { ServiceStart(); return true; }
      else return false;
    } 

    ServiceStart();
    return true;
  }

  void EndService() {
    for(uint8_t i = 0; i < 16; i++) {
      for(auto it : nodesInChannel[i]) {
        delete it.second;
      }
        nodesInChannel[i].clear();
        nodesConfigNum[i].clear();
    }

    bpm.SetPtr(&bpmPrv);
    swing.SetPtr(&swingPrv);
    appName = "";
    delete midiAppUI;     delete clipSelector;     delete multiPad;      seqData->Destroy(); heap_caps_free(seqData);
    midiAppUI = nullptr;  clipSelector = nullptr;  multiPad = nullptr;   seqData = nullptr;
    Color* color = nullptr;
    MatrixOS::KnobCenter::SetColor(color);
    MatrixOS::FATFS::VarManageEnd(ROUTER_SUFFIX);
  }

  void AddMidiAppTo(UI &ui, Point point) { if(midiAppUI != nullptr) ui.AddUIComponent(*midiAppUI, point); }
  void AddClipSelectorTo(UI &ui, Point point) { if(clipSelector != nullptr) ui.AddUIComponent(*clipSelector, point); }
  void SetMidiAppNode(NodeID nodeID) { if(midiAppUI != nullptr) midiAppUI->SetUI(nodeID); }

}

