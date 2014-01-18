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

#include "overlay_interface.h"

#include "helper.h"

HINSTANCE oLib = NULL;

OVERLAY_INIT oInit = NULL;
OVERLAY_DEINIT oDeinit = NULL;
OVERLAY_DRAW oDraw = NULL;
OVERLAY_CLICK oClick = NULL;

void loadOverlay()
{
	if(oLib)
  {
		FreeLibrary(oLib);
  }

	oLib = LoadLibrary("SoftTH_overlay.dll");
	if(!oLib) {
		dbg("Overlay interface not found");
		return;
	}

	oInit = (OVERLAY_INIT) GetProcAddress((HMODULE)oLib, "overlayInit");
	oDeinit = (OVERLAY_DEINIT) GetProcAddress((HMODULE)oLib, "overlayDeinit");
	oDraw = (OVERLAY_DRAW) GetProcAddress((HMODULE)oLib, "overlayDraw");
  oClick = (OVERLAY_CLICK) GetProcAddress((HMODULE)oLib, "overlayClick");
}

void initOverlay(OVERLAY_INIT_BLOCK *param)
{
  if(!oInit) return;
  oInit(param);
}

void deinitOverlay()
{
  if(!oDeinit) return;
  oDeinit();
}
void overlayDoDraw(OVERLAY_DRAW_BLOCK *param)
{
  if(!oDraw) return;
  oDraw(param);
}

void overlayDoClick(OVERLAY_CLICK_BLOCK *param)
{
  if(!oClick) return;
  oClick(param);
}