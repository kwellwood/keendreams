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

// ID_CA.C

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include "id_heads.h"
#pragma hdrstop

#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

typedef struct
{
  uint16_t bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;


PACKED(mapfiletype)
{
	uint16_t 	RLEWtag;
	uint32_t	headeroffsets[100];
	byte		headersize[100];		// headers are very small
	byte		tileinfo[];
} mapfiletype;


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

byte 			*tinf;
int			mapon;

uint16_t		*mapsegs[3];
maptype			*mapheaderseg[NUMMAPS];
byte			*audiosegs[NUMSNDCHUNKS];
void			*grsegs[NUMCHUNKS] = {0};

byte		grneeded[NUMCHUNKS];
byte		ca_levelbit,ca_levelnum;

char		*titleptr[8];

int			profilehandle;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

extern	int32_t		CGAhead;
extern	int32_t		EGAhead;
extern	byte	CGAdict;
extern	byte	EGAdict;
extern	byte		maphead;
extern	byte	mapdict;
extern	byte		audiohead;
extern	byte	audiodict;


int32_t		*grstarts;	// array of offsets in egagraph, -1 for sparse
int32_t		*audiostarts;	// array of offsets in audio / audiot

#ifdef GRHEADERLINKED
huffnode	*grhuffman;
#else
huffnode	grhuffman[255];
#endif

#ifdef MAPHEADERLINKED
huffnode	*maphuffman;
#else
huffnode	maphuffman[255];
#endif

#ifdef AUDIOHEADERLINKED
huffnode	*audiohuffman;
#else
huffnode	audiohuffman[255];
#endif


int			grhandle;		// handle to EGAGRAPH
int			maphandle;		// handle to MAPTEMP / GAMEMAPS
int			audiohandle;	// handle to AUDIOT / AUDIO

uint32_t		chunkcomplen,chunkexplen;

SDMode		oldsoundmode;

/*
=============================================================================

					   LOW LEVEL ROUTINES

=============================================================================
*/

/*
============================
=
= CAL_filelength
=
= Gets the length of the file with handle 'fd'
=
============================
*/
off_t CAL_filelength(int fd)
{
	struct stat filestats;
	fstat(fd, &filestats);
	return filestats.st_size;
}
/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/

void CAL_GetGrChunkLength (int chunk)
{
	lseek(grhandle,grstarts[chunk],SEEK_SET);
	read(grhandle,&chunkexplen,sizeof(chunkexplen));
	chunkcomplen = grstarts[chunk+1]-grstarts[chunk]-4;
}


/*
==========================
=
= CA_FarRead
=
= Read from a file to a  pointer
=
==========================
*/

boolean CA_FarRead (int handle, byte  *dest, long length)
{
	return	(read(handle, dest, length) == length);
}


/*
==========================
=
= CA_SegWrite
=
= Write from a file to a  pointer
=
==========================
*/

boolean CA_FarWrite (int handle, byte  *source, long length)
{
	return (write(handle, source, length) == length);
}


/*
==========================
=
= CA_LoadFile
=
= Allocate space for and load a file
=
==========================
*/

boolean CA_LoadFile(char *filename, memptr *ptr)
{
	// Copied this from Omnispeak.
	FILE *f = fopen(filename, "rb");

	//Get length of file
	fseek(f,0,SEEK_END);
	int length = ftell(f);
	fseek(f,0,SEEK_SET);

	MM_GetPtr(ptr,length);

	int amountRead = fread(*ptr,1, length,f);

	fclose(f);	

	if (amountRead != length)
		return false;
	return true;
}

/*
============================================================================

		COMPRESSION routines, see JHUFF.C for more

============================================================================
*/



/*
===============
=
= CAL_OptimizeNodes
=
= Goes through a huffman table and changes the 256-511 node numbers to the
= actular address of the node.  Must be called before CAL_HuffExpand
=
= Defanged in Omnispeak-based builds for modern systems.
=
===============
*/

void CAL_OptimizeNodes(huffnode *table)
{
	//STUB: This optimization is not very helpful on modern machines.
}



/*
======================
=
= CAL_HuffExpand
=
= Length is the length of the EXPANDED data
=
======================
*/
void CAL_HuffExpand(byte *src, byte *dest, int expLength, huffnode *table)
{
	int headptr = 254;
	byte *srcptr = (byte*)src;
	byte *dstptr = (byte*)dest;
	int src_bit = 1;	//ch in asm src
	byte src_char = *(srcptr++);
	int len = 0;
	int complen = 1;
	while (len < expLength)
	{
		if (src_char & src_bit)
		{
			// We've got a '1' bit.
			headptr = table[headptr].bit1;
		}
		else
		{
			// We've got a '0' bit.
			headptr = table[headptr].bit0;
		}
		
		
		if (headptr > 255) headptr -= 256;
		else {
			*(dstptr++) = (byte)(headptr & 0xff);
			headptr = 254;
			len++;
			if (len == expLength) break;
		}

		src_bit <<= 1;
		if (src_bit == 256) {
			src_char = *(srcptr++);
			src_bit = 1;
			complen++;
		}
	}
}




/*
======================
=
= CA_RLEWcompress
=
======================
*/
long CA_RLEWCompress (word *src, int expLength, word *dest, word rletag)
{
	int compLength = 0;
	uint16_t *srcptr = (uint16_t*)src;
	uint16_t *dstptr = (uint16_t*)dest-1;
	uint16_t count = 0;

	while (expLength)
	{
		uint16_t val = *srcptr++;
		expLength -= 2;
		count = 1;
		while (*srcptr == val && expLength)
		{
			count++;
			expLength -= 2;
			srcptr++;
		}
		if (count > 3 || val == rletag)
		{
			*dstptr++ = rletag;
			*dstptr++ = count;
			*dstptr++ = val;
			compLength += 6;
		}
		else
		{
			compLength += count*2;
			while(count--) *dstptr++ = val;
		}
	}
	return compLength;
}


/*
======================
=
= CA_RLEWexpand
= length is COMPRESSED length
=
======================
*/

void CA_RLEWexpand (word *src, word *dest, int expLength, word rletag)
{
	uint16_t *srcptr = (uint16_t*)src;
	uint16_t *dstptr = (uint16_t*)dest;
	uint16_t count, value;
	int i;

	while (expLength > 0)
	{
		value = *(srcptr++);
		if (value != rletag)
		{
			*(dstptr++) = value;
			expLength -= 2;
		}
		else
		{
			count = *(srcptr++);
			value = *(srcptr++);
			expLength -= count*2;
			if(expLength < 0) return; //count += expLength;
			for(i = 0; i < count; ++i) { *(dstptr++) = value; }
		}
	}
}



/*
=============================================================================

					 CACHE MANAGER ROUTINES

=============================================================================
*/


/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile (void)
{
	int handle;
	uint32_t headersize,length;
	memptr compseg;

#ifdef GRHEADERLINKED

#if GRMODE == EGAGR
	grhuffman = (huffnode *)&EGAdict;
	grstarts = (uint32_t *)(&EGAhead);
#endif
#if GRMODE == CGAGR
	grhuffman = (huffnode *)&CGAdict;
	grstarts = (uint32_t  *)(&CGAhead);
#endif

	CAL_OptimizeNodes (grhuffman);

#else

//
// load ???dict.ext (huffman dictionary for graphics files)
//

	if ((handle = open(GREXT"DICT."EXTENSION,
//	if ((handle = open("KDREAMS.EGA",
		 O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open KDREAMS.EGA!");

	read(handle, &grhuffman, sizeof(grhuffman));
	close(handle);
	CAL_OptimizeNodes (grhuffman);
//
// load the data offsets from ???head.ext
//
	MM_GetPtr ((memptr)&grstarts,(NUMCHUNKS+1)*4);

	if ((handle = open(GREXT"HEAD."EXTENSION,
		 O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open "GREXT"HEAD."EXTENSION"!");

	CA_FarRead(handle, (memptr)grstarts, (NUMCHUNKS+1)*4);

	close(handle);


#endif

//
// Open the graphics file, leaving it open until the game is finished
//
//	grhandle = open(GREXT"GRAPH."EXTENSION, O_RDONLY | O_BINARY); NOLAN
	grhandle = open("KDREAMS.EGA", O_RDONLY | O_BINARY);
	if (grhandle == -1)
		Quit ("Cannot open KDREAMS.EGA!");


//
// load the pic and sprite headers into the arrays in the data segment
//
#if NUMPICS>0
	MM_GetPtr((memptr)&pictable,NUMPICS*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPIC);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte  *)pictable,NUMPICS*sizeof(pictabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

#if NUMPICM>0
	MM_GetPtr((memptr)&picmtable,NUMPICM*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPICM);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte  *)picmtable,NUMPICM*sizeof(pictabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

#if NUMSPRITES>0
	MM_GetPtr((memptr)&spritetable,NUMSPRITES*sizeof(spritetabletype));
	CAL_GetGrChunkLength(STRUCTSPRITE);	// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte  *)spritetable,NUMSPRITES*sizeof(spritetabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

}

//==========================================================================

void CAL_SetupGrFileCGA (void)
{
	int handle;
	uint32_t headersize,length;
	memptr compseg;

#ifdef GRHEADERLINKED
	grhuffman = (huffnode *)&CGAdict;
	grstarts = (uint32_t  *)(&CGAhead);
	CAL_OptimizeNodes (grhuffman);
#else

//
// load ???dict.ext (huffman dictionary for graphics files)
//

	if ((handle = open(GREXT"DICT.CGA",
//	if ((handle = open("KDREAMS.EGA",
		 O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open EGADICT.CGA!");

	read(handle, &grhuffman, sizeof(grhuffman));
	close(handle);
	CAL_OptimizeNodes (grhuffman);
//
// load the data offsets from ???head.ext
//
	MM_GetPtr ((memptr)&grstarts,(NUMCHUNKS+1)*4);

	if ((handle = open(GREXT"HEAD.CGA",
		 O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open "GREXT"HEAD.CGA!");

	CA_FarRead(handle, (memptr)grstarts, (NUMCHUNKS+1)*4);

	close(handle);


#endif

//
// Open the graphics file, leaving it open until the game is finished
//
//	grhandle = open(GREXT"GRAPH."EXTENSION, O_RDONLY | O_BINARY); NOLAN
	grhandle = open("KDREAMS.CGA", O_RDONLY | O_BINARY);
	if (grhandle == -1)
		Quit ("Cannot open KDREAMS.CGA!");


//
// load the pic and sprite headers into the arrays in the data segment
//
#if NUMPICS>0
	MM_GetPtr((memptr)&pictable,NUMPICS*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPIC);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte  *)pictable,NUMPICS*sizeof(pictabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

#if NUMPICM>0
	MM_GetPtr((memptr)&picmtable,NUMPICM*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPICM);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte  *)picmtable,NUMPICM*sizeof(pictabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

#if NUMSPRITES>0
	MM_GetPtr((memptr)&spritetable,NUMSPRITES*sizeof(spritetabletype));
	CAL_GetGrChunkLength(STRUCTSPRITE);	// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte  *)spritetable,NUMSPRITES*sizeof(spritetabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

}

/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile (void)
{
	int handle,i;
	uint32_t length;
	byte  *buffer;

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef MAPHEADERLINKED
	if ((handle = open("MAPHEAD."EXTENSION,
//	if ((handle = open("KDREAMS.MAP",
		 O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open KDREAMS.MAP!");
	length = CAL_filelength(handle);
	MM_GetPtr ((memptr)&tinf,length);
	CA_FarRead(handle, tinf, length);
	close(handle);
#ifdef MAPSCOMPRESSED
//
// load mapdict.ext (huffman tree)
//
	if ((handle = open("MAPDICT."EXTENSION, O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open MAPDICT."EXTENSION"!");
	length = CAL_filelength(handle);
	//assert(length == 1020);
	CA_FarRead(handle, maphuffman, 255*sizeof(huffnode));
	close(handle);
#endif
#else

	maphuffman = (huffnode *)&mapdict;
	CAL_OptimizeNodes (maphuffman);
	tinf = (byte  *)(&maphead);

#endif

//
// open the data file
//
#if defined(MAPHEADERLINKED) || defined(MAPSCOMPRESSED)
	if ((maphandle = open("KDREAMS.MAP",
		 O_RDONLY)) == -1)
		Quit ("Can't open KDREAMS.MAP!");
#else
	if ((maphandle = open("MAPTEMP."EXTENSION,
		 O_RDONLY)) == -1)
		Quit ("Can't open MAPTEMP."EXTENSION"!");
#endif
}

//==========================================================================


/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile (void)
{
	int handle,i;
	uint32_t length;
	byte  *buffer;

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef AUDIOHEADERLINKED
#ifdef AUDIOCOMPRESSED
	if ((handle = open("SOUNDHHD."EXTENSION,
		 O_RDONLY)) == -1)
		Quit ("Can't open SOUNDHHD."EXTENSION"!");
#else
	if ((handle = open("SOUNDHED."EXTENSION,
		 O_RDONLY)) == -1)
		Quit ("Can't open SOUNDHED."EXTENSION"!");
#endif
	length = CAL_filelength(handle);
	MM_GetPtr ((memptr)&audiostarts,length);
	CA_FarRead(handle, (byte  *)audiostarts, length);
	close(handle);
#ifdef AUDIOCOMPRESSED
//
// load audiodct.ext (huffman tree)
//
	if ((handle = open("SOUNDDCT."EXTENSION, O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open SOUNDDCT."EXTENSION"!");
	length = CAL_filelength(handle);
	//assert(length >= 1020);
	CA_FarRead(handle, audiohuffman, sizeof(audiohuffman));
	close(handle);
#endif
#else
	audiohuffman = (huffnode *)&audiodict;
	CAL_OptimizeNodes (audiohuffman);
	audiostarts = (uint32_t *)(&audiohead);
#endif

//
// open the data file
//
#if !defined(AUDIOHEADERLINKED) && !defined(AUDIOCOMPRESSED)
	if ((audiohandle = open("SOUNDT."EXTENSION,
		 O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open SOUNDT."EXTENSION"!");
#else
//	if ((audiohandle = open("AUDIO."EXTENSION,	NOLAN
	if ((audiohandle = open("KDREAMS.SND",
		 O_RDONLY | O_BINARY)) == -1)
		Quit ("Can't open KDREAMS.SND!");
#endif
}

//==========================================================================


/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/

void CA_Startup (void)
{
#ifdef PROFILE
	unlink ("PROFILE.TXT");
	profilehandle = open("PROFILE.TXT", O_CREAT, S_IRUSR | S_IWUSR);
#endif

	CAL_SetupMapFile ();
	if (fakecga)
		CAL_SetupGrFileCGA();
	else
		CAL_SetupGrFile ();
	CAL_SetupAudioFile ();

	mapon = -1;
	ca_levelbit = 1;
	ca_levelnum = 0;
}

//==========================================================================


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown (void)
{
#ifdef PROFILE
	close (profilehandle);
#endif

	close (maphandle);
	close (grhandle);
}

//===========================================================================

/*
======================
=
= CA_CacheAudioChunk
=
======================
*/

void CA_CacheAudioChunk (int chunk)
{
	uint32_t pos,compressed,expanded;
	memptr	bigbufferseg;
	byte	 *source;

	if (audiosegs[chunk])
	{
		MM_SetPurge ((memptr)&audiosegs[chunk],0);
		return;							// allready in memory
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = audiostarts[chunk];
	compressed = audiostarts[chunk+1]-pos;

	lseek(audiohandle,pos,SEEK_SET);

#if !defined(AUDIOHEADERLINKED) && !defined(AUDIOCOMPRESSED)

	MM_GetPtr ((memptr)&audiosegs[chunk],compressed);
	CA_FarRead(audiohandle,audiosegs[chunk],compressed);

#else

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(audiohandle,bufferseg,compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		CA_FarRead(audiohandle,bigbufferseg,compressed);
		source = bigbufferseg;
	}

	expanded = *(uint32_t *)source;
	source += 4;			// skip over length
	MM_GetPtr ((memptr)&audiosegs[chunk],expanded);
	CAL_HuffExpand (source,audiosegs[chunk],expanded,audiohuffman);

	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
#endif
}

//===========================================================================

/*
======================
=
= CA_LoadAllSounds
=
= Purges all sounds, then loads all new ones (mode switch)
=
======================
*/

void CA_LoadAllSounds (void)
{
	unsigned	start,i;

	switch (oldsoundmode)
	{
	case sdm_Off:
		goto cachein;
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	case sdm_SoundBlaster:
	case sdm_SoundSource:
		start = STARTDIGISOUNDS;
		break;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		if (audiosegs[start])
			MM_SetPurge ((memptr)&audiosegs[start],3);		// make purgable

cachein:

	switch (SoundMode)
	{
	case sdm_Off:
		return;
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	case sdm_SoundBlaster:
	case sdm_SoundSource:
		start = STARTDIGISOUNDS;
		break;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		CA_CacheAudioChunk (start);

	oldsoundmode = SoundMode;
}

//===========================================================================

/*
======================
=
= CAL_CacheSprite
=
= Generate shifts and set up sprite structure for a given sprite
=
======================
*/

void CAL_CacheSprite (int chunk, byte  *compressed)
{
	int i;
	uint16_t shiftstarts[5];
	uint16_t smallplane,bigplane,expanded;
	spritetabletype  *spr;

#if GRMODE == CGAGR
//
// CGA has no pel panning, so shifts are never needed
//
	spr = &spritetable[chunk-STARTSPRITES];
	smallplane = spr->width*spr->height;
	MM_GetPtr (&grsegs[chunk],smallplane*2+MAXSHIFTS*6);
	dest = (spritetype  *)grsegs[chunk];
	dest->sourceoffset[0] = MAXSHIFTS*6;	// start data after 3 unsigned tables
	dest->planesize[0] = smallplane;
	dest->width[0] = spr->width;

//
// expand the unshifted shape
//
	CAL_HuffExpand (compressed, &dest->data[0],smallplane*2,grhuffman);

#endif


#if GRMODE == EGAGR

//
// calculate sizes
//
	spr = &spritetable[chunk-STARTSPRITES];

	expanded = spr->width * spr->height;
	memptr temp;
	MM_GetPtr (&temp, expanded * 5);
	MM_GetPtr (&grsegs[chunk],expanded * 8);

//
// expand the unshifted shape
//
	CAL_HuffExpand (compressed, temp, expanded * 5, grhuffman);

//
// convert to PAL8/VGA to cut down on conversion time later
//
	VW_MaskedToPAL8(temp, grsegs[chunk], 0, 0, spr->width * 8, spr->width * 8, spr->height);
	MM_FreePtr(&temp);
	switch (spr->shifts)
	{
	case	1:
	case	2:
	case	3:
	case	4:
		break;
	default:
		Quit ("CAL_CacheSprite: Bad shifts number!");
	}

#endif
}

//===========================================================================


/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk (int chunk, byte  *source)
{
	int32_t pos,compressed,expanded;
	int		next;
	spritetabletype	*spr;


	if (chunk>=STARTTILE8)
	{
	//
	// expanded sizes of tile8/16/32 are implicit
	//

#if GRMODE == EGAGR
#define BLOCK		32
#define MASKBLOCK	40
#endif

#if GRMODE == CGAGR
#define BLOCK		16
#define MASKBLOCK	32
#endif

		if (chunk<STARTTILE8M)			// tile 8s are all in one chunk!
			expanded = BLOCK*NUMTILE8;
		else if (chunk<STARTTILE16)
			expanded = MASKBLOCK*NUMTILE8M;
		else if (chunk<STARTTILE16M)	// all other tiles are one/chunk
			expanded = BLOCK*4;
		else if (chunk<STARTTILE32)
			expanded = MASKBLOCK*4;
		else if (chunk<STARTTILE32M)
			expanded = BLOCK*16;
		else
			expanded = MASKBLOCK*16;
	}
	else
	{
	//
	// everything else has an explicit size longword
	//
		expanded = *(uint32_t  *)source;
		source += 4;			// skip over length
	}

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
	if (chunk>=STARTSPRITES && chunk< STARTTILE8)
		CAL_CacheSprite(chunk,source);
	else
	{
		MM_GetPtr (&grsegs[chunk],expanded);
		CAL_HuffExpand (source,grsegs[chunk],expanded,grhuffman);
	}
}


/*
======================
=
= CAL_ReadGrChunk
=
= Gets a chunk off disk, optimizing reads to general buffer
=
======================
*/

void CAL_ReadGrChunk (int chunk)
{
	int32_t pos,compressed,expanded;
	memptr	bigbufferseg;
	byte	 *source;
	int		next;
	spritetabletype	*spr;

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = grstarts[chunk];
	if (pos<0)							// $FFFFFFFF start is a sparse tile
	  return;

	next = chunk +1;
	while (grstarts[next] == -1)		// skip past any sparse tiles
		next++;

	compressed = grstarts[next]-pos;

	lseek(grhandle,pos,SEEK_SET);

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(grhandle,bufferseg,compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		CA_FarRead(grhandle,bigbufferseg,compressed);
		source = bigbufferseg;
	}

	CAL_ExpandGrChunk (chunk,source);

	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
}

/*
======================
=
= CA_ReloadGrChunks
=
= Re-loads all presently loaded graphics chunks from disk
=
======================
*/

void CA_ReloadGrChunks ()
{
	if (fakecga)
		CAL_SetupGrFileCGA();
	else
		CAL_SetupGrFile();
	int  i = 0;
	for (i = 0; i < NUMCHUNKS; ++i)
	{
		if (grsegs[i])
			CAL_ReadGrChunk(i);
	}
}

/*
======================
=
= CA_CacheGrChunk
=
= Makes sure a given chunk is in memory, loadiing it if needed
=
======================
*/

void CA_CacheGrChunk (int chunk)
{
	int32_t pos,compressed,expanded;
	memptr	bigbufferseg;
	byte	 *source;
	int		next;

	grneeded[chunk] |= ca_levelbit;		// make sure it doesn't get removed
	if (grsegs[chunk])
	  return;							// allready in memory

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = grstarts[chunk];
	if (pos<0)							// $FFFFFFFF start is a sparse tile
	  return;

	next = chunk +1;
	while (grstarts[next] == -1)		// skip past any sparse tiles
		next++;

	compressed = grstarts[next]-pos;

	lseek(grhandle,pos,SEEK_SET);

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(grhandle,bufferseg,compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		CA_FarRead(grhandle,bigbufferseg,compressed);
		source = bigbufferseg;
	}

	CAL_ExpandGrChunk (chunk,source);

	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
}



//==========================================================================

/*
======================
=
= CA_CacheMap
=
======================
*/

void CA_CacheMap (int mapnum)
{
	int32_t pos,compressed,expanded;
	int		plane;
	memptr	*dest,bigbufferseg,buffer2seg;
	unsigned	size;
	uint16_t	*source;


//
// free up memory from last map
//
	if (mapon>-1 && mapheaderseg[mapon])
		MM_SetPurge ((memptr)&mapheaderseg[mapon],3);
	for (plane=0;plane<3;plane++)
		if (mapsegs[plane])
			MM_FreePtr ((memptr)&mapsegs[plane]);

	mapon = mapnum;


//
// load map header
// The header will be cached if it is still around
//
	if (!mapheaderseg[mapnum])
	{
		pos = ((mapfiletype	 *)tinf)->headeroffsets[mapnum];
		if (pos<0)						// $FFFFFFFF start is a sparse map
		  Quit ("CA_CacheMap: Tried to load a non existant map!");

		MM_GetPtr((memptr)&mapheaderseg[mapnum],sizeof(maptype));
		lseek(maphandle,pos,SEEK_SET);

#if defined(MAPHEADERLINKED) || defined(MAPSCOMPRESSED)
//#if BUFFERSIZE < sizeof(maptype)
//#error The general buffer size is too small!
//#endif
		//
		// load in, then unhuffman to the destination
		//
		CA_FarRead (maphandle,bufferseg,((mapfiletype	 *)tinf)->headersize[mapnum]);
		CAL_HuffExpand ((byte  *)bufferseg,
			(byte  *)mapheaderseg[mapnum],sizeof(maptype),maphuffman);
#else
		CA_FarRead (maphandle,(memptr)mapheaderseg[mapnum],sizeof(maptype));
#endif
	}
	else
		MM_SetPurge ((memptr)&mapheaderseg[mapnum],0);

//
// load the planes in
// If a plane's pointer still exists it will be overwritten (levels are
// allways reloaded, never cached)
//

	size = mapheaderseg[mapnum]->width * mapheaderseg[mapnum]->height * 2;

	for (plane = 0; plane<3; plane++)
	{
		dest = (memptr)&mapsegs[plane];
		MM_GetPtr(dest,size);

		pos = mapheaderseg[mapnum]->planestart[plane];
		compressed = mapheaderseg[mapnum]->planelength[plane];
		lseek(maphandle,pos,SEEK_SET);
		if (compressed<=BUFFERSIZE)
			source = bufferseg;
		else
		{
			MM_GetPtr(&bigbufferseg,compressed);
			source = bigbufferseg;
		}

		CA_FarRead(maphandle,(byte  *)source,compressed);
#if defined(MAPHEADERLINKED) || defined(MAPSCOMPRESSED)
		//
		// unhuffman, then unRLEW
		// The huffman'd chunk has a two byte expanded length first
		// The resulting RLEW chunk also does, even though it's not really
		// needed
		//
		expanded = *source;
		source++;
		MM_GetPtr (&buffer2seg,expanded);
		CAL_HuffExpand ((byte  *)source, buffer2seg,expanded,maphuffman);
		CA_RLEWexpand (((uint16_t  *)buffer2seg)+1,*dest,size,
		((mapfiletype  *)tinf)->RLEWtag);
		MM_FreePtr (&buffer2seg);

#else
		//
		// unRLEW, skipping expanded length
		//
		CA_RLEWexpand (source+1, *dest,size,
		((mapfiletype  *)tinf)->RLEWtag);
#endif

		if (compressed>BUFFERSIZE)
			MM_FreePtr(&bigbufferseg);
	}
}

//===========================================================================

/*
======================
=
= CA_UpLevel
=
= Goes up a bit level in the needed lists and clears it out.
= Everything is made purgable
=
======================
*/

void CA_UpLevel (void)
{
	int i;

	if (ca_levelnum==7)
		Quit ("CA_UpLevel: Up past level 7!");

//	for (i=0;i<NUMCHUNKS;i++)
//			if (grsegs[i])
//				MM_SetPurge(&grsegs[i],3);

	ca_levelbit<<=1;
	ca_levelnum++;
}

//===========================================================================

/*
======================
=
= CA_DownLevel
=
= Goes down a bit level in the needed lists and recaches
= everything from the lower level
=
======================
*/

void CA_DownLevel (void)
{
	if (!ca_levelnum)
		Quit ("CA_DownLevel: Down past level 0!");
	ca_levelbit>>=1;
	ca_levelnum--;
	CA_CacheMarks(titleptr[ca_levelnum], 1);
}

//===========================================================================

/*
======================
=
= CA_ClearMarks
=
= Clears out all the marks at the current level
=
======================
*/

void CA_ClearMarks (void)
{
	int i;

	for (i=0;i<NUMCHUNKS;i++)
		grneeded[i]&=~ca_levelbit;
}


//===========================================================================

/*
======================
=
= CA_ClearAllMarks
=
= Clears out all the marks on all the levels
=
======================
*/

void CA_ClearAllMarks (void)
{
	memset (grneeded,0,sizeof(grneeded));
	ca_levelbit = 1;
	ca_levelnum = 0;
}


//===========================================================================


/*
======================
=
= CA_CacheMarks
=
======================
*/

#define NUMBARS	(17l*8)
#define BARSTEP	8
#define MAXEMPTYREAD	1024

void CA_CacheMarks (char *title, boolean cachedownlevel)
{
	boolean dialog;
	int 	i,next,homex,homey,x,y,thx,thy,numcache,lastx,xl,xh;
	long	barx,barstep;
	int32_t	pos,endpos,nextpos,nextendpos,compressed;
	uint32_t bufferstart,bufferend;	// file position of general buffer
	byte	 *source;
	memptr	bigbufferseg;

	//
	// save title so cache down level can redraw it
	//
	titleptr[ca_levelnum] = title;

	dialog = (title!=NULL);

	if (cachedownlevel)
		dialog = false;

	if (dialog)
	{
	//
	// draw dialog window (masked tiles 12 - 20 are window borders)
	//
		US_CenterWindow (20,8);
		homex = PrintX;
		homey = PrintY;

		US_CPrint ("Loading");
		fontcolor = F_SECONDCOLOR;
		US_CPrint (title);
		fontcolor = F_BLACK;
		VW_UpdateScreen();
#ifdef PROFILE
		write(profilehandle,title,strlen(title));
		write(profilehandle,"\n",1);
#endif

	}

	numcache = 0;
//
// go through and make everything not needed purgable
//
	for (i=0;i<NUMCHUNKS;i++)
	{
		if (grneeded[i]&ca_levelbit)
		{
			if (grsegs[i])					// its allready in memory, make
				MM_SetPurge(&grsegs[i],0);	// sure it stays there!
			else
				numcache++;
		}
		else
		{
			if (grsegs[i])					// not needed, so make it purgeable
				MM_SetPurge(&grsegs[i],3);
		}
	}

	if (!numcache)			// nothing to cache!
		return;

	if (dialog)
	{
	//
	// draw thermometer bar
	//
		thx = homex + 8;
		thy = homey + 32;
		VWB_DrawTile8(thx,thy,11);
		VWB_DrawTile8(thx,thy+8,14);
		VWB_DrawTile8(thx,thy+16,17);
		VWB_DrawTile8(thx+17*8,thy,13);
		VWB_DrawTile8(thx+17*8,thy+8,16);
		VWB_DrawTile8(thx+17*8,thy+16,19);
		for (x=thx+8;x<thx+17*8;x+=8)
		{
			VWB_DrawTile8(x,thy,12);
			VWB_DrawTile8(x,thy+8,15);
			VWB_DrawTile8(x,thy+16,18);
		}

		thx += 4;		// first line location
		thy += 5;
		barx = (long)thx<<16;
		lastx = thx;
		VW_UpdateScreen();
	}

//
// go through and load in anything still needed
//
	barstep = (NUMBARS<<16)/numcache;
	bufferstart = bufferend = 0;		// nothing good in buffer now

	for (i=0;i<NUMCHUNKS;i++)
		if ( (grneeded[i]&ca_levelbit) && !grsegs[i])
		{
//
// update thermometer
//
			if (dialog)
			{
				barx+=barstep;
				xh = barx>>16;
				if (xh - lastx > BARSTEP)
				{
					for (x=lastx;x<=xh;x++)
#if GRMODE == EGAGR
						VWB_Vlin (thy,thy+13,x,fakecga?SECONDCOLOR:14);
#endif
#if GRMODE == CGAGR
						VWB_Vlin (thy,thy+13,x,SECONDCOLOR);
#endif
					lastx = xh;
					VW_UpdateScreen();
				}

			}
			pos = grstarts[i];
			if (pos<0)
				continue;

			next = i +1;
			while (grstarts[next] == -1)		// skip past any sparse tiles
				next++;

			compressed = grstarts[next]-pos;
			endpos = pos+compressed;

			if (compressed<=BUFFERSIZE)
			{
				if (bufferstart<=pos
				&& bufferend>= endpos)
				{
				// data is allready in buffer
					source = (byte  *)bufferseg+(pos-bufferstart);
				}
				else
				{
				// load buffer with a new block from disk
				// try to get as many of the needed blocks in as possible
					while ( next < NUMCHUNKS )
					{
						while (next < NUMCHUNKS &&
						!(grneeded[next]&ca_levelbit && !grsegs[next]))
							next++;
						if (next == NUMCHUNKS)
							continue;

						nextpos = grstarts[next];
						while (grstarts[++next] == -1)	// skip past any sparse tiles
							;
						nextendpos = grstarts[next];
						if (nextpos - endpos <= MAXEMPTYREAD
						&& nextendpos-pos <= BUFFERSIZE)
							endpos = nextendpos;
						else
							next = NUMCHUNKS;			// read pos to posend
					}

					lseek(grhandle,pos,SEEK_SET);
					CA_FarRead(grhandle,bufferseg,endpos-pos);
					bufferstart = pos;
					bufferend = endpos;
					source = bufferseg;
				}
			}
			else
			{
			// big chunk, allocate temporary buffer
				MM_GetPtr(&bigbufferseg,compressed);
				lseek(grhandle,pos,SEEK_SET);
				CA_FarRead(grhandle,bigbufferseg,compressed);
				source = bigbufferseg;
			}

			CAL_ExpandGrChunk (i,source);

			if (compressed>BUFFERSIZE)
				MM_FreePtr(&bigbufferseg);

		}

//
// finish up any thermometer remnants
//
		if (dialog)
		{
			xh = thx + NUMBARS;
			for (x=lastx;x<=xh;x++)
#if GRMODE == EGAGR
				VWB_Vlin (thy,thy+13,x,14);
#endif
#if GRMODE == CGAGR
				VWB_Vlin (thy,thy+13,x,SECONDCOLOR);
#endif
			VW_UpdateScreen();
		}
}

