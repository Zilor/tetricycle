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

/** @file PowerupBigHand.cpp
 * @author Cale Scholl / calvinss4
 */

#include "PowerupBigHand.h"

#include "Player.h"        // for Player
#include "libwiigui/gui.h" // for GuiImageData

extern Player *g_players; ///< the player instances

PowerupId PowerupBigHand::powerupId;
Powerup *PowerupBigHand::instance = new PowerupBigHand();

GuiImageData *PowerupBigHand::imageData = 
  new GuiImageData(powerup_bighand_png);

string PowerupBigHand::helpText[2] = 
  {"Big Hand", "Hey! Your hand's in the way!"};

void PowerupBigHand::StartEffect(u8 player)
{
  g_players[player].gameData.powerupData.isBigHand = true;
}

/// Allow more than one PowerupBigHand to be in effect at the same time.
/**
 * This function assumes this powerup has already been removed from the powerup 
 * effect queue. If another PowerupBigHand is in the queue then allow that one 
 * to stop the effect.
 */
void PowerupBigHand::StopEffect(u8 player)
{
  for (int i = 0; i < MAX_POWERUP_EFFECTS; ++i)
  {
    Powerup *powerup = g_players[player].gameData.powerupEffects[i];
    if (powerup && powerup->GetPowerupId() == powerupId)
    {
      return;
    }
  }
  
  g_players[player].gameData.powerupData.isBigHand = false;
}