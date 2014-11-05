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

// ID_VW.C

#include "id_heads.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define VIEWWIDTH		40

#define PIXTOBLOCK		4		// 16 pixels to an update block

#if GRMODE == EGAGR
#define SCREENXMASK		(~7)
#define SCREENXPLUS		(7)
#define SCREENXDIV		(1)
#endif

#if GRMODE == CGAGR
#define SCREENXMASK		(~3)
#define SCREENXDIV		(4)
#endif
/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

cardtype	videocard;		// set by VW_Startup
grtype		grmode;			// CGAgr, EGAgr, VGAgr
SDL_Window	*window;
SDL_GLContext	glcontext;

unsigned	bufferofs = 0;		// hidden area to draw to before displaying
unsigned	displayofs = 0;		// origin of the visable screen
unsigned	panx,pany;		// panning adjustments inside port in pixels
unsigned	pansx,pansy;	// panning adjustments inside port in screen
							// block limited pixel values (ie 0/8 for ega x)
unsigned	panadjust;		// panx/pany adjusted by screen resolution

unsigned	screenseg;		// normally 0xa000 / 0xb800
unsigned	linewidth;
unsigned	ylookup[VIRTUALHEIGHT];

boolean		screenfaded;

pictabletype	*pictable;
pictabletype	*picmtable;
spritetabletype *spritetable;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

void	VWL_MeasureString (char *string, word *width, word *height,
		fontstruct *font);
void 	VWL_DrawCursor (void);
void 	VWL_EraseCursor (void);
void 	VWL_DBSetup (void);
void	VWL_UpdateScreenBlocks (void);


int			bordercolor;
int			cursorvisible;
int			cursornumber,cursorwidth,cursorheight,cursorx,cursory;
memptr		cursorsave;
unsigned	cursorspot;
boolean		cursorhw = true;			// Are we using a hardware cursor?

extern	unsigned	bufferwidth,bufferheight;	// used by font drawing stuff

//===========================================================================


/*
=======================
=
= VW_Startup
=
=======================
*/

static	char *ParmStrings[] = {"HIDDENCARD", "FULLSCREEN", ""};

void	VW_Startup (void)
{
	int i;
	int fullscreen = false;
	SDL_Init(SDL_INIT_VIDEO);


	videocard = 0;

	for (i = 1;i < _argc;i++)
	{
		int p = US_CheckParm(_argv[i],ParmStrings);
		if (p == 0)
		{
			videocard = EGAcard;
		}
		else if (p == 1)
		{
			fullscreen = true;
		}
	}

	if (!videocard)
		videocard = VW_VideoID ();

#if GRMODE == EGAGR
	grmode = EGAGR;
	if (videocard != EGAcard && videocard != VGAcard)
Quit ("Improper video card!  If you really have an EGA/VGA card that I am not \n"
	  "detecting, use the -HIDDENCARD command line parameter!");
	EGAWRITEMODE(0);
#endif

#if GRMODE == CGAGR
	grmode = CGAGR;
	if (videocard < CGAcard || videocard > VGAcard)
Quit ("Improper video card!  If you really have a CGA card that I am not \n"
	  "detecting, use the -HIDDENCARD command line parameter!");
	MM_GetPtr (&(memptr)screenseg,0x10000l);	// grab 64k for floating screen
#endif

	cursorvisible = 0;
	
	VWL_SetupVideoMemory();
	window = SDL_CreateWindow("Keen Dreams", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(window, 320, 200);
	if (fullscreen)
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	glcontext = SDL_GL_CreateContext(window);
	glewInit();
	glViewport(0,0,320,200);
	VW_GL_Init();
}

//===========================================================================

/*
=======================
=
= VW_Shutdown
=
=======================
*/

void	VW_Shutdown (void)
{
	VW_SetScreenMode (TEXTGR);
#if GRMODE == EGAGR
	VW_SetLineWidth (80);
#endif
}

//===========================================================================

/*
========================
=
= VW_SetScreenMode
= Call BIOS to set TEXT / CGAgr / EGAgr / VGAgr
=
========================
*/

void VW_SetScreenMode (int grmode)
{
	switch (grmode)
	{
	  case TEXTGR:  
		  screenseg=0xb000;
		  break;
	  case CGAGR: 
		  // screenseg is actually a main mem buffer
		  break;
	  case EGAGR: 
		  screenseg=0xa000;
		  break;
#ifdef VGAGAME
	  case VGAGR:{
		  char extern VGAPAL;	// deluxepaint vga pallet .OBJ file
		  void far *vgapal = &VGAPAL;
		  SetCool256 ();		// custom 256 color mode
		  screenseg=0xa000;
		  _ES = FP_SEG(vgapal);
		  _DX = FP_OFF(vgapal);
		  _BX = 0;
		  _CX = 0x100;
		  _AX = 0x1012;
		  geninterrupt(0x10);			// set the deluxepaint pallet

		  break;
#endif
	}
	VW_SetLineWidth(SCREENWIDTH);
}

/*
=============================================================================

							SCREEN FADES

=============================================================================
*/

char colors[7][17]=
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,0},
 {0,0,0,0,0,0,0,0,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0},
 {0,1,2,3,4,5,6,7,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0},
 {0,1,2,3,4,5,6,7,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0},
 {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f}};


void VW_ColorBorder (int color)
{
	// XXX: Do interupt.
	bordercolor = color;
}

void VW_SetDefaultColors(void)
{
#if GRMODE == EGAGR
	colors[3][16] = bordercolor;
	VW_GL_SetEGAPalette(colors[3]);
	screenfaded = false;
#endif
}


void VW_FadeOut(void)
{
#if GRMODE == EGAGR
	int i;

	for (i=3;i>=0;i--)
	{
		colors[i][16] = bordercolor;
		VW_GL_SetEGAPalette(colors[i]);
		VW_WaitVBL(6);
	}
	screenfaded = true;
#endif
}


void VW_FadeIn(void)
{
#if GRMODE == EGAGR
	int i;

	for (i=0;i<4;i++)
	{
		colors[i][16] = bordercolor;
		VW_GL_SetEGAPalette(colors[i]);
		VW_WaitVBL(6);
	}
	screenfaded = false;
#endif
}

void VW_FadeUp(void)
{
#if GRMODE == EGAGR
	int i;

	for (i=3;i<6;i++)
	{
		colors[i][16] = bordercolor;
		VW_GL_SetEGAPalette(colors[i]);
		VW_WaitVBL(6);
	}
	screenfaded = true;
#endif
}

void VW_FadeDown(void)
{
#if GRMODE == EGAGR
	int i;

	for (i=5;i>2;i--)
	{
		colors[i][16] = bordercolor;
		VW_GL_SetEGAPalette(colors[i]);
		VW_WaitVBL(6);
	}
	screenfaded = false;
#endif
}



//===========================================================================

/*
====================
=
= VW_SetLineWidth
=
= Must be an even number of bytes
=
====================
*/

void VW_SetLineWidth (int width)
{
  int i,offset;

#if GRMODE == EGAGR
//
// set wide virtual screen
//
#endif

//
// set up lookup tables
//
  linewidth = width;

  offset = 0;

  for (i=0;i<VIRTUALHEIGHT;i++)
  {
	ylookup[i]=offset;
	offset += width;
  }
  
  VW_GL_UpdateLineWidth();
}

//===========================================================================

/*
====================
=
= VW_ClearVideo
=
====================
*/

void	VW_ClearVideo (int color)
{
#if GRMODE == EGAGR
	EGAWRITEMODE(2);
	EGAMAPMASK(15);
#endif
// XXX memset vmem 0xffff bytes to zero
#if GRMODE == EGAGR
	EGAWRITEMODE(0);
#endif
}

//===========================================================================

#if NUMPICS>0

/*
====================
=
= VW_DrawPic
=
= X in bytes, y in pixels, chunknum is the #defined picnum
=
====================
*/

void VW_DrawPic(unsigned x, unsigned y, unsigned chunknum)
{
	int	picnum = chunknum - STARTPICS;
	memptr source;
	unsigned dest,width,height;

	source = grsegs[chunknum];
	dest = ylookup[y]+x+bufferofs;
	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VW_MemToScreen(source,dest,width,height);
}

#endif

#if NUMPICM>0

/*
====================
=
= VW_DrawMPic
=
= X in bytes, y in pixels, chunknum is the #defined picnum
=
====================
*/

void VW_DrawMPic(unsigned x, unsigned y, unsigned chunknum)
{
	int	picnum = chunknum - STARTPICM;
	memptr source;
	unsigned dest,width,height;

	source = grsegs[chunknum];
	dest = ylookup[y]+x+bufferofs;
	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VW_MaskBlock(source,0,dest,width,height,width*height);
}

#endif

//===========================================================================

#if NUMSPRITES>0

/*
====================
=
= VW_DrawSprite
=
= X and Y in pixels, it will match the closest shift possible
=
= To do:
= Add vertical clipping!
= Make the shifts act as center points, rather than break points
=
====================
*/

void VW_DrawSprite(int x, int y, unsigned chunknum)
{
	spritetabletype far *spr;
	unsigned	dest,shift;

	spr = &spritetable[chunknum-STARTSPRITES];

	y+=spr->orgy>>G_P_SHIFT;
	x+=spr->orgx>>G_P_SHIFT;

#if GRMODE == EGAGR
	shift = 0;// XXX (x&7)/2;
#endif
#if GRMODE == CGAGR
	shift = 0;
#endif

	dest = bufferofs + ylookup[y];
	if (x>=0)
		dest += x/SCREENXDIV;
	else
		dest += (x+1)/SCREENXDIV;

	VW_RawBlitToScreen (grsegs[chunknum],dest,
		spr->width*8,spr->height);
}

#endif


/*
==================
=
= VW_Hlin
=
==================
*/


#if GRMODE == EGAGR

unsigned char leftmask[8] = {0xff,0x7f,0x3f,0x1f,0xf,7,3,1};
unsigned char rightmask[8] = {0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff};

void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color)
{
	uint8_t *screen = &vw_videomem[bufferofs];
	unsigned x;
	for(x = xl; x <= xh; ++x)
	{
		screen[(y*linewidth+x)%VW_VIDEOMEM_SIZE] = color;
	}
	
}
#endif


#if GRMODE == CGAGR

unsigned char pixmask[4] = {0xc0,0x30,0x0c,0x03};
unsigned char leftmask[4] = {0xff,0x3f,0x0f,0x03};
unsigned char rightmask[4] = {0xc0,0xf0,0xfc,0xff};
unsigned char colorbyte[4] = {0,0x55,0xaa,0xff};

//
// could be optimized for rep stosw
//
void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color)
{
	unsigned dest,xlb,xhb,mid;
	byte maskleft,maskright;

	color = colorbyte[color];	// expand 2 color bits to 8

	xlb=xl/4;
	xhb=xh/4;

	maskleft = leftmask[xl&3];
	maskright = rightmask[xh&3];

	mid = xhb-xlb-1;
	dest = bufferofs+ylookup[y]+xlb;

	if (xlb==xhb)
	{
	//
	// entire line is in one byte
	//
		maskleft&=maskright;

		return;
	}
}
#endif


/*
==================
=
= VW_Bar
=
= Pixel addressable block fill routine
=
==================
*/
void VW_Bar (unsigned x, unsigned y, unsigned width, unsigned height,
	unsigned color)
{
	unsigned xh = x+width-1;

	while (height--)
		VW_Hlin (x,xh,y++,color);
}

//==========================================================================

/*
==================
=
= VW_MeasureString
=
==================
*/

#if NUMFONT+NUMFONTM>0
void
VWL_MeasureString (char far *string, word *width, word *height, fontstruct _seg *font)
{
	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*string];		// proportional width
}

void	VW_MeasurePropString (char far *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct _seg *)grsegs[STARTFONT]);
}

void	VW_MeasureMPropString  (char far *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct _seg *)grsegs[STARTFONTM]);
}


#endif


/*
=============================================================================

							CGA stuff

=============================================================================
*/

#if GRMODE == CGAGR

#define CGACRTCWIDTH	40

/*
==========================
=
= VW_CGAFullUpdate
=
==========================
*/

void VW_CGAFullUpdate (void)
{
	byte	*update;
	boolean	halftile;
	unsigned	x,y,middlerows,middlecollumns;

	displayofs = bufferofs+panadjust;

	// XXX fascinating CGA code, read in orig cource

	updateptr = baseupdateptr;
	*(unsigned *)(updateptr + UPDATEWIDE*PORTTILESHIGH) = UPDATETERMINATE;
}


#endif

/*
=============================================================================

					   CURSOR ROUTINES

These only work in the context of the double buffered update routines

=============================================================================
*/

/*
====================
=
= VWL_DrawCursor
=
= Background saves, then draws the cursor at cursorspot
=
====================
*/

void VWL_DrawCursor (void)
{
	if (!cursorhw)
	{
		cursorspot = bufferofs + ylookup[cursory+pansy]+(cursorx+pansx)/SCREENXDIV;
		VW_RawScreenToMem(cursorspot,cursorsave,cursorwidth,cursorheight);
		VWB_DrawSprite(cursorx,cursory,cursornumber);
	}
}


//==========================================================================


/*
====================
=
= VWL_EraseCursor
=
====================
*/

void VWL_EraseCursor (void)
{
	if (!cursorhw)
	{
		VW_RawMemToScreen(cursorsave,cursorspot,cursorwidth,cursorheight);
		VW_MarkUpdateBlock ((cursorx+pansx)&SCREENXMASK,cursory+pansy,
			( (cursorx+pansx)&SCREENXMASK)+cursorwidth,
			cursory+pansy+cursorheight);
	}
}


//==========================================================================


/*
====================
=
= VW_ShowCursor
=
====================
*/

void VW_ShowCursor (void)
{
	cursorvisible++;
	if (cursorhw)
	{
		SDL_ShowCursor(cursorvisible);
	}
	SDL_SetRelativeMouseMode(!cursorvisible);
}


//==========================================================================

/*
====================
=
= VW_HideCursor
=
====================
*/

void VW_HideCursor (void)
{
	cursorvisible--;
	if (cursorhw)
	{
		SDL_ShowCursor(cursorvisible);
	}
	SDL_SetRelativeMouseMode(!cursorvisible);
}

//==========================================================================

/*
====================
=
= VW_MoveCursor
=
====================
*/

void VW_MoveCursor (int x, int y)
{
	if (cursorhw)
	{
		int vx, vy, vw, vh;
		VW_GL_GetViewport(&vx, &vy, &vw, &vh);
		SDL_WarpMouseInWindow(window, x*vw/320+vx, y*vh/200+vx);
	}
	cursorx = x;
	cursory = y;
}

//==========================================================================

/*
====================
=
= VW_SetCursor
=
= Load in a sprite to be used as a cursor, and allocate background save space
=
====================
*/

void VW_SetCursor (int spritenum)
{
	if (cursornumber)
	{
		MM_SetLock (&grsegs[cursornumber],false);
		if (!cursorhw)
			MM_FreePtr (&cursorsave);
	}

	cursornumber = spritenum;

	CA_CacheGrChunk (spritenum);
	MM_SetLock (&grsegs[cursornumber],true);

	cursorwidth = spritetable[spritenum-STARTSPRITES].width*8;
	cursorheight = spritetable[spritenum-STARTSPRITES].height;
	int cursorscalex = 1, cursorscaley = 1;
	int winw, winh;
	VW_GL_GetViewport(0,0, &winw, &winh);
	cursorscalex = winw/320;
	cursorscaley = winh/200;
	// Intel graphics hw fails on large cursors.
	if (cursorscalex > 2) cursorscalex = 2;
	if (cursorscaley > 2) cursorscaley = 2;
	if (cursorhw)
	{
		SDL_Surface *mouseSurface = SDL_CreateRGBSurface(0, cursorwidth*cursorscalex, cursorheight*cursorscaley, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		SDL_LockSurface(mouseSurface);
		VW_PAL8ScaleToRGBA(((grsegs[cursornumber])), mouseSurface->pixels, cursorscalex, cursorscaley, mouseSurface->pitch, cursorwidth, cursorheight);
		SDL_UnlockSurface(mouseSurface);
		SDL_Cursor *mouseCursor = SDL_CreateColorCursor(mouseSurface, spritetable[cursornumber-STARTSPRITES].orgx*cursorscalex, spritetable[cursornumber-STARTSPRITES].orgy*cursorscaley);
		SDL_SetCursor(mouseCursor);
	}
	else
		MM_GetPtr (&cursorsave,cursorwidth*cursorheight);
}


/*
=============================================================================

				Double buffer management routines

=============================================================================
*/

/*
======================
=
= VW_InitDoubleBuffer
=
======================
*/

void VW_InitDoubleBuffer (void)
{
#if GRMODE == EGAGR
	VW_SetScreen (displayofs+panadjust,0);			// no pel pan
#endif
}


/*
======================
=
= VW_FixRefreshBuffer
=
= Copies the view page to the buffer page on page flipped refreshes to
= avoid a one frame shear around pop up windows
=
======================
*/

void VW_FixRefreshBuffer (void)
{
#if GRMODE == EGAGR
	VW_ScreenToScreen (displayofs,bufferofs,PORTTILESWIDE*4*CHARWIDTH,
		PORTTILESHIGH*16);
#endif
}


/*
======================
=
= VW_QuitDoubleBuffer
=
======================
*/

void VW_QuitDoubleBuffer (void)
{
}


/*
=======================
=
= VW_MarkUpdateBlock
=
= Takes a pixel bounded block and marks the tiles in bufferblocks
= Returns 0 if the entire block is off the buffer screen
=
=======================
*/

int VW_MarkUpdateBlock (int x1, int y1, int x2, int y2)
{
	int	x,y,xt1,yt1,xt2,yt2,nextline;
	byte *mark;

	xt1 = x1>>PIXTOBLOCK;
	yt1 = y1>>PIXTOBLOCK;

	xt2 = x2>>PIXTOBLOCK;
	yt2 = y2>>PIXTOBLOCK;

	if (xt1<0)
		xt1=0;
	else if (xt1>=UPDATEWIDE-1)
		return 0;

	if (yt1<0)
		yt1=0;
	else if (yt1>UPDATEHIGH)
		return 0;

	if (xt2<0)
		return 0;
	else if (xt2>=UPDATEWIDE-1)
		xt2 = UPDATEWIDE-2;

	if (yt2<0)
		return 0;
	else if (yt2>=UPDATEHIGH)
		yt2 = UPDATEHIGH-1;

	mark = updateptr + uwidthtable[yt1] + xt1;
	nextline = UPDATEWIDE - (xt2-xt1) - 1;

	for (y=yt1;y<=yt2;y++)
	{
		for (x=xt1;x<=xt2;x++)
			*mark++ = 1;			// this tile will need to be updated

		mark += nextline;
	}

	return 1;
}


/*
===========================
=
= VW_UpdateScreen
=
= Updates any changed areas of the double buffer and displays the cursor
=
===========================
*/

void VW_UpdateScreen (void)
{
	if (cursorvisible>0)
		VWL_DrawCursor();

#if GRMODE == EGAGR
	VWL_UpdateScreenBlocks();

	// XXX set displayofs panadjust

#endif
#if GRMODE == CGAGR
	VW_CGAFullUpdate();
#endif

	if (cursorvisible>0)
		VWL_EraseCursor();
	VW_GL_UpdateGLBuffer();
	VW_GL_Present();
}



void VWB_DrawTile8 (int x, int y, int tile)
{
	x+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x,y,(x)+7,y+7))
		VW_DrawTile8 (x/SCREENXDIV,y,tile);
}

void VWB_DrawTile8M (int x, int y, int tile)
{
	int xb;

	x+=pansx;
	y+=pansy;
	xb = x/SCREENXDIV; 			// use intermediate because VW_DT8M is macro
	if (VW_MarkUpdateBlock (x,y,(x)+7,y+7))
		VW_DrawTile8M (xb,y,tile);
}

void VWB_DrawTile16 (int x, int y, int tile)
{
	x+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x,y,(x)+15,y+15))
		VW_DrawTile16 (x/SCREENXDIV,y,tile);
}

void VWB_DrawTile16M (int x, int y, int tile)
{
	int xb;

	x+=pansx;
	y+=pansy;
	xb = x/SCREENXDIV;		// use intermediate because VW_DT16M is macro
	if (VW_MarkUpdateBlock (x,y,(x)+15,y+15))
		VW_DrawTile16M (xb,y,tile);
}

#if NUMPICS
void VWB_DrawPic (int x, int y, int chunknum)
{
// mostly copied from drawpic
	int	picnum = chunknum - STARTPICS;
	memptr source;
	unsigned dest,width,height;

	x+=pansx;
	y+=pansy;
	x/= SCREENXDIV;

	source = grsegs[chunknum];
	dest = ylookup[y]+x+bufferofs;
	width = pictable[picnum].width*8;
	height = pictable[picnum].height;

	if (VW_MarkUpdateBlock (x*SCREENXDIV,y,(x+width)*SCREENXDIV-1,y+height-1))
		VW_MemToScreen(source,dest,width,height);
}
#endif

#if NUMPICM>0
void VWB_DrawMPic(int x, int y, int chunknum)
{
// mostly copied from drawmpic
	int	picnum = chunknum - STARTPICM;
	memptr source;
	unsigned dest,width,height;

	x+=pansx;
	y+=pansy;
	x/=SCREENXDIV;

	source = grsegs[chunknum];
	dest = ylookup[y]+x+bufferofs;
	width = picmtable[picnum].width*8;
	height = picmtable[picnum].height;

	if (VW_MarkUpdateBlock (x*SCREENXDIV,y,(x+width)*SCREENXDIV-1,y+height-1))
		VW_MaskBlock(source,0,dest,width,height,width*height/8);
}
#endif


void VWB_Bar (int x, int y, int width, int height, int color)
{
	x+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x,y,x+width,y+height-1) )
		VW_Bar (x,y,width,height,color);
}


#if NUMFONT
void VWB_DrawPropString	 (char far *string)
{
	int x,y;
	x = px+pansx;
	y = py+pansy;
	VW_DrawPropString (string);
	VW_MarkUpdateBlock(0,0,320,200);
	//VW_MarkUpdateBlock(x,y,x+bufferwidth*8,y+bufferheight);
}
#endif


#if NUMFONTM
void VWB_DrawMPropString (char far *string)
{
	int x,y;
	x = px+pansx;
	y = py+pansy;
	VW_DrawMPropString (string);
	VW_MarkUpdateBlock(x,y,x+bufferwidth*8-1,y+bufferheight-1);
}
#endif

#if NUMSPRITES
void VWB_DrawSprite(int x, int y, int chunknum)
{
	spritetabletype far *spr;
	unsigned	dest,shift,width,height;

	x+=pansx;
	y+=pansy;

	spr = &spritetable[chunknum-STARTSPRITES];

	y+=spr->orgy>>G_P_SHIFT;
	x+=spr->orgx>>G_P_SHIFT;


#if GRMODE == EGAGR
	shift = 0;//(x&7)/2;
#endif
#if GRMODE == CGAGR
	shift = 0;
#endif

	dest = bufferofs + ylookup[y];
	if (x>=0)
		dest += x/SCREENXDIV;
	else
		dest += (x+1)/SCREENXDIV;

	width = spr->width * 8;
	height = spr->height;

	if (VW_MarkUpdateBlock (x,y,(x)+width-1
		,y+height-1))
		VW_RawBlitToScreen (grsegs[chunknum],dest,
			width,height);
}
#endif

void VWB_Plot (int x, int y, int color)
{
	x+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x,y,x,y))
		VW_Plot(x,y,color);
}

void VWB_Hlin (int x1, int x2, int y, int color)
{
	x1+=pansx;
	x2+=pansx;
	y+=pansy;
	if (VW_MarkUpdateBlock (x1,y,x2,y))
		VW_Hlin(x1,x2,y,color);
}

void VWB_Vlin (int y1, int y2, int x, int color)
{
	x+=pansx;
	y1+=pansy;
	y2+=pansy;
	if (VW_MarkUpdateBlock (x,y1,x,y2))
		VW_Vlin(y1,y2,x,color);
}


//===========================================================================
