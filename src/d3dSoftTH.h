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

#ifndef __D3DSOFTTH_H__
#define __D3DSOFTTH_H__

#include "d3d.h"
#include "outD3D9.h"
#include "configFile.h"
#include "d3dDrawing.h"
#include "shcopy.h"

#define USE_D3DX

#define NUM_FPS_GRAPH 175

#ifdef USE_D3DX
#include <d3dx9.h>
#pragma comment(lib, "d3dx9")
#endif

#ifdef USE_D3DX
#define D3DX(x) {x;}
#else
#define D3DX(x) // x
#endif


// Global helpers
bool isSoftTHmode(int w, int h); // Return true if resolution is SoftTH mode
volatile extern int SoftTHActive; // >0 if SoftTH is currently active and resolution is overridden
extern bool *SoftTHActiveSquashed; // Pointer to latest SoftTH device squash variable (TODO: this is horrible)

// SoftTH device interface(s)
typedef struct {
  outDirect3D9  *output;
  IDirect3DSurface9 *localSurfA; // Local surface bound to shared handle of output
  IDirect3DSurface9 *localSurfB;
  IDirect3DTexture9 *tempTex; // Temporary texture for shader copy effects
  HEAD *cfg;  // Pointer to configuration data
} OUTDEVICE;

DEFINE_GUID(IID_IDirect3DDevice9SoftTH, 0xb18b10ce, 0x2649, 0x405a, 0x87, 0xf, 0x95, 0xf7, 0xaa, 0xbb, 0xcc, 0xdd);

interface IDirect3DDevice9SoftTH : public IDirect3DDevice9New
{
  IDirect3DDevice9SoftTH(IDirect3D9New *parentNew, IDirect3D9Ex *direct3D, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pp);
  ~IDirect3DDevice9SoftTH();
  STDMETHOD_(ULONG,Release)(THIS);
/*    STDMETHOD(TestCooperativeLevel)(THIS) {
      HRESULT ret = dev->TestCooperativeLevel();
      dbg("dev: TestCooperativeLevel: %s", getD3DError(ret));
      return ret;
    };*/
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
    {
      dbgf("IDirect3DDevice9SoftTH: QUERYINTERFACE");
      if(riid == IID_IDirect3DDevice9SoftTH) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      }
      if(riid == IID_IDirect3DDevice9 || (isEx && riid == IID_IDirect3DDevice9Ex)) {
        dbgf("IDirect3DDevice9SoftTH: QUERYINTERFACE: %s", riid==IID_IDirect3DDevice9?"IID_IDirect3DDevice9":"IID_IDirect3DDevice9Ex");
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      }
      return __super::QueryInterface(riid, ppvObj);
    };

  STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget);
  STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);
  STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
    {return PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, NULL);};
  STDMETHOD(PresentEx)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags);
  STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
  STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode);
  STDMETHOD(Clear)(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil);
  STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
  STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain,IDirect3DSwapChain9** pSwapChain);
  STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp);

  /*
  STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
    {
      dbg("dev: GetRenderTargetData");
      dbg("dev: GetRenderTargetData source:");
      dumpSurfaceDesc(pRenderTarget);
      dbg("dev: GetRenderTargetData target:");
      dumpSurfaceDesc(pDestSurface);
      HRESULT ret = dev->GetRenderTargetData(OriginalFromNewSurface(pRenderTarget), OriginalFromNewSurface(pDestSurface));
      dbg("dev: GetRenderTargetData result: %s", getD3DError(ret));
      return ret;
  };
  STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
    {
      dbg("dev: CreateOffscreenPlainSurface %dx%d %s %s %d", Width, Height, getMode(Format), getPool(Pool), pSharedHandle);
      CHECKPOOL(Pool);
      HRESULT ret = dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
      dbg("dev: CreateOffscreenPlainSurface result: %s", getD3DError(ret));
      return ret;
  };*/

  HRESULT getCreateDeviceResult() {return createDeviceResult;};
  HWND getDeviceWindow() {return hFocusWindow;};
private:

  int matchRefresh(D3DPRESENT_PARAMETERS *pp);

  IDirect3D9New *parent;

  HRESULT createDeviceResult;
  D3DPRESENT_PARAMETERS lastPp; // Latest present parameters

  void adjustPP(D3DPRESENT_PARAMETERS* pp);  // Adjusts present_parameters for SoftTH
  void createBuffers();  // Create new rendertarget etc.
  void destroyBuffers(); // Restore to stock device
  void saveScreenshot();
  void drawOverlay();
  bool validateSettings(IDirect3D9Ex *d3d);
  int getRefs() {dev->AddRef();return dev->Release();};

  D3DSURFACE_DESC bbDesc, newbbDesc;  // Descriptors of real backbuffer and new backbuffer
  IDirect3DSurface9 *bb;  // Real device backbuffer
  IDirect3DSurface9 *newbb; // New backbuffer
  IDirect3DSurface9 *newdepth; // New backbuffer

#if !USE_D3DEX
  // This is just to fool the compiler - SoftTH device wont work without Ex device!
  IDirect3D9Ex *d3d;
  IDirect3DDevice9Ex *dev;
#endif

  bool postprocess;
  IDirect3DTexture9 *bbtex; // Postprocess texture (owner of ppaux)
  IDirect3DSurface9 *ppaux; // Postprocess buffer

  bool curSurfA;  // true = use surf A, false = use B
  IDirect3DQuery9* squeryA, *squeryB;
  d3dDrawing* drawing;

  IDirect3DSurface9 *copybuf;

  int numDevs;
  OUTDEVICE *outDevs;

  int FPS;
  int fCounter;
  int fTimer;

  HWND hFocusWindow;
  int wantedX, wantedY;
  D3DMULTISAMPLE_TYPE msWanted;
  DWORD msQWanted;
  bool depthWanted;
  D3DFORMAT depthFormatWanted;
  bool discardDepth;
  bool lockableBB;
  bool fpuPreserve;

  int refs;   // Additional references created to device
  RECT wpos;  // Device window position

#ifdef USE_D3DX
  ID3DXFont *font;
  ID3DXFont *fontWide; // Wide font for BB drawing TODO: why is this required?
#endif

  bool squash, nocopy;
  bool showGraph, showLog;

  float fpsGraph[NUM_FPS_GRAPH];  // Framerate graph data
  double lastFrameTime;

  shCopy *copyPack24to32; // shader copy effect, packs RGB data to RGBA surface
  shCopy *copyDither; // shader copy effect, dithers RGB888 to RGB565

  // On-screen notification messages
  void printMessage(char *first, ...);
  char message[512];
  DWORD messageTime;
};

// SoftTH Swapchain: need to override present and getbackbuffer here as well
interface IDirect3DSwapChain9SoftTH : public IDirect3DSwapChain9
{
  IDirect3DSwapChain9SoftTH(IDirect3DDevice9SoftTH *device, IDirect3DSwapChain9* SwapChain) {
    dev = device;
    sc = SwapChain;
  }

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
      {return sc->QueryInterface(riid, ppvObj);};
    STDMETHOD_(ULONG,AddRef)(THIS)
      {return sc->AddRef();};
    STDMETHOD_(ULONG,Release)(THIS)
      {
        int r = sc->Release();
        if(r == 0) {
          dbgf("Released IDirect3DSwapChain9SoftTH");
          delete this;
        }
        return r;
      };

    STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags)
      {return dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);}; // from SoftTH device
    STDMETHOD(GetFrontBufferData)(THIS_ IDirect3DSurface9* pDestSurface)
      {return sc->GetFrontBufferData(pDestSurface);};
    STDMETHOD(GetBackBuffer)(THIS_ UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
      {return dev->GetBackBuffer(0, iBackBuffer, Type, ppBackBuffer);};  // from SoftTH device
    STDMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus)
      {return sc->GetRasterStatus(pRasterStatus);};
    STDMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode)
      {return dev->GetDisplayMode(0, pMode);};  // from SoftTH device
    STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)
      {*ppDevice = dev;dev->AddRef();return D3D_OK;};
    STDMETHOD(GetPresentParameters)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
      {return sc->GetPresentParameters(pPresentationParameters);};

private:
  IDirect3DDevice9SoftTH *dev;
  IDirect3DSwapChain9 *sc;
};

int detectTransportType(int devID);
void detectTransportMethods();

#endif
