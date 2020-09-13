/* Keen Dreams (SDL2/Steam Port) Source Code
 * Copyright (C) 2014 David Gow <david@davidgow.net>
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


// stubs for id_vw_ae.asm

#include "id_heads.h"
#include "GL/glew.h"

uint8_t *vw_videomem;

#ifndef _MSC_VER
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


void VWL_SetupVideoMemory()
{
#ifdef VW_MMAP_RINGBUFFER
	// We support video memory wrapping by getting the OS to set up page tables
	// for us. Needs VW_VIDEOMEM_SIZE to be a multiple of the page size.
#ifdef WIN32
	vw_videomem = VirtualAlloc(0, VW_VIDEOMEM_SIZE * 2, MEM_RESERVE, PAGE_NOACCESS);
	if (!vw_videomem)
		Quit("Couldn't reserve address space for emulated video memory.");
	
	VirtualFree(vw_videomem, 0, MEM_RELEASE);
	
	HANDLE mapping = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, /* (uintptr_t)(VW_VIDEOMEM_SIZE) >> 32 */ 0, (uintptr_t)VW_VIDEOMEM_SIZE & 0xffffffff, 0);
	if (!mapping)
		Quit("Couldn't create emulated video memory mapping.");
	
	
	if (!(vw_videomem = MapViewOfFileEx(mapping, FILE_MAP_ALL_ACCESS, 0, 0, VW_VIDEOMEM_SIZE, vw_videomem)))
		Quit("Couldn't map emulated video memory.");
	if (!MapViewOfFileEx(mapping, FILE_MAP_ALL_ACCESS, 0, 0, VW_VIDEOMEM_SIZE, vw_videomem + VW_VIDEOMEM_SIZE))
		Quit("Couldn't map emulated video memory.");
	
#else
	char vmpath[] = "/dev/shm/kdreams-vmem-XXXXXX";
	int fd = mkstemp(vmpath);
	if (fd < 0) Quit("Couldn't create emulated video memory mapping.");
	
	if (unlink(vmpath))
		Quit("Couldn't delete backing file for emulated video memory.");
	
	if (ftruncate(fd, VW_VIDEOMEM_SIZE))
		Quit("Couldn't size file for emulated video memory."); 
	
	// Try to reserve 2*VW_VIDEOMEM_SIZE bytes of address space.
	vw_videomem = mmap(NULL, VW_VIDEOMEM_SIZE * 2, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (vw_videomem == MAP_FAILED)
		Quit("Couldn't reserve address space for emulated video memory.");
	
	// Now map two copies of videomem so that we can wrap around.
	if (mmap(vw_videomem, VW_VIDEOMEM_SIZE, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0) == MAP_FAILED)
		Quit("Couldn't map emulated video memory.");
	if (mmap(vw_videomem + VW_VIDEOMEM_SIZE, VW_VIDEOMEM_SIZE, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0) == MAP_FAILED)
		Quit("Couldn't map emulated video memory.");
	
	close(fd);
#endif
#else
	vw_videomem = malloc(VW_VIDEOMEM_SIZE);
#endif
}

void VW_Plot(unsigned x, unsigned y, unsigned color)
{
	uint8_t *screen = &vw_videomem[bufferofs];
	screen[(y*linewidth+x) % VW_VIDEOMEM_SIZE] = color;
}

void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color)
{
	uint8_t *screen = &vw_videomem[bufferofs];
	unsigned y;
	for(y = yl; y <= yh; ++y)
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
#ifndef VW_MMAP_RINGBUFFER
		dest = (dest) % VW_VIDEOMEM_SIZE;
		source = (source) % VW_VIDEOMEM_SIZE;
#endif
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

void VW_RawMemToScreen(memptr source, unsigned dest, unsigned wide, unsigned height)
{
	for (int y = 0; y < height; ++y)
	{
		memcpy(&vw_videomem[dest+y*linewidth], (byte*)source + (y*wide), wide);
	}
}

void VW_RawScreenToMem(unsigned source, memptr dest, unsigned wide, unsigned height)
{
	for (int y = 0; y < height; ++y)
	{
		memcpy((byte*)dest + (y*wide), &vw_videomem[source+y*linewidth], wide);
	}
}

void VW_RawBlitToScreen(memptr source, unsigned dest, unsigned wide, unsigned height)
{
#ifndef __SSE__ 
	// I'm pretty horribly ashamed of this, but I did profile it and it was faster.
	uint32_t *src = (uint32_t*)source;
	uint32_t *dst = (uint32_t*)&vw_videomem[dest];
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < wide/4; ++x)
		{
			uint32_t c = *src++;
			if (c != 0xffffffff)
			{
				if ((c & 0xff) == 0xff) c = (c & 0xffffff00) | (*dst & 0xff);
				if ((c & 0xff00) == 0xff00) c = (c & 0xffff00ff) | (*dst & 0xff00);
				if ((c & 0xff0000) == 0xff0000) c = (c & 0xff00ffff) | (*dst & 0xff0000);
				if ((c & 0xff000000) == 0xff000000) c = (c & 0x00ffffff) | (*dst & 0xff000000);
				
				*dst = c;
			}
			dst++;
		}
		dst += linewidth / 4 - wide / 4;
	}
#else
        __m64 *src = (__m64*)source;
        uint8_t *dst = (uint8_t*)&vw_videomem[dest];
        __m64 colourkey = _mm_set1_pi8(0xFF);
        for (int y = 0; y < height; ++y)
        {
                for (int x = 0; x < wide/8; ++x)
                {
                        __m64 c = *src++;
                        __m64 mask = _mm_xor_si64(_mm_cmpeq_pi8(c, colourkey), colourkey);
                        _mm_maskmove_si64(c, mask, (char*)dst);
                        dst += 8;
                }
                dst += linewidth - wide;
        }
#endif
}

void VWL_UpdateScreenBlocks()
{
	byte *cur = updateptr;
	byte *end = cur + (UPDATEWIDE)*UPDATEHIGH;
	do
	{
		unsigned updateoffset = (unsigned)(cur - updateptr);
		if (cur >= end) break;
		if (*cur++ != 1) continue;
		unsigned copy = 16;
		//while(*cur++ == 1)
		//	copy += 16;

		unsigned dst = displayofs + blockstarts[updateoffset];
		unsigned src = bufferofs + blockstarts[updateoffset];
		VW_ScreenToScreen(src, dst, copy, 16);
	}
	while(cur < end);
	memset(updateptr, 0, UPDATEWIDE*UPDATEHIGH);
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
