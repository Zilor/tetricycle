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

/** @file Color.h
 * @brief Defines ColorId and ColorGradient.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __TCYC_COLOR_H__
#define __TCYC_COLOR_H__

#include <ogc/gx.h> // for GXColor

/// Enumerates all the color types.
enum ColorId
{
  COLOR_ID_YELLOW,
  COLOR_ID_CYAN,
  COLOR_ID_GREEN,
  COLOR_ID_RED,
  COLOR_ID_ORANGE,
  COLOR_ID_BLUE,
  COLOR_ID_MAGENTA,
  COLOR_ID_DEAD,
  COLOR_ID_BASE,
  COLOR_ID_MAX
};

/// A color gradient for coloring the face of a tetris piece block.
struct ColorGradient
{
  GXColor light;
  GXColor medium;
  GXColor dark;

  void SetColor(GXColor &c) { light = medium = dark = c; }
};

#endif // __TCYC_COLOR_H__