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

#include <dxgi.h>
#include "helper.h"

DEFINE_GUID(IID_IDXGIFactory1New, 0xc1bbaf12, 0x70f6, 0x4c47, 0xa3, 0x92, 0x2b, 0xc4, 0xf1, 0xc6, 0xf0, 0x49);

interface IDXGIFactory1New : IDXGIFactory1
{
public:
  IDXGIFactory1New(IDXGIFactory1 *dxgifNew);
  ~IDXGIFactory1New();

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgif: QueryInterface %s", matchRiid(riid));
      if(riid == IID_IDXGIFactory1New) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return dxgif->QueryInterface(riid, ppvObj);
  };

  DECALE_DXGICOMMONIF(dxgif);

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxgif: GetParent %s", matchRiid(riid));return dxgif->GetParent(riid, ppParent);};

  HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
    ;//{dbg("dxgif: EnumAdapters");return dxgif->EnumAdapters(Adapter, ppAdapter);};  
  HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags)
    {dbg("dxgif: MakeWindowAssociation");return dxgif->MakeWindowAssociation(WindowHandle, Flags);};  
  HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND *pWindowHandle)
    {dbg("dxgif: GetWindowAssociation");return dxgif->GetWindowAssociation(pWindowHandle);};  
  HRESULT STDMETHODCALLTYPE CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *pDesc, IDXGISwapChain **ppSwapChain)
    ;//{dbg("dxgif: CreateSwapChain");return dxgif->CreateSwapChain(pDevice, pDesc, ppSwapChain);};  
  HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(HMODULE Module, IDXGIAdapter **ppAdapter)
    {dbg("dxgif: CreateSoftwareAdapter");return dxgif->CreateSoftwareAdapter(Module, ppAdapter);};  
  HRESULT STDMETHODCALLTYPE EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter)
    ;//{dbg("dxgif: EnumAdapters1");return dxgif->EnumAdapters1(Adapter, ppAdapter);};  
  BOOL STDMETHODCALLTYPE IsCurrent()
    {dbg("dxgif: IsCurrent");return dxgif->IsCurrent();};

  IDXGIFactory1* getReal() {dbg("Get real factory");return dxgif;};
private:
  IDXGIFactory1 *dxgif;
};

#endif