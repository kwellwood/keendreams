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

// KD_DEMO.C

#include "kd_def.h"

#pragma	hdrstop

#define RLETAG	0xABCD

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


/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

//===========================================================================

/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame (void)
{
	word	i;

	gamestate.worldx = 0;		// spawn keen at starting spot

	gamestate.mapon = 0;
	gamestate.score = 0;
	gamestate.nextextra = 20000;
	gamestate.lives = 3;
	gamestate.keys = 0;
	gamestate.flowerpowers = gamestate.boobusbombs = 0;
	for (i = 0;i < GAMELEVELS;i++)
		gamestate.leveldone[i] = false;
	STAT_ValidateGame();
}

//===========================================================================

/*
=====================
=
= WaitOrKey
=
=====================
*/

int WaitOrKey (int vbls)
{
	while (vbls--)
	{
		IN_ReadControl(0,&c);		// get player input
		if (LastScan || c.button0 || c.button1)
		{
			IN_ClearKeysDown ();
			return 1;
		}
		VW_WaitVBL(1);
	}
	return 0;
}

//===========================================================================

/*
=====================
=
= GameOver
=
=====================
*/

void
GameOver (void)
{
	VW_InitDoubleBuffer ();
	US_CenterWindow (16,3);

	US_PrintCentered("Game Over!");

	VW_UpdateScreen ();
	IN_ClearKeysDown ();
	IN_Ack ();

}


//===========================================================================

/*
==================
=
= StatusWindow
=
==================
*/

void StatusWindow (void)
{
	word	x;

	// DEBUG - make this look better

	US_CenterWindow(22,7);
	US_CPrint("Status Window");

	WindowX += 8;
	WindowW -= 8;
	WindowY += 20;
	WindowH -= 20;
	PrintX = WindowX;
	PrintY = WindowY;

	VWB_DrawTile8(PrintX,PrintY,26);
	VWB_DrawTile8(PrintX + 8,PrintY,27);
	PrintX += 24;
	US_PrintUnsigned(gamestate.lives);
	US_Print("\n");

	VWB_DrawTile8(PrintX,PrintY,32);
	VWB_DrawTile8(PrintX + 8,PrintY,33);
	VWB_DrawTile8(PrintX,PrintY + 8,34);
	VWB_DrawTile8(PrintX + 8,PrintY + 8,35);
	PrintX += 24;
	US_PrintUnsigned(gamestate.boobusbombs);
	US_Print("\n");

	WindowX += 50;
	WindowW -= 50;
	PrintX = WindowX;
	PrintY = WindowY;

	fontcolor = F_FIRSTCOLOR;
	US_Print("Next ");
	fontcolor = F_BLACK;
	x = PrintX;
	VWB_DrawTile8(PrintX,PrintY,26);
	VWB_DrawTile8(PrintX + 8,PrintY,27);
	PrintX += 24;
	US_PrintUnsigned(gamestate.nextextra);
	US_Print("\n");

	PrintX = x;
	VWB_DrawTile8(PrintX,PrintY,24);
	VWB_DrawTile8(PrintX + 8,PrintY,25);
	PrintX += 24;
	US_PrintUnsigned(gamestate.keys);
	US_Print("\n");

	// DEBUG - add flower powers (#36)

	VW_UpdateScreen();
	IN_Ack();
}

boolean
SaveGame(int file)
{
	word	i,size,compressed,expanded;
	objtype	*o;
	memptr	bigbuffer;

	if (!CA_FarWrite(file,(void far *)&gamestate,sizeof(gamestate)))
		return(false);

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i++)	// Write all three planes of the map
	{
//
// leave a word at start of compressed data for compressed length
//
		compressed = CA_RLEWCompress ((word *)mapsegs[i]
			,expanded,((word *)bigbuffer)+1,RLETAG);

		WriteU16(file, compressed);

		if (!CA_FarWrite(file,(void *)bigbuffer,compressed) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}
	}

	for (o = player;o;o = o->next)
		WriteObjStruct(file, o);

	MM_FreePtr (&bigbuffer);
	return(true);
}


boolean
LoadGame(int file)
{
	word	i,j,size;
	objtype	*o;
	int orgx,orgy;
	objtype		*prev,*next,*followed;
	uint16_t	compressed,expanded;
	memptr	bigbuffer;

	if (!CA_FarRead(file,(void far *)&gamestate,sizeof(gamestate)))
		return(false);

// drop down a cache level and mark everything, so when the option screen
// is exited it will be cached

	ca_levelbit >>= 1;
	ca_levelnum--;

	SetupGameLevel (false);		// load in and cache the base old level
	titleptr[ca_levelnum] = levelnames[mapon];

	ca_levelbit <<= 1;
	ca_levelnum ++;

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i++)	// Read all three planes of the map
	{
		compressed = ReadU16(file);
		if (!CA_FarRead(file,(void far *)bigbuffer,compressed) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		CA_RLEWexpand ((word *)bigbuffer,
			(word *)mapsegs[i],expanded,RLETAG);
	}

	MM_FreePtr (&bigbuffer);

	// Read the object list back in - assumes at least one object in list

	InitObjArray ();
	new = player;
	prev = new->prev;
	next = new->next;
	ReadObjStruct(file, new);
	new->prev = prev;
	new->next = next;
	new->needtoreact = true;
	new->sprite = NULL;
	new = scoreobj;
	while (true)
	{
		prev = new->prev;
		next = new->next;
		ReadObjStruct(file, new);
		followed = new->next;
		new->prev = prev;
		new->next = next;
		new->needtoreact = true;
		new->sprite = NULL;

		if (followed)
			GetNewObj (false);
		else
			break;
	}

	*((uint32_t *)&(scoreobj->temp1)) = -1;		// force score to be updated
	scoreobj->temp3 = -1;			// and flower power
	scoreobj->temp4 = -1;			// and lives

	// A loaded game is not valid for whole-game achievements,
	// or per-map achievements on the loaded map.
	STAT_Invalidate();

	return(true);
}

void
ResetGame(void)
{
	NewGame ();

	ca_levelnum--;
	CA_ClearMarks();
	titleptr[ca_levelnum] = NULL;		// don't reload old level
	ca_levelnum++;
}

#if FRILLS
void
TEDDeath(void)
{
	ShutdownId();
	execlp("TED5.EXE","TED5.EXE","/LAUNCH","KDREAMS",NULL);
}
#endif

static boolean
MoveTitleTo(int offset)
{
	boolean		done;
	int			dir,
				chunk,
				move;
	longword	lasttime,delay;

	if (offset < originxglobal)
		dir = -1;
	else
		dir = +1;

	chunk = dir * PIXGLOBAL;

	done = false;
	delay = 1;
	while (!done)
	{
		lasttime = SD_GetTimeCount();
		move = delay * chunk;
		if (chunk < 0)
			done = originxglobal + move <= offset;
		else
			done = originxglobal + move >= offset;
		if (!done)
		{
			RF_Scroll(move,0);
			RF_Refresh();
		}
		if (IN_IsUserInput())
			return(true);
		delay = SD_GetTimeCount() - lasttime;
	}
	if (originxglobal != offset)
	{
		RF_Scroll(offset - originxglobal,0);
		RF_Refresh();
	}
	return(false);
}

static boolean
Wait(longword time)
{
	time += SD_GetTimeCount();
	while ((SD_GetTimeCount() < time) && (!IN_IsUserInput()))
	{
		if (!(SD_GetTimeCount() % MINTICS))
			RF_Refresh();
	}
	return(IN_IsUserInput());
}

static boolean
ShowText(int offset,WindowRec *wr,char *s)
{
	if (MoveTitleTo(offset))
		return(true);

	US_RestoreWindow(wr);
	US_CPrint(s);
	VW_UpdateScreen();

	if (Wait(TickBase * 5))
		return(true);

	US_RestoreWindow(wr);
	US_CPrint(s);
	VW_UpdateScreen();
	return(false);
}

/*
=====================
=
= DemoLoop
=
=====================
*/

void
DemoLoop (void)
{
	char		*s;
	word		move;
	longword	lasttime;
	char *FileName1;
	struct Shape FileShape1;
#if CREDITS
	char *FileName2;
	struct Shape FileShape2;
	char *FileName3;
	struct Shape FileShape3;
#endif
	//struct ffblk ffblk;
	WindowRec	mywin;
	int bufsave	= bufferofs;
	int dissave	= displayofs;


#if FRILLS
//
// check for launch from ted
//
	if (tedlevel)
	{
		NewGame();
		gamestate.mapon = tedlevelnum;
		GameLoop();
		TEDDeath();
	}
#endif

//
// demo loop
//
	US_SetLoadSaveHooks(LoadGame,SaveGame,ResetGame);
	restartgame = gd_Continue;

	//if (findfirst("KDREAMS.CMP", &ffblk, 0) == -1)
	//	Quit("Couldn't find KDREAMS.CMP");

	while (true)
	{

		boolean titlever = fakecga;
		loadedgame = false;

		FileName1 = fakecga?"TITLECGA.LBM":"TITLESCR.LBM";
		if (LoadLIBShape("KDREAMS.CMP", FileName1, &FileShape1))
			Quit("Can't load TITLE SCREEN");
#if CREDITS
		FileName2 = "CREDITS1.LBM";
		if (LoadLIBShape("KDREAMS.CMP", FileName2, &FileShape2))
			Quit("Can't load CREDITS SCREEN 1");
		FileName3 = "CREDITS2.LBM";
		if (LoadLIBShape("KDREAMS.CMP", FileName3, &FileShape3))
			Quit("Can't load CREDITS SCREEN 3");
#endif

		while (!restartgame && !loadedgame)
		{

			VW_InitDoubleBuffer();
			IN_ClearKeysDown();

			while (true)
			{

				VW_SetScreen(0, 0);
				MoveGfxDst(0, 200);
				if (titlever != fakecga)
				{
					FileName1 = fakecga?"TITLECGA.LBM":"TITLESCR.LBM";
					if (LoadLIBShape("KDREAMS.CMP", FileName1, &FileShape1))
						Quit("Can't load TITLE SCREEN");
				}
				UnpackEGAShapeToScreen(&FileShape1, 0, 0);
				VW_ScreenToScreen (64*8*200,0,40*8,200);
				VW_GL_UpdateGLBuffer();
				VW_GL_Present();

#if CREDITS
				if (IN_UserInput(TickBase * 8, false))
					break;
#else
				if (IN_UserInput(TickBase * 4, false))
					break;
#endif

#if CREDITS
				if (!fakecga)
				{
					MoveGfxDst(0, 200);
					UnpackEGAShapeToScreen(&FileShape2, 0, 0);
					VW_ScreenToScreen (64*8*200,0,40*8,200);
					VW_GL_UpdateGLBuffer();
					VW_GL_Present();

					if (IN_UserInput(TickBase * 7, false))
						break;

					UnpackEGAShapeToScreen(&FileShape3, 0, 0);
					VW_ScreenToScreen (64*8*200,0,40*8,200);
					VW_GL_UpdateGLBuffer();
					VW_GL_Present();

					if (IN_UserInput(TickBase * 7, false))
						break;
				}
#else
				MoveGfxDst(0, 200);
				UnpackEGAShapeToScreen(&FileShape1, 0, 0);
				VW_ScreenToScreen (64*8*200,0,40*8,200);
				VW_GL_UpdateGLBuffer();
				VW_GL_Present();

				if (IN_UserInput(TickBase * 3, false))
					break;
#endif

				displayofs = 0;
				VWB_Bar(0,0,320,200,FIRSTCOLOR);
//				US_DisplayHighScores(-1);
				US_DisplayLeaderboards();
				VW_GL_UpdateGLBuffer();
				VW_GL_Present();

				if (IN_UserInput(TickBase * 6, false))
					break;

			}

			bufferofs = bufsave;
			displayofs = dissave;

			VW_FixRefreshBuffer();
			US_ControlPanel ();
		}

		if (!loadedgame)
			NewGame();

		FreeShape(&FileShape1);
#if CREDITS
		FreeShape(&FileShape2);
#endif
		GameLoop();
	}
}
