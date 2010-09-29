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

/** @file PowerupLinePiece.h
 * @author Cale Scholl / calvinss4
 *
 * @page poweruplistpage Powerup List
 *
 * @section PowerupLinePiece
 * The target player's next piece will be a line piece.
 */

#pragma once
#ifndef __POWERUPLINEPIECE_H__
#define __POWERUPLINEPIECE_H__

#include "Powerup.h"

/// The target player's next piece will be a line piece.
class PowerupLinePiece : public Powerup
{
public:
  virtual PowerupId GetPowerupId() { return powerupId; }
  virtual GuiImageData* GetImageData() { return imageData; }
  virtual string* GetHelpText() { return helpText; }

protected:
  PowerupLinePiece()
  {
    STATIC
    (
      vector<Powerup *> &powerupVector = Powerup::GetVector();
      powerupId = powerupVector.size();
      powerupVector.push_back(this);
    )
  }
  virtual ~PowerupLinePiece() { }

  /// Returns a new powerup instance.
  virtual Powerup* GetInstance() { return new PowerupLinePiece(); }

  virtual void StartEffect(u8 player);
  virtual void StopEffect(u8 player);
  virtual u32 GetDuration() { return 1000; }

private:
  static PowerupId powerupId;
  static Powerup *instance;
  static GuiImageData *imageData;
  static string helpText[2];
};

#endif // __POWERUPLINEPIECE_H__