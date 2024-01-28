#pragma once

enum RouterNode : uint8_t {
  NODE_NONE         = 0x00,
  NODE_INPUT        = 0x01,
  NODE_KEYPAD       = 0x02,

  NODE_CHORDSEQ     = 0x20,
  NODE_CHANCE777    = 0x21,
  NODE_M1SHA        = 0x22,

  NODE_ROUTIN       = 0x30,

  NODE_NOTESEQ      = 0x51,
  NODE_DRUMSEQ      = 0x52,
  NODE_EUCLIDEAN    = 0x53,
  NODE_ARPEGGIO     = 0x54,

  NODE_CHORD        = 0x80,
  NODE_ARP          = 0x90,
  NODE_HUMANIZER    = 0xA0,
  NODE_ROUTOUT      = 0xE0,
  NODE_MIDIOUT      = 0xFF,
};