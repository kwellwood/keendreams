// stubs for id_vw_ae.asm

#include "id_heads.h"
#include "GL/glew.h"

uint8_t vw_videomem[VW_VIDEOMEM_SIZE];

void VW_Plot(unsigned x, unsigned y, unsigned color)
{
	uint8_t *screen = &vw_videomem[bufferofs];
	screen[(y*linewidth+x) % VW_VIDEOMEM_SIZE] = color;
}

void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color)
{
	uint8_t *screen = &vw_videomem[bufferofs];
	unsigned y;
	for(y = yl; y < yh; ++y)
	{
		screen[(y*linewidth+x) % VW_VIDEOMEM_SIZE] = color;
	}
}

void VW_DrawTile8(unsigned x, unsigned y, unsigned tile)
{
	uint8_t *src = ((uint8_t*)(grsegs[STARTTILE8])) + (tile * 8 * 4);
	uint8_t *screen = &vw_videomem[bufferofs];
	VW_UnmaskedSubRectToPAL8(src, screen, x, y, linewidth, 8, 8, 8, 1);
	
}

/*void VW_DrawTile8M(unsigned x, unsigned y, word tile)
{
}*/

void VW_MaskBlock(memptr segment, unsigned ofs, unsigned dest, unsigned wide, unsigned height, unsigned planesize)
{
	VW_MaskedBlitWPlaneSizeToPAL8((uint8_t*)segment+ofs, &vw_videomem[dest], 0, 0, linewidth, wide, height, planesize);
}

void VW_ScreenToScreen(unsigned source, unsigned dest, unsigned wide, unsigned height)
{
	int y;
	if (source == dest) return;
	for (y = 0; y < height; y++)
	{
		dest = (dest) % VW_VIDEOMEM_SIZE;
		source = (source) % VW_VIDEOMEM_SIZE;
		memmove(&vw_videomem[dest], &vw_videomem[source], wide);
		dest += linewidth;
		source += linewidth;
	}
}

void VW_MemToScreen(memptr source, unsigned dest, unsigned wide, unsigned height)
{
	VW_UnmaskedToPAL8(source, &vw_videomem[dest], 0, 0, linewidth, wide, height);
}

void VW_ScreenToMem(unsigned source, memptr dest, unsigned wide, unsigned height)
{
}


void VWL_UpdateScreenBlocks()
{
	byte *cur = updateptr;
	byte *end = cur + (UPDATEWIDE)*UPDATEHIGH+1;
	do
	{
		unsigned updateoffset = (unsigned)(cur - updateptr);
		unsigned copy = 16;
		while(*cur++ == 1)
			copy += 16;

		unsigned dst = displayofs + blockstarts[updateoffset];
		unsigned src = bufferofs + blockstarts[updateoffset];
		VW_ScreenToScreen(src, dst, copy, 16);
	}
	while(cur < end);
}

void VW_SetScreen(unsigned crtc, unsigned pel)
{
	lastdrawnbuffer = (crtc % VW_VIDEOMEM_SIZE);
	lastdrawnpan = pel;
	VW_GL_UpdateGLBuffer();
	VW_GL_Present();
}

int px, py;
byte pdrawmode, fontcolor;


#define BUFFWIDTH 50
#define BUFFHEIGHT 32

byte databuffer[BUFFWIDTH * BUFFHEIGHT];

int bufferwidth, bufferheight;

void BufferToScreen()
{
}

void ShiftPropChar()
{
}

void VW_DrawPropString(char *str)
{
	int oldpx = px;
	uint8_t *screen = &vw_videomem[bufferofs + panadjust];
	fontstruct *font = (fontstruct*)(grsegs[STARTFONT]);
	while (*str)
	{
		VW_1bppXorWithPAL8((uint8_t*)grsegs[STARTFONT] + font->location[*str], screen, px, py, linewidth, font->width[*str], font->height, fontcolor);
		px += font->width[*str];
		str++;
	}
}

void ShiftMPropChar()
{
}

void VW_DrawMPropString(char *str)
{
}
