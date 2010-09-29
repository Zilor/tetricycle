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

/** @file mt.h
 * @brief Randomization functions used for tetris.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __MT_H__
#define __MT_H__

// This is for calling C code from C++.
#ifdef __cplusplus
extern "C" {
#endif

void sgenrand(unsigned long seed);
double genrand();

#ifdef __cplusplus
}
#endif

#endif // __MT_H__