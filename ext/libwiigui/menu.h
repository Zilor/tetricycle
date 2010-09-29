/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * menu.h
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#ifndef _MENU_H_
#define _MENU_H_

#include <ogcsys.h>

#include "libwiigui/gui.h"

#define THREAD_SLEEP 100

void InitGUIThreads();
void MainMenu();
void ResumeGui();
void HaltGui();
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label = NULL, GuiWindow *parentWindow = NULL);

enum
{
	MENU_EXIT = -1,
	MENU_NONE,
	MENU_SETTINGS,
	MENU_SETTINGS_FILE,
	MENU_BROWSE_DEVICE
};

#endif
