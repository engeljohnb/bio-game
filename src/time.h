/*
    Bio-Game is a game for designing your own organism. 
    Copyright (C) 2022 John Engel 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __TIME_H__
#define __TIME_H__

#include <SDL2/SDL.h>
#include "common.h"

#define MINUTES_PER_IN_GAME_DAY 5
#define MINUTES_PER_IN_GAME_HOUR (MINUTES_PER_IN_GAME_DAY/24.0f)
#define SECONDS_PER_IN_GAME_HOUR (MINUTES_PER_IN_GAME_HOUR*60)
#define SECONDS_PER_IN_GAME_DAY  (MINUTES_PER_IN_GAME_DAY*60)

/* B_SUNRISE_TIME = 6 in-game hours after midnight. You can think of midnight as being 0,
 * but some of the math only works when midnight is treated as 24. */
#define B_SUNRISE_TIME 6.0f
#define B_MIDDAY_TIME 12.0f
#define B_SUNSET_TIME 18.0f
#define B_MIDNIGHT_TIME 24.0f

enum SUN_POSITIONS
{
	B_SUNRISE,
	B_MIDDAY,
	B_SUNSET,
	B_MIDNIGHT,
};

enum TIME_OF_DAY
{
	B_MORNING,
	B_AFTERNOON,
	B_EVENING,
	B_NIGHT,
};

void B_keep_time(int target_period);
void B_stopwatch(char *message);
float B_get_frame_time(void);
double B_get_current_second(void);
double B_get_current_minute(void);
double B_get_current_playtime_hour(void);
double B_get_current_in_game_hour(void);
double B_get_seconds_into_current_day(void);
	
#endif
