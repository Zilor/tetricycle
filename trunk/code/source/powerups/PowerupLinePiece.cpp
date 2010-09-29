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

/** @file PowerupLinePiece.cpp
 * @author Cale Scholl / calvinss4
 */

#include "PowerupLinePiece.h"

#include "Player.h"        // for Player
#include "libwiigui/gui.h" // for GuiImageData

extern Player *g_players; ///< the player instances

PowerupId PowerupLinePiece::powerupId;
Powerup *PowerupLinePiece::instance = new PowerupLinePiece();

GuiImageData *PowerupLinePiece::imageData = 
  new GuiImageData(powerup_linepiece_png);

string PowerupLinePiece::helpText[2] = 
  {"Line Piece", "The target player's next piece will be a line piece."};

void PowerupLinePiece::StartEffect(u8 plyrIdx)
{
  g_players[plyrIdx].nextPiece.InitPiece(TETRISPIECE_ID_I);
}

void PowerupLinePiece::StopEffect(u8 plyrIdx)
{
  
}