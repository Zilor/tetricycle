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

/** @file Powerup.cpp
 * @author Cale Scholl / calvinss4
 */

#include "Powerup.h"

#include "Options.h"       // for Options
#include "Player.h"        // for Player
#include "libwiigui/gui.h" // for GuiSound

extern Options *g_options; ///< the global options
extern Player *g_players;  ///< the player instances

GuiSound *Powerup::defaultSound = 
  new GuiSound(powerup_default_pcm, powerup_default_pcm_size, SOUND_PCM);

/**
 * Do not override this function. 
 * This function is called automatically whenever a powerup is dropped on a 
 * player.
 * @param targetPlayer The target player index.
 * @return True if the powerup was successfully used on a player.
 */
bool Powerup::Initiate(u8 targetPlayer)
{
  int slot;
  Powerup *powerup = NULL;

  switch (GetTargetType())
  {
    case POWERUP_TARGET_ONE:
    {
      slot = g_players[targetPlayer].GetEffectQueueSlot();
      if (slot >= 0)
      {
        if (!powerup)
          powerup = GetInstance();

        g_players[targetPlayer].QueueEffect(powerup, slot);
        StartEffect(targetPlayer);
      }
      break;
    }

    case POWERUP_TARGET_ALL:
    {
      for (int i = 0; i < g_options->players; ++i)
      {
        slot = g_players[i].GetEffectQueueSlot();
        if (slot >= 0)
        {
          if (!powerup)
            powerup = GetInstance();

          g_players[i].QueueEffect(powerup, slot);
          StartEffect(i);
        }
      }
      break;
    }

    case POWERUP_TARGET_ALL_BUT_ONE:
    {
      for (int i = 0; i < g_options->players; ++i)
      {
        if (i != targetPlayer)
        {
          slot = g_players[i].GetEffectQueueSlot();
          if (slot >= 0)
          {
            if (!powerup)
              powerup = GetInstance();
          
            g_players[i].QueueEffect(powerup, slot);
            StartEffect(i);
          }
        }
      }
      break;
    }
  }

  if (powerup)
  {
    GetSound()->Play();
    powerup->startTime = gettime();
  }
  
  return powerup != NULL;
}
  
void Powerup::Terminate()
{
  for (int i = 0; i < g_options->players; ++i)
  {
    if (g_players[i].RemoveEffect(this))
      StopEffect(i);
  }

  delete this;
}