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

// DXGI Swap chain class

#ifndef __DXGISWAPCHAIN_H__
#define __DXGISWAPCHAIN_H__

#include <dxgi.h>
#include "helper.h"
#include "configFile.h"

#include <d3d11.h>
#include "outD3D11.h"
//#include <d3d10_1.h>
#include "outD3D10.h"
//#include "d3dSoftTH.h"
#include "dxgiAdapterOutput.h"

// SoftTH device interface(s)
typedef struct {
  outDirect3D10  *output;
  ID3D10Texture2D *localSurf; // Local surface bound to shared handle of output
  HEAD *cfg;  // Pointer to configuration data
} OUTDEVICE10;

typedef struct {
  outDirect3D11  *output;
  ID3D11Texture2D *localSurf; // Local surface bound to shared handle of output
  HEAD *cfg;  // Pointer to configuration data
} OUTDEVICE11;

DEFINE_GUID(IID_IDXGISwapChainNew, 0x41ba0075, 0xbc7b, 0x4eee, 0x99, 0x8d, 0xb6, 0xdb, 0xb7, 0xba, 0xeb, 0x46);

interface IDXGISwapChainNew : IDXGISwapChain
{
public:
  //IDXGISwapChainNew(IDXGISwapChain *scApp, IDXGIFactory1 *parentNew, ID3D10Device *deviceNew, HWND winNew);
  IDXGISwapChainNew(IDXGIFactory *parentNew, IDXGIFactory *dxgifNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd);
  IDXGISwapChainNew(IDXGIFactory1 *parentNew, IDXGIFactory1 *dxgifNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd);
  ~IDXGISwapChainNew();

  DECALE_DXGICOMMONIF(dxgsc);

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxgsc: GetParent %s 0x%08X", matchRiid(riid), *ppParent);
      if(riid == IID_IDXGIFactory) {
        *ppParent = parent;
        parent->AddRef();
        return S_OK;
      }
      return dxgsc->GetParent(riid, ppParent);
    };

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgsc: QueryInterface %s 0x%08X", matchRiid(riid), *ppvObj);
      if(riid == IID_IDXGISwapChainNew) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return dxgsc->QueryInterface(riid, ppvObj);
  };

  // IDXGIDeviceSubObject
  HRESULT STDMETHODCALLTYPE GetDevice( REFIID riid, void **ppDevice)
    {dbg("dxgsc: GetDevice");return dxgsc->GetDevice(riid, ppDevice);};

  HRESULT STDMETHODCALLTYPE Present(UINT SyncInterval,UINT Flags)
    ;//{dbg("dxgsc: Present");return dxgsc->Present(SyncInterval, Flags);};
  HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID riid, void **ppSurface)
    ;//{dbg("dxgsc: GetBuffer");HRESULT ret = dxgsc->GetBuffer(Buffer, riid, ppSurface);dbg("joo-o"); return ret;};
  HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL Fullscreen, IDXGIOutput *pTarget)
    {dbg("dxgsc: SetFullscreenState 0x%08X", pTarget);
    /*if(pTarget) {
      dbg("test here");
      IDXGIOutputNew *onew;
      if(pTarget->QueryInterface(IID_IDXGIOutputNew, (void**) &onew) == S_OK) {
        pTarget = onew->getReal();
        onew->Release();
      }
    }*/
    return dxgsc->SetFullscreenState(Fullscreen, pTarget);
    };
  HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL *pFullscreen,IDXGIOutput **ppTarget)
    {dbg("dxgsc: WARNING: GetFullscreenState, not implemented!");return dxgsc->GetFullscreenState(pFullscreen, ppTarget);};
  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc)
    ;//{dbg("dxgsc: GetDesc");return dxgsc->GetDesc(pDesc);};
  HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT BufferCount,UINT Width,UINT Height,DXGI_FORMAT NewFormat,UINT SwapChainFlags)
    ;//{dbg("dxgsc: ResizeBuffers %dx%d", Width, Height);return dxgsc->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);};
  HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC *tgtp)
    ;//{dbg("dxgsc: ResizeTarget %dx%d", tgtp->Width, tgtp->Height);return dxgsc->ResizeTarget(tgtp);};
  HRESULT STDMETHODCALLTYPE GetContainingOutput(IDXGIOutput **ppOutput)
    {dbg("dxgsc: GetContainingOutput");return dxgsc->GetContainingOutput(ppOutput);};
  HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats)
    {dbg("dxgsc: GetFrameStatistics");return dxgsc->GetFrameStatistics(pStats);};
  HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT *pLastPresentCount)
    {dbg("dxgsc: GetLastPresentCount");return dxgsc->GetLastPresentCount(pLastPresentCount);};

  IDXGISwapChain* getReal() {dbg("Get real swapchain 0x%08X", &dxgsc);return dxgsc;};

private:
  IDXGISwapChain *dxgsc;
  IDXGIFactory1 *parent;
  IDXGIFactory1 *dxgif;
  ID3D10Device *dev10;
  ID3D10Device1 *dev10_1;
  ID3D11Device *dev11;
  //ID3D12Device *dev12;
  HWND win;

  ID3D10Texture2D *newbb10; // New backbuffer (full size)
  D3D10_TEXTURE2D_DESC newbbDesc10;
  ID3D10Texture2D *realbb10; // Real backbuffer (one monitor)
  D3D10_TEXTURE2D_DESC realbbDesc10;

  ID3D11Texture2D *newbb11; // New backbuffer (full size)
  D3D11_TEXTURE2D_DESC newbbDesc11;
  ID3D11Texture2D *realbb11; // Real backbuffer (one monitor)
  D3D11_TEXTURE2D_DESC realbbDesc11;

  //ID3D12Texture2D *newbb12; // New backbuffer (full size)
  //D3D12_TEXTURE2D_DESC newbbDesc12;
  //ID3D12Texture2D *realbb12; // Real backbuffer (one monitor)
  //D3D12_TEXTURE2D_DESC realbbDesc12;

  void preUpdateBB(UINT *width, UINT *height);
  void updateBB();  // Updates backbuffer data

  int numDevs;
  OUTDEVICE10   *outDevs10;
  OUTDEVICE11   *outDevs11;
  //OUTDEVICE12   *outDevs12;

};

#endif
