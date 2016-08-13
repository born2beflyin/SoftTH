/*
SoftTH, Software multihead solution for Direct3D
Copyright (C) 2005-2012 Keijo Ruotsalainen, www.kegetys.fi
              2014-     C. Justin Ratcliff, www.softth.net

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

#ifndef _D3D10MAIN_H_
#define _D3D10MAIN_H_

#include "configFile.h"

#define DLL __declspec(dllimport)

extern "C" DLL configFile config; // Main configuration

#include "module.h"

Module* SoftTHMod;
//Module* D3D10Mod;

HINSTANCE hLibSoftTH;
extern "C" DLL HINSTANCE hLibD3D10;

char libfn[256];

#endif
