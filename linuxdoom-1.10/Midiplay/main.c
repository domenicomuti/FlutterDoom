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

#include <SDL/SDL.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include "midiplay.h"

#define ERROR           printf("\t%s\n\r", strerror(errno))
#define UNKNOWN_FILE    printf("\tUnknown/unsupported\n\r") // error 2
#define INVALID_FILE    printf("\tInternal file error\n\r") // error 3

#define SAMPLERATE      44100
#define SAMPLECOUNT     1024

#define RMI_HDRSIZE     20

#define PRINT_STATUS    printf("\r[%s] [%3i%%]", status, volume)
#define PRINT_TIME(c)   printf(" %*i:%02i.%i\33[%iC", count, time / 600, (time / 10) % 60, time % 10, c)

typedef unsigned char   u8;
typedef unsigned int    u32;

u32 LE32(char *data)
{
    u8      *byte = (u8 *)data;

    return (u32)(*byte | (*(byte + 1) << 8) | (*(byte + 2) << 16) | (*(byte + 3) << 24));
}

short Clamp(int sample)
{
    if (sample > 32767)
    {
        sample = 32767;
    }
    else if (sample < -32768)
    {
        sample = -32768;
    }

    return (short)sample;
}

void SdlCallback(void *unused, Uint8 *stream, int length)
{
    (void)unused;

    short   *output = (short *)stream;
    int     sample[2];

    while (length)
    {
        Midiplay_Output(sample);
        *output++ = Clamp(sample[0]);
        *output++ = Clamp(sample[1]);

        length -= 4;
    }
}
 
int Digits(int number, int count)
{
    if (number > 9)
    {
        count = Digits(number / 10, count + 1);
    }

    return count;
}

char *ReadFile(char *name, int *size)
{
    struct stat status;
    FILE        *file;
    char        *buffer;

    *size = 0;

    if (stat(name, &status) < 0)
    {
        ERROR;
        return NULL;
    }

    if (S_ISDIR(status.st_mode))
    {
        errno = EISDIR; // needs to be set explicitly
        ERROR;
        return NULL;
    }

    if ((file = fopen(name, "rb")) == NULL)
    {
        ERROR;
        return NULL;
    }

    buffer = malloc(status.st_size);
    fread(buffer, status.st_size, 1, file);
    fclose(file);

    *size = status.st_size;

    return buffer;
}

int main(int argc, char **argv)
{
    SDL_AudioSpec   want;

    struct termios  termAttr;
    tcflag_t        localMode, outputMode;
    int             blockMode;

    char            *buffer, *data, *genmidi;
    int             length;

    int             arg;
    char            *name;

    int             time;

    int             result;

    int             count;

    int             looping = 0;
    int             playing = 0;
    int             volume = 100;
    char            status[3] = "P ";

    char            key;

    if (argc < 3)
    {
        printf("Usage: %s GENMIDI [FILES]\n\tSpecify one or more FILES to play\n", argv[0]);
        return 1;
    }

    if ((genmidi = ReadFile(argv[1], &length)) == NULL)
    {
        return 1;
    }

    if (length != GENMIDI_SIZE)
    {
        free(genmidi);
        printf("GENMIDI byte size incorrect: %i\n", length);
        return 1;
    }

    if (Midiplay_Init(SAMPLERATE, genmidi))
    {
        free(genmidi);
        printf("Invalid GENMIDI\n");
        return 1;
    }

    printf("P = Play/Pause | N = Next | R = Replay | L = Loop | +/- Volume | Q = Quit\n");

    SDL_Init(SDL_INIT_AUDIO);

    want.freq = SAMPLERATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = SAMPLECOUNT;
    want.callback = SdlCallback;

    SDL_OpenAudio(&want, NULL);
    SDL_PauseAudio(SDL_TRUE);

    blockMode = fcntl(STDIN_FILENO, F_GETFL, FNONBLOCK);
    tcgetattr(STDIN_FILENO, &termAttr);
    localMode = termAttr.c_lflag;
    outputMode = termAttr.c_oflag;
    termAttr.c_lflag &= ~(ICANON | ECHO | ISIG);
    termAttr.c_oflag &= ~ONLCR;
    tcsetattr(STDIN_FILENO, TCSANOW, &termAttr);
    fcntl(STDIN_FILENO, F_SETFL, FNONBLOCK);

    for (arg = 2; arg < argc; arg++)
    {
        if ((name = strrchr(argv[arg], '/')) == NULL)
        {
            name = argv[arg];
        }
        else
        {
            name++;
        }

        count = Digits(argc - 2, 1);
        printf("%*i/%i: %s\r\n", count, arg - 1, argc - 2, name);

        if ((buffer = ReadFile(argv[arg], &length)) == NULL)
        {
            continue;
        }

        data = buffer;
        if (length >= 4 && strncmp(data, "RIFF", 4) == 0)
        {
            if (length < RMI_HDRSIZE || LE32(data + 4) > length - 8)
            {
                INVALID_FILE;
                continue;
            }

            if (strncmp((data + 8), "RMIDdata", 8) != 0)
            {
                INVALID_FILE;
                continue;
            }

            if (length - RMI_HDRSIZE < LE32(data + 16))
            {
                INVALID_FILE;
                continue;
            }
            // looks like an RMI file
            // adjust the size to the midi data chunk
            length = LE32(data + 16);
            data += RMI_HDRSIZE;
        }

        if ((result = Midiplay_Load(data, length)) == 0)
        {
            PRINT_STATUS;
            time = Midiplay_Time();
            count = Digits(time / 600, 1);
            printf(" %*i:00.0 /", count, 0);
            PRINT_TIME(1);

            Midiplay_Play(1);

            while (Midiplay_IsPlaying())
            {
                SDL_Delay(10);

                key = getc(stdin);
                if (key == 'l')
                {
                    looping = 1 - looping;
                    status[1] = looping ? 'L' : ' ';
                    Midiplay_Loop(looping);
                }
                else if (key == 'p')
                {
                    playing = 1 - playing;
                    status[0] = playing ? ' ' : 'P';
                    SDL_PauseAudio(playing ? SDL_FALSE : SDL_TRUE);
                }
                else if (key == 'n')
                {
                    break;
                }
                else if (key == 'q')
                {
                    arg = argc;
                    break;
                }
                else if (key == '-' && volume > 0)
                {
                    volume--;
                    Midiplay_SetVolume(volume * 127 / 100);
                }
                else if (key == '=' && volume < 100)
                {
                    volume++;
                    Midiplay_SetVolume(volume * 127 / 100);
                }
                else if (key == 'r')
                {
                    Midiplay_Restart();
                    Midiplay_Play(1);
                }

                PRINT_STATUS;
                time = Midiplay_Time();
                PRINT_TIME(count + 9);
            }

            Midiplay_Play(0);

            printf("\r\33[K\r");
        }
        else if (result == 2)
        {
            UNKNOWN_FILE;
        }
        else if (result == 3)
        {
            INVALID_FILE;
        }

        free(buffer);
    }

    free(genmidi);

    termAttr.c_lflag = localMode;
    termAttr.c_oflag = outputMode;
    tcsetattr(STDIN_FILENO, TCSANOW, &termAttr);
    fcntl(STDIN_FILENO, F_SETFL, blockMode);

    SDL_CloseAudio();

    SDL_Quit();

    return 0;
}

// midiplay
