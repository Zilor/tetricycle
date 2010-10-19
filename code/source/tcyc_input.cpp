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

/** @file tcyc_input.cpp
 * @author Cale Scholl / calvinss4
 */

#include "tcyc_input.h"

#include "main.h"          // for TCYC_GetTargetPlayer, TCYC_GetTargetPowerupSlot
#include "tcyc_menu.h"     // for TCYC_MenuPause
#include "Player.h"        // for Player
#include "Options.h"       // for Options
#include "libwiigui/gui.h" // for GuiTrigger
#include "PowerupUtils.h"  // for PowerupUtils

extern Player *g_players;       ///< the player instances
extern int g_tcycMenu;          ///< the current menu state
extern Options *g_options;      ///< the global options
extern GuiTrigger userInput[4]; ///< user input
extern bool g_isClassicMode;    ///< classic mode

// helper routines
static void _HandlePowerups(int plyrIdx);
static void _HandleLeftRight(int plyrIdx, bool isEditMode = false);
static void _HandleDown(int plyrIdx);

void TCYC_ProcessInput()
{
  // Allow any player to pause the game.
  if (PausePressedAnyPlayer())
  {
    TCYC_MenuPause("Pause");
    if (g_tcycMenu != TCYC_MENU_NONE)
      return;

    // This is necessary so that powerups don't expire as soon as we unpause 
    // the game.
    PowerupUtils::ResetPowerupStartTimes();
  }

  for (int i = 0; i < g_options->players; ++i)
  {
    Player &player = g_players[i];
    TetrisPiece &piece = player.currPiece;

    if (player.gameData.isDead)
      continue;

    // POWERUPS:
    _HandlePowerups(i);

    // ROTATE PIECE:
    int rot = piece.GetRotation();

    if (ROTATE_PRESSED(i))
    {
      ++rot;
      if (rot > 3)
        rot = 0;
      player.RotateCurrentPiece(rot);
    }
    else if (ROTATE2_PRESSED(i))
    {
      --rot;
      if (rot < 0)
        rot = 3;
      player.RotateCurrentPiece(rot);
    }

    // LEFT, RIGHT:
    _HandleLeftRight(i);

    // DOWN:
    _HandleDown(i);

    // DROP:
    bool isAccelPressed = ACCEL_Z_PRESSED(i) && piece.IsAccelEnabled();

    if (DROP_PRESSED(i) || (player.isShakeEnabled && isAccelPressed))
    {
      while (player.MovePiece(DOWN));
      player.DoMovement();
    }
  }
}

void TCYC_ProcessEditModeInput()
{
  for (int i = 0; i < g_options->players; ++i)
  {
    _HandleLeftRight(i, true);
  }
}

bool PausePressedAnyPlayer()
{
  for (int i = 0; i < g_options->players; ++i)
  {
    if (PAUSE_PRESSED(i))
      return true;
  }

  return false;
}

void _HandlePowerups(int plyrIdx)
{
  Player &player = g_players[plyrIdx];

  // Get the target powerup slot.
  int targetSlot = -1;
  if (userInput[plyrIdx].wpad->ir.valid)
  {
    targetSlot = TCYC_GetTargetPowerupSlot(plyrIdx, 
                                           userInput[plyrIdx].wpad->ir.x, 
                                           userInput[plyrIdx].wpad->ir.y);
  }

  if (GRAB_HELD(plyrIdx))
  {
    if (!player.gameData.isGrabHeld)
    {
      player.gameData.isGrabHeld = true;
      if (targetSlot >= 0)
      {
        // The player grabbed a powerup this frame.
        player.gameData.grabbedPowerup = player.gameData.powerupQueue[targetSlot];
        player.gameData.powerupQueue[targetSlot] = NULL;

        // If we're in this player's powerup zone and grab was pressed this 
        // frame, then ignore all other button presses.
        WPAD_Data(plyrIdx)->btns_d = 0;
      }
    }
  }
  else
  {
    player.gameData.isGrabHeld = false;
    if (player.gameData.grabbedPowerup)
    {
      // The player dropped a powerup. If we're in this player's powerup 
      // zone then try to put the powerup back.
      if (targetSlot >= 0)
      {
        int emptySlot = player.GetPowerupQueueSlot();
        if (emptySlot >= 0)
        {
          player.QueuePowerup(player.gameData.grabbedPowerup, emptySlot);
        }
        else
        {
          // play a buzz sound for negative reinforcement
          PowerupUtils::GetInvalidTargetSound()->Play();
        }
      }
      // Otherwise try to use the powerup on the target player.
      else
      {
        int targetPlayer = -1;
        if (userInput[plyrIdx].wpad->ir.valid)
          targetPlayer = TCYC_GetTargetPlayer(userInput[plyrIdx].wpad->ir.x);

        bool success = false;
        if (targetPlayer >= 0)
          success = player.gameData.grabbedPowerup->Initiate(targetPlayer);

        if (!success)
        {
          // If we couldn't use the powerup then try to put it back.
          int emptySlot = player.GetPowerupQueueSlot();
          if (emptySlot >= 0)
            player.QueuePowerup(player.gameData.grabbedPowerup, emptySlot);

          // play a buzz sound for negative reinforcement
          PowerupUtils::GetInvalidTargetSound()->Play();
        }
      }

      player.gameData.grabbedPowerup = NULL;
    }
  }
}

void _HandleLeftRight(int plyrIdx, bool isEditMode)
{
  Player &player = g_players[plyrIdx];

  // When the analog stick is held in a given direction, only register a 
  // press for that direction every 'leftRightTimeout' frames.
  static const int leftRightTimeout = 5;

  bool isLeftPressed  = LEFT_PRESSED(plyrIdx);
  bool isRightPressed = RIGHT_PRESSED(plyrIdx);

  if (player.gameData.powerupData.isReverse)
    SWAP(isLeftPressed, isRightPressed);

  if (!isLeftPressed && !isRightPressed)
  {
    player.gameData.isLeftRightHeld = false;
    player.gameData.leftRightCtr = 0;
  }
  else if (!player.gameData.isLeftRightHeld 
           || player.gameData.leftRightCtr > leftRightTimeout)
  {
    player.gameData.isLeftRightHeld = true;

    if (player.gameData.leftRightCtr > leftRightTimeout)
      player.gameData.leftRightCtr = 0;

    if (isLeftPressed) 
    {
      if (!isEditMode)
      {
        if (player.rotation != ROTATE_PIECE && !g_isClassicMode)
        {
          if (player.MovePlayfield(LEFT))
            player.IncrementCycle();
        }
        else
        {
          player.MovePiece(LEFT);
        }
      }
      else
      {
        player.IncrementCycle();
      }
    }

    if (isRightPressed)
    {
      if (!isEditMode)
      {
        if (player.rotation != ROTATE_PIECE && !g_isClassicMode)
        {
          if (player.MovePlayfield(RIGHT))
            player.DecrementCycle();
        }
        else
        {
          player.MovePiece(RIGHT);
        }
      }
      else
      {
        player.DecrementCycle();
      }
    }
  }
  else
  {
    player.gameData.leftRightCtr++;
  }
}

void _HandleDown(int plyrIdx)
{
  Player &player = g_players[plyrIdx];

  // When the analog stick is held in the down direction, only register a 
  // press for that direction every 'downTimeout' frames.
  static const int downTimeout = 1; // ==> every other frame

  bool isDownPressed = DOWN_PRESSED(plyrIdx);

  if (!isDownPressed)
  {
    player.gameData.isDownHeld = false;
    player.gameData.downCtr = 0;
  }
  else if (!player.gameData.isDownHeld 
           || player.gameData.downCtr > downTimeout)
  {
    player.gameData.isDownHeld = true;

    if (player.gameData.downCtr > downTimeout)
      player.gameData.downCtr = 0;

    player.DoMovement();
  }
  else
  {
    player.gameData.downCtr++;
  }

  // Move the piece down every 'speed' frames.
  if (player.gameData.frame 
      && player.gameData.frame % player.gameData.speed == 0)
  {
    player.DoMovement();
  }
}