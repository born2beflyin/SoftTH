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

#ifndef __OUTD3D9_H__
#define __OUTD3D9_H__

#include <d3d9.h>
#include <windows.h>
#include "shcopy.h"
#include "helper.h"
#include "main.h"

//#define INDIRECTCOPY

class outDirect3D9
{
public:
  outDirect3D9(int devID, int wantMethod, int w, int h, int transX, int transY, RECT *destRectWanted, HWND primaryFocusWindow, bool fpuPreserve, int logoShowTime, bool wantIndirect);
  ~outDirect3D9();

  void DoCopy(bool doWait, bool useB);
  void Present();
  void PresentOff();  // Display logo (output turned off)
  HANDLE GetShareHandleA() {return shareHandleA;};
  HANDLE GetShareHandleB() {return shareHandleB;};
  bool WaitForTask(bool doWait);
  bool isReadyForData();

  void setLogoShowTime(int time) {logoTime = time;};

  int getWidth() {return bbWidth;};
  int getHeight() {return bbHeight;};
  int getBufWidth() {return transWidth;};
  int getBufHeight() {return transHeight;};
  D3DFORMAT getFormat() {return bbFormat;};
  int getFPS() {return FPS;};
  void minimize();

  HWND getWindow() {return outWin;};
  void beginThread();

  void SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP *pRamp);

private:
  IDirect3D9Ex *d3d;
  IDirect3DDevice9Ex *dev;
  IDirect3DDevice9Ex *devSec;

  D3DPRESENT_PARAMETERS pp;
  IDirect3DSurface9 *bb;     // Device backbuffer

  int copyMethod; // One of OUTMETHOD_x
  HWND outWin;

  RECT destRect;

  int bbWidth, bbHeight;        // Backbuffer size
  int transWidth, transHeight;  // Transport resolution size
  D3DFORMAT bbFormat;

  bool useIndirect; // Use indirect frame copying (AMD cards, NVIDIA handles direct read fine)

  IDirect3DSurface9 *shareSurfA, *shareSurfB;    // Shared surface for this device
  IDirect3DSurface9 *surfPri;
//#ifdef INDIRECTCOPY
  IDirect3DSurface9 *surfPriSys;
//#endif
  IDirect3DTexture9 *texSec;       // Texture on devSec if not using shared surface directly, used for packed RGB copy
  IDirect3DSurface9 *surfSec;      // Surface on devSec if not using shared surface directly 
  HANDLE shareHandleA;              // Handle of shared surface A
  HANDLE shareHandleB;              // Handle of shared surface B

  bool frameReady;
  bool threadActive;
  bool threadDoEnd;
  IDirect3DSurface9 *threadSourceSurface;
  D3DLOCKED_RECT threadSourceRect;
  HANDLE threadGo;  // Signal process thread to start
  HANDLE threadWorking;

  HMONITOR mId;
  MONITORINFO mInfo;

  int FPS;
  int fCounter;
  int fTimer;
  void countFPS();

  GAMMARAMP *defaultGammaRamp;
  bool didSetGammaRamp;
  /*bool didSetGammaRamp;
  WORD defaultGammaRamp[256*3];*/

  // Thread tasks
  void threadTaskNonlocal();  // For OUTMETHOD_NONLOCAL
  void threadTaskBlit();      // For OUTMETHOD_BLIT  
  void threadTaskCUDA();      // For OUTMETHOD_CUDA

  // LOGO =D
  IDirect3DTexture9 *logo;
  void drawLogo(IDirect3DDevice9Ex *ldev, bool nofade=false);
  int logoTime; // Time at which logo is hidden

  IDirect3DQuery9 *q;     // smoothing query for dev
  IDirect3DQuery9 *qsec;  // smoothing query for devSec

#ifdef USE_NEW_CUDA
  int cudaDstThreadId; // Cuda destination copy thread ID
  int cudaSrcAThreadId; // Cuda source copy thread ID
  int cudaSrcBThreadId; // Cuda source copy thread ID
#endif

  shCopy *copyUnpack24to32; // shader copy effect, unpacks RGB data from RGBA surface
};

#endif