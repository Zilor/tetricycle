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
 * @brief Simple utility for generating Powerup classes.
 * @author Cale Scholl / calvinss4
 */

#include <iostream>
#include <string>
#include <fstream>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ofstream;
using std::ifstream;

string StringToUpper(string strToConvert);
string StringToLower(string strToConvert);
string GetPowerupSpacedName(string powerupName);
void ReplaceToken(string &phrase, string &token, string &value);

/// Prints a fatal error message and exits the program.
#define FATAL_ERROR(message) \
  do { cerr << message << endl, exit(1); } while (0)

int main()
{
  cout << "Input your name (this will be used to specify the author):" << endl;
  string author;
  getline(cin, author);
  cout << "The author of this Powerup class is: " << author << endl;

  cout << "Input a powerup name. For example, if you input \"CleverName\" your powerup class will be named PowerupCleverName." << endl;
  string powerupName;
  cin >> powerupName;
  cin.get(); // extract newline
  cout << "The name of your Powerup class is: Powerup" << powerupName << endl;

  cout << "Input a short description for your powerup (this will be used in the game's help menu):" << endl;
  string powerupDesc;
  getline(cin, powerupDesc);

  // specify tokens
  string authorToken = "Nnnn";
  string powerupNameToken = "Xxxx";
  string powerupUppercaseNameToken = "XXXX";
  string powerupLowercaseNameToken = "xxxx";
  string powerupSpacedNameToken = "Xx_Xx";
  string powerupDescToken = "POWERUP_DESCRIPTION";

  // generate powerup names for the corresponding tokens
  string powerupUppercaseName = StringToUpper(powerupName);
  string powerupLowercaseName = StringToLower(powerupName);
  string powerupSpacedName = GetPowerupSpacedName(powerupName);

  ifstream infile;
  ofstream outfile;
  string powerupFileName;
  string phrase;

  for (int f = 0; f < 2; ++f)
  {
    // open template file and output file
    if (f == 0)
    {
      infile.open("powerup_template_h.txt");
      if (infile.fail())
        FATAL_ERROR("Failed to open template file: powerup_template_h.txt");

      powerupFileName = "Powerup" + powerupName + ".h";
      outfile.open(powerupFileName);
      if (outfile.fail())
        FATAL_ERROR("Failed to create output file: " + powerupFileName);
    }
    else
    {
      infile.open("powerup_template_cpp.txt");
      if (infile.fail())
        FATAL_ERROR("Failed to open template file: powerup_template_cpp.txt");

      powerupFileName = "Powerup" + powerupName + ".cpp";
      outfile.open(powerupFileName);
      if (outfile.fail())
        FATAL_ERROR("Failed to create output file: " + powerupFileName);
    }

    // read from the template file into the output file and replace the tokens
    while (!infile.eof())
    {
      // copy whitespace to output
      while (isspace(infile.peek()))
        outfile << (char)infile.get();

      // extract a phrase
      infile >> phrase;

      // replace tokens
      ReplaceToken(phrase, authorToken, author);
      ReplaceToken(phrase, powerupNameToken, powerupName);
      ReplaceToken(phrase, powerupUppercaseNameToken, powerupUppercaseName);
      ReplaceToken(phrase, powerupLowercaseNameToken, powerupLowercaseName);
      ReplaceToken(phrase, powerupSpacedNameToken, powerupSpacedName);
      ReplaceToken(phrase, powerupDescToken, powerupDesc);

      // copy phrase to output
      outfile << phrase;
    }

    // close output file
    outfile.close();

    // close template file
    infile.close();
  }
  
  cout << "Press Enter to exit..." << endl;
  getline(cin, phrase);
}

string StringToUpper(string strToConvert)
{
  for (size_t i = 0; i < strToConvert.length(); ++i)
    strToConvert[i] = toupper(strToConvert[i]);

  return strToConvert;
}

string StringToLower(string strToConvert)
{
  for (size_t i = 0; i < strToConvert.length(); ++i)
    strToConvert[i] = tolower(strToConvert[i]);

  return strToConvert;
}

string GetPowerupSpacedName(string powerupName)
{
  // Find the 2nd uppercase character
  size_t i;
  for (i = 1; i < powerupName.length(); ++i)
  {
    if (powerupName[i] >= 'A' && powerupName[i] <= 'Z')
      break;
  }

  if (i == powerupName.length())
    return powerupName;

  string spacedName = powerupName.substr(0, i);
  
  size_t start = i;
  ++i;

  for (; i < powerupName.length(); ++i)
  {
    if (powerupName[i] >= 'A' && powerupName[i] <= 'Z')
    {
      spacedName += " " + powerupName.substr(start, i - start);
      start = i;
    }
  }

  return spacedName += " " + powerupName.substr(start, i - start);
}

/// If token is contained in phrase, replaces token with value.
void ReplaceToken(string &phrase, string &token, string &value)
{
  // find token
  size_t index = phrase.find(token);

  if (index == string::npos)
    return; // not found

  // replace token
  phrase.replace(index, token.size(), value);
}