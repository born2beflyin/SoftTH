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

#include <d3d11.h>
#include "dxgiSwapChain.h"
#include "helper.h"
#include "version.h"

#include <dxgi.h>
#include "main.h"
#include <D3DX11Tex.h>
#include <D3DX10Tex.h>

#include "InputHandler.h"

#include <INITGUID.H>
DEFINE_GUID(IID_IDXGISwapChainNew, 0x41ba0075, 0xbc7b, 0x4eee, 0x99, 0x8d, 0xb6, 0xdb, 0xb7, 0xba, 0xeb, 0x46);

volatile extern int SoftTHActive; // >0 if SoftTH is currently active and resolution is overridden
/*
IDXGISwapChainNew::IDXGISwapChainNew(IDXGISwapChain *dxgscNew, IDXGIFactory1 *parentNew, ID3D10Device *dev10new, HWND winNew)
{
  dbg("IDXGISwapChainNew 0x%08X 0x%08X", this, dev10new);
  dxgsc = dxgscNew;
  parent = parentNew;
  dev10 = dev10new;
  win = winNew;
  newbb = NULL;

  updateBB();

  DWORD pid;
  char name[256];
  GetWindowText(win, name, 256);
  GetWindowThreadProcessId(win, &pid);
  dbg("Device window 0x%08X <%s>, thread 0x%08X", win, name, pid);
  ihGlobal.hookRemoteThread(pid);
}*/

IDXGISwapChainNew::IDXGISwapChainNew(IDXGIFactory1 *parentNew, IDXGIFactory1 *dxgifNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd)
{
  // Creating new swapchain
  dbg("IDXGISwapChainNew 0x%08X 0x%08X", this, pDevice);


  win = scd->OutputWindow;
  newbb10 = NULL;
  newbb11 = NULL;
  dev10 = NULL;
  dev10_1 = NULL;
  dev11 = NULL;
  dxgsc = NULL;
  dxgif = dxgifNew;
  parent = parentNew;
  realbb10 = NULL;
  realbb11 = NULL;

  if(!pDevice)
    dbg("ERROR: NULL device!");
  else {
    // Check for D3D10/11 device
    /*if(pDevice->QueryInterface(__uuidof(ID3D10Device), (void**) &dev10) == S_OK)
      dbg("Got Direct3D 10 device");
    else */if(pDevice->QueryInterface(__uuidof(ID3D10Device1), (void**) &dev10_1) == S_OK)
      dbg("Got Direct3D 10.1 device");
    else if(pDevice->QueryInterface(__uuidof(ID3D11Device), (void**) &dev11) == S_OK)
      dbg("Got Direct3D 11 device");
    else
      dbg("ERROR: Unknown swapchain device type!");

    if(dev11 || dev10_1 || dev10)
    {
      // Check for TH mode and create bb texture
      preUpdateBB(&scd->BufferDesc.Width, &scd->BufferDesc.Height);
    }
  }

  // Create the swapchain
  HRESULT ret = dxgif->CreateSwapChain(pDevice, scd, &dxgsc);
  if(ret != S_OK)
    dbg("CreateSwapChain failed!");
  else
    updateBB();

}

IDXGISwapChainNew::~IDXGISwapChainNew()
{
  dbg("~IDXGISwapChainNew 0x%08X", this);
}

HRESULT IDXGISwapChainNew::GetBuffer(UINT Buffer, REFIID riid, void **ppSurface)
{
  // App wants pointer to backbuffer
  dbg("dxgsc: GetBuffer %d %s", Buffer, matchRiid(riid));
  if(newbb10 || newbb11) {
    // Return our fake buffer
    if(newbb11)
    {
      *ppSurface = newbb11;
      newbb11->AddRef();
    }
    else
    {
      *ppSurface = newbb10;
      newbb10->AddRef();
    }
    return S_OK;
  } else {
    // Return real backbuffer
    return dxgsc->GetBuffer(Buffer, riid, ppSurface);
  }
}

HRESULT IDXGISwapChainNew::GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc)
{
  dbg("dxgsc: GetDesc");
  HRESULT ret = dxgsc->GetDesc(pDesc);
  if(newbb10 || newbb11) {
    // Pretend it is triplehead
    pDesc->BufferDesc.Width = config.main.renderResolution.x;
    pDesc->BufferDesc.Height = config.main.renderResolution.y;
  }
  return ret;
};

HRESULT IDXGISwapChainNew::Present(UINT SyncInterval,UINT Flags)
{
  if(newbb11)
    dbg("dxgsc: BB11 Present %d %d", SyncInterval, newbb11);
  else if(newbb10)
    dbg("dxgsc: BB10 Present %d %d", SyncInterval, newbb10);

  if(!newbb10 && !newbb11) {
    // Not multihead mode, plain old present
    dbg("dxgsc: Not multihead, just plain present");
    return dxgsc->Present(SyncInterval, Flags);
  }

  #if defined(SOFTTHMAIN) || defined(D3D11)
  if(dev11)
  {
    D3D11_TEXTURE2D_DESC dt, ds;
    dbg("realbbdesc11... %d", realbb11);
    realbb11->GetDesc(&dt);
    newbb11->GetDesc(&ds);

    dbg("Source: %dx%d ms%d %s", ds.Width, ds.Height, ds.SampleDesc.Count, getFormatDXGI(ds.Format));
    dbg("Target: %dx%d ms%d %s", dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));

    HEAD *h = config.getPrimaryHead();
    D3D11_BOX sb = {h->sourceRect.left, h->sourceRect.top, 0, h->sourceRect.right, h->sourceRect.bottom, 1};
    ID3D11DeviceContext *dev11context;
    dev11->GetImmediateContext(&dev11context);
    dev11context->CopySubresourceRegion(realbb11, 0, 0, 0, 0, newbb11, 0, &sb);

    /*if(GetKeyState('O') < 0)
      D3DX11SaveTextureToFile(realbb10, D3DX10_IFF_JPG, "d:\\pelit\\_realbb.jpg");
    if(GetKeyState('P') < 0)
      D3DX11SaveTextureToFile(newbb10, D3DX10_IFF_JPG, "d:\\pelit\\_newbb.jpg");*/

    // Copy & Present secondary heads
    for(int i=0;i<numDevs;i++)
    {
      OUTDEVICE11 *o = &outDevs11[i];
      D3D11_BOX sb = {o->cfg->sourceRect.left, o->cfg->sourceRect.top, 0, o->cfg->sourceRect.right, o->cfg->sourceRect.bottom, 1};
      dev11context->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, newbb11, 0, &sb);
      dev11context->Flush();

      o->output->present();
    }
  }
  #endif
  #ifdef SOFTTHMAIN
  else
  #endif
  #if defined(SOFTTHMAIN) || defined(D3D10)
  if(dev10_1)
  {
    D3D10_TEXTURE2D_DESC dt, ds;
    dbg("realbbdesc10.1 ... %d", realbb10);
    realbb10->GetDesc(&dt);
    newbb10->GetDesc(&ds);

    dbg("Source: %dx%d ms%d %s", ds.Width, ds.Height, ds.SampleDesc.Count, getFormatDXGI(ds.Format));
    dbg("Target: %dx%d ms%d %s", dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));

    HEAD *h = config.getPrimaryHead();
    D3D10_BOX sb = {h->sourceRect.left, h->sourceRect.top, 0, h->sourceRect.right, h->sourceRect.bottom, 1};
    dev10_1->CopySubresourceRegion(realbb10, 0, 0, 0, 0, newbb10, 0, &sb);

    if(GetKeyState('O') < 0)
      D3DX10SaveTextureToFile(realbb10, D3DX10_IFF_JPG, "d:\\pelit\\_realbb.jpg");
    if(GetKeyState('P') < 0)
      D3DX10SaveTextureToFile(newbb10, D3DX10_IFF_JPG, "d:\\pelit\\_newbb.jpg");

    // Copy & Present secondary heads
    for(int i=0;i<numDevs;i++)
    {
      OUTDEVICE10 *o = &outDevs10[i];
      D3D10_BOX sb = {o->cfg->sourceRect.left, o->cfg->sourceRect.top, 0, o->cfg->sourceRect.right, o->cfg->sourceRect.bottom, 1};
      dev10_1->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, newbb10, 0, &sb);
      dev10_1->Flush();

      o->output->present();
    }
  }
  #endif // defined
  #ifdef SOFTTHMAIN
  else
  #endif
  #if defined(SOFTTHMAIN) || defined(D3D10)
  if(dev10)
  {
    D3D10_TEXTURE2D_DESC dt, ds;
    dbg("realbbdesc10... %d", realbb10);
    realbb10->GetDesc(&dt);
    newbb10->GetDesc(&ds);

    dbg("Source: %dx%d ms%d %s", ds.Width, ds.Height, ds.SampleDesc.Count, getFormatDXGI(ds.Format));
    dbg("Target: %dx%d ms%d %s", dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));

    HEAD *h = config.getPrimaryHead();
    D3D10_BOX sb = {h->sourceRect.left, h->sourceRect.top, 0, h->sourceRect.right, h->sourceRect.bottom, 1};
    dev10->CopySubresourceRegion(realbb10, 0, 0, 0, 0, newbb10, 0, &sb);

    if(GetKeyState('O') < 0)
      D3DX10SaveTextureToFile(realbb10, D3DX10_IFF_JPG, "d:\\pelit\\_realbb.jpg");
    if(GetKeyState('P') < 0)
      D3DX10SaveTextureToFile(newbb10, D3DX10_IFF_JPG, "d:\\pelit\\_newbb.jpg");

    // Copy & Present secondary heads
    for(int i=0;i<numDevs;i++)
    {
      OUTDEVICE10 *o = &outDevs10[i];
      D3D10_BOX sb = {o->cfg->sourceRect.left, o->cfg->sourceRect.top, 0, o->cfg->sourceRect.right, o->cfg->sourceRect.bottom, 1};
      dev10->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, newbb10, 0, &sb);
      dev10->Flush();

      o->output->present();
    }
  }
  #endif // defined

  HRESULT ret = dxgsc->Present(SyncInterval, Flags);
  if(ret != S_OK)
    dbg("IDXGISwapChainNew::Present: Failed");
  return ret;
}

HRESULT IDXGISwapChainNew::ResizeTarget(const DXGI_MODE_DESC *tgtp)
{
  dbg("dxgsc: ResizeTarget %dx%d", tgtp->Width, tgtp->Height);

  DXGI_MODE_DESC m = *tgtp;
  m.Width = 1920;
  m.Height = 1200;

  HRESULT ret = dxgsc->ResizeTarget(&m);
  if(ret != S_OK)
    dbg("dxgsc: ResizeTarget failed %dx%d", tgtp->Width, tgtp->Height);
  return ret;
};

HRESULT IDXGISwapChainNew::ResizeBuffers(UINT BufferCount,UINT Width,UINT Height,DXGI_FORMAT NewFormat,UINT SwapChainFlags)
{
  dbg("dxgsc: ResizeBuffers %dx%d", Width, Height);
  preUpdateBB(&Width, &Height);
  HRESULT ret = dxgsc->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
  if(ret == S_OK) {
    updateBB(); // Backbuffer was recreated
  } else {
    dbg("ResizeBuffers failed!");
  }
  return ret;
};

void IDXGISwapChainNew::preUpdateBB(UINT *width, UINT *height)
{
  dbg("dxgsc: preUpdateBB");
  int rrx = config.main.renderResolution.x;
  int rry = config.main.renderResolution.y;
  if(*width == rrx && *height == rry) {
    dbg("Multihead swapchain mode detected");
    HEAD *h = config.getPrimaryHead();
    *width = h->screenMode.x;
    *height = h->screenMode.y;

    // Set mouse hook on application focus window
    ihGlobal.setHWND(win);
    SoftTHActive++;
    h->hwnd = win;

    // Create new backbuffer
    dbg("dxgsc: Creating new backbuffer");
    // TODO: format
    #if defined(SOFTTHMAIN) || defined(D3D11)
    if(dev11)
    {
      dbg("dxgsc: Creating backbuffer for D3D11 Device");
      //CD3D10_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, D3D10_BIND_RENDER_TARGET, D3D10_USAGE_DEFAULT, NULL);
      CD3D11_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, D3D11_BIND_RENDER_TARGET, D3D11_USAGE_DEFAULT, NULL);
      newbbDesc11 = d;
      if(dev11->CreateTexture2D(&newbbDesc11, NULL, &newbb11) != S_OK)
        dbg("CreateTexture2D failed :("), exit(0);

      // Initialize outputs
      numDevs = config.getNumAdditionalHeads();
      dbg("Initializing %d outputs", numDevs);
      int logoStopTime = GetTickCount() + 4000;

      bool fpuPreserve = true; // TODO: does this exist in d3d10?

      outDevs11 = new OUTDEVICE11[numDevs];
      for(int i=0;i<numDevs;i++)
      {
        OUTDEVICE11 *o = &outDevs11[i];

        // Create the output device
        HEAD *h = config.getHead(i);
        bool local = h->transportMethod==OUTMETHOD_LOCAL;
        dbg("Initializing head %d (DevID: %d, %s)...", i+1, h->devID, local?"local":"non-local");
        o->output = new outDirect3D11(h->devID, h->screenMode.x, h->screenMode.y, h->transportRes.x, h->transportRes.y, win);
        o->cfg = h;

        // Create shared surfaces
        HANDLE sha = o->output->GetShareHandle();
        if(sha) {
          o->localSurf = NULL;

          { // Open surfA share handle
            ID3D11Resource *tr;
            if(dev11->OpenSharedResource(sha, __uuidof(ID3D11Resource), (void**)(&tr)) != S_OK)
              dbg("OpenSharedResource A failed!"), exit(0);
            if(tr->QueryInterface(__uuidof(ID3D11Texture2D), (void**)(&o->localSurf)) != S_OK)
              dbg("Shared surface QueryInterface failed!"), exit(0);
            tr->Release();
          }
          dbg("Opened share handles");
        } else
          dbg("ERROR: Head %d: No share handle!", i+1), exit(0);
      }
    }
    #endif
    #ifdef SOFTTHMAIN
    else
    #endif
    #if defined(SOFTTHMAIN) || defined(D3D10_1)
    if(dev10_1)
    {
      dbg("dxgsc: Creating backbuffer for D3D10.1 Device");
      CD3D10_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, D3D10_BIND_RENDER_TARGET, D3D10_USAGE_DEFAULT, NULL);
      newbbDesc10 = d;
      if(dev10_1->CreateTexture2D(&newbbDesc10, NULL, &newbb10) != S_OK)
        dbg("CreateTexture2D failed :("), exit(0);

      // Initialize outputs
      numDevs = config.getNumAdditionalHeads();
      dbg("Initializing %d outputs", numDevs);
      int logoStopTime = GetTickCount() + 4000;

      bool fpuPreserve = true; // TODO: does this exist in d3d10?

      outDevs10 = new OUTDEVICE10[numDevs];
      for(int i=0;i<numDevs;i++)
      {
        OUTDEVICE10 *o = &outDevs10[i];

        // Create the output device
        HEAD *h = config.getHead(i);
        bool local = h->transportMethod==OUTMETHOD_LOCAL;
        dbg("Initializing head %d (DevID: %d, %s)...", i+1, h->devID, local?"local":"non-local");
        o->output = new outDirect3D10(h->devID, h->screenMode.x, h->screenMode.y, h->transportRes.x, h->transportRes.y, win);
        o->cfg = h;

        // Create shared surfaces
        HANDLE sha = o->output->GetShareHandle();
        if(sha) {
          o->localSurf = NULL;

          { // Open surfA share handle
            ID3D10Resource *tr;
            if(dev10_1->OpenSharedResource(sha, __uuidof(ID3D10Resource), (void**)(&tr)) != S_OK)
              dbg("OpenSharedResource A failed!"), exit(0);
            if(tr->QueryInterface(__uuidof(ID3D10Texture2D), (void**)(&o->localSurf)) != S_OK)
              dbg("Shared surface QueryInterface failed!"), exit(0);
            tr->Release();
          }
          dbg("Opened share handles");
        } else
          dbg("ERROR: Head %d: No share handle!", i+1), exit(0);
      }
    }
    #endif
    #ifdef SOFTTHMAIN
    else
    #endif
    #if defined(SOFTTHMAIN) || defined(D3D10)
    if(dev10)
    {
      dbg("dxgsc: Creating backbuffer for D3D10 Device");
      CD3D10_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, D3D10_BIND_RENDER_TARGET, D3D10_USAGE_DEFAULT, NULL);
      newbbDesc10 = d;
      if(dev10->CreateTexture2D(&newbbDesc10, NULL, &newbb10) != S_OK)
        dbg("CreateTexture2D failed :("), exit(0);

      // Initialize outputs
      numDevs = config.getNumAdditionalHeads();
      dbg("Initializing %d outputs", numDevs);
      int logoStopTime = GetTickCount() + 4000;

      bool fpuPreserve = true; // TODO: does this exist in d3d10?

      outDevs10 = new OUTDEVICE10[numDevs];
      for(int i=0;i<numDevs;i++)
      {
        OUTDEVICE10 *o = &outDevs10[i];

        // Create the output device
        HEAD *h = config.getHead(i);
        bool local = h->transportMethod==OUTMETHOD_LOCAL;
        dbg("Initializing head %d (DevID: %d, %s)...", i+1, h->devID, local?"local":"non-local");
        o->output = new outDirect3D10(h->devID, h->screenMode.x, h->screenMode.y, h->transportRes.x, h->transportRes.y, win);
        o->cfg = h;

        // Create shared surfaces
        HANDLE sha = o->output->GetShareHandle();
        if(sha) {
          o->localSurf = NULL;

          { // Open surfA share handle
            ID3D10Resource *tr;
            if(dev10->OpenSharedResource(sha, __uuidof(ID3D10Resource), (void**)(&tr)) != S_OK)
              dbg("OpenSharedResource A failed!"), exit(0);
            if(tr->QueryInterface(__uuidof(ID3D10Texture2D), (void**)(&o->localSurf)) != S_OK)
              dbg("Shared surface QueryInterface failed!"), exit(0);
            tr->Release();
          }
          dbg("Opened share handles");
        } else
          dbg("ERROR: Head %d: No share handle!", i+1), exit(0);
      }
    }
    #endif

  } else {
    dbg("Singlehead swapchain mode");
    SoftTHActive--;

    if(dev11)
    {
      if(newbb11)
        SAFE_RELEASE_LAST(newbb11);
      newbb11 = NULL;
    }
    else if(dev10 || dev10_1)
    {
      if(newbb10)
        SAFE_RELEASE_LAST(newbb10);
      newbb10 = NULL;
    }

  }
}

void IDXGISwapChainNew::updateBB()
{
  dbg("IDXGISwapChainNew::updateBB!");

  // Get realbb
  if(dev11)
  {
    if(dxgsc->GetBuffer(0, IID_ID3D11Texture2D, (void**)&realbb11) != S_OK)
      dbg("dxgsc->GetBuffer failed!"), exit(0);

    realbb11->GetDesc(&realbbDesc11);
    dbg("Backbuffer: %dx%d ms%d %s", realbbDesc11.Width, realbbDesc11.Height, realbbDesc11.SampleDesc.Count, getFormatDXGI(realbbDesc11.Format));
    realbb11->Release();  // Pretend we didn't get pointer to it so it can be released by size changes
  }
  else if(dev10 || dev10_1)
  {
    if(dxgsc->GetBuffer(0, IID_ID3D10Texture2D, (void**)&realbb10) != S_OK)
      dbg("dxgsc->GetBuffer failed!"), exit(0);

    realbb10->GetDesc(&realbbDesc10);
    dbg("Backbuffer: %dx%d ms%d %s", realbbDesc10.Width, realbbDesc10.Height, realbbDesc10.SampleDesc.Count, getFormatDXGI(realbbDesc10.Format));
    realbb10->Release();  // Pretend we didn't get pointer to it so it can be released by size changes
  }
}
