# Midiplay

## A MIDI/MUS player
Midiplay is a very robust and fully functional command line player for MIDI using OPL audio synthesis.

Written for Linux in C using SDL for audio output.

Midiplay can be integrated directly into any other project by simply omitting `main.c` and using `midiplay.h` for the API calls.

Midiplay is licensed under the Zlib license, and Nuked OPL3 v1.8 under the GNU LGPL v2.1 license.

[GNU Lesser General Public License, version 2.1](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html)

### Features
Supported MIDI formats:
- MIDI (types 0, 1 & 2)[^3]
- MUS (DMX)
- HMP (both versions)

Also supports the RMI file format; a RIFF container for MIDI.

[^3]: Midiplay does not handle the SMPTE format as it's not widely used, if at all.

### Inspiration
The inspiration for this project came from the original DOOM source code.

The DOOM source code did not come with the ability to play music. DOOM music is in the MUS form; a derivative of MIDI. It has the same basic event/delta-time structure as MIDI.

### Code
The MIDI/MUS parser is written to be as efficient as possible using function pointers to handle events, eliminating unnecessary conditional checks. File data is assumed to be completely valid, and only basic checks are made on loading the data into memory.

Audio is generated using a modified version of Nuked OPL3. Strictly speaking, it is no longer an OPL emulator but just harnesses the OPL synthesizer using more direct function calls.
- 2 operator mode (no percussion modes)
- Increased note range
- Single "chip" implementation capable of 128 voices[^1]
- Full stereo panning
- Post processed operator volume[^2]

[^1]: Change NVOICES in `opl.h` to however many voices are required.
[^2]: Each group of operators (modulator and carrier) are generated before having the volume applied.

To take advantage of OPL, you will need a GENMIDI (General Midi instrument data file). This is the same GENMIDI as found in DOOM. To hear the audio at its best, it's recommended to use DMXOPL.

[DMXOPL - YMF262-enhanced FM patch set for Doom and source ports](https://github.com/sneakernets/DMXOPL)

### Events
Midiplay can handle enough events that should satisfy most MIDI/MUS data.
- MIDI/MUS
  - Note On / Off
  - Channel Volume & Expression
  - Panning
  - All Notes Off
  - All Sounds Off
  - Pitch Bend
  - Change Instrument
  - Sustain
- MIDI
  - After Touch (key pressure)
  - After Touch (channel pressure)
  - Tempo
  - Registered Parameter Number
    - Pitch Bend Range
  - Data Entry

## Sources
- [MID format](https://moddingwiki.shikadi.net/wiki/MID_Format)
- [MUS format](https://moddingwiki.shikadi.net/wiki/MUS_Format)
- [Technical docs](http://midi.teragonaudio.com/)
- [Standard MIDI-File Format Spec 1.1](http://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html)
- [Official MIDI Specifications](https://www.midi.org/specifications)
- [NUKED-OPL3](https://github.com/nukeykt/Nuked-OPL3)
