#include "MidiCenter.h"
#include <bitset>

namespace MatrixOS::MidiCenter
{
  std::map<uint8_t, Node*> chords; // channel, Chord
  std::set<uint16_t> CNTR_Chord; // midiID

  enum NoteNum : uint8_t { N_root, N_2ed, N_3rd, N_4th, N_5th, N_6th, N_7th};
  
  enum Interval : uint8_t 
  {
    I_m2 = 1,   I_M2 = 2,   I_A2 = 3,
    I_m3 = 3,   I_M3 = 4,
    I_d4 = 4,   I_P4 = 5,   I_A4 = 6,
    I_d5 = 6,   I_P5 = 7,   I_A5 = 8,
    I_m6 = 8,   I_M6 = 9,
    I_d7 = 9,   I_m7 = 10,  I_M7 = 11,
  };

  void Chorder::Scan()
  {
    GetCurrentScale();

    if(inputList.empty()) 
    {
      if(inputEmpty) return;

      auto it = tempList.begin(); 
      if (it != tempList.end()) {
        lastTemp = it->first;
        inputList.insert(*it);
        it = tempList.erase(it);
      }

      if (inputList.empty())
      {
        inputEmpty = true;
        inputListPrv.clear();
        for (uint8_t i = 0; i < 4 ; i++) { currentVoice[i] = -1; }
        releaseTime = MatrixOS::SYS::Millis();
        ClearChord(channel); 
        return;
      }
    } 
    else inputEmpty = false;

    if(inputList.size() > 1) {
      for(auto it = inputList.begin(); it != inputList.end() && inputList.size() > 1;) {
        if(it->first != lastRoot) {
          MidiRouter(NODE_CHORD, SEND_NOTE, channel, it->first, it->second.velocity);
          tempList.insert(*it);
          it = inputList.erase(it);
        } else it++;
      }
    }

    if(inputListPrv.empty() || inputList.begin()->first != inputListPrv.begin()->first)
    {
      createTime = MatrixOS::SYS::Millis();
      lastRoot = inputList.begin()->first;
      if(lastTemp == lastRoot) 
      {
        MidiRouter(NODE_CHORD, SEND_NOTE, channel, lastTemp, 0);
        lastTemp = 255;
      }
      CreateChord();
      inputListPrv = inputList;
    }
  }

  void Chorder::OutListNoteOff(uint8_t note)
  {
    if(tempList.find(note) != tempList.end()) tempList.erase(note);
    MidiRouter(thisNode, SEND_NOTE, channel, note, 0);
  }

  void Chorder::SetActiveConfig(uint8_t num)
  {
    if (num >= NODES_MAX_CONFIGS) num = NODES_MAX_CONFIGS - 1;
    activeConfig = config + num;
    nodesConfigNum[channel].insert_or_assign(NODE_CHORD, num);
  }

  void Chorder::CreateChord()
  {
    uint8_t voice[7] = {inputList.begin()->first, 0, 0, 0, 0, 0, 0};
    std::vector<uint8_t> createChord;
    keyScale = ((scale >> voice[N_root] % 12) | (scale << (12 - voice[N_root] % 12))) & 0x0FFF;
    sus2 = false ; sus4 = false;
    
    // CheckInput(voice);
    CheckVoices(voice);

    // MLOGD("Chord","keyScale = %d, root: %d, 3rd: %d, 5th: %d, 7th: %d", keyScale, voice[N_root],voice[N_3rd],voice[N_5th],voice[N_7th]);

    uint32_t voicing = Voicing(voice);
    // MLOGD("Chord","voicing = %d", voicing);
    for (uint8_t i = 0; i < 20; i++){
      if (voicing >> i & 1)
      {
        uint8_t note = (i % 4 ? voice[i % 4 * 2] : 0) + voice[N_root] + i / 4 * 12 - 24;
        if (note <= 127) createChord.push_back(note);
      }
    }

    SendChord(createChord, voice);
  }

  void Chorder::CheckInput(uint8_t* voice)
  { 
    // if(inputList.size() < 2) return;

    // std::set<uint8_t> inputToInterval;
    // for (auto it = inputList.begin()++; it != inputList.end(); it++)
    // {
    //   inputToInterval.insert(it->first - voice[N_root]);
    // }
    

    // bool A4_mark = false;
    // bool d5_mark = false;
    // bool M6_mark = false;
    // bool d7_mark = false;
    // for (auto it = inputToInterval.begin(); it != inputToInterval.end(); it++)
    // {
    //   switch (*it)
    //   {
    //     case 0: break;
    //     case 1: // m2
    //       voice[N_2ed] = *it % 12; 
    //       sus2 = true; break;
    //     case 2: // M2
    //       voice[N_2ed] = voice[N_2ed] == I_m2 ? 0 : *it % 12; 
    //       sus2 = true; break;
    //     case 3: // m3
    //       voice[N_3rd] = *it % 12; break; 
    //     case 4: // M3
    //       voice[N_3rd] = voice[N_3rd] == I_m3 ? 0 : *it % 12; break;
    //     case 5: // P4
    //       voice[N_4th] = *it % 12; 
    //       sus4 = true; break;
    //     case 6: // d5, A4
    //       A4_mark = voice[N_4th] != I_P4;
    //       d5_mark = true; break;
    //     case 7: // P5
    //       voice[N_5th] = *it % 12;
    //       d5_mark = false; break;
    //     case 8: // A5, m6
    //       if (voice[N_5th] == I_P5) voice[N_6th] = *it % 12;
    //       else
    //       { voice[N_5th] = I_A5; d5_mark = false; } break; 
    //     case 9: // d7, M6
    //       M6_mark = voice[N_6th] != I_m6;
    //       d7_mark = true; break; 
    //     case 10: // m7
    //       voice[N_7th] = *it % 12;
    //       d7_mark = false; break; 
    //     case 11: // M7
    //       voice[N_7th] = voice[N_7th] == I_m7 ? 0 : *it % 12;
    //       d7_mark = false; break;
    //   }
    // }

    // if(d5_mark) 
    // { 
    //   if(keyScale >> I_m3 & 1) voice[N_3rd] = I_m3; 
    //   voice[N_5th] = I_d5; 
    // }
    // if(A4_mark & !d5_mark) { voice[N_4th] = I_A4; sus4 = true; }
    // if(d7_mark) 
    // {
    //   if((chordCheck[TRI_Diminished] >> voice[N_root] % 12) & 1)
    //     { voice[N_3rd] = I_m3; voice[N_5th] = I_d5; voice[N_7th] = I_d7; }
    //   else
    //     d7_mark = false;
    // }
    // if(M6_mark & !d7_mark) 
    //   voice[N_6th] = I_M6;
    
  }  

  void Chorder::CheckVoices(uint8_t* voice)
  {
    srand(MatrixOS::SYS::Millis());
    std::bitset<12> check = keyScale;
    auto Check   = [&](uint8_t i)->bool { 
      bool random = activeConfig->harmony != 100 ? rand() % 101 > activeConfig->harmony : false;
      bool ret = check[i] || random;
      return i ? ret : false; 
    };
    auto Rand    = [&](bool harmony)->bool { return harmony ? rand() % 101 > activeConfig->harmony : rand() % 2;};
    auto Choose  = [&](uint8_t i1, uint8_t i2, uint8_t n, bool harmony = false) { 
      uint8_t check = Check(i1) | (Check(i2) << 1);
      switch (check) {
        case 0b00: break;
        case 0b01: voice[n] = i1; break;
        case 0b10: voice[n] = i2; break;
        case 0b11: voice[n] = Rand(harmony) ? i1 : i2; break;
      }
    };

    // check 2ed
    if(!voice[N_2ed]) {
      uint8_t check_A2 = Check(I_M3) ? I_A2 : 0;  // if 3rd can't be M3, 2ed con't be A2
      Choose (check_A2, I_m2, N_2ed, true);       // if random by harmony is true, choose I_m2
      Choose (voice[N_2ed], I_M2, N_2ed);         // random choose m2/A2 or M2                           
    }   

    // check 3ed
    if(!voice[N_3rd]) {
      uint8_t check_m3 = voice[N_2ed] == I_A2 ? 0 : I_m3; // check if 2ed is A2, 3rd can't be m3
      switch (voice[N_2ed]) 
      { 
        case I_m2: Choose (I_M3, check_m3, N_3rd, true); break;
        case I_M2: Choose (check_m3, I_M3, N_3rd, true); break;
        default: Choose (I_M3, check_m3, N_3rd);
      }
    }

    // check 4th
    if(!voice[N_4th]) {
      uint8_t check_d4 = voice[N_3rd] == I_M3 ? 0 : I_d4;
      uint8_t check_A4 = (Check(I_P5) | Check(I_A5)) ? I_A4 : 0;
      Choose (check_A4, check_d4, N_4th, true);
      Choose (voice[N_4th], I_P4, N_4th, true);
    }

    // check 5th
    if(!voice[N_5th]) {
      uint8_t check_d5 = voice[N_3rd] == I_M3 || voice[N_4th] == I_A4 ? 0 : I_d5;                    
      Choose (I_A5, check_d5, N_5th, true);
      Choose (voice[N_5th], I_P5, N_5th, true);           
    }                          

    // check 6th
    if(!voice[N_6th]) {
      uint8_t check_m6 = voice[N_5th] == I_A5 ? 0 : I_m6;
      uint8_t check_M6 = (Check(I_M7) | Check(I_m7)) ? I_M6 : 0;
      switch (voice[N_3rd]) 
      { 
        case I_m3: Choose (check_M6, check_m6, N_6th, true); break;
        case I_M3: Choose (check_m6, check_M6, N_6th, true); break;
        default: Choose (check_m6, check_M6, N_6th);
      }
      Choose(check_m6, check_M6, N_6th); 
    }  

    // check 7th
    if(!voice[N_7th]) {
      uint8_t check_d7 = (voice[N_6th] == I_M6) ? 0 : I_d7;
      switch(voice[N_3rd])
      {
        case I_m3:
          Choose(I_M7, check_d7, N_7th, true);
          Choose(voice[N_7th], I_m7, N_7th, true); break;
        case I_M3:
          Choose(check_d7, I_m7, N_7th, true);
          Choose(voice[N_7th], I_M7, N_7th, true); break;
        default:
        {
          Choose(I_m7, I_M7, N_7th);
          bool inharmony = rand() % 101 > activeConfig->harmony && rand() % 101 > activeConfig->harmony;
          if(inharmony && check_d7) voice[N_7th] = I_d7;
        }
      }
    }                                                           
  }

  const uint8_t voicing_3rd_h[9][3] = {    // third 7th+, 5th+, 3rd+, 1st+, 7th, 5th, 3rd, 1st
  // 3notes      4notes      5notes
    {0b01100001, 0b01110001, 0b01110101},  // high5 low1 normal
    {0b00100101, 0b00110101, 0b00110111},  // high3 low1
    {0b00010101, 0b00010111, 0b00010111},  // high1 low1
    {0b01010010, 0b01110010, 0b01110110},  // high5 low3
    {0b00100110, 0b00110110, 0b00110110},  // high3 low3
    {0b00010110, 0b00010110, 0b00010110},  // high1 low3
    {0b01010100, 0b01110100, 0b01110100},  // high5 low5
    {0b00110100, 0b00110100, 0b00110100},  // high3 low5
    {0b00010110, 0b00010110, 0b00010110},  // error
  };
  
  const uint8_t voicing_7th_h[16][3] = {   // seventh
  // 3notes      4notes      5notes
    {0b10100001, 0b10100101, 0b10110101},  // high7 low1 normal
    {0b01100001, 0b01101001, 0b01111001},  // high5 low1
    {0b00101001, 0b00111001, 0b00111101},  // high3 low1
    {0b00010101, 0b00011011, 0b00011111},  // high1 low1
    {0b10010010, 0b10110010, 0b11110010},  // high7 low3
    {0b01010010, 0b01011010, 0b01111010},  // high5 low3
    {0b00101010, 0b00111010, 0b00111110},  // high3 low3
    {0b00011010, 0b00011110, 0b00011110},  // high1 low3
    {0b10010100, 0b10110100, 0b11110100},  // high7 low5
    {0b01010100, 0b01011100, 0b01111100},  // high5 low5
    {0b00110100, 0b00111100, 0b00111100},  // high3 low5
    {0b00010101, 0b00011011, 0b00011111},  // error
    {0b10011000, 0b10111000, 0b11111000},  // high7 low7
    {0b01011000, 0b01111000, 0b01111000},  // high5 low7
    {0b00110100, 0b00111100, 0b00111101},  // error
    {0b00011010, 0b00011110, 0b00011111},  // error
  };
  
  const uint8_t voicing_3rd_l[3][4] = {    // third
  // 1O-1N,      1O-2N,      2O-2N       2Octave-3Notes
    {0b00000001, 0b00000101, 0b00010001, 0b00010101}, // normal
    {0b00000010, 0b00000110, 0b00010010, 0b00010110}, // drop2
    {0b00000100, 0b00000100, 0b00010100, 0b01010100}, // drop3
  };
  
  const uint8_t voicing_7th_l[5][4] = {    // seventh
    {0b00000001, 0b00001001, 0b00001001, 0b00101001}, // normal
    {0b00000010, 0b00001010, 0b00010010, 0b00101010}, // drop2
    {0b00000100, 0b00001100, 0b00010100, 0b01010100}, // drop3
    {0b00001000, 0b00001100, 0b00011000, 0b10011000}, // drop4
  }; 

  uint32_t Chorder::Voicing(uint8_t* voice)
  {
    uint8_t range = 0, dropNow = 0, v_l = 0, v_h = 0;
    auto Index_h = [&]()->uint8_t {
      uint8_t num = 3 + config->seventh;
      int8_t autoBass = -1, autoTreble = -1, bassIndex = 0, trebleIndex = 0;
      if (config ->autoVoicing && (releaseTime + tickInterval * 24) > MatrixOS::SYS::Millis()){
        std::map<uint8_t, uint8_t> bass; 
        std::map<uint8_t, uint8_t> treble;
        for(uint8_t i = 0; i < num; i++) {
          uint8_t note = (i == 0 ? 0 : voice[i * 2]) + voice[N_root];
          uint8_t thisBass = note - (2 - config->bassRange) * 12;
          uint8_t thisTreble = note + (config->trebleRange - 2) * 12;
          bass.insert_or_assign(std::abs(thisBass - lastBass), i);
          treble.insert_or_assign(std::abs(thisTreble - lastTreble), i);
        }
        if (rand() % 2) bass.erase(bass.begin()); 
        if (rand() % 2) treble.erase(treble.begin());
        if (treble.begin()->second != 0 && bass.begin()->second > treble.begin()->second - 1) 
          bass.begin()->second = treble.begin()->second - 1;
        autoBass = bass.begin()->second * num; 
        autoTreble = num - 1 - treble.begin()->second;
      }
      if (range < 2)
        {if (config->randomDrop) bassIndex = autoBass == -1 ? dropNow * num : autoBass;}
      else 
        bassIndex = dropNow * num;
      if(config->randomTreble) trebleIndex = autoTreble == -1 ? rand() % (num - (autoBass == 0 ? 0 : autoBass - 1)) : autoTreble;
      return bassIndex + trebleIndex;
    };

    auto Index_l = [&]() -> uint8_t {
      int8_t autoBass = -1;
      if (config->autoVoicing  && (releaseTime + tickInterval * 24) > MatrixOS::SYS::Millis()) 
      {
        std::map<uint8_t, uint8_t> bass;
        for(uint8_t i = 0; i < 3 + config->seventh; i++) 
        {
          uint8_t note = (i == 0 ? 0 : voice[i * 2]) + voice[N_root];
          uint8_t thisBass = note - (2 - config->bassRange) * 12;
          bass.insert_or_assign(std::abs(thisBass - lastBass), i);
        }
        autoBass = bass.begin()->second;
      }
      return (autoBass == -1 ? dropNow : autoBass);
    };

    srand(MatrixOS::SYS::Millis());

    if(drop) dropNow = drop;
    else if(config->randomDrop) dropNow = rand() % (3 + config->seventh);
    else dropNow = 0;

    uint32_t voiceBits = 0;  // {++7, ++5, ++3, ++1},{+7, +5, +3, +1},{7, 5, 3, 1},{-7, -5, -3, -1},{--7, --5, --3, --1}
    uint8_t bassRange = (config->bassRange == 2 && dropNow) ? 1 : config->bassRange;
    uint8_t trebleRange = config->trebleRange;
    range = trebleRange - bassRange;
    const uint8_t (*voicing_h)[3] = config->seventh ? voicing_7th_h : voicing_3rd_h;
    const uint8_t (*voicing_l)[4] = config->seventh ? voicing_7th_l : voicing_3rd_l;

    switch(range) {
      case 0: // maxVoices = 2 - 4
        if (config->maxVoices == 2) {voiceBits = 0b0101 << 8; break;}
        else if (config->maxVoices == 3) { voiceBits = (config->seventh ? 0b1011 : 0b0111) << 8; }
        else { voiceBits = 0b1111 << 8; } 
        break;
      case 1: // maxVoices = 3 - 5
        v_h = voicing_h[Index_h()][config->maxVoices - 3];
        voiceBits = v_h << bassRange * 4; 
        break;
      case 2: // maxVoices = 4 - 7
        v_h = voicing_h[Index_h()][config->maxVoices == 7 ? 2 : config->maxVoices - 4];
        v_l = voicing_l[Index_l()][config->maxVoices == 7 ? 1 : 0];
        voiceBits = (v_h << (trebleRange - 1) * 4) | (v_l << bassRange * 4); 
        break;
      case 3: case 4: // maxVoices = 5 - 8
        v_h = voicing_h[Index_h()][config->maxVoices == 8 ? 2 : config->maxVoices - 5];
        v_l = voicing_l[Index_l()][config->maxVoices == 8 ? 3 : 2]; 
        voiceBits = (v_h << (trebleRange - 1) * 4) | (v_l << bassRange * 4); 
        break;
    };
    return voiceBits;
  }

  void Chorder::SendChord(std::vector<uint8_t>& createChord, uint8_t* voice)
  {
    uint8_t velocity = inputList.begin()->second.velocity;
    lastBass = 127;  lastTreble = 0;
    ClearChord(channel);

    for (auto it = createChord.begin(); it != createChord.end(); it++)
    {
      if (*it > 127)
        continue;
      lastBass = *it < lastBass ? *it : lastBass;
      lastTreble = *it > lastTreble ? *it : lastTreble;
      uint16_t midiID = MidiID(SEND_NOTE, channel, *it);
      MidiRouter(NODE_CHORD, SEND_NOTE, channel, *it, velocity);
      CNTR_Chord.insert(midiID);
    }

  }

  void Chorder::GetCurrentScale()
  {
    if (channel != MatrixOS::UserVar::global_channel) return;
    if (channelConfig == nullptr || notePadConfig == nullptr) { scale = 0x0FFF; return; }

    uint8_t padType = channelConfig->padType[channel];
    uint8_t padConfig = channelConfig->activePadConfig[channel][padType != NOTE_PAD ? PIANO_PAD : NOTE_PAD];
    scale = notePadConfig[padConfig].scale;

    if(scale != scalePrv)
    {
      scalePrv = scale;

      if(scale == 0x0FFF) // if scale == CHOMATIC，return all chords are in scale
      {
        for(uint8_t type = 0; type < 12; type++)
          chordCheck[type] = 0x0FFF;
        return;
      }
      
      for(uint8_t type = 0; type < 12; type++) // check all chord type
      {  
        chordCheck[type] = 0;
        for(uint8_t root = 0; root < 12; root++) // check on each root note of the scale
        {
          uint16_t tempScale = ((scale >> root) | (scale << (12 - root))) & 0x0FFF;
          bool check = true;
          for(uint8_t v = 0; v < 12 && check; v++) // check if each voice is in the scale
          {
            if(chordType[type] >> v & 1)
              check &= tempScale >> v & 1;
          }
          if (check)
            chordCheck[type] |= 1 << root;
        }
      }
    }
  }

}