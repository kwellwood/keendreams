// stubs for id_vw_ae.asm

#include "id_heads.h"
#include "GL/glew.h"

uint8_t vw_videomem[VW_VIDEOMEM_SIZE];

void VW_Plot(unsigned x, unsigned y, unsigned color)
{
	uint8_t *screen = &vw_videomem[bufferofs];
	screen[y*linewidth+x] = color;
	VW_SetScreen(bufferofs,0);
}

void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color)
{
	uint8_t *screen = &vw_videomem[bufferofs];
	unsigned y;
	for(y = yl; y < yh; ++y)
	{
		screen[y*linewidth+x] = color;
	}
	VW_SetScreen(bufferofs,0);
}

void VW_DrawTile8(unsigned x, unsigned y, unsigned tile)
{
	uint8_t *src = ((uint8_t*)(grsegs[STARTTILE8])) + (tile * 8 * 4);
	uint8_t *screen = &vw_videomem[bufferofs];
	VW_UnmaskedSubRectToPAL8(src, screen, x, y, linewidth, 8, 8, 8, 1);
	VW_SetScreen(bufferofs,0);
	VW_GL_UpdateGLBuffer();
	VW_GL_Present();
	
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
	for (y = 0; y < height; y++)
	{
		memmove(&vw_videomem[dest], &vw_videomem[source], wide);
		dest += linewidth;
		source += linewidth;
	}
	VW_SetScreen(bufferofs,0);
}

void VW_MemToScreen(memptr source, unsigned dest, unsigned wide, unsigned height)
{
	VW_UnmaskedToPAL8(source, &vw_videomem[dest], 0, 0, linewidth, wide, height);
	VW_SetScreen(bufferofs,0);
}

void VW_ScreenToMem(unsigned source, memptr dest, unsigned wide, unsigned height)
{
}


void VWL_UpdateScreenBlocks()
{
}

void VW_SetScreen(unsigned crtc, unsigned pel)
{
	lastdrawnbuffer = crtc;
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
	uint8_t *screen = &vw_videomem[bufferofs];
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
