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

#include "opl.h"

#include "midiplay.h"

#define LE16(i)     ((i)[0] | ((i)[1] << 8))
#define LE32(i)     (LE16(i) | (LE16(i + 2) << 16))
#define BE16(i)     (((i)[0] << 8) | (i)[1])
#define BE32(i)     ((BE16(i) << 16) | BE16(i + 2))

const u32       volumeTable[128] =
{
    0x000, 0x001, 0x002, 0x004, 0x005, 0x007, 0x008, 0x009, 0x00b, 0x00c, 0x00e, 0x00f, 0x011, 0x012, 0x014, 0x015,
    0x017, 0x018, 0x01a, 0x01b, 0x01d, 0x01f, 0x020, 0x022, 0x023, 0x025, 0x027, 0x028, 0x02a, 0x02b, 0x02d, 0x02f,
    0x030, 0x032, 0x034, 0x035, 0x037, 0x039, 0x03b, 0x03c, 0x03e, 0x040, 0x041, 0x043, 0x045, 0x047, 0x049, 0x04a,
    0x04c, 0x04e, 0x050, 0x052, 0x054, 0x055, 0x057, 0x059, 0x05b, 0x05d, 0x05f, 0x061, 0x063, 0x065, 0x067, 0x069,
    0x06b, 0x06d, 0x06f, 0x071, 0x073, 0x075, 0x077, 0x079, 0x07b, 0x07d, 0x07f, 0x081, 0x083, 0x085, 0x087, 0x089,
    0x08c, 0x08e, 0x090, 0x092, 0x094, 0x097, 0x099, 0x09b, 0x09d, 0x0a0, 0x0a2, 0x0a4, 0x0a6, 0x0a9, 0x0ab, 0x0ad,
    0x0b0, 0x0b2, 0x0b5, 0x0b7, 0x0b9, 0x0bc, 0x0be, 0x0c1, 0x0c3, 0x0c6, 0x0c8, 0x0cb, 0x0cd, 0x0d0, 0x0d2, 0x0d5,
    0x0d7, 0x0da, 0x0dc, 0x0df, 0x0e2, 0x0e4, 0x0e7, 0x0ea, 0x0ec, 0x0ef, 0x0f2, 0x0f4, 0x0f7, 0x0fa, 0x0fd, 0x100
};

// genmidi ---------------------------------------------------------------------
#define GM_FLAG_FIXED           0x0001
#define GM_FLAG_2VOICE          0x0004

typedef struct
{
    u8      mod[6];
    u8      feedback;
    u8      car[6];
    u8      unused;
    s16     baseNoteOffset;
}
GMVOICE;

typedef struct
{
    u16     flags;
    s8      fineTuning;
    u8      fixedNote;
    GMVOICE voice[2];
}
GENMIDI;

static GENMIDI      *gmInstr;

// timer -----------------------------------------------------------------------
typedef struct
{
    int     rate;
    int     acc;
    int     remainder;
    int     divisor;
}
TIMER;

static TIMER    timerPhase, timerSample, timerTick;

// event -----------------------------------------------------------------------
#define NOTE_OFF        0
#define NOTE_PLAY       1

#define CC_01   1   // mod wheel
#define CC_06   6   // data entry coarse
#define CC_07   7   // volume
#define CC_0a   10  // pan
#define CC_0b   11  // expression
#define CC_40   64  // sustain
#define CC_64   100 // reg lsb
#define CC_65   101 // reg msb
#define CC_78   120 // all sounds off
#define CC_79   121 // reset controllers
#define CC_7b   123 // all notes off
#define CC_80   128 // MUS instrument change
#define CC_FF   255

typedef struct _voice   VOICE;
struct _voice
{
    VOICE   *prev, *next;
    int     index;
    int     note, pitch;
    int     volume;
    int     playing;
};

typedef struct
{
    VOICE   voice;
    GENMIDI *instrument;
    int     voiceCount;
    int     tuning[2];
    int     volume, expression;
    int     prevVolume; // MUS only
    int     pan;
    int     bend, bendRange;
    int     sustain;
    int     rpn;
}
CHANNEL;

static CHANNEL  midChannel[16] =
{
    {.voice = {.prev = &midChannel[0].voice, .next = &midChannel[0].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[1].voice, .next = &midChannel[1].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[2].voice, .next = &midChannel[2].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[3].voice, .next = &midChannel[3].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[4].voice, .next = &midChannel[4].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[5].voice, .next = &midChannel[5].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[6].voice, .next = &midChannel[6].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[7].voice, .next = &midChannel[7].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[8].voice, .next = &midChannel[8].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[9].voice, .next = &midChannel[9].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[10].voice, .next = &midChannel[10].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[11].voice, .next = &midChannel[11].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[12].voice, .next = &midChannel[12].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[13].voice, .next = &midChannel[13].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[14].voice, .next = &midChannel[14].voice, .volume = 257}, .tuning[0] = 0},
    {.voice = {.prev = &midChannel[15].voice, .next = &midChannel[15].voice, .volume = 257}, .tuning[0] = 0}
};
static VOICE    midVoice[NVOICES];
static VOICE    voiceList = {.prev = &voiceList, .next = &voiceList, .index = -1};

static int      percChannel = 9;
static int      voiceCount = 0;

// control ---------------------------------------------------------------------
#define MICROSEC    1000000

typedef void (*EVENT)(void);
static void (*AddEvent)(EVENT);
static u32 (*GetDelta)(void);

typedef struct
{
    u8      *track, *pos;
    u32     clock;
    EVENT   Event;
    int     channel;
    u8      data[3];
    u8      running;
    int     done;
}
TRACK;

static TRACK        midTrack[65536], *curTrack, *endTrack;
static int          numTracks, numTracksEnded;

static int          midType2;

static int          beatTicks = 96, beatTempo = MICROSEC / 2;
static int          playSamples;
static u32          musicClock;

static int          timeTicks, timeRate;

static EVENT        MusicEvents;

// controller map for MUS
static const int    controllerMap[16] = // CMD_TYPE would suggest only 16
{
    CC_80, CC_FF, CC_01, CC_07, CC_0a, CC_0b, CC_FF, CC_FF,
    CC_40, CC_FF, CC_78, CC_7b, CC_FF, CC_FF, CC_79, CC_FF
};

// midiplay --------------------------------------------------------------------
#define MUS_HDRSIZE     16
#define MID_HDRSIZE     14
#define HMP_HDRSIZE     64

static int      musicInit = 0;
static int      musicLooping;
static int      musicPlaying = 0;
static int      musicVolume = 0x100;

// misc ------------------------------------------------------------------------
static int ID(void *ptr, char *check)
{
    char *id = (char *)ptr;

    do
    {
        if (*id++ != *check++)
        {
            return 0;
        }
    }
    while (*check);

    return 1;
}

// timer -----------------------------------------------------------------------
static int Timer_Update(TIMER *timer)
{
    timer->acc += timer->remainder;
    if (timer->acc < timer->divisor)
    {
        return timer->rate;
    }

    timer->acc -= timer->divisor;

    return timer->rate + 1;
}

static void Timer_Set(TIMER *timer, int numerator, int divisor)
{
    timer->acc = 0;
    timer->rate = numerator / divisor;
    timer->remainder = numerator - timer->rate * divisor;
    timer->divisor = divisor;
}

// event -----------------------------------------------------------------------
static void Voice_ToList(VOICE *list, VOICE *voice)
{
    voice->next = list;
    voice->prev = list->prev;
    list->prev = voice;
    voice->prev->next = voice;
}

static void Voice_FromList(VOICE *voice)
{
    voice->prev->next = voice->next;
    voice->next->prev = voice->prev;
}

static void ResetChannel(int channel)
{
    midChannel[channel].sustain = 0;
    midChannel[channel].bend = 0;
    midChannel[channel].expression = 127;
    midChannel[channel].rpn = (127 << 7) | 127;
}

static void Voice_Off(VOICE *voice)
{
    OPL_VoiceOff(voice->index);

    Voice_FromList(voice);
    Voice_ToList(&voiceList, voice);

    voiceCount--;
}

static void VoiceVolume(CHANNEL *channel, VOICE *voice)
{
    u32     volume;

    volume = volumeTable[channel->volume] * volumeTable[channel->expression] * volumeTable[voice->volume];
    OPL_Volume(voice->index, volume >> 12);
}

static void Event_ResetControllers()
{
    ResetChannel(curTrack->channel);
}

static void Event_NoteOff()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    int     note = curTrack->data[0];
    //int         volume = curTrack->data[1];
    VOICE   *voice = channel->voice.next;

    while (voice != &channel->voice)
    {
        if (voice->note == note)
        {
            if (channel->sustain)
            {
                voice->playing = NOTE_OFF;
            }
            else
            {
                voice = voice->prev;
                Voice_Off(voice->next);
            }
        }
        voice = voice->next;
    }
}

static void Voice_Frequency(VOICE *voice, CHANNEL *channel)
{
    OPL_Frequency(voice->index, voice->pitch + channel->bend * channel->bendRange);
}

static void KillQuietest(VOICE *voice)
{
    CHANNEL *channel = &midChannel[0];
    int     i;

    for (i = 0; i < 16; i++, channel++)
    {
        if (channel->voice.prev != &channel->voice)
        {
            // pick the quietest of the oldest voices
            if (channel->voice.prev->volume < voice->volume)
            {
                voice = channel->voice.prev;
            }
        }
    }

    Voice_Off(voice);
}

static void SetupInstrument(CHANNEL *channel, int instrument)
{
    channel->instrument = &gmInstr[instrument];
    channel->voiceCount = 1;
    channel->tuning[1] = 0;
    if (channel->instrument->flags & GM_FLAG_2VOICE)
    {
        channel->voiceCount = 2;
        channel->tuning[1] = (u8)channel->instrument->fineTuning - 128;
    }
}

static void Event_NoteOn()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    int     note = curTrack->data[0], key = note;
    int     volume = curTrack->data[1];
    GMVOICE *gmVoice;

    VOICE   *voice;
    int     count = 0;

    if (volume == 0)
    {
        Event_NoteOff();
        return;
    }

    if (curTrack->channel == percChannel)
    {
        if (note < 35 || note > 81)
        {
            return;
        }

        SetupInstrument(channel, 128 + note - 35);
    }

    voice = channel->voice.next;
    while (voice != &channel->voice && count < channel->voiceCount)
    {
        if (voice->note == note)
        {
            voice->playing = NOTE_PLAY;
            voice->volume = volume;
            VoiceVolume(channel, voice);
            OPL_VoiceOn(voice->index); // restart note
            count++;
        }
        voice = voice->next;
    }

    while (NVOICES - voiceCount < channel->voiceCount - count)
    {
        KillQuietest(&channel->voice);
    }

    for ( ; voiceList.next != &voiceList && count < channel->voiceCount; count++)
    {
        voice = voiceList.next;
        Voice_FromList(voice);
        Voice_ToList(&channel->voice, voice);

        // original note value or percussion instrument
        voice->note = note;

        gmVoice = &channel->instrument->voice[count];

        OPL_Op(voice->index, 0, gmVoice->mod);
        OPL_Op(voice->index, 1, gmVoice->car);

        OPL_Feedback(voice->index, gmVoice->feedback);
        OPL_Pan(voice->index, channel->pan);

        voice->volume = volume;
        VoiceVolume(channel, voice);

        if (channel->instrument->flags & GM_FLAG_FIXED)
        {
            key = channel->instrument->fixedNote;
        }
        else if (curTrack->channel != percChannel)
        {
            key = note + gmVoice->baseNoteOffset;
        }
        voice->pitch = key * 64 + channel->tuning[count];
        Voice_Frequency(voice, channel);

        OPL_VoiceOn(voice->index);

        voice->playing = NOTE_PLAY;
        voiceCount++;
    }
}

static void Event_AllSoundOff(CHANNEL *channel)
{
    while (channel->voice.next != &channel->voice)
    {
        OPL_Volume(channel->voice.next->index, 0x00);
        Voice_Off(channel->voice.next);
    }
}

static void Event_AllNotesOff()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    VOICE   *voice;

    if (!channel->sustain)
    {
        while (channel->voice.next != &channel->voice)
        {
            Voice_Off(channel->voice.next);
        }

        return;
    }

    voice = channel->voice.next;
    while (voice != &channel->voice)
    {
        voice->playing = NOTE_OFF;
        voice = voice->next;
    }
}

static void Event_PitchWheel()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    VOICE   *voice = channel->voice.next;

    channel->bend = curTrack->data[1] - 64; // coarse value

    while (voice != &channel->voice)
    {
        Voice_Frequency(voice, channel);
        voice = voice->next;
    }
}

static void Event_Aftertouch()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    int     note = curTrack->data[0];
    int     volume = curTrack->data[1];
    VOICE   *voice = channel->voice.next;

    while (voice != &channel->voice)
    {
        if (voice->note == note)
        {
            voice->volume = volume;
            VoiceVolume(channel, voice);
        }
        voice = voice->next;
    }
}

static void Event_Sustain()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    VOICE   *voice = channel->voice.next;

    channel->sustain = curTrack->data[1] < 64 ? 0 : 1;

    if (channel->sustain)
    {
        return;
    }

    while (voice != &channel->voice)
    {
        if (voice->playing == NOTE_OFF)
        {
            voice = voice->prev;
            Voice_Off(voice->next);
        }

        voice = voice->next;
    }
}

static void Event_ChannelVolume()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    VOICE   *voice = channel->voice.next;

    channel->volume = curTrack->data[1];

    while (voice != &channel->voice)
    {
        VoiceVolume(channel, voice);
        voice = voice->next;
    }
}

static void Event_Pan()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    VOICE   *voice = channel->voice.next;

    channel->pan = curTrack->data[1];

    while (voice != &channel->voice)
    {
        OPL_Pan(voice->index, channel->pan);
        voice = voice->next;
    }
}

static void Event_ChannelAftertouch()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    int     volume = curTrack->data[0];
    VOICE   *voice = channel->voice.next;

    while (voice != &channel->voice)
    {
        voice->volume = volume;
        VoiceVolume(channel, voice);
        voice = voice->next;
    }
}

static void Event_Expression()
{
    CHANNEL *channel = &midChannel[curTrack->channel];
    VOICE   *voice = channel->voice.next;

    channel->expression = curTrack->data[1];

    while (voice != &channel->voice)
    {
        VoiceVolume(channel, voice);
        voice = voice->next;
    }
}

static void Event_ChangeInstrument()
{
    CHANNEL *channel = &midChannel[curTrack->channel];

    SetupInstrument(channel, curTrack->data[1]);
}

static void Event_DataEntryCoarse()
{
    CHANNEL *channel = &midChannel[curTrack->channel];

    if (channel->rpn == 0)
    {
        channel->bendRange = curTrack->data[1];
    }
}

static void Event_Message()
{
    switch (curTrack->data[0])
    {
      case CC_01:
        break;

      case CC_06:
        Event_DataEntryCoarse();
        break;

      case CC_07:
        Event_ChannelVolume();
        break;

      case CC_0a:
        Event_Pan();
        break;

      case CC_0b:
        Event_Expression();
        break;

      case CC_40:
        Event_Sustain();
        break;

      case CC_64:
        midChannel[curTrack->channel].rpn &= (127 << 7);
        midChannel[curTrack->channel].rpn |= curTrack->data[1];
        break;

      case CC_65:
        midChannel[curTrack->channel].rpn &= 127;
        midChannel[curTrack->channel].rpn |= (curTrack->data[1] << 7);
        break;

      case CC_78:
        Event_AllSoundOff(&midChannel[curTrack->channel]);
        break;

      case CC_79:
        Event_ResetControllers();
        break;

      case CC_7b:
        Event_AllNotesOff();
        break;

      case CC_80:
        Event_ChangeInstrument();
        break;

      default:
        break;
    }
}

// control ---------------------------------------------------------------------
static void DoNothing()
{
}

static void SetBeatTempo(int tempo)
{
    beatTempo = tempo;

    Timer_Set(&timerTick, tempo, beatTicks);
}

static void InitTracks()
{
    int     i;

    for (i = 0; i < numTracks; i++)
    {
        midTrack[i].pos = midTrack[i].track;
        midTrack[i].done = 0;
        midTrack[i].clock = 0;
        midTrack[i].Event = DoNothing;
    }

    for (i = 0; i < 16; i++)
    {
        Event_AllSoundOff(&midChannel[i]);
        ResetChannel(i);
        SetupInstrument(&midChannel[i], 0);
        midChannel[i].volume = 100;
        midChannel[i].pan = 64;
        midChannel[i].bendRange = 2;
        midChannel[i].prevVolume = 0;
    }

    musicClock = 0;
    playSamples = 0;

    SetBeatTempo(MICROSEC / 2);
    timerSample.acc = 0;

    numTracksEnded = 0;
    curTrack = &midTrack[0];

    timeTicks = timeRate = 0;

    timerPhase.acc = 0;
}

static void SetTempo()
{
    // if the tempo changes, should playSamples be reset?
    SetBeatTempo((curTrack->data[0] << 16) | (curTrack->data[1] << 8) | curTrack->data[2]);
}

static void EndOfTrack()
{
    curTrack->done = 1;
    numTracksEnded++;
}

static void EndOfMidiTrack()
{
    EndOfTrack();
    curTrack->Event = DoNothing;
    if (numTracksEnded < numTracks)
    {
        if (midType2)
        {
            curTrack++;
            curTrack->clock = musicClock;
        }
        return;
    }

    if (musicLooping == 0 || musicInit < 2)
    {
        musicPlaying = 0;
        return;
    }

    InitTracks();
}

static u32 GetDeltaMidi()
{
    u32     length;
    u8      data;

    // unrolled
    data = *curTrack->pos++;
    length = (data & 127);
    if (data & 128)
    {
        data = *curTrack->pos++;
        length = (length << 7) | (data & 127);
        if (data & 128)
        {
            data = *curTrack->pos++;
            length = (length << 7) | (data & 127);
            if (data & 128)
            {
                data = *curTrack->pos++;
                length = (length << 7) | (data & 127);
            }
        }
    }

    return length;
}

static u32 GetDeltaAlt()
{
    u32     length;
    u8      data;

    // unrolled
    data = *curTrack->pos++;
    length = (data & 127);
    if (data < 128)
    {
        data = *curTrack->pos++;
        length |= ((data & 127) << 7);
        if (data < 128)
        {
            data = *curTrack->pos++;
            length |= ((data & 127) << 14);
            if (data < 128)
            {
                data = *curTrack->pos++;
                length |= ((data & 127) << 21);
            }
        }
    }

    return length;
}

// prevents unecessary events during inital score timing
static void NoEvent(EVENT event)
{
    (void)event;
}

static void NewEvent(EVENT event)
{
    curTrack->Event = event;
}

static int GetMusEvent(u32 *time)
{
    u8      data, last;

    curTrack->Event = DoNothing;

    data = *curTrack->pos++;
    curTrack->channel = data & 0x0f;
    last = data & 0x80;

    *time = 0;

    switch (data & 0x70)
    {
      case 0x00: // release note
        curTrack->data[0] = *curTrack->pos++ & 0x7f;
        AddEvent(Event_NoteOff);
        break;

      case 0x10: // play note
        data = *curTrack->pos++;
        curTrack->data[0] = data & 0x7f;
        if (data & 0x80)
        {
            curTrack->data[1] = *curTrack->pos++ & 0x7f;
            // should the volume be saved if the value is 0?
            midChannel[curTrack->channel].prevVolume = curTrack->data[1];
        }
        else
        {
            curTrack->data[1] = midChannel[curTrack->channel].prevVolume;
        }
        AddEvent(Event_NoteOn);
        break;

      case 0x20: // pitch wheel adjusted to 7 bit
        curTrack->data[0] = 0;
        curTrack->data[1] = *curTrack->pos++ >> 1;
        AddEvent(Event_PitchWheel);
        break;

      case 0x30: // system event
        curTrack->data[0] = controllerMap[*curTrack->pos++ & 0x0f];
        AddEvent(Event_Message);
        break;

      case 0x40: // change controller
        curTrack->data[0] = controllerMap[*curTrack->pos++ & 0x0f];
        curTrack->data[1] = *curTrack->pos++ & 0x7f;
        AddEvent(Event_Message);
        break;

      case 0x50: // end of measure?
        break;

      case 0x60: // score end
        last = 1; // assume last
        if (musicLooping && musicInit == 2)
        {
            AddEvent(InitTracks);
            last = 0;
        }
        else
        {
            curTrack->Event = EndOfTrack;
        }
        break;

      case 0x70:
        // requires one data byte but is unused
        curTrack->data[0] = *curTrack->pos++;
        break;
    }

    curTrack->Event();

    if (last & 0x80)
    {
        *time = GetDeltaMidi();
    }

    return last;
}

static void GetMidiEvent()
{
    u8      data, event = 0x0;
    u32     length;

    curTrack->Event = DoNothing;

    data = *curTrack->pos;

    if (data & 0x80)
    {
        event = *curTrack->pos++;
    }
    else
    {
        data = curTrack->running;
    }

    curTrack->channel = data & 0x0f;

    switch (data & 0xf0)
    {
      case 0x80:
        curTrack->data[0] = *curTrack->pos++ & 0x7f;
        curTrack->data[1] = *curTrack->pos++ & 0x7f;
        AddEvent(Event_NoteOff);
        break;

      case 0x90:
        curTrack->data[0] = *curTrack->pos++ & 0x7f;
        curTrack->data[1] = *curTrack->pos++ & 0x7f;
        AddEvent(Event_NoteOn);
        break;

      case 0xa0:
        curTrack->data[0] = *curTrack->pos++ & 0x7f;
        curTrack->data[1] = *curTrack->pos++ & 0x7f;
        AddEvent(Event_Aftertouch);
        break;

      case 0xb0: // controller message
        curTrack->data[0] = *curTrack->pos++ & 0x7f;
        curTrack->data[1] = *curTrack->pos++ & 0x7f;
        AddEvent(Event_Message);
        break;

      case 0xc0:
        // instrument number must be in 2nd byte of event.data
        //  as that's where MUS puts it
        curTrack->data[1] = *curTrack->pos++ & 0x7f;
        AddEvent(Event_ChangeInstrument);
        break;

      case 0xd0:
        curTrack->data[0] = *curTrack->pos++ & 0x7f;
        AddEvent(Event_ChannelAftertouch);
        break;

      case 0xe0: // pitch wheel
        curTrack->data[0] = *curTrack->pos++ & 0x7f; // fine
        curTrack->data[1] = *curTrack->pos++ & 0x7f; // coarse
        AddEvent(Event_PitchWheel);
        break;

      case 0xf0:
        if ((data & 0x0f) == 0xf) // meta event
        {
            data = *curTrack->pos++;
            if (data == 0x2f) // end of track
            {
                curTrack->Event = EndOfMidiTrack;
                return;
            }
            else if (data == 0x51 && *curTrack->pos == 3)
            {
                curTrack->data[0] = curTrack->pos[1];
                curTrack->data[1] = curTrack->pos[2];
                curTrack->data[2] = curTrack->pos[3];
                curTrack->Event = SetTempo;
            }
        }

        length = GetDelta();
        curTrack->pos += length;
        return; // don't want to affect running events
    }

    if (event & 0x80)
    {
        curTrack->running = event;
    }
}

static void TrackMusEvents()
{
    u32     ticks;

    if (curTrack->done)
    {
        musicPlaying = 0;
        return;
    }

    if (curTrack->clock > musicClock)
    {
        return;
    }

    while (GetMusEvent(&ticks) == 0)
    {
    }

    curTrack->clock += ticks;
}

static void SingleTrackMidiEvents()
{
    u32     ticks;

    if (curTrack->clock == musicClock)
    {
        do
        {
            curTrack->Event();

            if (curTrack->done)
            {
                break;
            }

            ticks = GetDelta();
            curTrack->clock += ticks;
            GetMidiEvent();
        }
        while (ticks == 0);
    }
}

static void TrackMidiEvents()
{
    curTrack = &midTrack[0];

    do
    {
        SingleTrackMidiEvents();

        curTrack++;
    }
    while (curTrack <= endTrack);
}

static int UpdateEvents()
{
    int     rate = Timer_Update(&timerTick);

    MusicEvents();
    musicClock++;

    timeRate += rate;
    if (timeRate >= MICROSEC / 10)
    {
        timeRate -= MICROSEC / 10;
        timeTicks++;
    }

    return rate;
}

static void LoadMusTrack(u8 *data)
{
    midTrack[0].track = data;

    numTracks = 1;

    MusicEvents = TrackMusEvents;
    percChannel = 15;
}

static int LoadMidiTracks(int count, u8 *data, int size)
{
    int     track;
    u32     length;

    for (track = 0; track < count; track++)
    {
        if (size < 8 || !ID(data, "MTrk"))
        {
            return 1;
        }

        length = BE32(data + 4) + 8;
        if (size < length)
        {
            return 1;
        }

        midTrack[track].track = data + 8;

        data += length;
        size -= length;
    }

    numTracks = count;
    endTrack = &midTrack[numTracks - 1];

    GetDelta = GetDeltaMidi;

    MusicEvents = TrackMidiEvents;
    percChannel = 9;

    return 0;
}

static int LoadHmpTrack(int count, u8 *data, int size)
{
    int     track;
    u32     length;

    for (track = 0; track < count; track++)
    {
        if (size < 12)
        {
            return 1;
        }

        length = LE32(data + 4);
        if (size < length)
        {
            return 1;
        }

        midTrack[track].track = data + 12;

        data += length;
        size -= length;
    }

    numTracks = count;
    endTrack = &midTrack[numTracks - 1];

    GetDelta = GetDeltaAlt;

    MusicEvents = TrackMidiEvents;
    percChannel = 9;

    return 0;
}

// midiplay --------------------------------------------------------------------
int Midiplay_Init(int samplerate, char *genmidi)
{
    VOICE   *voice;

    if (!ID(genmidi, "#OPL_II#"))
    {
        return 1;
    }

    gmInstr = (GENMIDI *)(genmidi + 8);

    // 0 = off, 1 = on
    OPL_TremoloDepth(1);
    OPL_VibratoDepth(1);
    OPL_Reset();

    Timer_Set(&timerPhase, 49716, samplerate);
    Timer_Set(&timerSample, MICROSEC, samplerate);

    voice = &midVoice[0];
    for ( ; voiceList.prev->index + 1 < NVOICES; voice++)
    {
        voice->index = voiceList.prev->index + 1;
        Voice_ToList(&voiceList, voice);
    }

    AddEvent = NewEvent;

    musicInit = 1;

    return 0;
}

int Midiplay_Load(void *data, int size)
{
    u8      *byte = (u8 *)data;
    int     offset;

    if (musicInit == 0)
    {
        return 1;
    }

    musicInit = 1;
    musicPlaying = 0;

    midType2 = 0;

    if (size > MUS_HDRSIZE && ID(byte, "MUS\x1a"))
    {
        if (size < LE16(byte + 6) + LE16(byte + 4))
        {
            return 3;
        }

        beatTicks = LE16(byte + 14);
        if (beatTicks == 0)
        {
            beatTicks = 70;
        }

        LoadMusTrack(byte + LE16(byte + 6));
    }
    else if (size > MID_HDRSIZE && ID(byte, "MThd"))
    {
        beatTicks = BE16(byte + 12);
        if (beatTicks < 0)
        {
            return 2; // no support for SMPTE format
        }

        size -= MID_HDRSIZE;
        if (LoadMidiTracks(BE16(byte + 10), byte + MID_HDRSIZE, size) == 1)
        {
            return 3;
        }
        if (BE16(byte + 8) == 2)
        {
            midType2 = 1;
            MusicEvents = SingleTrackMidiEvents;
        }
    }
    else if (size > HMP_HDRSIZE && ID(byte, "HMIMIDIP"))
    {
        // with version 1 these are always the same
        //  but differ by varying amounts in version 2
        if (size < LE32(byte + 32))
        {
            return 3;
        }

        offset = 712; // HMP version 1
        if (ID(byte + 8, "013195"))
        {
            offset += 128; // version 2
        }

        beatTicks = 60;
        // beats per minute seem to be 120 for every HMP file (byte + 56)

        offset += HMP_HDRSIZE;
        if (LoadHmpTrack(LE32(byte + 48), byte + offset, size - offset) == 1)
        {
            return 3;
        }
    }
    else
    {
        return 2;
    }

    InitTracks();

    AddEvent = NoEvent;
    while (numTracksEnded < numTracks)
    {
        UpdateEvents();
    }
    AddEvent = NewEvent;

    musicInit = 2;

    return 0;
}

void Midiplay_Play(int playing)
{
    if (musicInit < 2)
    {
        return;
    }

    if (numTracksEnded == numTracks)
    {
        InitTracks();
    }

    musicPlaying = playing ? 2 : 0;
}

void Midiplay_SetVolume(int volume)
{
    if (volume < 0)
    {
        volume = 0;
    }
    else if (volume > 127)
    {
        volume = 127;
    }

    musicVolume = volumeTable[volume];
}

int Midiplay_IsPlaying()
{
    if (musicInit < 2)
    {
        return 0;
    }

    if (numTracksEnded < numTracks)
    {
        return 1;
    }

    return 0;
}

void Midiplay_Output(int sample[2])
{
    int     buffer[2] = {0, 0};
    int     rate;
    int     divisor = 256;

    if (musicPlaying == 2)
    {
        playSamples -= Timer_Update(&timerSample);
        if (playSamples < 0)
        {
            playSamples += UpdateEvents();
        }
    }

    rate = Timer_Update(&timerPhase);
    divisor *= rate;
    while (rate--)
    {
        OPL_Generate(buffer);
    }

    sample[0] = buffer[0] * musicVolume / divisor;
    sample[1] = buffer[1] * musicVolume / divisor;
}

int Midiplay_Time()
{
    if (musicInit < 2)
    {
        return 0;
    }

    return timeTicks;
}

void Midiplay_Loop(int looping)
{
    musicLooping = looping;
}

void Midiplay_Restart()
{
    // this safeguards the callback from generating events
    //  whilst we're initialising the tracks
    musicPlaying |= 1; // pause playback

    InitTracks();
}

// midiplay
