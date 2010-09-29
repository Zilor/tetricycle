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

/** @file tcyc_menu.h
 * @brief A file for TetriCycle menu functions.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __TCYC_MENU_H__
#define __TCYC_MENU_H__

typedef struct _gx_color GXColor;

#ifndef NULL
#define NULL 0
#endif

enum
{
  TCYC_MENU_NONE,
  TCYC_MENU_EXIT,
  TCYC_MENU_MAIN,
  TCYC_MENU_GAME
};

void TCYC_MenuLoop();
void TCYC_MenuPause(const char *pauseTxt = NULL, GXColor *txtColor = NULL);
void TCYC_MenuError(const char *errorTxt);

#endif // __TCYC_MENU_H__