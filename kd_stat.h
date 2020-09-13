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
#include "stdint.h"
#include "stdbool.h"


// The player has been cheating, etc. Disable stats.
void STAT_Invalidate();

// Started a new map, enable per-map stats if cheats disabled.
void STAT_ValidateMap();

// Started a new game, enable all stats if cheats disabled.
void STAT_ValidateGame();

// Player disabled CGA mode: prevent from getting CGA achievements. 
void STAT_InvalidateCGA();

// Player completed a map: update stats, leaderboards and achievements.
void STAT_FinishMap();

// Player completed the game: update stats, leaderboards and achievements.
void STAT_FinishGame();

// --- Functions below this line are part of the core stats subsystem and must
// --- be implemented by a stats plugin (e.g. Steam)


typedef struct KD_LeaderboardEntry
{
	int32_t rank;
	int32_t score;
	const char *name;
	unsigned int isplayer;
} KD_LeaderboardEntry;

extern KD_LeaderboardEntry kd_currentLeaderboard[16];

extern int kd_leaderboardSize;

// Initialise the stats subsystem.
void STAT_Init();

// Is the stats backend available?
bool STAT_IsAvailable();

// Gamepad text input.
bool STAT_IsKbdAvailable();
void STAT_BeginTextEntry(char *buffer, int buflen, char *description);
bool STAT_IsTextEntryDone();
bool STAT_GetText(char *buffer, int buflen);

// Update a per-user integer statistic.
void STAT_UpdateIntStat(const char *name, int32_t stat);

// Update a per-user floating-point statistic.
void STAT_UpdateFloatStat(const char *name, float stat);

// Unlock an achievement.
void STAT_UnlockAchievement(const char *name);

// Mark significant progress on an achievement.
void STAT_ShowProgress(const char *name, int32_t progress, int32_t maxprogress);

// Flush any changed stats to the stats provider (e.g. Steam).
void STAT_FlushStats();

// Upload an integer statistic for the given Leaderboard.
void STAT_UploadIntLeaderboardStat(const char *name, int32_t stat);

// Get the top 10 scores from a given Leaderboard.
bool STAT_GetGlobalLeaderboardEntries(const char *name);

// Get 10 scores from around the player's rank from the given Leaderboard. 
bool STAT_GetGlobalLeaderboardEntriesAroundMe(const char *name);

// Get leaderboard entries from the player's "friends".
bool STAT_GetFriendLeaderboardEntries(const char *name);
