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

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#include "d3dSoftTH.h"
#include "d3dtexture.h"

#include <stdio.h>
#include "main.h"

#include "win32.h"
#include <process.h>

#include "InputHandler.h"
#include "d3dDrawing.h"

#include "time.h"
#include "Shlobj.h"
#include "Dwmapi.h"

#include "overlay_interface.h"

#include <INITGUID.H>
DEFINE_GUID(IID_IDirect3DDevice9SoftTH, 0xb18b10ce, 0x2649, 0x405a, 0x87, 0xf, 0x95, 0xf7, 0xaa, 0xbb, 0xcc, 0xdd);
DEFINE_GUID(IID_SoftTHInvalidRTT, 0x12345542, 0x2134, 0x4545, 0xff, 0xff, 0xba, 0xdf, 0x00, 0xdb, 0xFF, 0xFF);

#undef dbgf
#define dbgf if(0)
//#define dbgf dbg

#define YIELD_CPU  YieldProcessor()
// #define YIELD_CPU SwitchToThread()
// #define YIELD_CPU Sleep(1)

volatile int SoftTHActive = 0; // >0 if SoftTH is currently active and resolution is overridden
bool *SoftTHActiveSquashed = NULL; // Pointer to latest SoftTH device squash variable (TODO: horrible)

// New SoftTH device instace created
// Create our fake backbuffer etc.
IDirect3DDevice9SoftTH::IDirect3DDevice9SoftTH(IDirect3D9New *parentNew, IDirect3D9Ex *direct3D, HWND hFocusWindowNew, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pp)
:IDirect3DDevice9New(dev, direct3D)
{

  // Disable Vista desktop composition - this will give us a performance increase
  if(!config.main.keepComposition) {
    extern bool didDisableComposition;
    if(!didDisableComposition) {
      didDisableComposition = true;
      DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
      Sleep(100);
    }
  }

  parent = parentNew;
  memcpy(&lastPp, pp, sizeof(D3DPRESENT_PARAMETERS));
  hFocusWindow = hFocusWindowNew;
  copybuf = 0;
  lockableBB = squash = showGraph = nocopy = showLog = false;
  curSurfA = true;

  refs = messageTime = numDevs = 0;

  bbtex = NULL;
  ppaux = NULL;

  postprocess = false;

  newbb = NULL;
  copyPack24to32 = NULL;
  copyDither = NULL;

  SoftTHActiveSquashed = &squash;

  /*
  // Force fullscreen mode
  if(true) {
    if(pp->BackBufferFormat == D3DFMT_UNKNOWN)
      pp->BackBufferFormat = D3DFMT_A8R8G8B8;
    pp->Windowed = false;
  }
  */

  /*
  // Move window
  if(true)
  {
    SetWindowPos(pp->hDeviceWindow, HWND_TOPMOST, -1920, 0, 5760, 1200, SWP_SHOWWINDOW);
    pp->BackBufferWidth = config.main.renderResolution.x;
    pp->BackBufferHeight = config.main.renderResolution.y;
  }
  */

  if(config.overrides.forceResolution && !pp->Windowed) {
    dbg("Forcing device resolution from %dx%d to %dx%d", pp->BackBufferWidth, pp->BackBufferHeight, config.main.renderResolution.x, config.main.renderResolution.y);
    pp->BackBufferWidth = config.main.renderResolution.x;
    pp->BackBufferHeight = config.main.renderResolution.y;
  }

  wantedX = pp->BackBufferWidth;
  wantedY = pp->BackBufferHeight;
  fpuPreserve = (BehaviorFlags&D3DCREATE_FPU_PRESERVE)!=0;

  if(!validateSettings(d3d)) {
    dbg("SoftTH: validateSettings FAILED");
    createDeviceResult = D3DERR_INVALIDCALL;
    return;
  }

  dbg("Initializing SoftTH device (%dx%d) %s %s", pp->BackBufferWidth, pp->BackBufferHeight, getMode(pp->BackBufferFormat), pp->Windowed?"Windowed":"Fullscreen");
  dbg("BehaviorFlags: <%s>", getD3DCreate(BehaviorFlags));
  dbg("Swapeffect: %s (%d backbuffer(s))", getSwapEffect(pp->SwapEffect), pp->BackBufferCount);

  detectTransportMethods();
  timeBeginPeriod(1);

  if(isSoftTHmode(pp->BackBufferWidth, pp->BackBufferHeight) && !pp->Windowed)
  {
    dbg("Multihead mode %dx%d detected", pp->BackBufferWidth, pp->BackBufferHeight);

    loadOverlay();

    D3DPRESENT_PARAMETERS newpp = *pp;
    adjustPP(&newpp);

    // Create device
    bool fullscreen = pp->Windowed==0;
    D3DDISPLAYMODEEX mode = {
      sizeof(D3DDISPLAYMODEEX),
      newpp.BackBufferWidth,  newpp.BackBufferHeight,
      newpp.FullScreen_RefreshRateInHz, newpp.BackBufferFormat,
      D3DSCANLINEORDERING_PROGRESSIVE
    };

    char name[256];
    DWORD pid;
    GetWindowText(hFocusWindow, name, 256);
    GetWindowThreadProcessId(hFocusWindow, &pid);
    dbg("Focus window 0x%08X <%s>, thread 0x%08X", hFocusWindow, name, pid);
    ihGlobal.hookRemoteThread(pid);
    GetWindowText(pp->hDeviceWindow, name, 256);
    GetWindowThreadProcessId(pp->hDeviceWindow, &pid);
    dbg("Device window 0x%08X <%s>, thread 0x%08X", pp->hDeviceWindow, name, pid);
    ihGlobal.hookRemoteThread(pid);

    RECT r;
    GetWindowRect(hFocusWindow, &r);
    dbg("Device window  pre-create: %s", strRect(&r));

    HRESULT ret = direct3D->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hFocusWindow, BehaviorFlags, &newpp, !fullscreen?NULL:&mode, &dev);
    createDeviceResult = ret;
    if(ret != D3D_OK) {
      dbg("SoftTH: CreateDeviceEx FAILED!");
      return;
    }

    /*dev->SetMaximumFrameLatency(2);*/

    GetWindowRect(hFocusWindow, &wpos);
    dbg("Device window post-create: %s", strRect(&wpos));

    pp->BackBufferFormat = newpp.BackBufferFormat;
    createBuffers();

    // Initialize overlay
    OVERLAY_INIT_BLOCK op;
    op.overlayVersion = OVERLAY_VERSION;
    op.dev = dev;
    initOverlay(&op);

  } else {
    dbg("Singlehead mode %dx%d %dHz", pp->BackBufferWidth, pp->BackBufferHeight, pp->FullScreen_RefreshRateInHz);

    pp->FullScreen_RefreshRateInHz = matchRefresh(pp);

    char name[256];
    DWORD pid;
    GetWindowText(hFocusWindow, name, 256);
    GetWindowThreadProcessId(hFocusWindow, &pid);
    dbg("Focus window 0x%08X <%s>, thread 0x%08X", hFocusWindow, name, pid);
    ihGlobal.hookRemoteThread(pid);
    GetWindowText(pp->hDeviceWindow, name, 256);
    GetWindowThreadProcessId(pp->hDeviceWindow, &pid);
    dbg("Device window 0x%08X <%s>, thread 0x%08X", pp->hDeviceWindow, name, pid);
    ihGlobal.hookRemoteThread(pid);

    RECT r;
    GetWindowRect(hFocusWindow, &r);
    dbg("Device window  pre-create: %s", strRect(&r));

    // Create device
    bool fullscreen = pp->Windowed==0;
    D3DDISPLAYMODEEX mode = {
      sizeof(D3DDISPLAYMODEEX),
      pp->BackBufferWidth,   pp->BackBufferHeight,
      pp->FullScreen_RefreshRateInHz,  pp->BackBufferFormat,
      D3DSCANLINEORDERING_PROGRESSIVE
    };
    HRESULT ret = direct3D->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hFocusWindow, BehaviorFlags, pp, !fullscreen?NULL:&mode, &dev);
    createDeviceResult = ret;
    if(ret != D3D_OK) {
      dbg("SoftTH: CreateDeviceEx FAILED!");
      return;
    }

    GetWindowRect(hFocusWindow, &wpos);
    dbg("Device window post-create: %s", strRect(&wpos));
  }
}

void IDirect3DDevice9SoftTH::createBuffers()
{
  int br = getRefs();

  // Set mouse hook on application focus window
  ihGlobal.setHWND(hFocusWindow);
  dev->SetMaximumFrameLatency(1);

  // Get backbuffer size
  D3DCALL( dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bb) );
  bb->GetDesc(&bbDesc);
  bb->Release();  // Pretend we never touched it
  dbg("Real backbuffer: %dx%d %s ms%d:%d", bbDesc.Width, bbDesc.Height, getMode(bbDesc.Format), bbDesc.MultiSampleType, bbDesc.MultiSampleQuality);

#ifdef ENABLE_POSTPROCESS
  if(postprocess)
  {
    // Override XRGB mode to ARGB
    // TODO: use only with FXAA pre-pass
    if(bbDesc.Format == D3DFMT_X8R8G8B8) bbDesc.Format = D3DFMT_A8R8G8B8;
  }
#endif

  // Create our new backbuffer
  dbg("Creating new backbuffer: %dx%d %s ms%d:%d", wantedX, wantedY, getMode(bbDesc.Format), msWanted, msQWanted);
  D3DCALL( dev->CreateRenderTarget(wantedX, wantedY, bbDesc.Format, msWanted, msQWanted, lockableBB, &newbb, NULL) );
  D3DCALL( dev->SetRenderTarget(0, newbb) );
  newbb->GetDesc(&newbbDesc);

#ifdef ENABLE_POSTPROCESS
  if(postprocess)
  {
    // Create auxiliary buffer for postprocess
    dbg("Creating postprocess auxiliary buffer: %dx%d %s", wantedX, wantedY, getMode(bbDesc.Format));
    D3DCALL( dev->CreateTexture(wantedX, wantedY, 1, D3DUSAGE_RENDERTARGET, bbDesc.Format, D3DPOOL_DEFAULT, &bbtex, NULL) );
    D3DCALL( bbtex->GetSurfaceLevel(0, &ppaux) );
  }
#endif

  if(depthWanted) {
    dbg("Creating DepthStencil surface: %dx%d %s %s", wantedX, wantedY, getMode(depthFormatWanted), discardDepth?"DISCARD":"");
    D3DCALL( dev->CreateDepthStencilSurface(wantedX, wantedY, depthFormatWanted, msWanted, msQWanted, discardDepth, &newdepth, NULL)  );
    dev->SetDepthStencilSurface(newdepth);
  } else
    newdepth = NULL;

  // Create temporary copy buffer.
  // Copy path is as follows: newbb -> copybuf (present bb) copybuf -> bb
  dbg("Creating copybuffer: %dx%d %s ms%d:%d", bbDesc.Width, bbDesc.Height, getMode(bbDesc.Format), bbDesc.MultiSampleType, bbDesc.MultiSampleQuality);
  D3DCALL( dev->CreateRenderTarget(bbDesc.Width, bbDesc.Height, bbDesc.Format, bbDesc.MultiSampleType, bbDesc.MultiSampleQuality, false, &copybuf, NULL) );

  // Init additional heads
  numDevs = config.getNumAdditionalHeads();
  int logoStopTime = GetTickCount() + 4000;

  bool needIndirect = true;
  if(config.getPrimaryHead()->manufacturer == MANF_NVIDIA)
  {
    needIndirect = false;
    dbg("NVIDIA card detected, using direct nonlocal read");
  }

  outDevs = new OUTDEVICE[numDevs];
  for(int i=0;i<numDevs;i++)
  {
    OUTDEVICE *o = &outDevs[i];

    // Create the output device
    HEAD *h = config.getHead(i);
    bool local = h->transportMethod==OUTMETHOD_LOCAL;
    dbg("Initializing head %d (DevID: %d, %s)...", i+1, h->devID, local?"local":"non-local");
    o->output = new outDirect3D9(h->devID, h->transportMethod, h->screenMode.x, h->screenMode.y, h->transportRes.x, h->transportRes.y, isNullRect(&h->destRect), hFocusWindow, fpuPreserve, logoStopTime, needIndirect);
    o->cfg = h;

    // Create shared surface
    HANDLE sha = o->output->GetShareHandleA();
    HANDLE shb = o->output->GetShareHandleB();
    if(sha && shb) {
      o->localSurfA = NULL;
      o->localSurfB = NULL;
      D3DCALL( dev->CreateRenderTargetEx(o->output->getBufWidth(), o->output->getBufHeight(), o->output->getFormat(), D3DMULTISAMPLE_NONE, 0, false, &o->localSurfA, &sha, NULL) );
      D3DCALL( dev->CreateRenderTargetEx(o->output->getBufWidth(), o->output->getBufHeight(), o->output->getFormat(), D3DMULTISAMPLE_NONE, 0, false, &o->localSurfB, &shb, NULL) );
    } else
      dbg("ERROR: Head %d: No share handles!", i+1);

    o->tempTex = NULL;

    if(h->transportMethod==OUTMETHOD_NONLOCAL_24b) {
      // Create packer copy
      if(!copyPack24to32)
        copyPack24to32 = new shCopy(dev, "D:\\dev\\data\\pack.psh");
      D3DCALL( dev->CreateTexture(o->output->getBufWidth(), o->output->getBufHeight(), 1, D3DUSAGE_RENDERTARGET, o->output->getFormat(), D3DPOOL_DEFAULT, &o->tempTex, NULL)  );
    }

    if(h->transportMethod==OUTMETHOD_NONLOCAL_16bDither) {
      // Create dither copy
      if(!copyDither)
        copyDither = new shCopy(dev, SHCOPY_DITHER);
        //copyDither = new shCopy(dev, "D:\\dev\\data\\dither.psh");
      D3DCALL( dev->CreateTexture(o->output->getBufWidth(), o->output->getBufHeight(), 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &o->tempTex, NULL)  );
    }
  }

  dev->SetRenderState(D3DRS_ZENABLE, depthWanted?D3DZB_TRUE:D3DZB_FALSE); // Default value depends on original state of EnableAutoDepthStencil

  D3DX( D3DXCreateFont(dev, 16, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana", &font) );
  D3DX( D3DXCreateFont(dev, 16, 16, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana", &fontWide) );

  // Query used for pipeline synchronization
  D3DCALL( dev->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &squeryA) );
  D3DCALL( dev->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &squeryB) );

	// Check process affinity mask
#ifndef _WIN64
  DWORD mproc, msys;
	if(GetProcessAffinityMask(GetCurrentProcess(), &mproc, &msys)) {
		if(config.overrides.processAffinity <= 0) {
			// Display warning about affinity
			if(mproc < msys) {
				dbg("Warning! Process affinity is forced to less than processors available (0x%02X < 0x%02X),", mproc, msys);
				dbg("         SoftTH performance may not be optimal. Use processAffinity=1 to override.");
			}
		} else {
			// Force-fix affinity to all processors
			dbg("Forcing process affinity (0x%02X -> 0x%02X)", mproc, msys);
			if(!SetProcessAffinityMask(GetCurrentProcess(), msys))
				dbg("SetProcessAffinityMask failed!");
		}
	}
#endif

  drawing = new d3dDrawing(dev);
  int er = getRefs();
  refs = er-br;
  dbg("Additional refs: %d (%d - %d)", refs, br, er);

  SoftTHActive++;
}

void IDirect3DDevice9SoftTH::adjustPP(D3DPRESENT_PARAMETERS *pp)
{
  int wantedX = pp->BackBufferWidth;
  int wantedY = pp->BackBufferHeight;

  msWanted = pp->MultiSampleType;
  msQWanted = pp->MultiSampleQuality;
  depthWanted = pp->EnableAutoDepthStencil!=0;
  int numbbWanted =  pp->BackBufferCount;
  depthFormatWanted = pp->AutoDepthStencilFormat;

  if(config.overrides.antialiasing > 0)
    msWanted = (D3DMULTISAMPLE_TYPE) config.overrides.antialiasing;

  discardDepth = (pp->Flags&D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL)!=0;
  lockableBB = (pp->Flags&D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)!=0;
  /*bool fpuPreserve = (BehaviorFlags&D3DCREATE_FPU_PRESERVE)!=0;*/
  dbg("D3DCREATE_FPU_PRESERVE: %s", fpuPreserve?"enabled":"disabled");
  dbg("D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL: %s", discardDepth?"enabled":"disabled");
  dbg("D3DPRESENTFLAG_LOCKABLE_BACKBUFFER: %s", lockableBB?"enabled":"disabled");

  dbg("%s (%d backbuffer(s))", getSwapEffect(pp->SwapEffect), pp->BackBufferCount);
  dbg("%s", getPresentationInterval(pp->PresentationInterval));
  dbg("Multisample level: %d (Quality %d)", msWanted, msQWanted);
  dbg("DepthStencil: %s (%s)", pp->EnableAutoDepthStencil?"enabled":"disabled", getMode(pp->AutoDepthStencilFormat));

  // Get mode from primary head config
  HEAD *hp = config.getPrimaryHead();
  pp->BackBufferWidth = hp->screenMode.x;
  pp->BackBufferHeight = hp->screenMode.y;
  dbg("Primary head: %dx%d", hp->screenMode.x, hp->screenMode.y);

  hp->hwnd = hFocusWindow;

  // No multisampling required for the backbuffer
  pp->MultiSampleType = D3DMULTISAMPLE_NONE;
  pp->MultiSampleQuality = 0;
  pp->EnableAutoDepthStencil = false; // No depth buffer needed, we create our own

  pp->PresentationInterval = config.main.vsync?D3DPRESENT_INTERVAL_ONE:D3DPRESENT_INTERVAL_IMMEDIATE;
  pp->BackBufferCount = config.main.tripleBuffer?2:0;

  if(discardDepth)
    pp->Flags ^= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
  if(config.main.tripleBuffer && pp->SwapEffect != D3DSWAPEFFECT_DISCARD) {
    dbg("Warning: Overriding swapeffect to D3DSWAPEFFECT_DISCARD due to triplebuffer override");
    pp->SwapEffect = D3DSWAPEFFECT_DISCARD;
  }
  memcpy(&lastPp, pp, sizeof(D3DPRESENT_PARAMETERS));
}

void IDirect3DDevice9SoftTH::destroyBuffers()
{
  dbg("SoftTH: Releasing buffers (%d devices)", numDevs);
  for(int i=0;i<numDevs;i++) {
    delete outDevs[i].output;
    SAFE_RELEASE_LAST(outDevs[i].localSurfA);
    SAFE_RELEASE_LAST(outDevs[i].localSurfB);
    SAFE_RELEASE_LAST(outDevs[i].tempTex);
  }
  numDevs = 0;

  D3DX(
    SAFE_RELEASE_LAST(font);
    SAFE_RELEASE_LAST(fontWide);
  )

/*
  IDirect3DSurface9 *cd;
  dev->GetDepthStencilSurface(&cd);
  if(cd == newdepth)
  {
    dbg("depth was still set");
    dev->SetDepthStencilSurface(NULL);
  }
  else
    if(cd)
      cd->Release();
*/

  SAFE_RELEASE_LAST(copybuf);
  SAFE_RELEASE_LAST(squeryA);
  SAFE_RELEASE_LAST(squeryB);
  SAFE_RELEASE_LAST(bbtex);
  SAFE_RELEASE_LAST(newbb);
  SAFE_RELEASE_LAST(ppaux);
  SAFE_RELEASE_LAST(newdepth);

  newbb = NULL;
  bbtex = NULL;
  ppaux = NULL;
  newdepth = NULL;
  squeryB = squeryA = NULL;
  copybuf = NULL;

  delete copyPack24to32;
  delete copyDither;
  delete drawing;

  copyDither = NULL;
  copyPack24to32 = NULL;
  drawing = NULL;

  timeEndPeriod(1);
  SoftTHActive--;
  refs = 0;
}

ULONG IDirect3DDevice9SoftTH::Release()
{
  if(newbb)
  {
    int rr = getRefs();
    if(getRefs()-refs-1 == 0 && newbb) {

      // Let overlay release its stuff
      deinitOverlay();

      // Last ref - device will be freed on Release so release our stuff
      dbg("Release: SoftTH device freed (refcount %d-%d=0)", rr-1, refs);
      destroyBuffers();
      if(getRefs() != 1)
        dbg("Release: WARNING! Reference leak detected: Post-release refcount not 1 (%d)", getRefs());
      /*else
        delete this;*/
    }

    ULONG r = __super::Release();
    r -= refs;
    dbgf("dev SoftTH: Release result: %d", r);
    /*if(r == 0) {
      delete this;
    }*/
    return r;

  } else {
    return __super::Release();
  }
}

IDirect3DDevice9SoftTH::~IDirect3DDevice9SoftTH()
{
  dbg("~IDirect3DDevice9SoftTH");
  /*destroyBuffers();*/
  parent->destroyed(this);
}

void IDirect3DDevice9SoftTH::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
  dbgf("dev: SetGammaRamp %d", iSwapChain);
  if(iSwapChain == 0)
    for(int i=0;i<numDevs;i++)
      outDevs[i].output->SetGammaRamp(Flags, pRamp);
  dev->SetGammaRamp(iSwapChain, Flags, pRamp);
  return;
}

// Return matching refresh rate (emulate non-ex behaviour)
int IDirect3DDevice9SoftTH::matchRefresh(D3DPRESENT_PARAMETERS *pp)
{
  int numModes = d3d->GetAdapterModeCount(0, pp->BackBufferFormat);
  for(int m=0;m<numModes;m++) {
    D3DDISPLAYMODE mode;
    d3d->GetAdapterDisplayMode(0, &mode);
    if(mode.Width == pp->BackBufferWidth && mode.Height == pp->BackBufferHeight && mode.RefreshRate == pp->FullScreen_RefreshRateInHz)
      return mode.RefreshRate;
  }
  dbg("Mode %dx%d %dHz: Not found, using default refresh rate", pp->BackBufferWidth, pp->BackBufferHeight, pp->FullScreen_RefreshRateInHz);
  return 0; // No match found, use default
}

// Device reset - This is not needed with D3D9Ex but app can do it anyway
HRESULT IDirect3DDevice9SoftTH::Reset(D3DPRESENT_PARAMETERS* pp)
{
  dbg("RESET");
  memcpy(&lastPp, pp, sizeof(D3DPRESENT_PARAMETERS));
  if(config.overrides.forceResolution && !pp->Windowed) {
    dbg("Forcing device resolution from %dx%d to %dx%d", pp->BackBufferWidth, pp->BackBufferHeight, config.main.renderResolution.x, config.main.renderResolution.y);
    pp->BackBufferWidth = config.main.renderResolution.x;
    pp->BackBufferHeight = config.main.renderResolution.y;
  }

  wantedX = pp->BackBufferWidth;
  wantedY = pp->BackBufferHeight;

  //dbg("SoftTH: RESET (%dx%d)", pp->BackBufferWidth, pp->BackBufferHeight);
  if(isSoftTHmode(pp->BackBufferWidth, pp->BackBufferHeight) && !pp->Windowed)
  {
    dbg("Reset: Multihead mode %dx%d %s detected", pp->BackBufferWidth, pp->BackBufferHeight, pp->Windowed?"(windowed)":"(fullscreen)");

    D3DPRESENT_PARAMETERS newpp = *pp;
    adjustPP(&newpp);

    RECT r;
    GetWindowRect(hFocusWindow, &r);
    dbg("Device window  pre-reset: %s", strRect(&r));

    HRESULT ret = dev->Reset(&newpp);
    if(ret == D3D_OK)
    {
      if(newbb)
        destroyBuffers();
      pp->BackBufferFormat = newpp.BackBufferFormat;
      createBuffers();

      // Initialize overlay
      OVERLAY_INIT_BLOCK op;
      op.overlayVersion = OVERLAY_VERSION;
      op.dev = dev;
      initOverlay(&op);
    } else {
      dbg("Reset: FAILED");
    }
    GetWindowRect(hFocusWindow, &r);
    dbg("Device window  post-reset: %s", strRect(&r));
    return ret;
  } else {
    dbg("Reset: Singlehead mode %dx%d %dHz %s", pp->BackBufferWidth, pp->BackBufferHeight, pp->FullScreen_RefreshRateInHz, pp->Windowed?"(windowed)":"(fullscreen)");
    if(newbb)
      destroyBuffers();

    pp->FullScreen_RefreshRateInHz = matchRefresh(pp);

    // Create device
    bool fullscreen = (pp->Windowed==0);
    D3DDISPLAYMODEEX mode = {
      sizeof(D3DDISPLAYMODEEX),
      pp->BackBufferWidth,   pp->BackBufferHeight,
      pp->FullScreen_RefreshRateInHz,  pp->BackBufferFormat,
      D3DSCANLINEORDERING_PROGRESSIVE
    };

    HRESULT ret = dev->ResetEx(pp, fullscreen?&mode:NULL);
    if(ret != D3D_OK)
      dbg("Reset: FAILED: %s", getD3DError(ret));
    else {
      HRESULT r = dev->CheckDeviceState(pp->hDeviceWindow);
      if(r != D3D_OK) {
        dbg("Reset: DeviceState: %s!", getD3DError(r));
      }
      if(r == S_PRESENT_MODE_CHANGED)
        ret = D3DERR_DEVICELOST;  // Emulate invalid mode
    }

    //dbg("RESET ret ret");
    return ret;
  }
  //dbg("RESET ret");
  return D3D_OK;
  //HRESULT ret = __super::Reset(pp);

  //return ret;
}

HRESULT IDirect3DDevice9SoftTH::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
  if(!newbb)
    return dev->Clear(Count, pRects, Flags, Color, Z, Stencil);

  HRESULT ret;
  if(Flags&D3DCLEAR_ZBUFFER && (pRects==NULL || Count == 0) && config.main.zClear && !squash)
  //if(Flags&D3DCLEAR_ZBUFFER && (pRects==NULL || Count == 0) && config.main.zClear)
  {
    // App is clearing depth buffer...
    IDirect3DSurface9 *cdepth;
    dev->GetDepthStencilSurface(&cdepth);
    if(!cdepth)
      return dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
    D3DSURFACE_DESC d;
    cdepth->GetDesc(&d);
    cdepth->Release();
    //if(cdepth == newdepth)
    if(d.Width == newbbDesc.Width && d.Height == newbbDesc.Height)  // Size matches, assume it is the render depthbuffer
    {
      // ...which is our buffer!
      if(Flags-D3DCLEAR_ZBUFFER)
        ret = dev->Clear(Count, pRects, Flags-D3DCLEAR_ZBUFFER, Color, Z, Stencil);
      else
        ret = D3D_OK;

      // Clear whole buffer with Z = 0 so no rendering is done with depth testing
      // Then re-clear display areas with Z = Z so rendering is only done on visible areas
      D3DCALL( dev->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 0, 0) );
      const int bz = 16; // Border zone
      int maxx = config.main.renderResolution.x;
      int maxy = config.main.renderResolution.y;

      D3DRECT *r = new D3DRECT[numDevs+1];
      HEAD *ph = config.getPrimaryHead();
      r[0].x1 = max(ph->sourceRect.left-bz, 0);  r[0].y1 = max(ph->sourceRect.top-bz, 0);
      r[0].x2 = min(ph->sourceRect.right+bz, maxx); r[0].y2 = min(ph->sourceRect.bottom+bz, maxy);
      for(int i=0;i<numDevs;i++) {
        r[i+1].x1 = max(outDevs[i].cfg->sourceRect.left-bz, 0);  r[i+1].y1 = max(outDevs[i].cfg->sourceRect.top-bz, 0);
        r[i+1].x2 = min(outDevs[i].cfg->sourceRect.right+bz, maxx); r[i+1].y2 = min(outDevs[i].cfg->sourceRect.bottom+bz, maxy);
      }
      D3DCALL( dev->Clear(numDevs+1, r, D3DCLEAR_ZBUFFER, 0, Z, 0) );
      delete[] r;
      return ret;
    }
  }

  return dev->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

void IDirect3DDevice9SoftTH::printMessage(char *first, ...)
{
  static char tmp[512];
  va_list  argptr;
  va_start (argptr, first);
  vsprintf (tmp, first, argptr);
  va_end   (argptr);
  sprintf(message, "  %s  ", tmp);
  messageTime = GetTickCount() + 3000;
}

void IDirect3DDevice9SoftTH::drawOverlay()
{
  IDirect3DSurface9 *lastbb, *lastd = NULL;
  D3DVIEWPORT9 lastvp;
  dev->GetRenderTarget(0, &lastbb);
  dev->SetRenderTarget(0, bb);

  dev->GetDepthStencilSurface(&lastd);
  dev->SetDepthStencilSurface(NULL);

  D3DVIEWPORT9 vp = {0, 0, bbDesc.Width, bbDesc.Height, 0, 1};
  dev->GetViewport(&lastvp);
  dev->SetViewport(&vp);

  if(GetTickCount() < messageTime) {
    // Draw on-screen message
    HEAD *ph = config.getPrimaryHead();
    drawing->beginDraw();
    const int height = 24;
    const int width = 400;
    RECT r = {0, bbDesc.Height - height, width, bbDesc.Height};
    //dbg("rectPRE: %s", strRect(&r));
    fontWide->DrawText(NULL, message, -1, &r, DT_CALCRECT, D3DCOLOR_ARGB(255, 255, 255, 255));
    //dbg("rectPOST: %s", strRect(&r));
    drawing->drawBox(r.left, r.top, r.right-r.left+24, height, 0xA0000000);
    r.bottom = bbDesc.Height;
    fontWide->DrawText(NULL, message, -1, &r, DT_CENTER|DT_VCENTER, D3DCOLOR_ARGB(255, 255, 255, 255));
    drawing->endDraw();
  }

  dev->SetDepthStencilSurface(lastd);
  dev->SetRenderTarget(0, lastbb);
  dev->SetViewport(&lastvp);
  lastbb->Release();
  if(lastd)
    lastd->Release();
}


// Present backbuffer contents
HRESULT IDirect3DDevice9SoftTH::PresentEx(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
  //dbg("frame: %d", frameCounter);
  dbgf("IDirect3DDevice9SoftTH Present %d", dev);
  if(pSourceRect || pDestRect || pDirtyRegion) {
    ONCE {
      dbg("Warning: Complex present detected (%d, %d, %d)", pSourceRect, pDestRect, pDirtyRegion);
    }
  }
  if(hDestWindowOverride) {
    ONCE {
      dbg("Warning: Present with window override detected (0x%08X, 0x%08X)", hDestWindowOverride, hFocusWindow);
    }
  }

  /*
  if(true)
  {
    RECT r;
    GetWindowRect(hFocusWindow, &r);
    if(r.left != wpos.left || r.right != wpos.right || r.top != wpos.top ||r.bottom != wpos.bottom)
    {
      // Window has moved, put it back
      // Non-ex d3d9 seems to do this automatically?
      dbg("Resetting device window position (%s)", strRect(&r));
      MoveWindow(hFocusWindow, wpos.left, wpos.top, wpos.right-wpos.left, wpos.bottom-wpos.top, false);
      GetWindowRect(hFocusWindow, &r);
      dbg("Device window moved to (%s)", strRect(&r));
    }
  }*/

  static bool hasSetLogoTime = false;
  if(!hasSetLogoTime)
  {
    for(int i=0;i<numDevs;i++)
      outDevs[i].output->setLogoShowTime(GetTickCount() + 4000);
    hasSetLogoTime = true;
  }

  // Collect timing data
  float t = (float)stopTimer()*10;
  for(int i=1;i<NUM_FPS_GRAPH;i++)
    fpsGraph[i-1] = fpsGraph[i];
  fpsGraph[NUM_FPS_GRAPH-1] = t>1?1:t;

  float avg = 0;
  for(int i=0;i<50;i++)
    avg += fpsGraph[NUM_FPS_GRAPH-1-i];
  avg /= 50;

  startTimer();
  lastFrameTime = t;

  bool notactive = false;
  HWND fgw = GetForegroundWindow();
  if(fgw != hFocusWindow)
  {
    dbgf("IDirect3DDevice9SoftTH::Present: Focus lost");
    bool outsider = true;
    for(int i=0;i<numDevs;i++)
      if(fgw == outDevs[i].output->getWindow())
        outsider = false;
    if(outsider)
    {
      char foo[256];
      GetWindowText(GetForegroundWindow(), foo, 256);
      dbg("Lost focus to: <%s>", foo);
      notactive = true;
      /*for(int i=0;i<numDevs;i++)
        outDevs[i].output->minimize();*/
      //ShowWindow(hFocusWindow, SW_SHOWMINNOACTIVE);
    }
  }

  if(!newbb || notactive)
  {
    HRESULT ret = dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
    dbgf("IDirect3DDevice9SoftTH::Present: notactive present result: %s", getD3DError(ret));
    if(ret == S_PRESENT_OCCLUDED || ret == S_PRESENT_MODE_CHANGED)
      ret = D3D_OK; // TODO: do this only if not an Ex device
    frameCounter++;
    return ret;
    //return dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
  }

#ifdef USE_D3DX

  // Make sure to render graph to BB
  IDirect3DSurface9 *ort = NULL, *od = NULL;
  D3DVIEWPORT9 ovp;
  dev->GetRenderTarget(0, &ort);
  dev->GetDepthStencilSurface(&od);
  dev->GetViewport(&ovp);
  dev->SetRenderTarget(0, newbb);
  dev->SetDepthStencilSurface(NULL);

#ifdef ENABLE_POSTPROCESS
  // Apply postprocessing
  if(postprocess && GetKeyState('O')>=0)
  {
    // Copy backbuffer to pp texture
    //dev->ColorFill(ppaux, NULL, 0xFF00FFFF);
    D3DCALL( dev->StretchRect(newbb, NULL, ppaux, NULL, D3DTEXF_NONE) );

    //dev->ColorFill(newbb, NULL, 0xFF00FFFF);
    drawing->beginDraw();

    bool doLumaFXAAPass = false;
    if(doLumaFXAAPass)
    {
      // FXAA requires luma thing
      drawing->drawTexturePP(0, 0, 5760, 1800, bbtex, "PostProcessFXAAlumaPass");
      // Copy result back to ppaux so it will be available in FXAA pass
      // TODO: copy only alpha data on correct area?
      D3DCALL( dev->StretchRect(newbb, NULL, ppaux, NULL, D3DTEXF_NONE) );
    }
    //drawing->drawTexturePP(1920, 0, 1920, 1200, bbtex);

    drawing->drawTexturePP(0, 0, 1800, 900, bbtex, "PostProcessTest");
/*
    if(GetKeyState('O')<0)
      drawing->drawTexturePP(0, 0, 1920*2, 1200, bbtex, "PostProcessFXAA");
    else
      drawing->drawTexturePP(0, 0, 5760, 1800, bbtex, "PostProcessFXAA");
*/
    //drawing->drawTextureLens(0, 0, newbbDesc.Width, newbbDesc.Height, bbtex);
    drawing->endDraw();
  }
#endif

  // Let overlay plugin draw its things
  OVERLAY_DRAW_BLOCK op;
  op.overlayVersion = OVERLAY_VERSION;
  op.width = newbbDesc.Width;
  op.height = newbbDesc.Height;
  op.dev = dev;
  op.newbb = newbb;

  op.primaryHead = config.getPrimaryHead();
  op.numHeads = config.getNumAdditionalHeads();
  for(int i=0;i<op.numHeads;i++)
  {
    op.extraHeads[i] = config.getHead(i);
  }

  overlayDoDraw(&op);

  if(showGraph) {
    // FPS graph
    dbgf("IDirect3DDevice9SoftTH::Present: draw graph");
    HEAD *ph = config.getPrimaryHead();
    drawing->beginDraw();

    drawing->drawGraph(ph->sourceRect.left+100, ph->sourceRect.top+100, (ph->sourceRect.right-ph->sourceRect.left)/2, 300, fpsGraph, NUM_FPS_GRAPH);

    char foo[256];
    sprintf(foo, "FPS: %d ", FPS);
    for(int i=0;i<numDevs;i++)
      sprintf(foo, "%s/ %d", foo, outDevs[i].output->getFPS() );

    dev->AddRef();
    sprintf(foo, "%s\nRefcount: %d", foo, dev->Release());

    RECT r = {0, 100, bbDesc.Width*3, 200};
    (squash?fontWide:font)->DrawText(NULL, foo, -1, &r, DT_CENTER|DT_TOP, D3DCOLOR_ARGB(255, 255, 255, 255));

    // Draw backlog
/*
    if(showLog) {

      int l = 0;
      char *line = NULL;
      do {
        line = getBackLogLine(l);
        if(line) {
          RECT r = {0, 100+(l*16), bbDesc.Width*3, 100+(l*16)+16};
          (squash?fontWide:font)->DrawText(NULL, line, -1, &r, DT_LEFT|DT_TOP, D3DCOLOR_ARGB(255, 255, 255, 255));
        }
        l++;
      } while(line);

    }
    */

    //dev->EndScene();

    drawing->endDraw();
  }

  static bool showDebugBars = false;
  if(showDebugBars) {
    drawing->beginDraw();

    int debugBarY = (int) ((float)config.main.renderResolution.y*((sin((float)GetTickCount()/500.0f)+1.57079633f) / 3.14159265f));
    drawing->drawBox(0, debugBarY, config.main.renderResolution.x, 100, 0xFFFFFFFF);

    drawing->endDraw();
  }

  dev->SetRenderTarget(0, ort);
  dev->SetDepthStencilSurface(od);
  dev->SetViewport(&ovp);
  if(ort) ort->Release();
  if(od) od->Release();
#endif

  IDirect3DSurface9 *srcbuf = newbb;  // Source buffer for head stretchrects
  if(srcbuf == bb)
    dbg("ERROR: srcbuf == bb??");

  static bool doStall = config.main.smoothing;

  if(ihGlobal.key(VK_APPLICATION) || true) // TODO: remove this
  {
    if(ihGlobal.keyAsync('S'))
      squash = !squash, printMessage("Squash: %s", squash?"ON":"OFF"), Sleep(50);
    if(ihGlobal.keyAsync('W'))
      nocopy = !nocopy, printMessage("No copy: %s", nocopy?"ON":"OFF"), Sleep(50);
    if(ihGlobal.keyAsync('G')) {
      if(!showGraph) showGraph = true, showLog = false;
      else if(showGraph && !showLog) showLog = true;
      else if(showGraph && showLog) showGraph = showLog = false;
      printMessage("Graph: %s %s", showGraph?"ON":"OFF", showLog?"+log":"");
    }
    if(ihGlobal.keyAsync('E'))
      doStall = !doStall, printMessage("Smoothing: %s", doStall?"ON":"OFF");
    if(ihGlobal.keyAsync('B'))
      showDebugBars = !showDebugBars, printMessage("Debug bar: %s", showDebugBars?"ON":"OFF");
    if(ihGlobal.keyAsync('M'))
      dbg("--- mark ---");
    if(ihGlobal.keyAsync('N'))
      SetCursorPos(2880, 600);

    if(ihGlobal.keyAsync(VK_F4))
      exit(0);
  }
  if(ihGlobal.keyAsync(VK_SNAPSHOT))
    saveScreenshot();
  ihGlobal.resetAsyncKeys();

  // FPS counter
  fCounter++;
  if(GetTickCount() - fTimer >= 1000)
  {
    FPS = fCounter;
    fCounter = 0;
    fTimer = GetTickCount();
  }

  // Squash? Dump everything to primary head
  if(squash) {
    D3DCALL( dev->StretchRect(srcbuf, NULL, bb, NULL, D3DTEXF_LINEAR) );
    for(int i=0;i<numDevs;i++)
      outDevs[i].output->PresentOff();
    drawOverlay();
    frameCounter++;
    return dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
  }

  // nocopy? present only primary head
  if(nocopy) {
    HEAD *ph = config.getPrimaryHead();
    D3DCALL( dev->StretchRect(srcbuf, &ph->sourceRect, bb, isNullRect(&ph->destRect), D3DTEXF_LINEAR) );
    for(int i=0;i<numDevs;i++)
      outDevs[i].output->PresentOff();
    drawOverlay();
    frameCounter++;
    return dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
  }

  if(doStall) {
    // Stall the pipeline
    squeryA->Issue(D3DISSUE_END);
  }

  // Copy primary head rect
  dbgf("IDirect3DDevice9SoftTH::Present: primary head stretchrect");
  int tb = GetTickCount();
  HEAD *ph = config.getPrimaryHead();
  D3DCALL( dev->StretchRect(srcbuf, &ph->sourceRect, copybuf, isNullRect(&ph->destRect), D3DTEXF_LINEAR) );
  timeWarn(tb, 250, "Primary head stretchrect");
  // Copy secondary heads
  tb = GetTickCount();
  for(int i=0;i<numDevs;i++) {
    dbgf("IDirect3DDevice9SoftTH::Present: copy head %d", i);

    // FPS limiting
    if(outDevs[i].cfg->rateLimit && (GetTickCount() - outDevs[i].cfg->lastUpdate) <= outDevs[i].cfg->rateLimit)
    {
      outDevs[i].cfg->skipPresentNext = true;
      continue;
    }
    outDevs[i].cfg->lastUpdate = GetTickCount();
    outDevs[i].cfg->skipPresentNext = false;

    /*if(outDevs[i].cfg->rateLimit)
      dbg("update! %d", GetTickCount());*/

    if(outDevs[i].output->isReadyForData()) {
      if(outDevs[i].cfg->transportMethod == OUTMETHOD_NONLOCAL_24b) {
        // Copy srcbuf -> tempTex, then shader copy to remote surface
        IDirect3DSurface9 *s;
        outDevs[i].tempTex->GetSurfaceLevel(0, &s);
        D3DCALL( dev->StretchRect(srcbuf, &outDevs[i].cfg->sourceRect, s, NULL, D3DTEXF_LINEAR) );
        //D3DCALL( dev->StretchRect(srcbuf, &outDevs[i].cfg->sourceRect, s, isNullRect(&outDevs[i].cfg->destRect), D3DTEXF_LINEAR) );
        s->Release();
        copyPack24to32->surfCopyShader(outDevs[i].tempTex, curSurfA?outDevs[i].localSurfA:outDevs[i].localSurfB);
      } else if(outDevs[i].cfg->transportMethod == OUTMETHOD_NONLOCAL_16bDither) {
        // Same as above but with dither effect
        IDirect3DSurface9 *s;
        outDevs[i].tempTex->GetSurfaceLevel(0, &s);
        D3DCALL( dev->StretchRect(srcbuf, &outDevs[i].cfg->sourceRect, s, NULL, D3DTEXF_LINEAR) );
        //D3DCALL( dev->StretchRect(srcbuf, &outDevs[i].cfg->sourceRect, s, isNullRect(&outDevs[i].cfg->destRect), D3DTEXF_LINEAR) );
        s->Release();
        copyDither->surfCopyShader(outDevs[i].tempTex, curSurfA?outDevs[i].localSurfA:outDevs[i].localSurfB);
      } else {
        D3DCALL( dev->StretchRect(srcbuf, &outDevs[i].cfg->sourceRect, curSurfA?outDevs[i].localSurfA:outDevs[i].localSurfB, NULL, D3DTEXF_LINEAR) );
        //D3DCALL( dev->StretchRect(srcbuf, &outDevs[i].cfg->sourceRect, curSurfA?outDevs[i].localSurfA:outDevs[i].localSurfB, isNullRect(&outDevs[i].cfg->destRect), D3DTEXF_LINEAR) );
      }
    }
  }
  timeWarn(tb, 250, "Secondary head stretchrect");

  tb = GetTickCount();

  if(doStall) {
    // Stall the pipeline
    //squeryA->Issue(D3DISSUE_END);
    long long x = 0;
    while(squeryA->GetData((void *)&x, sizeof(long long), D3DGETDATA_FLUSH) == S_FALSE) YIELD_CPU;

  } else {
    // Signal this frame end...
    if(curSurfA)
      squeryA->Issue(D3DISSUE_END);
    else
      squeryB->Issue(D3DISSUE_END);

    // ...and wait for previous one to complete
    long long x = 0;
    if(curSurfA)
      while(squeryB->GetData((void *)&x, sizeof(long long), D3DGETDATA_FLUSH) == S_FALSE) YIELD_CPU;
    else
      while(squeryA->GetData((void *)&x, sizeof(long long), D3DGETDATA_FLUSH) == S_FALSE) YIELD_CPU;
  }
  timeWarn(tb, 250, "doStall");

  // Copy to secondary heads
  tb = GetTickCount();
  dbgf("IDirect3DDevice9SoftTH::Present: output heads");
  for(int i=0;i<numDevs;i++)
  {
    if(!outDevs[i].cfg->skipPresent)
    {
      outDevs[i].output->DoCopy(!outDevs[i].cfg->noSync, curSurfA);
    }
  }
  timeWarn(tb, 250, "Secondary head copy");

  //dev->WaitForVBlank(0);

  // Present primary head
  tb = GetTickCount();
  drawOverlay();
  dbgf("IDirect3DDevice9SoftTH::Present: present primary head");
  HRESULT ret = dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
  if(ret != D3D_OK) {
    dbg("Present failed: %s", getD3DError(ret));
    if(ret == S_PRESENT_OCCLUDED || ret == S_PRESENT_MODE_CHANGED)
      ret = D3D_OK; // TODO: do this only if not an Ex device
  }
  timeWarn(tb, 250, "Primary head PresentEx");

  // Present secondary heads
  tb = GetTickCount();
  dbgf("IDirect3DDevice9SoftTH::Present: present secondary heads");
  for(int i=0;i<numDevs;i++) {
    if(!outDevs[i].cfg->skipPresent)
    {
      outDevs[i].output->Present();
    }
  }
  timeWarn(tb, 250, "Secondary head Present");

  // Flip shared buffers
  tb = GetTickCount();
  dbgf("IDirect3DDevice9SoftTH::Present: flipbuffers");
  curSurfA=!curSurfA;
  D3DCALL( dev->StretchRect(copybuf, NULL, bb, NULL, D3DTEXF_NONE) );
  timeWarn(tb, 250, "Flip buffers");

  // Discard mode emulation - clear "backbuffer"
  tb = GetTickCount();
  d3dClearTarget(dev, newbb, newdepth);
  timeWarn(tb, 250, "d3dClearTarget");

  for(int i=0;i<numDevs;i++)
  {
    outDevs[i].cfg->skipPresent = false;
    // FPS limiting: We want to skip _next_ frame
    if(outDevs[i].cfg->skipPresentNext)
    {
      outDevs[i].cfg->skipPresent = true;
    }
    outDevs[i].cfg->skipPresentNext = false;
  }
  frameCounter++;
  return ret;
}

HRESULT IDirect3DDevice9SoftTH::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain) {
  dbgf("IDirect3DDevice9SoftTH: GetSwapChain");
  HRESULT ret = dev->GetSwapChain(iSwapChain, pSwapChain);
  //if(newbb && ret == D3D_OK) {
  if(ret == D3D_OK) {
    (*pSwapChain) = new IDirect3DSwapChain9SoftTH(this, *pSwapChain);
  }
  return ret;
}

bool needQuirkRTT = false;
HRESULT IDirect3DDevice9SoftTH::SetRenderTarget(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
  dbgf("IDirect3DDevice9SoftTH: SetRenderTarget %d %d", RenderTargetIndex, pRenderTarget);

  if(needQuirkRTT && pRenderTarget)
  {
    DWORD foo = 0;
    DWORD size = sizeof(foo);
    pRenderTarget->GetPrivateData(IID_SoftTHInvalidRTT, &foo, &size);
    if(foo)
    {
      ONCE dbg("QUIRK MODE: Program attempted to use invalid backbuffer for rendering, overriding");

      IDirect3DSurface9 *bbZero;
      dev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bbZero);
      dev->SetRenderTarget(0, bbZero);
      bbZero->Release();
      return D3D_OK;
    }
  }

  HRESULT ret = dev->SetRenderTarget(RenderTargetIndex, OriginalFromNewSurface(pRenderTarget));
  return ret;
}

// Must return our fake backbuffer to application
HRESULT IDirect3DDevice9SoftTH::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
  dbgf("IDirect3DDevice9SoftTH: GetBackBuffer %d", ppBackBuffer);
  HRESULT ret = dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);

  if(ret == D3D_OK && iBackBuffer > 0 && !newbb)
  {
    // QUIRK for Falcon 4 BMS
    // D3D9Ex doesn't like >0 backbuffer to be set as rendertarget - mark this as invalid for RT use
    // So in case it's set as RT later we can override it
    DWORD foo = 1;
    (*ppBackBuffer)->SetPrivateData(IID_SoftTHInvalidRTT, &foo, sizeof(DWORD), NULL);
    needQuirkRTT = true;
  }

  if(newbb && ret == D3D_OK) {
    (*ppBackBuffer)->Release();
    newbb->AddRef();
    *ppBackBuffer = newbb;
  }
  return ret;
}

HRESULT IDirect3DDevice9SoftTH::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
  dbgf("IDirect3DDevice9SoftTH::GetDisplayMode");
  HRESULT ret = dev->GetDisplayMode(iSwapChain, pMode);
  // Return our mode
  if(newbb)
  {
    pMode->Width = newbbDesc.Width;
    pMode->Height = newbbDesc.Height;
  }
  return ret;
}

// FOV overrides (for non-VS apps only)
HRESULT IDirect3DDevice9SoftTH::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
  if(State == D3DTS_PROJECTION && newbb && (config.overrides.FOVForceHorizontal||config.overrides.FOVForceVertical))
  {
    D3DMATRIX *newMatrix = (D3DMATRIX*) pMatrix;
		D3DVIEWPORT9 vp;
		dev->GetViewport(&vp);
    if(vp.Width == newbbDesc.Width && vp.Height == newbbDesc.Height)
    {
      if(config.overrides.FOVForceHorizontal)
        newMatrix->_11 /= 3.0f; // TODO: Calculate from new width / real width
      if(config.overrides.FOVForceVertical)
        newMatrix->_22 *= (float)((float)newbbDesc.Width/(float)newbbDesc.Height);
    }
  }
  return dev->SetTransform(State, pMatrix);
}


void IDirect3DDevice9SoftTH::saveScreenshot()
{
  char ext[4];
  strcpy(ext, config.main.screenshotFormat);

  D3DXIMAGE_FILEFORMAT fmt = (D3DXIMAGE_FILEFORMAT) -1;
  if(!_strcmpi(ext, "jpg")) fmt = D3DXIFF_JPG;
  if(!_strcmpi(ext, "bmp")) fmt = D3DXIFF_BMP;
  if(!_strcmpi(ext, "png")) fmt = D3DXIFF_PNG;
  if(fmt == -1) {
    dbg("Unrecognized screenshot format: <%s>, supported formats: jpg,bmp,png", ext);
    fmt = D3DXIFF_PNG;
    strcpy(ext, "png");
  }

  // Parse time string
  char timestr[64];
  struct tm * timeinfo;
  time_t rawtime;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timestr, 64 ,"%Y_%m_%d_%H_%M_%S", timeinfo);

  char mydocs[256] = "";
  SHGetFolderPath(0, CSIDL_PERSONAL, NULL, NULL, mydocs);

  char path[256];
  sprintf(path, "%s\\SoftTH\\Screenshots\\SoftTH_%s_%s_%d.%s", mydocs, processName(), timestr, GetTickCount()%60, ext);
  D3DXSaveSurfaceToFile(path, fmt, newbb, NULL, NULL);
  dbg("Saved screenshot: <%s>", path);
  printMessage("Saved screenshot: <%s>", path);
}


bool isSoftTHmode(int w, int h)
{
  if(w == config.main.renderResolution.x && h == config.main.renderResolution.y)
    return true;
  return false;
}

static int detectManufacturer(int devID, int headID)
{
  int result = MANF_UNKNOWN;
  IDirect3D9Ex *d3d = NULL;
  Direct3DCreate9Ex(-D3D_SDK_VERSION, &d3d);
  if(!d3d) {
    dbg("detectManufacturer: Direct3DCreate9Ex failed!", devID);
    return result;
  }

  D3DADAPTER_IDENTIFIER9 id;
  d3d->GetAdapterIdentifier(devID, NULL, &id);

  char orig[256];
  char lowr[256];
  ZeroMemory(orig, 256);
  ZeroMemory(lowr, 256);
  strncpy(orig, id.Description, 128);
  for(int i=0;i<128;i++)
  {
    lowr[i] = tolower(orig[i]);
  }

  if(strstr(orig, "AMD") || strstr(lowr, "radeon"))
  {
    result = MANF_AMD;
  }
  if(strstr(orig, "NVIDIA") || strstr(lowr, "geforce") || strstr(lowr, "quadro"))
  {
    result = MANF_NVIDIA;
  }

  if(result == MANF_NVIDIA) dbg("Head %d manufacturer: NVIDIA", headID);
  if(result == MANF_AMD) dbg("Head %d manufacturer: AMD", headID);
  if(result == MANF_UNKNOWN) dbg("Head %d manufacturer: Unknown", headID);

  d3d->Release();
  return result;
}

static void detectTransportMethods()
{
  bool gotLocal = false;
  bool allLocal = true;
  // Do transportmethod autodetection
  for(int i=0;i<config.getNumAdditionalHeads();i++) {
    HEAD *h = config.getHead(i);
    if(h->transportMethod == OUTMETHOD_AUTO)
      h->transportMethod = detectTransportType(h->devID);
    if(h->transportMethod == OUTMETHOD_LOCAL)
      gotLocal = true;

    if(h->transportMethod != OUTMETHOD_LOCAL)
      allLocal = false;
  }

  if(!gotLocal && config.getNumAdditionalHeads()>0)
    dbg("WARNING: All secondary heads are non-local.");

  if(allLocal && config.main.smoothing)
  {
    dbg("All heads are local, auto-disabling smoothing");
    config.main.smoothing = false;
  }

  // Detect device manufacturers (NVIDIA/AMD)
  config.getPrimaryHead()->manufacturer = detectManufacturer(0, 0);

  for(int i=0;i<config.getNumAdditionalHeads();i++) {
    HEAD *h = config.getHead(i);
    h->manufacturer = detectManufacturer(h->devID, i+1);
  }
}

static int detectTransportType(int devID)
{

  IDirect3D9Ex *d3d = NULL;
  IDirect3DDevice9Ex *dev = NULL, *devSec = NULL;
  D3DPRESENT_PARAMETERS pp;
  int method = OUTMETHOD_BLIT;

  Direct3DCreate9Ex(-D3D_SDK_VERSION, &d3d);
  if(!d3d) {
    dbg("Using transport method BLIT for device %d (Direct3DCreate9Ex failed)", devID);
    return method;
  }

  WINDOWPARAMS wp = {0, 0, 10, 10, NULL, NULL, false, NULL};
  wp.hWnd = NULL;
  _beginthread(windowHandler, 0, (void*) &wp);
  while(!wp.hWnd)
    Sleep(1);

  // Create device
  pp.BackBufferWidth = 32;
  pp.BackBufferHeight = 32;
  pp.BackBufferFormat = D3DFMT_A8R8G8B8;
  pp.BackBufferCount = 0;
  pp.MultiSampleType = D3DMULTISAMPLE_NONE;
  pp.MultiSampleQuality = 0;
  pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  pp.hDeviceWindow = wp.hWnd;
  pp.Windowed  = true;
  pp.EnableAutoDepthStencil = false;
  pp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
  pp.Flags = 0;
  pp.FullScreen_RefreshRateInHz = 0;
  pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

  DWORD flags = D3DCREATE_MIXED_VERTEXPROCESSING|D3DCREATE_FPU_PRESERVE;
  d3d->CreateDeviceEx(devID, D3DDEVTYPE_HAL, wp.hWnd, flags, &pp, NULL, &dev);
  if(!dev) {
    DestroyWindow(wp.hWnd);
    dbg("Using transport method BLIT for device %d (CreateDeviceEx failed)", devID);
    return method;
  }

  // Device creation succeeded - we can at least use NONLOCAL
  method = nonlocalMethodDefault;

  // Create primary head device + shared rendertargets
  IDirect3DSurface9 *surfA = NULL, *surfB = NULL;
  HANDLE shareHandle = NULL;
  dev->CreateRenderTargetEx(32, 32, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, false, &surfA, &shareHandle, NULL);
  if(!surfA || !shareHandle) {
    DestroyWindow(wp.hWnd);
    dev->Release();
    d3d->Release();
    dbg("Using transport method NONLOCAL for device %d (CreateRenderTargetEx failed)", devID);
    return method;
  }

  d3d->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wp.hWnd, flags, &pp, NULL, &devSec);
  if(!devSec) {
    DestroyWindow(wp.hWnd);
    surfA->Release();
    dev->Release();
    d3d->Release();
    dbg("Using transport method NONLOCAL for device %d (CreateDeviceEx step 2 failed)", devID);
    return method;
  }

  devSec->CreateRenderTargetEx(32, 32, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0, false, &surfB, &shareHandle, NULL);
  if(!surfB) {
    surfA->Release();
    dev->Release();
    devSec->Release();
    d3d->Release();
    DestroyWindow(wp.hWnd);
    dbg("Using transport method NONLOCAL for device %d (CreateRenderTargetEx step 2 failed)", devID);
    return method;
  }

  // Sharing succeeded! local device!
  method = OUTMETHOD_LOCAL;
  surfB->Release();
  surfA->Release();
  dev->Release();
  devSec->Release();
  d3d->Release();
  DestroyWindow(wp.hWnd);
  DestroyWindow(wp.hWnd);
  dbg("Using transport method LOCAL for device %d", devID);
  return method;
}

bool IDirect3DDevice9SoftTH::validateSettings(IDirect3D9Ex *d3d)
{
  dbg("Validating settings");
  D3DCAPS9 caps;
  d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);

  int resX = config.main.renderResolution.x;
  int resY = config.main.renderResolution.y;

  if(resX < 2 || resY < 2) {
		ShowMessage("Invalid resolution (%dx%d)", resX, resY);
		return false;
  }

	if((DWORD)resX > caps.MaxTextureWidth) {
		ShowMessage("Requested resolution is too wide for device (%d > %d)", resX, caps.MaxTextureWidth);
		return false;
	}
	if((DWORD)resY > caps.MaxTextureHeight) {
		ShowMessage("Requested resolution is too tall for device (%d > %d)", resY, caps.MaxTextureHeight);
		return false;
	}

  int numDevs = config.getNumAdditionalHeads();
  for(int i=-1;i<numDevs;i++)
  {
    HEAD *h = i==-1?config.getPrimaryHead():config.getHead(i);
    if(i != -1)
    {
      if(h->devID == D3DADAPTER_DEFAULT) {
		    ShowMessage("Head %d device ID invalid (DevID 0 reserved for head_primary)", i+1);
		    return false;
	    }

      if(!d3d->GetAdapterMonitor(h->devID)) {
        ShowMessage("Head %d device ID invalid (%d)", i+1, h->devID);
		    return false;
	    }

      for(int o=0;o<numDevs;o++) {
        HEAD *hh = config.getHead(o);
        if(hh->devID == h->devID && o!=i) {
          ShowMessage("Head %d and %d: Device ID conflict (%d)", i+1,o+1, h->devID);
		      return false;
        }
      }
    }

    if(h->sourceRect.left < 0 || h->sourceRect.top < 0 || h->sourceRect.right > resX || h->sourceRect.bottom > resY) {
      ShowMessage("Head %d: sourceRect (%dx%d-%dx%d) outside\nrender surface (%dx%d)", i+1, h->sourceRect.left, h->sourceRect.top, h->sourceRect.right, h->sourceRect.bottom, resX, resY);
      return false;
    }

    dbg("Head %d: OK", i+1);
  }

  return true;
}
