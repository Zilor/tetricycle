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

/** @file PowerupMirror.cpp
 * @author Cale Scholl
 */

#include "PowerupMirror.h"

#include "Player.h"        // for Player
#include "libwiigui/gui.h" // for GuiImageData

extern Player *g_players; ///< the player instances

PowerupId PowerupMirror::powerupId;
Powerup *PowerupMirror::instance = new PowerupMirror();

GuiImageData *PowerupMirror::imageData = 
  new GuiImageData(powerup_mirror_png);

string PowerupMirror::helpText[2] = 
  {"Mirror", "A defensive powerup that reflects powerup attacks."};

// OPTIONAL:
//GuiSound *PowerupMirror::sound = 
//  new GuiSound(powerup_mirror_pcm, powerup_mirror_pcm_size, SOUND_PCM);

void PowerupMirror::StartEffect(u8 player)
{
  ++g_players[player].gameData.powerupData.mirrorCtr;
}

void PowerupMirror::StopEffect(u8 player)
{
  --g_players[player].gameData.powerupData.mirrorCtr;
}