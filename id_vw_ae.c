// stubs for id_vw_ae.asm

#include "id_heads.h"

void VW_Plot(unsigned x, unsigned y, unsigned color)
{
}

void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color)
{
}

void VW_DrawTile8(unsigned x, unsigned y, unsigned tile)
{
}

/*void VW_DrawTile8M(unsigned x, unsigned y, word tile)
{
}*/

void VW_MaskBlock(memptr segment, unsigned ofs, unsigned dest, unsigned wide, unsigned height, unsigned planesize)
{
}

void VW_ScreenToScreen(unsigned source, unsigned dest, unsigned wide, unsigned height)
{
}

void VW_MemToScreen(memptr source, unsigned dest, unsigned wide, unsigned height)
{
}

void VW_ScreenToMem(unsigned source, memptr dest, unsigned wide, unsigned height)
{
}


void VWL_UpdateScreenBlocks()
{
}

void VW_SetScreen(unsigned crtc, unsigned pel)
{
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
}

void ShiftMPropChar()
{
}

void VW_DrawMPropString(char *str)
{
}
