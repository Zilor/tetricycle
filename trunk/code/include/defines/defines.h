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

/** @file defines.h
 * @brief A file for global scope generic defines.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __DEFINES_H__
#define __DEFINES_H__

#define DEBUG 0

#define DOWN 0
#define UP 1
#define LEFT 2
#define RIGHT 3

#define MAX_PLAYERS 4
#define MAX_LEVEL 9
#define START_SPEED 30

#define P2_X_BORDER 320
#define P3_X_BORDER_1 213.3
#define P3_X_BORDER_2 426.6
#define P4_X_BORDER_1 160
#define P4_X_BORDER_2 320
#define P4_X_BORDER_3 480

#define SWAP(a, b) do { a ^= b, b ^= a, a ^= b; } while(0)

#endif // __DEFINES_H__
