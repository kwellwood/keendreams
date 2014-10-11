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

// ID_VW.H

#ifndef __TYPES__
#include "id_types.h"
#endif

#ifndef __ID_MM__
#include "id_mm.h"
#endif

#ifndef __ID_GLOB__
#include "id_glob.h"
#endif

#define __ID_VW__

//===========================================================================

#define VW_MMAP_RINGBUFFER


#define	G_P_SHIFT		4	// global >> ?? = pixels

#if GRMODE == EGAGR
#define	SCREENWIDTH		64*8
#define CHARWIDTH		8
#define TILEWIDTH		16
#endif
#if GRMODE == CGAGR
#define	SCREENWIDTH		128
#define CHARWIDTH		2
#define TILEWIDTH		4
#endif

#define VIRTUALHEIGHT	300
#define	VIRTUALWIDTH	512


#if GRMODE == CGAGR

#define	MAXSHIFTS		1

#define WHITE			3			// graphics mode independant colors
#define BLACK			0
#define FIRSTCOLOR		1
#define SECONDCOLOR		2
#define F_WHITE			0			// for XOR font drawing
#define F_BLACK			3
#define F_FIRSTCOLOR	2
#define F_SECONDCOLOR	1

#endif

#if GRMODE == EGAGR

#define	MAXSHIFTS		8

#define WHITE			15			// graphics mode independant colors
#define BLACK			0
#define FIRSTCOLOR		1
#define SECONDCOLOR		12
#define F_WHITE			0			// for XOR font drawing
#define F_BLACK			15
#define F_FIRSTCOLOR	14
#define F_SECONDCOLOR	3

#endif

//===========================================================================


#define SC_INDEX	0x3C4
#define SC_RESET	0
#define SC_CLOCK	1
#define SC_MAPMASK	2
#define SC_CHARMAP	3
#define SC_MEMMODE	4

#define CRTC_INDEX	0x3D4
#define CRTC_H_TOTAL	0
#define CRTC_H_DISPEND	1
#define CRTC_H_BLANK	2
#define CRTC_H_ENDBLANK	3
#define CRTC_H_RETRACE	4
#define CRTC_H_ENDRETRACE 5
#define CRTC_V_TOTAL	6
#define CRTC_OVERFLOW	7
#define CRTC_ROWSCAN	8
#define CRTC_MAXSCANLINE 9
#define CRTC_CURSORSTART 10
#define CRTC_CURSOREND	11
#define CRTC_STARTHIGH	12
#define CRTC_STARTLOW	13
#define CRTC_CURSORHIGH	14
#define CRTC_CURSORLOW	15
#define CRTC_V_RETRACE	16
#define CRTC_V_ENDRETRACE 17
#define CRTC_V_DISPEND	18
#define CRTC_OFFSET	19
#define CRTC_UNDERLINE	20
#define CRTC_V_BLANK	21
#define CRTC_V_ENDBLANK	22
#define CRTC_MODE	23
#define CRTC_LINECOMPARE 24


#define GC_INDEX	0x3CE
#define GC_SETRESET	0
#define GC_ENABLESETRESET 1
#define GC_COLORCOMPARE	2
#define GC_DATAROTATE	3
#define GC_READMAP	4
#define GC_MODE		5
#define GC_MISCELLANEOUS 6
#define GC_COLORDONTCARE 7
#define GC_BITMASK	8

#define ATR_INDEX	0x3c0
#define ATR_MODE	16
#define ATR_OVERSCAN	17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN	19
#define ATR_COLORSELECT	20


//===========================================================================

typedef enum {NOcard,MDAcard,CGAcard,EGAcard,MCGAcard,VGAcard,
		  HGCcard=0x80,HGCPcard,HICcard} cardtype;

typedef struct __attribute__((__packed__))
{
  int16_t	width,
		height,
		orgx,orgy,
		xl,yl,xh,yh,
		shifts;
} spritetabletype;

typedef	struct __attribute__((__packed__))
{
	uint16_t	sourceoffset[MAXSHIFTS];
	uint16_t	planesize[MAXSHIFTS];
	uint16_t	width[MAXSHIFTS];
	byte		data[];
} spritetype;		// the memptr for each sprite points to this

typedef struct __attribute__((__packed__))
{
	int16_t width,height;
} pictabletype;


typedef struct __attribute__((__packed__))
{
	int16_t height;
	int16_t location[256];
	char width[256];
} fontstruct;


typedef enum {CGAgr,EGAgr,VGAgr} grtype;

//===========================================================================

extern	cardtype	videocard;		// set by VW_Startup
extern	grtype		grmode;			// CGAgr, EGAgr, VGAgr

extern	unsigned	bufferofs;		// hidden port to draw to before displaying
extern	unsigned	displayofs;		// origin of port on visable screen
extern	unsigned	panx,pany;		// panning adjustments inside port in pixels
extern	unsigned	pansx,pansy;
extern	unsigned	panadjust;		// panx/pany adjusted by screen resolution
#define VW_VIDEOMEM_SIZE (4096 * 128)		// This must be a multiple of PAGE_SIZE
extern uint8_t *vw_videomem;

extern	unsigned	screenseg;		// normally 0xa000 or buffer segment

extern	unsigned	linewidth;
extern	unsigned	ylookup[VIRTUALHEIGHT];

extern	boolean		screenfaded;

extern	pictabletype	*pictable;
extern	pictabletype	*picmtable;
extern	spritetabletype *spritetable;

extern	int			px,py;
extern	byte		pdrawmode,fontcolor;

//
// asm globals
//

extern	word 		**shifttabletable;


//===========================================================================

extern unsigned lastdrawnbuffer;
extern unsigned lastdrawnpan;

void VW_GL_UpdateGLBuffer();

//===========================================================================

void	VW_Startup (void);
void	VW_Shutdown (void);

cardtype	VW_VideoID (void);

//
// EGA hardware routines
//

// XXX: See Abrash's Black Book for details as to what was here
#define EGAWRITEMODE(x) 
#define EGABITMASK(x) 
#define EGAMAPMASK(x) 

void 	VW_SetLineWidth(int width);
void 	VW_SetScreen (unsigned CRTC, unsigned pelpan);

void	VW_SetScreenMode (int grmode);
void	VW_ClearVideo (int color);
void	VW_WaitVBL (int number);

void	VW_ColorBorder (int color);
void	VW_SetDefaultColors(void);
void	VW_FadeOut(void);
void	VW_FadeIn(void);
void	VW_FadeUp(void);
void	VW_FadeDown(void);

//
// block primitives
//

void VW_MaskBlock(memptr segm,unsigned ofs,unsigned dest,
	unsigned wide,unsigned height,unsigned planesize);
void VW_MemToScreen(memptr source,unsigned dest,unsigned width,unsigned height);
void VW_ScreenToMem(unsigned source,memptr dest,unsigned width,unsigned height);
void VW_ScreenToScreen(unsigned source,unsigned dest,unsigned width,unsigned height);
void VW_RawMemToScreen(memptr source,unsigned dest,unsigned width,unsigned height);
void VW_RawScreenToMem(unsigned source,memptr dest,unsigned width,unsigned height);


//
// block addressable routines
//

void VW_DrawTile8(unsigned x, unsigned y, unsigned tile);

#if GRMODE == EGAGR

#define VW_DrawTile8M(x,y,t) \
	VW_MaskBlock(grsegs[STARTTILE8M],(t)*40,bufferofs+ylookup[y]+(x),8,8,8)
#define VW_DrawTile16(x,y,t) \
	VW_MemToScreen(grsegs[STARTTILE16+t],bufferofs+ylookup[y]+(x),16,16)
#define VW_DrawTile16M(x,y,t) \
	VW_MaskBlock(grsegs[STARTTILE16M],(t)*160,bufferofs+ylookup[y]+(x),16,16,32)

#endif

#if GRMODE == CGAGR

#define VW_DrawTile8M(x,y,t) \
	VW_MaskBlock(grsegs[STARTTILE8M],(t)*32,bufferofs+ylookup[y]+(x),2,8,16)
#define VW_DrawTile16(x,y,t) \
	VW_MemToScreen(grsegs[STARTTILE16+t],bufferofs+ylookup[y]+(x),4,16)
#define VW_DrawTile16M(x,y,t) \
	VW_MaskBlock(grsegs[STARTTILE16M],(t)*128,bufferofs+ylookup[y]+(x),4,16,64)

#endif

void VW_DrawPic(unsigned x, unsigned y, unsigned chunknum);
void VW_DrawMPic(unsigned x, unsigned y, unsigned chunknum);

//
// pixel addressable routines
//
void	VW_MeasurePropString (char far *string, word *width, word *height);
void	VW_MeasureMPropString  (char far *string, word *width, word *height);

void VW_DrawPropString (char far *string);
void VW_DrawMPropString (char far *string);
void VW_DrawSprite(int x, int y, unsigned sprite);
void VW_Plot(unsigned x, unsigned y, unsigned color);
void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color);
void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color);
void VW_Bar (unsigned x, unsigned y, unsigned width, unsigned height,
	unsigned color);

//===========================================================================

//
// Double buffer management routines
//

void VW_InitDoubleBuffer (void);
void VW_FixRefreshBuffer (void);
int	 VW_MarkUpdateBlock (int x1, int y1, int x2, int y2);
void VW_UpdateScreen (void);
void VW_CGAFullUpdate (void);

//
// cursor
//

void VW_ShowCursor (void);
void VW_HideCursor (void);
void VW_MoveCursor (int x, int y);
void VW_SetCursor (int spritenum);
extern boolean	cursorhw;			// Are we using a hardware cursor?
extern int			cursorvisible;

//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8 (int x, int y, int tile);
void VWB_DrawTile8M (int x, int y, int tile);
void VWB_DrawTile16 (int x, int y, int tile);
void VWB_DrawTile16M (int x, int y, int tile);
void VWB_DrawPic (int x, int y, int chunknum);
void VWB_DrawMPic(int x, int y, int chunknum);
void VWB_Bar (int x, int y, int width, int height, int color);

void VWB_DrawPropString	 (char far *string);
void VWB_DrawMPropString (char far *string);
void VWB_DrawSprite (int x, int y, int chunknum);
void VWB_Plot (int x, int y, int color);
void VWB_Hlin (int x1, int x2, int y, int color);
void VWB_Vlin (int y1, int y2, int x, int color);

//===========================================================================

//
// Video memory mapping functions
//

void VWL_SetupVideoMemory();

//===========================================================================

//
// Omnispeak conversion utility functions.
//

void VW_UnmaskedToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h);
void VW_UnmaskedSubRectToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int planesize, int planepitch);
void VW_MaskedToRGBA(void *src,void *dest, int x, int y, int pitch, int w, int h);
void VW_MaskedToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h);
void VW_MaskedBlitWPlaneSizeToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int planesize);
void VW_1bppXorWithPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int colour);

//===========================================================================

//
// GL blitting
//

void VW_GL_Init();
void VW_GL_UpdateLineWidth();
void VW_GL_UpdateGLBuffer();
void VW_GL_Present();

//===========================================================================
