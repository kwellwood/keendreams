/*
 * Keen Dreams SDL2 / Steam port
 * Copyright (C) 2015 David Gow <david@ingeniumdigital.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "id_heads.h"

#define	SDL_SoundFinished() {SoundNumber = (word)0; SoundPriority = 0;}

#define PC_PIT_RATE 1193182
#define SD_SFX_PART_RATE 140
/* In the original exe, upon setting a rate of 140Hz or 560Hz for some
 * interrupt handler, the value 1192030 divided by the desired rate is
 * calculated, to be programmed for timer 0 as a consequence.
 * For THIS value, it is rather 1193182 that should be divided by it, in order
 * to obtain a better approximation of the actual rate.
 */
#define SD_SOUND_PART_RATE_BASE 1192030

// Global variables (TODO more to add)
boolean AdLibPresent, NeedsMusic;
SDMode SoundMode;
SMMode MusicMode;
uint16_t SoundPriority, DigiPriority;
// Internal variables (TODO more to add)
static boolean SD_Started;
word SoundNumber,DigiNumber;
uint8_t **SoundTable;

// PC Sound variables
uint8_t pcLastSample, *pcSound;
uint32_t pcLengthLeft;
uint16_t pcSoundLookup[255];

boolean quiet_sfx;

// WARNING: These vars refer to the libSDL library!!!
SDL_AudioSpec SD_SDL_AudioSpec;
static boolean SD_SDL_AudioSubsystem_Up;
static uint32_t SD_SDL_SampleOffsetInSound, SD_SDL_SamplesPerPart/*, SD_SDL_MusSamplesPerPart*/;
static int SD_CurrentSoundChunk;


// Timing stuff from Omnispeak (thanks NY00123).
longword	TimeCount;					// Global time in ticks
uint16_t SpriteSync = 0;
// PIT timer divisor, scaled (bt 8 if music is on, 2 otherwise)
int16_t ScaledTimerDivisor;
// A few variables used for timing measurements (PC_PIT_RATE units per second)
uint64_t SD_LastPITTickTime;

uint32_t SD_GetTimeCount(void)
{
	// FIXME: What happens when SDL_GetTicks() reaches the upper bound?
	// May be challenging to fix... A proper solution should
	// only work with *differences between SDL_GetTicks values*.
	uint64_t currPitTicks = (uint64_t)(SDL_GetTicks()) * SD_SOUND_PART_RATE_BASE / 1000;
	uint32_t ticksToAdd = (currPitTicks - SD_LastPITTickTime) / ScaledTimerDivisor;
	SD_LastPITTickTime += ticksToAdd * ScaledTimerDivisor;
	TimeCount += ticksToAdd;
	return TimeCount;
}

void SD_SetTimeCount(uint32_t newval)
{
	TimeCount = newval;
}

int32_t SD_GetLastTimeCount(void) { return lasttimecount; }
void SD_SetLastTimeCount(int32_t newval) { lasttimecount = newval; }
uint16_t SD_GetSpriteSync(void) { return SpriteSync; }
void SD_SetSpriteSync(uint16_t newval) { SpriteSync = newval; }


void SDL_t0Service(void);

int16_t soundPrev = 0, soundDiff = 0;

static SDL_mutex *sd_mutex;

// Custom DPCM decoder (cut filesize in half)
void SD_SDL_DecodeSound(int16_t *dest, int16_t *src, int lenInSamples)
{
	while (lenInSamples--)
	{
		*dest = (*src + soundPrev) + soundDiff;
		soundDiff = *dest - soundPrev;
		soundPrev = *dest;
		src++;
		dest++;
	}
}


void SD_SDL_CallBack(void *unused, Uint8 *stream, int len)
{
	int16_t *currSamplePtr = (int16_t *)stream;
	uint32_t currNumOfSamples;
	boolean isPartCompleted;
#if SDL_VERSION_ATLEAST(1,3,0)
	memset(stream, 0, len);
#endif
	SDL_LockMutex(sd_mutex);
	if (!SoundPriority)
	{
		SDL_UnlockMutex(sd_mutex);
		return;
	}
	
	uint32_t length = SDL_SwapLE32(*(uint32_t *)(SoundTable[SoundNumber])) + 6;
	int lenToCopy = SDL_min(len, length - SD_SDL_SampleOffsetInSound);
	SD_SDL_DecodeSound(stream, SoundTable[SoundNumber] + SD_SDL_SampleOffsetInSound, lenToCopy/2);
	SD_SDL_SampleOffsetInSound += lenToCopy;
	if (SD_SDL_SampleOffsetInSound >= length)
	{
		SoundNumber = 0;
		SoundPriority = 0;
	}
	SDL_UnlockMutex(sd_mutex);
}

void SDL_SetTimer0(int16_t int_8_divisor)
{
	SD_SDL_SamplesPerPart = (int32_t)int_8_divisor * SD_SDL_AudioSpec.freq / PC_PIT_RATE;
	ScaledTimerDivisor = int_8_divisor;
}

void SDL_SetIntsPerSecond(int16_t tickrate)
{
	SDL_SetTimer0(((int32_t)SD_SOUND_PART_RATE_BASE / (int32_t)tickrate) & 0xFFFF);
}


/* NEVER call this from the callback!!! */
static boolean SDL_DetectAdlib(boolean assumepresence)
{
	return true;
}

void SDL_ShutDevice(void)
{
	SoundMode = sdm_Off;
}

void SDL_CleanDevice(void)
{
}

void SDL_StartDevice(void)
{
	SoundNumber = (word)0; SoundPriority = 0;
}

void SDL_SetTimerSpeed(void)
{
	int16_t scaleFactor = (MusicMode == smm_AdLib) ? 4 : 1;
	SDL_SetIntsPerSecond(SD_SFX_PART_RATE * scaleFactor);
	ScaledTimerDivisor *= (2*scaleFactor);
}

void SD_StopSound(void);

boolean SD_SetSoundMode(SDMode mode)
{
	boolean any_sound; // FIXME: Should be set to false here?
	int16_t offset; // FIXME: Should be set to 0 here?
	SD_StopSound();
	switch (mode)
	{
		case sdm_Off:
			any_sound = true;
			break;
		case sdm_PC:
			offset = 0;
			any_sound = true;
			break;
		case sdm_AdLib:
			if (!AdLibPresent)
			{
				break;
			}
			offset = NUMSOUNDS;
			any_sound = true;
			break;
		default:
			any_sound = false;
	}
	if (any_sound && (mode != SoundMode))
	{
		SDL_ShutDevice();
		SoundMode = mode;
		/* TODO: Is that useful? */
		SoundTable = audiosegs + offset;
		SDL_StartDevice();
	}
	SDL_SetTimerSpeed();
	return any_sound;
}

boolean SD_MusicPlaying(void);
void SD_FadeOutMusic(void);

boolean SD_SetMusicMode(SMMode mode)
{
	boolean result; // FIXME: Should be set to false here?
	SD_FadeOutMusic();
	while (SD_MusicPlaying())
	{
		// The original code simply waits in a busy loop.
		// Bad idea for new code.
		// TODO: What about checking for input/graphics/other status?
		SDL_Delay(1);
	}
	switch (mode)
	{
	case smm_Off:
		NeedsMusic = 0;
		result = true;
		break;
	case smm_AdLib:
		if (AdLibPresent)
		{
			NeedsMusic = 1;
			result = true;
		}
		break;
	default:
		result = false;
	}
	if (result)
		MusicMode = mode;
	SDL_SetTimerSpeed();
	return result;
}

void SD_Startup(void)
{
	/****** TODO: FINISH! ******/

	if (SD_Started)
	{
		return;
	}
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		SD_SDL_AudioSubsystem_Up = false;
	}
	else
	{
		SD_SDL_AudioSpec.freq = 44100; // PCM rate
		SD_SDL_AudioSpec.format = AUDIO_S16;
		SD_SDL_AudioSpec.channels = 1;
		// Under wine, small buffer sizes cause a lot of crackling, so we double the
		// buffer size. This will result in a tiny amount (~10ms) of extra lag on windows,
		// but it's a price I'm prepared to pay to not have my ears explode.
#ifdef _WIN32
		SD_SDL_AudioSpec.samples = 1024;
#else
		SD_SDL_AudioSpec.samples = 512;
#endif
		SD_SDL_AudioSpec.callback = SD_SDL_CallBack;
		SD_SDL_AudioSpec.userdata = NULL;
		if (SDL_OpenAudio(&SD_SDL_AudioSpec, NULL))
		{
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
			SD_SDL_AudioSubsystem_Up = false;
		}
		else
		{
#if 0
			// TODO: This depends on music on/off? (560Hz vs 140Hz for general interrupt handler)
			SD_SDL_SamplesPerPart = ((uint64_t)SD_SOUND_PART_RATE_BASE / SD_SFX_PART_RATE) * SD_SDL_AudioSpec.freq / PC_PIT_RATE;
			SD_SDL_MusSamplesPerPart = ((uint64_t)SD_SOUND_PART_RATE_BASE / (4*SD_SFX_PART_RATE)) * SD_SDL_AudioSpec.freq / PC_PIT_RATE;
#endif
			SD_SDL_AudioSubsystem_Up = true;
		}

	}

	SD_SetTimeCount(0);

	SD_SetSoundMode(sdm_Off);
	SD_SetMusicMode(smm_Off);
	
	sd_mutex = SDL_CreateMutex();

	AdLibPresent = SDL_DetectAdlib(true);


	if (SD_SDL_AudioSubsystem_Up)
	{
		SDL_PauseAudio(0);
	}
	SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Default() - Sets up the default behaviour for the Sound Mgr whether
//		the config file was present or not.
//
///////////////////////////////////////////////////////////////////////////
void
SD_Default(boolean gotit,SDMode sd,SMMode sm)
{
	boolean	gotsd,gotsm;

	gotsd = gotsm = gotit;

	if (gotsd)	// Make sure requested sound hardware is available
	{
		switch (sd)
		{
		case sdm_AdLib:
			gotsd = AdLibPresent;
			break;
		}
	}
	if (!gotsd)
	{
		if (AdLibPresent)
			sd = sdm_AdLib;
		else
			sd = sdm_PC;
	}
	if (sd != SoundMode)
		SD_SetSoundMode(sd);


	if (gotsm)	// Make sure requested music hardware is available
	{
		switch (sm)
		{
		case sdm_AdLib:
			gotsm = AdLibPresent;
			break;
		}
	}
	if (!gotsm)
	{
		if (AdLibPresent)
			sm = smm_AdLib;
	}
	if (sm != MusicMode)
		SD_SetMusicMode(sm);
}

void SD_MusicOff(void);

void SD_Shutdown(void)
{
	if (!SD_Started)
	{
		return;
	}
	if (SD_SDL_AudioSubsystem_Up)
	{
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		SD_SDL_AudioSubsystem_Up = false;
	}
	SD_MusicOff();
	//SD_StopSound();
	SDL_ShutDevice();
	SDL_CleanDevice();

	SDL_DestroyMutex(sd_mutex);
	// Some timer stuff not done here

	SD_Started = false;
}

void SD_PlaySound(word sound)
{
	uint32_t length;
	uint16_t priority;
	if (SoundMode == sdm_Off)
		return;
	if (!SoundTable[sound])
		Quit("SD_PlaySound() - Uncached sound");
	length = SDL_SwapLE32(*(uint32_t *)(SoundTable[sound]));
	if (!length)
		Quit("SD_PlaySound() - Zero length sound");
	priority = SDL_SwapLE16(*(uint16_t *)(SoundTable[sound] + 4));
	if (priority < SoundPriority)
	{
		return;
	}
	SDL_LockMutex(sd_mutex);
	SD_SDL_SampleOffsetInSound = 6;
	SoundNumber = sound;
	SoundPriority = priority;
	soundPrev = soundDiff = 0;
	SDL_UnlockMutex(sd_mutex);
	//printf("Playing sound %d, length %d (%d secs), prio %d\n", sound, length, length / (44100*2), priority);
}

uint16_t SD_SoundPlaying(void)
{
	return SoundNumber;
}

void SD_StopSound(void)
{
	if (!SD_Started)
		return;
	SDL_LockMutex(sd_mutex);
	SDL_SoundFinished();
	SDL_UnlockMutex(sd_mutex);
}

void SD_WaitSoundDone(void)
{
	while (SoundPriority)
	{
		// The original code simply waits in a busy loop.
		// Bad idea for new code.
		// TODO: What about checking for input?
		VW_GL_Present();
	}
}

void SD_MusicOn(void)
{
}

/* NEVER call this from the callback!!! */
void SD_MusicOff(void)
{
}

void SD_StartMusic(MusicGroup *music)
{
}

void SD_FadeOutMusic(void)
{
}

boolean SD_MusicPlaying(void)
{
	return false; // All it really does...
}
