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

#ifndef __OVERLAYIF_H__
#define __OVERLAYIF_H__

#include <d3d9.h>
#include "configFile.h"

#define OVERLAY_VERSION    3

typedef struct{
  int overlayVersion;

  // Version 1:
  IDirect3DDevice9Ex *dev;

} OVERLAY_INIT_BLOCK;

typedef struct{
  int overlayVersion;
  
  // Version 1
  IDirect3DDevice9Ex *dev; // D3D device
  IDirect3DSurface9 *newbb; // (fake) backbuffer to render to
  int width, height; // Buffer width & height
  
  HEAD *primaryHead;
  HEAD *extraHeads[32];
  int numHeads;

} OVERLAY_DRAW_BLOCK;

typedef struct {
  int overlayVersion;
  
  // Version 1
  int x, y;
  HWND appWindow;
  bool up; // Deprecated

  // Version 3:
  int event;
  LPARAM lparam;
  WPARAM wparam;
} OVERLAY_CLICK_BLOCK;

typedef void * (__stdcall*OVERLAY_INIT)(OVERLAY_INIT_BLOCK *params); // Initialize overlay
typedef void * (__stdcall*OVERLAY_DEINIT)(); // Deinitialize overlay
typedef void * (__stdcall*OVERLAY_DRAW)(OVERLAY_DRAW_BLOCK *params); // Draw overlay
typedef void * (__stdcall*OVERLAY_CLICK)(OVERLAY_CLICK_BLOCK *params); // Mouse click

void loadOverlay();
void initOverlay(OVERLAY_INIT_BLOCK *param);
void deinitOverlay();
void overlayDoDraw(OVERLAY_DRAW_BLOCK *param);
void overlayDoClick(OVERLAY_CLICK_BLOCK *param);

#endif