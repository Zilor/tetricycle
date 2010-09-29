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

/** @file Profile.h
 * @brief Defines the Profile struct.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <cstring> // for memset
#include <vector>  // for vector

#include "defines_Powerup.h" // for PowerupId
#include "defines_Player.h"  // for DEFAULT_MAX_PLAYLINES

using std::vector;

/// A player profile.
struct Profile
{
  Profile() : isPowerupEnabled(g_totalPowerups, true),
              maxLines(DEFAULT_MAX_PLAYLINES),
              attackRate(DEFAULT_ATTACK_RATE),
              powerupRate(DEFAULT_POWERUP_RATE)
  {
    memset(powerupStartQueue, POWERUP_ID_NONE, sizeof(powerupStartQueue));
  }

  Profile(const Profile &profile) : 
    isPowerupEnabled(profile.isPowerupEnabled),
    maxLines(profile.maxLines),
    attackRate(profile.attackRate),
    powerupRate(profile.powerupRate)
  {
    memcpy(powerupStartQueue, profile.powerupStartQueue, sizeof(powerupStartQueue));
  }

  vector<bool> isPowerupEnabled; ///< whether powerups are enabled/disabled for this player
  PowerupId powerupStartQueue[MAX_ACQUIRED_POWERUPS]; ///< powerups this player starts with
  u16 maxLines;   ///< number of lines needed to win the game
  u16 attackRate; ///< attack opponents every time score is a multiple of attackRate
  u8 powerupRate; ///< every 'powerupRate' pieces will be a powerup piece
};

#endif // __PROFILE_H__