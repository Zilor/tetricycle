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

/** @file defines_Player.h
 * @brief Defines for the Player class.
 * @author Cale Scholl / calvinss4
 */

#pragma once
#ifndef __DEFINES_PLAYER_H__
#define __DEFINES_PLAYER_H__

#define DEFAULT_PLAYFIELD_WIDTH 10
#define DEFAULT_PLAYFIELD_HEIGHT 20
#define DEFAULT_PLAYFIELD_SCALE 10
#define DEFAULT_CUBE_ANGLE 10
#define DEFAULT_PLAYFIELD_DX 0
#define DEFAULT_PLAYFIELD_DY 0
#define DEFAULT_MAX_PLAYLINES 30
#define DEFAULT_ATTACK_RATE 3
#define DEFAULT_POWERUP_RATE 5

#define MIN_PLAYFIELD_WIDTH 10
#define MIN_PLAYFIELD_HEIGHT 20
#define MIN_PLAYFIELD_SCALE 1
#define MIN_CUBE_ANGLE 0

#define MAX_PLAYFIELD_WIDTH 30
#define MAX_PLAYFIELD_HEIGHT 20
#define MAX_PLAYFIELD_SCALE 15
#define MAX_PLAYFIELD_DELTA 995

#define MAX_ACQUIRED_POWERUPS 3
#define MAX_POWERUP_EFFECTS 2

#endif // __DEFINES_PLAYER_H__