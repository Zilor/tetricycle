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

/** @file Options.h
 * @brief Defines the Options class.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include "Profile.h" // for Profile

/// The game options.
class Options
{
public:
  Profile profile;            ///< the global player profile
  vector<PowerupId> powerups; ///< powerups that are enabled
  u8 powerupsSize;            ///< actual size of the powerups array
  u8 players;                 ///< the number of players
  bool isPaused;              ///< whether the game is paused
  bool isNetplay;

  static Options& GetInstance() 
  {
    static Options options;
    return options;
  }

private:
  Options() : powerups(g_totalPowerups),
              powerupsSize(g_totalPowerups),
              players(1),
              isPaused(false),
              isNetplay(false)
  {
    for (int i = 0; i < g_totalPowerups; ++i)
      powerups[i] = (PowerupId)i;
  }
};

#endif // __OPTIONS_H__