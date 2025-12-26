/* Nuked OPL3
 * Copyright (C) 2013-2020 Nuke.YKT
 *
 * This file is part of Nuked OPL3.
 *
 * Nuked OPL3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * Nuked OPL3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Nuked OPL3. If not, see <https://www.gnu.org/licenses/>.

 *  Nuked OPL3 emulator.
 *  Thanks:
 *      MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
 *          Feedback and Rhythm part calculation information.
 *      forums.submarine.org.uk(carbon14, opl3):
 *          Tremolo and phase generator calculation information.
 *      OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
 *          OPL2 ROMs.
 *      siliconpr0n.org(John McMaster, digshadow):
 *          YMF262 and VRC VII decaps and die shots.
 *
 * version: 1.8
 */

/* This is a modified version of Nuked OPL3.
   2023-2024    Steve Clark (fawtytoo)                                        */

#ifndef __OPL_H__
#define __OPL_H__

#define NVOICES     128

typedef unsigned int    u32;
typedef signed int      s32;
typedef unsigned short  u16;
typedef signed short    s16;
typedef unsigned char   u8;
typedef signed char     s8;

void OPL_Reset(void);
void OPL_Generate(s32 [2]);
void OPL_TremoloDepth(u8);
void OPL_VibratoDepth(u8);
void OPL_Op(int, int, u8 [6]);
void OPL_Feedback(int, u8);
void OPL_Pan(int, int);
void OPL_Frequency(int, int);
void OPL_Volume(int, u16);
void OPL_VoiceOn(int);
void OPL_VoiceOff(int);

#endif
