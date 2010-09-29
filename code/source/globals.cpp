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

/** @file globals.cpp
 * @author Cale Scholl / calvinss4
 */

#include <wiiuse/wpad.h> // for vec3w_t, expansion_t
#include <gcmodplay.h>   // for MODPlay
#include "Player.h"      // for Player
#include "Options.h"     // for Options
#include "Color.h"       // for ColorGradient

Options *g_options; // the global options
TetrisPieceDesc g_pieceDesc[TETRISPIECE_ID_MAX][4]; // static description of every tetris piece for all 4 rotations
Player *g_players; // the player instances
MODPlay g_modPlay; // used for playing the game music
bool g_isEditMode = false; // true when editing the playfield
int g_tcycMenu; // the current menu state
Mtx g_view; // the global view matrix
vec3w_t g_wiiacc[MAX_PLAYERS];     // wiimote acceleration data
expansion_t g_wiiexp[MAX_PLAYERS]; // wiimote expansion-controller data
int g_totalPowerups; // the total number of unique powerups

// gradients for coloring the face of a tetris piece block
ColorGradient g_cubeGradients[COLOR_ID_MAX] = 
{
  {{255, 255, 64, 255}, {128, 128, 0, 255}, {64, 64, 0, 255}},  // yellow
  {{64, 255, 255, 255}, {0, 128, 128, 255}, {0, 64, 64, 255}},  // cyan
  {{64, 255, 64, 255}, {0, 128, 0, 255}, {0, 64, 0, 255}},      // green
  {{255, 64, 64, 255}, {128, 0, 0, 255}, {64, 0, 0, 255}},      // red
  {{255, 128, 64, 255}, {128, 64, 0, 255}, {64, 32, 0, 255}},   // orange
  {{64, 64, 255, 255}, {0, 0, 128, 255}, {0, 0, 64, 255}},      // blue
  {{255, 64, 255, 255}, {128, 0, 128, 255}, {64, 0, 64, 255}},  // magenta
  {{100, 100, 100, 255}, {50, 50, 50, 255}, {25, 25, 25, 255}}, // dead color
  {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}                    // base color
  //{{200, 200, 200, 255}, {128, 128, 128, 255}, {64, 64, 64, 255}}, // gray
};

//void (*reload)() = (void(*)())0x90000020;
//void (*reloadGecko)() = (void(*)())0x80001800;