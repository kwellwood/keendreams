/* Keen Dreams (SDL2/Steam Port) Source Code
 * Copyright (C) 2014 Javier M. Chavez
 * Copyright (C) 2015 David Gow <david@davidgow.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// KD_MAIN.C
/*
=============================================================================

							KEEN DREAMS

					An Id Software production

=============================================================================
*/

#include "string.h"

#include "kd_def.h"
#include <SDL2/SDL.h>
#pragma hdrstop

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

char		str[80],str2[20];
boolean		singlestep,jumpcheat,godmode,tedlevel;
unsigned	tedlevelnum;

int		_argc;
char		**_argv;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

void	DebugMemory (void);
void	TestSprites(void);
int		DebugKeys (void);
void	ShutdownId (void);
void	Quit (char *error);
void	InitGame (void);
int	main (int argc, char **argv);
int	g_minTics = 2;

//===========================================================================

#if FRILLS

/*
==================
=
= DebugMemory
=
==================
*/

void DebugMemory (void)
{
	VW_FixRefreshBuffer ();
	US_CenterWindow (16,10);

	US_CPrint ("Memory Usage");
	US_CPrint ("------------");
	US_Print ("Total      :");
	US_PrintUnsigned (MM_SystemMegs());
	US_Print ("m\nFree      :");
	US_PrintUnsigned (MM_UnusedMemory());
	US_Print ("m\nIn Use    :");
	US_PrintUnsigned (MM_UsedMemory() / 1024);
	US_Print ("k\n");
	US_Print ("Blocks:\nUsed      :");
	US_PrintUnsigned (MM_UsedBlocks());
	US_Print ("\nPurgable :");
	US_PrintUnsigned (MM_PurgableBlocks());
	VW_UpdateScreen();
	IN_Ack ();
#if GRMODE == EGAGR
	MM_ShowMemory ();
#endif
}

/*
===================
=
= TestSprites
=
===================
*/

#define DISPWIDTH	110
#define	TEXTWIDTH   40
void TestSprites(void)
{
	int hx,hy,sprite,oldsprite,bottomy,topx,shift;
	spritetabletype *spr;
	unsigned	mem,scan;


	VW_FixRefreshBuffer ();
	US_CenterWindow (30,17);

	US_CPrint ("Sprite Test");
	US_CPrint ("-----------");

	hy=PrintY;
	hx=(PrintX+56)&(~7);
	topx = hx+TEXTWIDTH;

	US_Print ("Chunk:\nWidth:\nHeight:\nOrgx:\nOrgy:\nXl:\nYl:\nXh:\nYh:\n"
			  "Shifts:\nMem:\n");

	bottomy = PrintY;

	sprite = STARTSPRITES;
	shift = 0;

	do
	{
		if (sprite>=STARTTILE8)
			sprite = STARTTILE8-1;
		else if (sprite<STARTSPRITES)
			sprite = STARTSPRITES;

		spr = &spritetable[sprite-STARTSPRITES];

		VWB_Bar (hx,hy,TEXTWIDTH,bottomy-hy,WHITE);

		PrintX=hx;
		PrintY=hy;
		US_PrintUnsigned (sprite);US_Print ("\n");PrintX=hx;
		US_PrintUnsigned (spr->width);US_Print ("\n");PrintX=hx;
		US_PrintUnsigned (spr->height);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->orgx);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->orgy);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->xl);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->yl);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->xh);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->yh);US_Print ("\n");PrintX=hx;
		US_PrintSigned (spr->shifts);US_Print ("\n");PrintX=hx;
		if (!grsegs[sprite])
		{
			US_Print ("-----");
		}
		else
		{
			mem = spr->width * spr->height;
			mem = (mem+15)&(~15);		// round to paragraphs
			US_PrintUnsigned (mem);
		}

		oldsprite = sprite;
		do
		{
		//
		// draw the current shift, then wait for key
		//
			VWB_Bar(topx,hy,DISPWIDTH,bottomy-hy,WHITE);
			if (grsegs[sprite])
			{
				PrintX = topx;
				PrintY = hy;
				US_Print ("Shift:");
				US_PrintUnsigned (shift);
				US_Print ("\n");
				VWB_DrawSprite (topx+16+shift*2,PrintY,sprite);
			}

			VW_UpdateScreen();

			scan = IN_WaitForKey ();

			switch (scan)
			{
			case sc_UpArrow:
				sprite++;
				break;
			case sc_DownArrow:
				sprite--;
				break;
			case sc_LeftArrow:
				if (--shift == -1)
					shift = 3;
				break;
			case sc_RightArrow:
				if (++shift == 4)
					shift = 0;
				break;
			case sc_Escape:
				return;
			}

		} while (sprite == oldsprite);

  } while (1);


}

#endif

extern	unsigned	xpanmask;			// prevent panning to odd pixels
extern char *lineInputDescription;

/*
================
=
= DebugKeys
=
================
*/
int DebugKeys (void)
{
	boolean esc;
	int level;

#if FRILLS
	if (Keyboard[SDL_GetScancodeFromKey(SDLK_e)] && ingame)	// DEBUG: end + 'E' to quit level
	{
		if (tedlevel)
			TEDDeath();
		playstate = levelcomplete;
	}
#endif

	if (Keyboard[SDL_GetScancodeFromKey(SDLK_g)] && ingame)		// G = god mode
	{
		VW_FixRefreshBuffer ();
		US_CenterWindow (12,2);
		if (godmode)
		  US_PrintCentered ("God mode OFF");
		else
		  US_PrintCentered ("God mode ON");
		VW_UpdateScreen();
		IN_Ack();
		godmode ^= 1;
		return 1;
	}
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_i)])			// I = item cheat
	{
		VW_FixRefreshBuffer ();
		US_CenterWindow (12,3);
		US_PrintCentered ("Free items!");
		gamestate.boobusbombs=99;
		gamestate.flowerpowers=99;
		gamestate.keys=99;
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_j)])			// J = jump cheat
	{
		jumpcheat^=1;
		VW_FixRefreshBuffer ();
		US_CenterWindow (18,3);
		if (jumpcheat)
			US_PrintCentered ("Jump cheat ON");
		else
			US_PrintCentered ("Jump cheat OFF");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
#if FRILLS
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_m)])			// M = memory info
	{
		DebugMemory();
		return 1;
	}
#endif
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_p)])			// P = pause with no screen disruptioon
	{
		IN_Ack();
	}
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_s)] && ingame)	// S = slow motion
	{
		singlestep^=1;
		VW_FixRefreshBuffer ();
		US_CenterWindow (18,3);
		if (singlestep)
			US_PrintCentered ("Slow motion ON");
		else
			US_PrintCentered ("Slow motion OFF");
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
#if FRILLS
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_t)])			// T = sprite test
	{
		TestSprites();
		return 1;
	}
#endif
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_w)] && ingame)	// W = warp to level
	{
		VW_FixRefreshBuffer ();
		US_CenterWindow(26,3);
		PrintY+=6;
		US_Print("  Warp to which level(0-16):");
		VW_UpdateScreen();
		lineInputDescription = "Warp to which level(0-16):";
		esc = !US_LineInput (px,py,str,NULL,true,2,0);
		if (!esc)
		{
			level = atoi (str);
			if (level>=0 && level<=16)
			{
				gamestate.mapon = level;
				playstate = warptolevel;
			}
		}
		return 1;
	}
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_v)])
	{
		int vsync = SDL_GL_GetSwapInterval();
		VW_FixRefreshBuffer ();
		US_CenterWindow (18,3);
		if (vsync == -1)
		{
			US_PrintCentered ("VSync OFF");
			SDL_GL_SetSwapInterval(0);
		}
		else if (vsync == 0)
		{
			US_PrintCentered ("VSync ON");
			SDL_GL_SetSwapInterval(1);
		}
		else
		{
			if (SDL_GL_SetSwapInterval(-1) == 0)
			{
				US_PrintCentered ("VSync ADAPTIVE");
			}
			else
			{
				US_PrintCentered ("VSync OFF");
				SDL_GL_SetSwapInterval(0);
			}
		}
		VW_UpdateScreen();
		IN_Ack ();
		return 1;
	}
	else if (Keyboard[SDL_GetScancodeFromKey(SDLK_h)])
	{
		VW_FixRefreshBuffer ();
		US_CenterWindow (18, 3);
		if (g_minTics == 2)
		{
			US_PrintCentered ("70Hz Mode ON");
			g_minTics = 1;
			xpanmask = 7;
		}
		else
		{
			US_PrintCentered ("70Hz Mode OFF");
			xpanmask = (fakecga)?4:6;
			g_minTics = 2;
		}
		VW_UpdateScreen();
		IN_Ack();
		return 1;
	}
	return 0;
}

//===========================================================================

/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId (void)
{
  US_Shutdown ();
  SD_Shutdown ();
  IN_Shutdown ();
  RF_Shutdown ();
  VW_Shutdown ();
  CA_Shutdown ();
  MM_Shutdown ();
}

//===========================================================================

/*
==========================
=
= Quit
=
==========================
*/

void Quit (char *error)
{
  ShutdownId ();
  if (error && *error)
  {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Keen Dreams", error, 0);
	puts(error);
	puts("\n");
	exit(1);
  }
	exit (0);
}

//===========================================================================

/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

#if 0
#include "piracy.h"
#endif

void InitGame (void)
{
	int i;

	MM_Startup ();


#if 0
	// Handle piracy screen...
	//
	movedata(FP_SEG(PIRACY),(unsigned)PIRACY,0xb800,displayofs,4000);
	while ((bioskey(0)>>8) != sc_Return);
#endif


	// XXX: Removed check that the user has at least 335KB of memory if
	// EGA

	// XXX We don't do a text screen.
	//US_TextScreen();

	VW_Startup ();
	RF_Startup ();
	IN_Startup ();
	SD_Startup ();
	US_Startup ();

//	US_UpdateTextScreen();

	CA_Startup ();
	US_Setup ();

//
// load in and lock down some basic chunks
//

	CA_ClearMarks ();

	CA_MarkGrChunk(STARTFONT);
	CA_MarkGrChunk(STARTFONTM);
	CA_MarkGrChunk(STARTTILE8);
	CA_MarkGrChunk(STARTTILE8M);
	for (i=KEEN_LUMP_START;i<=KEEN_LUMP_END;i++)
		CA_MarkGrChunk(i);

	CA_CacheMarks (NULL, 0);

	MM_SetLock (&grsegs[STARTFONT],true);
	MM_SetLock (&grsegs[STARTFONTM],true);
	MM_SetLock (&grsegs[STARTTILE8],true);
	MM_SetLock (&grsegs[STARTTILE8M],true);
	for (i=KEEN_LUMP_START;i<=KEEN_LUMP_END;i++)
		MM_SetLock (&grsegs[i],true);

	CA_LoadAllSounds ();

	fontcolor = WHITE;

	//US_FinishTextScreen();

	VW_SetScreenMode (GRMODE);
	VW_ClearVideo (BLACK);

	STAT_Init();
}



//===========================================================================

/*
==========================
=
= main
=
==========================
*/

int main (int argc, char **argv)
{
	short i;
	_argc = argc;
	_argv = argv;

	if (_argc > 1 && stricmp(_argv[1], "/VER") == 0)
	{
		printf("\nKeen Dreams version 2.00 (Steam)\n");
		printf("Copyright 1991-1993 Softdisk Publishing.\n");
		printf("Copyright 2014 Javier M. Chavez.\n");
		printf("Remastered version by David Gow\n");
		printf("Commander Keen is a trademark of Id Software.\n");
		exit(0);
	}

	if (_argc > 1 && stricmp(_argv[1], "/?") == 0)
	{
		printf("\nKeen Dreams version 2.00beta1\n");
		printf("Copyright 1991-1993 Softdisk Publishing.\n");
		printf("Copyright 2014 Javier M. Chavez.\n");
		printf("Remastered version by David Gow\n");
		printf("Commander Keen is a trademark of Id Software.\n");
		printf("Type ./KDreams from the DOS prompt to run.\n\n");
		printf("KDREAMS /NOAL disables AdLib and Sound Blaster detection\n");
		printf("KDREAMS /NOJOYS ignores joystick\n");
		printf("KDREAMS /NOMOUSE ignores mouse\n");
		printf("KDREAMS /SWMOUSE for software mouse cursor\n");
		printf("KDREAMS /FULLSCREEN to start in fullscreen mode\n");
		printf("KDREAMS /NOASPECT to start in widescreen mode\n");
		printf("KDREAMS /VER  for version and compatibility information\n");
		printf("KDREAMS /? for this help information\n");
		exit(0);
	}

	//textcolor(7);
	//textbackground(0);

	InitGame();

	DemoLoop();					// DemoLoop calls Quit when everything is done
	Quit("Demo loop exited???");
	return 0;
}

