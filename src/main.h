/*
SoftTH, Software multihead solution for Direct3D
Copyright (C) 2005-2012 Keijo Ruotsalainen, www.kegetys.fi

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MAIN_H_
#define _MAIN_H_

#define _BIND_TO_CURRENT_CRT_VERSION 0
#define _BIND_TO_CURRENT_VCLIBS_VERSION 0

#define DUMP_IMPORTS 0  // Dump all DLL imports

#define DEBUG_TIMESTAMPED

#include "version.h"
#include <list>

#include "configFile.h"

extern "C" __declspec(dllexport) configFile config; // Main configuration
//extern "C" __declspec(dllexport) configFile* pconfig; // external pointer to the main config file
extern bool emergencyRelease;

typedef struct {
  HWND hwnd;
  HDC hdc;
  WORD ramp[256*3];
} GAMMARAMP;
extern std::list<GAMMARAMP*> restoreGammaRamps;

#endif
