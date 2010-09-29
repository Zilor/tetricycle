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

/** @file defines_Powerup.h
 * @brief Defines for the Powerup class.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __DEFINES_POWERUP_H__
#define __DEFINES_POWERUP_H__

#include <gctypes.h> // for u8

#define DEFAULT_POWERUP_DURATION 10000 // milliseconds
#define BIG_HAND_SCALE 2.5

// for drawing powerup textures
#define POWERUP_Y_OFFSET 25
#define POWERUP_X_OFFSET 5
#define POWERUP_WIDTH 32

/// For code inside a function that should only be executed once.
#define STATIC(code) \
  static bool firstTime = true; \
  if (firstTime) \
  { \
    firstTime = false; \
    code \
  }

/// Every powerup class is represented by a unique PowerupId.
typedef u8 PowerupId; // u8 allows 255 powerups

/// A PowerupId that represents a null powerup.
#define POWERUP_ID_NONE 255 // max value of u8

/// The total number of powerup classes (determined at runtime).
extern int g_totalPowerups;

/// Determines what players are the target of this powerup.
enum PowerupTarget
{
  POWERUP_TARGET_ONE,        ///< Affects the target player.
  POWERUP_TARGET_ALL,        ///< Affects all players.
  POWERUP_TARGET_ALL_BUT_ONE ///< Affects all but the target player.
};

#endif // __DEFINES_POWERUP_H__