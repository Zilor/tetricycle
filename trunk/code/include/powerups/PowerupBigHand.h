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

/** @file PowerupBigHand.h
 * @author Cale Scholl / calvinss4
 *
 * @page poweruplistpage Powerup List
 *
 * @section PowerupBigHand
 * A powerup used to obscure the player's view.
 */

#pragma once
#ifndef __POWERUPBIGHAND_H__
#define __POWERUPBIGHAND_H__

#include "Powerup.h"

/// A powerup used to obscure the player's view.
class PowerupBigHand : public Powerup
{
public:
  virtual PowerupId GetPowerupId() { return powerupId; }
  virtual GuiImageData* GetImageData() { return imageData; }
  virtual string* GetHelpText() { return helpText; }

protected:
  PowerupBigHand()
  {
    STATIC
    (
      vector<Powerup *> &powerupVector = Powerup::GetVector();
      powerupId = powerupVector.size();
      powerupVector.push_back(this);
    )
  }
  virtual ~PowerupBigHand() { }

  /// Returns a new powerup instance.
  virtual Powerup* GetInstance() { return new PowerupBigHand(); }

  virtual void StartEffect(u8 player);
  virtual void StopEffect(u8 player);

private:
  static PowerupId powerupId;
  static Powerup *instance;
  static GuiImageData *imageData;
  static string helpText[2];
};

#endif // __POWERUPBIGHAND_H__