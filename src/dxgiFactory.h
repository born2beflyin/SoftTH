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

#include "helper.h"
#include "main.h"

DEFINE_GUID(IID_IDXGIFactoryNew, 0xd9d267c0, 0x52d, 0x4a8f, 0x98, 0xf4, 0xbe, 0x28, 0xb2, 0xcc, 0x32, 0x74); // {D9D267C0-052D-4a8f-98F4-BE28B2CC3274}
//DEFINE_GUID(IID_IDXGIFactory1New, 0xc1bbaf12, 0x70f6, 0x4c47, 0xa3, 0x92, 0x2b, 0xc4, 0xf1, 0xc6, 0xf0, 0x49);
//DEFINE_GUID(IID_IDXGIFactory2New, 0xee85e851, 0x1363, 0x440d, 0x89, 0x82, 0xac, 0x5c, 0x56, 0x5a, 0x6, 0xea); // {EE85E851-1363-440d-8982-AC5C565A06EA}


/* IDXGIFactoryNew */
interface IDXGIFactoryNew : IDXGIFactory2
{
public:
  IDXGIFactoryNew(IDXGIFactory2 *dxgifNew);
  ~IDXGIFactoryNew();

  DECALE_DXGICOMMONIF(dxgif);

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgi_f2: 0x%08X QueryInterface %s", this, matchRiid(riid));
      if(riid == IID_IDXGIFactory ||
         riid == IID_IDXGIFactory1 ||
         riid == IID_IDXGIFactory2 ||
         riid == IID_IDXGIFactoryNew) {
        this->AddRef();
        *ppvObj = this;
        dbg("dxgi_f2: -- Got interface: %s 0x%08X", matchRiid(IID_IDXGIFactoryNew), *ppvObj);
        return S_OK;
      }
      dbg("dxgi_f2: -- Could not get interface");
      return dxgif->QueryInterface(riid, ppvObj);
  };

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxgi_f2: 0x%08X GetParent %s", this, matchRiid(riid));return dxgif->GetParent(riid, ppParent);};


  /* IDXGIFactory Interface */
  HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
    ;//{dbg("dxgi_f2: EnumAdapters");return dxgif->EnumAdapters(Adapter, ppAdapter);};
  HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags)
    {dbg("dxgi_f2: 0x%08X MakeWindowAssociation: hwnd = 0x%08X", this, WindowHandle);return dxgif->MakeWindowAssociation(WindowHandle, Flags);};
  HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND *pWindowHandle)
    {dbg("dxgi_f2: 0x%08X GetWindowAssociation", this);return dxgif->GetWindowAssociation(pWindowHandle);};
  HRESULT STDMETHODCALLTYPE CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain)
    ;//{dbg("dxgi_f2: CreateSwapChain");return dxgif->CreateSwapChain(pDevice, pDesc, ppSwapChain);};
  HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter **ppAdapter)
    {dbg("dxgi_f2: 0x%08X CreateSoftwareAdapter", this);return dxgif->CreateSoftwareAdapter(Module, ppAdapter);};


  /* IDXGIFactory1 Interface */
  HRESULT STDMETHODCALLTYPE EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter)
    ;//{dbg("dxgi_f2: EnumAdapters1");return dxgif->EnumAdapters1(Adapter, ppAdapter);};
  BOOL STDMETHODCALLTYPE IsCurrent()
    {dbg("dxgi_f2: 0x%08X IsCurrent", this);return dxgif->IsCurrent();};


  /* IDXGIFactory2 Interface */
  HRESULT STDMETHODCALLTYPE CreateSwapChainForComposition(IUnknown *pDevice,
                                                          const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                          IDXGIOutput *pRestrictToOutput,
                                                          IDXGISwapChain1 **ppSwapChain)
    {dbg("dxgi_f2: 0x%08X CreateSwapChainForComposition", this);return dxgif->CreateSwapChainForComposition(pDevice,
                                                                                               pDesc,
                                                                                               pRestrictToOutput,
                                                                                               ppSwapChain);};
  HRESULT STDMETHODCALLTYPE CreateSwapChainForCoreWindow(IUnknown *pDevice,
                                                         IUnknown *pWindow,
                                                         const DXGI_SWAP_CHAIN_DESC1 *pDesc,
                                                         IDXGIOutput *pRestrictToOutput,
                                                         IDXGISwapChain1 **ppSwapChain)
    {dbg("dxgi_f2: 0x%08X CreateSwapChainForCoreWindow", this);return dxgif->CreateSwapChainForCoreWindow(pDevice,
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
    {dbg("dxgi_f2: 0x%08X CreateSwapChainForHwnd", this);return dxgif->CreateSwapChainForHwnd(pDevice,
                                                                                 hWnd,
                                                                                 pDesc,
                                                                                 pFullscreenDesc,
                                                                                 pRestrictToOutput,
                                                                                 ppSwapChain);};
  HRESULT STDMETHODCALLTYPE GetSharedResourceAdapterLuid(HANDLE hResource, LUID *pLuid)
    {dbg("dxgi_f2: 0x%08X GetSharedResourceAdapterLuid", this);return dxgif->GetSharedResourceAdapterLuid(hResource,pLuid);};
  BOOL STDMETHODCALLTYPE IsWindowedStereoEnabled()
    {dbg("dxgi_f2: 0x%08X IsWindowedStereoEnabled", this);return dxgif->IsWindowedStereoEnabled();};
  HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusEvent(HANDLE hEvent, DWORD *pdwCookie)
    {dbg("dxgi_f2: 0x%08X RegisterOcclusionStatusEvent", this);return dxgif->RegisterOcclusionStatusEvent(hEvent,pdwCookie);};
  HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusWindow(HWND WindowHandle, UINT wMsg, DWORD *pdwCookie)
    {dbg("dxgi_f2: 0x%08X RegisterOcclusionStatusWindow", this);return dxgif->RegisterOcclusionStatusWindow(WindowHandle,wMsg,pdwCookie);};
  HRESULT STDMETHODCALLTYPE RegisterStereoStatusEvent(HANDLE hEvent, DWORD *pdwCookie)
    {dbg("dxgi_f2: 0x%08X RegisterStereoStatusEvent", this);return dxgif->RegisterStereoStatusEvent(hEvent,pdwCookie);};
  HRESULT STDMETHODCALLTYPE RegisterStereoStatusWindow(HWND WindowHandle, UINT wMsg, DWORD *pdwCookie)
    {dbg("dxgi_f2: 0x%08X RegisterStereoStatusWindow", this);return dxgif->RegisterStereoStatusWindow(WindowHandle,wMsg,pdwCookie);};
  void STDMETHODCALLTYPE UnregisterOcclusionStatus(DWORD dwCookie)
    {dbg("dxgi_f2: 0x%08X UnregisterOcclusionStatus", this);return dxgif->UnregisterOcclusionStatus(dwCookie);};
  void STDMETHODCALLTYPE UnregisterStereoStatus(DWORD dwCookie)
    {dbg("dxgi_f2: 0x%08X UnregisterStereoStatus", this);return dxgif->UnregisterStereoStatus(dwCookie);};

  IDXGIFactory2* getReal() {dbg("dxgi_f2: 0x%08X Get real factory 0x%08X", this, dxgif);return dxgif;};
private:
  IDXGIFactory2 *dxgif;
};


#endif
