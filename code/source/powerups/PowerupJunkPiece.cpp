/*
 * TetriCycle
 * Copyright (C) 2010 Cale Scholl
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

/** @file PowerupJunkPiece.cpp
 * @author Cale Scholl
 */

#include "PowerupJunkPiece.h"

#include "Player.h"        // for Player
#include "libwiigui/gui.h" // for GuiImageData

extern Player *g_players; ///< the player instances

PowerupId PowerupJunkPiece::powerupId;
Powerup *PowerupJunkPiece::instance = new PowerupJunkPiece();

GuiImageData *PowerupJunkPiece::imageData = 
  new GuiImageData(powerup_junkpiece_png);

string PowerupJunkPiece::helpText[2] = 
  {"Junk Piece", "The target player's next piece will be a junk piece."};

// OPTIONAL:
//GuiSound *PowerupJunkPiece::sound = 
//  new GuiSound(powerup_junkpiece_pcm, powerup_junkpiece_pcm_size, SOUND_PCM);

void PowerupJunkPiece::StartEffect(u8 player)
{
  g_players[player].nextPiece.InitPiece(TETRISPIECE_ID_JUNK);
}

void PowerupJunkPiece::StopEffect(u8 player)
{
  
}