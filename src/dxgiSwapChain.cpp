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

#include "dxgiSwapChain.h"

#include <INITGUID.H>
DEFINE_GUID(IID_IDXGISwapChainNew, 0x41ba0075, 0xbc7b, 0x4eee, 0x99, 0x8d, 0xb6, 0xdb, 0xb7, 0xba, 0xeb, 0x46);

//volatile extern int SoftTHActive; // >0 if SoftTH is currently active and resolution is overridden
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

/*IDXGISwapChainNew::IDXGISwapChainNew(IDXGIFactory *parentNew, IDXGIFactory *dxgifNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd)
{
  // Creating new swapchain
  dbg("dxgi_sc: IDXGISwapChainNew 0x%08X 0x%08X", this, pDevice);


  win       = scd->OutputWindow;
  newbb10   = NULL;
  newbb11   = NULL;
  //stagedSurfs11 = NULL;
  dev10     = NULL;
  dev10_1   = NULL;
  dev11     = NULL;
  //dev12     = NULL;
  dxgsc     = NULL;
  dxgif     = (IDXGIFactory2New*)dxgifNew;
  parent    = (IDXGIFactory2New*)parentNew;
  realbb10  = NULL;
  realbb11  = NULL;

  if(!pDevice)
    dbg("dxgi_sc: ERROR: NULL device!");
  else {
    // Check for D3D10/11 device
    if(pDevice->QueryInterface(__uuidof(ID3D10Device), (void**) &dev10) == S_OK)
      dbg("dxgi_sc: Got Direct3D 10 device");
    else if(pDevice->QueryInterface(__uuidof(ID3D10Device1), (void**) &dev10_1) == S_OK)
      dbg("dxgi_sc: Got Direct3D 10.1 device");
    else if(pDevice->QueryInterface(__uuidof(ID3D11Device), (void**) &dev11) == S_OK)
      dbg("dxgi_sc: Got Direct3D 11 device");
    else
      dbg("dxgi_sc: ERROR: Unknown swapchain device type!");

    if(dev11 || dev10_1 || dev10)
    {
      // Check for TH mode and create bb texture
      preUpdateBB(&scd->BufferDesc.Width, &scd->BufferDesc.Height);
    }
  }

  // Create the swapchain
  HRESULT ret = dxgif->CreateSwapChain(pDevice, scd, &dxgsc);
  if(ret != S_OK)
    dbg("dxgi_sc: CreateSwapChain failed!");
  else
    updateBB();

}

IDXGISwapChainNew::IDXGISwapChainNew(IDXGIFactory1 *parentNew, IDXGIFactory1 *dxgifNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC1 *scd)
{
  // Creating new swapchain
  dbg("dxgi_sc: IDXGISwapChainNew 0x%08X 0x%08X", this, pDevice);


  win = scd->OutputWindow;
  newbb10   = NULL;
  newbb11   = NULL;
  //stagedSurfs11 = NULL;
  dev10     = NULL;
  dev10_1   = NULL;
  dev11     = NULL;
  //dev12     = NULL;
  dxgsc     = NULL;
  dxgif     = (IDXGIFactory2New*) dxgifNew;
  parent    = (IDXGIFactory2New*) parentNew;
  realbb10  = NULL;
  realbb11  = NULL;

  if(!pDevice)
    dbg("dxgi_sc: ERROR: NULL device!");
  else {
    // Check for D3D10/11 device
    if(pDevice->QueryInterface(__uuidof(ID3D10Device), (void**) &dev10) == S_OK)
      dbg("dxgi_sc: Got Direct3D 10 device");
    else if(pDevice->QueryInterface(__uuidof(ID3D10Device1), (void**) &dev10_1) == S_OK)
      dbg("dxgi_sc: Got Direct3D 10.1 device");
    else if(pDevice->QueryInterface(__uuidof(ID3D11Device), (void**) &dev11) == S_OK)
      dbg("dxgi_sc: Got Direct3D 11 device");
    else
      dbg("dxgi_sc: ERROR: Unknown swapchain device type!");

    if(dev11 || dev10_1 || dev10)
    {
      // Check for TH mode and create bb texture
      preUpdateBB(&scd->BufferDesc.Width, &scd->BufferDesc.Height);
    }
  }

  // Create the swapchain
  HRESULT ret = dxgif->CreateSwapChain(pDevice, scd, &dxgsc);
  if(ret != S_OK)
    dbg("dxgi_sc: CreateSwapChain failed!");
  else
    updateBB();

}*/

IDXGISwapChainNew::IDXGISwapChainNew(IDXGIFactory2 *dxgifNew, IDXGIFactoryNew *parentNew, IUnknown *pDevice, DXGI_SWAP_CHAIN_DESC *scd)
{
  // Creating new swapchain
  dbg("dxgi_sc: 0x%08X IDXGISwapChainNew", this);
  dbg("dxgi_sc: -- Real: 0x%08X - SoftTH: 0x%08X - Parent: 0x%08X - Device: 0x%08X",
      dxgifNew,this,parentNew,pDevice);


  win       = scd->OutputWindow;
  newbb10   = NULL;
  newbb11   = NULL;
  //stagedSurfs11 = NULL;
  dev10     = NULL;
  dev10_1   = NULL;
  dev11     = NULL;
  //dev12     = NULL;
  dxgsc     = NULL;
  dxgif     = dxgifNew;
  parent    = parentNew;
  realbb10  = NULL;
  realbb11  = NULL;

  dbg("dxgi_sc: 0x%08X OutputWindow: 0x%08X", this, win);

  if(!pDevice)
    dbg("dxgi_sc: -- ERROR: NULL device!");
  else {
    // Check for D3D10/11 device
    if(pDevice->QueryInterface(__uuidof(ID3D10Device), (void**) &dev10) == S_OK)
      {dbg("dxgi_sc: -- Got Direct3D 10 device");}
    else if(pDevice->QueryInterface(__uuidof(ID3D10Device1), (void**) &dev10_1) == S_OK)
      {dbg("dxgi_sc: -- Got Direct3D 10.1 device");}
    else if(pDevice->QueryInterface(__uuidof(ID3D11Device), (void**) &dev11) == S_OK)
      {dbg("dxgi_sc: -- Got Direct3D 11 device");}
    else
      {dbg("dxgi_sc: -- ERROR: Unknown swapchain device type!");}

    if(dev11 || dev10_1 || dev10)
    {
      // Check for TH mode and create bb texture
      preUpdateBB(&scd->BufferDesc.Width, &scd->BufferDesc.Height);
    }
  }

  // Create the swapchain
  HRESULT ret = dxgif->CreateSwapChain(pDevice, scd, (IDXGISwapChain **) &dxgsc);
  if(ret != S_OK)
    dbg("dxgi_sc: -- CreateSwapChain failed!");
  else
    updateBB();

}

IDXGISwapChainNew::~IDXGISwapChainNew()
{
  dbg("dxgi_sc: 0x%08X ~IDXGISwapChainNew", this);
}

HRESULT IDXGISwapChainNew::GetBuffer(UINT Buffer, REFIID riid, void **ppSurface)
{
  // App wants pointer to backbuffer
  dbg("dxgi_sc: 0x%08X GetBuffer %d %s", this, Buffer, matchRiid(riid));
  if(newbb10 || newbb11) {
    // Return our fake buffer
    if(newbb11)
    {
      dbg("dxgi_sc: -- Got SoftTH D3D11 buffer");
      newbb11->AddRef();
      *ppSurface = newbb11;
    }
    else
    {
      dbg("dxgi_sc: -- Got SoftTH D3D10 buffer");
      newbb10->AddRef();
      *ppSurface = newbb10;
    }
    return S_OK;
  } else {
    // Return real backbuffer
    HRESULT ret = dxgsc->GetBuffer(Buffer, riid, ppSurface);
    dbg("dxgi_sc: -- Couldn't get SoftTH buffer. Getting real buffer 0x%08X",*ppSurface);
    return ret;
  }
}

HRESULT IDXGISwapChainNew::GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc)
{
  dbg("dxgi_sc: 0x%08X GetDesc", this);
  HRESULT ret = dxgsc->GetDesc(pDesc);
  if(newbb10 || newbb11) {
    // Pretend it is triplehead
    dbg("dxgi_sc: -- Got SoftTH buffer description");
    pDesc->BufferDesc.Width = config.main.renderResolution.x;
    pDesc->BufferDesc.Height = config.main.renderResolution.y;
  } else
    dbg("dxgi_sc: -- Got real buffer description");
  return ret;
};

HRESULT IDXGISwapChainNew::GetDesc1(DXGI_SWAP_CHAIN_DESC1 *pDesc)
{
  dbg("dxgi_sc: 0x%08X GetDesc1",this);
  HRESULT ret = dxgsc->GetDesc1(pDesc);
  if(newbb10 || newbb11) {
    // Pretend it is triplehead
    dbg("dxgi_sc: -- Got SoftTH buffer description");
    pDesc->Width = config.main.renderResolution.x;
    pDesc->Height = config.main.renderResolution.y;
  } else
    dbg("dxgi_sc: -- Got real buffer description");
  return ret;
};

HRESULT IDXGISwapChainNew::Present(UINT SyncInterval,UINT Flags)
{
  if(newbb11)
    dbg("dxgi_sc: 0x%08X newBB11 Present %d 0x%08X", this, SyncInterval, newbb11);
  else if(newbb10)
    dbg("dxgi_sc: 0x%08X newBB10 Present %d 0x%08X", this, SyncInterval, newbb10);
  else if(realbb11)
    dbg("dxgi_sc: 0x%08X RealBB11 Present %d 0x%08X", this, SyncInterval, realbb11);
  else if(realbb10)
    dbg("dxgi_sc: 0x%08X RealBB10 Present %d 0x%08X", this, SyncInterval, realbb10);

  if(!newbb10 && !newbb11) {
    // Not multihead mode, plain old present
    dbg("dxgi_sc: -- Not multihead, just plain present");
    return dxgsc->Present(SyncInterval, Flags);
  }

  #if defined(SOFTTHMAIN) || defined(D3D11)
  if(dev11)
  {
    D3D11_TEXTURE2D_DESC dt, ds;
    dbg("dxgi_sc: -- realbbdesc11... %d", realbb11);
    realbb11->GetDesc(&dt);
    newbb11->GetDesc(&ds);

    dbg("dxgi_sc: -- Source           : %dx%d ms%d %s", ds.Width, ds.Height, ds.SampleDesc.Count, getFormatDXGI(ds.Format));
    dbg("dxgi_sc: -- Primary Head     : %dx%d ms%d %s", dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));

    // Copy and Present the Primary Head
    HEAD *h = config.getPrimaryHead();
    D3D11_BOX sb = {(UINT)h->sourceRect.left, (UINT)h->sourceRect.top, 0, (UINT)h->sourceRect.right, (UINT)h->sourceRect.bottom, 1};
    ID3D11DeviceContext *dev11context;
    dev11->GetImmediateContext(&dev11context);
    dev11context->CopySubresourceRegion(realbb11, 0, 0, 0, 0, newbb11, 0, &sb);

    /*if(GetKeyState('O') < 0)
      D3DX11SaveTextureToFile(realbb10, D3DX10_IFF_JPG, "d:\\pelit\\_realbb.jpg");
    if(GetKeyState('P') < 0)
      D3DX11SaveTextureToFile(newbb10, D3DX10_IFF_JPG, "d:\\pelit\\_newbb.jpg");*/

    // If we have non-local adapters, stage a copy of the buffer for non-local access
    /*if (has_nonlocal) {
      // Copy from the main backbuffer to the main staged texture
      dev11context->CopyResource(newbb11staged,newbb11);
      // Map the main staged texture
      D3D11_MAPPED_SUBRESOURCE submain;
      dev11context->Map(newbb11staged, 0, D3D11_MAP_READ, 0, &submain);
    }*/




    // Copy and Present Secondary Heads
    for(int i=0;i<numDevs;i++)
    {
      OUTDEVICE11 *o  = &outDevs11[i];
      STAGINGOUT11 *so = &stagingOuts11[i];

      o->localSurf->GetDesc(&dt);
      dbg("dxgi_sc: -- Secondary Head %d : %dx%d ms%d %s", i+1, dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));
      sb = {(UINT)o->cfg->sourceRect.left, (UINT)o->cfg->sourceRect.top, 0, (UINT)o->cfg->sourceRect.right, (UINT)o->cfg->sourceRect.bottom, 1};

      // Check if the head is local and copy accordingly
      if (o->cfg->transportMethod == OUTMETHOD_LOCAL) {
        // Local head (on the primary video adapter)
        // - just copy the region directly to the head's localSurf
        dbg("dxgi_sc: -- Local head: CopySubresourceRegion");
        dev11context->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, newbb11, 0, &sb);
      } else {
        // Non-local head (on a secondary adapter
        dbg("dxgi_sc: -- Non-local head: Map/Unmap");
        // Copy from the main backbuffer to the main staged texture
        dev11context->CopySubresourceRegion(so->stagingSurf, 0, 0, 0, 0, newbb11, 0, &sb);
        // Map the main staged texture
        D3D11_MAPPED_SUBRESOURCE submain;
        if (dev11context->Map(so->stagingSurf, 0, D3D11_MAP_READ, 0, &submain) != S_OK)
          dbg("dxgi_sc: -- Mapping Main staging surface failed!");
        // Map the head's staged texture
        D3D11_MAPPED_SUBRESOURCE subhead;
        if (o->output->devContext->Map(o->output->stagingSurface, 0, D3D11_MAP_WRITE, 0, &subhead) != S_OK)
          dbg("dxgi_sc: -- Mapping Head %d staging surface failed!",i+1);
        // Copy from the main staged texture to the head's staged texture
        memcpy(subhead.pData, submain.pData, dt.Width*dt.Height*4);
        // Unmap the head's staged texture
        o->output->devContext->Unmap(o->output->stagingSurface, 0);
        // Unmap the main staged texture
        dev11context->Unmap(so->stagingSurf, 0);
        // Copy from head's staged texture to the head's localSurf
        o->output->devContext->CopyResource(o->localSurf, o->output->stagingSurface);
        //o->output->devContext->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, o->output->stagingSurface, 0, NULL);
        // Release the main staged texture
        //so->stagingSurf->Release();
      }

      /*if (has_nonlocal) {
        // Unmap the main staged texture
        dev11context->Unmap(newbb11staged, 0);
      }*/

      // Flush the main render pipeline (full SoftTH buffer)
      dev11context->Flush(); // DOESN'T APPEAR TO BE A NEED TO DO THIS MANUALLY!!!

      // Draw each secondary output
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
    dbg("dxgi_sc: -- realbbdesc10.1 ... %d", realbb10);
    realbb10->GetDesc(&dt);
    newbb10->GetDesc(&ds);

    dbg("dxgi_sc: -- Source: %dx%d ms%d %s", ds.Width, ds.Height, ds.SampleDesc.Count, getFormatDXGI(ds.Format));
    dbg("dxgi_sc: -- Target: %dx%d ms%d %s", dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));

    // Copy and Present the Primary Head
    HEAD *h = config.getPrimaryHead();
    D3D10_BOX sb = {(UINT)h->sourceRect.left, (UINT)h->sourceRect.top, 0, (UINT)h->sourceRect.right, (UINT)h->sourceRect.bottom, 1};
    dev10_1->CopySubresourceRegion(realbb10, 0, 0, 0, 0, newbb10, 0, &sb);

    /*if(GetKeyState('O') < 0)
      D3DX10SaveTextureToFile(realbb10, D3DX10_IFF_JPG, "d:\\pelit\\_realbb.jpg");
    if(GetKeyState('P') < 0)
      D3DX10SaveTextureToFile(newbb10, D3DX10_IFF_JPG, "d:\\pelit\\_newbb.jpg");*/

    /*// Copy & Present secondary heads
    for(int i=0;i<numDevs;i++)
    {
      OUTDEVICE10 *o = &outDevs10[i];
      D3D10_BOX sb = {o->cfg->sourceRect.left, o->cfg->sourceRect.top, 0, o->cfg->sourceRect.right, o->cfg->sourceRect.bottom, 1};
      dev10_1->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, newbb10, 0, &sb);
      dev10_1->Flush();

      o->output->present();
    }*/

    // Copy and Present Secondary Heads
    for(int i=0;i<numDevs;i++)
    {
      OUTDEVICE10 *o  = &outDevs10[i];
      STAGINGOUT10 *so = &stagingOuts10[i];

      o->localSurf->GetDesc(&dt);
      dbg("dxgi_sc: -- Secondary Head %d : %dx%d ms%d %s", i+1, dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));
      sb = {(UINT)o->cfg->sourceRect.left, (UINT)o->cfg->sourceRect.top, 0, (UINT)o->cfg->sourceRect.right, (UINT)o->cfg->sourceRect.bottom, 1};

      // Check if the head is local and copy accordingly
      if (o->cfg->transportMethod == OUTMETHOD_LOCAL) {
        // Local head (on the primary video adapter)
        // - just copy the region directly to the head's localSurf
        dbg("dxgi_sc: -- Local head: CopySubresourceRegion");
        dev10_1->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, newbb10, 0, &sb);
      } else {
        // Non-local head (on a secondary adapter
        dbg("dxgi_sc: -- Non-local head: Map/Unmap");
        // Copy from the main backbuffer to the main staged texture
        dev10_1->CopySubresourceRegion(so->stagingSurf, 0, 0, 0, 0, newbb10, 0, &sb);
        // Map the main staged texture
        D3D10_MAPPED_TEXTURE2D submain;
        if (so->stagingSurf->Map(0, D3D10_MAP_READ, 0, &submain) != S_OK)
          dbg("dxgi_sc: -- Mapping Main staging surface failed!");
        // Map the head's staged texture
        D3D10_MAPPED_TEXTURE2D subhead;
        if (o->output->stagingSurface->Map(0, D3D10_MAP_WRITE, 0, &subhead) != S_OK)
          dbg("dxgi_sc: -- Mapping Head %d staging surface failed!",i+1);
        // Copy from the main staged texture to the head's staged texture
        memcpy(subhead.pData, submain.pData, dt.Width*dt.Height*4);
        // Unmap the head's staged texture
        o->output->stagingSurface->Unmap(0);
        // Unmap the main staged texture
        so->stagingSurf->Unmap(0);
        // Copy from head's staged texture to the head's localSurf
        o->output->dev1->CopyResource(o->localSurf, o->output->stagingSurface);
        //o->output->devContext->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, o->output->stagingSurface, 0, NULL);
        // Release the main staged texture
        //so->stagingSurf->Release();
      }

      /*if (has_nonlocal) {
        // Unmap the main staged texture
        dev11context->Unmap(newbb11staged, 0);
      }*/

      // Flush the main render pipeline (full SoftTH buffer)
      dev10_1->Flush(); // DOESN'T APPEAR TO BE A NEED TO DO THIS MANUALLY!!!

      // Draw each secondary output
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
    dbg("dxgi_sc: -- realbbdesc10.1 ... %d", realbb10);
    realbb10->GetDesc(&dt);
    newbb10->GetDesc(&ds);

    dbg("dxgi_sc: -- Source: %dx%d ms%d %s", ds.Width, ds.Height, ds.SampleDesc.Count, getFormatDXGI(ds.Format));
    dbg("dxgi_sc: -- Target: %dx%d ms%d %s", dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));

    // Copy and Present the Primary Head
    HEAD *h = config.getPrimaryHead();
    D3D10_BOX sb = {(UINT)h->sourceRect.left, (UINT)h->sourceRect.top, 0, (UINT)h->sourceRect.right, (UINT)h->sourceRect.bottom, 1};
    dev10->CopySubresourceRegion(realbb10, 0, 0, 0, 0, newbb10, 0, &sb);

    /*if(GetKeyState('O') < 0)
      D3DX10SaveTextureToFile(realbb10, D3DX10_IFF_JPG, "d:\\pelit\\_realbb.jpg");
    if(GetKeyState('P') < 0)
      D3DX10SaveTextureToFile(newbb10, D3DX10_IFF_JPG, "d:\\pelit\\_newbb.jpg");*/

    /*// Copy & Present secondary heads
    for(int i=0;i<numDevs;i++)
    {
      OUTDEVICE10 *o = &outDevs10[i];
      D3D10_BOX sb = {o->cfg->sourceRect.left, o->cfg->sourceRect.top, 0, o->cfg->sourceRect.right, o->cfg->sourceRect.bottom, 1};
      dev10_1->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, newbb10, 0, &sb);
      dev10_1->Flush();

      o->output->present();
    }*/

    // Copy and Present Secondary Heads
    for(int i=0;i<numDevs;i++)
    {
      OUTDEVICE10 *o  = &outDevs10[i];
      STAGINGOUT10 *so = &stagingOuts10[i];

      o->localSurf->GetDesc(&dt);
      dbg("dxgi_sc: -- Secondary Head %d : %dx%d ms%d %s", i+1, dt.Width, dt.Height, dt.SampleDesc.Count, getFormatDXGI(dt.Format));
      sb = {(UINT)o->cfg->sourceRect.left, (UINT)o->cfg->sourceRect.top, 0, (UINT)o->cfg->sourceRect.right, (UINT)o->cfg->sourceRect.bottom, 1};

      // Check if the head is local and copy accordingly
      if (o->cfg->transportMethod == OUTMETHOD_LOCAL) {
        // Local head (on the primary video adapter)
        // - just copy the region directly to the head's localSurf
        dbg("dxgi_sc: -- Local head: CopySubresourceRegion");
        dev10->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, newbb10, 0, &sb);
      } else {
        // Non-local head (on a secondary adapter
        dbg("dxgi_sc: -- Non-local head: Map/Unmap");
        // Copy from the main backbuffer to the main staged texture
        dev10->CopySubresourceRegion(so->stagingSurf, 0, 0, 0, 0, newbb10, 0, &sb);
        // Map the main staged texture
        D3D10_MAPPED_TEXTURE2D submain;
        if (so->stagingSurf->Map(0, D3D10_MAP_READ, 0, &submain) != S_OK)
          dbg("dxgi_sc: -- Mapping Main staging surface failed!");
        // Map the head's staged texture
        D3D10_MAPPED_TEXTURE2D subhead;
        if (o->output->stagingSurface->Map(0, D3D10_MAP_WRITE, 0, &subhead) != S_OK)
          dbg("dxgi_sc: -- Mapping Head %d staging surface failed!",i+1);
        // Copy from the main staged texture to the head's staged texture
        memcpy(subhead.pData, submain.pData, dt.Width*dt.Height*4);
        // Unmap the head's staged texture
        o->output->stagingSurface->Unmap(0);
        // Unmap the main staged texture
        so->stagingSurf->Unmap(0);
        // Copy from head's staged texture to the head's localSurf
        o->output->dev1->CopyResource(o->localSurf, o->output->stagingSurface);
        //o->output->devContext->CopySubresourceRegion(o->localSurf, 0, 0, 0, 0, o->output->stagingSurface, 0, NULL);
        // Release the main staged texture
        //so->stagingSurf->Release();
      }

      /*if (has_nonlocal) {
        // Unmap the main staged texture
        dev11context->Unmap(newbb11staged, 0);
      }*/

      // Flush the main render pipeline (full SoftTH buffer)
      dev10->Flush(); // DOESN'T APPEAR TO BE A NEED TO DO THIS MANUALLY!!!

      // Draw each secondary output
      o->output->present();
    }
  }
  #endif // defined

  HRESULT ret = dxgsc->Present(SyncInterval, Flags);
  if(ret != S_OK)
    dbg("dxgi_sc: -- IDXGISwapChainNew::Present: Failed");
  return ret;
}

HRESULT IDXGISwapChainNew::ResizeTarget(const DXGI_MODE_DESC *tgtp)
{
  dbg("dxgi_sc: 0x%08X ResizeTarget %dx%d", this, tgtp->Width, tgtp->Height);

  DXGI_MODE_DESC m = *tgtp;
  m.Width = 1920;
  m.Height = 1200;

  HRESULT ret = dxgsc->ResizeTarget(&m);
  if(ret != S_OK)
    dbg("dxgi_sc: -- ResizeTarget failed %dx%d", tgtp->Width, tgtp->Height);
  return ret;
};

HRESULT IDXGISwapChainNew::ResizeBuffers(UINT BufferCount,UINT Width,UINT Height,DXGI_FORMAT NewFormat,UINT SwapChainFlags)
{
  dbg("dxgi_sc: 0x%08X ResizeBuffers %dx%d", this, Width, Height);
  preUpdateBB(&Width, &Height);
  HRESULT ret = dxgsc->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
  if(ret == S_OK) {
    updateBB(); // Backbuffer was recreated
  } else {
    dbg("dxgi_sc: ResizeBuffers failed!");
  }
  return ret;
};

void IDXGISwapChainNew::preUpdateBB(UINT *width, UINT *height)
{
  dbg("dxgi_sc: 0x%08X preUpdateBB", this);

  int rrx = config.main.renderResolution.x;
  int rry = config.main.renderResolution.y;

  if(*width == rrx && *height == rry) {
    dbg("dxgi_sc: -- Multihead swapchain mode detected");
    HEAD *h = config.getPrimaryHead();
    *width = h->screenMode.x;
    *height = h->screenMode.y;

    // Set mouse hook on application focus window
    ihGlobal.setHWND(win);
    SoftTHActive++;
    h->hwnd = win;

    // Create new backbuffer
    dbg("dxgi_sc: -- Creating new backbuffer");
    // TODO: format
    #if defined(SOFTTHMAIN) || defined(D3D11)
    if(dev11)
    {
      // Create the full backbuffer render texture
      dbg("dxgi_sc: -- Creating FULL backbuffer for D3D11 Device");
      //CD3D10_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, D3D10_BIND_RENDER_TARGET, D3D10_USAGE_DEFAULT, NULL);
      CD3D11_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, D3D11_BIND_RENDER_TARGET, D3D11_USAGE_DEFAULT, NULL);
      newbbDesc11 = d;
      if(dev11->CreateTexture2D(&newbbDesc11, NULL, &newbb11) != S_OK)
        dbg("dxgi_sc: -- CreateTexture2D failed :("), exit(0);

      // Initialize outputs
      numDevs = config.getNumAdditionalHeads();
      dbg("dxgi_sc: -- Initializing %d outputs", numDevs);
      int logoStopTime = GetTickCount() + 4000;

      bool fpuPreserve = true; // TODO: does this exist in d3d10?

      outDevs11 = new OUTDEVICE11[numDevs];
      stagingOuts11 = new STAGINGOUT11[numDevs];
      for(int i=0;i<numDevs;i++)
      {
        OUTDEVICE11 *o  = &outDevs11[i];
        STAGINGOUT11 *so = &stagingOuts11[i];
        so->headID = i+1;
        so->devID = h->devID;
        so->stagingSurf = NULL;

        // Create the output device
        HEAD *h = config.getHead(i);
        dbg("dxgi_sc: -- Initializing Head %d (DevID: %d)",i+1,h->devID);
        o->output = new outDirect3D11(h->devID, h->screenMode.x, h->screenMode.y, h->transportRes.x, h->transportRes.y, win);
        o->cfg = h;
        bool local = h->transportMethod==OUTMETHOD_LOCAL;
        if (!local) has_nonlocal = true;
        dbg("dxgi_sc: -- Head %d is %s", i+1, local?"local":"non-local");

        // Create a main staging buffer sized for this head if non-local
        if (!local) {
          dbg("dxgi_sc: -- Creating a main non-local staging buffer for Head %d (DevID %d)", i + 1, h->devID);
          CD3D11_TEXTURE2D_DESC dss(DXGI_FORMAT_R8G8B8A8_UNORM, h->transportRes.x, h->transportRes.y, 1, 1, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 1, 0, 0);
          /*DWORD32 *fillbuf = new DWORD32[h->transportRes.x*h->transportRes.y];
          for (int ii = 0; ii < h->transportRes.y; ii++)
            for (int jj = 0; jj < h->transportRes.x; jj++)
            {
              if ((ii&32)==(jj&32))
                fillbuf[ii*h->transportRes.x + jj] = (DWORD32) 0x0000ff00;
              else
                fillbuf[ii*h->transportRes.x + jj] = (DWORD32) 0xffffffff;
            }
          D3D11_SUBRESOURCE_DATA fillsr;
          ZeroMemory(&fillsr, sizeof(fillsr));
          fillsr.pSysMem = (void *)fillbuf;
          fillsr.SysMemPitch = h->transportRes.x * 4;
          fillsr.SysMemSlicePitch = h->transportRes.x * h->transportRes.y * 4;
          if (dev11->CreateTexture2D(&dss, &fillsr, &so->stagingSurf) != S_OK) {*/
          if (dev11->CreateTexture2D(&dss, NULL, &so->stagingSurf) != S_OK) {
            dbg("dxgi_sc: -- CreateTexture2D staged for Head %d (DevID %d) failed :(",i+1,h->devID), exit(0);
          }
        }

        // Create shared surfaces
        HANDLE sha = o->output->GetShareHandle();
        if(sha) {
          o->localSurf = NULL;

          { // Open surfA share handle
            ID3D11Resource *tr;
			      if (o->cfg->transportMethod == OUTMETHOD_LOCAL) {
              // Local output
			        if (dev11->OpenSharedResource(sha, __uuidof(ID3D11Resource), (void**)(&tr)) != S_OK)
				        dbg("dxgi_sc: -- Local OpenSharedResource A failed!"), exit(0);
			      }
			      else
			      {
              // Non-local output
				      if (o->output->dev->OpenSharedResource(sha, __uuidof(ID3D11Resource), (void**)(&tr)) != S_OK)
					      dbg("dxgi_sc: -- Non-local OpenSharedResource A failed!"), exit(0);
			      }
            if(tr->QueryInterface(__uuidof(ID3D11Texture2D), (void**)(&o->localSurf)) != S_OK)
              dbg("dxgi_sc: -- Shared surface QueryInterface failed!"), exit(0);
            tr->Release();
          }
          dbg("dxgi_sc: -- Opened share handles");
        } else
          dbg("dxgi_sc: -- ERROR: Head %d: No share handle!", i+1), exit(0);
      }

      // Create the full backbuffer staged texture if we have non-local head
      /*if (has_nonlocal) {
        CD3D11_TEXTURE2D_DESC ds(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, NULL, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ, 1, 0, D3D11_RESOURCE_MISC_SHARED);
        newbbDesc11staged = ds;
        if(dev11->CreateTexture2D(&newbbDesc11staged, NULL, &newbb11staged) != S_OK)
          dbg("dxgi_sc: CreateTexture2D staged failed :("), exit(0);
      }*/
    }
    #endif
    #ifdef SOFTTHMAIN
    else
    #endif
    #if defined(SOFTTHMAIN) || defined(D3D10_1)
    if(dev10_1)
    {
      dbg("dxgi_sc: -- Creating backbuffer for D3D10.1 Device");
      CD3D10_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, D3D10_BIND_RENDER_TARGET, D3D10_USAGE_DEFAULT, NULL);
      newbbDesc10 = d;
      if(dev10_1->CreateTexture2D(&newbbDesc10, NULL, &newbb10) != S_OK)
        dbg("dxgi_sc: -- CreateTexture2D failed :("), exit(0);

      // Initialize outputs
      numDevs = config.getNumAdditionalHeads();
      dbg("dxgi_sc: -- Initializing %d outputs", numDevs);
      int logoStopTime = GetTickCount() + 4000;

      bool fpuPreserve = true; // TODO: does this exist in d3d10?

      outDevs10 = new OUTDEVICE10[numDevs];
      stagingOuts10 = new STAGINGOUT10[numDevs];
      for(int i=0;i<numDevs;i++)
      {
        OUTDEVICE10 *o = &outDevs10[i];
        STAGINGOUT10 *so = &stagingOuts10[i];
        so->headID = i+1;
        so->devID = h->devID;
        so->stagingSurf = NULL;

        // Create the output device
        HEAD *h = config.getHead(i);
        dbg("dxgi_sc: -- Initializing Head %d (DevID: %d)",i+1,h->devID);
        o->output = new outDirect3D10(h->devID, h->screenMode.x, h->screenMode.y, h->transportRes.x, h->transportRes.y, win);
        o->cfg = h;
        bool local = h->transportMethod==OUTMETHOD_LOCAL;
        if (!local) has_nonlocal = true;
        dbg("dxgi_sc: -- Head %d is %s", i+1, local?"local":"non-local");

        // Create a main staging buffer sized for this head if non-local
        if (!local) {
          dbg("dxgi_sc: -- Creating a main non-local staging buffer for Head %d (DevID %d)", i + 1, h->devID);
          CD3D10_TEXTURE2D_DESC dss(DXGI_FORMAT_R8G8B8A8_UNORM, h->transportRes.x, h->transportRes.y, 1, 1, 0, D3D10_USAGE_STAGING, D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE, 1, 0, 0);
          /*DWORD32 *fillbuf = new DWORD32[h->transportRes.x*h->transportRes.y];
          for (int ii = 0; ii < h->transportRes.y; ii++)
            for (int jj = 0; jj < h->transportRes.x; jj++)
            {
              if ((ii&32)==(jj&32))
                fillbuf[ii*h->transportRes.x + jj] = (DWORD32) 0x0000ff00;
              else
                fillbuf[ii*h->transportRes.x + jj] = (DWORD32) 0xffffffff;
            }
          D3D11_SUBRESOURCE_DATA fillsr;
          ZeroMemory(&fillsr, sizeof(fillsr));
          fillsr.pSysMem = (void *)fillbuf;
          fillsr.SysMemPitch = h->transportRes.x * 4;
          fillsr.SysMemSlicePitch = h->transportRes.x * h->transportRes.y * 4;
          if (dev11->CreateTexture2D(&dss, &fillsr, &so->stagingSurf) != S_OK) {*/
          if (dev10_1->CreateTexture2D(&dss, NULL, &so->stagingSurf) != S_OK) {
            dbg("dxgi_sc: -- CreateTexture2D staged for Head %d (DevID %d) failed :(",i+1,h->devID), exit(0);
          }
        }

        // Create shared surfaces
        HANDLE sha = o->output->GetShareHandle();
        if(sha) {
          o->localSurf = NULL;

          { // Open surfA share handle
            ID3D10Resource *tr;
			      if (o->cfg->transportMethod == OUTMETHOD_LOCAL) {
              // Local output
			        if (dev10_1->OpenSharedResource(sha, __uuidof(ID3D10Resource), (void**)(&tr)) != S_OK)
				        dbg("dxgi_sc: -- Local OpenSharedResource A failed!"), exit(0);
			      }
			      else
			      {
              // Non-local output
				      if (o->output->dev1->OpenSharedResource(sha, __uuidof(ID3D10Resource), (void**)(&tr)) != S_OK)
					      dbg("dxgi_sc: -- Non-local OpenSharedResource A failed!"), exit(0);
			      }
            if(tr->QueryInterface(__uuidof(ID3D10Texture2D), (void**)(&o->localSurf)) != S_OK)
              dbg("dxgi_sc: -- Shared surface QueryInterface failed!"), exit(0);
            tr->Release();
          }
          dbg("dxgi_sc: -- Opened share handles");
        } else
          dbg("dxgi_sc: -- ERROR: Head %d: No share handle!", i+1), exit(0);
      }
    }
    #endif
    #ifdef SOFTTHMAIN
    else
    #endif
    #if defined(SOFTTHMAIN) || defined(D3D10)
    if (dev10)
    {
      dbg("dxgi_sc: -- Creating backbuffer for D3D10.1 Device");
      CD3D10_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, rrx, rry, 1, 1, D3D10_BIND_RENDER_TARGET, D3D10_USAGE_DEFAULT, NULL);
      newbbDesc10 = d;
      if(dev10->CreateTexture2D(&newbbDesc10, NULL, &newbb10) != S_OK)
        dbg("dxgi_sc: -- CreateTexture2D failed :("), exit(0);

      // Initialize outputs
      numDevs = config.getNumAdditionalHeads();
      dbg("dxgi_sc: -- Initializing %d outputs", numDevs);
      int logoStopTime = GetTickCount() + 4000;

      bool fpuPreserve = true; // TODO: does this exist in d3d10?

      outDevs10 = new OUTDEVICE10[numDevs];
      stagingOuts10 = new STAGINGOUT10[numDevs];
      for(int i=0;i<numDevs;i++)
      {
        OUTDEVICE10 *o = &outDevs10[i];
        STAGINGOUT10 *so = &stagingOuts10[i];
        so->headID = i+1;
        so->devID = h->devID;
        so->stagingSurf = NULL;

        // Create the output device
        HEAD *h = config.getHead(i);
        dbg("dxgi_sc: -- Initializing Head %d (DevID: %d)",i+1,h->devID);
        o->output = new outDirect3D10(h->devID, h->screenMode.x, h->screenMode.y, h->transportRes.x, h->transportRes.y, win);
        o->cfg = h;
        bool local = h->transportMethod==OUTMETHOD_LOCAL;
        if (!local) has_nonlocal = true;
        dbg("dxgi_sc: -- Head %d is %s", i+1, local?"local":"non-local");

        // Create a main staging buffer sized for this head if non-local
        if (!local) {
          dbg("dxgi_sc: -- Creating a main non-local staging buffer for Head %d (DevID %d)", i + 1, h->devID);
          CD3D10_TEXTURE2D_DESC dss(DXGI_FORMAT_R8G8B8A8_UNORM, h->transportRes.x, h->transportRes.y, 1, 1, 0, D3D10_USAGE_STAGING, D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE, 1, 0, 0);
          /*DWORD32 *fillbuf = new DWORD32[h->transportRes.x*h->transportRes.y];
          for (int ii = 0; ii < h->transportRes.y; ii++)
            for (int jj = 0; jj < h->transportRes.x; jj++)
            {
              if ((ii&32)==(jj&32))
                fillbuf[ii*h->transportRes.x + jj] = (DWORD32) 0x0000ff00;
              else
                fillbuf[ii*h->transportRes.x + jj] = (DWORD32) 0xffffffff;
            }
          D3D11_SUBRESOURCE_DATA fillsr;
          ZeroMemory(&fillsr, sizeof(fillsr));
          fillsr.pSysMem = (void *)fillbuf;
          fillsr.SysMemPitch = h->transportRes.x * 4;
          fillsr.SysMemSlicePitch = h->transportRes.x * h->transportRes.y * 4;
          if (dev11->CreateTexture2D(&dss, &fillsr, &so->stagingSurf) != S_OK) {*/
          if (dev10->CreateTexture2D(&dss, NULL, &so->stagingSurf) != S_OK) {
            dbg("dxgi_sc: -- CreateTexture2D staged for Head %d (DevID %d) failed :(",i+1,h->devID), exit(0);
          }
        }

        // Create shared surfaces
        HANDLE sha = o->output->GetShareHandle();
        if(sha) {
          o->localSurf = NULL;

          { // Open surfA share handle
            ID3D10Resource *tr;
			      if (o->cfg->transportMethod == OUTMETHOD_LOCAL) {
              // Local output
			        if (dev10->OpenSharedResource(sha, __uuidof(ID3D10Resource), (void**)(&tr)) != S_OK)
				        dbg("dxgi_sc: -- Local OpenSharedResource A failed!"), exit(0);
			      }
			      else
			      {
              // Non-local output
				      if (o->output->dev1->OpenSharedResource(sha, __uuidof(ID3D10Resource), (void**)(&tr)) != S_OK)
					      dbg("dxgi_sc: -- Non-local OpenSharedResource A failed!"), exit(0);
			      }
            if(tr->QueryInterface(__uuidof(ID3D10Texture2D), (void**)(&o->localSurf)) != S_OK)
              dbg("dxgi_sc: -- Shared surface QueryInterface failed!"), exit(0);
            tr->Release();
          }
          dbg("dxgi_sc: -- Opened share handles");
        } else
          dbg("dxgi_sc: -- ERROR: Head %d: No share handle!", i+1), exit(0);
      }
    }
    #endif

  } else {
    dbg("dxgi_sc: -- Singlehead swapchain mode");
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
  dbg("dxgi_sc: 0x%08X updateBB", this);

  // Get realbb
  if(dev11)
  {
    if(dxgsc->GetBuffer(0, IID_ID3D11Texture2D, (void**)&realbb11) != S_OK)
      dbg("dxgi_sc: -- GetBuffer failed!"), exit(0);

    realbb11->GetDesc(&realbbDesc11);
    dbg("dxgi_sc: -- Real Backbuffer: %dx%d ms%d %s", realbbDesc11.Width, realbbDesc11.Height, realbbDesc11.SampleDesc.Count, getFormatDXGI(realbbDesc11.Format));
    realbb11->Release();  // Pretend we didn't get pointer to it so it can be released by size changes
  }
  else if(dev10 || dev10_1)
  {
    if(dxgsc->GetBuffer(0, IID_ID3D10Texture2D, (void**)&realbb10) != S_OK)
      dbg("dxgi_sc: -- GetBuffer failed!"), exit(0);

    realbb10->GetDesc(&realbbDesc10);
    dbg("dxgi_sc: -- Real Backbuffer: %dx%d ms%d %s", realbbDesc10.Width, realbbDesc10.Height, realbbDesc10.SampleDesc.Count, getFormatDXGI(realbbDesc10.Format));
    realbb10->Release();  // Pretend we didn't get pointer to it so it can be released by size changes
  }
}
