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

#include <dxgi.h>

#include <d3d10.h>

#include <INITGUID.H>
DEFINE_GUID(IID_IDXGIFactory1New, 0xc1bbaf12, 0x70f6, 0x4c47, 0xa3, 0x92, 0x2b, 0xc4, 0xf1, 0xc6, 0xf0, 0x49);

IDXGIFactory1New::IDXGIFactory1New(IDXGIFactory1 *dxgifNew)
{
  dbg("IDXGIFactory1New");
  dxgif = dxgifNew;
}

IDXGIFactory1New::~IDXGIFactory1New()
{
  dbg("~IDXGIFactory1New");
}

HRESULT IDXGIFactory1New::EnumAdapters(UINT Adapter, IDXGIAdapter **ppAdapter)
{
  dbg("dxgif: EnumAdapters %d", Adapter);

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
  dbg("dxgif: EnumAdapters1 %d", Adapter);
  return EnumAdapters(Adapter, (IDXGIAdapter**)ppAdapter);  // EnumAdapters will handle this just fine - it creates Adapter1 anyway
}

HRESULT IDXGIFactory1New::CreateSwapChain(IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd, IDXGISwapChain **ppSwapChain)
{
  dbg("dxgif: CreateSwapChain");

  dbg("Mode: %dx%d %d.%dHz %s", scd->BufferDesc.Width, scd->BufferDesc.Height, scd->BufferDesc.RefreshRate.Numerator, scd->BufferDesc.RefreshRate.Denominator, scd->Windowed?"Windowed":"Fullscreen");
  dbg("Multisample: %d samples, quality %d", scd->SampleDesc.Count, scd->SampleDesc.Quality);
  dbg("Buffers: %d (Usage %s), Swapeffect: %s", scd->BufferCount, getUsageDXGI(scd->BufferUsage), scd->SwapEffect==DXGI_SWAP_EFFECT_DISCARD?"DISCARD":"SEQUENTIAL");

  dbg("Flags: %s %s %s", scd->Flags&DXGI_SWAP_CHAIN_FLAG_NONPREROTATED?"NONPREROTATED":"", 
                         scd->Flags&DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH?"ALLOW_MODE_SWITCH":"",
                         scd->Flags&DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE?"GDI_COMPATIBLE":"");

  *ppSwapChain = new IDXGISwapChainNew(this, dxgif, pDevice, scd);
  return S_OK;  // TODO: get return value
/*
  if(scd->BufferDesc.Width == config.main.renderResolution.x && scd->BufferDesc.Height == config.main.renderResolution.y) {
    dbg("Multihead swapchain mode detected");
    HEAD *h = config.getPrimaryHead();
    scd->BufferDesc.Width = h->screenMode.x;
    scd->BufferDesc.Height = h->screenMode.y;
  } else
    dbg("Singlehead swapchain mode");
  
  IDXGISwapChain *sc = NULL;
  HRESULT ret = dxgif->CreateSwapChain(pDevice, scd, &sc);  
  if(ret != S_OK)
    dbg("CreateSwapChain failed!");
  else {
    *ppSwapChain = sc;
    if(!pDevice)
      dbg("NULL device!");

    // TODO: check for other devices
    ID3D10Device *d3d10 = NULL;
    if(pDevice->QueryInterface(__uuidof(ID3D10Device), (void**) &d3d10) == S_OK)
      dbg("Got Direct3D 10 device");    
    if(d3d10)
      *ppSwapChain = new IDXGISwapChainNew(sc, this, d3d10, scd->OutputWindow);
    else
      dbg("ERROR: Unkonwn swapchain device type!");
  }
  return ret;*/
}