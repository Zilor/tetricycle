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

/** @file Player.cpp
 * @author Cale Scholl / calvinss4
 */

#include "Player.h"

#include <gcmodplay.h>     // for MODPlay
#include "libwiigui/gui.h" // for GuiSound
#include "Options.h"       // for Options
#include "tcyc_menu.h"     // for TCYC_MenuPause
#include "main.h"          // for GX_Cube

extern MODPlay g_modPlay; ///< used for playing the game music
extern GuiSound *g_tetrisCheerSound; ///< the tetris sound effect
extern Options *g_options;  ///< the global options
extern Player *g_players;   ///< the player instances
extern GXRModeObj *g_vmode; ///< the video mode
extern Mtx g_view; ///< the global view matrix

// Reset all state associated with this player.
void Player::Reset()
{
  memset(&gameData, 0, sizeof(PlayerGameData));
  gameData.pieces = -1;
  gameData.speed = START_SPEED;

  for (int y = 0; y < MAX_PLAYFIELD_HEIGHT; ++y)
  {
    for (int x = 0; x < MAX_PLAYFIELD_WIDTH; ++x)
      gameData.playfield[x][y].pieceId = TETRISPIECE_ID_NONE;
  }

  nextPiece.InitPiece(TetrisPiece::GetNextId());
  nextPiece.SetPowerupId(PowerupUtils::GetNextId(*this));
  _SpawnNextPiece();

  PowerupId *tmpPowerupStartQueue = !isHandicapEnabled ? 
    g_options->profile.powerupStartQueue : profile.powerupStartQueue;
  
  for (int i = 0; i < MAX_ACQUIRED_POWERUPS; ++i)
  {
    PowerupId pid = tmpPowerupStartQueue[i];
    if (pid != POWERUP_ID_NONE)
      gameData.powerupQueue[i] = PowerupUtils::GetStaticInstance(pid);
  }
}

// Draw the playfield (all the static tetris pieces).
void Player::DrawPlayfield()
{
  TetrisPieceConnectivityInfo *connectivityInfo = NULL;
  GuiImageData *imgData = NULL;

  for (int y = 0; y < playfieldHeight; ++y)
  {
    for (int x = 0; x < playfieldWidth; ++x)
    {
      TetrisPieceId pieceId = gameData.playfield[x][y].pieceId;
      if (pieceId == TETRISPIECE_ID_NONE)
        continue;

      ColorId gfx = (ColorId)pieceId;

      if (gameData.isDead)
      {
        gfx = COLOR_ID_DEAD;
      }
      else
      {
        connectivityInfo = gameData.playfield[x][y].connectivityInfo;
        imgData = !connectivityInfo ?
          NULL : PowerupUtils::GetImageData(connectivityInfo->powerupId);
      }

      DrawBlockAsCube(x, y, gfx, 255, imgData);
    }
  }
}

/** If no piece is specified, then the player's current piece is drawn. */
void Player::DrawPiece(TetrisPiece* cp, u8 alpha)
{
  TetrisPiece &piece = !cp ? currPiece : *cp;
  const TetrisPieceDesc &desc = piece.GetPieceDescription();

  ColorId gfx = (ColorId)piece.GetPieceId();
  GuiImageData *imgData = (piece.GetPowerupId() == POWERUP_ID_NONE) ? 
    NULL : PowerupUtils::GetImageData(piece.GetPowerupId());

  if (gameData.isDead)
  {
    gfx = COLOR_ID_DEAD;
    imgData = NULL;
  }

  // Every tetris piece fits into a 4x4 grid.
  for (int y = 0; y < 4; ++y)
  {
    for (int x = 0; x < 4; ++x)
    {
      // The playfield is a playfield_width x playfield_height grid 
      // (default: 10x20); the upper left corner corresponds to (0,0).
      // (piece.x, piece.y) is a playfield coordinate that corresponds to the 
      // upper left corner of the tetris piece's 4x4 grid. Thus,
      // (calcx, calcy) gives the playfield coordinate for each of the 
      // tetris piece's 4x4 grid elements.
      int calcx = piece.GetX() + x;
      int calcy = piece.GetY() + y;

      // Allow the screen to wrap horizontally.
      if (calcx >= playfieldWidth)
        calcx -= playfieldWidth;
      else if (calcx < 0)
        calcx += playfieldWidth;

      if (desc.map[x][y] && calcx >= 0 && calcy >= 0)
        DrawBlockAsCube(calcx, calcy, gfx, alpha, imgData);
    }
  }
}

// Draw where the current piece will end up if dropped.
void Player::DrawPieceShadow()
{
  if (!isShadowEnabled)
    return;

  // Create a copy of the current piece, move it down as much as possible, 
  // then draw it. Only draw the shadow piece if it is more than x spaces 
  // away from the actual piece.
  TetrisPiece piece = currPiece;
  int y = piece.GetY();
  while (MovePiece(DOWN, &piece));
  if (y < 7 && piece.GetY() > 11)
    DrawPiece(&piece, 145);
}

// Draw the base of the TetriCycle.
void Player::DrawBase()
{
  GXColor color;

  for (int x = 0; x < playfieldWidth; ++x)
  {
    int idx = (x + gameData.cycleIdx) % playfieldWidth;
    u8 c = idx * 8; // black to white
    color = (GXColor){c, c, c, 255};
    _SetBaseColor(color);
    DrawBlockAsCube(x, playfieldHeight, COLOR_ID_BASE);
  }
}

// Move the playfield.
bool Player::MovePlayfield(int type)
{
  TetrisPiece &piece = currPiece;

  int oldx = piece.GetX();
  int x = oldx;

  // First move the current piece and check that it's a valid move.
  // Moving the playfield right is equivalent to moving the piece left.
  switch (type)
  {
    case RIGHT:
      --x;
      if (x < 0)
        x += playfieldWidth;
      piece.SetX(x);
      break;

    case LEFT:
      ++x;
      if (x >= playfieldWidth)
        x -= playfieldWidth;
      piece.SetX(x);
      break;
  }

  bool canPlace = _CanPlacePiece();

  // Move the piece back to where it was.
  piece.SetX(oldx);

  if (!canPlace)
    return false;

  // Move the playfield.
  TetrisPieceBlock *playfieldTPB = &gameData.playfield[0][0];
  TetrisPieceBlock tmpTPB;
  TetrisPieceBlock *prevTPB, *currTPB;

  if (type == LEFT)
  {
    for (int r = 0; r < playfieldHeight; ++r)
    {
      prevTPB = playfieldTPB + r;
      currTPB = prevTPB + MAX_PLAYFIELD_HEIGHT;
      tmpTPB = *prevTPB;

      for (int c = 0; c < playfieldWidth - 1; ++c, prevTPB += MAX_PLAYFIELD_HEIGHT, currTPB += MAX_PLAYFIELD_HEIGHT)
        *prevTPB = *currTPB;

      *prevTPB = tmpTPB;
    }
  }
  else if (type == RIGHT)
  {
    for (int r = 0; r < playfieldHeight; ++r)
    {
      prevTPB = playfieldTPB + (playfieldWidth - 1) * MAX_PLAYFIELD_HEIGHT + r;
      currTPB = prevTPB - MAX_PLAYFIELD_HEIGHT;
      tmpTPB = *prevTPB;

      for (int c = 0; c < playfieldWidth - 1; ++c, prevTPB -= MAX_PLAYFIELD_HEIGHT, currTPB -= MAX_PLAYFIELD_HEIGHT)
        *prevTPB = *currTPB;

      *prevTPB = tmpTPB;
    }
  }

  return true;
}

/** If no piece is specified then the player's current piece is used. */
bool Player::MovePiece(int type, TetrisPiece *cp)
{
  TetrisPiece &piece = !cp ? currPiece : *cp;

  int oldx = piece.GetX();
  int oldy = piece.GetY();
  int x = oldx;

  switch(type)
  {
    case DOWN:
      piece.SetY(oldy + 1);
      break;

    case LEFT:
      --x;
      if (x < 0)
        x += playfieldWidth;
      piece.SetX(x);
      break;

    case RIGHT:
      ++x;
      if (x >= playfieldWidth)
        x -= playfieldWidth;
      piece.SetX(x);
      break;
  }

  if (_CanPlacePiece(&piece))
    return true;

  piece.SetX(oldx);
  piece.SetY(oldy);
  return false;
}

// Move the current piece down automatically.
void Player::DoMovement()
{
  const TetrisPieceDesc &desc = currPiece.GetPieceDescription();

  gameData.frame = 0;
  currPiece.IncrementDownCounter();

  bool canMove  = MovePiece(DOWN);
  bool canplace = _CanPlacePiece();

  // Write piece to map if it can't be moved more.
  if (!canMove)
  {
    TetrisPieceConnectivityInfo *info = (currPiece.GetPowerupId() == POWERUP_ID_NONE) ? 
      NULL : new TetrisPieceConnectivityInfo(currPiece.GetPowerupId());

    for (int y = 0; y < 4; ++y)
    {
      for (int x = 0; x < 4; ++x)
      {
        // The playfield is a playfield_width x playfield_height grid 
        // (default: 10x20); the upper left corner corresponds to (0,0).
        // (piece.x, piece.y) is a playfield coordinate that corresponds to the 
        // upper left corner of the tetris piece's 4x4 grid. Thus,
        // (calcx, calcy) gives the playfield coordinate for each of the 
        // tetris piece's 4x4 grid elements.
        int calcx = currPiece.GetX() + x;
        int calcy = currPiece.GetY() + y;

        // Allow the screen to wrap horizontally.
        if (calcx >= playfieldWidth)
          calcx -= playfieldWidth;
        else if (calcx < 0)
          calcx += playfieldWidth;

        if (calcx >= 0 && calcx < playfieldWidth && calcy >= 0 
            && calcy < playfieldHeight && desc.map[x][y])
        {
          gameData.playfield[calcx][calcy].pieceId = currPiece.GetPieceId();
          gameData.playfield[calcx][calcy].SetConnectivityInfo(info);
        }
      }
    }
  }

  // Player can't move or place ==> dead.
  if (!canMove && !canplace)
  {
    if (g_options->players == 1)
    {
      gameData.isDead = true;
      TCYC_MenuPause("Game Over");
      return;
    }

    gameData.isDead = true;
    _GetWinner();
    return;
  }

  // Can't move, make a new block.
  if (!canMove) 
  {
    int linesInFrame = 0;
    
    while (_RemoveLines())
      ++linesInFrame;

    if (linesInFrame == 3)
    {
      gameData.score++; // add one bonus point if you cleared 3 lines
    }
    else if (linesInFrame == 4)
    {
      gameData.score += 2;          // add two bonus points if you get a tetris
      MODPlay_Pause(&g_modPlay, 1); // pause music
      g_tetrisCheerSound->Play();   // Yay you got a tetris!
    }

    _SpawnNextPiece();
  }

//#ifndef EMU
//  if (g_options->netplay == 1 && plyrIdx == 0)
//    GC_BBA_SendUDPE(9003, (unsigned char*)map[0], 200);
//#endif
}

// Draw each tetris piece block as a cube.
void Player::DrawBlockAsCube(float x, float y, ColorId colorIdx, u8 alpha, GuiImageData *imgData)
{
  float scale = _GetScale() / (float)DEFAULT_PLAYFIELD_SCALE;
  float vx = 0;

  if (g_options->players == 1 && !g_options->isNetplay)
  {
    
  }
  else if (g_options->players == 2)
  {
    if (id == 0)
      vx = -106.6;
    else
      vx = 213.4;
  }
  else if (g_options->players == 3)
  {
    if (id == 0)
      vx = -210;
    else if (id == 2)
      vx = 210;
  }
  else if (g_options->players == 4)
  {
    if (id == 0)
      vx = -240;
    else if (id == 1)
      vx = -80;
    else if (id == 2)
      vx = 80;
    else
      vx = 240;
  }

  GX_SetViewport(vx + playfieldDX, playfieldDY, g_vmode->fbWidth, g_vmode->efbHeight, 0, 1);

  // x should be a value in {0,...,playfield_width - 1}
  // y should be a value in {0,...,playfield_height - 1}
  
  // Calculate the offset from the center of the playfield.
  int centerRight = playfieldWidth >> 1; // playfield_width / 2
  int centerLeft  = centerRight - 1;
  int offsetFromCenter = (x > centerLeft) ? x - centerLeft : x - centerRight;

  // Convert y to an offset from the bottom, and scale it.
  y = ((playfieldHeight >> 1) - 1 - y) * scale; // (playfield_height / 2) - 1

  // The half-cube z-translation.
  float hz = scale / 2;

  // TRS ==> Scale should be multiplied last.
  static Mtx mscale;
  guMtxIdentity(mscale);
  guMtxScale(mscale, scale, scale, scale);
  
  // The angle between cubes in degrees.
  float cubeRotation = cubeAngle;

  if (offsetFromCenter < 0)
  {
    scale *= -1;
    cubeRotation *= -1;
    offsetFromCenter *= -1;
  }
  
  // The cube translation matrix.
  // Translate the cube along the x-axis by its length.
  static Mtx mtrans;
  guMtxIdentity(mtrans);
  guMtxTransApply(mtrans, mtrans, scale, 0.f, 0.f); // src, dst, x, y, z

  // The cube rotation matrix.
  // Rotate the cube about the y-axis by cubeRotation degrees.
  static guVector cubeAxis = {0, 1, 0}; // y-axis
  static Mtx mrot;
  guMtxIdentity(mrot);
  guMtxRotAxisDeg(mrot, &cubeAxis, cubeRotation);

  static Mtx mrottrans;
  guMtxIdentity(mrottrans);
  guMtxConcat(mrot, mtrans, mrottrans);

  static Mtx model;
  static Mtx modelview;

  guMtxIdentity(model);

  for (int i = 0; i < offsetFromCenter - 1; ++i)
  {
    guMtxConcat(model, mrottrans, model);
  }

  guMtxConcat(model, mrot, model);

  // This halfcubetrans is applied before all other translations and rotations.
  static Mtx halfcubetrans;
  guMtxIdentity(halfcubetrans);
  guMtxTransApply(halfcubetrans, halfcubetrans, scale / 2, 0.f, -hz);

  guMtxConcat(model, halfcubetrans, model);
  guMtxConcat(model, mscale, model);
  
  // Translate the cube to the correct y and z position.
  static Mtx trans;
  guMtxIdentity(trans);
  guMtxTransApply(trans, trans, 0, y, -32); // src, dst, x, y, z
  
  guMtxConcat(trans, model, model);
  guMtxConcat(g_view, model, modelview);

  // load the modelview matrix into matrix memory
  GX_LoadPosMtxImm(modelview, GX_PNMTX0);
  GX_Cube(colorIdx, alpha, imgData);
  GX_SetViewport(0, 0, g_vmode->fbWidth, g_vmode->efbHeight, 0, 1);
}

//--- PRIVATE ---

void Player::_GetWinner()
{
  int ndead = 0;
  int winner = 0;

  for (int i = 0; i < g_options->players; ++i)
  {
    if (g_players[i].gameData.isDead)
      ++ndead;
    else
      winner = i;
  }

  if (ndead == g_options->players - 1)
    _DisplayWinner(winner);
}

void Player::_DisplayWinner(int winner)
{
  char buf[15];
  sprintf(buf, "Player %d wins!", winner + 1);
  TCYC_MenuPause(buf);
}

void Player::_IncreaseLevelAllBut(int plyrIdx)
{
  for (int i = 0; i < g_options->players; ++i)
  {
    if (i != plyrIdx)
      g_players[i]._IncreaseLevel();
  }
}

/** If no piece is specified then the player's current piece is used. */
bool Player::_CanPlacePiece(TetrisPiece *cp)
{
  TetrisPiece &piece = !cp ? currPiece : *cp;
  const TetrisPieceDesc &desc = piece.GetPieceDescription();

  for (int y = 0; y < 4; y++)
  {
    for (int x = 0; x < 4; x++)
    {
      // Only check squares that are set in the map
      if (desc.map[x][y])
      {
        // The playfield is a playfield_width x playfield_height grid 
        // (default: 10x20); the upper left corner corresponds to (0,0).
        // (piece.x, piece.y) is a playfield coordinate that corresponds to the 
        // upper left corner of the tetris piece's 4x4 grid. Thus,
        // (calcx, calcy) gives the playfield coordinate for each of the 
        // tetris piece's 4x4 grid elements.
        int calcx = piece.GetX() + x;
        int calcy = piece.GetY() + y;

        // Outside the map?
        // Allow the screen to wrap horizontally.
        if (calcx >= playfieldWidth)
          calcx -= playfieldWidth;
        else if (calcx < 0)
          calcx += playfieldWidth;

        if (calcy >= playfieldHeight)
          return false;

        // On top of something else?
        if (calcy >= 0 
            && gameData.playfield[calcx][calcy].pieceId != TETRISPIECE_ID_NONE)
          return false;
      }
    }
  }

  return true;
}

// Removes all the completed lines.
bool Player::_RemoveLines()
{
  int blocksInLine = 0;

  for (int y = 0; y < playfieldHeight; ++y)
  {
    for (int x = 0; x < playfieldWidth; ++x)
    {
      if (gameData.playfield[x][y].pieceId != TETRISPIECE_ID_NONE)
        ++blocksInLine;
    }

    if (blocksInLine == playfieldWidth)
    {
      _RemoveLine(y);
      return true;
    }

    blocksInLine = 0;
  }

  return false;
}

// Removes the line at the given row.
void Player::_RemoveLine(int line)
{
  TetrisPieceConnectivityInfo *connectivityInfo = NULL;

  for (int x = 0; x < playfieldWidth; ++x)
  {
    // If a block has connectivity info then it belongs to a powerup piece.
    connectivityInfo = gameData.playfield[x][line].connectivityInfo;
    if (connectivityInfo)
    {
      connectivityInfo->counter--;

      if (connectivityInfo->counter == 0)
      {
        // Add powerup to powerup queue.
        int slot = GetPowerupQueueSlot();
        if (slot >= 0)
        {
          Powerup *powerup = PowerupUtils::GetStaticInstance(connectivityInfo->powerupId);
          QueuePowerup(powerup, slot);
        }

        gameData.playfield[x][line].Free();
      }
    }
  }

  for (int y = line; y > 0; --y)
  {
    for (int x = 0; x < playfieldWidth; ++x)
    {
      gameData.playfield[x][y] = gameData.playfield[x][y-1];
    }
  }

  for (int x = 0; x < playfieldWidth; ++x)
  {
    gameData.playfield[x][0].pieceId = TETRISPIECE_ID_NONE;
    gameData.playfield[x][0].SetConnectivityInfo(NULL);
  }

  gameData.lines++;
  gameData.score++;

  if (g_options->players == 1)
  {
    if (gameData.lines % 10 == 0)
      _IncreaseLevel();

    return;
  }

  int tmpMaxLines = isHandicapEnabled ? profile.maxLines : g_options->profile.maxLines;

  if (tmpMaxLines && gameData.lines == tmpMaxLines)
  {
    _DisplayWinner(id);
    return;
  }

  int tmpAttackRate = isHandicapEnabled ? profile.attackRate : g_options->profile.attackRate;

  // Attack opponents whenever score is a multiple of attack rate.
  if (tmpAttackRate && gameData.score % tmpAttackRate == 0)
  {
    _IncreaseLevelAllBut(id);
  }	
}