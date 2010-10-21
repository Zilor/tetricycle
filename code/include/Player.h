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

/** @file Player.h
 * @brief Defines the Player class.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <cstring> // for memset

#include "TetrisPiece.h"
#include "PowerupUtils.h"
#include "Color.h"
#include "Powerup.h"
#include "Profile.h"
#include "defines.h"
#include "defines_Player.h"

extern ColorGradient g_cubeGradients[COLOR_ID_MAX];

enum
{
  ROTATE_NORMAL,
  ROTATE_REVERSE,
  ROTATE_PIECE,
  ROTATE_SIZE
};

enum
{
  GUIDE_OFF,
  GUIDE_SHADOW,
  GUIDE_LINE,
  GUIDE_SIZE
};

/// This class represents a player.
class Player
{
  /// Player data used when a powerup is in effect.
  /** If the data is non-zero then it overrides the normal data. */
  struct PlayerPowerupData
  {
    PlayerPowerupData() : playfieldScale(0),
                          isBigHand(false),
                          isReverse(false),
                          mirrorCtr(0) { }

    float playfieldScale;
    bool isBigHand;
    bool isReverse;
    u8 mirrorCtr;
  };

  /// Player data that needs to be reset every game should go here.
  struct PlayerGameData
  {
    PlayerGameData() : grabbedPowerup(NULL),
                       score(0),
                       lines(0),
                       pieces(-1),
                       frame(0),
                       level(0),
                       speed(START_SPEED),
                       cycleIdx(0),
                       leftRightCtr(0),
                       downCtr(0),
                       isDead(false),
                       isGrabHeld(false),
                       isLeftRightHeld(false),
                       isDownHeld(false)
    {
      memset(powerupQueue, 0, sizeof(powerupQueue));
      memset(powerupEffects, 0, sizeof(powerupEffects));
    }

    TetrisPieceBlock playfield[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
    Powerup *powerupQueue[MAX_ACQUIRED_POWERUPS]; ///< powerups gained by this player
    Powerup *powerupEffects[MAX_POWERUP_EFFECTS]; ///< powerups used on this player
    PlayerPowerupData powerupData;
    Powerup *grabbedPowerup;
    u16 score;
    u16 lines;
    u16 pieces;
    u8 frame;
    u8 level;
    u8 speed;
    u8 cycleIdx;
    u8 leftRightCtr;
    u8 downCtr;
    bool isDead;
    bool isGrabHeld;
    bool isLeftRightHeld;
    bool isDownHeld;
  };
  
public:
  Player() : powerups(g_totalPowerups),
             cubeAngle(DEFAULT_CUBE_ANGLE), 
             playfieldDX(DEFAULT_PLAYFIELD_DX),
             playfieldDY(DEFAULT_PLAYFIELD_DY),
             powerupsSize(g_totalPowerups),
             playfieldWidth(DEFAULT_PLAYFIELD_WIDTH), 
             playfieldHeight(DEFAULT_PLAYFIELD_HEIGHT), 
             playfieldScale(DEFAULT_PLAYFIELD_SCALE),
             rotation(ROTATE_NORMAL),
             guide(GUIDE_SHADOW),
             isShakeEnabled(false),
             isPreviewEnabled(true), 
             isHandicapEnabled(false)
  {
    for (int i = 0; i < g_totalPowerups; ++i)
      powerups[i] = (PowerupId)i;
  }

  PlayerGameData gameData;    ///< the player game data
  Profile profile;            ///< the player profile
  vector<PowerupId> powerups; ///< powerups that are enabled for this player
  TetrisPiece currPiece;      ///< the currently falling tetris piece
  TetrisPiece nextPiece;      ///< the next tetris piece
  float cubeAngle;
  s16 playfieldDX;
  s16 playfieldDY;
  u8 powerupsSize; ///< the actual size of the powerups array
  u8 playfieldWidth;
  u8 playfieldHeight;
  u8 playfieldScale;
  u8 id;
  u8 rotation;
  u8 guide;
  bool isShakeEnabled;
  bool isPreviewEnabled;
  bool isHandicapEnabled;

  /// Rotate the TetriCycle base to the right.
  void IncrementCycle() 
  { 
    gameData.cycleIdx = ++gameData.cycleIdx % playfieldWidth;
  }

  /// Rotate the TetriCycle base to the left.
  void DecrementCycle() 
  { 
    gameData.cycleIdx = (gameData.cycleIdx > 0) ? gameData.cycleIdx - 1 : playfieldWidth - 1;
  }

  /// Reset all state associated with this player.
  void Reset();

  /// Get an open slot for storing an acquired powerup.
  /** @return The index of the first open slot, if one exists; else, -1. */
  int GetPowerupQueueSlot()
  {
    for (int i = 0; i < MAX_ACQUIRED_POWERUPS; ++i)
    {
      if (!gameData.powerupQueue[i])
      {
        return i;
      }
    }

    return -1;
  }

  /// Adds the acquired powerup to the powerup queue.
  void QueuePowerup(Powerup *powerup, int slot)
  {
    gameData.powerupQueue[slot] = powerup;
  }

  /// Removes an acquired powerup from the powerup queue.
  void RemovePowerup(int slot)
  {
    gameData.powerupQueue[slot] = NULL;
  }

  /// Get an open slot for storing a powerup effect.
  /** @return The index of the first open slot, if one exists; else, -1. */
  int GetEffectQueueSlot()
  {
    for (int i = 0; i < MAX_POWERUP_EFFECTS; ++i)
    {
      if (!gameData.powerupEffects[i])
      {
        return i;
      }
    }

    return -1;
  }

  /// Adds a powerup to the powerup effect queue.
  /** @return True if the powerup was successfully used on this player; else, false. */
  bool QueueEffect(Powerup *powerup)
  {
    for (int i = 0; i < MAX_POWERUP_EFFECTS; ++i)
    {
      if (!gameData.powerupEffects[i])
      {
        gameData.powerupEffects[i] = powerup;
        return true;
      }
    }

    return false;
  }

  /// Queue a powerup effect at the specified slot.
  void QueueEffect(Powerup *powerup, int slot)
  {
    gameData.powerupEffects[slot] = powerup;
  }

  /// Removes a powerup from the powerup effect queue.
  /** @return True if the powerup was successfully removed; else, false. */
  bool RemoveEffect(Powerup *powerup)
  {
    for (int i = 0; i < MAX_POWERUP_EFFECTS; ++i)
    {
      if (gameData.powerupEffects[i] == powerup)
      {
        gameData.powerupEffects[i] = NULL;
        return true;
      }
    }

    return false;
  }

  /// Updates the player state.
  void Update()
  {
    Powerup *powerup;
    for (int i = 0; i < MAX_POWERUP_EFFECTS; ++i)
    {
      powerup = gameData.powerupEffects[i];
      if (powerup)
        powerup->Update();
    }
  }

  /// Draw the playfield (all the static tetris pieces).
  void DrawPlayfield();

  /// Draws a tetris piece.
  void DrawPiece(TetrisPiece* cp = NULL, u8 alpha = 255);

  /// Draw where the current piece will end up if dropped.
  void DrawPieceShadow();

  /// Draw the next piece.
  void DrawNextPiece()
  {
    if (isPreviewEnabled)
      DrawPiece(&nextPiece, 145);
  }

  /// Draw the base of the TetriCycle.
  void DrawBase();

  /// Rotate the current piece.
  void RotateCurrentPiece(int rot)
  {
    int oldRot = currPiece.GetRotation();
    currPiece.SetRotation(rot);

    // If the new rotation can't be placed, revert to the old one.
    if (!_CanPlacePiece())
      currPiece.SetRotation(oldRot);
  }

  /// Move the playfield.
  bool MovePlayfield(int type);

  /// Moves the tetris piece in the given direction.
  bool MovePiece(int type, TetrisPiece *cp = NULL);

  /// Move the current piece down automatically.
  void DoMovement();

  /// Draw each tetris piece block as a cube.
  void DrawBlockAsCube(float x, float y, ColorId colorIdx, u8 alpha = 255, GuiImageData *imgData = NULL, bool isGuideDot = false);

private:
  float _GetScale() { return !gameData.powerupData.playfieldScale ? (float)playfieldScale : gameData.powerupData.playfieldScale; } ///< Get the scale factor used for drawing this player's TetriCycle.
  void _SetBaseColor(GXColor &c) { g_cubeGradients[COLOR_ID_BASE].SetColor(c); }
  void _GetWinner();
  void _DisplayWinner(int winner);
  void _IncreaseLevelAllBut(int plyrIdx);

  /// Increases the player's level.
  /** This increases the speed of the falling piece. */
  void _IncreaseLevel()
  {
    if (gameData.level < MAX_LEVEL)
    {
      gameData.level++;
      gameData.speed = START_SPEED - 3 * gameData.level;
    }
  }

  /// Assigns the next piece to the current piece.
  void _SpawnNextPiece()
  {
    currPiece.InitPiece(nextPiece.GetPieceId(), playfieldWidth);
    currPiece.SetPowerupId(nextPiece.GetPowerupId());

    nextPiece.InitPiece(TetrisPiece::GetNextId(), playfieldWidth);
    nextPiece.SetPowerupId(PowerupUtils::GetNextId(*this));
  }

  /// Returns true if the piece can be placed on the playfield.
  bool _CanPlacePiece(TetrisPiece *cp = NULL);

  /// Removes all the completed lines.
  bool _RemoveLines();

  /// Removes the line at the given row.
  void _RemoveLine(int line);
};

#endif // __PLAYER_H__