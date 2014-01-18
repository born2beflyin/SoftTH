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

#ifndef __WIN32_H__
#define __WIN32_H__

#include <windows.h>
#include "configFile.h"

typedef struct {
  int x, y;
  int width, height;
  HWND hWnd;
  DWORD flags;
  bool show;
  HWND parent;
} WINDOWPARAMS;

void __cdecl windowHandler(void* a);
void bitsToWin(HDC hdc, int srcWidth, int srcHeight, int tgtWidth, int tgtHeight, void *pBits, int bpp);

#endif