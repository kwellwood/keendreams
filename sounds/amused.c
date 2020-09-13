// aMUSEd: Packer for MUSE-style files filled with PCM-y goodness.


#include <SDL.h>
#include <stdint.h>
#include <stdbool.h>

#define MAXCHUNKS 512

uint32_t header[MAXCHUNKS];
bool compressed = true;
FILE *datafile;
int current_chunk = 0;

// from jhuff.c
void CountBytes (unsigned char *start, long length);
void Huffmanize (void);
long HuffCompress (unsigned char *source, long length,
  unsigned char *dest);
typedef struct
{
  unsigned short bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;
extern huffnode nodearray[256];	// 256 nodes is worst case

void AddChunk(size_t size, void *data)
{
	header[current_chunk++] = ftell(datafile);
	if (!size) return;
	if (!compressed)
	{
		fwrite(data, size, 1, datafile);
	}
	else
	{
		fwrite(&size, 4, 1, datafile);
		uint8_t *compchunk = malloc(size * 2);
		uint32_t complen = HuffCompress(data, size, compchunk);
		printf("Huffman'd %d bytes -> %d bytes (%d%%)\n", size, complen, complen * 100 / size);
		fwrite(compchunk, complen, 1, datafile);
	}
	header[current_chunk] = ftell(datafile);
}

void EncodeSound(int16_t *dest, int16_t *src, int lenInSamples)
{
	int16_t dcoffset = src[lenInSamples - 1];
	int16_t prev = 0, diff = 0;
	while (lenInSamples--)
	{
		*src -= dcoffset;
		*dest = (*src - prev) - diff;
		diff = *src - prev;
		prev = *src;
		src++;
		dest++;
	}
	printf("dc offset %hd ", dcoffset);
}

void WriteSoundHead(const char *filename)
{
	FILE *hed = fopen(filename, "wb");
	fwrite(header, current_chunk + 1, 4, hed);
	fclose(hed);
	if (compressed)
	{
		FILE *dct = fopen("SOUNDDCT.KDR", "wb");
		fwrite(nodearray, sizeof(nodearray), 1, dct);
		fclose(dct);
	}
}

void AddWaveChunk(const char *filename, int prio)
{
	SDL_AudioSpec wav_spec;
	uint32_t wav_length;
	uint8_t *wav_data;
	if (!SDL_LoadWAV(filename, &wav_spec, &wav_data, &wav_length))
	{
		fprintf(stderr, "Error loading WAV file: %s.\n Error: \"%s\" Inserting silence.\n", filename, SDL_GetError());
		AddChunk(0, 0);
		return;
	}
	uint8_t *chunk_data = malloc(wav_length + 6);
	*(uint32_t*)chunk_data = wav_length;
	*(uint16_t*)(chunk_data + 4) = prio;
	printf("File \"%s\": ", filename);
	EncodeSound(chunk_data + 6, wav_data, wav_length/2);
	AddChunk(wav_length + 6, chunk_data);
	SDL_FreeWAV(wav_data);
}

void Begin(const char *filename)
{
	datafile = fopen(filename, "wb");
}

void End()
{
	fclose(datafile);
	if (compressed)
	{
		WriteSoundHead("SOUNDHHD.KDR");
	}
	else
	{
		WriteSoundHead("SOUNDHED.KDR");
	}
	printf("Packed %d sound chunks.\n", current_chunk);
}
	
int main(int argc, char **argv)
{
	if (compressed)
	{
		FILE *uncompfile = fopen("SOUNDT.KDR", "rb");
		unsigned char blah[6146396];
		fread(blah, 1, 6146396, uncompfile);
		CountBytes(blah, 6146396);
		fclose(uncompfile);
		Huffmanize();
		Begin("KDREAMS.SND");
	}
	else
		Begin("SOUNDT.KDR");
	int startpcsounds = current_chunk;
	AddWaveChunk("pc/00KEENWALK1.wav", 10);
	AddWaveChunk("pc/01KEENWALK2.wav", 10);
	AddWaveChunk("pc/02JUMP.wav", 20);
	AddWaveChunk("pc/03LAND.wav", 30);
	AddWaveChunk("pc/04THROW.wav", 50);
	AddWaveChunk("pc/05DIVE.wav", 50);
	AddWaveChunk("pc/06GETPOWER.wav", 80);
	AddWaveChunk("pc/07GETPOINTS.wav", 80);
	AddWaveChunk("pc/08GETBOMB.wav", 90);
	AddWaveChunk("pc/09FLOWERPOWER.wav", 80);
	AddWaveChunk("pc/10UNFLOWERPOWER.wav", 40);
	AddWaveChunk("pc/11ENTERLEVEL.wav", 99);
	AddWaveChunk("pc/12WAKEUP.wav", 99);
	AddWaveChunk("pc/13LEVELDONE.wav", 99);
	AddWaveChunk("pc/14NOWAY.wav", 80);
	AddWaveChunk("pc/15HITHEAD.wav", 50);
	AddWaveChunk("pc/16TATERSWING.wav", 60);
	AddWaveChunk("pc/17BOUNCE.wav", 40);
	AddWaveChunk("pc/18EXTRAKEEN.wav", 80);
	AddWaveChunk("pc/19OPENDOOR.wav", 60);
	AddWaveChunk("pc/20THROWBOMB.wav", 80);
	AddWaveChunk("pc/21BOMBBOOM.wav", 80);
	AddWaveChunk("pc/22BOOBUSGONE.wav", 99);
	AddWaveChunk("pc/23GETKEY.wav", 80);
	AddWaveChunk("pc/24GRAPESCREAM.wav", 80);
	AddWaveChunk("pc/25PLUMMET.wav", 50);
	AddWaveChunk("pc/26CLICK.wav", 0);
	AddWaveChunk("pc/27TICK.wav", 0);
	int startadlibsounds = current_chunk;
	AddWaveChunk("adlib/00KEENWALK1.wav", 10);
	AddWaveChunk("adlib/01KEENWALK2.wav", 10);
	AddWaveChunk("adlib/02JUMP.wav", 20);
	AddWaveChunk("adlib/03LAND.wav", 30);
	AddWaveChunk("adlib/04THROW.wav", 50);
	AddWaveChunk("adlib/05DIVE.wav", 50);
	AddWaveChunk("adlib/06GETPOWER.wav", 80);
	AddWaveChunk("adlib/07GETPOINTS.wav", 80);
	AddWaveChunk("adlib/08GETBOMB.wav", 90);
	AddWaveChunk("adlib/09FLOWERPOWER.wav", 80);
	AddWaveChunk("adlib/10UNFLOWERPOWER.wav", 40);
	AddWaveChunk("adlib/11ENTERLEVEL.wav", 99);
	AddWaveChunk("adlib/12WAKEUP.wav", 99);
	AddWaveChunk("adlib/13LEVELDONE.wav", 99);
	AddWaveChunk("adlib/14NOWAY.wav", 80);
	AddWaveChunk("adlib/15HITHEAD.wav", 50);
	AddWaveChunk("adlib/16TATERSWING.wav", 60);
	AddWaveChunk("adlib/17BOUNCE.wav", 40);
	AddWaveChunk("adlib/18EXTRAKEEN.wav", 80);
	AddWaveChunk("adlib/19OPENDOOR.wav", 60);
	AddWaveChunk("adlib/20THROWBOMB.wav", 80);
	AddWaveChunk("adlib/21BOMBBOOM.wav", 80);
	AddWaveChunk("adlib/22BOOBUSGONE.wav", 99);
	AddWaveChunk("adlib/23GETKEY.wav", 80);
	AddWaveChunk("adlib/24GRAPESCREAM.wav", 80);
	AddWaveChunk("adlib/25PLUMMET.wav", 50);
	AddWaveChunk("adlib/26CLICK.wav", 0);
	AddWaveChunk("adlib/27TICK.wav", 0);
	int startmusic = current_chunk;
	End();
}