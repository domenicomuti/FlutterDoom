//  Copyright 2021-2025 by Steve Clark

//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.

//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:

//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required. 
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.

#ifndef __MIDIPLAY_H__
#define __MIDIPLAY_H__

// file size of genmidi data
#define GENMIDI_SIZE    11908

// Midiplay_Init(samplerate, genmidi)
// samplerate   The samplerate of the audio output stream
// genmidi      GENMIDI instrument data
// Returns 0 on success, 1 on failure
int Midiplay_Init(int, char *);

// Midiplay_Load(data, size)
// data         Memory pointer to the music data
// size         Size of the music data in bytes
// Returns 0 on success, 1 if not Init'ed, 2 if unsupported or 3 on failure
int Midiplay_Load(void *, int);

// Midiplay_Play(playing)
// playing      0 = pause, 1 = play
void Midiplay_Play(int);

// Midiplay_Volume(volume)
// volume       Range 0 (minimum) to 127 (maximum)
void Midiplay_SetVolume(int);

// Midiplay_Output(sample)
// sample       Pointer to signed 32bit stereo sample
// Audio should be clamped by the calling function to signed 16bit
void Midiplay_Output(int [2]);

// Midiplay_IsPlaying
// Returns 0 if not playing
// Paused/looping music will still return 1
int Midiplay_IsPlaying(void);

// Midiplay_Time
// Returns time in seconds
// Calling this immediately after Midiplay_Load returns the length of the score
// Calling this while Midiplay_IsPlaying returns the current position
int Midiplay_Time(void);

// Midiplay_Loop(looping)
// Changes looping mode
// looping      Whether the music will repeat (0 = off, 1 = repeating)
void Midiplay_Loop(int);

// Midiplay_Restart
// Restarts score from beginning (and pauses playback)
void Midiplay_Restart(void);

#endif

// midiplay
