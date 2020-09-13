/* NULL Keen Dreams Stats backend.

Copyright (C) 2015 David Gow <david@ingeniumdigital.com>

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

#include "stdint.h"
#include "kd_stat.h"

void STAT_Init()
{
}

bool STAT_IsAvailable()
{
	return false;
}

void STAT_UpdateIntStat(const char *name, int32_t stat)
{
}

void STAT_UpdateFloatStat(const char *name, float stat)
{
}

void STAT_UnlockAchievement(const char *name)
{
}

void STAT_ShowProgress(const char *name, int32_t progress, int32_t maxprogress)
{
}

void STAT_FlushStats()
{
}


KD_LeaderboardEntry kd_currentLeaderboard[16];
int kd_leaderboardSize;


void STAT_UploadIntLeaderboardStat(const char *name, int32_t stat)
{
}

bool STAT_GetGlobalLeaderboardEntries(const char *name)
{
	kd_leaderboardSize = 0;
	return false;
}
bool STAT_GetGlobalLeaderboardEntriesAroundMe(const char *name)
{
	kd_leaderboardSize = 0;
	return false;
}
bool STAT_GetFriendLeaderboardEntries(const char *name)
{
	kd_leaderboardSize = 0;
	return false;
}

// On-screen keyboard functions.
bool STAT_IsKbdAvailable()
{
	return false;
}

void STAT_BeginTextEntry(char *buffer, int buflen, char *description)
{
	// STUB
}

bool STAT_IsTextEntryDone()
{
	return false;
}

bool STAT_GetText(char *buffer, int buflen)
{
	return false;
}


