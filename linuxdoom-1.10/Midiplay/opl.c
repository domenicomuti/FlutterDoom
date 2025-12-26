//  Nuked OPL3
//  Copyright (C) 2013-2020 Nuke.YKT

//  This file is part of Nuked OPL3.

//  Nuked OPL3 is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as
//  published by the Free Software Foundation, either version 2.1
//  of the License, or (at your option) any later version.

//  Nuked OPL3 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.

//  You should have received a copy of the GNU Lesser General Public License
//  along with Nuked OPL3. If not, see <https://www.gnu.org/licenses/>.

//   Nuked OPL3 emulator.
//   Thanks:
//       MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
//           Feedback and Rhythm part calculation information.
//       forums.submarine.org.uk(carbon14, opl3):
//           Tremolo and phase generator calculation information.
//       OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
//           OPL2 ROMs.
//       siliconpr0n.org(John McMaster, digshadow):
//           YMF262 and VRC VII decaps and die shots.

//  version: 1.8

// -----------------------------------------------------------------------------
//  This is a modified version of Nuked OPL3.
//  The original API functions have been replaced to give a more direct way to
//    access the synthesizer allowing for more control of its capabilities.

//  Changes:
//       Combined functions for operator settings
//       Post processed voice volume
//       Full stereo panning
//       Increased note range
//       Capable of 128 voices! (maybe more)
//       LUT optimisations
//       2-op mode with no percussion modes

//  2023-2025    Steve Clark (fawtytoo)

#include "opl.h"

#define ENV_ATTACK      0
#define ENV_DECAY       1
#define ENV_SUSTAIN     2
#define ENV_RELEASE     3

typedef s16 (*WAVE)(u16, u16 *);

typedef struct
{
    s16     out;
    s16     fbmod;
    s16     *mod;
    u8      *trem;
    s16     *vibrato;
    u32     pg_phase;
    u16     pg_phase_out;
    s16     prout;
    u16     eg_rout;
    u16     eg_out;
    int     eg_gen;
    int     eg_reset;
    u8      eg_inc;         // unused
    u8      eg_rate;        // unused
    u8      eg_ksl;
    u8      reg_ksr;        // key scale rate
    u8      reg_mult;       // frequency multiplier
    u8      reg_ksl;        // key scale level
    u8      reg_tl;         // total level (volume)
    u8      reg_sl;         // sustain level
    WAVE    Wave;           // wave function
    u8      reg_rate[4];    // attack, decay, sustain & release rates
}
OP;

typedef struct
{
    OP      op[2];
    s16     *out[2];
    u16     f_num;          // 11 bit value which doubles the frequency range
    u16     block;          //  ... with the same 8 blocks
    u8      fb;             // feedback
    u8      ksv;            // key scale value
    u16     volume;
    u16     left, right;
}
VOICE;

static VOICE    oplVoice[NVOICES];
static u16      oplClock = 0;
static u32      oplEgTimer = 0;
static u8       oplEgState = 0;
static u8       oplEgAdd = 0;
static s16      oplVibrato = 0;
static u8       oplVibratoShift = 0;    // vibrato depth
static u8       oplTremolo = 0;
static u8       oplTremoloShift = 0;    // tremolo depth
static s16      oplZeroS16 = 0;
static u8       oplZeroU8 = 0;

static const u32    panTable[2][128] =
{
    {
        128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113,
        112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100,  99,  98,  97,
         96,  95,  94,  93,  92,  91,  90,  89,  87,  86,  85,  84,  83,  82,  81,  80,
         79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  69,  68,  67,  66,  65,  64,
         63,  62,  61,  60,  59,  58,  57,  56,  55,  54,  53,  52,  51,  50,  49,  48,
         47,  46,  45,  44,  43,  42,  41,  40,  38,  37,  36,  35,  34,  33,  32,  31,
         30,  29,  28,  27,  26,  25,  24,  23,  22,  21,  20,  19,  18,  17,  16,  15,
         14,  13,  12,  11,  10,   9,   8,   7,   6,   5,   4,   3,   2,   1,   0
    },
    {
          0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
         16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
         32,  33,  34,  35,  36,  37,  38,  39,  41,  42,  43,  44,  45,  46,  47,  48,
         49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,
         65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,
         81,  82,  83,  84,  85,  86,  87,  88,  90,  91,  92,  93,  94,  95,  96,  97,
         98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
        114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128
    }
};

static const u16    freqTable[128][3] = // block, f_num, diff
{
    {0,  172,  10}, {0,  182,  11}, {0,  193,  12}, {0,  205,  12}, {0,  217,  13}, {0,  230,  13}, {0,  243,  15}, {0,  258,  15},
    {0,  273,  17}, {0,  290,  17}, {0,  307,  18}, {0,  325,  19}, {0,  344,  21}, {0,  365,  22}, {0,  387,  23}, {0,  410,  24},
    {0,  434,  26}, {0,  460,  27}, {0,  487,  29}, {0,  516,  31}, {0,  547,  33}, {0,  580,  34}, {0,  614,  37}, {0,  651,  38},
    {0,  689,  41}, {0,  730,  44}, {0,  774,  46}, {0,  820,  49}, {0,  869,  51}, {0,  920,  55}, {0,  975,  58}, {0, 1033,  61},
    {0, 1094,  66}, {0, 1160,  69}, {0, 1229,  73}, {0, 1302,  77}, {0, 1379,  82}, {0, 1461,  87}, {0, 1548,  92}, {0, 1640,  98},
    {0, 1738, 103}, {0, 1841, 109}, {0, 1950, 107}, {1, 1033,  61}, {1, 1094,  66}, {1, 1160,  69}, {1, 1229,  73}, {1, 1302,  77},
    {1, 1379,  82}, {1, 1461,  87}, {1, 1548,  92}, {1, 1640,  98}, {1, 1738, 103}, {1, 1841, 109}, {1, 1950, 107}, {2, 1033,  61},
    {2, 1094,  66}, {2, 1160,  69}, {2, 1229,  73}, {2, 1302,  77}, {2, 1379,  82}, {2, 1461,  87}, {2, 1548,  92}, {2, 1640,  98},
    {2, 1738, 103}, {2, 1841, 109}, {2, 1950, 107}, {3, 1033,  61}, {3, 1094,  66}, {3, 1160,  69}, {3, 1229,  73}, {3, 1302,  77},
    {3, 1379,  82}, {3, 1461,  87}, {3, 1548,  92}, {3, 1640,  98}, {3, 1738, 103}, {3, 1841, 109}, {3, 1950, 107}, {4, 1033,  61},
    {4, 1094,  66}, {4, 1160,  69}, {4, 1229,  73}, {4, 1302,  77}, {4, 1379,  82}, {4, 1461,  87}, {4, 1548,  92}, {4, 1640,  98},
    {4, 1738, 103}, {4, 1841, 109}, {4, 1950, 107}, {5, 1033,  61}, {5, 1094,  66}, {5, 1160,  69}, {5, 1229,  73}, {5, 1302,  77},
    {5, 1379,  82}, {5, 1461,  87}, {5, 1548,  92}, {5, 1640,  98}, {5, 1738, 103}, {5, 1841, 109}, {5, 1950, 107}, {6, 1033,  61},
    {6, 1094,  66}, {6, 1160,  69}, {6, 1229,  73}, {6, 1302,  77}, {6, 1379,  82}, {6, 1461,  87}, {6, 1548,  92}, {6, 1640,  98},
    {6, 1738, 103}, {6, 1841, 109}, {6, 1950, 107}, {7, 1033,  61}, {7, 1094,  66}, {7, 1160,  69}, {7, 1229,  73}, {7, 1302,  77},
    {7, 1379,  82}, {7, 1461,  87}, {7, 1548,  92}, {7, 1640,  98}, {7, 1738, 103}, {7, 1841, 109}, {7, 1950, 116}, {7, 2066, 123}
};

static const u8     tremoloTable[2][210] =
{
    {
         0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
         2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
         3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,
         5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,
         6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
         5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
         3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,
         2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
         1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    },
    {
         0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,  4,  5,
         5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10,
        10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15,
        15, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20,
        21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25, 26,
        26, 26, 25, 25, 25, 25, 24, 24, 24, 24, 23, 23, 23, 23, 22, 22, 22, 22, 21, 21, 21,
        21, 20, 20, 20, 20, 19, 19, 19, 19, 18, 18, 18, 18, 17, 17, 17, 17, 16, 16, 16, 16,
        15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11, 10,
        10, 10, 10,  9,  9,  9,  9,  8,  8,  8,  8,  7,  7,  7,  7,  6,  6,  6,  6,  5,  5,
         5,  5,  4,  4,  4,  4,  3,  3,  3,  3,  2,  2,  2,  2,  1,  1,  1,  1,  0,  0,  0
    }
};

static const s16    vibratoTable[2][8][8] =
{
    {
        {0, 0, 0, 0, 0,  0,  0,  0}, {0, 0, 0, 0, 0,  0,  0,  0},
        {0, 0, 1, 0, 0,  0, -1,  0}, {0, 0, 1, 0, 0,  0, -1,  0},
        {0, 1, 2, 1, 0, -1, -2, -1}, {0, 1, 2, 1, 0, -1, -2, -1},
        {0, 1, 3, 1, 0, -1, -3, -1}, {0, 1, 3, 1, 0, -1, -3, -1}
    },
    {
        {0, 0, 0, 0, 0,  0,  0,  0}, {0, 0, 1, 0, 0,  0, -1,  0},
        {0, 1, 2, 1, 0, -1, -2, -1}, {0, 1, 3, 1, 0, -1, -3, -1},
        {0, 2, 4, 2, 0, -2, -4, -2}, {0, 2, 5, 2, 0, -2, -5, -2},
        {0, 3, 6, 3, 0, -3, -6, -3}, {0, 3, 7, 3, 0, -3, -7, -3}
    }
};

// ½ sine wave
static const u16    sineTable[512] =
{
    0x859, 0x6c3, 0x607, 0x58b, 0x52e, 0x4e4, 0x4a6, 0x471, 0x443, 0x41a, 0x3f5, 0x3d3, 0x3b5, 0x398, 0x37e, 0x365,
    0x34e, 0x339, 0x324, 0x311, 0x2ff, 0x2ed, 0x2dc, 0x2cd, 0x2bd, 0x2af, 0x2a0, 0x293, 0x286, 0x279, 0x26d, 0x261,
    0x256, 0x24b, 0x240, 0x236, 0x22c, 0x222, 0x218, 0x20f, 0x206, 0x1fd, 0x1f5, 0x1ec, 0x1e4, 0x1dc, 0x1d4, 0x1cd,
    0x1c5, 0x1be, 0x1b7, 0x1b0, 0x1a9, 0x1a2, 0x19b, 0x195, 0x18f, 0x188, 0x182, 0x17c, 0x177, 0x171, 0x16b, 0x166,
    0x160, 0x15b, 0x155, 0x150, 0x14b, 0x146, 0x141, 0x13c, 0x137, 0x133, 0x12e, 0x129, 0x125, 0x121, 0x11c, 0x118,
    0x114, 0x10f, 0x10b, 0x107, 0x103, 0x0ff, 0x0fb, 0x0f8, 0x0f4, 0x0f0, 0x0ec, 0x0e9, 0x0e5, 0x0e2, 0x0de, 0x0db,
    0x0d7, 0x0d4, 0x0d1, 0x0cd, 0x0ca, 0x0c7, 0x0c4, 0x0c1, 0x0be, 0x0bb, 0x0b8, 0x0b5, 0x0b2, 0x0af, 0x0ac, 0x0a9,
    0x0a7, 0x0a4, 0x0a1, 0x09f, 0x09c, 0x099, 0x097, 0x094, 0x092, 0x08f, 0x08d, 0x08a, 0x088, 0x086, 0x083, 0x081,
    0x07f, 0x07d, 0x07a, 0x078, 0x076, 0x074, 0x072, 0x070, 0x06e, 0x06c, 0x06a, 0x068, 0x066, 0x064, 0x062, 0x060,
    0x05e, 0x05c, 0x05b, 0x059, 0x057, 0x055, 0x053, 0x052, 0x050, 0x04e, 0x04d, 0x04b, 0x04a, 0x048, 0x046, 0x045,
    0x043, 0x042, 0x040, 0x03f, 0x03e, 0x03c, 0x03b, 0x039, 0x038, 0x037, 0x035, 0x034, 0x033, 0x031, 0x030, 0x02f,
    0x02e, 0x02d, 0x02b, 0x02a, 0x029, 0x028, 0x027, 0x026, 0x025, 0x024, 0x023, 0x022, 0x021, 0x020, 0x01f, 0x01e,
    0x01d, 0x01c, 0x01b, 0x01a, 0x019, 0x018, 0x017, 0x017, 0x016, 0x015, 0x014, 0x014, 0x013, 0x012, 0x011, 0x011,
    0x010, 0x00f, 0x00f, 0x00e, 0x00d, 0x00d, 0x00c, 0x00c, 0x00b, 0x00a, 0x00a, 0x009, 0x009, 0x008, 0x008, 0x007,
    0x007, 0x007, 0x006, 0x006, 0x005, 0x005, 0x005, 0x004, 0x004, 0x004, 0x003, 0x003, 0x003, 0x002, 0x002, 0x002,
    0x002, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x002,
    0x002, 0x002, 0x002, 0x003, 0x003, 0x003, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x006, 0x006, 0x007, 0x007,
    0x007, 0x008, 0x008, 0x009, 0x009, 0x00a, 0x00a, 0x00b, 0x00c, 0x00c, 0x00d, 0x00d, 0x00e, 0x00f, 0x00f, 0x010,
    0x011, 0x011, 0x012, 0x013, 0x014, 0x014, 0x015, 0x016, 0x017, 0x017, 0x018, 0x019, 0x01a, 0x01b, 0x01c, 0x01d,
    0x01e, 0x01f, 0x020, 0x021, 0x022, 0x023, 0x024, 0x025, 0x026, 0x027, 0x028, 0x029, 0x02a, 0x02b, 0x02d, 0x02e,
    0x02f, 0x030, 0x031, 0x033, 0x034, 0x035, 0x037, 0x038, 0x039, 0x03b, 0x03c, 0x03e, 0x03f, 0x040, 0x042, 0x043,
    0x045, 0x046, 0x048, 0x04a, 0x04b, 0x04d, 0x04e, 0x050, 0x052, 0x053, 0x055, 0x057, 0x059, 0x05b, 0x05c, 0x05e,
    0x060, 0x062, 0x064, 0x066, 0x068, 0x06a, 0x06c, 0x06e, 0x070, 0x072, 0x074, 0x076, 0x078, 0x07a, 0x07d, 0x07f,
    0x081, 0x083, 0x086, 0x088, 0x08a, 0x08d, 0x08f, 0x092, 0x094, 0x097, 0x099, 0x09c, 0x09f, 0x0a1, 0x0a4, 0x0a7,
    0x0a9, 0x0ac, 0x0af, 0x0b2, 0x0b5, 0x0b8, 0x0bb, 0x0be, 0x0c1, 0x0c4, 0x0c7, 0x0ca, 0x0cd, 0x0d1, 0x0d4, 0x0d7,
    0x0db, 0x0de, 0x0e2, 0x0e5, 0x0e9, 0x0ec, 0x0f0, 0x0f4, 0x0f8, 0x0fb, 0x0ff, 0x103, 0x107, 0x10b, 0x10f, 0x114,
    0x118, 0x11c, 0x121, 0x125, 0x129, 0x12e, 0x133, 0x137, 0x13c, 0x141, 0x146, 0x14b, 0x150, 0x155, 0x15b, 0x160,
    0x166, 0x16b, 0x171, 0x177, 0x17c, 0x182, 0x188, 0x18f, 0x195, 0x19b, 0x1a2, 0x1a9, 0x1b0, 0x1b7, 0x1be, 0x1c5,
    0x1cd, 0x1d4, 0x1dc, 0x1e4, 0x1ec, 0x1f5, 0x1fd, 0x206, 0x20f, 0x218, 0x222, 0x22c, 0x236, 0x240, 0x24b, 0x256,
    0x261, 0x26d, 0x279, 0x286, 0x293, 0x2a0, 0x2af, 0x2bd, 0x2cd, 0x2dc, 0x2ed, 0x2ff, 0x311, 0x324, 0x339, 0x34e,
    0x365, 0x37e, 0x398, 0x3b5, 0x3d3, 0x3f5, 0x41a, 0x443, 0x471, 0x4a6, 0x4e4, 0x52e, 0x58b, 0x607, 0x6c3, 0x859
};

// exp table multiplied by 2
static const u16    expTable[256] =
{
    0xff4, 0xfea, 0xfde, 0xfd4, 0xfc8, 0xfbe, 0xfb4, 0xfa8, 0xf9e, 0xf92, 0xf88, 0xf7e, 0xf72, 0xf68, 0xf5c, 0xf52,
    0xf48, 0xf3e, 0xf32, 0xf28, 0xf1e, 0xf14, 0xf08, 0xefe, 0xef4, 0xeea, 0xee0, 0xed4, 0xeca, 0xec0, 0xeb6, 0xeac,
    0xea2, 0xe98, 0xe8e, 0xe84, 0xe7a, 0xe70, 0xe66, 0xe5c, 0xe52, 0xe48, 0xe3e, 0xe34, 0xe2a, 0xe20, 0xe16, 0xe0c,
    0xe04, 0xdfa, 0xdf0, 0xde6, 0xddc, 0xdd2, 0xdca, 0xdc0, 0xdb6, 0xdac, 0xda4, 0xd9a, 0xd90, 0xd88, 0xd7e, 0xd74,
    0xd6a, 0xd62, 0xd58, 0xd50, 0xd46, 0xd3c, 0xd34, 0xd2a, 0xd22, 0xd18, 0xd10, 0xd06, 0xcfe, 0xcf4, 0xcec, 0xce2,
    0xcda, 0xcd0, 0xcc8, 0xcbe, 0xcb6, 0xcae, 0xca4, 0xc9c, 0xc92, 0xc8a, 0xc82, 0xc78, 0xc70, 0xc68, 0xc60, 0xc56,
    0xc4e, 0xc46, 0xc3c, 0xc34, 0xc2c, 0xc24, 0xc1c, 0xc12, 0xc0a, 0xc02, 0xbfa, 0xbf2, 0xbea, 0xbe0, 0xbd8, 0xbd0,
    0xbc8, 0xbc0, 0xbb8, 0xbb0, 0xba8, 0xba0, 0xb98, 0xb90, 0xb88, 0xb80, 0xb78, 0xb70, 0xb68, 0xb60, 0xb58, 0xb50,
    0xb48, 0xb40, 0xb38, 0xb32, 0xb2a, 0xb22, 0xb1a, 0xb12, 0xb0a, 0xb02, 0xafc, 0xaf4, 0xaec, 0xae4, 0xade, 0xad6,
    0xace, 0xac6, 0xac0, 0xab8, 0xab0, 0xaa8, 0xaa2, 0xa9a, 0xa92, 0xa8c, 0xa84, 0xa7c, 0xa76, 0xa6e, 0xa68, 0xa60,
    0xa58, 0xa52, 0xa4a, 0xa44, 0xa3c, 0xa36, 0xa2e, 0xa28, 0xa20, 0xa18, 0xa12, 0xa0c, 0xa04, 0x9fe, 0x9f6, 0x9f0,
    0x9e8, 0x9e2, 0x9da, 0x9d4, 0x9ce, 0x9c6, 0x9c0, 0x9b8, 0x9b2, 0x9ac, 0x9a4, 0x99e, 0x998, 0x990, 0x98a, 0x984,
    0x97c, 0x976, 0x970, 0x96a, 0x962, 0x95c, 0x956, 0x950, 0x948, 0x942, 0x93c, 0x936, 0x930, 0x928, 0x922, 0x91c,
    0x916, 0x910, 0x90a, 0x904, 0x8fc, 0x8f6, 0x8f0, 0x8ea, 0x8e4, 0x8de, 0x8d8, 0x8d2, 0x8cc, 0x8c6, 0x8c0, 0x8ba,
    0x8b4, 0x8ae, 0x8a8, 0x8a2, 0x89c, 0x896, 0x890, 0x88a, 0x884, 0x87e, 0x878, 0x872, 0x86c, 0x866, 0x860, 0x85a,
    0x854, 0x850, 0x84a, 0x844, 0x83e, 0x838, 0x832, 0x82c, 0x828, 0x822, 0x81c, 0x816, 0x810, 0x80c, 0x806, 0x800
};

// freq mult table multiplied by 2       ½, 1, 2, 3, 4,  5,  6,  7,  8,  9, 10, 10, 12, 12, 15, 15
static const u8     multiplyTable[16] = {1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30};

// kslrom[16] = {0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64}
// (kslrom[f_num >> 6] << 2) - ((8 - block) << 5)
static const u8     kslTable[8][16] =
{
    {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0},
    {0,   0,   0,   0,   0,   0,   0,   0,   0,   8,  12,  16,  20,  24,  28,  32},
    {0,   0,   0,   0,   0,  12,  20,  28,  32,  40,  44,  48,  52,  56,  60,  64},
    {0,   0,   0,  20,  32,  44,  52,  60,  64,  72,  76,  80,  84,  88,  92,  96},
    {0,   0,  32,  52,  64,  76,  84,  92,  96, 104, 108, 112, 116, 120, 124, 128},
    {0,  32,  64,  84,  96, 108, 116, 124, 128, 136, 140, 144, 148, 152, 156, 160},
    {0,  64,  96, 116, 128, 140, 148, 156, 160, 168, 172, 176, 180, 184, 188, 192},
    {0,  96, 128, 148, 160, 172, 180, 188, 192, 200, 204, 208, 212, 216, 220, 224}
};

static const u8     kslShiftTable[4] = {8, 1, 2, 0};

// envelope generator constants
static const u8     egIncStepTable[4][4][4] =
{
    {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 0, 1, 0}, {1, 1, 1, 0}},
    {{1, 1, 1, 1}, {2, 1, 1, 1}, {2, 1, 2, 1}, {2, 2, 2, 1}},
    {{2, 2, 2, 2}, {3, 2, 2, 2}, {3, 2, 3, 2}, {3, 3, 3, 2}},
    {{3, 3, 3, 3}, {3, 3, 3, 3}, {3, 3, 3, 3}, {3, 3, 3, 3}}
};

static const u8     egKeyScaleRateTable[16][16] =
{
    { 0,  4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60},
    { 1,  5,  9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61},
    { 2,  6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62},
    { 3,  7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63},
    { 4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 60},
    { 5,  9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 61},
    { 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 62},
    { 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63, 63},
    { 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 60, 60},
    { 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 61, 61},
    {10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 62, 62},
    {11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63, 63, 63},
    {12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 60, 60, 60},
    {13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61, 61, 61, 61},
    {14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62, 62, 62, 62},
    {15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63, 63, 63, 63}
};

static void Op_Feedback(OP *op, u8 fb)
{
    op->fbmod = (op->prout + op->out) >> fb;
    op->prout = op->out;
}

static void Op_Envelope(OP *op, u8 ksv)
{
    u8      reg_rate, rate, rate_hi, rate_lo;
    u8      eg_shift, shift;
    u16     eg_rout;
    s16     eg_inc;
    u8      eg_off;
    int     reset = op->eg_reset;

    op->eg_out = op->eg_rout + op->reg_tl + (op->eg_ksl >> op->reg_ksl) + *op->trem;

    if (op->eg_reset)
    {
        op->eg_reset = 0;
        op->eg_gen = ENV_ATTACK;
        op->eg_rout = 0x1ff;
    }

    reg_rate = op->reg_rate[op->eg_gen];

    rate = egKeyScaleRateTable[ksv >> op->reg_ksr][reg_rate];
    rate_hi = rate >> 2;
    rate_lo = rate & 0x03;
    eg_shift = rate_hi + oplEgAdd;
    shift = 0;
    if (reg_rate != 0)
    {
        if (rate_hi < 12)
        {
            if (oplEgState)
            {
                switch (eg_shift)
                {
                  case 12:
                    shift = 1;
                    break;

                  case 13:
                    shift = (rate_lo >> 1) & 0x01;
                    break;

                  case 14:
                    shift = rate_lo & 0x01;
                    break;
                }
            }
        }
        else
        {
            shift = egIncStepTable[rate_hi & 3][rate_lo][oplClock & 3];
            if (shift == 0)
            {
                shift = oplEgState;
            }
        }
    }
    eg_rout = op->eg_rout;
    eg_inc = 0;
    eg_off = 0;
    if (reset && rate_hi == 0x0f) // Instant attack
    {
        eg_rout = 0;
    }
    else if ((op->eg_rout & 0x1f8) == 0x1f8)
    {
        eg_off = 1;
        if (!reset && op->eg_gen != ENV_ATTACK)
        {
            eg_rout = 0x1ff;
        }
    }
    switch (op->eg_gen)
    {
      case ENV_ATTACK:
        if (op->eg_rout == 0)
        {
            op->eg_gen = ENV_DECAY;
        }
        else if (shift > 0 && rate_hi != 0x0f)
        {
            eg_inc = ~op->eg_rout >> (4 - shift);
        }
        break;

      case ENV_DECAY:
        if ((op->eg_rout >> 4) == op->reg_sl)
        {
            op->eg_gen = ENV_SUSTAIN;
            break;
        }
        // FALLTHRU
      case ENV_SUSTAIN:
        // FALLTHRU
      case ENV_RELEASE:
        if (!reset && !eg_off && shift > 0)
        {
            eg_inc = 1 << (shift - 1);
        }
        break;
    }

    op->eg_rout = (eg_rout + eg_inc) & 0x1ff;
}

static void Op_PhaseGenerate(OP *op, u16 block, u16 f_num)
{
    f_num += *op->vibrato;
    op->pg_phase_out = op->pg_phase >> 9;
    op->pg_phase += ((f_num << block) * op->reg_mult >> 1);
}

// wave generate ---------------------------------------------------------------
static s16 Op_Wave1(u16 phase, u16 *neg)
{
    if (phase & 0x200)
    {
        *neg = 0xffff;
    }

    return sineTable[phase & 0x1ff];
}

static s16 Op_Wave2(u16 phase, u16 *neg)
{
    if (phase & 0x200)
    {
        return 0x1000;
    }

    return sineTable[phase & 0x1ff];
}

static s16 Op_Wave3(u16 phase, u16 *neg)
{
    return sineTable[phase & 0x1ff];
}

static s16 Op_Wave4(u16 phase, u16 *neg)
{
    if (phase & 0x100)
    {
        return 0x1000;
    }

    return sineTable[phase & 0xff];
}

static s16 Op_Wave5(u16 phase, u16 *neg)
{
    if (phase & 0x200)
    {
        return 0x1000;
    }

    if (phase & 0x100)
    {
        *neg = 0xffff;
    }

    return sineTable[(phase << 1) & 0x1ff];
}

static s16 Op_Wave6(u16 phase, u16 *neg)
{
    if (phase & 0x200)
    {
        return 0x1000;
    }

    return sineTable[(phase << 1) & 0x1ff];
}

static s16 Op_Wave7(u16 phase, u16 *neg)
{
    if (phase & 0x200)
    {
        *neg = 0xffff;
    }

    return 0;
}

static s16 Op_Wave8(u16 phase, u16 *neg)
{
    if (phase & 0x200)
    {
        *neg = 0xffff;
        phase = (phase & 0x1ff) ^ 0x1ff;
    }

    return phase << 3;
}

static void Op_Generate(OP *op)
{
    u16     neg = 0;
    u32     level;

    level = op->Wave((op->pg_phase_out + *op->mod) & 0x3ff, &neg) + (op->eg_out << 3);
    if (level > 0x1fff)
    {
        level = 0x1fff;
    }

    op->out = (expTable[level & 0xff] >> (level >> 8)) ^ neg;
}

// opl -------------------------------------------------------------------------
void OPL_VoiceOff(int index)
{
    VOICE   *voice = &oplVoice[index];

    voice->op[0].eg_gen = ENV_RELEASE;
    voice->op[1].eg_gen = ENV_RELEASE;
}

void OPL_VoiceOn(int index)
{
    VOICE   *voice = &oplVoice[index];

    voice->op[0].pg_phase = 0;
    voice->op[1].pg_phase = 0;
    voice->op[0].eg_reset = 1;
    voice->op[1].eg_reset = 1;
}

void OPL_Volume(int index, u16 volume)
{
    oplVoice[index].volume = volume;
}

void OPL_Frequency(int index, int pitch)
{
    VOICE   *voice = &oplVoice[index];
    u16     block, f_num;
    int     key;

    if (pitch < 0)
    {
        pitch = 0;
    }
    else if (pitch > 8191)
    {
        pitch = 8191;
    }

    key = pitch / 64;

    block = freqTable[key][0];
    f_num = freqTable[key][1] + (pitch & 63) * freqTable[key][2] / 64;
    if (f_num > 2047)
    {
        if (block < 7)
        {
            block++;
            f_num >>= 1;
        }
        else
        {
            f_num = 2047;
        }
    }

    voice->block = block;
    voice->f_num = f_num;
    voice->ksv = (block << 1) | ((f_num >> 9) & 1);

    voice->op[0].eg_ksl = kslTable[block][(f_num >> 6) & 15];
    voice->op[1].eg_ksl = kslTable[block][(f_num >> 6) & 15];
}

void OPL_Pan(int index, int pan)
{
    VOICE   *voice = &oplVoice[index];

    if (pan == 0)
    {
        pan = 1;
    }
    pan--;

    voice->left = panTable[0][pan];
    voice->right = panTable[1][pan];
}

void OPL_Feedback(int index, u8 data)
{
    VOICE   *voice = &oplVoice[index];
    u8      fb = (data >> 1) & 7;

    voice->fb = 9 - fb;

    voice->op[0].mod = fb == 0 ? &oplZeroS16 : &voice->op[0].fbmod;

    if (data & 0x01)    // AM
    {
        voice->op[1].mod = &oplZeroS16;
        voice->out[0] = &voice->op[0].out;
        voice->out[1] = &voice->op[1].out;
    }
    else                // FM
    {
        voice->op[1].mod = &voice->op[0].out;
        voice->out[0] = &voice->op[1].out;
        voice->out[1] = &voice->op[1].out /*&oplZeroS16*/; // experimental
    }
}

void OPL_Op(int index, int operator, u8 data[6])
{
    OP      *op = &oplVoice[index].op[operator];

    op->trem = (data[0] >> 7) & 0x01 ? &oplTremolo : &oplZeroU8;
    op->vibrato = (data[0] >> 6) & 0x01 ? &oplVibrato : &oplZeroS16;
    op->reg_ksr = (((data[0] >> 4) & 0x01) ^ 1) << 1;
    op->reg_mult = multiplyTable[data[0] & 0x0f];

    op->reg_rate[ENV_ATTACK] = data[1] >> 4;
    op->reg_rate[ENV_DECAY] = data[1] & 0x0f;

    op->reg_sl = data[2] >> 4;
    if (op->reg_sl == 0x0f)
    {
        op->reg_sl = 0x1f;
    }

    op->reg_rate[ENV_SUSTAIN] = (data[0] >> 5) & 0x01 ? 0 : data[2] & 0x0f;
    op->reg_rate[ENV_RELEASE] = data[2] & 0x0f;

    switch (data[3] & 0x07)
    {
      case 0:
        op->Wave = Op_Wave1;
        break;

      case 1:
        op->Wave = Op_Wave2;
        break;

      case 2:
        op->Wave = Op_Wave3;
        break;

      case 3:
        op->Wave = Op_Wave4;
        break;

      case 4:
        op->Wave = Op_Wave5;
        break;

      case 5:
        op->Wave = Op_Wave6;
        break;

      case 6:
        op->Wave = Op_Wave7;
        break;

      case 7:
        op->Wave = Op_Wave8;
        break;
    }

    op->reg_ksl = kslShiftTable[data[4] >> 6];
    op->reg_tl = (data[5] & 0x3f) << 2;
}

void OPL_Generate(s32 sample[2])
{
    VOICE   *voice = &oplVoice[0];
    s32     acc;
    int     i;
    int     vibrato_pos = (oplClock >> 10) & 7;

    oplTremolo = tremoloTable[oplTremoloShift][(oplClock >> 6) % 210];

    oplEgTimer += oplEgState;

    if (oplEgState)
    {
        if (oplEgTimer & 1)
        {
            oplEgAdd = 1;
        }
        else if (oplEgTimer & 2)
        {
            oplEgAdd = 2;
        }
        else if (oplEgTimer & 4)
        {
            oplEgAdd = 3;
        }
        else if (oplEgTimer & 8)
        {
            oplEgAdd = 4;
        }
        else if (oplEgTimer & 16)
        {
            oplEgAdd = 5;
        }
        else if (oplEgTimer & 32)
        {
            oplEgAdd = 6;
        }
        else if (oplEgTimer & 64)
        {
            oplEgAdd = 7;
        }
        else if (oplEgTimer & 128)
        {
            oplEgAdd = 8;
        }
        else if (oplEgTimer & 256)
        {
            oplEgAdd = 9;
        }
        else if (oplEgTimer & 512)
        {
            oplEgAdd = 10;
        }
        else if (oplEgTimer & 1024)
        {
            oplEgAdd = 11;
        }
        else if (oplEgTimer & 2048)
        {
            oplEgAdd = 12;
        }
        else if (oplEgTimer & 4096)
        {
            oplEgAdd = 13;
        }
        else
        {
            oplEgAdd = 0;
        }
    }

    for (i = 0; i < NVOICES; i++, voice++)
    {
        Op_Feedback(&voice->op[0], voice->fb);
        Op_Feedback(&voice->op[1], voice->fb);
        Op_Envelope(&voice->op[0], voice->ksv);
        Op_Envelope(&voice->op[1], voice->ksv);
        oplVibrato = vibratoTable[oplVibratoShift][(voice->f_num >> 7) & 7][vibrato_pos];
        Op_PhaseGenerate(&voice->op[0], voice->block, voice->f_num);
        Op_PhaseGenerate(&voice->op[1], voice->block, voice->f_num);
        Op_Generate(&voice->op[0]);
        Op_Generate(&voice->op[1]);

        acc = (*voice->out[0] + *voice->out[1]) * voice->volume / 256;

        sample[0] += (acc * voice->left / 128);
        sample[1] += (acc * voice->right / 128);
    }

    oplClock++;

    oplEgState ^= 1;
}

void OPL_VibratoDepth(u8 data)
{
    oplVibratoShift = data & 1;
}

void OPL_TremoloDepth(u8 data)
{
    oplTremoloShift = data & 1;
}

void OPL_Reset()
{
    VOICE   *voice = &oplVoice[0];
    int     i;

    for (i = 0; i < NVOICES; i++, voice++)
    {
        voice->out[0] = &oplZeroS16;
        voice->out[1] = &oplZeroS16;
        voice->op[0].mod = &oplZeroS16;
        voice->op[1].mod = &oplZeroS16;
        voice->op[0].trem = &oplZeroU8;
        voice->op[1].trem = &oplZeroU8;
        voice->op[0].vibrato = &oplZeroS16;
        voice->op[1].vibrato = &oplZeroS16;
        voice->op[0].eg_rout = 0x1ff;
        voice->op[1].eg_rout = 0x1ff;
        voice->op[0].Wave = Op_Wave1;
        voice->op[1].Wave = Op_Wave1;
    }
}
