/* Keen Dreams Source Code
 * Copyright (C) 2014 Javier M. Chavez
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
	US_CenterWindow (16,7);

	US_CPrint ("Memory Usage");
	US_CPrint ("------------");
	US_Print ("Total     :");
	US_PrintUnsigned (MM_SystemMegs());
	US_Print ("m\nFree      :");
	US_PrintUnsigned (MM_UnusedMemory());
	US_Print ("m\nWith purge:");
	US_PrintUnsigned (MM_TotalFree());
	US_Print ("m\n");
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
	if (Keyboard[0x12] && ingame)	// DEBUG: end + 'E' to quit level
	{
		if (tedlevel)
			TEDDeath();
		playstate = levelcomplete;
	}
#endif

	if (Keyboard[0x22] && ingame)		// G = god mode
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
	else if (Keyboard[0x17])			// I = item cheat
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
	else if (Keyboard[0x24])			// J = jump cheat
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
	else if (Keyboard[0x32])			// M = memory info
	{
		DebugMemory();
		return 1;
	}
#endif
	else if (Keyboard[0x19])			// P = pause with no screen disruptioon
	{
		IN_Ack();
	}
	else if (Keyboard[0x1f] && ingame)	// S = slow motion
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
	else if (Keyboard[0x14])			// T = sprite test
	{
		TestSprites();
		return 1;
	}
#endif
	else if (Keyboard[0x11] && ingame)	// W = warp to level
	{
		VW_FixRefreshBuffer ();
		US_CenterWindow(26,3);
		PrintY+=6;
		US_Print("  Warp to which level(0-16):");
		VW_UpdateScreen();
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
		printf("\nKeen Dreams version 1.93 (Rev 1)\n");
		printf("developed for use with 100%% IBM compatibles\n");
		printf("that have 640K memory, DOS version 3.3 or later,\n");
		printf("and an EGA or VGA display adapter.\n");
		printf("Copyright 1991-1993 Softdisk Publishing.\n");
		printf("Commander Keen is a trademark of Id Software.\n");
		exit(0);
	}

	if (_argc > 1 && stricmp(_argv[1], "/?") == 0)
	{
		printf("\nKeen Dreams version 1.93\n");
		printf("Copyright 1991-1993 Softdisk Publishing.\n\n");
		printf("Type KDREAMS from the DOS prompt to run.\n\n");
		printf("KDREAMS /COMP for SVGA compatibility mode\n");
		printf("KDREAMS /NODR stops program hang with the drive still on\n");
		printf("KDREAMS /NOAL disables AdLib and Sound Blaster detection\n");
		printf("KDREAMS /NOSB disables Sound Blaster detection\n");
		printf("KDREAMS /NOJOYS ignores joystick\n");
		printf("KDREAMS /NOMOUSE ignores mouse\n");
		printf("KDREAMS /HIDDENCARD overrides video card detection\n");
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

