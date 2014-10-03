/*
Omnispeak: A Commander Keen Reimplementation
Copyright (C) 2012 David Gow <david@ingeniumdigital.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// ID_VW_CONV: This contains code (that I originally wrote for Omnispeak) for
// converting EGA planar data to VGA data or RGBA.

#include "id_heads.h"

#define max(a,b) (((a) < (b))?(b):(a))
#define min(a,b) (((a) < (b))?(a):(b))

const uint8_t VW_EGAPalette[16][3] = {
	0x00, 0x00, 0x00, // Black
	0x00, 0x00, 0xaa, // Blue
	0x00, 0xaa, 0x00, // Green
	0x00, 0xaa, 0xaa, // Cyan
	0xaa, 0x00, 0x00, // Red
	0xaa, 0x00, 0xaa, // Magenta
	0xaa, 0x55, 0x00, // Brown
	0xaa, 0xaa, 0xaa, // Light Grey
	0x55, 0x55, 0x55, // Dark Grey
	0x55, 0x55, 0xff, // Light Blue
	0x55, 0xff, 0x55, // Light Green
	0x55, 0xff, 0xff, // Light Cyan
	0xff, 0x55, 0x55, // Light Red
	0xff, 0x55, 0xff, // Light Magenta
	0xff, 0xff, 0x55, // Yellow
	0xff, 0xff, 0xff, // White
};

void VW_UnmaskedToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr_b = (uint8_t*)src;
	uint8_t *srcptr_g = srcptr_b + (w/8)*h;
	uint8_t *srcptr_r = srcptr_g + (w/8)*h;
	uint8_t *srcptr_i = srcptr_r + (w/8)*h;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * w + sx) >> 3;
			int plane_bit = 1<<(7-((sy * w + sx) & 7));
			
			int pixel = ((srcptr_i[plane_off] & plane_bit)?8:0) |
					((srcptr_r[plane_off] & plane_bit)?4:0) |
					((srcptr_g[plane_off] & plane_bit)?2:0) |
					((srcptr_b[plane_off] & plane_bit)?1:0);

			dstptr[(sy+y)*pitch+(sx+x)] = pixel;
		}
	}		
}

void VW_UnmaskedSubRectToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int planesize, int planepitch)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr_b = (uint8_t*)src;
	uint8_t *srcptr_g = srcptr_b + planesize;
	uint8_t *srcptr_r = srcptr_g + planesize;
	uint8_t *srcptr_i = srcptr_r + planesize;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * planepitch*8 + sx) >> 3;
			int plane_bit = 1<<(7-((sy * planepitch*8 + sx) & 7));
			
			int pixel = ((srcptr_i[plane_off] & plane_bit)?8:0) |
					((srcptr_r[plane_off] & plane_bit)?4:0) |
					((srcptr_g[plane_off] & plane_bit)?2:0) |
					((srcptr_b[plane_off] & plane_bit)?1:0);

			dstptr[(sy+y)*pitch+(sx+x)] = pixel;
		}
	}		
}

// This is used for the mouse cursor, if we're doing HW cursors.
void VW_MaskedScaleToRGBA(void *src,void *dest, int xscale, int yscale, int pitch, int w, int h)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr_a = (uint8_t*)src;
	uint8_t *srcptr_b = srcptr_a + (w/8)*h;
	uint8_t *srcptr_g = srcptr_b + (w/8)*h;
	uint8_t *srcptr_r = srcptr_g + (w/8)*h;
	uint8_t *srcptr_i = srcptr_r + (w/8)*h;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * w + sx) >> 3;
			int plane_bit = 1<<(7-((sy * w + sx) & 7));
			
			int pixel = ((srcptr_i[plane_off] & plane_bit)?8:0) |
					((srcptr_r[plane_off] & plane_bit)?4:0) |
					((srcptr_g[plane_off] & plane_bit)?2:0) |
					((srcptr_b[plane_off] & plane_bit)?1:0);

			// ARGB LE output
			for(int dy = 0; dy < yscale; ++dy)
			{
				for(int dx = 0; dx < xscale; ++dx)
				{
					size_t pixcoord = (sy*yscale+dy)*pitch+(sx*xscale+dx)*4;
					dstptr[pixcoord+0] = VW_EGAPalette[pixel][2];
					dstptr[pixcoord+1] = VW_EGAPalette[pixel][1];
					dstptr[pixcoord+2] = VW_EGAPalette[pixel][0];
					dstptr[pixcoord+3] = ((srcptr_a[plane_off] & plane_bit)?0x00:0xFF);
				}
			}
		}
	}		
}

void VW_MaskedToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr_a = (uint8_t*)src;
	uint8_t *srcptr_b = srcptr_a + (w/8)*h;
	uint8_t *srcptr_g = srcptr_b + (w/8)*h;
	uint8_t *srcptr_r = srcptr_g + (w/8)*h;
	uint8_t *srcptr_i = srcptr_r + (w/8)*h;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * w + sx) >> 3;
			int plane_bit = 1<<(7-((sy * w + sx) & 7));
			
			int pixel = ((srcptr_i[plane_off] & plane_bit)?8:0) |
					((srcptr_r[plane_off] & plane_bit)?4:0) |
					((srcptr_g[plane_off] & plane_bit)?2:0) |
					((srcptr_b[plane_off] & plane_bit)?1:0);

			if(!(srcptr_a[plane_off] & plane_bit))
				dstptr[(sy+y)*pitch+(sx+x)] = pixel;
		}
	}		
}

void VW_MaskedBlitToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr_a = (uint8_t*)src;
	uint8_t *srcptr_b = srcptr_a + (w/8)*h;
	uint8_t *srcptr_g = srcptr_b + (w/8)*h;
	uint8_t *srcptr_r = srcptr_g + (w/8)*h;
	uint8_t *srcptr_i = srcptr_r + (w/8)*h;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * w + sx) >> 3;
			int plane_bit = 1<<(7-((sy * w + sx) & 7));
			
			
			if ((srcptr_a[plane_off] & plane_bit) != 0) {
				continue;
			}
			int pixel = ((srcptr_i[plane_off] & plane_bit)?8:0) |
					((srcptr_r[plane_off] & plane_bit)?4:0) |
					((srcptr_g[plane_off] & plane_bit)?2:0) |
					((srcptr_b[plane_off] & plane_bit)?1:0);

			dstptr[(sy+y)*pitch+(sx+x)] = pixel;
		}
	}		
}

void VW_MaskedBlitWPlaneSizeToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int planesize)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr_a = (uint8_t*)src;
	uint8_t *srcptr_b = srcptr_a + planesize;
	uint8_t *srcptr_g = srcptr_b + planesize;
	uint8_t *srcptr_r = srcptr_g + planesize;
	uint8_t *srcptr_i = srcptr_r + planesize;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * w + sx) >> 3;
			int plane_bit = 1<<(7-((sy * w + sx) & 7));
			
			
			if ((srcptr_a[plane_off] & plane_bit) != 0) {
				continue;
			}
			int pixel = ((srcptr_i[plane_off] & plane_bit)?8:0) |
					((srcptr_r[plane_off] & plane_bit)?4:0) |
					((srcptr_g[plane_off] & plane_bit)?2:0) |
					((srcptr_b[plane_off] & plane_bit)?1:0);

			dstptr[(sy+y)*pitch+(sx+x)] = pixel;
		}
	}		
}

void VW_MaskedBlitClipToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int dw, int dh)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr_a = (uint8_t*)src;
	uint8_t *srcptr_b = srcptr_a + (w/8)*h;
	uint8_t *srcptr_g = srcptr_b + (w/8)*h;
	uint8_t *srcptr_r = srcptr_g + (w/8)*h;
	uint8_t *srcptr_i = srcptr_r + (w/8)*h;
	int initialX = max(-x,0);
	int initialY = max(-y,0);
	int finalW = min(max(dw-x,0), w);
	int finalH = min(max(dh-y,0), h);

	for(int sy = initialY; sy < finalH; ++sy)
	{
		for(int sx = initialX; sx < finalW; ++sx)
		{
			int plane_off = (sy * w + sx) >> 3;
			int plane_bit = 1<<(7-((sy * w + sx) & 7));
			
			
			if ((srcptr_a[plane_off] & plane_bit) != 0) {
				continue;
			}
			int pixel = ((srcptr_i[plane_off] & plane_bit)?8:0) |
					((srcptr_r[plane_off] & plane_bit)?4:0) |
					((srcptr_g[plane_off] & plane_bit)?2:0) |
					((srcptr_b[plane_off] & plane_bit)?1:0);

			dstptr[(sy+y)*pitch+(sx+x)] = pixel;
		}
	}		
}

void VW_1bppToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int colour)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr= (uint8_t*)src;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * w + sx) >> 3;
			int plane_bit = 1<<(7-((sy * w + sx) & 7));
			
			int pixel = ((srcptr[plane_off] & plane_bit)?colour:colour&0xF0);

			dstptr[(sy+y)*pitch+(sx+x)] = pixel;
		}
	}		
}

void VW_1bppXorWithPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int colour)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr= (uint8_t*)src;

	int spitch = ((w + 7)/8)*8;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * spitch + sx) >> 3;
			int plane_bit = 1<<(7-((sy * spitch + sx) & 7));
			
			if (!(srcptr[plane_off] & plane_bit)) continue;

			dstptr[(sy+y)*pitch+(sx+x)] ^= colour;
		}
	}		
}

void VW_1bppBlitToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int colour)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr= (uint8_t*)src;

	int spitch = ((w + 7)/8)*8;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * spitch + sx) >> 3;
			int plane_bit = 1<<(7-((sy * spitch + sx) & 7));
			
			if (!(srcptr[plane_off] & plane_bit)) continue;
			
			dstptr[(sy+y)*pitch+(sx+x)] = colour;
		}
	}		
}

void VW_1bppInvBlitToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int colour)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr= (uint8_t*)src;

	int spitch = ((w + 7)/8)*8;

	for(int sy = 0; sy < h; ++sy)
	{
		for(int sx = 0; sx < w; ++sx)
		{
			int plane_off = (sy * spitch + sx) >> 3;
			int plane_bit = 1<<(7-((sy * spitch + sx) & 7));
			
			if ((srcptr[plane_off] & plane_bit)) continue;
			
			dstptr[(sy+y)*pitch+(sx+x)] = colour;
		}
	}		
}

void VW_1bppInvBlitClipToPAL8(void *src,void *dest, int x, int y, int pitch, int w, int h, int dw, int dh, int colour)
{
	uint8_t *dstptr = (uint8_t*)dest;
	uint8_t *srcptr = (uint8_t*)src;
	int initialX = max(-x,0);
	int initialY = max(-y,0);
	int finalW = min(max(dw-x,0), w);
	int finalH = min(max(dh-y,0), h);
	int spitch = ((w + 7)/8)*8;

	for(int sy = initialY; sy < finalH; ++sy)
	{
		for(int sx = initialX; sx < finalW; ++sx)
		{
			int plane_off = (sy * spitch + sx) >> 3;
			int plane_bit = 1<<(7-((sy * spitch + sx) & 7));
			
			if ((srcptr[plane_off] & plane_bit)) continue;
			
			dstptr[(sy+y)*pitch+(sx+x)] = colour;
		}
	}		
}
