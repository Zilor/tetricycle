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

/** @file main.cpp
 * @brief TetriCycle for Wii.
 * @author Cale Scholl / calvinss4
 *
 * @mainpage TetriCycle
 *
 * @section Introduction
 * TetriCycle is Tetris projected onto a cylindrical surface. Rotate the 
 * cylinder left/right such that the descending piece falls into place.
 * TetriCycle is freeware, can be distributed freely and should never be charged for. \n
 * http://www.wiibrew.org/wiki/TetriCycle
 *
 * @section License
 * TetriCycle is distributed under the GNU Lesser General Public License.
 *
 * @section Contact
 * If you have any suggestions for documentation, or want to contribute, 
 * please visit the TetriCycle code website: \n
 * http://code.google.com/p/tetricycle/ \n
 * Please do not release mods; contribute to the source!
 *
 * @section Credits
 * Version 1.0 written by calvinss4.
 * @par
 *
 * - Thanks to Sonicdude41 for the idea of Tetris on a Cylindrical Surface: http://forum.wiibrew.org/read.php?26,41651
 * - Thanks to DesktopMan for sharing his source: http://www.wiibrew.org/wiki/Tetris_Wii
 * - Thanks to Tantric for his awesome Libwiigui library: http://www.wiibrew.org/wiki/Libwiigui
 * - Menu music is from 'tetricycle by dj dimz': http://www.youtube.com/watch?v=JGfJUM6-e10
 *
 * @section powerupsection Powerup Framework
 * For a list of all powerups, please see the @subpage poweruplistpage "Powerup List" page. \n
 * For information on adding new powerups, please see the @subpage powerupcreationpage "Powerup Creation" page.
 */

#include <gcmodplay.h> // for MODPlay
#include <fat.h>       // for fatInitDefault

#include "main.h"       // for function prototypes
#include "tcyc_input.h" // for input macros
#include "tcyc_menu.h"  // for TCYC_MenuLoop
#include "audio.h"      // for InitAudio
#include "menu.h"       // for InitVideo
#include "Options.h"    // for Options
#include "Player.h"     // for Player

// include generated headers
#include "tetris_mod.h"
#include "pieces_bin.h"

extern TetrisPieceDesc g_pieceDesc[TETRISPIECE_ID_MAX][4]; ///< static description of every tetris piece for all 4 rotations
extern ColorGradient g_cubeGradients[COLOR_ID_MAX]; ///< gradients for coloring the face of a tetris piece block
extern Player *g_players;   ///< the player instances
extern u32 *g_xfb[2];       ///< the external frame buffer
extern GXRModeObj *g_vmode; ///< the video mode
extern Mtx GXmodelView2D;   ///< 2D modelview matrix
extern Mtx44 orthographic;  ///< 2D projection matrix
extern Mtx g_view;          ///< the global view matrix
extern Options *g_options;  ///< the global options
extern MODPlay g_modPlay;   ///< used for playing the game music
extern bool g_isEditMode;   ///< true when editing the playfield
extern int g_tcycMenu;      ///< the current menu state

extern GuiWindow *mainWindow;
extern GuiTrigger *trigA;
extern GuiSound *btnSoundOver;
extern GuiSound *g_tetrisCheerSound;
extern GuiImageData *btnData40x40Square;
extern GuiImageData *btnData40x40SquareOver;
extern GuiImageData *grabber[MAX_PLAYERS];
extern GuiImageData *debug_grabber1;
extern GuiImageData *debug_grabber2;
extern GuiImageData *debug_grabber3;
extern GuiImageData *debug_grabber4;
extern GuiImageData *pointer[MAX_PLAYERS];

static Mtx44 projection;

// function prototypes
static void TCYC_Update();
static void TCYC_Draw();
static void TCYC_DrawPlayfieldBoundary();
static void TCYC_DrawTetriCycle();
static void TCYC_DrawPowerups();
static void TCYC_DrawPowerupBorders();
static void TCYC_DrawPowerupTextures();
static void TCYC_DrawPowerupTexture(int upperLeftX, int upperLeftY, GuiImageData *imgData, u8 alpha);
static void TCYC_Render();

static void TCYC_DrawEditMode();
static void TCYC_DrawTetriCycleEditMode();
static void TCYC_DrawEditPlayfieldMenu();

// helper routines
static void TCYC_InitPieceDescriptions();
static void TCYC_GameInit();
static void TCYC_GameOnExit();

int main()
{
  // VIDEO and GX initialization is handled by video#InitVideo.
  // PAD and WPAD initialization is handled by input#SetupPads
  // AUDIO and ASND init is handled by audio#InitAudio
  InitVideo(); // Initialize video
  SetupPads(); // Initialize input
  InitAudio(); // Initialize audio
  fatInitDefault(); // Initialize file system
  InitFreeType((u8*)font_ttf, font_ttf_size); // Initialize font system
  InitGUIThreads(); // Initialize GUI

  // Initialize the view matrix.
  // Setup the camera at the origin, looking down the -z axis with y up.
  guVector cam  = {0.0F, 0.0F, 0.0F};
  guVector up   = {0.0F, 1.0F, 0.0F};
  guVector look = {0.0F, 0.0F, -1.0F};
  guLookAt(g_view, &cam, &up, &look);

  // Initialize the projection matrix.
  // This creates a perspective matrix with a view angle of 90,
  // and aspect ratio based on the display resolution.
  f32 w = g_vmode->viWidth;
  f32 h = g_vmode->viHeight;
  guPerspective(projection, 45, (f32)w/h, 0.1F, 300.0F);

  // Initialize the main gui resources.
  mainWindow = new GuiWindow(screenwidth, screenheight);
  pointer[0] = new GuiImageData(player1_point_png);
  pointer[1] = new GuiImageData(player2_point_png);
  pointer[2] = new GuiImageData(player3_point_png);
  pointer[3] = new GuiImageData(player4_point_png);

  // Initialize TetriCycle settings.
  // Vertex data initialization is handled by video#ResetVideo_Menu.
  TCYC_InitPieceDescriptions();
  MODPlay_Init(&g_modPlay);
  MODPlay_SetMOD(&g_modPlay, tetris_mod);
  g_totalPowerups = PowerupUtils::GetTotalPowerups();

  // Options and Players must be initialized after setting g_totalPowerups!
  g_options = &Options::GetInstance(); // the global options
  g_players = new Player[MAX_PLAYERS];
  for (int i = 0; i < MAX_PLAYERS; ++i)
    g_players[i].id = i;

  // Enter the game state loop.
  TCYC_MenuLoop();
}

/// Runs the game loop.
void TCYC_Game()
{
  TCYC_GameInit();

  while (true)
  {
    TCYC_Update();

    // Do this here so the screen doesn't flicker when the game is quit.
    if (g_tcycMenu != TCYC_MENU_NONE)
      break;

    TCYC_Draw();
    TCYC_Render();
  }

  TCYC_GameOnExit();
}

/// Updates the game.
void TCYC_Update()
{
  ALL_ScanPads();
  TCYC_ProcessInput();

  for (int i = 0; i < g_options->players; ++i)
    g_players[i].Update();

  if (!g_tetrisCheerSound->IsPlaying())
    MODPlay_Pause(&g_modPlay, 0); // unpause music
}

/// Draws the game.
void TCYC_Draw()
{
  TCYC_SetUp2D();
  TCYC_DrawPlayfieldBoundary();
  TCYC_DrawText();

  TCYC_DrawTetriCycle();

  if (g_options->players > 1)
  {
    TCYC_SetUp2D();
    TCYC_DrawPowerups();
  }
}

/// Prepares for drawing 2D.
void TCYC_SetUp2D()
{
  // Prepare for drawing 2D.
  GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_TRUE);
  GX_LoadPosMtxImm(GXmodelView2D, GX_PNMTX0);
  GX_LoadProjectionMtx(orthographic, GX_ORTHOGRAPHIC);
}

/// Draws the playfield boundary for every player.
void TCYC_DrawPlayfieldBoundary()
{
  switch (g_options->players)
  {
    case 1:
      GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        //--- 1P quad ---
        GX_Position3f32(0, 0, 0);     // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(640, 0, 0);   // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(640, 480, 0); // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(0, 480, 0);   // bottom left
        GX_Color4u8(88, 88, 108, 255);
      GX_End();
    break;

    case 2:
      GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
        //--- 1P quad ---
        GX_Position3f32(0, 0, 0);             // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(P2_X_BORDER, 0, 0);   // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(P2_X_BORDER, 480, 0); // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(0, 480, 0);           // bottom left
        GX_Color4u8(88, 88, 108, 255);

        //--- 2P quad ---
        GX_Position3f32(P2_X_BORDER, 0, 0);   // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(640, 0, 0);           // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(640, 480, 0);         // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(P2_X_BORDER, 480, 0); // bottom left
        GX_Color4u8(88, 88, 108, 255);
      GX_End();
    break;

    case 3:
      GX_Begin(GX_QUADS, GX_VTXFMT0, 12);
        //--- 1P quad ---
        GX_Position3f32(0, 0, 0);               // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(P3_X_BORDER_1, 0, 0);   // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(P3_X_BORDER_1, 480, 0); // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(0, 480, 0);             // bottom left
        GX_Color4u8(88, 88, 108, 255);

        //--- 2P quad ---
        GX_Position3f32(P3_X_BORDER_1, 0, 0);   // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(P3_X_BORDER_2, 0, 0);   // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(P3_X_BORDER_2, 480, 0); // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(P3_X_BORDER_1, 480, 0); // bottom left
        GX_Color4u8(88, 88, 108, 255);

        //--- 3P quad ---
        GX_Position3f32(P3_X_BORDER_2, 0, 0);   // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(640, 0, 0);             // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(640, 480, 0);           // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(P3_X_BORDER_2, 480, 0); // bottom left
        GX_Color4u8(88, 88, 108, 255);
      GX_End();
    break;

    case 4:
      GX_Begin(GX_QUADS, GX_VTXFMT0, 16);
        //--- 1P quad ---
        GX_Position3f32(0, 0, 0);               // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(P4_X_BORDER_1, 0, 0);   // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(P4_X_BORDER_1, 480, 0); // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(0, 480, 0);             // bottom left
        GX_Color4u8(88, 88, 108, 255);

        //--- 2P quad ---
        GX_Position3f32(P4_X_BORDER_1, 0, 0);   // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(P4_X_BORDER_2, 0, 0);   // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(P4_X_BORDER_2, 480, 0); // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(P4_X_BORDER_1, 480, 0); // bottom left
        GX_Color4u8(88, 88, 108, 255);

        //--- 3P quad ---
        GX_Position3f32(P4_X_BORDER_2, 0, 0);   // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(P4_X_BORDER_3, 0, 0);   // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(P4_X_BORDER_3, 480, 0); // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(P4_X_BORDER_2, 480, 0); // bottom left
        GX_Color4u8(88, 88, 108, 255);

        //--- 4P quad ---
        GX_Position3f32(P4_X_BORDER_3, 0, 0);   // top left
        GX_Color4u8(0, 0, 20, 255);
        GX_Position3f32(640, 0, 0);             // top right
        GX_Color4u8(88, 88, 108, 255);
        GX_Position3f32(640, 480, 0);           // bottom right
        GX_Color4u8(195, 195, 215, 255);
        GX_Position3f32(P4_X_BORDER_3, 480, 0); // bottom left
        GX_Color4u8(88, 88, 108, 255);
      GX_End();
    break;
  }
}

/// Draws in-game text such as lines, score, level.
void TCYC_DrawText()
{
  // Draw text. Add 2D effects such as lines, score, level.
  static const u8 a = 64; // alpha: 0 = transparent, 255 = opaque
  GXColor txtColor = !g_options->isPaused ? 
    (GXColor){255, 255, 255, a} : (GXColor){255, 255, 255, 255};
  GXColor deadColor = (GXColor){200, 0, 0, 255};
  
  GuiText txt(NULL, 28, txtColor); 
  txt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);

  char buf[5]; // allows for max value of 9999
  int player_x = 10, txt_dx = 100, y = 0, dy = 32;
  int player_dx = screenwidth / g_options->players; // assume: g_options->netplay == 0

  for (int i = 0; i < g_options->players; ++i, player_x += player_dx)
  {
    y = 0;
    txt.SetColor(!g_players[i].gameData.isDead ? txtColor : deadColor);

    // Draw lines, score, level.
    txt.SetText("lines:");
    txt.SetPosition(player_x, y += dy);
    txt.Draw();
    sprintf(buf, "%d", g_players[i].gameData.lines);
    txt.SetText(buf);
    txt.SetPosition(player_x + txt_dx, y);
    txt.Draw();

    txt.SetText("score:");
    txt.SetPosition(player_x, y += dy);
    txt.Draw();
    sprintf(buf, "%d", g_players[i].gameData.score);
    txt.SetText(buf);
    txt.SetPosition(player_x + txt_dx, y);
    txt.Draw();

    txt.SetText("level:");
    txt.SetPosition(player_x, y += dy);
    txt.Draw();
    sprintf(buf, "%d", g_players[i].gameData.level);
    txt.SetText(buf);
    txt.SetPosition(player_x + txt_dx, y);
    txt.Draw();
  }
}

/// Returns the player associated with the passed in x-coordinate.
int TCYC_GetTargetPlayer(int x)
{
  switch (g_options->players)
  {
    case 2: return x < P2_X_BORDER ? 0 : 1;
    case 3: return x < P3_X_BORDER_1 ? 0 : (x < P3_X_BORDER_2 ? 1 : 2);
    case 4: return x < P4_X_BORDER_1 ? 0 : (x < P4_X_BORDER_2 ? 1 : (x < P4_X_BORDER_3 ? 2 : 3));
  }

  TCYC_MenuError("Error in function TCYC_GetTargetPlayer.");
  return 0;
}

/// Returns the player's powerup slot associated with the passed in (x,y)-coordinate.
/** Returns -1 if the cursor is outside the powerup zone. */
int TCYC_GetTargetPowerupSlot(int player, int x, int y)
{
  int borders[MAX_PLAYERS+1];
  borders[0] = 0;
  borders[g_options->players] = 640;

  switch (g_options->players)
  {
    case 2:
      borders[1] = P2_X_BORDER;
      break;
    case 3:
      borders[1] = P3_X_BORDER_1;
      borders[2] = P3_X_BORDER_2;
      break;
    case 4:
      borders[1] = P4_X_BORDER_1;
      borders[2] = P4_X_BORDER_2;
      borders[3] = P4_X_BORDER_3;
      break;
  }

  // Check if we are in the x-zone.
  if (x >= borders[player+1] - POWERUP_WIDTH - 2 * POWERUP_X_OFFSET && x <= borders[player+1])
  {
    // Determine what y-zone we are in.
    int py = POWERUP_Y_OFFSET + POWERUP_WIDTH;
    for (int i = 0; i < MAX_ACQUIRED_POWERUPS - 1; ++i, py += POWERUP_WIDTH)
    {
      if (y < py)
        return i;
    }

    if (y < py + POWERUP_Y_OFFSET)
      return MAX_ACQUIRED_POWERUPS - 1;
  }

  return -1;
}

/// Draws the black borders for the on-screen powerup queues.
void TCYC_DrawPowerupBorders()
{
  int borders[MAX_PLAYERS+1];
  borders[0] = 0;
  borders[g_options->players] = 640;

  switch (g_options->players)
  {
    case 2:
      borders[1] = P2_X_BORDER;
      break;
    case 3:
      borders[1] = P3_X_BORDER_1;
      borders[2] = P3_X_BORDER_2;
      break;
    case 4:
      borders[1] = P4_X_BORDER_1;
      borders[2] = P4_X_BORDER_2;
      borders[3] = P4_X_BORDER_3;
      break;
  }

  int x;
  int y;

  for (int i = 0; i < g_options->players; ++i)
  {
    x = 0;
    y = 0;

    // powerup queue
    GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 2);
    GX_Position3f32(borders[i+1] - POWERUP_X_OFFSET - POWERUP_WIDTH, POWERUP_Y_OFFSET, 0); // top left
    GX_Color4u8(0, 0, 0, 255);
    GX_Position3f32(borders[i+1] - POWERUP_X_OFFSET, POWERUP_Y_OFFSET, 0); // top right
    GX_Color4u8(0, 0, 0, 255);
    GX_End();
  
    for (int j = 0; j < MAX_ACQUIRED_POWERUPS; ++j, y += POWERUP_WIDTH)
    {
      GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
      GX_Position3f32(borders[i+1] - POWERUP_X_OFFSET - POWERUP_WIDTH, y + POWERUP_Y_OFFSET, 0); // top left
      GX_Color4u8(0, 0, 0, 255);
      GX_Position3f32(borders[i+1] - POWERUP_X_OFFSET - POWERUP_WIDTH, y + POWERUP_Y_OFFSET + POWERUP_WIDTH, 0); // bottom left
      GX_Color4u8(0, 0, 0, 255);
      GX_Position3f32(borders[i+1] - POWERUP_X_OFFSET, y + POWERUP_Y_OFFSET + POWERUP_WIDTH, 0); // bottom right
      GX_Color4u8(0, 0, 0, 255);
      GX_Position3f32(borders[i+1] - POWERUP_X_OFFSET, y + POWERUP_Y_OFFSET, 0); // top right
      GX_Color4u8(0, 0, 0, 255);
      GX_End();
    }

    // effects queue
    GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 2);
    GX_Position3f32(borders[i] + POWERUP_X_OFFSET, 480 - POWERUP_Y_OFFSET - POWERUP_WIDTH, 0); // top left
    GX_Color4u8(0, 0, 0, 255);
    GX_Position3f32(borders[i] + POWERUP_X_OFFSET, 480 - POWERUP_Y_OFFSET, 0); // bottom left
    GX_Color4u8(0, 0, 0, 255);
    GX_End();

    for (int j = 0; j < MAX_POWERUP_EFFECTS; ++j, x += POWERUP_WIDTH)
    {
      GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 4);
      GX_Position3f32(borders[i] + POWERUP_X_OFFSET + x, 480 - POWERUP_Y_OFFSET - POWERUP_WIDTH, 0); // top left
      GX_Color4u8(0, 0, 0, 255);
      GX_Position3f32(borders[i] + POWERUP_X_OFFSET + POWERUP_WIDTH + x, 480 - POWERUP_Y_OFFSET - POWERUP_WIDTH, 0); // top right
      GX_Color4u8(0, 0, 0, 255);
      GX_Position3f32(borders[i] + POWERUP_X_OFFSET + POWERUP_WIDTH + x, 480 - POWERUP_Y_OFFSET, 0); // bottom right
      GX_Color4u8(0, 0, 0, 255);
      GX_Position3f32(borders[i] + POWERUP_X_OFFSET + x, 480 - POWERUP_Y_OFFSET, 0); // bottom left
      GX_Color4u8(0, 0, 0, 255);
      GX_End();
    }
  }
}

/// Draws a powerup texture.
/** (x,y) corresponds to the upper left corner. */
void TCYC_DrawPowerupTexture(int x, int y, GuiImageData *imgData, u8 alpha)
{
  GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
  GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

  GXTexObj texObj;
  GX_InitTexObj(&texObj, imgData->GetImage(), imgData->GetWidth(), imgData->GetHeight(), GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
  GX_LoadTexObj(&texObj, GX_TEXMAP0);
  GX_InvalidateTexAll();

  GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
  GX_Position3f32(x, y, 0); // top left
  GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
  GX_TexCoord2f32(0, 0);
  GX_Position3f32(x + POWERUP_WIDTH, y, 0); // top right
  GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
  GX_TexCoord2f32(1, 0);
  GX_Position3f32(x + POWERUP_WIDTH, y + POWERUP_WIDTH, 0); // bottom right
  GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
  GX_TexCoord2f32(1, 1);
  GX_Position3f32(x, y + POWERUP_WIDTH, 0); // bottom left
  GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
  GX_TexCoord2f32(0, 1);
  GX_End();

  GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
  GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

/// Draws the textures for all the on-screen powerups.
void TCYC_DrawPowerupTextures()
{
  static const int yoffset = POWERUP_Y_OFFSET;
  static const int xoffset = POWERUP_X_OFFSET;
  static const int width = POWERUP_WIDTH;

  int x;
  int y;
  u8 alpha;
  Powerup *powerup;
  GuiImageData *imgData;
  GXTexObj texObj;

  int borders[MAX_PLAYERS+1];
  borders[0] = 0;
  borders[g_options->players] = 640;

  switch (g_options->players)
  {
    case 2:
      borders[1] = P2_X_BORDER;
      break;
    case 3:
      borders[1] = P3_X_BORDER_1;
      borders[2] = P3_X_BORDER_2;
      break;
    case 4:
      borders[1] = P4_X_BORDER_1;
      borders[2] = P4_X_BORDER_2;
      borders[3] = P4_X_BORDER_3;
      break;
  }

  GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
  GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

  for (int i = 0; i < g_options->players; ++i)
  {
    x = 0;
    y = 0;
    alpha = 128;

    // powerup queue
    for (int j = 0; j < MAX_ACQUIRED_POWERUPS; ++j, y += width)
    {
      powerup = g_players[i].gameData.powerupQueue[j];
      if (powerup)
      {
        imgData = powerup->GetImageData();
        GX_InitTexObj(&texObj, imgData->GetImage(), imgData->GetWidth(), imgData->GetHeight(), GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GX_LoadTexObj(&texObj, GX_TEXMAP0);
        GX_InvalidateTexAll();

        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(borders[i+1] - xoffset - width, y + yoffset, 0); // top left
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(0, 0);
        GX_Position3f32(borders[i+1] - xoffset, y + yoffset, 0); // top right
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(1, 0);
        GX_Position3f32(borders[i+1] - xoffset, y + yoffset + width, 0); // bottom right
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(1, 1);
        GX_Position3f32(borders[i+1] - xoffset - width, y + yoffset + width, 0); // bottom left
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(0, 1);
        GX_End();
      }
    }

    alpha = 64;

    // effects queue
    for (int j = 0; j < MAX_POWERUP_EFFECTS; ++j, x += width)
    {
      powerup = g_players[i].gameData.powerupEffects[j];
      if (powerup)
      {
        imgData = powerup->GetImageData();
        GX_InitTexObj(&texObj, imgData->GetImage(), imgData->GetWidth(), imgData->GetHeight(), GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GX_LoadTexObj(&texObj, GX_TEXMAP0);
        GX_InvalidateTexAll();

        GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
        GX_Position3f32(borders[i] + xoffset + x, 480 - yoffset - width, 0); // top left
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(0, 0);
        GX_Position3f32(borders[i] + xoffset + width + x, 480 - yoffset - width, 0); // top right
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(1, 0);
        GX_Position3f32(borders[i] + xoffset + width + x, 480 - yoffset, 0); // bottom right
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(1, 1);
        GX_Position3f32(borders[i] + xoffset + x, 480 - yoffset, 0); // bottom left
        GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
        GX_TexCoord2f32(0, 1);
        GX_End();
      }
    }
  }

  GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
  GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
}

/// Draws the on-screen powerups.
void TCYC_DrawPowerups()
{
  TCYC_DrawPowerupBorders();
  TCYC_DrawPowerupTextures();

  for (int i = g_options->players - 1; i >= 0; --i) // so that player 1's cursor appears on top!
  {
    if (userInput[i].wpad->ir.valid)
    {
      int x = userInput[i].wpad->ir.x;
      int y = userInput[i].wpad->ir.y;
      u8 alpha = (TCYC_GetTargetPlayer(x) == i) ? 255 : 32;
      Powerup *powerup = g_players[i].gameData.grabbedPowerup;

      if (powerup)
        TCYC_DrawPowerupTexture(x - (POWERUP_WIDTH >> 1), y - (POWERUP_WIDTH >> 1), powerup->GetImageData(), alpha);

      GuiImageData *imgData = GRAB_HELD(i) ? grabber[i] : pointer[i];
      float scale = 1;

      if (g_players[i].gameData.powerupData.isBigHand)
      {
        scale = BIG_HAND_SCALE;
        alpha = 255;
      }

      // TODO Cale - There is a weird bug here. If I try to reference grabber[0] 
      // in 4-player mode then it crashes the game!
      /*if (g_options->players < MAX_PLAYERS || i != 0)
        Menu_DrawImg(x - 48, y - 48, 96, 96, imgData->GetImage(), userInput[i].wpad->ir.angle, scale, scale, alpha);
      else
        Menu_DrawImg(x - 48, y - 48, 96, 96, GRAB_HELD(i) ? debug_grabber1->GetImage() : imgData->GetImage(), userInput[i].wpad->ir.angle, scale, scale, alpha);*/

      // DEBUG FURTHER! The above code still causes the game to crash in 
      // strange ways, for example when switching from 4-player mode to 
      // 3-player mode.
      switch (i)
      {
        case 0:
          imgData = GRAB_HELD(i) ? debug_grabber1 : pointer[i];
          break;
        case 1:
          imgData = GRAB_HELD(i) ? debug_grabber2 : pointer[i];
          break;
        case 2:
          imgData = GRAB_HELD(i) ? debug_grabber3 : pointer[i];
          break;
        case 3:
          imgData = GRAB_HELD(i) ? debug_grabber4 : pointer[i];
          break;
      }
      Menu_DrawImg(x - 48, y - 48, 96, 96, imgData->GetImage(), userInput[i].wpad->ir.angle, scale, scale, alpha);
    }

    DoRumble(i);
  }
}

/// Draws the 3D TetriCycle for every player.
void TCYC_DrawTetriCycle()
{
  // Prepare for drawing TetriCycle
  GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
  GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

  for (int i = 0; i < g_options->players; ++i)
  {
    Player &player = g_players[i];
    player.DrawPlayfield();
    player.DrawNextPiece();
    player.DrawPiece();
    player.DrawPieceShadow();
    player.DrawBase();
  }
}

/// Renders the game.
void TCYC_Render()
{
  static int currFrame = 0;
  currFrame ^= 1; // flip framebuffer

  GX_CopyDisp(g_xfb[currFrame], GX_TRUE);
  GX_DrawDone();

  VIDEO_SetNextFramebuffer(g_xfb[currFrame]);
  VIDEO_Flush();   
  VIDEO_WaitVSync();

  for (int i = 0; i < g_options->players; ++i)
    g_players[i].gameData.frame++;
}

/// Draws a unit cube centered at the origin.
void GX_Cube(int colorIdx, u8 alpha, GuiImageData *imgData)
{
  ColorGradient &gradient = g_cubeGradients[colorIdx];

  u8 lightR = gradient.light.r;
  u8 lightG = gradient.light.g;
  u8 lightB = gradient.light.b;

  u8 mediumR = gradient.medium.r;
  u8 mediumG = gradient.medium.g;
  u8 mediumB = gradient.medium.b;

  u8 darkR = gradient.dark.r;
  u8 darkG = gradient.dark.g;
  u8 darkB = gradient.dark.b;
  
  static const float unit = 0.5;

  GX_Begin(GX_QUADS, GX_VTXFMT0, 8);
    //--- TOP quad ---
    GX_Position3f32(-unit, unit, -unit);  // top left
    GX_Color4u8(lightR, lightG, lightB, alpha);
    GX_Position3f32(unit, unit, -unit);   // top right
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(unit, unit, unit);    // bottom right
    GX_Color4u8(darkR, darkG, darkB, alpha);
    GX_Position3f32(-unit, unit, unit);   // bottom left
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);

    //--- BOTTOM quad ---
    GX_Position3f32(-unit, -unit, unit);  // top left
    GX_Color4u8(lightR, lightG, lightB, alpha);
    GX_Position3f32(unit, -unit, unit);   // top right
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(unit, -unit, -unit);  // bottom right
    GX_Color4u8(darkR, darkG, darkB, alpha);
    GX_Position3f32(-unit, -unit, -unit); // bottom left
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
  GX_End();

  //--- FRONT quad ---
  if (!imgData)
  {
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(-unit, unit, unit);   // top left
    GX_Color4u8(lightR, lightG, lightB, alpha);
    GX_Position3f32(unit, unit, unit);    // top right
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(unit, -unit, unit);   // bottom right
    GX_Color4u8(darkR, darkG, darkB, alpha);
    GX_Position3f32(-unit, -unit, unit);  // bottom left
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_End();
  }
  else
  {
    GXTexObj texObj;

    int width  = imgData->GetWidth();
    int height = imgData->GetHeight();
    GX_InitTexObj(&texObj, imgData->GetImage(), width, height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_LoadTexObj(&texObj, GX_TEXMAP0);
    GX_InvalidateTexAll();

    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(-unit, unit, unit);   // top left
    GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
    GX_TexCoord2f32(0, 0);
    GX_Position3f32(unit, unit, unit);    // top right
    GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
    GX_TexCoord2f32(1, 0);
    GX_Position3f32(unit, -unit, unit);   // bottom right
    GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
    GX_TexCoord2f32(1, 1);
    GX_Position3f32(-unit, -unit, unit);  // bottom left
    GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
    GX_TexCoord2f32(0, 1);
    GX_End();

    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);
  }

  GX_Begin(GX_QUADS, GX_VTXFMT0, 12);
    //--- BACK quad ---
    GX_Position3f32(unit, -unit, -unit);  // bottom left
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(-unit, -unit, -unit); // bottom right
    GX_Color4u8(darkR, darkG, darkB, alpha);
    GX_Position3f32(-unit, unit, -unit);  // top right
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(unit, unit, -unit);   // top left
    GX_Color4u8(lightR, lightG, lightB, alpha);

    //--- LEFT quad ---
    GX_Position3f32(-unit, unit, unit);   // top right
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(-unit, unit, -unit);  // top left
    GX_Color4u8(lightR, lightG, lightB, alpha);
    GX_Position3f32(-unit, -unit, -unit); // bottom left
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(-unit, -unit, unit);  // bottom right
    GX_Color4u8(darkR, darkG, darkB, alpha);

    //--- RIGHT quad ---
    GX_Position3f32(unit, unit, -unit);  // top right
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(unit, unit, unit);   // top left
    GX_Color4u8(lightR, lightG, lightB, alpha);
    GX_Position3f32(unit, -unit, unit);  // bottom left
    GX_Color4u8(mediumR, mediumG, mediumB, alpha);
    GX_Position3f32(unit, -unit, -unit); // bottom right
    GX_Color4u8(darkR, darkG, darkB, alpha);
  GX_End();
}

/// Runs the "edit playfield" loop.
void TCYC_EditPlayfield()
{
  GXColor white = (GXColor){255, 255, 255, 255};
  GXColor green = (GXColor){0, 255, 0, 255};

  char buf[7 + 5]; // allows for max value of 9999
  int player_x = 10, y = 0, dy = 32;
  int player_dx = screenwidth / g_options->players; // assume: g_options->netplay == 0

  GuiWindow window(screenwidth, screenheight);

  // CONFIRM BUTTON
  GuiButton confirmBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText confirmBtnTxt("OK", 22, (GXColor){0, 0, 255, 255});
  GuiImage confirmBtnImg(btnData40x40Square);
  GuiImage confirmBtnImgOver(btnData40x40SquareOver);

  confirmBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  confirmBtn.SetPosition(-10, -20);
  confirmBtn.SetLabel(&confirmBtnTxt);
  confirmBtn.SetImage(&confirmBtnImg);
  confirmBtn.SetImageOver(&confirmBtnImgOver);
  confirmBtn.SetSoundOver(btnSoundOver);
  confirmBtn.SetTrigger(trigA);
  confirmBtn.SetEffectGrow();

  // CANCEL BUTTON
  GuiButton cancelBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText cancelBtnTxt("X", 22, (GXColor){255, 0, 0, 255});
  GuiImage cancelBtnImg(btnData40x40Square);
  GuiImage cancelBtnImgOver(btnData40x40SquareOver);

  cancelBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
  cancelBtn.SetPosition(10, -20);
  cancelBtn.SetLabel(&cancelBtnTxt);
  cancelBtn.SetImage(&cancelBtnImg);
  cancelBtn.SetImageOver(&cancelBtnImgOver);
  cancelBtn.SetSoundOver(btnSoundOver);
  cancelBtn.SetTrigger(trigA);
  cancelBtn.SetEffectGrow();

  // 1P Playfield Width Button
  GuiButton p1widthBtn((9 + 1) * 14, 28);
  GuiText p1widthTxt(NULL, 28, white);
  GuiText p1widthTxtOver(NULL, 28, green);

  // 2P Playfield Width Button
  GuiButton p2widthBtn((9 + 1) * 14, 28);
  GuiText p2widthTxt(NULL, 28, white);
  GuiText p2widthTxtOver(NULL, 28, green);

  // 3P Playfield Width Button
  GuiButton p3widthBtn((9 + 1) * 14, 28);
  GuiText p3widthTxt(NULL, 28, white);
  GuiText p3widthTxtOver(NULL, 28, green);

  // 4P Playfield Width Button
  GuiButton p4widthBtn((9 + 1) * 14, 28);
  GuiText p4widthTxt(NULL, 28, white);
  GuiText p4widthTxtOver(NULL, 28, green);

  // 1P Playfield Scale Button
  GuiButton p1scaleBtn((9 + 1) * 14, 28);
  GuiText p1scaleTxt(NULL, 28, white);
  GuiText p1scaleTxtOver(NULL, 28, green);

  // 2P Playfield Scale Button
  GuiButton p2scaleBtn((9 + 1) * 14, 28);
  GuiText p2scaleTxt(NULL, 28, white);
  GuiText p2scaleTxtOver(NULL, 28, green);

  // 3P Playfield Scale Button
  GuiButton p3scaleBtn((9 + 1) * 14, 28);
  GuiText p3scaleTxt(NULL, 28, white);
  GuiText p3scaleTxtOver(NULL, 28, green);

  // 4P Playfield Scale Button
  GuiButton p4scaleBtn((9 + 1) * 14, 28);
  GuiText p4scaleTxt(NULL, 28, white);
  GuiText p4scaleTxtOver(NULL, 28, green);

  // 1P Playfield Angle Button
  GuiButton p1angleBtn((9 + 1) * 14, 28);
  GuiText p1angleTxt(NULL, 28, white);
  GuiText p1angleTxtOver(NULL, 28, green);

  // 2P Playfield Angle Button
  GuiButton p2angleBtn((9 + 1) * 14, 28);
  GuiText p2angleTxt(NULL, 28, white);
  GuiText p2angleTxtOver(NULL, 28, green);

  // 3P Playfield Angle Button
  GuiButton p3angleBtn((9 + 1) * 14, 28);
  GuiText p3angleTxt(NULL, 28, white);
  GuiText p3angleTxtOver(NULL, 28, green);

  // 4P Playfield Angle Button
  GuiButton p4angleBtn((9 + 1) * 14, 28);
  GuiText p4angleTxt(NULL, 28, white);
  GuiText p4angleTxtOver(NULL, 28, green);

  // 1P Playfield DX Button
  GuiButton p1dxBtn((7 + 1) * 14, 28);
  GuiText p1dxTxt(NULL, 28, white);
  GuiText p1dxTxtOver(NULL, 28, green);

  // 2P Playfield DX Button
  GuiButton p2dxBtn((7 + 1) * 14, 28);
  GuiText p2dxTxt(NULL, 28, white);
  GuiText p2dxTxtOver(NULL, 28, green);

  // 3P Playfield DX Button
  GuiButton p3dxBtn((7 + 1) * 14, 28);
  GuiText p3dxTxt(NULL, 28, white);
  GuiText p3dxTxtOver(NULL, 28, green);

  // 4P Playfield DX Button
  GuiButton p4dxBtn((7 + 1) * 14, 28);
  GuiText p4dxTxt(NULL, 28, white);
  GuiText p4dxTxtOver(NULL, 28, green);

  // 1P Playfield DY Button
  GuiButton p1dyBtn((7 + 1) * 14, 28);
  GuiText p1dyTxt(NULL, 28, white);
  GuiText p1dyTxtOver(NULL, 28, green);

  // 2P Playfield DY Button
  GuiButton p2dyBtn((7 + 1) * 14, 28);
  GuiText p2dyTxt(NULL, 28, white);
  GuiText p2dyTxtOver(NULL, 28, green);

  // 3P Playfield DY Button
  GuiButton p3dyBtn((7 + 1) * 14, 28);
  GuiText p3dyTxt(NULL, 28, white);
  GuiText p3dyTxtOver(NULL, 28, green);

  // 4P Playfield DY Button
  GuiButton p4dyBtn((7 + 1) * 14, 28);
  GuiText p4dyTxt(NULL, 28, white);
  GuiText p4dyTxtOver(NULL, 28, green);

  // GuiElement Arrays
  GuiButton *widthButtons[MAX_PLAYERS] = {&p1widthBtn, &p2widthBtn, &p3widthBtn, &p4widthBtn};
  GuiText *widthTexts[MAX_PLAYERS]     = {&p1widthTxt, &p2widthTxt, &p3widthTxt, &p4widthTxt};
  GuiText *widthTextsOver[MAX_PLAYERS] = {&p1widthTxtOver, &p2widthTxtOver, &p3widthTxtOver, &p4widthTxtOver};

  GuiButton *scaleButtons[MAX_PLAYERS] = {&p1scaleBtn, &p2scaleBtn, &p3scaleBtn, &p4scaleBtn};
  GuiText *scaleTexts[MAX_PLAYERS]     = {&p1scaleTxt, &p2scaleTxt, &p3scaleTxt, &p4scaleTxt};
  GuiText *scaleTextsOver[MAX_PLAYERS] = {&p1scaleTxtOver, &p2scaleTxtOver, &p3scaleTxtOver, &p4scaleTxtOver};

  GuiButton *angleButtons[MAX_PLAYERS] = {&p1angleBtn, &p2angleBtn, &p3angleBtn, &p4angleBtn};
  GuiText *angleTexts[MAX_PLAYERS]     = {&p1angleTxt, &p2angleTxt, &p3angleTxt, &p4angleTxt};
  GuiText *angleTextsOver[MAX_PLAYERS] = {&p1angleTxtOver, &p2angleTxtOver, &p3angleTxtOver, &p4angleTxtOver};

  GuiButton *dxButtons[MAX_PLAYERS] = {&p1dxBtn, &p2dxBtn, &p3dxBtn, &p4dxBtn};
  GuiText *dxTexts[MAX_PLAYERS]     = {&p1dxTxt, &p2dxTxt, &p3dxTxt, &p4dxTxt};
  GuiText *dxTextsOver[MAX_PLAYERS] = {&p1dxTxtOver, &p2dxTxtOver, &p3dxTxtOver, &p4dxTxtOver};

  GuiButton *dyButtons[MAX_PLAYERS] = {&p1dyBtn, &p2dyBtn, &p3dyBtn, &p4dyBtn};
  GuiText *dyTexts[MAX_PLAYERS]     = {&p1dyTxt, &p2dyTxt, &p3dyTxt, &p4dyTxt};
  GuiText *dyTextsOver[MAX_PLAYERS] = {&p1dyTxtOver, &p2dyTxtOver, &p3dyTxtOver, &p4dyTxtOver};

  // previous values
  u8 prevWidth[MAX_PLAYERS];
  u8 prevScale[MAX_PLAYERS];
  float prevAngle[MAX_PLAYERS];
  s16 prevDX[MAX_PLAYERS];
  s16 prevDY[MAX_PLAYERS];

  for (int i = 0; i < g_options->players; ++i, player_x += player_dx)
  {
    y = 0;

    widthButtons[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    widthButtons[i]->SetPosition(player_x, y += dy);
    widthButtons[i]->SetLabel(widthTexts[i]);
    widthButtons[i]->SetLabelOver(widthTextsOver[i]);
    widthButtons[i]->SetEffectGrow();     
    sprintf(buf, "width: %d", g_players[i].playfieldWidth);
    widthTexts[i]->SetText(buf);
    widthTextsOver[i]->SetText(buf);

    scaleButtons[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    scaleButtons[i]->SetPosition(player_x, y += dy);
    scaleButtons[i]->SetLabel(scaleTexts[i]);
    scaleButtons[i]->SetLabelOver(scaleTextsOver[i]);
    scaleButtons[i]->SetEffectGrow();     
    sprintf(buf, "scale: %d", g_players[i].playfieldScale);
    scaleTexts[i]->SetText(buf);
    scaleTextsOver[i]->SetText(buf);

    angleButtons[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    angleButtons[i]->SetPosition(player_x, y += dy);
    angleButtons[i]->SetLabel(angleTexts[i]);
    angleButtons[i]->SetLabelOver(angleTextsOver[i]);
    angleButtons[i]->SetEffectGrow();     
    sprintf(buf, "angle: %d", (int)g_players[i].cubeAngle);
    angleTexts[i]->SetText(buf);
    angleTextsOver[i]->SetText(buf);

    dxButtons[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    dxButtons[i]->SetPosition(player_x, y += dy);
    dxButtons[i]->SetLabel(dxTexts[i]);
    dxButtons[i]->SetLabelOver(dxTextsOver[i]);
    dxButtons[i]->SetEffectGrow();     
    sprintf(buf, "dx: %d", g_players[i].playfieldDX);
    dxTexts[i]->SetText(buf);
    dxTextsOver[i]->SetText(buf);

    dyButtons[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    dyButtons[i]->SetPosition(player_x, y += dy);
    dyButtons[i]->SetLabel(dyTexts[i]);
    dyButtons[i]->SetLabelOver(dyTextsOver[i]);
    dyButtons[i]->SetEffectGrow();     
    sprintf(buf, "dy: %d", g_players[i].playfieldDY);
    dyTexts[i]->SetText(buf);
    dyTextsOver[i]->SetText(buf);

    window.Append(widthButtons[i]);
    window.Append(scaleButtons[i]);
    window.Append(angleButtons[i]);
    window.Append(dxButtons[i]);
    window.Append(dyButtons[i]);

    prevWidth[i] = g_players[i].playfieldWidth;
    prevScale[i] = g_players[i].playfieldScale;
    prevAngle[i] = g_players[i].cubeAngle;
    prevDX[i] = g_players[i].playfieldDX;
    prevDY[i] = g_players[i].playfieldDY;
  }

  window.Append(&confirmBtn);
  window.Append(&cancelBtn);
  mainWindow->Append(&window);

  g_tcycMenu = TCYC_MENU_NONE;

  while (g_tcycMenu == TCYC_MENU_NONE)
  {
    ALL_ScanPads();
    TCYC_ProcessEditModeInput();
    TCYC_DrawEditMode();
    TCYC_Render();

    if (confirmBtn.GetState() == STATE_CLICKED)
    {
      g_tcycMenu = TCYC_MENU_MAIN;
      break;
    }
    else if (cancelBtn.GetState() == STATE_CLICKED)
    {
      for (int i = 0; i < g_options->players; ++i)
      {
        g_players[i].playfieldWidth = prevWidth[i];
        g_players[i].playfieldScale = prevScale[i];
        g_players[i].cubeAngle = prevAngle[i];
        g_players[i].playfieldDX = prevDX[i];
        g_players[i].playfieldDY = prevDY[i];
      }

      g_tcycMenu = TCYC_MENU_MAIN;
      break;
    }

    for (int i = 0; i < g_options->players; ++i)
    {
      if (widthButtons[i]->GetState() == STATE_SELECTED)
      {
        int incr = 0;

        if (A_PRESSED(i))
          incr = 2;
        else if (B_PRESSED(i))
          incr = -2;

        if (incr)
        {
          g_players[i].playfieldWidth += incr;

          if (g_players[i].playfieldWidth > MAX_PLAYFIELD_WIDTH)
            g_players[i].playfieldWidth = MAX_PLAYFIELD_WIDTH;
          else if (g_players[i].playfieldWidth < MIN_PLAYFIELD_WIDTH)
            g_players[i].playfieldWidth = MIN_PLAYFIELD_WIDTH;

          sprintf(buf, "width: %d", g_players[i].playfieldWidth);
          widthTexts[i]->SetText(buf);
          widthTextsOver[i]->SetText(buf);

          float maxAngle = 360 / (float)g_players[i].playfieldWidth;
          if (g_players[i].cubeAngle > maxAngle)
          {
            g_players[i].cubeAngle = maxAngle;
            sprintf(buf, "angle: %d", (int)g_players[i].cubeAngle);
            angleTexts[i]->SetText(buf);
            angleTextsOver[i]->SetText(buf);
          }
        }
      }
      else if (scaleButtons[i]->GetState() == STATE_SELECTED)
      {
        int incr = 0;

        if (A_PRESSED(i))
          incr = 1;
        else if (B_PRESSED(i))
          incr = -1;

        if (incr)
        {
          g_players[i].playfieldScale += incr;

          if (g_players[i].playfieldScale > MAX_PLAYFIELD_SCALE)
            g_players[i].playfieldScale = MAX_PLAYFIELD_SCALE;
          else if (g_players[i].playfieldScale < MIN_PLAYFIELD_SCALE)
            g_players[i].playfieldScale = MIN_PLAYFIELD_SCALE;

          sprintf(buf, "scale: %d", g_players[i].playfieldScale);
          scaleTexts[i]->SetText(buf);
          scaleTextsOver[i]->SetText(buf);
        }
      }
      else if (angleButtons[i]->GetState() == STATE_SELECTED)
      {
        int incr = 0;

        if (A_PRESSED(i))
          incr = 1;
        else if (B_PRESSED(i))
          incr = -1;

        if (incr)
        {
          g_players[i].cubeAngle += incr;

          float maxAngle = 360 / (float)g_players[i].playfieldWidth;
          if (g_players[i].cubeAngle > maxAngle)
          {
            g_players[i].cubeAngle = maxAngle;
          }
          else if (g_players[i].cubeAngle < MIN_CUBE_ANGLE)
          {
            g_players[i].cubeAngle = MIN_CUBE_ANGLE;
          }
          else
          {
            g_players[i].cubeAngle = (int)g_players[i].cubeAngle;
          }

          sprintf(buf, "angle: %d", (int)g_players[i].cubeAngle);
          angleTexts[i]->SetText(buf);
          angleTextsOver[i]->SetText(buf);
        }
      }
      else if (dxButtons[i]->GetState() == STATE_SELECTED)
      {
        int incr = 0;

        if (A_PRESSED(i))
          incr = 5;
        else if (B_PRESSED(i))
          incr = -5;

        if (incr)
        {
          g_players[i].playfieldDX += incr;

          if (g_players[i].playfieldDX > MAX_PLAYFIELD_DELTA)
            g_players[i].playfieldDX = MAX_PLAYFIELD_DELTA;
          else if (g_players[i].playfieldDX < -MAX_PLAYFIELD_DELTA)
            g_players[i].playfieldDX = -MAX_PLAYFIELD_DELTA;

          sprintf(buf, "dx: %d", g_players[i].playfieldDX);
          dxTexts[i]->SetText(buf);
          dxTextsOver[i]->SetText(buf);
        }
      }
      else if (dyButtons[i]->GetState() == STATE_SELECTED)
      {
        int incr = 0;

        if (A_PRESSED(i))
          incr = 5;
        else if (B_PRESSED(i))
          incr = -5;

        if (incr)
        {
          g_players[i].playfieldDY += incr;

          if (g_players[i].playfieldDY > MAX_PLAYFIELD_DELTA)
            g_players[i].playfieldDY = MAX_PLAYFIELD_DELTA;
          else if (g_players[i].playfieldDY < -MAX_PLAYFIELD_DELTA)
            g_players[i].playfieldDY = -MAX_PLAYFIELD_DELTA;

          sprintf(buf, "dy: %d", g_players[i].playfieldDY);
          dyTexts[i]->SetText(buf);
          dyTextsOver[i]->SetText(buf);
        }
      }
    }
  }

  mainWindow->Remove(&window);
}

/// Draws the "edit playfield" mode.
void TCYC_DrawEditMode()
{
  TCYC_SetUp2D();
  TCYC_DrawPlayfieldBoundary();
  
  TCYC_DrawTetriCycleEditMode();

  TCYC_SetUp2D();
  TCYC_DrawEditPlayfieldMenu();
}

/// Draws the 3D TetriCycle for every player in "edit playfield" mode.
void TCYC_DrawTetriCycleEditMode()
{
  // Prepare for drawing TetriCycle
  GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
  GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);

  int width;
  int height;
  int ci;
  int top;

  for (int i = 0; i < g_options->players; ++i)
  {
    Player &player = g_players[i];

    width = player.playfieldWidth;
    ci    = player.gameData.cycleIdx;
    top   = 0;

    for (int x = 0; x < width; ++x)
    {
      height = player.playfieldHeight;

      for (int y = height - 1; y >= top; --y)
      {
        int offset = (x - ci >= 0) ? x - ci : width - (ci - x);
        player.DrawBlockAsCube(offset, y, COLOR_ID_RED);
      }

      if (top < height - 1)
        ++top;
    }

    player.DrawBase();
  }
}

/// Draws the gui in "edit playfield" mode.
void TCYC_DrawEditPlayfieldMenu()
{
  mainWindow->Draw();

  for (int i = MAX_PLAYERS - 1; i >= 0; --i) // so that player 1's cursor appears on top!
  {
    if (userInput[i].wpad->ir.valid)
    {
	    Menu_DrawImg(userInput[i].wpad->ir.x - 48, userInput[i].wpad->ir.y - 48,
		               96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 
                   1, 1, 255);
    }

    DoRumble(i);
  }

  for (int i = 0; i < MAX_PLAYERS; ++i)
    mainWindow->Update(&userInput[i]);
}

/// Initializes the game state.
void TCYC_GameInit()
{
  MODPlay_Start(&g_modPlay);
  sgenrand(time(NULL));
  g_tcycMenu = TCYC_MENU_NONE;
  
  for (int i = 0; i < g_options->players; ++i)
    g_players[i].Reset();
}

/// Called when the game is reset/quit.
void TCYC_GameOnExit()
{
  MODPlay_Stop(&g_modPlay);

  // We have to delete any powerups from a previous game before 
  // memset'ing the PlayerGameData.
  PowerupUtils::DeleteAllPowerups();
}

/// Initialize the static description for every tetris piece.
void TCYC_InitPieceDescriptions()
{
	// Fill in data for every piece
	for (int i = 0; i < TETRISPIECE_ID_MAX; ++i)
	{
		// Every rotation
		for (int rot = 0; rot < 4; ++rot)
		{
			TetrisPieceDesc &desc = g_pieceDesc[i][rot];

			// Every square
			for (int y = 0; y < 4; ++y)
			{
				for (int x = 0; x < 4; ++x)
				{
					// Get square from data file
					desc.map[x][y] = pieces_bin[i * 4 * 4 * 4 + rot * 4 * 4 + y * 4 + x];
				}
			}
		}
	}
}