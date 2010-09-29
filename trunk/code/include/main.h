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

/** @file main.h
 * @brief Prototypes for functions defined in main.cpp.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __MAIN_H__
#define __MAIN_H__

#include <gctypes.h> // for u8
class GuiImageData;

// used by tetris_menu.cpp
void TCYC_SetUp2D();
void TCYC_DrawText();
void TCYC_Game();
void TCYC_EditPlayfield();

// used by tetris_input.cpp
int TCYC_GetTargetPlayer(int x);
int TCYC_GetTargetPowerupSlot(int player, int x, int y);

// used by Player.cpp
void GX_Cube(int colorIdx, u8 alpha = 255, GuiImageData *imgData = NULL);

#endif // __MAIN_H__
