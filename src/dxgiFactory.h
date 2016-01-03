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

#ifndef __DXGIFACTORY_H__
#define __DXGIFACTORY_H__

#include <dxgi1_3.h>

#include "helper.h"

DEFINE_GUID(IID_IDXGIFactoryNew, 0xd9d267c0, 0x52d, 0x4a8f, 0x98, 0xf4, 0xbe, 0x28, 0xb2, 0xcc, 0x32, 0x74); // {D9D267C0-052D-4a8f-98F4-BE28B2CC3274}
DEFINE_GUID(IID_IDXGIFactory1New, 0xc1bbaf12, 0x70f6, 0x4c47, 0xa3, 0x92, 0x2b, 0xc4, 0xf1, 0xc6, 0xf0, 0x49);
DEFINE_GUID(IID_IDXGIFactory2New, 0xee85e851, 0x1363, 0x440d, 0x89, 0x82, 0xac, 0x5c, 0x56, 0x5a, 0x6, 0xea); // {EE85E851-1363-440d-8982-AC5C565A06EA}



/* IDXGIFactoryNew */
interface IDXGIFactoryNew : IDXGIFactory
{
public:
  IDXGIFactoryNew(IDXGIFactory *dxgifNew);
  ~IDXGIFactoryNew();

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgi_f: QueryInterface %s 0x%08X", matchRiid(riid), *ppvObj);
      if(riid == IID_IDXGIFactoryNew) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return dxgif->QueryInterface(riid, ppvObj);
  };

  DECALE_DXGICOMMONIF(dxgif);

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxgi_f: GetParent %s", matchRiid(riid));return dxgif->GetParent(riid, ppParent);};

  HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
    ;//{dbg("dxgi_f: EnumAdapters");return dxgif->EnumAdapters(Adapter, ppAdapter);};
  HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags)
    {dbg("dxgi_f: MakeWindowAssociation");return dxgif->MakeWindowAssociation(WindowHandle, Flags);};
  HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND *pWindowHandle)
    {dbg("dxgi_f: GetWindowAssociation");return dxgif->GetWindowAssociation(pWindowHandle);};
  HRESULT STDMETHODCALLTYPE CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain)
    ;//{dbg("dxgi_f: CreateSwapChain");return dxgif->CreateSwapChain(pDevice, pDesc, ppSwapChain);};
  HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter **ppAdapter)
    {dbg("dxgi_f: CreateSoftwareAdapter");return dxgif->CreateSoftwareAdapter(Module, ppAdapter);};

  IDXGIFactory* getReal() {dbg("Get real factory 0x%08X", dxgif);return dxgif;};
private:
  IDXGIFactory  *dxgif;
};



/* IDXGIFactory1New */
interface IDXGIFactory1New : IDXGIFactory1
{
public:
  IDXGIFactory1New(IDXGIFactory1 *dxgifNew);
  ~IDXGIFactory1New();

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgi_f1: QueryInterface %s 0x%08X", matchRiid(riid), *ppvObj);
      if(riid == IID_IDXGIFactory1New) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
      return dxgif->QueryInterface(riid, ppvObj);
  };

  DECALE_DXGICOMMONIF(dxgif);

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxgi_f1: GetParent %s", matchRiid(riid));return dxgif->GetParent(riid, ppParent);};

  HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
    ;//{dbg("dxgi_f1: EnumAdapters");return dxgif->EnumAdapters(Adapter, ppAdapter);};
  HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags)
    {dbg("dxgi_f1: MakeWindowAssociation");return dxgif->MakeWindowAssociation(WindowHandle, Flags);};
  HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND *pWindowHandle)
    {dbg("dxgi_f1: GetWindowAssociation");return dxgif->GetWindowAssociation(pWindowHandle);};
  HRESULT STDMETHODCALLTYPE CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain)
    ;//{dbg("dxgi_f1: CreateSwapChain");return dxgif->CreateSwapChain(pDevice, pDesc, ppSwapChain);};
  HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter **ppAdapter)
    {dbg("dxgi_f1: CreateSoftwareAdapter");return dxgif->CreateSoftwareAdapter(Module, ppAdapter);};

  HRESULT STDMETHODCALLTYPE EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter)
    ;//{dbg("dxgi_f1: EnumAdapters1");return dxgif->EnumAdapters1(Adapter, ppAdapter);};
  BOOL STDMETHODCALLTYPE IsCurrent()
    {dbg("dxgi_f1: IsCurrent");return dxgif->IsCurrent();};

  IDXGIFactory1* getReal() {dbg("Get real factory 0x%08X", dxgif);return dxgif;};
private:
  IDXGIFactory1 *dxgif;
};



/* IDXGIFactory2New */
interface IDXGIFactory2New : IDXGIFactory2
{
public:
  IDXGIFactory2New(IDXGIFactory2 *dxgifNew);
  ~IDXGIFactory2New();

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgi_f2: QueryInterface %s 0x%08X", matchRiid(riid), *ppvObj);
      if(riid == IID_IDXGIFactory2New) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
      return dxgif->QueryInterface(riid, ppvObj);
  };

  DECALE_DXGICOMMONIF(dxgif);

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxgi_f2: GetParent %s", matchRiid(riid));return dxgif->GetParent(riid, ppParent);};

  HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
    ;//{dbg("dxgi_f2: EnumAdapters");return dxgif->EnumAdapters(Adapter, ppAdapter);};
  HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags)
    {dbg("dxgi_f2: MakeWindowAssociation");return dxgif->MakeWindowAssociation(WindowHandle, Flags);};
  HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND *pWindowHandle)
    {dbg("dxgi_f2: GetWindowAssociation");return dxgif->GetWindowAssociation(pWindowHandle);};
  HRESULT STDMETHODCALLTYPE CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain)
    ;//{dbg("dxgi_f2: CreateSwapChain");return dxgif->CreateSwapChain(pDevice, pDesc, ppSwapChain);};
  HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter **ppAdapter)
    {dbg("dxgi_f2: CreateSoftwareAdapter");return dxgif->CreateSoftwareAdapter(Module, ppAdapter);};

  HRESULT STDMETHODCALLTYPE EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter)
    ;//{dbg("dxgi_f2: EnumAdapters1");return dxgif->EnumAdapters1(Adapter, ppAdapter);};
  BOOL STDMETHODCALLTYPE IsCurrent()
    {dbg("dxgi_f2: IsCurrent");return dxgif->IsCurrent();};

  HRESULT STDMETHODCALLTYPE CreateSwapChainForComposition(IUnknown *pDevice,
                                                          const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                          IDXGIOutput *pRestrictToOutput,
                                                          IDXGISwapChain1 **ppSwapChain)
    {dbg("dxgi_f2: CreateSwapChainForComposition");return dxgif->CreateSwapChainForComposition(pDevice,
                                                                                               pDesc,
                                                                                               pRestrictToOutput,
                                                                                               ppSwapChain);};
  HRESULT STDMETHODCALLTYPE CreateSwapChainForCoreWindow(IUnknown *pDevice,
                                                         IUnknown *pWindow,
                                                         const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                         IDXGIOutput *pRestrictToOutput,
                                                         IDXGISwapChain1 **ppSwapChain)
    {dbg("dxgi_f2: CreateSwapChainForCoreWindow");return dxgif->CreateSwapChainForCoreWindow(pDevice,
                                                                                             pWindow,
                                                                                             pDesc,
                                                                                             pRestrictToOutput,
                                                                                             ppSwapChain);};
  HRESULT STDMETHODCALLTYPE CreateSwapChainForHwnd(IUnknown *pDevice,
                                                   HWND hWnd,
                                                   const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                   const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc,
                                                   IDXGIOutput *pRestrictToOutput,
                                                   IDXGISwapChain1 **ppSwapChain)
    {dbg("dxgi_f2: CreateSwapChainForHwnd");return dxgif->CreateSwapChainForHwnd(pDevice,
                                                                                 hWnd,
                                                                                 pDesc,
                                                                                 pFullscreenDesc,
                                                                                 pRestrictToOutput,
                                                                                 ppSwapChain);};
  HRESULT STDMETHODCALLTYPE GetSharedResourceAdapterLuid(HANDLE hResource, LUID *pLuid)
    {dbg("dxgi_f2: GetSharedResourceAdapterLuid");return dxgif->GetSharedResourceAdapterLuid(hResource,pLuid);};
  BOOL STDMETHODCALLTYPE IsWindowedStereoEnabled()
    {dbg("dxgi_f2: IsWindowedStereoEnabled");return dxgif->IsWindowedStereoEnabled();};
  HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusEvent(HANDLE hEvent, DWORD *pdwCookie)
    {dbg("dxgi_f2: RegisterOcclusionStatusEvent");return dxgif->RegisterOcclusionStatusEvent(hEvent,pdwCookie);};
  HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusWindow(HWND WindowHandle, UINT wMsg, DWORD *pdwCookie)
    {dbg("dxgi_f2: RegisterOcclusionStatusWindow");return dxgif->RegisterOcclusionStatusWindow(WindowHandle,wMsg,pdwCookie);};
  HRESULT STDMETHODCALLTYPE RegisterStereoStatusEvent(HANDLE hEvent, DWORD *pdwCookie)
    {dbg("dxgi_f2: RegisterStereoStatusEvent");return dxgif->RegisterStereoStatusEvent(hEvent,pdwCookie);};
  HRESULT STDMETHODCALLTYPE RegisterStereoStatusWindow(HWND WindowHandle, UINT wMsg, DWORD *pdwCookie)
    {dbg("dxgi_f2: RegisterStereoStatusWindow");return dxgif->RegisterStereoStatusWindow(WindowHandle,wMsg,pdwCookie);};
  void STDMETHODCALLTYPE UnregisterOcclusionStatus(DWORD dwCookie)
    {dbg("dxgi_f2: UnregisterOcclusionStatus");return dxgif->UnregisterOcclusionStatus(dwCookie);};
  void STDMETHODCALLTYPE UnregisterStereoStatus(DWORD dwCookie)
    {dbg("dxgi_f2: UnregisterStereoStatus");return dxgif->UnregisterStereoStatus(dwCookie);};

  IDXGIFactory2* getReal() {dbg("Get real factory 0x%08X", dxgif);return dxgif;};
private:
  IDXGIFactory2 *dxgif;
};


#endif
