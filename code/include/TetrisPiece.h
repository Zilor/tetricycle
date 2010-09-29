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

/** @file TetrisPiece.h
 * @brief Defines the TetrisPiece classes.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __TETRISPIECE_H__
#define __TETRISPIECE_H__

#include <gctypes.h> // for u8

#include "mt.h"               // for genrand()
#include "defines_Powerup.h"  // for PowerupId

#define DEFAULT_BLOCKS_PER_PIECE 4

/// Enumerates all tetris piece types.
enum TetrisPieceId
{
  TETRISPIECE_ID_NONE = -1,
  TETRISPIECE_ID_O, ///< square piece
  TETRISPIECE_ID_I, ///< line piece
  TETRISPIECE_ID_S, ///< reverse-squiggly piece
  TETRISPIECE_ID_Z, ///< squiggly piece
  TETRISPIECE_ID_L, ///< L piece
  TETRISPIECE_ID_J, ///< reverse-L piece
  TETRISPIECE_ID_T, ///< T piece
  TETRISPIECE_ID_MAX = 7
};

/// Every piece fits into a 4x4 grid.
struct TetrisPieceDesc
{
  u8 map[4][4];
};

/// Pieces have up to 4 possible rotations.
extern TetrisPieceDesc g_pieceDesc[TETRISPIECE_ID_MAX][4];

/// A tetris piece.
class TetrisPiece
{
public:
  const TetrisPieceDesc& GetPieceDescription() { return *desc; }
  TetrisPieceId GetPieceId() { return pieceId; }
  PowerupId GetPowerupId() { return powerupId; }
  int GetRotation() { return rotation; }
  int GetX() { return x; }
  int GetY() { return y; }

  void InitPiece(TetrisPieceId id, int width = 0)
  {
    pieceId = id;
    rotation = 0;
    _ResetDesc();
    downctr = 0;
    if (width)
    {
      x = (width >> 1) - 2; // (playfield_width / 2) - 2 
      y = -1;
    }
  }
  void SetPowerupId(PowerupId id) { powerupId = id; }
  void SetRotation(int rot) { rotation = rot, _ResetDesc(); }
  void SetX(int xval) { x = xval; }
  void SetY(int yval) { y = yval; }
  void IncrementDownCounter() { ++downctr; }
  bool IsAccelEnabled() { return downctr > 0; }

  static TetrisPieceId GetNextId()
  {
    return (TetrisPieceId)(int)(genrand() * TETRISPIECE_ID_MAX);
  }

private:
  const TetrisPieceDesc *desc;
  PowerupId powerupId;   ///< associated powerup
  TetrisPieceId pieceId; ///< tetris piece type
  u8 rotation;           ///< current rotation, in range [0-3]
  u8 downctr;            ///< number of times piece has moved down

  // The playfield is a playfield_width x playfield_height grid 
  // (default: 10x20); the upper left corner corresponds to (0,0).
  // (piece.x, piece.y) is a playfield coordinate that corresponds to the 
  // upper left corner of the tetris piece's 4x4 grid.
  s8 x; ///< playfield x-coordinate
  s8 y; ///< playfield y-coordinate

  void _ResetDesc() { desc = &g_pieceDesc[pieceId][rotation]; }
};

/// Connectivity information used for powerup pieces.
/**
 * Describes how many individual blocks (0-4) of a tetris piece are still 
 * connected; once all the blocks have been cleared, the player gains the
 * associated powerup.
 */
struct TetrisPieceConnectivityInfo
{
  TetrisPieceConnectivityInfo(PowerupId pupId = POWERUP_ID_NONE,
                              u8 maxCount = DEFAULT_BLOCKS_PER_PIECE) : 
    powerupId(pupId),
    counter(maxCount) {}

  PowerupId powerupId;
  u8 counter;
};

/// An individual block that composes a tetris piece.
/** 
 * All blocks composing a tetris piece point to the same 
 * TetrisPieceConnectivityInfo.
 */
class TetrisPieceBlock
{
public:
  TetrisPieceBlock(TetrisPieceId id = TETRISPIECE_ID_NONE,
                   TetrisPieceConnectivityInfo *info = NULL) : 
    pieceId(id),
    connectivityInfo(info) {}

  TetrisPieceId pieceId;
  TetrisPieceConnectivityInfo *connectivityInfo;

  void SetConnectivityInfo(TetrisPieceConnectivityInfo *info)
  {
    connectivityInfo = info;
  }

  void Free()
  {
    delete connectivityInfo;
    connectivityInfo = NULL;
  }
};

#endif // __TETRISPIECE_H__