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

/** @file tcyc_input.h
 * @brief Magical macros for handling user input.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __TCYC_INPUT_H__
#define __TCYC_INPUT_H__

#include <wiiuse/wpad.h>
#include "defines.h" // for MAX_PLAYERS

extern vec3w_t g_wiiacc[MAX_PLAYERS];     ///< wiimote acceleration data
extern expansion_t g_wiiexp[MAX_PLAYERS]; ///< wiimote expansion-controller data

// function prototypes
void TCYC_ProcessInput();         ///< Process player input during game mode.
void TCYC_ProcessEditModeInput(); ///< Process player input during edit mode.
bool PausePressedAnyPlayer();     ///< Returns true if any player pressed pause.
 
/// Scans gamecube and wiimote, then updates ACCEL and EXP data.
#define ALL_ScanPads() \
{ \
  PAD_ScanPads(); \
  WPAD_ScanPads(); \
  for (int i = 0; i < MAX_PLAYERS; ++i) \
  { \
    WPAD_Accel(i, g_wiiacc + i); \
    WPAD_Expansion(i, g_wiiexp + i); \
  } \
  for (int i = 3; i >= 0; --i) \
  { \
	  userInput[i].pad.btns_d = PAD_ButtonsDown(i); \
	  userInput[i].pad.btns_u = PAD_ButtonsUp(i); \
	  userInput[i].pad.btns_h = PAD_ButtonsHeld(i); \
	  userInput[i].pad.stickX = PAD_StickX(i); \
	  userInput[i].pad.stickY = PAD_StickY(i); \
	  userInput[i].pad.substickX = PAD_SubStickX(i); \
	  userInput[i].pad.substickY = PAD_SubStickY(i); \
	  userInput[i].pad.triggerL = PAD_TriggerL(i); \
	  userInput[i].pad.triggerR = PAD_TriggerR(i); \
  } \
}

// detect nunchuk:
// exp.type == WPAD_EXP_NUNCHUK
#define NUNCHUK_CONNECTED(i) (g_wiiexp[i].type == WPAD_EXP_NUNCHUK)

#define NUNCHUK_StickPosX(i) g_wiiexp[i].nunchuk.js.pos.x
#define NUNCHUK_StickPosY(i) g_wiiexp[i].nunchuk.js.pos.y

#define NUNCHUK_StickCenterX(i) g_wiiexp[i].nunchuk.js.center.x
#define NUNCHUK_StickCenterY(i) g_wiiexp[i].nunchuk.js.center.y

#define NUNCHUK_StickDX(i) (NUNCHUK_StickPosX(i) - NUNCHUK_StickCenterX(i))
#define NUNCHUK_StickDY(i) (NUNCHUK_StickPosY(i) - NUNCHUK_StickCenterY(i))

#define NUNCHUK_THRESHOLD 65

#define NUNCHUK_LEFT_PRESSED(i)  (NUNCHUK_StickDX(i) < -NUNCHUK_THRESHOLD)
#define NUNCHUK_RIGHT_PRESSED(i) (NUNCHUK_StickDX(i) > NUNCHUK_THRESHOLD)
#define NUNCHUK_UP_PRESSED(i)    (NUNCHUK_StickDY(i) > NUNCHUK_THRESHOLD)
#define NUNCHUK_DOWN_PRESSED(i)  (NUNCHUK_StickDY(i) < -NUNCHUK_THRESHOLD)

#define NUNCHUK_ButtonsDown(i) g_wiiexp[i].nunchuk.btns
#define NUNCHUK_ButtonsReleased(i) g_wiiexp[i].nunchuk.btns_released

#define NUNCHUK_C_PRESSED(i)  (NUNCHUK_ButtonsDown(i) & NUNCHUK_BUTTON_C)
#define NUNCHUK_Z_PRESSED(i)  (NUNCHUK_ButtonsDown(i) & NUNCHUK_BUTTON_Z)
#define NUNCHUK_Z_RELEASED(i) (NUNCHUK_ButtonsReleased(i) & NUNCHUK_BUTTON_Z)

// ACTION_PRESSED - 
// true if corresponding action button was pressed this frame.

#define MENU_LEFT_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_LEFT \
  || PAD_StickX(i) < -65 \
  || (NUNCHUK_CONNECTED(i) ? \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_LEFT || NUNCHUK_LEFT_PRESSED(i) \
         : \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_UP) \
)

#define MENU_RIGHT_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_RIGHT \
  || PAD_StickX(i) > 65 \
  || (NUNCHUK_CONNECTED(i) ? \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_RIGHT || NUNCHUK_RIGHT_PRESSED(i) \
         : \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_DOWN) \
)

#define MENU_UP_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_UP \
  || PAD_StickY(i) > 65 \
  || (NUNCHUK_CONNECTED(i) ? \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_UP || NUNCHUK_UP_PRESSED(i) \
         : \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_RIGHT) \
)

#define MENU_DOWN_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_DOWN \
  || PAD_StickY(i) < -65 \
  || (NUNCHUK_CONNECTED(i) ? \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_DOWN || NUNCHUK_DOWN_PRESSED(i) \
         : \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_LEFT) \
)

#define MENU_SELECT_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_A \
  || WPAD_ButtonsDown(i) & (NUNCHUK_CONNECTED(i) ? WPAD_BUTTON_A : WPAD_BUTTON_2) \
)

#define MENU_EXIT_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_B \
  || WPAD_ButtonsDown(i) & (NUNCHUK_CONNECTED(i) ? WPAD_BUTTON_B : WPAD_BUTTON_1) \
)

#define A_PRESSED(i) \
( \
  WPAD_ButtonsDown(i) & WPAD_BUTTON_A \
  || PAD_ButtonsDown(i) & PAD_BUTTON_A \
)

#define B_PRESSED(i) \
( \
  WPAD_ButtonsDown(i) & WPAD_BUTTON_B \
  || PAD_ButtonsDown(i) & PAD_BUTTON_B \
)

#define LEFT_PRESSED(i) \
( \
  (g_players[i].rotation != ROTATE_REVERSE ? \
    _LEFT_PRESSED(i) \
      : \
    _RIGHT_PRESSED(i)) \
)

#define RIGHT_PRESSED(i) \
( \
  (g_players[i].rotation != ROTATE_REVERSE ? \
    _RIGHT_PRESSED(i) \
      : \
    _LEFT_PRESSED(i)) \
)

#define _LEFT_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_LEFT \
  || PAD_StickX(i) < -65 \
  || (NUNCHUK_CONNECTED(i) ? \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_LEFT || NUNCHUK_LEFT_PRESSED(i) \
         : \
       WPAD_ButtonsHeld(i) & WPAD_BUTTON_UP) \
)

#define _RIGHT_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_RIGHT \
  || PAD_StickX(i) > 65 \
  || (NUNCHUK_CONNECTED(i) ? \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_RIGHT || NUNCHUK_RIGHT_PRESSED(i) \
         : \
       WPAD_ButtonsHeld(i) & WPAD_BUTTON_DOWN) \
)

#define UP_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_UP \
  || PAD_StickY(i) > 65 \
  || (NUNCHUK_CONNECTED(i) ? \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_UP || NUNCHUK_UP_PRESSED(i) \
         : \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_RIGHT) \
)

#define DOWN_PRESSED(i) \
( \
  PAD_ButtonsHeld(i) & PAD_BUTTON_X \
  || PAD_ButtonsHeld(i) & PAD_BUTTON_Y \
  || PAD_StickY(i) < -65 \
  || (NUNCHUK_CONNECTED(i) ? \
       WPAD_ButtonsDown(i) & WPAD_BUTTON_DOWN || NUNCHUK_DOWN_PRESSED(i) \
         : \
       WPAD_ButtonsHeld(i) & WPAD_BUTTON_LEFT) \
)

// rotate tetris piece
#define ROTATE_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_A \
  || WPAD_ButtonsDown(i) & (NUNCHUK_CONNECTED(i) ? WPAD_BUTTON_A : WPAD_BUTTON_2) \
)

// rotate tetris piece in opposite direction
#define ROTATE2_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_B \
  || WPAD_ButtonsDown(i) & (NUNCHUK_CONNECTED(i) ? WPAD_BUTTON_B : WPAD_BUTTON_1) \
)

// grab a powerup from the queue
#define GRAB_HELD(i) \
( \
  WPAD_ButtonsHeld(i) & WPAD_BUTTON_A \
)

#define PAUSE_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_START \
  || WPAD_ButtonsDown(i) & WPAD_BUTTON_PLUS \
)

#define MENU_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_TRIGGER_Z \
  || WPAD_ButtonsDown(i) & WPAD_BUTTON_MINUS \
)

#define LOADER_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_TRIGGER_R \
  || WPAD_ButtonsDown(i) & WPAD_BUTTON_HOME \
)

// When holding wiimote sideways, XYZ accel values at rest are: 500, 500, 600
#define ACCEL_Z_THRESHOLD 750
#define ACCEL_Z_PRESSED(i) (g_wiiacc[i].z > ACCEL_Z_THRESHOLD)

#define DROP_PRESSED(i) \
( \
  PAD_ButtonsDown(i) & PAD_BUTTON_DOWN \
  || WPAD_ButtonsDown(i) & (NUNCHUK_CONNECTED(i) ? WPAD_BUTTON_DOWN : WPAD_BUTTON_A) \
)

// ACTION_HELD - 
// true if corresponding action button was held this frame.

#define PAUSE_HELD(i) \
( \
  PAD_ButtonsHeld(i) & PAD_BUTTON_START \
  || WPAD_ButtonsHeld(i) & WPAD_BUTTON_PLUS \
)

#endif // __TCYC_INPUT_H__