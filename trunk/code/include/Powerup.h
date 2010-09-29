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

/** @file Powerup.h
 * @brief Defines the Powerup class, the base class for powerups.
 * @author Cale Scholl / calvinss4
 *
 * Powerups must extend this class and implement all the pure virtual methods.
 *
 * @page powerupcreationpage Powerup Creation
 *
 * @section powerupnamesection Powerup Name
 * Choose a clever name for your powerup, such as 'clevername'.
 *
 * @section powerupimgsection Powerup Image
 * <ul>
 * <li>Select a PNG image to represent your powerup. Try to keep it small 
 * (in order to reduce the size of the executable). \n 
 * Resize the image to 32x32; if it doesn't look good, try 64x64.</li>
 * <li>Name the file 'powerup_clevername.png' and put the file in the following 
 * directory: \n
 * @b /ext/libwiigui/images</li>
 * <li>Now you must add a declaration for your powerup image. 
 * Open the following file: \n
 * <b>/ext/libwiigui/filelist.h</b> \n
 * Look for the comment 'powerup image declarations go here' and add the 
 * following lines: \n (note: please keep powerups in alphabetical order)
 *   <ul>
 *     <li><b>extern const u8   powerup_clevername_png[];</b></li>
 *     <li><b>extern const u32  powerup_clevername_png_size;</b></li>
 *   </ul>
 * </li>
 * </ul>
 *
 * @section powerupsoundsection Powerup Sound (optional)
 * Selecting a sound to represent your powerup is optional; if you don't 
 * provide one, a default sound will be played.
 * <ul>
 * <li>Select a sound to represent your powerup. The sample should be no more 
 * than a couple seconds long.</li>
 * <li>Convert your sound sample to the following format: \n
 * <b>PCM signed 16 bit, big endian, stereo (raw) at 44100 Hz</b></li>
 *   <ul>
 *     <li>GoldWave is a good program for converting to .pcm format.</li>
 *     <li>If converting from .wav to .pcm, you may have to resample at 44100 Hz.</li>
 *     <li>I found that if I convert from .wav to .ogg or .mp3, then convert 
 *         to .pcm, it's automatically sampled at the correct frequency.</li>
 *   </ul>
 * <li>Name the file 'powerup_clevername.pcm' and put the file in the following 
 * directory: \n
 * @b /ext/libwiigui/sounds</li>
 * <li>Now you must add a declaration for your powerup sound. 
 * Open the following file: \n
 * <b>/ext/libwiigui/filelist.h</b> \n
 * Look for the comment 'powerup sound declarations go here' and add the 
 * following lines: \n (note: please keep powerups in alphabetical order)
 *   <ul>
 *     <li><b>extern const u8   powerup_clevername_pcm[];</b></li>
 *     <li><b>extern const u32  powerup_clevername_pcm_size;</b></li>
 *   </ul>
 * </li>
 * </ul>
 *
 * @section powerupcodefilessection Powerup Code Files
 * Now you're ready to write the header file and source file. There are 
 * templates located in: \n
 * <b>/templates/Powerup</b> \n
 * You could copy these templates and modify them by hand, but I wrote a 
 * script that does it for you; simply run <b>Powerup.exe</b> and follow 
 * the on-screen instructions (if you're paranoid, you can compile the 
 * program from the source).
 * - Move the generated header file to: \n
 * <b>/code/include/powerups</b>
 * - Move the generated source file to: \n
 * <b>/code/source/powerups</b>
 * - If you added a sound for your powerup, uncomment the optional sound code
 *   in the header file and source file.
 * - The only functions you need to implement are StartEffect and StopEffect 
 *   (located in the source file).
 * - Your powerup is automagically added to the game menu.
 *
 * @section powerupoverridessection Optional Overrides
 * By default powerups last for 10000 milliseconds and only affect one player. 
 * You can override this default behavior by providing definitions for
 * the GetDuration and GetTargetType functions in the powerup header file. \n
 * There are 3 target types: \n
 * - <b>POWERUP_TARGET_ONE</b> - Affects the target player.
 * - <b>POWERUP_TARGET_ALL</b> - Affects all players.
 * - <b>POWERUP_TARGET_ALL_BUT_ONE</b> - Affects all but the target player.
 *
 * @section powerupbehaviorsection Powerup Behavior
 * Now comes the hardest part: implementing the actual powerup behavior. 
 * As mentioned previously, you only need to implement the StartEffect and 
 * StopEffect functions in the powerup source file. Look at the other powerup 
 * classes for examples. In general, you will have to add a state flag to 
 * Player::PlayerPowerupData. StartEffect will turn the flag on, and StopEffect 
 * will turn the flag off; some piece of game logic will operate differently 
 * while the flag is turned on. Good luck, you can do it!
 */

#pragma once
#ifndef __POWERUP_H__
#define __POWERUP_H__

#include <ogc/lwp_watchdog.h> // for gettime, ticks_to_millisecs
#include <string>
#include <vector>

#include "defines_Powerup.h"

class GuiImageData;
class GuiSound;

using std::string;
using std::vector;

/// The base class for powerups.
class Powerup
{
  friend class PowerupUtils;

public:
  /// Initiates this powerup on the target player(s).
  bool Initiate(u8 targetPlayer);

  /// Updates the timer and terminates this powerup if its duration has been exceeded.
  void Update()
  {
    u64 currTime = gettime();
    elapsedTime += ticks_to_millisecs(currTime - startTime);
    startTime = currTime;
    if (elapsedTime >= GetDuration())
      Terminate();
  }

  // MUST OVERRIDE:
  virtual PowerupId GetPowerupId() = 0;     ///< The unique PowerupId representing this powerup.
  virtual GuiImageData* GetImageData() = 0; ///< The image associated with this powerup.
  virtual string* GetHelpText() = 0;        ///< A description of this powerup used in the menu.

  // OVERRIDE IF NECESSARY:
  virtual GuiSound* GetSound() { return defaultSound; } ///< The sound associated with this powerup.

protected:
  Powerup() : elapsedTime(0) { }
  virtual ~Powerup() { } 

  // MUST OVERRIDE:
  virtual Powerup* GetInstance() = 0;      ///< Returns a new powerup instance.
  virtual void StartEffect(u8 player) = 0; ///< The powerup state change goes here.
  virtual void StopEffect(u8 player) = 0;  ///< Reverts the state back to normal.

  // OVERRIDE IF NECESSARY:
  virtual u32 GetDuration() { return DEFAULT_POWERUP_DURATION; }       ///< The duration of this powerup, in milliseconds.
  virtual PowerupTarget GetTargetType() { return POWERUP_TARGET_ONE; } ///< Determines what players are the target of this powerup.

  /// Returns the global Powerup vector.
  /** This vector contains a static instance of every Powerup. */
  static vector<Powerup *>& GetVector()
  {
    static vector<Powerup *> powerupVector;
    return powerupVector;
  }

private:
  u64 startTime;
  u32 elapsedTime;
  static GuiSound *defaultSound;

  void Terminate(); ///< Called automatically when a powerup's duration expires.
  void ResetStartTime() { startTime = gettime(); }
};

#endif // __POWERUP_H__