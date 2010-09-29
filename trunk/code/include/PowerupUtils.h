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

/** @file PowerupUtils.h
 * @brief Defines the PowerupUtils class.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __POWERUPUTILS_H__
#define __POWERUPUTILS_H__

#include <string>
#include "defines_Powerup.h"

class Powerup;
class Player;
class GuiImageData;
class GuiSound;

using std::string;

/// A utility class for the Powerup framework.
class PowerupUtils
{
public:
  static Powerup* GetStaticInstance(PowerupId pid);    ///< Returns a static Powerup instance.
  static GuiImageData* GetImageData(PowerupId pid);    ///< Returns the powerup image data.
  static GuiSound* GetSound(PowerupId pid);            ///< Returns the powerup sound.
  static string* GetHelpText(PowerupId pid);           ///< Returns the powerup help text.
  static PowerupId GetNextId(Player &player);          ///< Get the PowerupId for the next piece.
  static PowerupId GetRandomId(Player *player = NULL); ///< Generates a random PowerupId.
  static int GetTotalPowerups();                       ///< Returns the total number of unique powerups.
  static void ResetPowerupStartTimes();                ///< Called when the game is unpaused.
  static void DeleteAllPowerups();                     ///< Called when the game is reset/quit.

  /// Sound effect for invalid use of Powerup.
  static GuiSound* GetInvalidTargetSound() { return invalidTargetSound; }

private:
  PowerupUtils() { }

  static GuiSound *invalidTargetSound;
};

#endif // __POWERUPUTILS_H__