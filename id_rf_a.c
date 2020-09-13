/* Keen Dreams (SDL2/Steam Port) Source Code
 * Copyright (C) 2015 David Gow <david@davidgow.net>
 * Copyright (C) 2014 Braden Obrzut
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

	if(grsegs[STARTTILE16+bg])
		VW_UnmaskedToPAL8(grsegs[STARTTILE16+bg], dst, 0, 0, linewidth, 16, 16);
	if(fg && grsegs[STARTTILE16M+fg])
		VW_MaskedBlitToPAL8(grsegs[STARTTILE16M+fg], dst, 0, 0, linewidth, 16, 16);
}

//=================
//
// RFL_MaskForegroundTiles
//
// Scan through update looking for 3's. If the foreground tile there is a
// masked foreground tile, draw it to the screen
//
//=================

void RFL_MaskForegroundTiles (void)
{
	byte *cur = updateptr;
	byte *end = cur + (TILESWIDE+1)*TILESHIGH;
	do
	{
		unsigned updateoffset = (unsigned)(cur - updateptr);
		if (cur >= end) break;
		if (*cur++ != 3) continue;

		unsigned tileofs = originmap + updatemapofs[updateoffset];
		uint16_t fg = mapsegs[1][tileofs/2];
		if(fg == 0 || !(tinf[INTILE+fg] & 0x80)) continue;

		uint8_t* dst = &vw_videomem[bufferofs] + blockstarts[updateoffset];
		VW_MaskedBlitToPAL8(grsegs[STARTTILE16M+fg], dst, 0, 0, linewidth, 16, 16);
	}
	while(cur < end);
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
//		while (cur < end && *cur++ == 1)
//			copy += 16;

		unsigned dst = bufferofs + blockstarts[updateoffset];
		unsigned src = masterofs + blockstarts[updateoffset];
		VW_ScreenToScreen(src, dst, copy, 16);
	}
	while(cur < end);
}
