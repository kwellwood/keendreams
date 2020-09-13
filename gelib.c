/* Keen Dreams (SDL2/Steam Port) Source Code
 * Copyright (C) 2014 Javier M. Chavez
 * Copyright (C) 2015 David Gow <david@davidgow.net>
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

#include "kd_def.h"
#include <SDL2/SDL.h>

#define BIO_BUFFER_LEN	(64)

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif


////////////////////////////////////////////////////////////////////////////
//
// FreeShape()
//
void FreeShape(struct Shape *shape)
{
	if (shape->Data)
		MM_FreePtr(&shape->Data);
}

////////////////////////////////////////////////////////////////////////////
//
// UnpackEGAShapeToScreen()
//
int UnpackEGAShapeToScreen(struct Shape *SHP,int startx,int starty)
{
	int currenty;
	signed char n, *Src,  loop, Plane;
	uint8_t Rep, *Dst[8];
	unsigned int BPR, Height;
	int NotWordAligned;

	NotWordAligned = SHP->BPR & 1;
	//startx>>=3;
	Src = SHP->Data;
	currenty = starty;
	Plane = 0;
	Height = SHP->bmHdr.h;
	while (Height--)
	{
		Dst[0] = &vw_videomem[displayofs]; //XXX: EGA stuff, needs hackery.//(MK_FP(0xA000,displayofs));
		Dst[0] += ylookup[currenty];
		Dst[0] += startx;
		for (loop=1; loop<SHP->bmHdr.d; loop++)
			Dst[loop] = Dst[0];

		memset(Dst[0], 0, SHP->bmHdr.w);

		for (Plane=0; Plane<SHP->bmHdr.d; Plane++)
		{
			uint8_t planemask = 1<<Plane;
			BPR = ((SHP->BPR+1) >> 1) << 1;               // IGNORE WORD ALIGN
			while (BPR)
			{
				if (SHP->bmHdr.comp)
					n = *Src++;
				else
					n = BPR-1;

				if (n < 0)
				{
					if (n != -128)
					{
						n = (-n)+1;
						BPR -= n;
						Rep = *(uint8_t*)Src++;
						if ((!BPR) && (NotWordAligned))   // IGNORE WORD ALIGN
							n--;

						while (n--)
						{
							int px = 0;
							for (px = 0; px < 8; ++px)
							{ 
								*Dst[Plane]++ |= (Rep & (1 << (7-px)))?planemask:0;
							}
						}
					}
					else
						BPR--;
				}
				else
				{
					n++;
					BPR -= n;
					if ((!BPR) && (NotWordAligned))     // IGNORE WORD ALIGN
						n--;

					while (n--)
					{
						int px = 0;
						for (px = 0; px < 8; ++px)
						{
							*Dst[Plane]++ |= ((*(uint8_t*)Src) & (1 << (7-px)))?planemask:0;
						}
						Src++;
					}

					if ((!BPR) && (NotWordAligned))     // IGNORE WORD ALIGN
						Src++;
				}
			}
		}
		currenty++;
	}
	return(0);
}

////////////////////////////////////////////////////////////////////////////
//
// Verify()
//
long Verify(char *filename)
{
	FILE *handle;
	long size;

	if ((handle=fopen(filename,"rb"))==NULL)
		return (0);
	fseek(handle, 0, SEEK_END);
	size=ftell(handle);
	fclose(handle);
	return(size);
}

//--------------------------------------------------------------------------
// InitBufferedIO()
//--------------------------------------------------------------------------
memptr InitBufferedIO(int handle, BufferedIO *bio)
{
	bio->handle = handle;
	bio->offset = BIO_BUFFER_LEN;
	bio->status = 0;
	MM_GetPtr(&bio->buffer,BIO_BUFFER_LEN);

	return(bio->buffer);
}

//--------------------------------------------------------------------------
// FreeBufferedIO()
//--------------------------------------------------------------------------
void FreeBufferedIO(BufferedIO *bio)
{
	if (bio->buffer)
		MM_FreePtr(&bio->buffer);
}

//--------------------------------------------------------------------------
// bio_readch()
//--------------------------------------------------------------------------
byte bio_readch(BufferedIO *bio)
{
	if (bio->offset == BIO_BUFFER_LEN)
	{
		bio->offset = 0;
		bio_fillbuffer(bio);
	}

	return (((byte*)bio->buffer)[bio->offset++]);
}

//--------------------------------------------------------------------------
// bio_fillbuffer()
//
// BUGS (Not really bugs... More like RULES!)
//
//    1) This code assumes BIO_BUFFER_LEN is no smaller than
//       NEAR_BUFFER_LEN!!
//
//    2) BufferedIO.status should be altered by this code to report
//       read errors, end of file, etc... If you know how big the file
//       is you're reading, determining EOF should be no problem.
//
//--------------------------------------------------------------------------
void bio_fillbuffer(BufferedIO *bio)
{
	short bio_length,bytes_read,bytes_requested;

	bytes_read = 0;
	bio_length = BIO_BUFFER_LEN;
	while (bio_length)
	{
		if (bio_length > BIO_BUFFER_LEN-1)
			bytes_requested = BIO_BUFFER_LEN;
		else
			bytes_requested = bio_length;

		bytes_requested = read(bio->handle,&((byte*)bio->buffer)[bytes_read],bytes_requested);
		if (bytes_requested <= 0) return;
//		memcpy(&((byte*)bio->buffer)[bytes_read],near_buffer,bytes_requested);

		bio_length -= bytes_requested;
		bytes_read += bytes_requested;
	}
}

///////////////////////////////////////////////////////////////////////////
//
// SwapLong()
//
void SwapLong(uint32_t *Var)
{
	*Var = SDL_Swap32(*Var);
}

///////////////////////////////////////////////////////////////////////////
//
// SwapWord()
//
void SwapWord(uint16_t *Var)
{
	*Var = SDL_Swap16(*Var);
}

////////////////////////////////////////////////////////////////////////////
//
// MoveGfxDst()
//
void MoveGfxDst(short x, short y)
{
	unsigned address;

	address = (y*linewidth)+(x/8);
	bufferofs = displayofs = address;
}
