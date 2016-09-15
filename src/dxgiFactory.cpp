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

#include <INITGUID.H>
DEFINE_GUID(IID_IDXGIFactoryNew, 0xd9d267c0, 0x52d, 0x4a8f, 0x98, 0xf4, 0xbe, 0x28, 0xb2, 0xcc, 0x32, 0x74); // {D9D267C0-052D-4a8f-98F4-BE28B2CC3274}
//DEFINE_GUID(IID_IDXGIFactory1New, 0xc1bbaf12, 0x70f6, 0x4c47, 0xa3, 0x92, 0x2b, 0xc4, 0xf1, 0xc6, 0xf0, 0x49);
//DEFINE_GUID(IID_IDXGIFactory2New, 0xee85e851, 0x1363, 0x440d, 0x89, 0x82, 0xac, 0x5c, 0x56, 0x5a, 0x6, 0xea); // {EE85E851-1363-440d-8982-AC5C565A06EA}


/*
TODO: Need to add all the IDXGIFactoryNew
functions, especially CreateSwapChainFor[XXXX] methods.
*/

/* IDXGIFactoryNew */
IDXGIFactoryNew::IDXGIFactoryNew(IDXGIFactory2 *dxgifNew)
{
  dbg("dxgi_f2: 0x%08X IDXGIFactoryNew",this);
  dbg("dxgi_f2: -- Real: 0x%08X - SoftTH: 0x%08X",dxgifNew,this);
  dxgif = dxgifNew;
}

IDXGIFactoryNew::~IDXGIFactoryNew()
{
  dbg("dxgi_f2: 0x%08X ~IDXGIFactoryNew",this);
}

HRESULT IDXGIFactoryNew::EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
{
  dbg("dxgi_f2: 0x%08X EnumAdapters %d", this, Adapter);

  // Pretend only one adapter exists
  if(Adapter > 0) {
    dbg("dxgi_f2: -- Tried to get more adapters");
    return DXGI_ERROR_NOT_FOUND;
  } else {
    dbg("dxgi_f2: -- First enumeration");
    // Its a SoftTH adapter!
    IDXGIAdapter *a;
    HRESULT ret = dxgif->EnumAdapters(Adapter, &a);
    dbg("dxgi_f2: -- Enumerated real adapter 0x%08X", a);
    *ppAdapter = new IDXGIAdapterNew((IDXGIAdapter2 *) a, this);
    dbg("dxgi_f2: -- Replaced real adapter with SoftTH adapter: 0x%08X", *ppAdapter);
    //dbg("adapterit: 0x%08X 0x%08X 0x%08X 0x%08X", *ppAdapter, ppAdapter, *a, a);
    //a->Release();
    return ret;
  }
}

HRESULT IDXGIFactoryNew::EnumAdapters1(UINT Adapter, IDXGIAdapter1 **ppAdapter)
{
  dbg("dxgi_f2: 0x%08X EnumAdapters1 %d", this, Adapter);

  // Pretend only one adapter exists
  if(Adapter > 0) {
    dbg("dxgi_f2: -- Tried to get more adapters");
    return DXGI_ERROR_NOT_FOUND;
  } else {
    dbg("dxgi_f2: -- First enumeration");
    // Its a SoftTH adapter!
    IDXGIAdapter1 *a;
    HRESULT ret = dxgif->EnumAdapters1(Adapter, &a);
    dbg("dxgi_f2: -- Enumerated real adapter 0x%08X", a);
    *ppAdapter = new IDXGIAdapterNew((IDXGIAdapter2 *) a, this);
    dbg("dxgi_f2: -- Replaced real adapter with SoftTH adapter: 0x%08X", *ppAdapter);
    //dbg("adapterit: 0x%08X 0x%08X 0x%08X 0x%08X", *ppAdapter, ppAdapter, *a, a);
    //a->Release();
    return ret;
  }
}

HRESULT IDXGIFactoryNew::CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd, IDXGISwapChain **ppSwapChain)
{
  dbg("dxgi_f2: 0x%08X CreateSwapChain",this);

  // DEBUG - Force to make a SoftTH swapchain
  /*scd->BufferDesc.Width  = config.main.renderResolution.x;
  scd->BufferDesc.Height = config.main.renderResolution.y;
  scd->BufferDesc.RefreshRate.Numerator = 59950;
  scd->BufferDesc.RefreshRate.Denominator = 1000;*/


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

  *ppSwapChain = new IDXGISwapChainNew(dxgif, this, pDevice, scd);

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
