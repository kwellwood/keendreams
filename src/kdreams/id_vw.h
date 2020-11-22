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

/*** REFKEEN - A few functions are now pointers (switchable based on game version),  ***/
/*** but mode-specific prototypes may also be declared as they can be used directly. ***/

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

#define	G_P_SHIFT		4	// global >> ?? = pixels

// REFKEEN - Now that GRMODE is a variable, use separate names for EGA and CGA
// definitions, while optionally maintaining variables in case
// a video mode-agonstic access to some value is desired

//#if GRMODE == EGAGR
#define	SCREENWIDTH_EGA		64
#define CHARWIDTH_EGA		1
#define TILEWIDTH_EGA		2
//#endif
//#if GRMODE == CGAGR
#define	SCREENWIDTH_CGA		128
#define CHARWIDTH_CGA		2
#define TILEWIDTH_CGA		4
//#endif
extern	id0_int_t	SCREENWIDTH, CHARWIDTH, TILEWIDTH;

#define VIRTUALHEIGHT	300
#define	VIRTUALWIDTH	512


// REFKEEN - Now that GRMODE is a variable, use separate names for EGA and CGA
// definitions, while optionally maintaining variables in case
// a video mode-agonstic access to some value is desired

//#if GRMODE == CGAGR

#define	MAXSHIFTS_CGA		1

#define WHITE_CGA			3			// graphics mode independant colors
#define BLACK_CGA			0
#define FIRSTCOLOR_CGA		1
#define SECONDCOLOR_CGA		2
#define F_WHITE_CGA			0			// for XOR font drawing
#define F_BLACK_CGA			3
#define F_FIRSTCOLOR_CGA	2
#define F_SECONDCOLOR_CGA	1

//#endif

//#if GRMODE == EGAGR

#define	MAXSHIFTS_EGA		8

#define WHITE_EGA			15			// graphics mode independant colors
#define BLACK_EGA			0
#define FIRSTCOLOR_EGA		1
#define SECONDCOLOR_EGA		12
#define F_WHITE_EGA			0			// for XOR font drawing
#define F_BLACK_EGA			15
#define F_FIRSTCOLOR_EGA	14
#define F_SECONDCOLOR_EGA	3

//#endif

extern	id0_int_t	WHITE, BLACK, FIRSTCOLOR, SECONDCOLOR, F_BLACK, F_FIRSTCOLOR, F_SECONDCOLOR;

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

// setting to 0 causes setscreen and waitvbl
// to skip waiting for VBL (for timing things)

#define WAITFORVBL 1

typedef enum {NOcard,MDAcard,CGAcard,EGAcard,MCGAcard,VGAcard,
		  HGCcard=0x80,HGCPcard,HICcard} cardtype;

#pragma pack(push, 1)

typedef struct
{
  id0_int_t	width,
	height,
	orgx,orgy,
	xl,yl,xh,yh,
	shifts;
} spritetabletype;

// REFKEEN - Split type based on MAXSHIFTS_CGA/MAXSHIFTS_EGA;
// Special routines may be required for accessing struct members
typedef	struct
{
	id0_unsigned_t	sourceoffset[MAXSHIFTS_EGA];
	id0_unsigned_t	planesize[MAXSHIFTS_EGA];
	id0_unsigned_t	width[MAXSHIFTS_EGA];
	id0_byte_t		data[];
} spritetype_ega;		// the memptr for each sprite points to this

typedef	struct
{
	id0_unsigned_t	sourceoffset[MAXSHIFTS_CGA];
	id0_unsigned_t	planesize[MAXSHIFTS_CGA];
	id0_unsigned_t	width[MAXSHIFTS_CGA];
	id0_byte_t		data[];
} spritetype_cga;		// the memptr for each sprite points to this

// REFKEEN - Add video mode-agnostic spritetype accessors (NEW DEFINITIONS)
#define VW_GetSpriteShiftSourceOffset(block, i) ((GRMODE == EGAGR) ? (((spritetype_ega *)(block))->sourceoffset[i]) : (((spritetype_cga *)(block))->sourceoffset[i]))
#define VW_GetSpriteShiftPlaneSize(block, i) ((GRMODE == EGAGR) ? (((spritetype_ega *)(block))->planesize[i]) : (((spritetype_cga *)(block))->planesize[i]))
#define VW_GetSpriteShiftWidth(block, i) ((GRMODE == EGAGR) ? (((spritetype_ega *)(block))->width[i]) : (((spritetype_cga *)(block))->width[i]))
#define VW_GetSpriteData(block) ((GRMODE == EGAGR) ? (((spritetype_ega *)(block))->data) : (((spritetype_cga *)(block))->data))

typedef struct
{
	id0_int_t width,height;
} pictabletype;


typedef struct
{
	id0_int_t height;
	id0_int_t location[256];
	id0_char_t width[256];
} fontstruct;

#pragma pack(pop)

// REFKEEN - Not originally used, and related code won't compile as C++ as-is
//typedef enum {CGAgr,EGAgr,VGAgr} grtype;

//===========================================================================

extern	cardtype	videocard;		// set by VW_Startup
// REFKEEN - Related code won't compile as C++ as-is with unused grtype
extern int grmode; // TEXTGR, CGAGR, EGAGR, VGAGR
//extern	grtype		grmode;			// CGAgr, EGAgr, VGAgr

extern	id0_unsigned_t	bufferofs;		// hidden port to draw to before displaying
extern	id0_unsigned_t	displayofs;		// origin of port on visable screen
extern	id0_unsigned_t	panx,pany;		// panning adjustments inside port in pixels
extern	id0_unsigned_t	pansx,pansy;
extern	id0_unsigned_t	panadjust;		// panx/pany adjusted by screen resolution

extern  id0_byte_t     *screenseg;
//extern	id0_unsigned_t	screenseg;		// normally 0xa000 or buffer segment

extern	id0_unsigned_t	linewidth;
extern	id0_unsigned_t	ylookup[VIRTUALHEIGHT];

extern	id0_boolean_t		screenfaded;

extern	pictabletype	id0_seg *pictable;
extern	pictabletype	id0_seg *picmtable;
extern	spritetabletype id0_seg *spritetable;

extern	id0_int_t			px,py;
extern	id0_byte_t		pdrawmode,fontcolor;

// REFKEEN - New variable for 2015 port data;
// **MUST** be set to false for DOS versions!!
extern	id0_boolean_t	fakecgamode;

//
// asm globals
//

extern	id0_unsigned_t	*shifttabletable[8];


//===========================================================================


void	VW_Startup (void);
void	VW_Shutdown (void);

cardtype	VW_VideoID (void);

//
// EGA hardware routines
//

#if 0
#define EGAWRITEMODE(x) asm{cli;mov dx,GC_INDEX;mov ax,GC_MODE+256*x;out dx,ax;sti;}
#define EGABITMASK(x) asm{mov dx,GC_INDEX;mov ax,GC_BITMASK+256*x;out dx,ax;sti;}
#define EGAMAPMASK(x) asm{cli;mov dx,SC_INDEX;mov ax,SC_MAPMASK+x*256;out dx,ax;sti;}
#endif
// (REFKEEN) Doing nothing
#define EGAWRITEMODE(x) {}
#define EGABITMASK(x) {}
#define EGAMAPMASK(x) {}

void 	VW_SetLineWidth(id0_int_t width);
// REFKEEN - No need to turn this into a function pointer,
// but since there's an unused no-op implementation in id_vw_ac,
// add EGA suffix for clarity
void 	VW_SetScreen_EGA (id0_unsigned_t CRTC, id0_unsigned_t pelpan);

void	VW_SetScreenMode (id0_int_t grmode);
void	VW_ClearVideo (id0_int_t color);

static inline void VW_WaitVBL (id0_int_t number)
{
	// TODO (REFKEEN) - Emulate behaviors for number < 0?
#if WAITFORVBL
	BE_ST_WaitForNewVerticalRetraces(number);
#endif
}

void	VW_ColorBorder (id0_int_t color);
void	VW_SetDefaultColors(void);
void	VW_FadeOut(void);
void	VW_FadeIn(void);
void	VW_FadeUp(void);
void	VW_FadeDown(void);

//
// block primitives
//

extern void (*VW_MaskBlock)(memptr segm,id0_unsigned_t ofs,id0_unsigned_t dest,
	id0_unsigned_t wide,id0_unsigned_t height,id0_unsigned_t planesize);
extern void (*VW_MemToScreen)(memptr source,id0_unsigned_t dest,id0_unsigned_t width,id0_unsigned_t height);
extern void (*VW_ScreenToMem)(id0_unsigned_t source,memptr dest,id0_unsigned_t width,id0_unsigned_t height);
extern void (*VW_ScreenToScreen)(id0_unsigned_t source,id0_unsigned_t dest,id0_unsigned_t width,id0_unsigned_t height);

void VW_MaskBlock_EGA(memptr segm,id0_unsigned_t ofs,id0_unsigned_t dest,
	id0_unsigned_t wide,id0_unsigned_t height,id0_unsigned_t planesize);
void VW_MemToScreen_EGA(memptr source,id0_unsigned_t dest,id0_unsigned_t width,id0_unsigned_t height);
void VW_ScreenToScreen_EGA(id0_unsigned_t source,id0_unsigned_t dest,id0_unsigned_t width,id0_unsigned_t height);

void VW_MaskBlock_CGA(memptr segm,id0_unsigned_t ofs,id0_unsigned_t dest,
	id0_unsigned_t wide,id0_unsigned_t height,id0_unsigned_t planesize);
void VW_MemToScreen_CGA(memptr source,id0_unsigned_t dest,id0_unsigned_t width,id0_unsigned_t height);
void VW_ScreenToScreen_CGA(id0_unsigned_t source,id0_unsigned_t dest,id0_unsigned_t width,id0_unsigned_t height);


//
// block addressable routines
//

extern void (*VW_DrawTile8)(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t tile);

// REFKEEN - Now that GRMODE is a variable, dynamically choose what to do here

#define VW_DrawTile8M(x,y,t) \
	do \
	{ \
		if (GRMODE == EGAGR) \
			VW_MaskBlock_EGA(grsegs[STARTTILE8M],(t)*40,bufferofs+ylookup[y]+(x),1,8,8); \
		else \
			VW_MaskBlock_CGA(grsegs[STARTTILE8M],(t)*32,bufferofs+ylookup[y]+(x),2,8,16); \
	} \
	while (0)

#define VW_DrawTile16(x,y,t) \
	do \
	{ \
		if (GRMODE == EGAGR) \
			VW_MemToScreen_EGA(grsegs[STARTTILE16+t],bufferofs+ylookup[y]+(x),2,16); \
		else \
			VW_MemToScreen_CGA(grsegs[STARTTILE16+t],bufferofs+ylookup[y]+(x),4,16); \
	} \
	while (0)

#define VW_DrawTile16M(x,y,t) \
	do \
	{ \
		if (GRMODE == EGAGR) \
			VW_MaskBlock(grsegs[STARTTILE16M],(t)*160,bufferofs+ylookup[y]+(x),2,16,32); \
		else \
			VW_MaskBlock(grsegs[STARTTILE16M],(t)*128,bufferofs+ylookup[y]+(x),4,16,64); \
	} \
	while (0)

#if 0
#if GRMODE == EGAGR

#define VW_DrawTile8M(x,y,t) \
	VW_MaskBlock(grsegs[STARTTILE8M],(t)*40,bufferofs+ylookup[y]+(x),1,8,8)
#define VW_DrawTile16(x,y,t) \
	VW_MemToScreen(grsegs[STARTTILE16+t],bufferofs+ylookup[y]+(x),2,16)
#define VW_DrawTile16M(x,y,t) \
	VW_MaskBlock(grsegs[STARTTILE16M],(t)*160,bufferofs+ylookup[y]+(x),2,16,32)

#endif

#if GRMODE == CGAGR

#define VW_DrawTile8M(x,y,t) \
	VW_MaskBlock(grsegs[STARTTILE8M],(t)*32,bufferofs+ylookup[y]+(x),2,8,16)
#define VW_DrawTile16(x,y,t) \
	VW_MemToScreen(grsegs[STARTTILE16+t],bufferofs+ylookup[y]+(x),4,16)
#define VW_DrawTile16M(x,y,t) \
	VW_MaskBlock(grsegs[STARTTILE16M],(t)*128,bufferofs+ylookup[y]+(x),4,16,64)

#endif
#endif

void VW_DrawPic(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t chunknum);
void VW_DrawMPic(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t chunknum);

//
// pixel addressable routines
//
void	VW_MeasurePropString (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend, id0_word_t *width, id0_word_t *height);
void	VW_MeasureMPropString  (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend, id0_word_t *width, id0_word_t *height);

extern void (*VW_DrawPropString) (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend);
// REFKEEN - Unused function
//extern void (*VW_DrawMPropString) (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend);
void VW_DrawSprite(id0_int_t x, id0_int_t y, id0_unsigned_t sprite);
extern void (*VW_Plot)(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t color);
extern void (*VW_Hlin)(id0_unsigned_t xl, id0_unsigned_t xh, id0_unsigned_t y, id0_unsigned_t color);
extern void (*VW_Vlin)(id0_unsigned_t yl, id0_unsigned_t yh, id0_unsigned_t x, id0_unsigned_t color);
extern void (*VW_Bar) (id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t width, id0_unsigned_t height,
	id0_unsigned_t color);

void VW_Plot_EGA(id0_unsigned_t x, id0_unsigned_t y, id0_unsigned_t color);
void VW_Hlin_EGA(id0_unsigned_t xl, id0_unsigned_t xh, id0_unsigned_t y, id0_unsigned_t color);


//===========================================================================

//
// Double buffer management routines
//

void VW_InitDoubleBuffer (void);
void VW_FixRefreshBuffer (void);
id0_int_t	 VW_MarkUpdateBlock (id0_int_t x1, id0_int_t y1, id0_int_t x2, id0_int_t y2);
void VW_UpdateScreen (void);
void VW_CGAFullUpdate (void);

//
// cursor
//

void VW_ShowCursor (void);
void VW_HideCursor (void);
void VW_MoveCursor (id0_int_t x, id0_int_t y);
void VW_SetCursor (id0_int_t spritenum);

//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8 (id0_int_t x, id0_int_t y, id0_int_t tile);
void VWB_DrawTile8M (id0_int_t x, id0_int_t y, id0_int_t tile);
void VWB_DrawTile16 (id0_int_t x, id0_int_t y, id0_int_t tile);
void VWB_DrawTile16M (id0_int_t x, id0_int_t y, id0_int_t tile);
void VWB_DrawPic (id0_int_t x, id0_int_t y, id0_int_t chunknum);
void VWB_DrawMPic(id0_int_t x, id0_int_t y, id0_int_t chunknum);
void VWB_Bar (id0_int_t x, id0_int_t y, id0_int_t width, id0_int_t height, id0_int_t color);

void VWB_DrawPropString	 (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend);
// REFKEEN - Unused function
//void VWB_DrawMPropString (const id0_char_t id0_far *string, const id0_char_t id0_far *optsend);
void VWB_DrawSprite (id0_int_t x, id0_int_t y, id0_int_t chunknum);
void VWB_Plot (id0_int_t x, id0_int_t y, id0_int_t color);
void VWB_Hlin (id0_int_t x1, id0_int_t x2, id0_int_t y, id0_int_t color);
void VWB_Vlin (id0_int_t y1, id0_int_t y2, id0_int_t x, id0_int_t color);

//===========================================================================
