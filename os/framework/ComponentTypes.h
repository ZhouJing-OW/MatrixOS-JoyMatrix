#pragma once

enum SendType : int8_t{
    SEND_NONE   = 0,
    SEND_CC     = 1,
    SEND_PC     = 2,
    SEND_NOTE   = 3,
};

enum PadType : int8_t{
    NOTE_PAD    = 0,
    PIANO_PAD   = 1,
    DRUM_PAD    = 2,
};
