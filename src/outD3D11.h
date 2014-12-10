/*
SoftTH, Software multihead solution for Direct3D
Copyright (C) 2014 C. Justin Ratcliff, www.softth.net

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

#ifndef __OUTD3D11_H__
#define __OUTD3D11_H__

#include <d3d11.h>
#include <windows.h>
#include "helper.h"

class outDirect3D11
{
public:
  outDirect3D11(int devID, int w, int h, int transX, int transY, HWND primaryFocusWindow);
  ~outDirect3D11();

  //void presentFromBuffer();
  void present();
  HANDLE GetShareHandle() {return shareHandle;};

private:
  IDXGIFactory *dxgf;

  HWND outWin;
  IDXGISwapChain *swapChain;
  ID3D11Device *dev;
  D3D_FEATURE_LEVEL featureLevel;
  ID3D11DeviceContext *devContext;

  HMONITOR mId;
  MONITORINFO mInfo;

  int bbWidth, bbHeight;        // Backbuffer size
  int transWidth, transHeight;  // Transport resolution size

  ID3D11Texture2D *bb;  // Device backbuffer
  ID3D11Texture2D *sharedSurface; // Shared render surface
  ID3D11RenderTargetView *rttView;

  HANDLE shareHandle;
};

#endif // __OUTD3D11_H__
