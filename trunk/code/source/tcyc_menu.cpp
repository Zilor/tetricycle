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

/** @file tcyc_menu.cpp
 * @author Cale Scholl / calvinss4
 */

#include "tcyc_menu.h"

#include <unistd.h>        // for usleep
#include <gcmodplay.h>     // for MODPlay
#include "Player.h"        // for Player
#include "Options.h"       // for Options
#include "PowerupUtils.h"  // for PowerupUtils
#include "libwiigui/gui.h" // for GuiSound
#include "menu.h"          // for ResumeGui
#include "main.h"          // for TCYC_SetUp2D, TCYC_DrawText, TetriCycle_main
#include "tcyc_input.h"    // for PausePressedAnyPlayer

extern GuiImageData *pointer[MAX_PLAYERS]; ///< pointing icon, represented by a hand
extern GuiImage *bgImg;       ///< the background image
extern GuiSound *bgMusic;     ///< the background music
extern GuiWindow *mainWindow; ///< the main window

extern Player *g_players;  ///< the player instances
extern MODPlay g_modPlay;  ///< used for playing the game music
extern bool g_isEditMode;  ///< true when editing the playfield
extern int g_tcycMenu;     ///< the current menu state
extern Options *g_options; ///< the global options

GuiTrigger *trigA;
GuiTrigger *trigHome;

GuiSound *btnSoundOver;
GuiSound *g_heartbeatSound;
GuiSound *g_tetrisCheerSound;

GuiImageData *btnOutline;
GuiImageData *btnOutlineOver;
GuiImageData *btnLargeOutline;
GuiImageData *btnLargeOutlineOver;
GuiImageData *btnData40x40Square;
GuiImageData *btnData40x40SquareOver;
GuiImageData *btnData80x40;
GuiImageData *btnData80x40Over;
GuiImageData *btnDataMinus;
GuiImageData *btnDataMinusOver;
GuiImageData *btnDataPlus;
GuiImageData *btnDataPlusOver;
GuiImageData *tetrisLove;
GuiImageData *grabber[MAX_PLAYERS];
GuiImageData *debug_grabber1;
GuiImageData *debug_grabber2;
GuiImageData *debug_grabber3;
GuiImageData *debug_grabber4;

// function prototypes
static void TCYC_MenuStartup();
static void TCYC_MenuLoadResources();
static int TCYC_MenuMainScreen();
static int TCYC_MenuGame();
static void TCYC_MenuPlayerProfilesPopup();
static void TCYC_MenuMpOptionsPopup(bool isGlobalOptions = true);
static void TCYC_MenuPowerupsPopup(GuiWindow *parentWindow, 
                                   vector<bool> &isPowerupEnabled,
                                   PowerupId powerupStartQueue[MAX_ACQUIRED_POWERUPS]);

static void _DoEasterEgg();

/// Pause the game.
void TCYC_MenuPause(const char *pauseTxt, GXColor *txtColor)
{
  g_options->isPaused = true;
  MODPlay_Pause(&g_modPlay, 1); // pause music

  GuiText txt(pauseTxt, 38, txtColor ? *txtColor : (GXColor){255, 255, 255, 255});
  txt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
  txt.SetPosition(0, 0);

  GuiWindow pauseWindow(screenwidth, screenheight);
  if (pauseTxt)
    pauseWindow.Append(&txt);

  // Quit Button
  GuiButton quitBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
  GuiText quitBtnTxt("Quit", 28, (GXColor){0, 0, 0, 255});
  GuiImage quitBtnImg(btnOutline);
  GuiImage quitBtnImgOver(btnOutlineOver);

  quitBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  quitBtn.SetPosition(-50 - btnOutline->GetWidth(), -50);
  quitBtn.SetLabel(&quitBtnTxt);
  quitBtn.SetImage(&quitBtnImg);
  quitBtn.SetImageOver(&quitBtnImgOver);
  quitBtn.SetSoundOver(btnSoundOver);
  quitBtn.SetTrigger(trigA);
  quitBtn.SetEffectGrow();
  pauseWindow.Append(&quitBtn);

  // Restart Button
  GuiButton restartBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
  GuiText restartBtnTxt("Restart", 28, (GXColor){0, 0, 0, 255});
  GuiImage restartBtnImg(btnOutline);
  GuiImage restartBtnImgOver(btnOutlineOver);

  restartBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  restartBtn.SetPosition(-50, -50);
  restartBtn.SetLabel(&restartBtnTxt);
  restartBtn.SetImage(&restartBtnImg);
  restartBtn.SetImageOver(&restartBtnImgOver);
  restartBtn.SetSoundOver(btnSoundOver);
  restartBtn.SetTrigger(trigA);
  restartBtn.SetEffectGrow();
  pauseWindow.Append(&restartBtn);

  // We can only pause from within the game, so the gui is already halted.
  mainWindow->Append(&pauseWindow);

  TCYC_SetUp2D();

  while (g_options->isPaused)
  {
    UpdatePads();
    TCYC_DrawText();
    mainWindow->Draw();

    for (int i = g_options->players - 1; i >= 0; --i) // so that player 1's cursor appears on top!
    {
      if (userInput[i].wpad->ir.valid)
      {
        Menu_DrawImg(userInput[i].wpad->ir.x - 48, userInput[i].wpad->ir.y - 48,
                     96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 
                     1, 1, 255);
      }

      DoRumble(i);
      mainWindow->Update(&userInput[i]);
    }

    Menu_Render();

    if (PausePressedAnyPlayer())
    {
      g_options->isPaused = false;
      MODPlay_Pause(&g_modPlay, 0); // unpause music
    }
    else if (quitBtn.GetState() == STATE_CLICKED)
    {
      g_options->isPaused = false;
      g_tcycMenu = TCYC_MENU_MAIN;
    }
    else if (restartBtn.GetState() == STATE_CLICKED)
    {
      g_options->isPaused = false;
      g_tcycMenu = TCYC_MENU_GAME;
    }
  }

  // Scan pads again or else pressing A will cause the piece to drop.
  UpdatePads();

  mainWindow->Remove(&pauseWindow);
}

/// Displays an error message and exits the program.
/**
 * Example:
  @code
  if (g_totalPowerups > MAX_POWERUPS)
  {
    char errorBuf[60] = "In defines_Powerup.h you must set MAX_POWERUPS to ";
    sprintf(errorBuf + 50, "%d.", g_totalPowerups);
    TCYC_MenuError(errorBuf);
  }
  @endcode
 */
void TCYC_MenuError(const char *errorTxt)
{
  WindowPrompt("ERROR", errorTxt, "exit");
  exit(1);
}

/// Load all the menu resources.
void TCYC_MenuLoadResources()
{
  trigA    = new GuiTrigger();
  trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
  trigHome = new GuiTrigger();
  trigHome->SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

  btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
  g_heartbeatSound = new GuiSound(heartbeat_pcm, heartbeat_pcm_size, SOUND_PCM);
  g_tetrisCheerSound = new GuiSound(tetris_cheer_pcm, tetris_cheer_pcm_size, SOUND_PCM);
  
  btnOutline             = new GuiImageData(button_png);
  btnOutlineOver         = new GuiImageData(button_over_png);
  btnLargeOutline        = new GuiImageData(button_large_png);
  btnLargeOutlineOver    = new GuiImageData(button_large_over_png);
  btnData40x40Square     = new GuiImageData(keyboard_key_png);
  btnData40x40SquareOver = new GuiImageData(keyboard_key_over_png);
  btnData80x40           = new GuiImageData(keyboard_mediumkey_png);
  btnData80x40Over       = new GuiImageData(keyboard_mediumkey_over_png);
  btnDataMinus           = new GuiImageData(scrollbar_arrowdown_png);
  btnDataMinusOver       = new GuiImageData(scrollbar_arrowdown_over_png);
  btnDataPlus            = new GuiImageData(scrollbar_arrowup_png);
  btnDataPlusOver        = new GuiImageData(scrollbar_arrowup_over_png);
  tetrisLove             = new GuiImageData(tetris_love_png);
  grabber[0]             = new GuiImageData(player1_grab_png);
  grabber[1]             = new GuiImageData(player2_grab_png);
  grabber[2]             = new GuiImageData(player3_grab_png);
  grabber[3]             = new GuiImageData(player4_grab_png);
  debug_grabber1         = new GuiImageData(player1_grab_png);
  debug_grabber2         = new GuiImageData(player2_grab_png);
  debug_grabber3         = new GuiImageData(player3_grab_png);
  debug_grabber4         = new GuiImageData(player4_grab_png);

  // Init the main screen background image and music.
  bgImg   = new GuiImage(tetrisLove);
  bgMusic = new GuiSound(tetricycle_by_dj_dimz_ogg, tetricycle_by_dj_dimz_ogg_size, SOUND_OGG);
  bgMusic->SetVolume(50);
  bgMusic->SetLoop(true);
}

/// Displays the startup screen.
void TCYC_MenuStartup()
{ 
  GuiText titleTxt("TetriCycle 1.0 by calvinss4", 30, (GXColor){255, 255, 255, 255});
  titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  titleTxt.SetPosition(0, 50);

  GuiText wwwTxt("www.wiibrew.org/wiki/TetriCycle", 28, (GXColor){255, 255, 255, 255});
  wwwTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  wwwTxt.SetPosition(0, 90);

  GuiText sourceTxt1("This game is open source. See:", 28, (GXColor){255, 255, 255, 255});
  sourceTxt1.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  sourceTxt1.SetPosition(0, 140);

  GuiText sourceTxt2("code.google.com/p/tetricycle", 28, (GXColor){255, 255, 255, 255});
  sourceTxt2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  sourceTxt2.SetPosition(0, 175);

  GuiText emailTxt1("Bugs, questions or comments:", 28, (GXColor){255, 255, 255, 255});
  emailTxt1.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  emailTxt1.SetPosition(0, 225);

  GuiText emailTxt2("code.google.com/p/tetricycle/issues", 28, (GXColor){255, 255, 255, 255});
  emailTxt2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  emailTxt2.SetPosition(0, 260);

  GuiText scamTxt("If you paid for this you were scammed!", 26, (GXColor){255, 255, 255, 255});
  scamTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  scamTxt.SetPosition(0, 310);

  GuiText initTxt("PRESS ANY BUTTON", 30, (GXColor){255, 255, 255, 255});
  initTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  initTxt.SetPosition(0, 360);

  GuiWindow w(screenwidth, screenheight);
  w.Append(&titleTxt);
  w.Append(&wwwTxt);
  w.Append(&sourceTxt1);
  w.Append(&sourceTxt2);
  w.Append(&emailTxt1);
  w.Append(&emailTxt2);
  w.Append(&scamTxt);
  w.Append(&initTxt);
  
  HaltGui();
  mainWindow->Append(&w);
  ResumeGui();

  // Load resources while the startup screen is displaying.
  TCYC_MenuLoadResources();

  bool loop = true;
  while (loop)
  {
    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
      if (WPAD_ButtonsDown(i))
      {
        loop = false;
        break;
      }
    }
  }

  HaltGui();
  mainWindow->Remove(&w);
}

/// The game state loop.
void TCYC_MenuLoop()
{
  // Display the startup screen.
  TCYC_MenuStartup();

  g_tcycMenu = TCYC_MENU_MAIN;

  while (g_tcycMenu != TCYC_MENU_EXIT)
  {
    switch (g_tcycMenu)
    {
      case TCYC_MENU_MAIN:
        g_tcycMenu = TCYC_MenuMainScreen(); // halts the gui before exiting
        break;
      case TCYC_MENU_GAME:
        g_tcycMenu = TCYC_MenuGame();
        break;
      default: // unrecognized menu
        g_tcycMenu = TCYC_MENU_EXIT;
        break;
    }
  }
}

/// Displays the main menu screen.
int TCYC_MenuMainScreen()
{
  ResetVideo_Menu();
  if (!g_isEditMode)
    bgMusic->Play();
  g_isEditMode = false;

  // title text
  GuiText titleTxt("TetriCycle", 28, (GXColor){255, 255, 255, 255});
  titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  titleTxt.SetPosition(0, 50);

  // 1P button
  GuiButton p1Btn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText p1BtnTxt("1P", 22, (GXColor){0, 0, 0, 255});
  GuiImage p1BtnImg(btnData40x40Square);
  GuiImage p1BtnImgOver(btnData40x40SquareOver);

  p1Btn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  p1Btn.SetPosition(50, 100);
  p1Btn.SetLabel(&p1BtnTxt);
  p1Btn.SetImage(&p1BtnImg);
  p1Btn.SetImageOver(&p1BtnImgOver);
  p1Btn.SetSoundOver(btnSoundOver);
  p1Btn.SetTrigger(trigA);
  p1Btn.SetEffectGrow();

  // 2P button
  GuiButton p2Btn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText p2BtnTxt("2P", 22, (GXColor){0, 0, 0, 255});
  GuiImage p2BtnImg(btnData40x40Square);
  GuiImage p2BtnImgOver(btnData40x40SquareOver);

  p2Btn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  p2Btn.SetPosition(50 + btnData40x40Square->GetWidth(), 100);
  p2Btn.SetLabel(&p2BtnTxt);
  p2Btn.SetImage(&p2BtnImg);
  p2Btn.SetImageOver(&p2BtnImgOver);
  p2Btn.SetSoundOver(btnSoundOver);
  p2Btn.SetTrigger(trigA);
  p2Btn.SetEffectGrow();

  // 3P button
  GuiButton p3Btn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText p3BtnTxt("3P", 22, (GXColor){0, 0, 0, 255});
  GuiImage p3BtnImg(btnData40x40Square);
  GuiImage p3BtnImgOver(btnData40x40SquareOver);

  p3Btn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  p3Btn.SetPosition(50 + 2 * btnData40x40Square->GetWidth(), 100);
  p3Btn.SetLabel(&p3BtnTxt);
  p3Btn.SetImage(&p3BtnImg);
  p3Btn.SetImageOver(&p3BtnImgOver);
  p3Btn.SetSoundOver(btnSoundOver);
  p3Btn.SetTrigger(trigA);
  p3Btn.SetEffectGrow();

  // 4P button
  GuiButton p4Btn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText p4BtnTxt("4P", 22, (GXColor){0, 0, 0, 255});
  GuiImage p4BtnImg(btnData40x40Square);
  GuiImage p4BtnImgOver(btnData40x40SquareOver);

  p4Btn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  p4Btn.SetPosition(50 + 3 * btnData40x40Square->GetWidth(), 100);
  p4Btn.SetLabel(&p4BtnTxt);
  p4Btn.SetImage(&p4BtnImg);
  p4Btn.SetImageOver(&p4BtnImgOver);
  p4Btn.SetSoundOver(btnSoundOver);
  p4Btn.SetTrigger(trigA);
  p4Btn.SetEffectGrow();

  // player profiles button
  GuiButton profileBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
  GuiText profileBtnTxt("Player Profiles", 22, (GXColor){0, 0, 0, 255});
  GuiImage profileBtnImg(btnOutline);
  GuiImage profileBtnImgOver(btnOutlineOver);

  profileBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  profileBtn.SetPosition(50 + 4 * btnData40x40Square->GetWidth() + 10, 100 - 6);
  profileBtn.SetLabel(&profileBtnTxt);
  profileBtn.SetImage(&profileBtnImg);
  profileBtn.SetImageOver(&profileBtnImgOver);
  profileBtn.SetSoundOver(btnSoundOver);
  profileBtn.SetTrigger(trigA);
  profileBtn.SetEffectGrow();

  // multiplayer options button
  GuiButton mpOptionsBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
  GuiText mpOptionsBtnTxt("MP Options", 22, (GXColor){0, 0, 0, 255});
  GuiImage mpOptionsBtnImg(btnOutline);
  GuiImage mpOptionsBtnImgOver(btnOutlineOver);

  mpOptionsBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  mpOptionsBtn.SetPosition(50 + 4 * btnData40x40Square->GetWidth() + 10 + btnOutline->GetWidth(), 100 - 6);
  mpOptionsBtn.SetLabel(&mpOptionsBtnTxt);
  mpOptionsBtn.SetImage(&mpOptionsBtnImg);
  mpOptionsBtn.SetImageOver(&mpOptionsBtnImgOver);
  mpOptionsBtn.SetSoundOver(btnSoundOver);
  mpOptionsBtn.SetTrigger(trigA);
  mpOptionsBtn.SetEffectGrow();

  // handicap options button
  GuiButton mpHandicapsBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
  GuiText mpHandicapsBtnTxt("MP Handicaps", 22, (GXColor){0, 0, 0, 255});
  GuiImage mpHandicapsBtnImg(btnOutline);
  GuiImage mpHandicapsBtnImgOver(btnOutlineOver);

  mpHandicapsBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  mpHandicapsBtn.SetPosition(50 + 4 * btnData40x40Square->GetWidth() + 10 + btnOutline->GetWidth(), 100 - 6 + btnOutline->GetHeight());
  mpHandicapsBtn.SetLabel(&mpHandicapsBtnTxt);
  mpHandicapsBtn.SetImage(&mpHandicapsBtnImg);
  mpHandicapsBtn.SetImageOver(&mpHandicapsBtnImgOver);
  mpHandicapsBtn.SetSoundOver(btnSoundOver);
  mpHandicapsBtn.SetTrigger(trigA);
  mpHandicapsBtn.SetEffectGrow();

  // return to loader button
  GuiButton returnBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
  GuiText returnBtnTxt("Return to loader", 20, (GXColor){0, 0, 0, 255});
  GuiImage returnBtnImg(btnOutline);
  GuiImage returnBtnImgOver(btnOutlineOver);

  returnBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  returnBtn.SetPosition(-50 - btnOutline->GetWidth(), -50);
  returnBtn.SetLabel(&returnBtnTxt);
  returnBtn.SetImage(&returnBtnImg);
  returnBtn.SetImageOver(&returnBtnImgOver);
  returnBtn.SetSoundOver(btnSoundOver);
  returnBtn.SetTrigger(trigA);
  returnBtn.SetTrigger(trigHome);
  returnBtn.SetEffectGrow();

  // exit to wii menu button
  GuiButton exitBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
  GuiText exitBtnTxt("Exit to Wii menu", 20, (GXColor){0, 0, 0, 255});
  GuiImage exitBtnImg(btnOutline);
  GuiImage exitBtnImgOver(btnOutlineOver);

  exitBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  exitBtn.SetPosition(-50, -50);
  exitBtn.SetLabel(&exitBtnTxt);
  exitBtn.SetImage(&exitBtnImg);
  exitBtn.SetImageOver(&exitBtnImgOver);
  exitBtn.SetSoundOver(btnSoundOver);
  exitBtn.SetTrigger(trigA);
  exitBtn.SetTrigger(trigHome);
  exitBtn.SetEffectGrow();

  // easter egg button
  GuiButton heartBtn(150, 150);
  heartBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
  heartBtn.SetPosition(0, 25);
  heartBtn.SetSoundOver(btnSoundOver);
  heartBtn.SetTrigger(trigA);

  //--- Adjust Playfield ---
  GuiText playfieldTxt("Edit Playfield:", 24, (GXColor){255, 255, 255, 255});
  playfieldTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  playfieldTxt.SetPosition(50, 260);

  GuiButton edit1PBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText edit1PBtnTxt("1P", 22, (GXColor){0, 0, 0, 255});
  GuiImage edit1PBtnImg(btnData40x40Square);
  GuiImage edit1PBtnImgOver(btnData40x40SquareOver);

  edit1PBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  edit1PBtn.SetPosition(50, 290);
  edit1PBtn.SetLabel(&edit1PBtnTxt);
  edit1PBtn.SetImage(&edit1PBtnImg);
  edit1PBtn.SetImageOver(&edit1PBtnImgOver);
  edit1PBtn.SetSoundOver(btnSoundOver);
  edit1PBtn.SetTrigger(trigA);
  edit1PBtn.SetEffectGrow();

  GuiButton edit2PBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText edit2PBtnTxt("2P", 22, (GXColor){0, 0, 0, 255});
  GuiImage edit2PBtnImg(btnData40x40Square);
  GuiImage edit2PBtnImgOver(btnData40x40SquareOver);

  edit2PBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  edit2PBtn.SetPosition(50 + btnData40x40Square->GetWidth(), 290);
  edit2PBtn.SetLabel(&edit2PBtnTxt);
  edit2PBtn.SetImage(&edit2PBtnImg);
  edit2PBtn.SetImageOver(&edit2PBtnImgOver);
  edit2PBtn.SetSoundOver(btnSoundOver);
  edit2PBtn.SetTrigger(trigA);
  edit2PBtn.SetEffectGrow();

  GuiButton edit3PBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText edit3PBtnTxt("3P", 22, (GXColor){0, 0, 0, 255});
  GuiImage edit3PBtnImg(btnData40x40Square);
  GuiImage edit3PBtnImgOver(btnData40x40SquareOver);

  edit3PBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  edit3PBtn.SetPosition(50 + 2 * btnData40x40Square->GetWidth(), 290);
  edit3PBtn.SetLabel(&edit3PBtnTxt);
  edit3PBtn.SetImage(&edit3PBtnImg);
  edit3PBtn.SetImageOver(&edit3PBtnImgOver);
  edit3PBtn.SetSoundOver(btnSoundOver);
  edit3PBtn.SetTrigger(trigA);
  edit3PBtn.SetEffectGrow();

  GuiButton edit4PBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText edit4PBtnTxt("4P", 22, (GXColor){0, 0, 0, 255});
  GuiImage edit4PBtnImg(btnData40x40Square);
  GuiImage edit4PBtnImgOver(btnData40x40SquareOver);

  edit4PBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  edit4PBtn.SetPosition(50 + 3 * btnData40x40Square->GetWidth(), 290);
  edit4PBtn.SetLabel(&edit4PBtnTxt);
  edit4PBtn.SetImage(&edit4PBtnImg);
  edit4PBtn.SetImageOver(&edit4PBtnImgOver);
  edit4PBtn.SetSoundOver(btnSoundOver);
  edit4PBtn.SetTrigger(trigA);
  edit4PBtn.SetEffectGrow();

  //--- Populate the window ---
  GuiWindow w(screenwidth, screenheight);
  w.Append(bgImg);
  w.Append(&titleTxt);
  w.Append(&p1Btn);
  w.Append(&p2Btn);
  w.Append(&p3Btn);
  w.Append(&p4Btn);
  w.Append(&profileBtn);
  w.Append(&mpOptionsBtn);
  w.Append(&mpHandicapsBtn);
  w.Append(&returnBtn);
  w.Append(&exitBtn);
  w.Append(&heartBtn);
  w.Append(&playfieldTxt);
  w.Append(&edit1PBtn);
  w.Append(&edit2PBtn);
  w.Append(&edit3PBtn);
  w.Append(&edit4PBtn);
  //w.Append(&networkBtn);

  HaltGui();
  mainWindow->Append(&w);
  ResumeGui();

  int menu = TCYC_MENU_NONE;

  while (menu == TCYC_MENU_NONE)
  {
    usleep(THREAD_SLEEP);
    if (p1Btn.GetState() == STATE_CLICKED)
    {
      g_options->players = 1;
      menu = TCYC_MENU_GAME;
    }
    else if (p2Btn.GetState() == STATE_CLICKED)
    {
      g_options->players = 2;
      menu = TCYC_MENU_GAME;
    }
    else if (p3Btn.GetState() == STATE_CLICKED)
    {
      g_options->players = 3;
      menu = TCYC_MENU_GAME;
    }
    else if (p4Btn.GetState() == STATE_CLICKED)
    {
      g_options->players = 4;
      menu = TCYC_MENU_GAME;
    }
    else if (profileBtn.GetState() == STATE_CLICKED)
    {
      profileBtn.ResetState();
      TCYC_MenuPlayerProfilesPopup();
    }
    else if (mpOptionsBtn.GetState() == STATE_CLICKED)
    {
      mpOptionsBtn.ResetState();
      TCYC_MenuMpOptionsPopup();
    }
    else if (mpHandicapsBtn.GetState() == STATE_CLICKED)
    {
      mpHandicapsBtn.ResetState();
      TCYC_MenuMpOptionsPopup(false/*isGlobalOptions*/);
    }
    else if (returnBtn.GetState() == STATE_CLICKED)
    {
      exit(0);
    }
    else if (exitBtn.GetState() == STATE_CLICKED)
    {
      SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
    }
    else if (heartBtn.GetState() == STATE_CLICKED)
    {
      heartBtn.ResetState();
      _DoEasterEgg();
    }
    else if (edit1PBtn.GetState() == STATE_CLICKED)
    {
      g_options->players = 1;
      menu = TCYC_MENU_GAME;
      g_isEditMode = true;
    }
    else if (edit2PBtn.GetState() == STATE_CLICKED)
    {
      g_options->players = 2;
      menu = TCYC_MENU_GAME;
      g_isEditMode = true;
    }
    else if (edit3PBtn.GetState() == STATE_CLICKED)
    {
      g_options->players = 3;
      menu = TCYC_MENU_GAME;
      g_isEditMode = true;
    }
    else if (edit4PBtn.GetState() == STATE_CLICKED)
    {
      g_options->players = 4;
      menu = TCYC_MENU_GAME;
      g_isEditMode = true;
    }
  }

  if (!g_isEditMode)
    bgMusic->Stop();

  HaltGui();
  mainWindow->Remove(&w);
  return menu;
}

/// Runs the game loop.
int TCYC_MenuGame()
{
  // Doing this in a loop allows us to reset the game.
  while (g_tcycMenu == TCYC_MENU_GAME)
  {
    if (g_isEditMode)
      TCYC_EditPlayfield();
    else
      TCYC_Game();
  }

  return TCYC_MENU_MAIN;
}

/// Launches the player profiles popup.
void TCYC_MenuPlayerProfilesPopup()
{
  static const int PADDING_TOP = 12;
  GXColor helpTxtColor = (GXColor){0, 170, 0, 255};
  GXColor blackColor = (GXColor){0, 0, 0, 255};

  int player = 0;
  char playerBuf[7+1+1] = "PLAYER "; // values: 1-MAX_PLAYERS + "PLAYER "
  sprintf(playerBuf + 7, "%d", player + 1);

  bool isNormalRotation[MAX_PLAYERS];
  bool isShakeEnabled[MAX_PLAYERS];
  bool isShadowEnabled[MAX_PLAYERS];
  bool isPreviewEnabled[MAX_PLAYERS];

  for (int i = 0; i < MAX_PLAYERS; ++i)
  {
    isNormalRotation[i] = g_players[i].isNormalRotation;
    isShakeEnabled[i]   = g_players[i].isShakeEnabled;
    isShadowEnabled[i]  = g_players[i].isShadowEnabled;
    isPreviewEnabled[i] = g_players[i].isPreviewEnabled;
  }

  // popup window
  GuiWindow promptWindow(screenwidth - 100, screenheight - 100);
  promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
  promptWindow.SetPosition(0, 0);

  // background image
  GuiImage profilesBgImg(screenwidth - 100, screenheight - 100, (GXColor){170, 170, 170, 255});

  // title text
  GuiText titleTxt("Player Profiles", 30, blackColor);
  titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  titleTxt.SetPosition(0, 15);

  // PLAYER LEFT ARROW BUTTON
  GuiButton playerLeftArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText playerLeftArrowBtnTxt("<", 22, blackColor);
  GuiText playerLeftArrowBtnTxtOver("<", 22, helpTxtColor);
  GuiImage playerLeftArrowBtnImg(btnData40x40Square);
  GuiImage playerLeftArrowBtnImgOver(btnData40x40SquareOver);

  playerLeftArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  playerLeftArrowBtn.SetPosition(-80, 60);
  playerLeftArrowBtn.SetLabel(&playerLeftArrowBtnTxt);
  playerLeftArrowBtn.SetLabelOver(&playerLeftArrowBtnTxtOver);
  playerLeftArrowBtn.SetImage(&playerLeftArrowBtnImg);
  playerLeftArrowBtn.SetImageOver(&playerLeftArrowBtnImgOver);
  playerLeftArrowBtn.SetSoundOver(btnSoundOver);
  playerLeftArrowBtn.SetTrigger(trigA);
  playerLeftArrowBtn.SetEffectGrow();

  // PLAYER RIGHT ARROW BUTTON
  GuiButton playerRightArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText playerRightArrowBtnTxt(">", 22, blackColor);
  GuiText playerRightArrowBtnTxtOver(">", 22, helpTxtColor);
  GuiImage playerRightArrowBtnImg(btnData40x40Square);
  GuiImage playerRightArrowBtnImgOver(btnData40x40SquareOver);

  playerRightArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  playerRightArrowBtn.SetPosition(80, 60);
  playerRightArrowBtn.SetLabel(&playerRightArrowBtnTxt);
  playerRightArrowBtn.SetLabelOver(&playerRightArrowBtnTxtOver);
  playerRightArrowBtn.SetImage(&playerRightArrowBtnImg);
  playerRightArrowBtn.SetImageOver(&playerRightArrowBtnImgOver);
  playerRightArrowBtn.SetSoundOver(btnSoundOver);
  playerRightArrowBtn.SetTrigger(trigA);
  playerRightArrowBtn.SetEffectGrow();

  // PLAYER TEXT
  GuiText playerTxt(playerBuf, 22, blackColor);
  playerTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  playerTxt.SetPosition(0, 60 + PADDING_TOP);

  // ROTATION TEXT BUTTON
  GuiButton rotationTxtBtn((9 + 1) * 11, 22);
  GuiText rotationTxtBtnTxt("rotation:", 22, blackColor);
  GuiText rotationTxtBtnTxtOver("rotation:", 22, helpTxtColor);

  rotationTxtBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  rotationTxtBtn.SetPosition(20, 60 + PADDING_TOP + 1 * 40);
  rotationTxtBtn.SetLabel(&rotationTxtBtnTxt);
  rotationTxtBtn.SetLabelOver(&rotationTxtBtnTxtOver);
  rotationTxtBtn.SetSoundOver(btnSoundOver);
  rotationTxtBtn.SetTrigger(trigA);

  // ROTATION LEFT ARROW BUTTON
  GuiButton rotationLeftArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText rotationLeftArrowBtnTxt("<", 22, blackColor);
  GuiText rotationLeftArrowBtnTxtOver("<", 22, helpTxtColor);
  GuiImage rotationLeftArrowBtnImg(btnData40x40Square);
  GuiImage rotationLeftArrowBtnImgOver(btnData40x40SquareOver);

  rotationLeftArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  rotationLeftArrowBtn.SetPosition(-80, 60 + 1 * 40);
  rotationLeftArrowBtn.SetLabel(&rotationLeftArrowBtnTxt);
  rotationLeftArrowBtn.SetLabelOver(&rotationLeftArrowBtnTxtOver);
  rotationLeftArrowBtn.SetImage(&rotationLeftArrowBtnImg);
  rotationLeftArrowBtn.SetImageOver(&rotationLeftArrowBtnImgOver);
  rotationLeftArrowBtn.SetSoundOver(btnSoundOver);
  rotationLeftArrowBtn.SetTrigger(trigA);
  rotationLeftArrowBtn.SetEffectGrow();

  // ROTATION RIGHT ARROW BUTTON
  GuiButton rotationRightArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText rotationRightArrowBtnTxt(">", 22, blackColor);
  GuiText rotationRightArrowBtnTxtOver(">", 22, helpTxtColor);
  GuiImage rotationRightArrowBtnImg(btnData40x40Square);
  GuiImage rotationRightArrowBtnImgOver(btnData40x40SquareOver);

  rotationRightArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  rotationRightArrowBtn.SetPosition(80, 60 + 1 * 40);
  rotationRightArrowBtn.SetLabel(&rotationRightArrowBtnTxt);
  rotationRightArrowBtn.SetLabelOver(&rotationRightArrowBtnTxtOver);
  rotationRightArrowBtn.SetImage(&rotationRightArrowBtnImg);
  rotationRightArrowBtn.SetImageOver(&rotationRightArrowBtnImgOver);
  rotationRightArrowBtn.SetSoundOver(btnSoundOver);
  rotationRightArrowBtn.SetTrigger(trigA);
  rotationRightArrowBtn.SetEffectGrow();

  // ROTATION TEXT
  GuiText rotationTxt(isNormalRotation[player] ? "normal" : "reverse", 22, blackColor);
  rotationTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  rotationTxt.SetPosition(0, 60 + PADDING_TOP + 1 * 40);

  //---
  // SHADOW TEXT BUTTON
  GuiButton shadowTxtBtn((9 + 1) * 11, 22);
  GuiText shadowTxtBtnTxt("shadow:", 22, blackColor);
  GuiText shadowTxtBtnTxtOver("shadow:", 22, helpTxtColor);

  shadowTxtBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  shadowTxtBtn.SetPosition(20, 60 + PADDING_TOP + 2 * 40);
  shadowTxtBtn.SetLabel(&shadowTxtBtnTxt);
  shadowTxtBtn.SetLabelOver(&shadowTxtBtnTxtOver);
  shadowTxtBtn.SetSoundOver(btnSoundOver);
  shadowTxtBtn.SetTrigger(trigA);

  // SHADOW LEFT ARROW BUTTON
  GuiButton shadowLeftArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText shadowLeftArrowBtnTxt("<", 22, blackColor);
  GuiText shadowLeftArrowBtnTxtOver("<", 22, helpTxtColor);
  GuiImage shadowLeftArrowBtnImg(btnData40x40Square);
  GuiImage shadowLeftArrowBtnImgOver(btnData40x40SquareOver);

  shadowLeftArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  shadowLeftArrowBtn.SetPosition(-80, 60 + 2 * 40);
  shadowLeftArrowBtn.SetLabel(&shadowLeftArrowBtnTxt);
  shadowLeftArrowBtn.SetLabelOver(&shadowLeftArrowBtnTxtOver);
  shadowLeftArrowBtn.SetImage(&shadowLeftArrowBtnImg);
  shadowLeftArrowBtn.SetImageOver(&shadowLeftArrowBtnImgOver);
  shadowLeftArrowBtn.SetSoundOver(btnSoundOver);
  shadowLeftArrowBtn.SetTrigger(trigA);
  shadowLeftArrowBtn.SetEffectGrow();

  // SHADOW RIGHT ARROW BUTTON
  GuiButton shadowRightArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText shadowRightArrowBtnTxt(">", 22, blackColor);
  GuiText shadowRightArrowBtnTxtOver(">", 22, helpTxtColor);
  GuiImage shadowRightArrowBtnImg(btnData40x40Square);
  GuiImage shadowRightArrowBtnImgOver(btnData40x40SquareOver);

  shadowRightArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  shadowRightArrowBtn.SetPosition(80, 60 + 2 * 40);
  shadowRightArrowBtn.SetLabel(&shadowRightArrowBtnTxt);
  shadowRightArrowBtn.SetLabelOver(&shadowRightArrowBtnTxtOver);
  shadowRightArrowBtn.SetImage(&shadowRightArrowBtnImg);
  shadowRightArrowBtn.SetImageOver(&shadowRightArrowBtnImgOver);
  shadowRightArrowBtn.SetSoundOver(btnSoundOver);
  shadowRightArrowBtn.SetTrigger(trigA);
  shadowRightArrowBtn.SetEffectGrow();

  // SHADOW TEXT
  GuiText shadowTxt(isShadowEnabled[player] ? "on" : "off", 22, blackColor);
  shadowTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  shadowTxt.SetPosition(0, 60 + PADDING_TOP + 2 * 40);

  //---
  // PREVIEW TEXT BUTTON
  GuiButton previewTxtBtn((9 + 1) * 11, 22);
  GuiText previewTxtBtnTxt("preview:", 22, blackColor);
  GuiText previewTxtBtnTxtOver("preview:", 22, helpTxtColor);

  previewTxtBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  previewTxtBtn.SetPosition(20, 60 + PADDING_TOP + 3 * 40);
  previewTxtBtn.SetLabel(&previewTxtBtnTxt);
  previewTxtBtn.SetLabelOver(&previewTxtBtnTxtOver);
  previewTxtBtn.SetSoundOver(btnSoundOver);
  previewTxtBtn.SetTrigger(trigA);

  // PREVIEW LEFT ARROW BUTTON
  GuiButton previewLeftArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText previewLeftArrowBtnTxt("<", 22, blackColor);
  GuiText previewLeftArrowBtnTxtOver("<", 22, helpTxtColor);
  GuiImage previewLeftArrowBtnImg(btnData40x40Square);
  GuiImage previewLeftArrowBtnImgOver(btnData40x40SquareOver);

  previewLeftArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  previewLeftArrowBtn.SetPosition(-80, 60 + 3 * 40);
  previewLeftArrowBtn.SetLabel(&previewLeftArrowBtnTxt);
  previewLeftArrowBtn.SetLabelOver(&previewLeftArrowBtnTxtOver);
  previewLeftArrowBtn.SetImage(&previewLeftArrowBtnImg);
  previewLeftArrowBtn.SetImageOver(&previewLeftArrowBtnImgOver);
  previewLeftArrowBtn.SetSoundOver(btnSoundOver);
  previewLeftArrowBtn.SetTrigger(trigA);
  previewLeftArrowBtn.SetEffectGrow();

  // PREVIEW RIGHT ARROW BUTTON
  GuiButton previewRightArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText previewRightArrowBtnTxt(">", 22, blackColor);
  GuiText previewRightArrowBtnTxtOver(">", 22, helpTxtColor);
  GuiImage previewRightArrowBtnImg(btnData40x40Square);
  GuiImage previewRightArrowBtnImgOver(btnData40x40SquareOver);

  previewRightArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  previewRightArrowBtn.SetPosition(80, 60 + 3 * 40);
  previewRightArrowBtn.SetLabel(&previewRightArrowBtnTxt);
  previewRightArrowBtn.SetLabelOver(&previewRightArrowBtnTxtOver);
  previewRightArrowBtn.SetImage(&previewRightArrowBtnImg);
  previewRightArrowBtn.SetImageOver(&previewRightArrowBtnImgOver);
  previewRightArrowBtn.SetSoundOver(btnSoundOver);
  previewRightArrowBtn.SetTrigger(trigA);
  previewRightArrowBtn.SetEffectGrow();

  // PREVIEW TEXT
  GuiText previewTxt(isPreviewEnabled[player] ? "on" : "off", 22, blackColor);
  previewTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  previewTxt.SetPosition(0, 60 + PADDING_TOP + 3 * 40);

  //---
  // SHAKE TEXT BUTTON
  GuiButton shakeTxtBtn((7 + 1) * 11, 22);
  GuiText shakeTxtBtnTxt("shake:", 22, blackColor);
  GuiText shakeTxtBtnTxtOver("shake:", 22, helpTxtColor);

  shakeTxtBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  shakeTxtBtn.SetPosition(20, 60 + PADDING_TOP + 4 * 40);
  shakeTxtBtn.SetLabel(&shakeTxtBtnTxt);
  shakeTxtBtn.SetLabelOver(&shakeTxtBtnTxtOver);
  shakeTxtBtn.SetSoundOver(btnSoundOver);
  shakeTxtBtn.SetTrigger(trigA);

  // SHAKE LEFT ARROW BUTTON
  GuiButton shakeLeftArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText shakeLeftArrowBtnTxt("<", 22, blackColor);
  GuiText shakeLeftArrowBtnTxtOver("<", 22, helpTxtColor);
  GuiImage shakeLeftArrowBtnImg(btnData40x40Square);
  GuiImage shakeLeftArrowBtnImgOver(btnData40x40SquareOver);

  shakeLeftArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  shakeLeftArrowBtn.SetPosition(-80, 60 + 4 * 40);
  shakeLeftArrowBtn.SetLabel(&shakeLeftArrowBtnTxt);
  shakeLeftArrowBtn.SetLabelOver(&shakeLeftArrowBtnTxtOver);
  shakeLeftArrowBtn.SetImage(&shakeLeftArrowBtnImg);
  shakeLeftArrowBtn.SetImageOver(&shakeLeftArrowBtnImgOver);
  shakeLeftArrowBtn.SetSoundOver(btnSoundOver);
  shakeLeftArrowBtn.SetTrigger(trigA);
  shakeLeftArrowBtn.SetEffectGrow();

  // SHAKE RIGHT ARROW BUTTON
  GuiButton shakeRightArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText shakeRightArrowBtnTxt(">", 22, blackColor);
  GuiText shakeRightArrowBtnTxtOver(">", 22, helpTxtColor);
  GuiImage shakeRightArrowBtnImg(btnData40x40Square);
  GuiImage shakeRightArrowBtnImgOver(btnData40x40SquareOver);

  shakeRightArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  shakeRightArrowBtn.SetPosition(80, 60 + 4 * 40);
  shakeRightArrowBtn.SetLabel(&shakeRightArrowBtnTxt);
  shakeRightArrowBtn.SetLabelOver(&shakeRightArrowBtnTxtOver);
  shakeRightArrowBtn.SetImage(&shakeRightArrowBtnImg);
  shakeRightArrowBtn.SetImageOver(&shakeRightArrowBtnImgOver);
  shakeRightArrowBtn.SetSoundOver(btnSoundOver);
  shakeRightArrowBtn.SetTrigger(trigA);
  shakeRightArrowBtn.SetEffectGrow();

  // SHAKE TEXT
  GuiText shakeTxt(isShakeEnabled[player] ? "on" : "off", 22, blackColor);
  shakeTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  shakeTxt.SetPosition(0, 60 + PADDING_TOP + 4 * 40);

  //---
  // CONFIRM BUTTON
  GuiButton confirmBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText confirmBtnTxt("OK", 22, (GXColor){0, 0, 255, 255});
  GuiImage confirmBtnImg(btnData40x40Square);
  GuiImage confirmBtnImgOver(btnData40x40SquareOver);

  confirmBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  confirmBtn.SetPosition(-10, -10);
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

  cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  cancelBtn.SetPosition(-10, 10);
  cancelBtn.SetLabel(&cancelBtnTxt);
  cancelBtn.SetImage(&cancelBtnImg);
  cancelBtn.SetImageOver(&cancelBtnImgOver);
  cancelBtn.SetSoundOver(btnSoundOver);
  cancelBtn.SetTrigger(trigA);
  cancelBtn.SetEffectGrow();

  // HELP BUTTON
  GuiButton helpBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText helpBtnTxt("?", 22, helpTxtColor);
  GuiImage helpBtnImg(btnData40x40Square);
  GuiImage helpBtnImgOver(btnData40x40SquareOver);

  helpBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
  helpBtn.SetPosition(10, -10);
  helpBtn.SetLabel(&helpBtnTxt);
  helpBtn.SetImage(&helpBtnImg);
  helpBtn.SetImageOver(&helpBtnImgOver);
  helpBtn.SetSoundOver(btnSoundOver);
  helpBtn.SetTrigger(trigA);
  helpBtn.SetEffectGrow();

  // Populate the window.
  promptWindow.Append(&profilesBgImg);
  promptWindow.Append(&titleTxt);
  promptWindow.Append(&playerLeftArrowBtn);
  promptWindow.Append(&playerRightArrowBtn);
  promptWindow.Append(&playerTxt);

  promptWindow.Append(&rotationTxtBtn);
  promptWindow.Append(&rotationLeftArrowBtn);
  promptWindow.Append(&rotationRightArrowBtn);
  promptWindow.Append(&rotationTxt);

  promptWindow.Append(&shakeTxtBtn);
  promptWindow.Append(&shakeLeftArrowBtn);
  promptWindow.Append(&shakeRightArrowBtn);
  promptWindow.Append(&shakeTxt);

  promptWindow.Append(&shadowTxtBtn);
  promptWindow.Append(&shadowLeftArrowBtn);
  promptWindow.Append(&shadowRightArrowBtn);
  promptWindow.Append(&shadowTxt);

  promptWindow.Append(&previewTxtBtn);
  promptWindow.Append(&previewLeftArrowBtn);
  promptWindow.Append(&previewRightArrowBtn);
  promptWindow.Append(&previewTxt);

  promptWindow.Append(&confirmBtn);
  promptWindow.Append(&cancelBtn);
  promptWindow.Append(&helpBtn);

  promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
  HaltGui();
  // This sets the state for all child elements, including child windows; 
  // that is why we disable the state first, then add the child window.
  mainWindow->SetState(STATE_DISABLED);
  mainWindow->Append(&promptWindow);
  mainWindow->ChangeFocus(&promptWindow);
  ResumeGui();

  while (true)
  {
    usleep(THREAD_SLEEP);

    if (confirmBtn.GetState() == STATE_CLICKED)
    {
      for (int i = 0; i < MAX_PLAYERS; ++i)
      {
        g_players[i].isNormalRotation = isNormalRotation[i];
        g_players[i].isShakeEnabled   = isShakeEnabled[i];
        g_players[i].isShadowEnabled  = isShadowEnabled[i];
        g_players[i].isPreviewEnabled = isPreviewEnabled[i];
      }

      break;
    }
    else if (cancelBtn.GetState() == STATE_CLICKED)
    {
      break;
    }
    else if (helpBtn.GetState() == STATE_CLICKED)
    {
      helpBtn.ResetState();

      WindowPrompt(
        "HINT",
        "The option menu labels are clickable. Click on them for more information!",
        "OK",
        NULL,
        &promptWindow);
    }
    // player buttons
    else if (playerLeftArrowBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      playerLeftArrowBtn.SetState(STATE_SELECTED);
      player--;
      if (player < 0)
        player = MAX_PLAYERS - 1;

      sprintf(playerBuf + 7, "%d", player + 1);
      playerTxt.SetText(playerBuf);
      rotationTxt.SetText(isNormalRotation[player] ? "normal" : "reverse");
      shakeTxt.SetText(isShakeEnabled[player] ? "on" : "off");
      shadowTxt.SetText(isShadowEnabled[player] ? "on" : "off");
      previewTxt.SetText(isPreviewEnabled[player] ? "on" : "off");
      ResumeGui();
    }
    else if (playerRightArrowBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      playerRightArrowBtn.SetState(STATE_SELECTED);
      player++;
      if (player >= MAX_PLAYERS)
        player = 0;

      sprintf(playerBuf + 7, "%d", player + 1);
      playerTxt.SetText(playerBuf);
      rotationTxt.SetText(isNormalRotation[player] ? "normal" : "reverse");
      shakeTxt.SetText(isShakeEnabled[player] ? "on" : "off");
      shadowTxt.SetText(isShadowEnabled[player] ? "on" : "off");
      previewTxt.SetText(isPreviewEnabled[player] ? "on" : "off");
      ResumeGui();
    }
    // rotation
    else if (rotationTxtBtn.GetState() == STATE_CLICKED)
    {
      rotationTxtBtn.ResetState();

      WindowPrompt(
        "ROTATION",
        "The direction in which the tetris cylinder rotates. "
        "Set it to reverse if you're too used to regular tetris.",
        "OK",
        NULL,
        &promptWindow);
    }
    else if (rotationLeftArrowBtn.GetState() == STATE_CLICKED 
      || rotationRightArrowBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      rotationLeftArrowBtn.ResetState();
      rotationRightArrowBtn.ResetState();
      isNormalRotation[player] = !isNormalRotation[player];
      rotationTxt.SetText(isNormalRotation[player] ? "normal" : "reverse");
      ResumeGui();
    }
    // shake
    else if (shakeTxtBtn.GetState() == STATE_CLICKED)
    {
      shakeTxtBtn.ResetState();

      WindowPrompt(
        "SHAKE",
        "Allows tetris pieces to be dropped by performing a vertical shake "
        "motion with the wiimote.",
        "OK",
        NULL,
        &promptWindow);
    }
    else if (shakeLeftArrowBtn.GetState() == STATE_CLICKED 
      || shakeRightArrowBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      shakeLeftArrowBtn.ResetState();
      shakeRightArrowBtn.ResetState();
      isShakeEnabled[player] = !isShakeEnabled[player];
      shakeTxt.SetText(isShakeEnabled[player] ? "on" : "off");
      ResumeGui();
    }
    // shadow
    else if (shadowTxtBtn.GetState() == STATE_CLICKED)
    {
      shadowTxtBtn.ResetState();

      WindowPrompt(
        "SHADOW",
        "Shows where the current piece will drop.",
        "OK",
        NULL,
        &promptWindow);
    }
    else if (shadowLeftArrowBtn.GetState() == STATE_CLICKED 
      || shadowRightArrowBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      shadowLeftArrowBtn.ResetState();
      shadowRightArrowBtn.ResetState();
      isShadowEnabled[player] = !isShadowEnabled[player];
      shadowTxt.SetText(isShadowEnabled[player] ? "on" : "off");
      ResumeGui();
    }
    // preview
    else if (previewTxtBtn.GetState() == STATE_CLICKED)
    {
      previewTxtBtn.ResetState();

      WindowPrompt(
        "PREVIEW",
        "Shows the next piece.",
        "OK",
        NULL,
        &promptWindow);
    }
    else if (previewLeftArrowBtn.GetState() == STATE_CLICKED 
      || previewRightArrowBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      previewLeftArrowBtn.ResetState();
      previewRightArrowBtn.ResetState();
      isPreviewEnabled[player] = !isPreviewEnabled[player];
      previewTxt.SetText(isPreviewEnabled[player] ? "on" : "off");
      ResumeGui();
    }
  }

  promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
  while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);
  HaltGui();
  mainWindow->Remove(&promptWindow);
  mainWindow->SetState(STATE_DEFAULT);
  ResumeGui();
}

/// Launches the powerups popup.
void TCYC_MenuPowerupsPopup(GuiWindow *parentWindow, 
                            vector<bool> &isPowerupEnabled,
                            PowerupId powerupStartQueue[MAX_ACQUIRED_POWERUPS])
{
  GXColor helpTxtColor = (GXColor){0, 170, 0, 255};
  GXColor blackColor = (GXColor){0, 0, 0, 255};

  vector<bool> tmpIsPowerupEnabled(isPowerupEnabled);

  PowerupId tmpPowerupStartQueue[MAX_ACQUIRED_POWERUPS];
  memcpy(tmpPowerupStartQueue, powerupStartQueue, sizeof(tmpPowerupStartQueue));

  // popup window
  GuiWindow window(screenwidth - 100, screenheight - 100);
  window.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
  window.SetPosition(0, 0);
  window.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

  // background image
  GuiImage bgImg(screenwidth - 100, screenheight - 100, (GXColor){150, 150, 150, 255});

  // title text
  GuiText titleTxt("Powerups", 30, blackColor);
  titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  titleTxt.SetPosition(0, 15);

  // CONFIRM BUTTON
  GuiButton confirmBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText confirmBtnTxt("OK", 22, (GXColor){0, 0, 255, 255});
  GuiImage confirmBtnImg(btnData40x40Square);
  GuiImage confirmBtnImgOver(btnData40x40SquareOver);

  confirmBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  confirmBtn.SetPosition(-10, -10);
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

  cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  cancelBtn.SetPosition(-10, 10);
  cancelBtn.SetLabel(&cancelBtnTxt);
  cancelBtn.SetImage(&cancelBtnImg);
  cancelBtn.SetImageOver(&cancelBtnImgOver);
  cancelBtn.SetSoundOver(btnSoundOver);
  cancelBtn.SetTrigger(trigA);
  cancelBtn.SetEffectGrow();

  // HELP BUTTON
  GuiButton helpBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText helpBtnTxt("?", 22, helpTxtColor);
  GuiImage helpBtnImg(btnData40x40Square);
  GuiImage helpBtnImgOver(btnData40x40SquareOver);

  helpBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
  helpBtn.SetPosition(10, -10);
  helpBtn.SetLabel(&helpBtnTxt);
  helpBtn.SetImage(&helpBtnImg);
  helpBtn.SetImageOver(&helpBtnImgOver);
  helpBtn.SetSoundOver(btnSoundOver);
  helpBtn.SetTrigger(trigA);
  helpBtn.SetEffectGrow();

  // ALL ON BUTTON
  GuiButton allOnBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText allOnTxt("on", 22, blackColor);
  GuiImage allOnImg(btnData40x40Square);
  GuiImage allOnImgOver(btnData40x40SquareOver);

  allOnBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
  allOnBtn.SetPosition(-btnData40x40Square->GetWidth() / 2, -10);
  allOnBtn.SetLabel(&allOnTxt);
  allOnBtn.SetImage(&allOnImg);
  allOnBtn.SetImageOver(&allOnImgOver);
  allOnBtn.SetSoundOver(btnSoundOver);
  allOnBtn.SetTrigger(trigA);
  allOnBtn.SetEffectGrow();

  // ALL OFF BUTTON
  GuiButton allOffBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText allOffTxt("off", 22, blackColor);
  GuiImage allOffImg(btnData40x40Square);
  GuiImage allOffImgOver(btnData40x40SquareOver);

  allOffBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
  allOffBtn.SetPosition(btnData40x40Square->GetWidth() / 2, -10);
  allOffBtn.SetLabel(&allOffTxt);
  allOffBtn.SetImage(&allOffImg);
  allOffBtn.SetImageOver(&allOffImgOver);
  allOffBtn.SetSoundOver(btnSoundOver);
  allOffBtn.SetTrigger(trigA);
  allOffBtn.SetEffectGrow();

  // SCROLL UP BUTTON
  GuiButton scrollUpBtn(btnDataPlus->GetWidth(), btnDataPlus->GetHeight());
  GuiImage scrollUpImg(btnDataPlus);
  GuiImage scrollUpImgOver(btnDataPlusOver);

  scrollUpBtn.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
  scrollUpBtn.SetPosition(-10, -btnDataPlus->GetHeight() / 2);
  scrollUpBtn.SetImage(&scrollUpImg);
  scrollUpBtn.SetImageOver(&scrollUpImgOver);
  scrollUpBtn.SetSoundOver(btnSoundOver);
  scrollUpBtn.SetTrigger(trigA);
  scrollUpBtn.SetEffectGrow();

  // SCROLL DOWN BUTTON
  GuiButton scrollDownBtn(btnDataMinus->GetWidth(), btnDataMinus->GetHeight());
  GuiImage scrollDownImg(btnDataMinus);
  GuiImage scrollDownImgOver(btnDataMinusOver);

  scrollDownBtn.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
  scrollDownBtn.SetPosition(-10, btnDataMinus->GetHeight() / 2);
  scrollDownBtn.SetImage(&scrollDownImg);
  scrollDownBtn.SetImageOver(&scrollDownImgOver);
  scrollDownBtn.SetSoundOver(btnSoundOver);
  scrollDownBtn.SetTrigger(trigA);
  scrollDownBtn.SetEffectGrow();

  // add buttons to window
  window.Append(&bgImg);
  window.Append(&titleTxt);
  window.Append(&confirmBtn);
  window.Append(&cancelBtn);
  window.Append(&helpBtn);
  window.Append(&allOnBtn);
  window.Append(&allOffBtn);
  window.Append(&scrollUpBtn);
  window.Append(&scrollDownBtn);

  // POWERUP BUTTONS
  int top = 60;

  vector<GuiButton> powerupIconBtns(g_totalPowerups);
  vector<GuiImage> powerupIconImgs(g_totalPowerups);

  vector<GuiButton> powerupOnBtns(g_totalPowerups);
  vector<GuiText> powerupOnTxts(g_totalPowerups);
  vector<GuiImage> powerupOnImgs(g_totalPowerups);
  vector<GuiImage> powerupOnImgsOver(g_totalPowerups);

  vector<GuiButton> powerupHelpBtns(g_totalPowerups);
  vector<GuiText> powerupHelpTxts(g_totalPowerups);
  vector<GuiImage> powerupHelpImgs(g_totalPowerups);
  vector<GuiImage> powerupHelpImgsOver(g_totalPowerups);

  for (int i = 0; i < g_totalPowerups; ++i)
  {
    // powerup icon buttons
    powerupIconImgs[i].SetImage(PowerupUtils::GetImageData((PowerupId)i));
    powerupIconImgs[i].SetDisplaySize(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());

    powerupIconBtns[i].SetSize(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
    powerupIconBtns[i].SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    powerupIconBtns[i].SetPosition(20, top + i * btnData40x40Square->GetHeight());
    powerupIconBtns[i].SetImage(&powerupIconImgs[i]);
    powerupIconBtns[i].SetSoundOver(btnSoundOver);
    powerupIconBtns[i].SetTrigger(trigA);
    powerupIconBtns[i].SetEffectGrow();

    // powerup on/off buttons
    powerupOnTxts[i].SetText(tmpIsPowerupEnabled[i] ? "on" : "off");
    powerupOnTxts[i].SetFontSize(22);
    powerupOnTxts[i].SetColor(blackColor);
    powerupOnImgs[i].SetImage(btnData40x40Square);
    powerupOnImgsOver[i].SetImage(btnData40x40SquareOver);

    powerupOnBtns[i].SetSize(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
    powerupOnBtns[i].SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    powerupOnBtns[i].SetPosition(20 + btnData40x40Square->GetWidth(), top + i * btnData40x40Square->GetHeight());
    powerupOnBtns[i].SetLabel(&powerupOnTxts[i]);
    powerupOnBtns[i].SetImage(&powerupOnImgs[i]);
    powerupOnBtns[i].SetImageOver(&powerupOnImgsOver[i]);
    powerupOnBtns[i].SetSoundOver(btnSoundOver);
    powerupOnBtns[i].SetTrigger(trigA);
    powerupOnBtns[i].SetEffectGrow();

    // powerup help buttons
    powerupHelpTxts[i].SetText("?");
    powerupHelpTxts[i].SetFontSize(22);
    powerupHelpTxts[i].SetColor(helpTxtColor);
    powerupHelpImgs[i].SetImage(btnData40x40Square);
    powerupHelpImgsOver[i].SetImage(btnData40x40SquareOver);

    powerupHelpBtns[i].SetSize(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
    powerupHelpBtns[i].SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    powerupHelpBtns[i].SetPosition(20 + 2 * btnData40x40Square->GetWidth(), top + i * btnData40x40Square->GetHeight());
    powerupHelpBtns[i].SetLabel(&powerupHelpTxts[i]);
    powerupHelpBtns[i].SetImage(&powerupHelpImgs[i]);
    powerupHelpBtns[i].SetImageOver(&powerupHelpImgsOver[i]);
    powerupHelpBtns[i].SetSoundOver(btnSoundOver);
    powerupHelpBtns[i].SetTrigger(trigA);
    powerupHelpBtns[i].SetEffectGrow();

    window.Append(&powerupIconBtns[i]);
    window.Append(&powerupOnBtns[i]);
    window.Append(&powerupHelpBtns[i]);
  }

  // POWERUP QUEUE BUTTONS
  GuiButton powerupQueueBtns[MAX_ACQUIRED_POWERUPS];
  GuiImage powerupQueueImgs[MAX_ACQUIRED_POWERUPS];

  int y = 0;
  for (int i = 0; i < MAX_ACQUIRED_POWERUPS; ++i, y += POWERUP_WIDTH)
  {
    powerupQueueImgs[i].SetImage(tmpPowerupStartQueue[i] == POWERUP_ID_NONE ? 
      btnData40x40Square : PowerupUtils::GetImageData(tmpPowerupStartQueue[i]));
    powerupQueueImgs[i].SetDisplaySize(POWERUP_WIDTH, POWERUP_WIDTH);

    powerupQueueBtns[i].SetSize(POWERUP_WIDTH, POWERUP_WIDTH);
    powerupQueueBtns[i].SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    powerupQueueBtns[i].SetPosition(-60, 60 + y);
    powerupQueueBtns[i].SetImage(powerupQueueImgs + i);
    powerupQueueBtns[i].SetSoundOver(btnSoundOver);
    powerupQueueBtns[i].SetTrigger(trigA);
    powerupQueueBtns[i].SetEffectGrow();

    window.Append(powerupQueueBtns + i);
  }

  HaltGui();
  // This sets the state for all child elements, including child windows; 
  // that is why we disable the state first, then add the child window.
  parentWindow->DisableChildren();
  parentWindow->Append(&window);
  ResumeGui();

  while (true)
  {
    usleep(THREAD_SLEEP);

    if (confirmBtn.GetState() == STATE_CLICKED)
    {
      isPowerupEnabled = tmpIsPowerupEnabled;
      memcpy(powerupStartQueue, tmpPowerupStartQueue, sizeof(tmpPowerupStartQueue));
      break;
    }
    else if (cancelBtn.GetState() == STATE_CLICKED)
    {
      break;
    }
    else if (helpBtn.GetState() == STATE_CLICKED)
    {
      helpBtn.ResetState();

      WindowPrompt(
        "HINT",
        "Click powerup icon to add it to initial powerup queue. Click "
        "icon in queue to remove it. Powerups only appear in "
        "game if turned on.",
        "OK",
        NULL,
        &window);
    }
    else if (allOnBtn.GetState() == STATE_CLICKED
             || allOffBtn.GetState() == STATE_CLICKED)
    {
      bool enabled = (allOnBtn.GetState() == STATE_CLICKED);
      allOnBtn.ResetState();
      allOffBtn.ResetState();
      for (int i = 0; i < g_totalPowerups; ++i)
      {
        tmpIsPowerupEnabled[i] = enabled;
        powerupOnTxts[i].SetText(enabled ? "on" : "off");
      }
    }
    else if (scrollUpBtn.GetState() == STATE_CLICKED
             || scrollDownBtn.GetState() == STATE_CLICKED)
    {
      bool scrollUp = (scrollUpBtn.GetState() == STATE_CLICKED);
      scrollUpBtn.ResetState();
      scrollDownBtn.ResetState();
      if (scrollUp)
      {
        top += 2 * 40; // scroll 2 powerups at a time
        if (top > 60)
          top = 60;
      }
      else
      {
        top -= 2 * 40; // scroll 2 powerups at a time
        int min = 60 - (g_totalPowerups - 2) * 40;
        if (top < min)
          top = min; // always show at least 2 powerups
      }
      for (int i = 0; i < g_totalPowerups; ++i)
      {
        int ypos = top + i * btnData40x40Square->GetHeight();
        powerupIconBtns[i].SetPosition(20, ypos);
        powerupOnBtns[i].SetPosition(20 + btnData40x40Square->GetWidth(), ypos);
        powerupHelpBtns[i].SetPosition(20 + 2 * btnData40x40Square->GetWidth(), ypos);

        int state = STATE_DEFAULT;
        bool visible = true;

        // only show 6 powerups at a time
        if (ypos < 60 || ypos > 60 + 5 * 40)
        {
          state = STATE_DISABLED;
          visible = false;
        }

        powerupIconBtns[i].SetState(state);
        powerupIconBtns[i].SetVisible(visible);
        powerupOnBtns[i].SetState(state);
        powerupOnBtns[i].SetVisible(visible);
        powerupHelpBtns[i].SetState(state);
        powerupHelpBtns[i].SetVisible(visible);
      }
    }

    for (int i = 0; i < g_totalPowerups; ++i)
    {
      if (powerupIconBtns[i].GetState() == STATE_CLICKED)
      {
        powerupIconBtns[i].ResetState();
        // Add powerup to queue.
        for (int j = 0; j < MAX_ACQUIRED_POWERUPS; ++j)
        {
          if (tmpPowerupStartQueue[j] == POWERUP_ID_NONE)
          {
            PowerupUtils::GetSound((PowerupId)i)->Play();
            tmpPowerupStartQueue[j] = (PowerupId)i;
            powerupQueueImgs[j].SetImage(PowerupUtils::GetImageData((PowerupId)i));
            break;
          }
        }
        break;
      }
      else if (powerupOnBtns[i].GetState() == STATE_CLICKED)
      {
        powerupOnBtns[i].ResetState();
        tmpIsPowerupEnabled[i] = !tmpIsPowerupEnabled[i];
        powerupOnTxts[i].SetText(tmpIsPowerupEnabled[i] ? "on" : "off");
        break;
      }
      else if (powerupHelpBtns[i].GetState() == STATE_CLICKED)
      {
        powerupHelpBtns[i].ResetState();
        std::string *helpText = PowerupUtils::GetHelpText((PowerupId)i);
        WindowPrompt(
          helpText[0].c_str(), // title
          helpText[1].c_str(), // message
          "OK",
          NULL,
          &window);
        break;
      }
    }

    for (int i = 0; i < MAX_ACQUIRED_POWERUPS; ++i)
    {
      if (powerupQueueBtns[i].GetState() == STATE_CLICKED)
      {
        powerupQueueBtns[i].ResetState();
        // Remove powerup from queue.
        PowerupUtils::GetInvalidTargetSound()->Play();
        tmpPowerupStartQueue[i] = POWERUP_ID_NONE;
        powerupQueueImgs[i].SetImage(btnData40x40Square);
        break;
      }
    }
  }

  window.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
  while (window.GetEffect() > 0) usleep(THREAD_SLEEP);
  HaltGui();
  parentWindow->Remove(&window);
  parentWindow->SetState(STATE_DEFAULT);
  ResumeGui();
}

/// Launches the multiplayer options popup.
void TCYC_MenuMpOptionsPopup(bool isGlobalOptions)
{
  static const int PADDING_TOP = 8;
  GXColor helpTxtColor = (GXColor){0, 170, 0, 255};
  GXColor blackColor = (GXColor){0, 0, 0, 255};

  int player = 0;
  char playerBuf[7+1+1] = "PLAYER "; // values: 1-MAX_PLAYERS + "PLAYER "
  sprintf(playerBuf + 7, "%d", player + 1);

  bool isHandicapEnabled[MAX_PLAYERS];
  Profile profiles[MAX_PLAYERS];

  if (isGlobalOptions)
  {
    // Store the global options at index 0.
    isHandicapEnabled[player] = true; // Global options are always enabled.
    profiles[player] = g_options->profile;
  }
  else
  {
    // Initialize the handicap options.
    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
      isHandicapEnabled[i] = g_players[i].isHandicapEnabled;
      profiles[i] = g_players[i].profile;
    }
  }

  char maxLinesBuf[3+1]; // values: 0-995
  sprintf(maxLinesBuf, "%d", profiles[player].maxLines);
  char defMaxLinesBuf[3+1];
  sprintf(defMaxLinesBuf, "%d", DEFAULT_MAX_PLAYLINES);

  char attackRateBuf[2+1]; // values: 0-99
  sprintf(attackRateBuf, "%d", profiles[player].attackRate);
  char defAttackRateBuf[2+1];
  sprintf(defAttackRateBuf, "%d", DEFAULT_ATTACK_RATE);

  char powerupRateBuf[2+1]; // values: 0-99
  sprintf(powerupRateBuf, "%d", profiles[player].powerupRate);
  char defPowerupRateBuf[2+1];
  sprintf(defPowerupRateBuf, "%d", DEFAULT_POWERUP_RATE);

  // popup window
  GuiWindow promptWindow(screenwidth - 100, screenheight - 100);
  promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
  promptWindow.SetPosition(0, 0);

  // background image
  GuiImage mpOptBgImg(screenwidth - 100, screenheight - 100, (GXColor){170, 170, 170, 255});

  // title text
  GuiText titleTxt(isGlobalOptions ? "Multiplayer Options" : "Handicap Options", 30, blackColor);
  titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  titleTxt.SetPosition(0, 15);

  // PLAYER LEFT ARROW BUTTON
  GuiButton playerLeftArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText playerLeftArrowBtnTxt("<", 22, blackColor);
  GuiText playerLeftArrowBtnTxtOver("<", 22, helpTxtColor);
  GuiImage playerLeftArrowBtnImg(btnData40x40Square);
  GuiImage playerLeftArrowBtnImgOver(btnData40x40SquareOver);

  playerLeftArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  playerLeftArrowBtn.SetPosition(-80, 60);
  playerLeftArrowBtn.SetLabel(&playerLeftArrowBtnTxt);
  playerLeftArrowBtn.SetLabelOver(&playerLeftArrowBtnTxtOver);
  playerLeftArrowBtn.SetImage(&playerLeftArrowBtnImg);
  playerLeftArrowBtn.SetImageOver(&playerLeftArrowBtnImgOver);
  playerLeftArrowBtn.SetSoundOver(btnSoundOver);
  playerLeftArrowBtn.SetTrigger(trigA);
  playerLeftArrowBtn.SetEffectGrow();

  // PLAYER RIGHT ARROW BUTTON
  GuiButton playerRightArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText playerRightArrowBtnTxt(">", 22, blackColor);
  GuiText playerRightArrowBtnTxtOver(">", 22, helpTxtColor);
  GuiImage playerRightArrowBtnImg(btnData40x40Square);
  GuiImage playerRightArrowBtnImgOver(btnData40x40SquareOver);

  playerRightArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  playerRightArrowBtn.SetPosition(80, 60);
  playerRightArrowBtn.SetLabel(&playerRightArrowBtnTxt);
  playerRightArrowBtn.SetLabelOver(&playerRightArrowBtnTxtOver);
  playerRightArrowBtn.SetImage(&playerRightArrowBtnImg);
  playerRightArrowBtn.SetImageOver(&playerRightArrowBtnImgOver);
  playerRightArrowBtn.SetSoundOver(btnSoundOver);
  playerRightArrowBtn.SetTrigger(trigA);
  playerRightArrowBtn.SetEffectGrow();

  // PLAYER TEXT
  GuiText playerTxt(playerBuf, 22, blackColor);
  playerTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  playerTxt.SetPosition(0, 60 + PADDING_TOP);

  // HANDICAP LEFT ARROW BUTTON
  GuiButton handicapLeftArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText handicapLeftArrowBtnTxt("<", 22, blackColor);
  GuiText handicapLeftArrowBtnTxtOver("<", 22, helpTxtColor);
  GuiImage handicapLeftArrowBtnImg(btnData40x40Square);
  GuiImage handicapLeftArrowBtnImgOver(btnData40x40SquareOver);

  handicapLeftArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  handicapLeftArrowBtn.SetPosition(-80, 60 + btnData40x40Square->GetHeight());
  handicapLeftArrowBtn.SetLabel(&handicapLeftArrowBtnTxt);
  handicapLeftArrowBtn.SetLabelOver(&handicapLeftArrowBtnTxtOver);
  handicapLeftArrowBtn.SetImage(&handicapLeftArrowBtnImg);
  handicapLeftArrowBtn.SetImageOver(&handicapLeftArrowBtnImgOver);
  handicapLeftArrowBtn.SetSoundOver(btnSoundOver);
  handicapLeftArrowBtn.SetTrigger(trigA);
  handicapLeftArrowBtn.SetEffectGrow();

  // HANDICAP RIGHT ARROW BUTTON
  GuiButton handicapRightArrowBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText handicapRightArrowBtnTxt(">", 22, blackColor);
  GuiText handicapRightArrowBtnTxtOver(">", 22, helpTxtColor);
  GuiImage handicapRightArrowBtnImg(btnData40x40Square);
  GuiImage handicapRightArrowBtnImgOver(btnData40x40SquareOver);

  handicapRightArrowBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  handicapRightArrowBtn.SetPosition(80, 60 + btnData40x40Square->GetHeight());
  handicapRightArrowBtn.SetLabel(&handicapRightArrowBtnTxt);
  handicapRightArrowBtn.SetLabelOver(&handicapRightArrowBtnTxtOver);
  handicapRightArrowBtn.SetImage(&handicapRightArrowBtnImg);
  handicapRightArrowBtn.SetImageOver(&handicapRightArrowBtnImgOver);
  handicapRightArrowBtn.SetSoundOver(btnSoundOver);
  handicapRightArrowBtn.SetTrigger(trigA);
  handicapRightArrowBtn.SetEffectGrow();

  // HANDICAP TEXT
  GuiText handicapTxt(isHandicapEnabled[player] ? "enabled" : "disabled", 22, blackColor);
  handicapTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  handicapTxt.SetPosition(0, 60 + PADDING_TOP + btnData40x40Square->GetHeight());

  // MAX LINES TEXT BUTTON
  GuiButton linesTxtBtn((10 + 1) * 11, 22);
  GuiText linesTxtBtnTxt("max lines:", 22, blackColor);
  GuiText linesTxtBtnTxtOver("max lines:", 22, helpTxtColor);

  linesTxtBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  linesTxtBtn.SetPosition(10, 60 + PADDING_TOP + 2 * btnData40x40Square->GetHeight());
  linesTxtBtn.SetLabel(&linesTxtBtnTxt);
  linesTxtBtn.SetLabelOver(&linesTxtBtnTxtOver);
  linesTxtBtn.SetSoundOver(btnSoundOver);
  linesTxtBtn.SetTrigger(trigA);

  // MAX LINES TEXT
  GuiText linesTxt(isHandicapEnabled[player] ? (profiles[player].maxLines ? maxLinesBuf : "off") : "disabled", 22, blackColor);
  linesTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  linesTxt.SetPosition(-20, 60 + PADDING_TOP + 2 * btnData40x40Square->GetHeight());

  // MAX LINES DEFAULT BUTTON
  GuiButton linesDefaultBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText linesDefaultBtnTxt(defMaxLinesBuf, 22, blackColor);
  GuiText linesDefaultBtnTxtOver(defMaxLinesBuf, 22, helpTxtColor);
  GuiImage linesDefaultBtnImg(btnData40x40Square);
  GuiImage linesDefaultBtnImgOver(btnData40x40SquareOver);

  linesDefaultBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  linesDefaultBtn.SetPosition(-10 - 2 * btnData40x40Square->GetWidth(), 60 + 2 * btnData40x40Square->GetHeight());
  linesDefaultBtn.SetLabel(&linesDefaultBtnTxt);
  linesDefaultBtn.SetLabelOver(&linesDefaultBtnTxtOver);
  linesDefaultBtn.SetImage(&linesDefaultBtnImg);
  linesDefaultBtn.SetImageOver(&linesDefaultBtnImgOver);
  linesDefaultBtn.SetSoundOver(btnSoundOver);
  linesDefaultBtn.SetTrigger(trigA);
  linesDefaultBtn.SetEffectGrow();

  // MAX LINES MINUS BUTTON
  GuiButton linesMinusBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText linesMinusBtnTxt("-", 22, blackColor);
  GuiText linesMinusBtnTxtOver("-", 22, helpTxtColor);
  GuiImage linesMinusBtnImg(btnData40x40Square);
  GuiImage linesMinusBtnImgOver(btnData40x40SquareOver);

  linesMinusBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  linesMinusBtn.SetPosition(-10 - btnData40x40Square->GetWidth(), 60 + 2 * btnData40x40Square->GetHeight());
  linesMinusBtn.SetLabel(&linesMinusBtnTxt);
  linesMinusBtn.SetLabelOver(&linesMinusBtnTxtOver);
  linesMinusBtn.SetImage(&linesMinusBtnImg);
  linesMinusBtn.SetImageOver(&linesMinusBtnImgOver);
  linesMinusBtn.SetSoundOver(btnSoundOver);
  linesMinusBtn.SetTrigger(trigA);
  linesMinusBtn.SetEffectGrow();

  // MAX LINES PLUS BUTTON
  GuiButton linesPlusBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText linesPlusBtnTxt("+", 22, blackColor);
  GuiText linesPlusBtnTxtOver("+", 22, helpTxtColor);
  GuiImage linesPlusBtnImg(btnData40x40Square);
  GuiImage linesPlusBtnImgOver(btnData40x40SquareOver);

  linesPlusBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  linesPlusBtn.SetPosition(-10, 60 + 2 * btnData40x40Square->GetHeight());
  linesPlusBtn.SetLabel(&linesPlusBtnTxt);
  linesPlusBtn.SetLabelOver(&linesPlusBtnTxtOver);
  linesPlusBtn.SetImage(&linesPlusBtnImg);
  linesPlusBtn.SetImageOver(&linesPlusBtnImgOver);
  linesPlusBtn.SetSoundOver(btnSoundOver);
  linesPlusBtn.SetTrigger(trigA);
  linesPlusBtn.SetEffectGrow();

  // ATTACK RATE TEXT BUTTON
  GuiButton attackTxtBtn((12 + 1) * 11, 22);
  GuiText attackTxtBtnTxt("attack rate:", 22, blackColor);
  GuiText attackTxtBtnTxtOver("attack rate:", 22, helpTxtColor);

  attackTxtBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  attackTxtBtn.SetPosition(10, 60 + PADDING_TOP + 3 * btnData40x40Square->GetHeight());
  attackTxtBtn.SetLabel(&attackTxtBtnTxt);
  attackTxtBtn.SetLabelOver(&attackTxtBtnTxtOver);
  attackTxtBtn.SetSoundOver(btnSoundOver);
  attackTxtBtn.SetTrigger(trigA);

  // ATTACK RATE TEXT
  GuiText attackTxt(isHandicapEnabled[player] ? (profiles[player].attackRate ? attackRateBuf : "off") : "disabled", 22, blackColor);
  attackTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  attackTxt.SetPosition(-20, 60 + PADDING_TOP + 3 * btnData40x40Square->GetHeight());

  // ATTACK RATE DEFAULT BUTTON
  GuiButton attackDefaultBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText attackDefaultBtnTxt(defAttackRateBuf, 22, blackColor);
  GuiText attackDefaultBtnTxtOver(defAttackRateBuf, 22, helpTxtColor);
  GuiImage attackDefaultBtnImg(btnData40x40Square);
  GuiImage attackDefaultBtnImgOver(btnData40x40SquareOver);

  attackDefaultBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  attackDefaultBtn.SetPosition(-10 - 2 * btnData40x40Square->GetWidth(), 60 + 3 * btnData40x40Square->GetHeight());
  attackDefaultBtn.SetLabel(&attackDefaultBtnTxt);
  attackDefaultBtn.SetLabelOver(&attackDefaultBtnTxtOver);
  attackDefaultBtn.SetImage(&attackDefaultBtnImg);
  attackDefaultBtn.SetImageOver(&attackDefaultBtnImgOver);
  attackDefaultBtn.SetSoundOver(btnSoundOver);
  attackDefaultBtn.SetTrigger(trigA);
  attackDefaultBtn.SetEffectGrow();

  // ATTACK RATE MINUS BUTTON
  GuiButton attackMinusBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText attackMinusBtnTxt("-", 22, blackColor);
  GuiText attackMinusBtnTxtOver("-", 22, helpTxtColor);
  GuiImage attackMinusBtnImg(btnData40x40Square);
  GuiImage attackMinusBtnImgOver(btnData40x40SquareOver);

  attackMinusBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  attackMinusBtn.SetPosition(-10 - btnData40x40Square->GetWidth(), 60 + 3 * btnData40x40Square->GetHeight());
  attackMinusBtn.SetLabel(&attackMinusBtnTxt);
  attackMinusBtn.SetLabelOver(&attackMinusBtnTxtOver);
  attackMinusBtn.SetImage(&attackMinusBtnImg);
  attackMinusBtn.SetImageOver(&attackMinusBtnImgOver);
  attackMinusBtn.SetSoundOver(btnSoundOver);
  attackMinusBtn.SetTrigger(trigA);
  attackMinusBtn.SetEffectGrow();

  // ATTACK RATE PLUS BUTTON
  GuiButton attackPlusBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText attackPlusBtnTxt("+", 22, blackColor);
  GuiText attackPlusBtnTxtOver("+", 22, helpTxtColor);
  GuiImage attackPlusBtnImg(btnData40x40Square);
  GuiImage attackPlusBtnImgOver(btnData40x40SquareOver);

  attackPlusBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  attackPlusBtn.SetPosition(-10, 60 + 3 * btnData40x40Square->GetHeight());
  attackPlusBtn.SetLabel(&attackPlusBtnTxt);
  attackPlusBtn.SetLabelOver(&attackPlusBtnTxtOver);
  attackPlusBtn.SetImage(&attackPlusBtnImg);
  attackPlusBtn.SetImageOver(&attackPlusBtnImgOver);
  attackPlusBtn.SetSoundOver(btnSoundOver);
  attackPlusBtn.SetTrigger(trigA);
  attackPlusBtn.SetEffectGrow();

  //---
  // POWERUP RATE TEXT BUTTON
  GuiButton powerupRateTxtBtn((14 + 1) * 11, 22);
  GuiText powerupRateTxtBtnTxt("powerup rate:", 22, blackColor);
  GuiText powerupRateTxtBtnTxtOver("powerup rate:", 22, helpTxtColor);

  powerupRateTxtBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
  powerupRateTxtBtn.SetPosition(10, 60 + PADDING_TOP + 4 * 40);
  powerupRateTxtBtn.SetLabel(&powerupRateTxtBtnTxt);
  powerupRateTxtBtn.SetLabelOver(&powerupRateTxtBtnTxtOver);
  powerupRateTxtBtn.SetSoundOver(btnSoundOver);
  powerupRateTxtBtn.SetTrigger(trigA);

  // POWERUP RATE TEXT
  GuiText powerupRateTxt(isHandicapEnabled[player] ? (profiles[player].powerupRate ? powerupRateBuf : "off") : "disabled", 22, blackColor);
  powerupRateTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
  powerupRateTxt.SetPosition(-20, 60 + PADDING_TOP + 4 * 40);

  // POWERUP RATE DEFAULT BUTTON
  GuiButton powerupRateDefaultBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText powerupRateDefaultBtnTxt(defPowerupRateBuf, 22, blackColor);
  GuiText powerupRateDefaultBtnTxtOver(defPowerupRateBuf, 22, helpTxtColor);
  GuiImage powerupRateDefaultBtnImg(btnData40x40Square);
  GuiImage powerupRateDefaultBtnImgOver(btnData40x40SquareOver);

  powerupRateDefaultBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  powerupRateDefaultBtn.SetPosition(-10 - 2 * btnData40x40Square->GetWidth(), 60 + 4 * 40);
  powerupRateDefaultBtn.SetLabel(&powerupRateDefaultBtnTxt);
  powerupRateDefaultBtn.SetLabelOver(&powerupRateDefaultBtnTxtOver);
  powerupRateDefaultBtn.SetImage(&powerupRateDefaultBtnImg);
  powerupRateDefaultBtn.SetImageOver(&powerupRateDefaultBtnImgOver);
  powerupRateDefaultBtn.SetSoundOver(btnSoundOver);
  powerupRateDefaultBtn.SetTrigger(trigA);
  powerupRateDefaultBtn.SetEffectGrow();

  // POWERUP RATE MINUS BUTTON
  GuiButton powerupRateMinusBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText powerupRateMinusBtnTxt("-", 22, blackColor);
  GuiText powerupRateMinusBtnTxtOver("-", 22, helpTxtColor);
  GuiImage powerupRateMinusBtnImg(btnData40x40Square);
  GuiImage powerupRateMinusBtnImgOver(btnData40x40SquareOver);

  powerupRateMinusBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  powerupRateMinusBtn.SetPosition(-10 - btnData40x40Square->GetWidth(), 60 + 4 * 40);
  powerupRateMinusBtn.SetLabel(&powerupRateMinusBtnTxt);
  powerupRateMinusBtn.SetLabelOver(&powerupRateMinusBtnTxtOver);
  powerupRateMinusBtn.SetImage(&powerupRateMinusBtnImg);
  powerupRateMinusBtn.SetImageOver(&powerupRateMinusBtnImgOver);
  powerupRateMinusBtn.SetSoundOver(btnSoundOver);
  powerupRateMinusBtn.SetTrigger(trigA);
  powerupRateMinusBtn.SetEffectGrow();

  // POWERUP RATE PLUS BUTTON
  GuiButton powerupRatePlusBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText powerupRatePlusBtnTxt("+", 22, blackColor);
  GuiText powerupRatePlusBtnTxtOver("+", 22, helpTxtColor);
  GuiImage powerupRatePlusBtnImg(btnData40x40Square);
  GuiImage powerupRatePlusBtnImgOver(btnData40x40SquareOver);

  powerupRatePlusBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  powerupRatePlusBtn.SetPosition(-10, 60 + 4 * 40);
  powerupRatePlusBtn.SetLabel(&powerupRatePlusBtnTxt);
  powerupRatePlusBtn.SetLabelOver(&powerupRatePlusBtnTxtOver);
  powerupRatePlusBtn.SetImage(&powerupRatePlusBtnImg);
  powerupRatePlusBtn.SetImageOver(&powerupRatePlusBtnImgOver);
  powerupRatePlusBtn.SetSoundOver(btnSoundOver);
  powerupRatePlusBtn.SetTrigger(trigA);
  powerupRatePlusBtn.SetEffectGrow();

  // CONFIRM BUTTON
  GuiButton confirmBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText confirmBtnTxt("OK", 22, (GXColor){0, 0, 255, 255});
  GuiImage confirmBtnImg(btnData40x40Square);
  GuiImage confirmBtnImgOver(btnData40x40SquareOver);

  confirmBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  confirmBtn.SetPosition(-10, -10);
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

  cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
  cancelBtn.SetPosition(-10, 10);
  cancelBtn.SetLabel(&cancelBtnTxt);
  cancelBtn.SetImage(&cancelBtnImg);
  cancelBtn.SetImageOver(&cancelBtnImgOver);
  cancelBtn.SetSoundOver(btnSoundOver);
  cancelBtn.SetTrigger(trigA);
  cancelBtn.SetEffectGrow();

  // HELP BUTTON
  GuiButton helpBtn(btnData40x40Square->GetWidth(), btnData40x40Square->GetHeight());
  GuiText helpBtnTxt("?", 22, helpTxtColor);
  GuiImage helpBtnImg(btnData40x40Square);
  GuiImage helpBtnImgOver(btnData40x40SquareOver);

  helpBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
  helpBtn.SetPosition(10, -10);
  helpBtn.SetLabel(&helpBtnTxt);
  helpBtn.SetImage(&helpBtnImg);
  helpBtn.SetImageOver(&helpBtnImgOver);
  helpBtn.SetSoundOver(btnSoundOver);
  helpBtn.SetTrigger(trigA);
  helpBtn.SetEffectGrow();

  // POWERUP BUTTON
  GuiButton powerupBtn(btnOutline->GetWidth(), btnOutline->GetHeight());
  GuiText powerupTxt("Powerups", 22, (GXColor){0, 0, 0, 255});
  GuiImage powerupImg(btnOutline);
  GuiImage powerupImgOver(btnOutlineOver);

  powerupBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
  powerupBtn.SetPosition(0, -10);
  powerupBtn.SetLabel(&powerupTxt);
  powerupBtn.SetImage(&powerupImg);
  powerupBtn.SetImageOver(&powerupImgOver);
  powerupBtn.SetSoundOver(btnSoundOver);
  powerupBtn.SetTrigger(trigA);
  powerupBtn.SetEffectGrow();
  powerupBtn.SetState(!isHandicapEnabled[player] ? STATE_DISABLED : STATE_DEFAULT);
  //powerupBtn.SetVisible(isHandicapEnabled[player]);

  promptWindow.Append(&mpOptBgImg);
  promptWindow.Append(&titleTxt);
  if (!isGlobalOptions)
  {
    // handicap options
    promptWindow.Append(&playerLeftArrowBtn);
    promptWindow.Append(&playerRightArrowBtn);
    promptWindow.Append(&playerTxt);
    promptWindow.Append(&handicapLeftArrowBtn);
    promptWindow.Append(&handicapRightArrowBtn);
    promptWindow.Append(&handicapTxt);
  }
  promptWindow.Append(&linesTxtBtn);
  promptWindow.Append(&linesTxt);
  promptWindow.Append(&linesDefaultBtn);
  promptWindow.Append(&linesMinusBtn);
  promptWindow.Append(&linesPlusBtn);
  promptWindow.Append(&attackTxtBtn);
  promptWindow.Append(&attackTxt);
  promptWindow.Append(&attackDefaultBtn);
  promptWindow.Append(&attackMinusBtn);
  promptWindow.Append(&attackPlusBtn);
  promptWindow.Append(&powerupRateTxtBtn);
  promptWindow.Append(&powerupRateTxt);
  promptWindow.Append(&powerupRateDefaultBtn);
  promptWindow.Append(&powerupRateMinusBtn);
  promptWindow.Append(&powerupRatePlusBtn);
  promptWindow.Append(&confirmBtn);
  promptWindow.Append(&cancelBtn);
  promptWindow.Append(&helpBtn);
  promptWindow.Append(&powerupBtn);

  promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
  HaltGui();
  // This sets the state for all child elements, including child windows; 
  // that is why we disable the state first, then add the child window.
  mainWindow->SetState(STATE_DISABLED);
  mainWindow->Append(&promptWindow);
  mainWindow->ChangeFocus(&promptWindow);
  ResumeGui();

  while (true)
  {
    usleep(THREAD_SLEEP);

    if (confirmBtn.GetState() == STATE_CLICKED)
    {
      if (isGlobalOptions)
      {
        g_options->profile = profiles[player];
        g_options->powerupsSize = 0;
        for (int j = 0; j < g_totalPowerups; ++j)
        {
          if (profiles[player].isPowerupEnabled[j])
            g_options->powerups[g_options->powerupsSize++] = (PowerupId)j;
        }
      }
      else // handicap options
      {
        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
          g_players[i].isHandicapEnabled = isHandicapEnabled[i];
          g_players[i].profile = profiles[i];
          g_players[i].powerupsSize = 0;
          for (int j = 0; j < g_totalPowerups; ++j)
          {
            if (profiles[i].isPowerupEnabled[j])
              g_players[i].powerups[g_players[i].powerupsSize++] = (PowerupId)j;
          }
        }
      }

      break;
    }
    else if (cancelBtn.GetState() == STATE_CLICKED)
    {
      break;
    }
    else if (helpBtn.GetState() == STATE_CLICKED)
    {
      helpBtn.ResetState();

      WindowPrompt(
        "HINT",
        isGlobalOptions ? 
          "The option menu labels are clickable. Click on them for more information!" 
          : "Handicap options override multiplayer options!",
        "OK",
        NULL,
        &promptWindow);
    }
    // player buttons
    else if (playerLeftArrowBtn.GetState() == STATE_CLICKED
             || playerRightArrowBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();

      if (playerLeftArrowBtn.GetState() == STATE_CLICKED)
      {
        playerLeftArrowBtn.SetState(STATE_SELECTED);
        player--;
        if (player < 0)
          player = MAX_PLAYERS - 1;
      }
      else if (playerRightArrowBtn.GetState() == STATE_CLICKED)
      {
        playerRightArrowBtn.SetState(STATE_SELECTED);
        player++;
        if (player >= MAX_PLAYERS)
          player = 0;
      }

      sprintf(playerBuf + 7, "%d", player + 1);
      playerTxt.SetText(playerBuf);
      handicapTxt.SetText(isHandicapEnabled[player] ? "enabled" : "disabled");

      sprintf(maxLinesBuf, "%d", profiles[player].maxLines);
      if (isHandicapEnabled[player])
        linesTxt.SetText(profiles[player].maxLines ? maxLinesBuf : "off");
      else
        linesTxt.SetText("disabled");

      sprintf(attackRateBuf, "%d", profiles[player].attackRate);
      if (isHandicapEnabled[player])
        attackTxt.SetText(profiles[player].attackRate ? attackRateBuf : "off");
      else
        attackTxt.SetText("disabled");

      sprintf(powerupRateBuf, "%d", profiles[player].powerupRate);
      if (isHandicapEnabled[player])
        powerupRateTxt.SetText(profiles[player].powerupRate ? powerupRateBuf : "off");
      else
        powerupRateTxt.SetText("disabled");

      //powerupBtn.SetVisible(isHandicapEnabled[player]);
      powerupBtn.SetState(!isHandicapEnabled[player] ? STATE_DISABLED : STATE_DEFAULT);

      ResumeGui();
    }
    // handicap buttons
    else if (handicapLeftArrowBtn.GetState() == STATE_CLICKED
             || handicapRightArrowBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();

      if (handicapLeftArrowBtn.GetState() == STATE_CLICKED)
        handicapLeftArrowBtn.SetState(STATE_SELECTED);
      else if (handicapRightArrowBtn.GetState() == STATE_CLICKED)
        handicapRightArrowBtn.SetState(STATE_SELECTED);

      isHandicapEnabled[player] = !isHandicapEnabled[player];
      handicapTxt.SetText(isHandicapEnabled[player] ? "enabled" : "disabled");

      sprintf(maxLinesBuf, "%d", profiles[player].maxLines);
      if (isHandicapEnabled[player])
        linesTxt.SetText(profiles[player].maxLines ? maxLinesBuf : "off");
      else
        linesTxt.SetText("disabled");

      sprintf(attackRateBuf, "%d", profiles[player].attackRate);
      if (isHandicapEnabled[player])
        attackTxt.SetText(profiles[player].attackRate ? attackRateBuf : "off");
      else
        attackTxt.SetText("disabled");

      sprintf(powerupRateBuf, "%d", profiles[player].powerupRate);
      if (isHandicapEnabled[player])
        powerupRateTxt.SetText(profiles[player].powerupRate ? powerupRateBuf : "off");
      else
        powerupRateTxt.SetText("disabled");

      //powerupBtn.SetVisible(isHandicapEnabled[player]);
      powerupBtn.SetState(!isHandicapEnabled[player] ? STATE_DISABLED : STATE_DEFAULT);

      ResumeGui();
    }
    // max lines
    else if (linesTxtBtn.GetState() == STATE_CLICKED)
    {
      linesTxtBtn.ResetState();

      WindowPrompt(
        "MAX LINES",
        "You win the game after clearing this many lines.",
        "OK",
        NULL,
        &promptWindow);
    }
    // attack rate
    else if (attackTxtBtn.GetState() == STATE_CLICKED)
    {
      attackTxtBtn.ResetState();

      WindowPrompt(
        "ATTACK RATE",
        "You attack all your opponents every time you score this many points. "
        "This raises the speed of the falling block by one level.",
        "OK",
        NULL,
        &promptWindow);
    }
    // powerup rate
    else if (powerupRateTxtBtn.GetState() == STATE_CLICKED)
    {
      powerupRateTxtBtn.ResetState();

      WindowPrompt(
        "POWERUP RATE",
        "Every 'x' pieces will be a powerup piece. "
        "Clear a powerup piece to earn that powerup.",
        "OK",
        NULL,
        &promptWindow);
    }

    if (!isHandicapEnabled[player])
      continue;

    // max lines
    if (linesDefaultBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      linesDefaultBtn.SetState(STATE_SELECTED);
      profiles[player].maxLines = DEFAULT_MAX_PLAYLINES;
      sprintf(maxLinesBuf, "%d", profiles[player].maxLines);
      linesTxt.SetText(maxLinesBuf);
      ResumeGui();
    }
    else if (linesMinusBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      linesMinusBtn.SetState(STATE_SELECTED);

      if (profiles[player].maxLines <= 5)
        profiles[player].maxLines = 0;
      else
        profiles[player].maxLines -= 5;

      if (profiles[player].maxLines <= 0)
      {
        profiles[player].maxLines = 0;
        linesTxt.SetText("off");
      }
      else
      {
        sprintf(maxLinesBuf, "%d", profiles[player].maxLines);
        linesTxt.SetText(maxLinesBuf);
      }
      ResumeGui();
    }
    else if (linesPlusBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      linesPlusBtn.SetState(STATE_SELECTED);
      profiles[player].maxLines += 5;
      if (profiles[player].maxLines >= 1000)
        profiles[player].maxLines = 995;

      sprintf(maxLinesBuf, "%d", profiles[player].maxLines);
      linesTxt.SetText(maxLinesBuf);
      ResumeGui();
    }
    // attack rate
    else if (attackDefaultBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      attackDefaultBtn.SetState(STATE_SELECTED);
      profiles[player].attackRate = DEFAULT_ATTACK_RATE;
      sprintf(attackRateBuf, "%d", profiles[player].attackRate);
      attackTxt.SetText(attackRateBuf);
      ResumeGui();
    }
    else if (attackMinusBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      attackMinusBtn.SetState(STATE_SELECTED);

      if (profiles[player].attackRate > 0)
        profiles[player].attackRate--;

      if (profiles[player].attackRate <= 0)
      {
        profiles[player].attackRate = 0;
        attackTxt.SetText("off");
      }
      else
      {
        sprintf(attackRateBuf, "%d", profiles[player].attackRate);
        attackTxt.SetText(attackRateBuf);
      }
      ResumeGui();
    }
    else if (attackPlusBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      attackPlusBtn.SetState(STATE_SELECTED);
      profiles[player].attackRate++;
      if (profiles[player].attackRate >= 100)
        profiles[player].attackRate = 99;

      sprintf(attackRateBuf, "%d", profiles[player].attackRate);
      attackTxt.SetText(attackRateBuf);
      ResumeGui();
    }
    // powerup rate
    else if (powerupRateDefaultBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      powerupRateDefaultBtn.SetState(STATE_SELECTED);
      profiles[player].powerupRate = DEFAULT_POWERUP_RATE;
      sprintf(powerupRateBuf, "%d", profiles[player].powerupRate);
      powerupRateTxt.SetText(powerupRateBuf);
      ResumeGui();
    }
    else if (powerupRateMinusBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      powerupRateMinusBtn.SetState(STATE_SELECTED);

      if (profiles[player].powerupRate > 0)
        profiles[player].powerupRate--;

      if (profiles[player].powerupRate <= 0)
      {
        profiles[player].powerupRate = 0;
        powerupRateTxt.SetText("off");
      }
      else
      {
        sprintf(powerupRateBuf, "%d", profiles[player].powerupRate);
        powerupRateTxt.SetText(powerupRateBuf);
      }
      ResumeGui();
    }
    else if (powerupRatePlusBtn.GetState() == STATE_CLICKED)
    {
      HaltGui();
      powerupRatePlusBtn.SetState(STATE_SELECTED);
      profiles[player].powerupRate++;
      if (profiles[player].powerupRate >= 100)
        profiles[player].powerupRate = 99;

      sprintf(powerupRateBuf, "%d", profiles[player].powerupRate);
      powerupRateTxt.SetText(powerupRateBuf);
      ResumeGui();
    }
    // powerups
    else if (powerupBtn.GetState() == STATE_CLICKED)
    {
      powerupBtn.ResetState();
      TCYC_MenuPowerupsPopup(&promptWindow, 
                             profiles[player].isPowerupEnabled, 
                             profiles[player].powerupStartQueue);
    }

    HaltGui();
    // max lines
    if (linesDefaultBtn.GetState() == STATE_SELECTED
        || linesDefaultBtn.GetState() == STATE_CLICKED
        || linesMinusBtn.GetState() == STATE_SELECTED 
        || linesMinusBtn.GetState() == STATE_CLICKED
        || linesPlusBtn.GetState() == STATE_SELECTED
        || linesPlusBtn.GetState() == STATE_CLICKED)
    {
      linesTxt.SetColor(helpTxtColor);
    }
    else
    {
      linesTxt.SetColor(blackColor);
    }
    // attack rate
    if (attackDefaultBtn.GetState() == STATE_SELECTED
        || attackDefaultBtn.GetState() == STATE_CLICKED
        || attackMinusBtn.GetState() == STATE_SELECTED 
        || attackMinusBtn.GetState() == STATE_CLICKED
        || attackPlusBtn.GetState() == STATE_SELECTED
        || attackPlusBtn.GetState() == STATE_CLICKED)
    {
      attackTxt.SetColor(helpTxtColor);
    }
    else
    {
      attackTxt.SetColor(blackColor);
    }
    // powerup rate
    if (powerupRateDefaultBtn.GetState() == STATE_SELECTED
        || powerupRateDefaultBtn.GetState() == STATE_CLICKED
        || powerupRateMinusBtn.GetState() == STATE_SELECTED 
        || powerupRateMinusBtn.GetState() == STATE_CLICKED
        || powerupRatePlusBtn.GetState() == STATE_SELECTED
        || powerupRatePlusBtn.GetState() == STATE_CLICKED)
    {
      powerupRateTxt.SetColor(helpTxtColor);
    }
    else
    {
      powerupRateTxt.SetColor(blackColor);
    }
    ResumeGui();
  }

  promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
  while (promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);
  HaltGui();
  mainWindow->Remove(&promptWindow);
  mainWindow->SetState(STATE_DEFAULT);
  ResumeGui();
}

void _DoEasterEgg()
{
  GuiWindow heart(screenwidth, screenheight);
  GuiImageData tetrisHeartPink(tetris_heart_pink_png);
  GuiImage heartBg(&tetrisHeartPink);

  GuiText thanks("Thank you fans!", 24, (GXColor){0, 0, 0, 255});
  thanks.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  thanks.SetPosition(-20, -55);

  GuiText sig("-Cale", 24, (GXColor){0, 0, 0, 255});
  sig.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
  sig.SetPosition(-20, -25);

  heart.Append(&heartBg);
  heart.Append(&thanks);
  heart.Append(&sig);

  HaltGui();
  mainWindow->DisableChildren();
  mainWindow->Append(&heart);
  mainWindow->ChangeFocus(&heart);
  ResumeGui();

  bgMusic->SetVolume(20);

  bool loop = true;
  while (loop)
  {
    if (!g_heartbeatSound->IsPlaying())
      g_heartbeatSound->Play();

    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
      if (WPAD_ButtonsDown(i))
      {
        loop = false;
        break;
      }
    }
  }

  g_heartbeatSound->Stop();
  bgMusic->SetVolume(50);

  HaltGui();
  mainWindow->Remove(&heart);
  mainWindow->SetState(STATE_DEFAULT);
  ResumeGui();
}