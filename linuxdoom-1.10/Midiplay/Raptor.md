MUS and Raptor
--------------

The 1994 game Raptor also used MUS files, but were played at a different rate than DOOM music.

In order to accommodate different tick rates (Pulses Per Quarter Note) that may arise in MUS data, Midiplay utilises unused bytes in the MUS header.

Although MUS documentation suggests a tick rate per second for MUS (DOOM = 140Hz, Raptor = 70Hz), it seemed better to translate these rates to fall in line with MIDI specification which uses a default beat (A.K.A. quarter note) tempo of 500,000µs (120 beats per minute), thus MUS data can be played using PPQN. This has the advantages of having more shared code within Midiplay for MUS and MIDI, but also not needing to change the MUS file delta-times to suit the player.

The MUS header is 16 bytes long, and has 2 unused/reserved bytes at the end which can specify the PPQN. A zero value will default to 70ppqn, and any non-zero value will be used instead of the default. For example, changing the value of these 2 bytes to 35 would be suitable for Raptor music. Either 70 or 0, would be suitable for DOOM music. Therefore, 70ppqn = 140Hz, 35ppqn = 70Hz.

Advantages of utilising this method is it standardises MUS files, and preserves the track data.
