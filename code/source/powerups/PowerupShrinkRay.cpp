/*
 * TetriCycle
 * Copyright (C) 2009, 2010 Cale Scholl
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

/** @file PowerupShrinkRay.cpp
 * @author Cale Scholl / calvinss4
 */

#include "PowerupShrinkRay.h"

#include "Player.h"        // for Player
#include "libwiigui/gui.h" // for GuiImageData

extern Player *g_players; ///< the player instances

#define SHRINK_RAY_SCALE 2.8

PowerupId PowerupShrinkRay::powerupId;
Powerup *PowerupShrinkRay::instance = new PowerupShrinkRay();

GuiImageData *PowerupShrinkRay::imageData = 
  new GuiImageData(powerup_shrinkray_png);

GuiSound *PowerupShrinkRay::sound = 
  new GuiSound(powerup_shrinkray_pcm, powerup_shrinkray_pcm_size, SOUND_PCM);

string PowerupShrinkRay::helpText[2] = 
  {"Shrink Ray", "Shrinks the target player's playfield. "
                 "Use this on people who have poor vision."};

void PowerupShrinkRay::StartEffect(u8 player)
{
  g_players[player].gameData.powerupData.playfieldScale = SHRINK_RAY_SCALE;
}

/// Allow more than one PowerupShrinkRay to be in effect at the same time.
/**
 * This function assumes this powerup has already been removed from the powerup 
 * effect queue. If another PowerupShrinkRay is in the queue then allow that one 
 * to stop the effect.
 */
void PowerupShrinkRay::StopEffect(u8 player)
{
  for (int i = 0; i < MAX_POWERUP_EFFECTS; ++i)
  {
    Powerup *powerup = g_players[player].gameData.powerupEffects[i];
    if (powerup && powerup->GetPowerupId() == powerupId)
    {
      return;
    }
  }
  
  g_players[player].gameData.powerupData.playfieldScale = 0;
}