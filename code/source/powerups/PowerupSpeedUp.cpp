/*
 * TetriCycle
 * Copyright (C) 2010 Cale Scholl
 *
 * This file is part of TetriCycle.
 *
 * TetriCycle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TetriCycle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with TetriCycle.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @file PowerupSpeedUp.cpp
 * @author Cale Scholl
 */

#include "PowerupSpeedUp.h"

#include "Player.h"        // for Player
#include "libwiigui/gui.h" // for GuiImageData

extern Player *g_players; ///< the player instances

PowerupId PowerupSpeedUp::powerupId;
Powerup *PowerupSpeedUp::instance = new PowerupSpeedUp();

GuiImageData *PowerupSpeedUp::imageData = 
  new GuiImageData(powerup_speedup_png);

string PowerupSpeedUp::helpText[2] = 
  {"Speed Up", "A powerup that temporarily increases the level (speed) of the target player."};

// OPTIONAL:
//GuiSound *PowerupSpeedUp::sound = 
//  new GuiSound(powerup_speedup_pcm, powerup_speedup_pcm_size, SOUND_PCM);

#define SPEED_INCREMENT 2

void PowerupSpeedUp::StartEffect(u8 player)
{
  g_players[player].gameData.level += SPEED_INCREMENT;
  g_players[player].gameData.speed = START_SPEED - 3 * g_players[player].gameData.level;
  if (g_players[player].gameData.speed <= 0)
    g_players[player].gameData.speed = 1;
}

void PowerupSpeedUp::StopEffect(u8 player)
{
  g_players[player].gameData.level -= SPEED_INCREMENT;
  g_players[player].gameData.speed = START_SPEED - 3 * g_players[player].gameData.level;
  if (g_players[player].gameData.speed <= 0)
    g_players[player].gameData.speed = 1;
}