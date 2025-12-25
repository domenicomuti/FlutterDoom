//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------



#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include "doomdef.h"

#if IS_WINDOWS
    #include <sysinfoapi.h>
	#include <malloc.h>
    #include <stdlib.h>
    #include <synchapi.h>
    #define sleep(x) Sleep((x) * 1000)
    #define usleep(x) Sleep((x) / 1000)
#else
    #include <sys/time.h>
    #include <unistd.h>
#endif

#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"

#include "d_net.h"
#include "g_game.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"
#include "debug.h"



int	mb_used = 6;

extern d_bool exit_doom_loop;


void
I_Tactile
( int	on,
  int	off,
  int	total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t	emptycmd;
ticcmd_t*	I_BaseTiccmd(void)
{
    return &emptycmd;
}


int  I_GetHeapSize (void)
{
    return mb_used*1024*1024;
}

byte* I_ZoneBase (int*	size)
{
    *size = mb_used*1024*1024;
    return (byte *) malloc (*size);
}



//
// I_GetTime
// returns time in 1/70th second tics
//
int  I_GetTime (void)
{
    #if IS_WINDOWS
    int	newtics;
    static UINT64 basetime = 0;

    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    UINT64 time = ((UINT64)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    if (!basetime)
        basetime = time;
    newtics = (time - basetime)*TICRATE/10000000;
    return newtics;

    #else
    struct timeval tp;
    struct timezone	tzp;
    int	newtics;
    static int basetime=0;
  
    gettimeofday(&tp, &tzp);
    if (!basetime)
	    basetime = tp.tv_sec;
    newtics = (tp.tv_sec-basetime)*TICRATE + tp.tv_usec*TICRATE/1000000;
    return newtics;

    #endif
}



//
// I_Init
//
void I_Init (void)
{
    I_InitSound();
    I_InitMusic();
    //I_InitGraphics();
}

//
// I_Quit
//
void I_Quit (void)
{
    D_QuitNetGame ();
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults ();
    I_ShutdownGraphics();
    exit(0);
    
    //exit_doom_loop = true;
}

void I_WaitVBL(int count)
{
#ifdef SGI
    sginap(1);                                           
#else
#ifdef SUN
    sleep(0);
#else
    usleep (count * (1000000/70) );                                
#endif
#endif
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

byte*	I_AllocLow(int length)
{
    byte*	mem;
        
    mem = (byte *)malloc (length);
    memset (mem,0,length);
    return mem;
}


//
// I_Error
//
extern d_bool demorecording;

void I_Error (char *error, ...)
{
    va_list	argptr;

    // Message first.
    va_start (argptr,error);
    LOG ( "Error: ");
    LOG (error,argptr);
    LOG ( "\n");
    va_end (argptr);

    //fflush( stderr );

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownGraphics();
    
    exit(-1);
}
