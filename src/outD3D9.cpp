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

// Direct3D 9 output

/* NONLOCAL procedure:
present: newbb -> localSurfA / localSurfB
doCopy: wait for worker thread
doCopy: localSurfA / localSurfB -> surfPri
Present: threadGo!
threadTaskNonlocal: surfPri->lockrect
threadTaskNonlocal: surfSec->lockrect
threadTaskNonlocal: memcpy
threadTaskNonlocal: unlockrect both
threadTaskNonlocal: surfSec -> bb
threadTaskNonlocal: present
*/

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#include "outD3D9.h"
#include "win32.h"
#include <process.h>
#include <vector>
#include <d3d9.h>
#include "configFile.h"
#include "main.h"

#include <d3dx9.h>

#include "cudainterface.h"
#include "cudaTasks.h"

#define USE_MT

#ifdef USE_NEW_CUDA
#include "cudaif.hpp"
cudaIf cudaInterface;
#endif


// starts outDirect3D9 thread
static void __cdecl trampoline(void* obj)
{
  outDirect3D9 *o = (outDirect3D9*) obj;
  o->beginThread();
}

outDirect3D9::outDirect3D9(int devID, int wantMethod, int w, int h, int transX, int transY, RECT *destRectWanted, HWND primaryFocusWindow, bool fpuPreserve, int logoStopTime, bool wantIndirect)
{
  dev = NULL;
  d3d = NULL;
  devSec = NULL;
  shareHandleA = shareHandleB = NULL;
  surfSec = surfPri = NULL;
//#ifdef INDIRECTCOPY
  surfPriSys = NULL;
//#endif
  texSec = NULL;
  copyMethod = wantMethod;
  logo = NULL;
  logoTime = logoStopTime;

  useIndirect = wantIndirect;

  q = qsec = NULL;

  if(destRectWanted) {
    destRect = *destRectWanted;
  } else {
    destRect.left = destRect.right = destRect.bottom = destRect.top = 0;
  }

  transWidth = transX;
  transHeight = transY;
  fCounter = FPS = fTimer = 0;

  threadSourceSurface = NULL;

  threadActive = false;
  threadDoEnd = false;
  threadWorking = NULL;
  copyUnpack24to32 = NULL;

  bbWidth = w;
  bbHeight = h;

  switch(copyMethod) {
    case OUTMETHOD_NONLOCAL_16bDither: bbFormat = D3DFMT_R5G6B5; break;
    case OUTMETHOD_NONLOCAL_16b: bbFormat = D3DFMT_R5G6B5; break;
    case OUTMETHOD_NONLOCAL_24b: bbFormat = D3DFMT_A8R8G8B8; break;
    default: bbFormat = D3DFMT_X8R8G8B8; break;
  }

  D3DCALL( Direct3DCreate9Ex(-D3D_SDK_VERSION, &d3d) );

  //int wflags = WS_EX_TOPMOST;
  int wflags = NULL;

  mInfo.cbSize = sizeof(MONITORINFO);
  mId = d3d->GetAdapterMonitor(devID);
  GetMonitorInfo(mId, &mInfo);

  // Get our head
  HEAD *head;
  int numDevs = config.getNumAdditionalHeads();
  for(int i=0;i<numDevs;i++) {
    HEAD *h = config.getHead(i);
    if(h->devID == devID)
      head = h;
  }

  // Set resolution to monitor size
  bbWidth = mInfo.rcMonitor.right - mInfo.rcMonitor.left;
  bbHeight = mInfo.rcMonitor.bottom - mInfo.rcMonitor.top;

  if(!head->screenMode.x)
    head->screenMode.x = bbWidth;
  if(!head->screenMode.y)
    head->screenMode.y = bbHeight;

  // Create output window
  WINDOWPARAMS wp = {mInfo.rcMonitor.left, mInfo.rcMonitor.top, bbWidth, bbHeight, NULL, wflags, true, primaryFocusWindow};

  wp.hWnd = NULL;
  _beginthread(windowHandler, 0, (void*) &wp);
  while(!wp.hWnd)
    Sleep(10);
  outWin = wp.hWnd;

  head->hwnd = outWin;

  ShowWindow(outWin, SW_SHOWNA);
  //UpdateWindow(hwnd);

  // Store current gamma ramp to reset it if changed
  defaultGammaRamp = new GAMMARAMP;
  defaultGammaRamp->hwnd = outWin;
  defaultGammaRamp->hdc = GetDC(outWin);
  GetDeviceGammaRamp(defaultGammaRamp->hdc, (void*) &defaultGammaRamp->ramp);
  restoreGammaRamps.push_back(defaultGammaRamp);
  didSetGammaRamp = false;

  bool local = copyMethod==OUTMETHOD_LOCAL;
  bool fullscreen = false;

  // Create device
  pp.BackBufferWidth = local?0:32;
  pp.BackBufferHeight = local?0:32;
  pp.BackBufferFormat = bbFormat;
  pp.BackBufferCount = 0;
  pp.MultiSampleType = D3DMULTISAMPLE_NONE;
  pp.MultiSampleQuality = 0;
  pp.SwapEffect = D3DSWAPEFFECT_DISCARD;  // FLIPEX
  //pp.SwapEffect = D3DSWAPEFFECT_FLIPEX;
  pp.hDeviceWindow = local?outWin:primaryFocusWindow;
  pp.Windowed  = !fullscreen;
  pp.EnableAutoDepthStencil = false;
  pp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
  pp.Flags = 0;
  pp.FullScreen_RefreshRateInHz = 0;
  pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //D3DPRESENT_FORCEIMMEDIATE

  D3DDISPLAYMODEEX mode = {
    sizeof(D3DDISPLAYMODEEX),
    pp.BackBufferWidth,  pp.BackBufferHeight,
    pp.FullScreen_RefreshRateInHz, pp.BackBufferFormat,
    D3DSCANLINEORDERING_PROGRESSIVE
  };

  DWORD flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
  //if(!local)
    flags |= D3DCREATE_MULTITHREADED;
  if(fpuPreserve)
    flags |= D3DCREATE_FPU_PRESERVE;
  D3DCALL( d3d->CreateDeviceEx(local?devID:0, D3DDEVTYPE_HAL, pp.hDeviceWindow, flags, &pp, fullscreen?&mode:NULL, &dev) );

  dev->SetMaximumFrameLatency(1);

  if(copyMethod == OUTMETHOD_NONLOCAL_24b)
  {
    // RGB data packed to RGBA surface - real texture size is 3/4 the width
    transWidth = (int) ((float)transWidth*(3.0/4.0));
  }

  if(transWidth > bbWidth || transHeight > bbHeight) {
    transWidth = min(transWidth, bbWidth);
    transHeight = min(transHeight, bbHeight);
    dbg("WARNING: Transport resolution higher than output resolution", devID);
    dbg("         Clamping transport resolution to %dx%d", transWidth, transHeight);
  }

  // Create shared surface
  D3DCALL( dev->CreateRenderTargetEx(transWidth, transHeight, bbFormat, D3DMULTISAMPLE_NONE, 0, false, &shareSurfA, &shareHandleA, NULL) );
  D3DCALL( dev->CreateRenderTargetEx(transWidth, transHeight, bbFormat, D3DMULTISAMPLE_NONE, 0, false, &shareSurfB, &shareHandleB, NULL) );
  if(!shareHandleB || !shareHandleA)
    dbg("WARNING: NULL share handle!");


  if(copyMethod == OUTMETHOD_CUDA)
  {
#ifdef USE_NEW_CUDA
    cudaSrcAThreadId = cudaInterface.createThread(dev, shareSurfA);
    cudaSrcBThreadId = cudaInterface.createThread(dev, shareSurfB);
#endif
  }


  if(copyMethod == OUTMETHOD_LOCAL)
  {
    // Get backbuffer
    D3DCALL( dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bb) );

    if(config.main.smoothing && false) // TODO: should this be enabled?
    {
      D3DCALL( dev->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &q) );
    }
  }
  else
  {

    // Create intermediate copy buffer
    D3DCALL( dev->CreateRenderTargetEx(transWidth, transHeight, bbFormat, D3DMULTISAMPLE_NONE, 0, true, &surfPri, NULL, NULL) );
//#ifdef INDIRECTCOPY
    if(useIndirect)
    {
      D3DCALL( dev->CreateOffscreenPlainSurface(transWidth, transHeight, bbFormat, D3DPOOL_SYSTEMMEM, &surfPriSys, NULL) );
    }
//#endif
/*
  if(copyMethod == OUTMETHOD_CUDA)
  {
#ifdef USE_NEW_CUDA
    cudaSrcAThreadId = cudaInterface.createThread(dev, surfPri);
#endif
  }
*/
    // Start handler thread
    threadGo = CreateEvent(NULL, FALSE, FALSE, NULL);
    threadWorking = CreateEvent(NULL, FALSE, FALSE, NULL);
    _beginthread(trampoline, 0, this);

    if(copyMethodNonlocal(copyMethod) || copyMethod == OUTMETHOD_CUDA)
    {
      pp.BackBufferWidth = bbWidth;
      pp.BackBufferHeight = bbHeight;
      pp.hDeviceWindow = outWin;

      pp.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

      D3DCALL( d3d->CreateDeviceEx(devID, D3DDEVTYPE_HAL, outWin, flags, &pp, NULL, &devSec) );
      D3DCALL( devSec->CreateRenderTargetEx(transWidth, transHeight, bbFormat, D3DMULTISAMPLE_NONE, 0, true, &surfSec, NULL, NULL) );
      devSec->SetMaximumFrameLatency(1);

      // Get backbuffer
      D3DCALL( devSec->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bb) );
/*
      if(copyMethod == OUTMETHOD_CUDA)
      {
#ifdef USE_NEW_CUDA
        cudaDstThreadId = cudaInterface.createThread(devSec, surfSec);
#endif
      }
*/
      if(config.main.smoothing  && false) // TODO: should this be enabled?
      {
        D3DCALL( dev->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &q) );
        D3DCALL( devSec->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &qsec) );
      }

      if(copyMethod == OUTMETHOD_NONLOCAL_24b)
      {
        // Create unpacker shcopy
        copyUnpack24to32 = new shCopy(devSec, "D:\\dev\\data\\unpack.psh");
        D3DCALL( devSec->CreateTexture(transWidth, transHeight, 1, D3DUSAGE_RENDERTARGET, bbFormat, D3DPOOL_DEFAULT, &texSec, NULL) );
      }
    }
  }

}

void outDirect3D9::threadTaskNonlocal()
{
  long long x = 0;

  if(q) {
    // Wait for pipelining
    q->Issue(D3DISSUE_END);
    while(q->GetData((void *)&x, sizeof(long long), D3DGETDATA_FLUSH) == S_FALSE) SwitchToThread();
  }

  if(qsec)
    qsec->Issue(D3DISSUE_END);

//#ifdef INDIRECTCOPY
  if(useIndirect)
  {
    DWORD tbb = GetTickCount();

    /*static IDirect3DQuery9 *qt = NULL;
    if(!qt)
    {
      D3DCALL( dev->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &qt) );
    }*/


	  HANDLE thread = GetCurrentThread();

    SetThreadPriority(thread, THREAD_PRIORITY_BELOW_NORMAL);

    /*if(qt)
    {
      qt->Issue(D3DISSUE_END);
      while(qt->GetData((void *)&x, sizeof(long long), D3DGETDATA_FLUSH) == S_FALSE) SwitchToThread();
    }*/

    D3DCALL( dev->GetRenderTargetData(surfPri, surfPriSys) );
    SetThreadPriority(thread, THREAD_PRIORITY_HIGHEST);
    timeWarn(tbb, 250, "threadTaskNonlocal: GetRenderTargetData");
  }
//#endif

  // Copy from testSurf to surfSec
  D3DLOCKED_RECT rs, rd;
  DWORD tb = GetTickCount();

  IDirect3DSurface9 *source = NULL;
  IDirect3DSurface9 *dest = NULL;

  if(useIndirect)
  {
    // Read from GetRenderTargetData surface
    source = surfPriSys;
    dest = surfSec;
  }
  else
  {
    // Read directly from surface
    source = surfPri;
    dest = surfSec;
  }
/*
#ifdef INDIRECTCOPY
  IDirect3DSurface9 *source = surfPriSys;
  IDirect3DSurface9 *dest = surfSec;
#else
  IDirect3DSurface9 *source = surfPri;
  IDirect3DSurface9 *dest = surfSec;

  /*static bool foxo = false;
  foxo = !foxo;
  IDirect3DSurface9 *source = foxo?surfPri:surfPriB;
  IDirect3DSurface9 *dest = foxo?surfSec:surfSecB;*
#endif
  */

  D3DCALL( source->LockRect(&rs, NULL, D3DLOCK_READONLY) );
  DWORD spitch = rs.Pitch;
  DWORD ssize = transHeight*rs.Pitch;

// Copy to temporary buffer, then signal threadWorking as finished immediately
// Doesnt help :(
//#define USEBUF

#ifdef USEBUF
  char *buf = new char[ssize];
  memcpy(buf, rs.pBits, ssize);
  source->UnlockRect();
  ResetEvent(threadWorking);
#endif

  timeWarn(tb, 250, "threadTaskNonlocal: source lock");

  if(qsec)
    while(qsec->GetData((void *)&x, sizeof(long long), D3DGETDATA_FLUSH) == S_FALSE) SwitchToThread();

  bool usePaska = false;

  if(!usePaska)
  {
    tb = GetTickCount();
    D3DCALL( dest->LockRect(&rd, NULL, D3DLOCK_DISCARD) );
    timeWarn(tb, 250, "threadTaskNonlocal: destination lock");

    tb = GetTickCount();
#ifdef USEBUF
    memcpyPitched(rd.pBits, buf, ssize, rd.Pitch, spitch);
#else
    memcpyPitched(rd.pBits, rs.pBits, ssize, rd.Pitch, spitch);
#endif
    timeWarn(tb, 250, "threadTaskNonlocal: memcpyPitched");

    tb = GetTickCount();
#ifdef USEBUF
    delete[] buf;
#else
    source->UnlockRect();
#endif
    dest->UnlockRect();
    timeWarn(tb, 250, "threadTaskNonlocal: unlock");

    // Copy from surfSec to backbuffer, then present
    if(copyMethod == OUTMETHOD_NONLOCAL_24b) {
      // Unpack RGB data
      IDirect3DSurface9 *s;
      texSec->GetSurfaceLevel(0, &s);
      D3DCALL( devSec->StretchRect(dest, 0, s, 0, D3DTEXF_LINEAR) );
      s->Release();
      copyUnpack24to32->surfCopyShader(texSec, bb);
    } else {
      D3DCALL( devSec->StretchRect(dest, 0, bb, 0, D3DTEXF_LINEAR) );
    }

  } else {
    // Experimental: Vista createtexture from sysmem pointer
    // Doesn't help at all
    static IDirect3DTexture9 *texDevTemp = NULL;
    ONCE {
      D3DCALL( devSec->CreateTexture(transWidth, transHeight, 1, NULL, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texDevTemp, NULL) );
    }

    IDirect3DTexture9 *texMemTemp = NULL;
    HANDLE *dptr = (HANDLE*)&rs.pBits;
    D3DCALL( devSec->CreateTexture(transWidth, transHeight, 1, NULL, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &texMemTemp, dptr) );
    D3DCALL( devSec->UpdateTexture(texMemTemp, texDevTemp) );
    texMemTemp->Release();
    source->UnlockRect();

    IDirect3DSurface9 *surff;
    texDevTemp->GetSurfaceLevel(0, &surff);
    D3DCALL( devSec->StretchRect(surff, NULL, bb, NULL, D3DTEXF_POINT) );
  }

  tb = GetTickCount();
  drawLogo((IDirect3DDevice9Ex*)devSec);
  timeWarn(tb, 250, "threadTaskNonlocal: draw A");

  // Need to pretend we draw to backbuffer, otherwise no filtering will occur with nvidia?
  tb = GetTickCount();
  devSec->BeginScene();
  static float foo[4] = {1,2,3,4};
  devSec->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 1, foo, 4);
  devSec->EndScene();
  timeWarn(tb, 250, "threadTaskNonlocal: draw B");

  tb = GetTickCount();
  D3DCALL( devSec->Present(NULL, isNullRect(&destRect), NULL, NULL) );
  timeWarn(tb, 250, "threadTaskNonlocal: present");

}


void outDirect3D9::threadTaskBlit()
{
  // Blit from surfPri to window
  D3DLOCKED_RECT rs;
  D3DCALL( surfPri->LockRect(&rs, NULL, D3DLOCK_READONLY) );

  HDC hdc = GetDC(outWin);
  bitsToWin(hdc, rs.Pitch/4, transHeight, bbWidth, bbHeight, rs.pBits, 32);
  //ReleaseDC(outWin);

  surfPri->UnlockRect();
}

#ifdef USE_CUDA
directCuda *cudaVmToSys = NULL;
#endif

void outDirect3D9::threadTaskCUDA()
{
#ifdef USE_CUDA
  if(!cudaVmToSys) {
	  cudaVmToSys = new directCuda(d3d, dev, "cudaVmToSys");
	  cudaVmToSys->addSurface(surfPri);
	  cudaVmToSys->beginThread();
  }

	cudaVmToSys->setTask(cudaTaskVMtoSM, NULL);
	void *cudaDataPtr = cudaVmToSys->taskExecute();
  if(cudaDataPtr) {
    HDC hdc = GetDC(outWin);
    bitsToWin(hdc, transWidth, transHeight, bbWidth, bbHeight, cudaDataPtr, 32);
  }
#endif

#ifdef USE_NEW_CUDA
  //cudaInterface.copyPeer(cudaDstThreadId, cudaSrcAThreadId);

  D3DCALL( devSec->StretchRect(dest, 0, bb, 0, D3DTEXF_LINEAR) );
  D3DCALL( devSec->Present(NULL, isNullRect(&destRect), NULL, NULL) );
#endif
}

// Worker for non-local/blit present
void outDirect3D9::beginThread()
{
  dbg("outDirect3D9::beginThread: START");
  threadActive = true;

  while(!threadDoEnd)
  {
    //SetProcessAffinityMask(GetCurrentProcess(), 0xff);
    //SetThreadAffinityMask(GetCurrentThread(), 0xff);

    // Wait for task
    ResetEvent(threadWorking);
    SignalObjectAndWait(threadWorking, threadGo, INFINITE, FALSE);
    if(threadDoEnd)
      break;

    // Do task
    if(copyMethod == OUTMETHOD_CUDA)
      threadTaskCUDA();
    else if(copyMethodNonlocal(copyMethod))
      threadTaskNonlocal();
    else if(copyMethod == OUTMETHOD_BLIT)
      threadTaskBlit();
    countFPS();
  }
  threadActive = false;
  dbg("outDirect3D9::beginThread: END");
}

void outDirect3D9::countFPS()
{
  // FPS counter
  fCounter++;
  if(GetTickCount() - fTimer >= 1000)
  {
    FPS = fCounter;
    fCounter = 0;
    fTimer = GetTickCount();
  }
}

bool outDirect3D9::isReadyForData()
{
  return frameReady;
}

bool outDirect3D9::WaitForTask(bool doWait)
{
  if(doWait) {
    // Synchronized - wait no matter how long it takes
    //WaitForSingleObject(threadWorking, INFINITE);
    if(WaitForSingleObject(threadWorking, 10000) == WAIT_TIMEOUT)
      dbg("outDirect3D9::WaitForTask: ERROR: WaitForSingleObject timeout reached!");
    return true;
  } else {
    // Just get signal state
    if(WaitForSingleObject(threadWorking, 1) == WAIT_TIMEOUT)
      return false;
    else
      return true;
  }
}

// Copy buffers
void outDirect3D9::DoCopy(bool doWait, bool useB)
{
  frameReady = false;

  if(copyMethod == OUTMETHOD_LOCAL)
  {
    // Just copy from shared surface to backbuffer - they reside on same device
    D3DCALL( dev->StretchRect(useB?shareSurfB:shareSurfA, 0, bb, 0, D3DTEXF_LINEAR) );

    frameReady = true;

  } else {
    // Ask handler thread to copy
    if(!WaitForTask(doWait))
      return;

    DWORD tb = GetTickCount();
    D3DCALL( dev->StretchRect(useB?shareSurfB:shareSurfA, 0, surfPri, 0, D3DTEXF_NONE) );

    //D3DCALL( dev->StretchRect(useB?shareSurfA:shareSurfB, NULL, useB?surfPriB:surfPri, NULL, D3DTEXF_NONE) );
    //cudaInterface.copyPeer(cudaDstThreadId, useB?cudaSrcAThreadId:cudaSrcBThreadId);

    timeWarn(tb, 250, "DoCopy nonlocal StretchRect");
    frameReady = true;
  }
}

void outDirect3D9::minimize()
{
  //SetWindowPos(outWin, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOSIZE);
  ShowWindow(outWin, SW_MINIMIZE);
}
// Present buffer to screen
void outDirect3D9::Present()
{
  if(!frameReady)
  {
    return;
  }
/*
  // check for minimized window
  WINDOWPLACEMENT wpl;
  GetWindowPlacement(outWin, &wpl);
  if(wpl.showCmd == SW_SHOWMINIMIZED)
  {
    dbg("outDirect3D9: Restoring window");
    ShowWindow(outWin, SW_SHOWNOACTIVATE);
    Sleep(100);
  }

  // Verify window is positioned corretly on monitor
  GetMonitorInfo(mId, &mInfo);
  if(wpl.rcNormalPosition.left != mInfo.rcMonitor.left || wpl.rcNormalPosition.right != mInfo.rcMonitor.right ||
     wpl.rcNormalPosition.top != mInfo.rcMonitor.top || wpl.rcNormalPosition.bottom != mInfo.rcMonitor.bottom)
  {
    dbg("outDirect3D9: Repositioning window");
    MoveWindow(outWin, mInfo.rcMonitor.left, mInfo.rcMonitor.top, mInfo.rcMonitor.right-mInfo.rcMonitor.left, mInfo.rcMonitor.bottom-mInfo.rcMonitor.top, false);
    //SetWindowPos(outWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOREDRAW);
  }

  // Make sure window is on top
  SetWindowPos(outWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOREDRAW|SWP_NOSIZE);
*/

  if(copyMethod == OUTMETHOD_LOCAL) {
    drawLogo(dev);

    if(q) {
      // Wait for pipelining
      long long x;
      q->Issue(D3DISSUE_END);
      while(q->GetData((void *)&x, sizeof(long long), D3DGETDATA_FLUSH) == S_FALSE) SwitchToThread();
    }

    D3DCALL( dev->Present(NULL, isNullRect(&destRect), NULL, NULL) );
    countFPS();
  } else {
    // Start worker task
    SetEvent(threadGo);
  }
}

void outDirect3D9::SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP *pRamp)
{
  dbgf("outDirect3D9::SetGammaRamp");
  HDC hdc = GetDC(outWin);
  SetDeviceGammaRamp(hdc, (void*)pRamp);
  didSetGammaRamp = true;
  ReleaseDC(outWin, hdc);
  //(devSec?devSec:dev)->SetGammaRamp(0, Flags, pRamp);
}

struct LVERTEX {
    D3DXVECTOR4 p;
    DWORD       color;
    FLOAT       tu, tv;
};
#define D3DFVF_LVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

void outDirect3D9::PresentOff()
{
  if(copyMethod == OUTMETHOD_LOCAL) {
    //if(GetKeyState('O') >= 0)
    drawLogo(dev, true);
    D3DCALL( dev->Present(NULL, NULL, NULL, NULL) );
    //D3DCALL( dev->PresentEx(NULL, NULL, NULL, NULL, D3DPRESENT_LINEAR_CONTENT) );
  }
  if(copyMethodNonlocal(copyMethod)) {
    drawLogo((IDirect3DDevice9Ex*)devSec, true);
    D3DCALL( devSec->Present(NULL, NULL, NULL, NULL) );
  }
}

#include "logodata.h"
#include "zlib.h"
//#include <fstream> // added by CJR to get the logo file

void outDirect3D9::drawLogo(IDirect3DDevice9Ex *ldev, bool nofade)
{

  if(GetTickCount() > (DWORD)logoTime && !nofade) {
    // Hide logo
    /*if(logo) {
      logo->Release();
      logo = NULL;
    }*/
    return;
  }

  if(!logo)
  {
    /*
    D3DXCreateTextureFromFile(ldev, "D:/dev/projects/SoftTH3/logo_paras.dds", &logo);
    */
    const int MAXDATALEN = 2048*512*4; // Enough for everyone
    BYTE *logoData = new BYTE[MAXDATALEN];

    // Verify checksum
    DWORD sum = 0;
    for(int i=0;i<logoGzippedLen;i++)
      sum+=logoGzipped[i];

    // Decompress to memory, then load as texture
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = logoGzippedLen;
    strm.next_in = (Bytef*) logoGzipped;
    strm.next_out = (Bytef*) logoData;
    strm.avail_out = MAXDATALEN;
    inflateInit(&strm);
    inflate(&strm, Z_NO_FLUSH);
    int logoDLen = MAXDATALEN-strm.avail_out;
    inflateEnd(&strm);

	// This section added by CJR to get the logo file
	//std::ofstream myFile("logofile.bin", std::ios::binary|std::ios::out);
	//myFile.write((char *)logoData,logoDLen);
	//myFile.close();
	// End section added by CJR

    D3DXCreateTextureFromFileInMemoryEx(ldev, logoData, logoDLen, D3DX_DEFAULT, D3DX_DEFAULT, 1, NULL, D3DFMT_FROM_FILE, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_FILTER_BOX, 0xFF000000, NULL, NULL, &logo);
/*#endif*/
    delete[] logoData;
  }

  const float la = 2048.0f/512.0f; // Logo aspect

  float w = (float) bbWidth;
  float h = (float) bbHeight;
  /*if(isNullRect(&destRect)) {
    w = destRect.right - destRect.left;
    h = destRect.bottom - destRect.top;
  }*/

  float t = (float)(logoTime-GetTickCount())/500.0f;
  if(t>1) t=1;

  BYTE a = (BYTE)(255.0f*t);
  if(nofade) a = 255;
  float x = w;
  float y = h/3.0f*1.40f;
  float u = y+(w/la);
  LVERTEX vdata[] = {
    D3DXVECTOR4(x,y,0,0.1f), D3DCOLOR_ARGB(a,255,255,255), 1, 0,
    D3DXVECTOR4(x,u,0,0.1f), D3DCOLOR_ARGB(a,255,255,255), 1, 1,
    D3DXVECTOR4(0,y,0,0.1f), D3DCOLOR_ARGB(a,255,255,255), 0, 0,
    D3DXVECTOR4(0,u,0,0.1f), D3DCOLOR_ARGB(a,255,255,255), 0, 1,
  };

  BYTE aa = (BYTE)(210.0f*t);
  if(nofade) aa = 255;
  LVERTEX bdata[] = {
    D3DXVECTOR4(w,0,0,0.1f), D3DCOLOR_ARGB(aa,0,0,0), 1, 0,
    D3DXVECTOR4(w,h,0,0.1f), D3DCOLOR_ARGB(aa,0,0,0), 1, 1,
    D3DXVECTOR4(0,0,0,0.1f), D3DCOLOR_ARGB(aa,0,0,0), 0, 0,
    D3DXVECTOR4(0,h,0,0.1f), D3DCOLOR_ARGB(aa,0,0,0), 0, 1,
  };


  ldev->SetFVF(D3DFVF_LVERTEX);
  ldev->SetRenderState(D3DRS_LIGHTING, TRUE);

  D3DLIGHT9 light0;
  ZeroMemory( &light0, sizeof(D3DLIGHT9) );
  light0.Type = D3DLIGHT_DIRECTIONAL;
  light0.Direction = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
  light0.Diffuse.r = 1.0f;
  light0.Diffuse.g = 1.0f;
  light0.Diffuse.b = 1.0f;
  light0.Diffuse.a = 1.0f;
  ldev->SetLight(0, &light0);
  ldev->LightEnable(0, TRUE);

  ldev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  ldev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  ldev->SetRenderState(D3DRS_ZENABLE, FALSE);
	ldev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	ldev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  ldev->SetRenderState(D3DRS_SRGBWRITEENABLE, 0);

  ldev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
  ldev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
  ldev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
  ldev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

  ldev->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, 0);
	ldev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	ldev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	ldev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	ldev->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 1);

  ldev->BeginScene();
  ldev->SetTexture(0, 0);
  ldev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, bdata, sizeof(LVERTEX));
  ldev->SetTexture(0, logo);
  ldev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vdata, sizeof(LVERTEX));
  ldev->SetTexture(0, 0);
  ldev->EndScene();
}

outDirect3D9::~outDirect3D9()
{
  // Stop the handler thread
  if(threadActive) {
    threadDoEnd = true;
    SetEvent(threadGo);
    while(threadActive)
      Sleep(1);
  }

  dbg("Free outDirect3D9");
  SAFE_RELEASE_LAST(bb);
  SAFE_RELEASE_LAST(surfPri);
//#ifdef INDIRECTCOPY
  SAFE_RELEASE_LAST(surfPriSys);
//#endif
  SAFE_RELEASE_LAST(surfSec);
  SAFE_RELEASE_LAST(texSec);
  SAFE_RELEASE_LAST(shareSurfA);
  SAFE_RELEASE_LAST(shareSurfB);

  SAFE_RELEASE_LAST(q);
  SAFE_RELEASE_LAST(qsec);

  SAFE_RELEASE_LAST(logo);

  delete copyUnpack24to32;
  copyUnpack24to32 = NULL;

  SAFE_RELEASE_LAST(dev);
  SAFE_RELEASE_LAST(devSec);
  SAFE_RELEASE_LAST(d3d);

  if(threadGo)
    CloseHandle(threadGo);
  if(threadWorking)
    CloseHandle(threadWorking);

  if(didSetGammaRamp) {
    // Restore gamma ramp
    if(!SetDeviceGammaRamp(defaultGammaRamp->hdc, (void*) &defaultGammaRamp->ramp))
      dbg("outDirect3D9: Restoring gamma ramp failed!");
    didSetGammaRamp = false;
    /*
    dbg("Restoring gamma ramp");
    HDC hdc = GetDC(outWin);
    SetDeviceGammaRamp(hdc, (void*) &defaultGammaRamp);
    ReleaseDC(outWin, hdc);
    didSetGammaRamp = false;
    */
  }
  // Store current gamma ramp to reset it if changed
  if(defaultGammaRamp) {
    // Delete gammaramp from release list and free it
    restoreGammaRamps.remove(defaultGammaRamp);
    ReleaseDC(defaultGammaRamp->hwnd, defaultGammaRamp->hdc);
    delete defaultGammaRamp;
    defaultGammaRamp = NULL;
  }

  SendMessage(outWin, WM_CLOSE, 0, 0);
}
