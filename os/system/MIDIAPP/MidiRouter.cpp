#include "MidiCenter.h"
#include "MidiAppUI.h"

#define ROUTER_SUFFIX "nod"
namespace MatrixOS::MidiCenter
{
  MidiAppUI* midiAppUI;
  MultiPad* multiPad;
  ChannelConfig* channelConfig;
  NotePadConfig* notePadConfig;
  MidiButtonConfig* drumPadConfig;
  string appName;
  
  std::map<RouterNode, Node*> nodesInChannel[16]; // 16 channel router node set
  std::map<RouterNode, uint8_t> nodesConfigNum[16]; // temp config number for each node

  uint16_t* nodesIndex; // RouterNode << 8 | config number , for router node save and load
  ChordConfig* chordConfig;
  ArpConfig* arpConfig;

  std::map<RouterNode, NodeInfo> nodesInfo = {
    {NODE_NONE,   {"",        COLOR_BLANK,    nullptr     }},
    {NODE_CHORD,  {"CHORD",   COLOR_YELLOW,   chordConfig }},
    {NODE_ARP,    {"ARP",     COLOR_ORANGE,   arpConfig   }},
  };
  
  std::list<SaveVarInfo> saveVarList = { // for var manager
    SaveVarInfo {(void**)&channelConfig,    sizeof(ChannelConfig),      1},
    SaveVarInfo {(void**)&notePadConfig,    sizeof(NotePadConfig),      4},
    SaveVarInfo {(void**)&drumPadConfig,    sizeof(MidiButtonConfig),   4 * 16},
    SaveVarInfo {(void**)&nodesIndex,       sizeof(uint16_t),           NODES_MAX_CONFIGS * NODES_PER_CHANNEL},
    SaveVarInfo {(void**)&chordConfig,      sizeof(ChordConfig),        NODES_MAX_CONFIGS},
    SaveVarInfo {(void**)&arpConfig,        sizeof(ArpConfig),          NODES_MAX_CONFIGS},
  };

  void MidiRouter(RouterNode from, uint8_t type, uint8_t channel, uint8_t byte1, uint8_t byte2)
  {
    if (type != SEND_NOTE)
    {
      if (byte2 != 0) Send_On(type, channel, byte1, byte2);
      else Send_Off(type, channel, byte1);
      return;
    }

    uint8_t order = (uint8_t)from | 0x0F;
    RouterNode to = NODE_MIDIOUT;
    
    auto it = nodesInChannel[channel].lower_bound((RouterNode)order);
    if (it != nodesInChannel[channel].end())
     to = (RouterNode)it->first;

    if (to == NODE_MIDIOUT)
      MatrixOS::MIDI::Send(MidiPacket(0, byte2 == 0 ? NoteOff : NoteOn, channel, byte1, byte2));
    else
      nodesInChannel[channel][to]->RouteIn(byte1, byte2);
  }

  void Send_On(int8_t type, int8_t channel, int8_t byte1, int8_t byte2)
  {
    switch(type) {
    case SEND_NONE: break; // None
    case SEND_CC: // CC
      MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, channel, byte1, byte2)); 
      break;
    case SEND_PC: // PC
      MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, channel, byte2, byte2)); 
      break;
    case SEND_NOTE: // Note
      byte2 = Device::KeyPad::GetVelocity();
      MatrixOS::MIDI::Send(MidiPacket(0, NoteOn, channel, byte1, byte2)); 
      break;
    default: break;
    }
  }

  void Send_Off(int8_t type, int8_t channel, int8_t byte1)
  {
    switch(type) {
    case SEND_CC: 
      MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, channel, byte1, 0)); 
      break;
    case SEND_NOTE:
      MatrixOS::MIDI::Send(MidiPacket(0, NoteOff, channel, byte1, 0));
      break;
    default: break;
    }
  }

  void NodeInsert(uint8_t channel, RouterNode node, uint8_t configNum)
  {
    if (channel > 15) return;
    if (configNum >= NODES_MAX_CONFIGS) configNum = NODES_MAX_CONFIGS - 1;
    if (nodesInChannel[channel].find(node) != nodesInChannel[channel].end()) return;

    switch(node)
    {
      case NODE_CHORD:
        nodesInChannel[channel].insert({NODE_CHORD, new Chord(channel, chordConfig, configNum)});
        break;
      case NODE_ARP:
        nodesInChannel[channel].insert({NODE_ARP, new Arpeggiator(channel, arpConfig, configNum)});
        break;
      default:
        break;
    }
  }

  void NodeDelete(uint8_t channel, RouterNode node)
  {
    auto it = nodesInChannel[channel].find(node);
    if(it != nodesInChannel[channel].end()) {
      delete it->second;
      nodesInChannel[channel].erase(it);
    }
  }

  bool RouterConfigInit(string name)
  {
    channelConfig = new ChannelConfig;
    for(uint8_t ch = 0; ch < 16; ch++){
      channelConfig->color[ch] = COLOR_LIME;
      channelConfig->padType[ch] = 1;
      channelConfig->bankLSB[ch] = 0;
      channelConfig->PC[ch] = 0;
      channelConfig->activeDrumNote[ch] = 36;
      for(uint8_t n = 0; n < 3; n++){
        channelConfig->activePadConfig[ch][n] = 0;
      }
    }
    channelConfig->padType[1]  = DRUM_PAD;
    channelConfig->padType[2]  = DRUM_PAD;
    channelConfig->color[0]  = COLOR_PURPLE;
    channelConfig->color[1]  = COLOR_ORANGE;
    channelConfig->color[2]  = COLOR_ORANGE;
    channelConfig->color[13] = COLOR_AZURE;
    channelConfig->color[14] = COLOR_AZURE;
    channelConfig->color[15] = COLOR_PURPLE;

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
    for(uint8_t ch = 0; ch < 16; ch++)
      for(uint8_t n = 0; n < NODES_PER_CHANNEL; n++)
        nodesIndex[ch * NODES_PER_CHANNEL + n] = NODE_NONE << 8 | 0;

    chordConfig = new ChordConfig[16];
    arpConfig   = new ArpConfig[16];

    bool ret =  MatrixOS::FATFS::ListSave(name, ROUTER_SUFFIX, saveVarList, true);
    delete   channelConfig;   delete[] notePadConfig;   delete[] drumPadConfig; 
    delete[] nodesIndex;      delete[] chordConfig;     delete[] arpConfig;
    return ret;
  }

  bool RequestService(string name, ChannelConfig*& CH_Config)
  {
    auto insertNodes = [&](){
      appName = name;
      for(uint8_t ch = 0; ch < 16; ch++) {
        for(uint8_t n = 0; n < NODES_PER_CHANNEL; n++) {
          uint16_t index = nodesIndex[ch * NODES_PER_CHANNEL + n];
          RouterNode node = RouterNode(index >> 8 & 0xFF);
          uint8_t num = index & 0xFF;
          if(node != NODE_NONE)
            NodeInsert(ch, node, num);
        }
      }
    };

    if (MatrixOS::FATFS::VarManager(name, ROUTER_SUFFIX, saveVarList) == false) {
      RouterConfigInit(name);
      if (MatrixOS::FATFS::VarManager(name, ROUTER_SUFFIX, saveVarList))
      { insertNodes(); return true; }
      else return false;
    } 
    midiAppUI = new MidiAppUI;
    midiAppUI->SetNode(NODE_NONE);
    multiPad = new MultiPad(Dimension(16, 2), 4, channelConfig, notePadConfig, drumPadConfig);
    MatrixOS::KnobCenter::SetColor(channelConfig->color);
    CH_Config = channelConfig;

    insertNodes();
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

    appName = "";
    delete midiAppUI;     delete multiPad;
    midiAppUI = nullptr;
    Color* color = nullptr;
    MatrixOS::KnobCenter::SetColor(color);
    MatrixOS::FATFS::VarManageEnd(ROUTER_SUFFIX);
  }

  void AddMidiAppTo(UI &ui, Point point) { if(midiAppUI != nullptr) ui.AddUIComponent(*midiAppUI, point); }
  void SetMidiAppNode(RouterNode node) { if(midiAppUI != nullptr) midiAppUI->SetNode(node); }

}

