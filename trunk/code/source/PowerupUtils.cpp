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

/** @file PowerupUtils.cpp
 * @author Cale Scholl / calvinss4
 */

#include "PowerupUtils.h"

#include "Powerup.h"       // for Powerup
#include "libwiigui/gui.h" // for GuiSound
#include "Options.h"       // for Options
#include "Player.h"        // for Player

extern Options *g_options;
extern Player *g_players;

GuiSound *PowerupUtils::invalidTargetSound = 
  new GuiSound(powerup_invalid_target_pcm, powerup_invalid_target_pcm_size, SOUND_PCM);

Powerup* PowerupUtils::GetStaticInstance(PowerupId pid)
{
  return Powerup::GetVector()[pid];
}

GuiImageData* PowerupUtils::GetImageData(PowerupId pid)
{
  return GetStaticInstance(pid)->GetImageData();
}

GuiSound* PowerupUtils::GetSound(PowerupId pid)
{
  return GetStaticInstance(pid)->GetSound();
}

std::string* PowerupUtils::GetHelpText(PowerupId pid)
{
  return GetStaticInstance(pid)->GetHelpText();
}

PowerupId PowerupUtils::GetNextId(Player &player)
{
  if (g_options->players == 1)
    return POWERUP_ID_NONE;

  int powerupRate   = g_options->profile.powerupRate;
  Player *playerPtr = NULL;

  if (player.isHandicapEnabled)
  {
    powerupRate = player.profile.powerupRate;
    playerPtr   = &player;
  }

  if (!powerupRate)
    return POWERUP_ID_NONE;

  return ((++player.gameData.pieces + 1) % powerupRate) ? 
    POWERUP_ID_NONE : PowerupUtils::GetRandomId(playerPtr);
}

PowerupId PowerupUtils::GetRandomId(Player *playerPtr)
{
  // Use the global options settings.
  int powerupsSize    = g_options->powerupsSize;
  PowerupId *powerups = &g_options->powerups[0];

  if (playerPtr)
  {
    // Use the handicap options settings.
    powerupsSize = playerPtr->powerupsSize;
    powerups     = &playerPtr->powerups[0];
  }
  
  return powerupsSize ? 
    powerups[(int)(genrand() * powerupsSize)] : POWERUP_ID_NONE;
}

int PowerupUtils::GetTotalPowerups()
{
  return Powerup::GetVector().size();
}

void PowerupUtils::ResetPowerupStartTimes()
{
  Powerup *powerup;
  for (int i = 0; i < g_options->players; ++i)
  {
    for (int j = 0; j < MAX_POWERUP_EFFECTS; ++j)
    {
      powerup = g_players[i].gameData.powerupEffects[j];
      if (powerup)
        powerup->ResetStartTime();
    }
  }
}

void PowerupUtils::DeleteAllPowerups()
{
  Powerup *powerup;
  Player *player = g_players;

  for (int i = 0; i < g_options->players; ++i, ++player)
  {
    for (int j = 0; j < MAX_POWERUP_EFFECTS; ++j)
    {
      powerup = player->gameData.powerupEffects[j];
      if (powerup)
        powerup->Terminate();
    }
  }
}