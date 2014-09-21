// Stubs for ID_RF_A.ASM

#include "id_heads.h"


#define TILESWIDE 21
#define TILESHIGH 14

//=================
//
// RFL_NewTile
//
// Draws a composit two plane tile to the master screen and sets the update
// spot to 1 in both update pages, forcing the tile to be copied to the
// view pages the next two refreshes
//
// Called to draw newlly scrolled on strips and animating tiles
//
// Assumes write mode 0
//
//=================

void RFL_NewTile (unsigned updateoffset)
{
	updatestart[1][updateoffset] = updatestart[0][updateoffset] = 1;
	unsigned tileofs = originmap + updatemapofs[updateoffset];
	uint8_t *dst = &vw_videomem[masterofs] + blockstarts[updateoffset];

	uint16_t fg = mapsegs[1][tileofs/2];
	uint16_t bg = mapsegs[0][tileofs/2];

	VW_UnmaskedToPAL8(grsegs[STARTTILE16+bg], dst, 0, 0, linewidth, 16, 16);
	if(fg)
		VW_MaskedBlitToPAL8(grsegs[STARTTILE16M+fg], dst, 0, 0, linewidth, 16, 16);
}

void RFL_MaskForegroundTiles (void)
{
}

//=================
//
// RFL_UpdateTiles
//
// Scans through the update matrix pointed to by updateptr, looking for 1s.
// A 1 represents a tile that needs to be copied from the master screen to the
// current screen (a new row or an animated tiled). If more than one adjacent
// tile in a horizontal row needs to be copied, they will be copied as a group.
//
// Assumes write mode 1
//
//=================

void RFL_UpdateTiles (void)
{
	byte *cur = updateptr;
	byte *end = cur + (TILESWIDE+1)*TILESHIGH;
	do
	{
		unsigned updateoffset = (unsigned)(cur - updateptr);
		unsigned copy = 16;
		if (cur >= end) break;
		if (*cur++ != 1) continue;
		while (cur < end && *cur++ == 1)
			copy += 16;

		unsigned dst = bufferofs + blockstarts[updateoffset];
		unsigned src = masterofs + blockstarts[updateoffset];
		VW_ScreenToScreen(src, dst, copy, 16);
	}
	while(cur < end);
}
