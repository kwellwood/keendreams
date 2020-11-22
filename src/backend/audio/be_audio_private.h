/* Copyright (C) 2014-2020 NY00123
 *
 * This file is part of the Reflection Keen back-end.
 *
 * The Reflection Keen back-end is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * The Reflection Keen back-end is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the Reflection Keen back-end. If not,
 * see <https://www.gnu.org/licenses/>.
 */

#ifndef BE_AUDIO_PRIVATE
#define BE_AUDIO_PRIVATE

#include "refkeen_config.h"
#include <stdbool.h>
#include <stdint.h>

#define PC_PIT_RATE 1193182

#define OPL_SAMPLE_RATE 49716

#if (defined REFKEEN_RESAMPLER_LIBSAMPLERATE) || (defined REFKEEN_RESAMPLER_LIBRESAMPLE)
#define MIXER_SAMPLE_FORMAT_FLOAT
#else
#define MIXER_SAMPLE_FORMAT_SINT16
#endif

#ifdef MIXER_SAMPLE_FORMAT_FLOAT
typedef float BE_ST_SndSample_T;
#elif (defined MIXER_SAMPLE_FORMAT_SINT16)
typedef int16_t BE_ST_SndSample_T;
#endif

extern bool g_sdlEmulatedOPLChipReady;

void BEL_ST_ResetOPLChip(void);

bool BEL_ST_InitAudioSubsystem(int *freq, int *bufferLen);
void BEL_ST_ShutdownAudioSubsystem(void);
void BEL_ST_StartAudioSubsystem(void);

/*** Use these functions for adding audio data to the stream ***/
void BEL_ST_GenPCSpeakerSamples(BE_ST_SndSample_T *stream, int length);
void BEL_ST_GenOPLSamples(BE_ST_SndSample_T *stream, int length);
void BEL_ST_GenDigiSamples(BE_ST_SndSample_T *stream, int length);

void BEL_ST_SetPCSpeakerSampleRate(int rate);

struct BE_ST_AudioMixerSource;
void BEL_ST_SetOPLMixerSource(struct BE_ST_AudioMixerSource *src);
void BEL_ST_SetDigiMixerSource(struct BE_ST_AudioMixerSource *src);

#endif
