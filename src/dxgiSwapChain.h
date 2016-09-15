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

#include "helper.h"
#include "main.h"
#include "InputHandler.h"

#include "outD3D11.h"
//#include <d3d10_1.h>
#include "outD3D10.h"
//#include "d3dSoftTH.h"
#include "dxgiAdapterOutput.h"
#include "dxgiFactory.h"


DEFINE_GUID(IID_IDXGISwapChainNew, 0x41ba0075, 0xbc7b, 0x4eee, 0x99, 0x8d, 0xb6, 0xdb, 0xb7, 0xba, 0xeb, 0x46);


// SoftTH device interface(s)
typedef struct {
  outDirect3D10  *output;
  ID3D10Texture2D *localSurf; // Local surface bound to shared handle of output
  HEAD *cfg;  // Pointer to configuration data
} OUTDEVICE10;

typedef struct {
  int headID, devID;
  ID3D10Texture2D *stagingSurf;
} STAGINGOUT10;

typedef struct {
  outDirect3D11  *output;
  ID3D11Texture2D *localSurf; // Local surface bound to shared handle of output
  HEAD *cfg;  // Pointer to configuration data
} OUTDEVICE11;

typedef struct {
  int headID, devID;
  ID3D11Texture2D *stagingSurf;
} STAGINGOUT11;

interface IDXGISwapChainNew : IDXGISwapChain1
{
public:
  //IDXGISwapChainNew(IDXGISwapChain *scApp, IDXGIFactory1 *parentNew, ID3D10Device *deviceNew, HWND winNew);
  //IDXGISwapChainNew(IDXGIFactory *parentNew, IDXGIFactory *dxgifNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd);
  //IDXGISwapChainNew(IDXGIFactory1 *parentNew, IDXGIFactory1 *dxgifNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd);
  IDXGISwapChainNew(IDXGIFactory2 *dxgifNew, IDXGIFactoryNew *parentNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd);
  ~IDXGISwapChainNew();

  DECALE_DXGICOMMONIF(dxgsc);

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgi_sc: 0x%08X QueryInterface %s", this, matchRiid(riid));
      if(riid == IID_IDXGISwapChain ||
         riid == IID_IDXGISwapChainNew ||
         riid == IID_IDXGIDeviceSubObject) {
        this->AddRef();
        *ppvObj = this;
        dbg("dxgi_sc: -- Got interface %s 0x%08X", matchRiid(IID_IDXGISwapChainNew), *ppvObj);
        return S_OK;
      }
      dbg("dxgi_sc: -- Couldn't get SoftTH interface");
      return dxgsc->QueryInterface(riid, ppvObj);
  };

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent) {
    dbg("dxgi_sc: 0x%08X GetParent %s", this, matchRiid(riid));
      if(riid == IID_IDXGIFactory ||
         riid == IID_IDXGIFactory1 ||
         riid == IID_IDXGIFactory2 ||
         riid == IID_IDXGIFactoryNew) {
        parent->AddRef();
        *ppParent = (IDXGIFactoryNew*) parent;
        dbg("dxgi_sc: -- Got parent %s 0x%08X", matchRiid(IID_IDXGIFactoryNew), *ppParent);
        return S_OK;
      }
      dbg("dxgi_sc: -- Couldn't get SoftTH parent");
      return dxgsc->GetParent(riid, ppParent);
    };

  // IDXGIDeviceSubObject
  HRESULT STDMETHODCALLTYPE GetDevice( REFIID riid, void **ppDevice) {
    //dbg("dxgi_sc: GetDevice");return dxgsc->GetDevice(riid, ppDevice);
    dbg("dxgi_sc: 0x%08X GetDevice", this);
    /*if (riid == IID_IDXGIDevice  ||
        riid == IID_IDXGIDevice1 ||
        riid == IID_ID3D11Device ||
        riid == IID_ID3D11Device1 //||
        //riid == IID_ID3D11Device2
        ) {
      dev11->AddRef();
      *ppDevice = dev11;
    } else if (riid == IID_ID3D10Device1) {
      dev10_1->AddRef();
      *ppDevice = dev10_1;
    } else if (riid == IID_ID3D10Device) {
      dev10->AddRef();
      *ppDevice = dev10;
    }*/
    if (dev10)        *ppDevice = dev10;
    else if (dev10_1) *ppDevice = dev10_1;
    else if (dev11)   *ppDevice = dev11;
    if (ppDevice) {
      dbg("dxgi_sc: -- Got device %s 0x%08X",matchRiid(riid), *ppDevice);
      return S_OK;
    }
    dbg("dxgi_sc: -- Couldn't get device")  ;
    return dxgsc->GetDevice(riid, ppDevice);
  };

  /* IDXGISwapChain */
  HRESULT STDMETHODCALLTYPE Present(UINT SyncInterval,UINT Flags)
    ;//{dbg("dxgsc: Present");return dxgsc->Present(SyncInterval, Flags);};
  HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID riid, void **ppSurface)
    ;//{dbg("dxgsc: GetBuffer");HRESULT ret = dxgsc->GetBuffer(Buffer, riid, ppSurface);dbg("joo-o"); return ret;};
  HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL Fullscreen, IDXGIOutput *pTarget)
    {
      dbg("dxgi_sc: 0x%08X SetFullscreenState - Target = 0x%08X", this, pTarget);
      if(pTarget) {
        IDXGIOutputNew *onew;
        if(pTarget->QueryInterface(IID_IDXGIOutputNew, (void**) &onew) == S_OK) {
          pTarget = onew->getReal();
          onew->Release();
        }
      }
      return dxgsc->SetFullscreenState(Fullscreen, pTarget);
    };
  HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL *pFullscreen,IDXGIOutput **ppTarget)
    {
      //IDXGIOutput *pTarget;
      //pTarget = *ppTarget;
      dbg("dxgi_sc: 0x%08X GetFullscreenState: target = 0x%08X", this, *ppTarget);
      /*if(pTarget) {
        IDXGIOutputNew *onew;
        if(pTarget->QueryInterface(IID_IDXGIOutputNew, (void**) &onew) == S_OK) {
          pTarget = onew->getReal();
          ppTarget = &pTarget;
          onew->Release();
        }
      }*/
      return dxgsc->GetFullscreenState(pFullscreen, ppTarget);
    };
  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc)
    ;//{dbg("dxgsc: GetDesc");return dxgsc->GetDesc(pDesc);};
  HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT BufferCount,UINT Width,UINT Height,DXGI_FORMAT NewFormat,UINT SwapChainFlags)
    ;//{dbg("dxgsc: ResizeBuffers %dx%d", Width, Height);return dxgsc->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);};
  HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC *tgtp)
    ;//{dbg("dxgsc: ResizeTarget %dx%d", tgtp->Width, tgtp->Height);return dxgsc->ResizeTarget(tgtp);};
  HRESULT STDMETHODCALLTYPE GetContainingOutput(IDXGIOutput **ppOutput)
    {dbg("dxgi_sc: 0x%08X GetContainingOutput", this);return dxgsc->GetContainingOutput(ppOutput);};
  HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats)
    {dbg("dxgi_sc: 0x%08X GetFrameStatistics", this);return dxgsc->GetFrameStatistics(pStats);};
  HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT *pLastPresentCount)
    {dbg("dxgi_sc: 0x%08X GetLastPresentCount", this);return dxgsc->GetLastPresentCount(pLastPresentCount);};

  /* IDXGISwapChain1 */
  HRESULT STDMETHODCALLTYPE GetDesc1(DXGI_SWAP_CHAIN_DESC1 *pDesc)
    ;//{dbg("dxgsc: GetDesc");return dxgsc->GetDesc(pDesc);};
  HRESULT STDMETHODCALLTYPE GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pDesc)
    {dbg("dxgi_sc: 0x%08X GetFullscreenDesc", this);return dxgsc->GetFullscreenDesc(pDesc);};
  HRESULT STDMETHODCALLTYPE GetHwnd(HWND *pHwnd)
    {dbg("dxgi_sc: 0x%08X GetHwnd", this);return dxgsc->GetHwnd(pHwnd);};
  HRESULT STDMETHODCALLTYPE GetCoreWindow(REFIID refiid, _COM_Outptr_  void **ppUnk)
    {dbg("dxgi_sc: 0x%08X GetCoreWindow", this);return dxgsc->GetCoreWindow(refiid, ppUnk);};
  HRESULT STDMETHODCALLTYPE Present1(UINT SyncInterval, UINT PresentFlags,const DXGI_PRESENT_PARAMETERS *pPresentParameters)
    {dbg("dxgi_sc: 0x%08X Present1", this);return dxgsc->Present1(SyncInterval, PresentFlags, pPresentParameters);};
  BOOL STDMETHODCALLTYPE IsTemporaryMonoSupported( void)
    {dbg("dxgi_sc: 0x%08X IsTemporaryMonoSupported", this);return dxgsc->IsTemporaryMonoSupported();};
  HRESULT STDMETHODCALLTYPE GetRestrictToOutput(IDXGIOutput **ppRestrictToOutput)
    {dbg("dxgi_sc: 0x%08X GetRestrictToOutput", this);return dxgsc->GetRestrictToOutput(ppRestrictToOutput);};
  HRESULT STDMETHODCALLTYPE SetBackgroundColor(const DXGI_RGBA *pColor)
    {dbg("dxgi_sc: 0x%08X SetBackgroundColor", this);return dxgsc->SetBackgroundColor(pColor);};
  HRESULT STDMETHODCALLTYPE GetBackgroundColor(DXGI_RGBA *pColor)
    {dbg("dxgi_sc: 0x%08X GetBackgroundColor", this);return dxgsc->GetBackgroundColor(pColor);};
  HRESULT STDMETHODCALLTYPE SetRotation(DXGI_MODE_ROTATION Rotation)
    {dbg("dxgi_sc: 0x%08X SetRotation", this);return dxgsc->SetRotation(Rotation);};
  HRESULT STDMETHODCALLTYPE GetRotation(DXGI_MODE_ROTATION *pRotation)
    {dbg("dxgi_sc: 0x%08X GetRotation", this);return dxgsc->GetRotation(pRotation);};

  IDXGISwapChain1* getReal() {dbg("dxgi_sc: 0x%08X Get real swapchain 0x%08X", this, &dxgsc);return dxgsc;};

private:
  bool has_nonlocal = false;

  IDXGISwapChain1   *dxgsc;
  IDXGIFactoryNew   *parent;
  IDXGIFactory2     *dxgif;
  ID3D10Device      *dev10;
  ID3D10Device1     *dev10_1;
  ID3D11Device      *dev11;
  //ID3D12Device      *dev12;
  HWND               win;

  ID3D10Texture2D         *newbb10;         // New backbuffer (full size)
  D3D10_TEXTURE2D_DESC     newbbDesc10;
  ID3D10Texture2D         *realbb10;        // Real backbuffer (one monitor)
  D3D10_TEXTURE2D_DESC     realbbDesc10;

  ID3D11Texture2D         *newbb11;         // New backbuffer (full size)
  D3D11_TEXTURE2D_DESC     newbbDesc11;
  ID3D11Texture2D         *realbb11;        // Real backbuffer (one monitor)
  D3D11_TEXTURE2D_DESC     realbbDesc11;
  ID3D11Texture2D         *stagingSurfs11;  // staging surfaces for each non-local output

  //ID3D12Texture2D         *newbb12;         // New backbuffer (full size)
  //D3D12_TEXTURE2D_DESC     newbbDesc12;
  //ID3D12Texture2D         *realbb12;        // Real backbuffer (one monitor)
  //D3D12_TEXTURE2D_DESC     realbbDesc12;

  void preUpdateBB(UINT *width, UINT *height);
  void updateBB();  // Updates backbuffer data

  int numDevs;
  OUTDEVICE10   *outDevs10;
  OUTDEVICE11   *outDevs11;
  //OUTDEVICE12   *outDevs12;

  STAGINGOUT10   *stagingOuts10;
  STAGINGOUT11   *stagingOuts11;


};

#endif
