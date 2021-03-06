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

#include "dxgiFactory.h"
#include "dxgiAdapterOutput.h"
#include "dxgiSwapChain.h"
#include "helper.h"
#include "main.h"

#include <dxgi1_3.h>

#include <d3d11.h>

#include <INITGUID.H>
DEFINE_GUID(IID_IDXGIFactoryNew, 0xd9d267c0, 0x52d, 0x4a8f, 0x98, 0xf4, 0xbe, 0x28, 0xb2, 0xcc, 0x32, 0x74); // {D9D267C0-052D-4a8f-98F4-BE28B2CC3274}
DEFINE_GUID(IID_IDXGIFactory1New, 0xc1bbaf12, 0x70f6, 0x4c47, 0xa3, 0x92, 0x2b, 0xc4, 0xf1, 0xc6, 0xf0, 0x49);
DEFINE_GUID(IID_IDXGIFactory2New, 0xee85e851, 0x1363, 0x440d, 0x89, 0x82, 0xac, 0x5c, 0x56, 0x5a, 0x6, 0xea); // {EE85E851-1363-440d-8982-AC5C565A06EA}


/* IDXGIFactoryNew */
IDXGIFactoryNew::IDXGIFactoryNew(IDXGIFactory *dxgifNew)
{
  dbg("dxgi_f: IDXGIFactoryNew 0x%08X",dxgifNew);
  dxgif = dxgifNew;
}

IDXGIFactoryNew::~IDXGIFactoryNew()
{
  dbg("dxgi_f: ~IDXGIFactoryNew");
}

HRESULT IDXGIFactoryNew::EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
{
  dbg("dxgi_f: EnumAdapters %d", Adapter);

  // Pretend only one adapter exists
  if(Adapter > 0)
    return DXGI_ERROR_NOT_FOUND;
  else {
    // Its a SoftTH adapter!
    IDXGIAdapter *a;
    HRESULT ret = dxgif->EnumAdapters(Adapter, &a);
    *ppAdapter = new IDXGIAdapterNew(a, this);
    //dbg("adapterit: 0x%08X 0x%08X 0x%08X 0x%08X", *ppAdapter, ppAdapter, *a, a);
    return ret;
  }
}

HRESULT IDXGIFactoryNew::CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd, IDXGISwapChain **ppSwapChain)
{
  dbg("dxgi_f: CreateSwapChain");

  dbg("dxgi_f: Mode: %dx%d %d.%dHz %s", scd->BufferDesc.Width, scd->BufferDesc.Height, scd->BufferDesc.RefreshRate.Numerator, scd->BufferDesc.RefreshRate.Denominator, scd->Windowed?"Windowed":"Fullscreen");
  dbg("dxgi_f: Multisample: %d samples, quality %d", scd->SampleDesc.Count, scd->SampleDesc.Quality);
  dbg("dxgi_f: Buffers: %d (Usage %s), Swapeffect: %s", scd->BufferCount, getUsageDXGI(scd->BufferUsage), scd->SwapEffect==DXGI_SWAP_EFFECT_DISCARD?"DISCARD":"SEQUENTIAL");

  dbg("dxgi_f: Flags: %s %s %s", scd->Flags&DXGI_SWAP_CHAIN_FLAG_NONPREROTATED?"NONPREROTATED":"",
                         scd->Flags&DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH?"ALLOW_MODE_SWITCH":"",
                         scd->Flags&DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE?"GDI_COMPATIBLE":"");

  *ppSwapChain = new IDXGISwapChainNew(this, dxgif, pDevice, scd);

  /*if(scd->BufferDesc.Width == config.main.renderResolution.x && scd->BufferDesc.Height == config.main.renderResolution.y) {
    dbg("dxgi_f: Multihead swapchain mode detected");
    HEAD *h = config.getPrimaryHead();
    scd->BufferDesc.Width = h->screenMode.x;
    scd->BufferDesc.Height = h->screenMode.y;
  } else
    dbg("dxgi_f: Singlehead swapchain mode");

  IDXGISwapChain *sc = NULL;
  HRESULT ret = dxgif->CreateSwapChain(pDevice, scd, &sc);
  if(ret != S_OK)
    dbg("dxgi_f: CreateSwapChain failed!");
  else {
    *ppSwapChain = sc;
    if(!pDevice)
      dbg("dxgi_f: NULL device!");

    // TODO: check for other devices
    ID3D10Device *d3d10 = NULL;
    if(pDevice->QueryInterface(__uuidof(ID3D10Device), (void**) &d3d10) == S_OK)
      dbg("dxgi_f: Got Direct3D 10 device");
    //if(d3d10)
    //  *ppSwapChain = new IDXGISwapChainNew(sc, this, d3d10, scd->OutputWindow);
    else
      dbg("dxgi_f: ERROR: Unknown swapchain device type!");
  }*/
  HRESULT ret = S_OK;
  return ret;
}



/* IDXGIFactory1New */
IDXGIFactory1New::IDXGIFactory1New(IDXGIFactory1 *dxgifNew)
{
  dbg("dxgi_f1: IDXGIFactory1New 0x%08X",dxgifNew);
  dxgif = dxgifNew;
}

IDXGIFactory1New::~IDXGIFactory1New()
{
  dbg("dxgi_f1: ~IDXGIFactory1New");
}

HRESULT IDXGIFactory1New::EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
{
  dbg("dxgi_f1: EnumAdapters %d", Adapter);

  // Pretend only one adapter exists
  if(Adapter > 0)
    return DXGI_ERROR_NOT_FOUND;
  else {
    // Its a SoftTH adapter!
    IDXGIAdapter1 *a;
    HRESULT ret = dxgif->EnumAdapters1(Adapter, &a);
    *ppAdapter = new IDXGIAdapter1New(a, this);
    //dbg("adapterit: 0x%08X 0x%08X 0x%08X 0x%08X", *ppAdapter, ppAdapter, *a, a);
    return ret;
  }
}

HRESULT IDXGIFactory1New::EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter)
{
  dbg("dxgi_f1: EnumAdapters1 %d", Adapter);
  return EnumAdapters(Adapter, (IDXGIAdapter**)ppAdapter);  // EnumAdapters will handle this just fine - it creates Adapter1 anyway
}

HRESULT IDXGIFactory1New::CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd, IDXGISwapChain **ppSwapChain)
{
  dbg("dxgi_f1: CreateSwapChain");

  dbg("dxgi_f1: Mode: %dx%d %d.%dHz %s", scd->BufferDesc.Width, scd->BufferDesc.Height, scd->BufferDesc.RefreshRate.Numerator, scd->BufferDesc.RefreshRate.Denominator, scd->Windowed?"Windowed":"Fullscreen");
  dbg("dxgi_f1: Multisample: %d samples, quality %d", scd->SampleDesc.Count, scd->SampleDesc.Quality);
  dbg("dxgi_f1: Buffers: %d (Usage %s), Swapeffect: %s", scd->BufferCount, getUsageDXGI(scd->BufferUsage), scd->SwapEffect==DXGI_SWAP_EFFECT_DISCARD?"DISCARD":"SEQUENTIAL");

  dbg("dxgi_f1: Flags: %s %s %s", scd->Flags&DXGI_SWAP_CHAIN_FLAG_NONPREROTATED?"NONPREROTATED":"",
                         scd->Flags&DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH?"ALLOW_MODE_SWITCH":"",
                         scd->Flags&DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE?"GDI_COMPATIBLE":"");

  /*if(scd->BufferDesc.Width == config.main.renderResolution.x && scd->BufferDesc.Height == config.main.renderResolution.y) {
    dbg("dxgi_f1: Multihead swapchain mode detected");
    HEAD *h = config.getPrimaryHead();
    scd->BufferDesc.Width = h->screenMode.x;
    scd->BufferDesc.Height = h->screenMode.y;
  } else
    dbg("dxgi_f1: Singlehead swapchain mode");*/

  *ppSwapChain = new IDXGISwapChainNew(this, dxgif, pDevice, scd);

  /*IDXGISwapChain *sc = NULL;
  HRESULT ret = dxgif->CreateSwapChain(pDevice, scd, &sc);
  if(ret != S_OK)
    dbg("dxgi_f1: CreateSwapChain failed!");
  else {
    *ppSwapChain = sc;
    if(!pDevice)
      dbg("dxgi_f1: NULL device!");

    *ppSwapChain = new IDXGISwapChainNew(this, dxgif, pDevice, scd);

    // TODO: check for other devices
    ID3D11Device *d3d11 = NULL;
    if(pDevice->QueryInterface(__uuidof(ID3D11Device), (void**) &d3d11) == S_OK)
      dbg("dxgi_f1: Got Direct3D 11 device");
    if(d3d11)
      *ppSwapChain = new IDXGISwapChainNew(dxgif, dxgif, d3d11, scd);
    else
      dbg("dxgi_f1: ERROR: Unknown swapchain device type!");
  }*/
  HRESULT ret = S_OK;

  return ret;
}




/*
TODO: Need to add all the IDXGIFactory2New
functions, especially CreateSwapChainFor[XXXX] methods.
*/

/* IDXGIFactory2New */
IDXGIFactory2New::IDXGIFactory2New(IDXGIFactory2 *dxgifNew)
{
  dbg("dxgi_f2: IDXGIFactory2New 0x%08X",dxgifNew);
  dxgif = dxgifNew;
}

IDXGIFactory2New::~IDXGIFactory2New()
{
  dbg("dxgi_f2: ~IDXGIFactory2New");
}

HRESULT IDXGIFactory2New::EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
{
  dbg("dxgi_f2: EnumAdapters %d", Adapter);

  // Pretend only one adapter exists
  if(Adapter > 0)
    return DXGI_ERROR_NOT_FOUND;
  else {
    // Its a SoftTH adapter!
    IDXGIAdapter1 *a;
    HRESULT ret = dxgif->EnumAdapters1(Adapter, &a);
    *ppAdapter = new IDXGIAdapter1New(a, this);
    //dbg("adapterit: 0x%08X 0x%08X 0x%08X 0x%08X", *ppAdapter, ppAdapter, *a, a);
    return ret;
  }
}

HRESULT IDXGIFactory2New::EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter)
{
  dbg("dxgi_f2: EnumAdapters1 %d", Adapter);
  return EnumAdapters(Adapter, (IDXGIAdapter**)ppAdapter);  // EnumAdapters will handle this just fine - it creates Adapter1 anyway
}

HRESULT IDXGIFactory2New::CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd, IDXGISwapChain **ppSwapChain)
{
  dbg("dxgi_f2: CreateSwapChain");

  dbg("dxgi_f2: Mode: %dx%d %d.%dHz %s", scd->BufferDesc.Width, scd->BufferDesc.Height, scd->BufferDesc.RefreshRate.Numerator, scd->BufferDesc.RefreshRate.Denominator, scd->Windowed?"Windowed":"Fullscreen");
  dbg("dxgi_f2: Multisample: %d samples, quality %d", scd->SampleDesc.Count, scd->SampleDesc.Quality);
  dbg("dxgi_f2: Buffers: %d (Usage %s), Swapeffect: %s", scd->BufferCount, getUsageDXGI(scd->BufferUsage), scd->SwapEffect==DXGI_SWAP_EFFECT_DISCARD?"DISCARD":"SEQUENTIAL");

  dbg("dxgi_f2: Flags: %s %s %s", scd->Flags&DXGI_SWAP_CHAIN_FLAG_NONPREROTATED?"NONPREROTATED":"",
                         scd->Flags&DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH?"ALLOW_MODE_SWITCH":"",
                         scd->Flags&DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE?"GDI_COMPATIBLE":"");

  /*if(scd->BufferDesc.Width == config.main.renderResolution.x && scd->BufferDesc.Height == config.main.renderResolution.y) {
    dbg("dxgi_f1: Multihead swapchain mode detected");
    HEAD *h = config.getPrimaryHead();
    scd->BufferDesc.Width = h->screenMode.x;
    scd->BufferDesc.Height = h->screenMode.y;
  } else
    dbg("dxgi_f1: Singlehead swapchain mode");*/

  *ppSwapChain = new IDXGISwapChainNew(this, dxgif, pDevice, scd);

  /*IDXGISwapChain *sc = NULL;
  HRESULT ret = dxgif->CreateSwapChain(pDevice, scd, &sc);
  if(ret != S_OK)
    dbg("dxgi_f1: CreateSwapChain failed!");
  else {
    *ppSwapChain = sc;
    if(!pDevice)
      dbg("dxgi_f1: NULL device!");

    *ppSwapChain = new IDXGISwapChainNew(this, dxgif, pDevice, scd);

    // TODO: check for other devices
    ID3D11Device *d3d11 = NULL;
    if(pDevice->QueryInterface(__uuidof(ID3D11Device), (void**) &d3d11) == S_OK)
      dbg("dxgi_f1: Got Direct3D 11 device");
    if(d3d11)
      *ppSwapChain = new IDXGISwapChainNew(dxgif, dxgif, d3d11, scd);
    else
      dbg("dxgi_f1: ERROR: Unknown swapchain device type!");
  }*/
  HRESULT ret = S_OK;

  return ret;
}
