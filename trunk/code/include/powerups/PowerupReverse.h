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

/** @file PowerupReverse.h
 * @author Cale Scholl / calvinss4
 *
 * @page poweruplistpage Powerup List
 *
 * @section PowerupReverse
 * Reverses the direction in which the target player's tetris cylinder rotates.
 */

#pragma once
#ifndef __POWERUPREVERSE_H__
#define __POWERUPREVERSE_H__

#include "Powerup.h"

/// Reverses the direction in which the target player's tetris cylinder rotates.
class PowerupReverse : public Powerup
{
public:
  virtual PowerupId GetPowerupId() { return powerupId; }
  virtual GuiImageData* GetImageData() { return imageData; }
  virtual std::string* GetHelpText() { return helpText; }

protected:
  PowerupReverse()
  {
    STATIC
    (
      vector<Powerup *> &powerupVector = Powerup::GetVector();
      powerupId = powerupVector.size();
      powerupVector.push_back(this);
    )
  }
  virtual ~PowerupReverse() { }

  /// Returns a new powerup instance.
  virtual Powerup* GetInstance() { return new PowerupReverse(); }

  virtual void StartEffect(u8 player);
  virtual void StopEffect(u8 player);

private:
  static PowerupId powerupId;
  static Powerup *instance;
  static GuiImageData *imageData;
  static std::string helpText[2];
};

#endif // __POWERUPREVERSE_H__