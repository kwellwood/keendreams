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
#include "kd_def.h"
#include "stdint.h"
#include "stdbool.h"
#include "kd_stat.h"

// Is the playthrough of the current game valid?
bool stat_gameValid = true;
// Is the playthrough of the current map valid?
bool stat_mapValid = true;
// Start time of the game
uint32_t stat_gameStart = 0;
// Start time of the map
uint32_t stat_mapStart = 0;

int32_t stat_mapStartScore = 0;
bool stat_gameCGA = false;

void STAT_Invalidate()
{
	stat_gameValid = false;
	stat_mapValid = false;
}

void STAT_ValidateMap()
{
	if (godmode || singlestep || jumpcheat || tedlevel)
		stat_mapValid = stat_gameValid = false;
	else
		stat_mapValid = true;

	stat_mapStartScore = gamestate.score;

	stat_mapStart = SDL_GetTicks();
}

void STAT_ValidateGame()
{
	stat_mapValid = stat_gameValid = true;
	stat_gameCGA = fakecga;
	stat_gameStart = SDL_GetTicks();
}

void STAT_InvalidateCGA()
{
	stat_gameCGA = false;
}

const char *mapTimeStats[17] =
{
	NULL,
	"Map1Time",
	"Map2Time",
	"Map3Time",
	"Map4Time",
	"Map5Time",
	NULL,
	"Map7Time",
	NULL,
	"Map9Time",
	"Map10Time",
	NULL,
	"Map12Time",
	NULL,
	"Map14Time",
	NULL,
	"Map16Time"
};

const char *mapScoreStats[17] =
{
	NULL,
	"Map1Score",
	"Map2Score",
	"Map3Score",
	"Map4Score",
	"Map5Score",
	NULL,
	"Map7Score",
	NULL,
	"Map9Score",
	"Map10Score",
	NULL,
	"Map12Score",
	NULL,
	"Map14Score",
	NULL,
	"Map16Score"
};

const char *mapScoreAchs[17] =
{
	NULL,
	"TopMap1Score",
	"TopMap2Score",
	"TopMap3Score",
	"TopMap4Score",
	"TopMap5Score",
	NULL,
	"TopMap7Score",
	NULL,
	"TopMap9Score",
	"TopMap10Score",
	NULL,
	"TopMap12Score",
	NULL,
	"TopMap14Score",
	NULL,
	"TopMap16Score"
};

int mapScoreMap[16] =
{
	8800,
	14900,
	10900,
	14000,
	13700,
	55300,
	38000,
	55300,
	22300,
	6700,
	55300,
	15000,
	55300,
	4300,
	0,
	45800
};

void STAT_FinishMap()
{
	int32_t mapScore = gamestate.score - stat_mapStartScore;
	uint32_t mapTime = SDL_GetTicks() - stat_mapStart;
	if (!stat_mapValid)
	{
		return;
	}
	if (mapScoreStats[mapon])
	{
		STAT_UpdateIntStat(mapScoreStats[mapon], mapScore);
		STAT_UploadIntLeaderboardStat(mapScoreStats[mapon], mapScore);
		STAT_UploadIntLeaderboardStat(mapTimeStats[mapon], mapTime);
		// Set this as the next leaderboard to show.
		US_SetLeaderboard(mapScoreStats[mapon]);

		if (mapScore >= mapScoreMap[mapon])
		{
			STAT_UnlockAchievement(mapScoreAchs[mapon]);
		}
		else if (mapScore * 2 >= mapScoreMap[mapon])
		{
			STAT_ShowProgress(mapScoreAchs[mapon], mapScore, mapScoreMap[mapon]);
		}
	}
	if (gamestate.boobusbombs == 24)
	{
		STAT_UnlockAchievement("BoobusBombs");
	}
	STAT_FlushStats();
}

void STAT_FinishGame()
{
	uint32_t gameTime = SDL_GetTicks() - stat_gameStart;
	if (!stat_mapValid || !stat_gameValid)
	{
		return;
	}
	if (stat_gameCGA)
	{
		STAT_UnlockAchievement("GameCGA");
	}

	switch (gamestate.difficulty)
	{
	case gd_Easy:
		STAT_UnlockAchievement("GameEasy");
		break;
	case gd_Normal:
		STAT_UnlockAchievement("GameNormal");
		break;
	case gd_Hard:
		STAT_UnlockAchievement("GameHard");
		break;
	}
	STAT_UpdateIntStat("GameScore", gamestate.score);
	STAT_UploadIntLeaderboardStat("GameTime", gameTime);
	STAT_UploadIntLeaderboardStat("GameScore", gamestate.score);
	US_SetLeaderboard("GameScore");
	STAT_FlushStats();
}
