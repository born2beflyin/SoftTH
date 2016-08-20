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

// Direct3D 10 output
#include "outD3D10.h"
#include "win32.h"
#include <process.h>
#include <vector>
//#include <d3d9.h>
#include "main.h"

#include "dxgiFactory.h"
#include "dxgiSwapChain.h"
//#include <D3DX10Tex.h> // Removed by CJR for SDK 8.1 - 9 Aug 2015

//#ifdef D3D10
/*
extern "C" HRESULT (WINAPI*dllD3D10CreateDeviceAndSwapChain)(IDXGIAdapter *adapter,
                                                             D3D10_DRIVER_TYPE DriverType,
                                                             HMODULE Software,
                                                             UINT Flags,
                                                             UINT SDKVersion,
                                                             DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                                             IDXGISwapChain **ppSwapChain,
                                                             ID3D10Device **ppDevice);
                                                             */
//#else
extern "C" __declspec(dllimport) HRESULT (WINAPI newD3D10CreateDeviceAndSwapChain1)(IDXGIAdapter *adapter,
                                                              D3D10_DRIVER_TYPE DriverType,
                                                              HMODULE Software,
                                                              UINT Flags,
                                                              D3D10_FEATURE_LEVEL1 HardwareLevel,
                                                              UINT SDKVersion,
                                                              DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                                              IDXGISwapChain **ppSwapChain,
                                                              ID3D10Device1 **ppDevice);
//#endif


outDirect3D10::outDirect3D10(int devID, int w, int h, int transX, int transY, HWND primaryFocusWindow)
{
  dbg("outDirect3D10: Initialize");

  // Get monitor info with D3D9
  IDirect3D9Ex *d3d9;
  D3DCALL( Direct3DCreate9Ex(-D3D_SDK_VERSION, &d3d9) );
  mInfo.cbSize = sizeof(MONITORINFO);
  mId = d3d9->GetAdapterMonitor(devID);
  GetMonitorInfo(mId, &mInfo);
  D3DADAPTER_IDENTIFIER9 ai;
  d3d9->GetAdapterIdentifier(devID, 0, &ai);
  dbg("outD3D10: This head's output is %s on <%s>", ai.DeviceName, ai.Description);
  d3d9->Release();

  // Get our head
  HEAD *head;
  int numDevs = config.getNumAdditionalHeads();
  for(int i=0;i<numDevs;i++) {
    HEAD *h = config.getHead(i);
    if(h->devID == devID)
      head = h;
  }

  // Set resolution to monitor size
  bbWidth = mInfo.rcMonitor.right - mInfo.rcMonitor.left;
  bbHeight = mInfo.rcMonitor.bottom - mInfo.rcMonitor.top;

  if(!head->screenMode.x)
    head->screenMode.x = bbWidth;
  if(!head->screenMode.y)
    head->screenMode.y = bbHeight;

  // Create output window
  int wflags = NULL;
  WINDOWPARAMS wp = {mInfo.rcMonitor.left, mInfo.rcMonitor.top, bbWidth, bbHeight, NULL, wflags, true, NULL}; // TODO: parent window?
  // Initialize the window
  wp.hWnd = NULL;
  _beginthread(windowHandler, 0, (void*) &wp);
  while(!wp.hWnd)
    Sleep(10);
  outWin = wp.hWnd;
  // Set this head's window
  head->hwnd = outWin;
  ShowWindow(outWin, SW_SHOWNA);

  // Create factory, then get the actual factory
  CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&dxgf));
  IDXGIFactory1New *fnew;
  if(dxgf->QueryInterface(IID_IDXGIFactory1New, (void**) &fnew) == S_OK) {
    dxgf = fnew->getReal();
    fnew->Release();
  }

  UINT i = 0;
  IDXGIAdapter *pAdapter;
  //IDXGIAdapter *vAdapters[64];
  dbg("outD3D10: Getting the correct adapter for this head ...");
  while(dxgf->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) {
    // Get the adapter description
    DXGI_ADAPTER_DESC desc;
    pAdapter->GetDesc(&desc);

    // Check the adapter name
    char *name = new char[128];
    WideCharToMultiByte(CP_ACP, 0, desc.Description, -1, name, 128, NULL, NULL);

    // Compare the name to the name we got above
    if (_strcmpi(name,ai.Description) == 0) {
      dbg("outD3D10: Got the adapter! Adapter %d: <%s>", i, name);
      /* Set this head's transport method */
      // Local if i == 0
      if (i == 0)
        head->transportMethod = OUTMETHOD_LOCAL;
      // Otherwise non-local
      else
        head->transportMethod = OUTMETHOD_NONLOCAL;

      // We found the adapter, so break out of the loop
      break;
    }
    ++i;
  }

  // Init Direct3D 10/10.1
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory( &sd, sizeof(sd) );
  sd.BufferCount = 1;
  sd.BufferDesc.Width = bbWidth;
  sd.BufferDesc.Height = bbHeight;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 0;
  sd.BufferDesc.RefreshRate.Denominator = 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = outWin;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;

  //D3D10_RESOURCE_MISC_SHARED

  //DWORD flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
  DWORD flags = 0;
  // TODO: verify devID match!
  //if(D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, flags, D3D10_SDK_VERSION, &sd, &swapChain, &dev) != S_OK) {
  //#ifdef D3D10
  //if(newD3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, flags, D3D10_SDK_VERSION, &sd, &swapChain, &dev) != S_OK) {
  //  dbg("D3D10CreateDeviceAndSwapChain FAILED");
  //#else
  if(newD3D10CreateDeviceAndSwapChain1(pAdapter,
                                       D3D10_DRIVER_TYPE_HARDWARE,
                                       NULL,
                                       flags,
                                       D3D10_FEATURE_LEVEL_10_1,
                                       D3D10_1_SDK_VERSION,
                                       &sd,
                                       &swapChain,
                                       &dev1) != S_OK) {
    dbg("outD3D10: D3D10CreateDeviceAndSwapChain1 FAILED");
  //#endif // D3D10
    exit(0);
  }
  dbg("outD3D10: Created D3D10/10.1 device & swap chain...");

  // Get true swapchain
  IDXGISwapChainNew *scnew;
  if(swapChain->QueryInterface(IID_IDXGISwapChainNew, (void**) &scnew) == S_OK) {
    swapChain = scnew->getReal();
    scnew->Release();
  } else
    dbg("outD3D10: Cannot get true swapchain!");

  // Get device backbuffer
  dbg("outD3D10: Creating secondary head (DevID %d) render backbuffer for D3D10/10.1 Device",devID);
  if(swapChain->GetBuffer( 0, __uuidof(ID3D10Texture2D), (LPVOID*) &bb) != S_OK) {
    dbg("swapChain->GetBuffer FAILED");
    exit(0);
  }

  // Create shared buffer
  dbg("outD3D10: Creating secondary head (DevID %d) shared transport buffer for D3D10/10.1 Device",devID);
  CD3D10_TEXTURE2D_DESC d(DXGI_FORMAT_R8G8B8A8_UNORM, transX, transY, 1, 1, D3D10_BIND_SHADER_RESOURCE|D3D10_BIND_RENDER_TARGET, D3D10_USAGE_DEFAULT, 0, 1, 0, D3D10_RESOURCE_MISC_SHARED);
  //#ifdef D3D10
  //if(dev->CreateTexture2D(&d, NULL, &sharedSurface) != S_OK)
  //#else
  if(dev1->CreateTexture2D(&d, NULL, &sharedSurface) != S_OK)
  //#endif // D3D10
    dbg("OutD3D10: CreateTexture2D failed :("), exit(0);

  // Create the staging surface
  if (head->transportMethod != OUTMETHOD_LOCAL) {
    CD3D10_TEXTURE2D_DESC dss(DXGI_FORMAT_R8G8B8A8_UNORM, transX, transY, 1, 1, 0, D3D10_USAGE_STAGING, D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE, 1, 0, 0);
    DWORD32 *fillbuf = new DWORD32[transX*transY];
    for (int ii = 0; ii < transY; ii++)
      for (int jj = 0; jj < transX; jj++)
      {
        if ((ii&32)==(jj&32))
          fillbuf[ii*transX + jj] = (DWORD32) 0x00ff0000;
        else
          fillbuf[ii*transX + jj] = (DWORD32) 0xffffffff;
      }
    D3D10_SUBRESOURCE_DATA fillsr;
    ZeroMemory(&fillsr, sizeof(fillsr));
    fillsr.pSysMem = (void *)fillbuf;
    fillsr.SysMemPitch = transX * 4;
    fillsr.SysMemSlicePitch = transX * transY * 4;
    if (dev1->CreateTexture2D(&dss, &fillsr, &stagingSurface) != S_OK) {
    //if (dev->CreateTexture2D(&dss, NULL, &stagingSurface) != S_OK) {
      dbg("outD3D10: CreateTexture2D staging surface failed!"), exit(0);
    }
  }

  // Get share handle
  IDXGIResource* texRes(NULL);
  sharedSurface->QueryInterface( __uuidof(IDXGIResource), (void**)&texRes);
  if (texRes->GetSharedHandle(&shareHandle) != S_OK)
    dbg("outD3D10: GetSharedHandle failed!");
  texRes->Release();
  dbg("outD3D10: Share handle: 0x%08X", shareHandle);

  //#ifdef D3D10
  //HRESULT ret = dev->CreateRenderTargetView(bb, NULL, &rttView);
  //#else
  HRESULT ret = dev1->CreateRenderTargetView(bb, NULL, &rttView);
  //#endif // D3D10
  bb->Release();
  if(FAILED(ret)) {
    dbg("outD3D10: dev1->CreateRenderTargetView FAILED");
    exit(0);
  }
  //#ifdef D3D10
  //dev->OMSetRenderTargets(1, &rttView, NULL);
  //#else
  dev1->OMSetRenderTargets(1, &rttView, NULL);
  //#endif // D3D10

  D3D10_VIEWPORT vp = {0, 0, bbWidth, bbHeight, 0, 1};
  //#ifdef D3D10
  //dev->RSSetViewports(1, &vp);
  //#else
  dev1->RSSetViewports(1, &vp);
  //#endif // D3D10

  dbg("outD3D10: Initialize  Head (DevID: %d) COMPLETE",devID);
  /*
  d3d9Surface = sourceSurface;

  // Create output window
  WINDOWPARAMS wp = {-1920+100, 100, 1024, 768};
  wp.hWnd = NULL;
  _beginthread(windowHandler, 0, (void*) &wp);
  while(!wp.hWnd)
    Sleep(10);
  outWin = wp.hWnd;

  // Find adapter
  IDXGIFactory * pFactory;
  CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

  UINT i = 0;
  IDXGIAdapter * pAdapter;
  IDXGIAdapter *vAdapters[64];
  dbg("Enum adapters...");
  while(pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) {
    vAdapters[i] = pAdapter;

    DXGI_ADAPTER_DESC desc;
    pAdapter->GetDesc(&desc);

    char *name = new char[128];
    WideCharToMultiByte(CP_ACP, 0, desc.Description, -1, name, 128, NULL, NULL);

    dbg("Adapter %d: <%s>", i, name);
    ++i;
  }


  // Init Direct3D 10
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory( &sd, sizeof(sd) );
  sd.BufferCount = 1;
  sd.BufferDesc.Width = 640;
  sd.BufferDesc.Height = 480;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = outWin;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;

  //D3D10_RESOURCE_MISC_SHARED

  //DWORD flags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
  DWORD flags = 0;
  if(D3D10CreateDeviceAndSwapChain(vAdapters[0], D3D10_DRIVER_TYPE_HARDWARE, NULL, flags, D3D10_SDK_VERSION, &sd, &swapChain, &dev) != S_OK) {
  //if(D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_REFERENCE, NULL, flags, D3D10_SDK_VERSION, &sd, &swapChain, &dev) != S_OK) {
    dbg("D3D10CreateDeviceAndSwapChain FAILED");
    exit(0);
  }

  // Create a render target view
  if(swapChain->GetBuffer( 0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer) != S_OK) {
    dbg("swapChain->GetBuffer FAILED");
    exit(0);
  }

  HRESULT ret = dev->CreateRenderTargetView(pBackBuffer, NULL, &rttView);
  pBackBuffer->Release();
  if(FAILED(ret)) {
    dbg("dev->CreateRenderTargetView FAILED");
    exit(0);
  }
  dev->OMSetRenderTargets(1, &rttView, NULL);

  ID3D10Texture2D *ppTexture;

  D3D10_TEXTURE2D_DESC Desc;
  Desc.Width              = 1024;
  Desc.Height             = 1024;
  Desc.MipLevels          = 1;
  Desc.ArraySize          = 1;
  Desc.Format             = DXGI_FORMAT_B8G8R8X8_UNORM;
  Desc.SampleDesc.Count   = 1;
  Desc.SampleDesc.Quality = 0;
  Desc.Usage              = D3D10_USAGE_DEFAULT;
  Desc.BindFlags          = D3D10_BIND_RENDER_TARGET;
  //Desc.BindFlags          = 0;
  Desc.CPUAccessFlags     = 0;
  Desc.MiscFlags          = D3D10_RESOURCE_MISC_SHARED;

  if(FAILED(dev->CreateTexture2D(&Desc, NULL, &ppTexture))) {
    dbg("dev->CreateTexture2D FAILED");
    exit(0);
  }

  IDXGIResource* pSurface;
  if(FAILED(ppTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&pSurface))) {
    dbg("dev->CreateTexture2D FAILED");
    exit(0);
  }
  HANDLE pHandle = NULL;
  pSurface->GetSharedHandle(&pHandle);
  pSurface->Release();

  d3d10Surface = pHandle;
  dbg("HANDLE %08X", pHandle);

  return;


  ID3D10Resource *tempResource10 = NULL;
  ID3D10Texture2D *sharedTex;
  dbg("OpenSharedResource 0x%08X", d3d9Surface);
  //if(FAILED(dev->OpenSharedResource(d3d9Surface, __uuidof(ID3D10Resource), (void**)(&tempResource10)))) {
  //if(FAILED(dev->OpenSharedResource(d3d9Surface, __uuidof(IDirect3DTexture9), (void**)(&tempResource10)))) {
  if(FAILED(dev->OpenSharedResource(d3d9Surface, __uuidof(ID3D10Texture2D), (void**)(&tempResource10)))) {
    dbg("dev->OpenSharedResource FAILED");
    exit(0);
  }
  dbg("QueryInterface %d", tempResource10);
  tempResource10->QueryInterface(__uuidof(ID3D10Texture2D), (void**)(&sharedTex));
  dbg("Release");
  tempResource10->Release();
  */
}

outDirect3D10::~outDirect3D10()
{
  dbg("outD3D10: destroy");
  DestroyWindow(outWin);
}

void outDirect3D10::present()
{
  // Copy from share surface to backbuffer & present
  dbg("outD3D10: Present - CopyResource...");

  // Removed by CJR for SDK 8.1 - 9 Aug 2015
  //if(GetKeyState('U') < 0)
  //  D3DX10SaveTextureToFile(sharedSurface, D3DX10_IFF_JPG, "d:\\pelit\\_sharedSurface.jpg");


  //#ifdef D3D10
  //dev->CopyResource(bb, sharedSurface);
  //dev->Flush();
  //#else
  dev1->CopyResource(bb, sharedSurface);
  dev1->Flush();
  //#endif
  /*D3D10_BOX sb = {0, 0, 0, 1920, 1200, 1};
  dev->CopySubresourceRegion(bb, 0, 0, 0, 0, sharedSurface, 0, &sb);

  dbg("CopySubresourceRegion done");
  /*
  dev->ClearRenderTargetView(rttView, D3DXVECTOR4(0, 1, 0, 1) );
  if(GetKeyState('U') < 0)
    if(D3DX10SaveTextureToFile(bb, D3DX10_IFF_JPG, "d:\\pelit\\_secondaryBB.jpg") != S_OK)
      dbg("ei sitte"), exit(0);
*/
  HRESULT r = swapChain->Present(0, 0);
  if(r != S_OK)
    dbg("outD3D10: Present failed 0x%08X", r);
}

/*
void outDirect3D10::presentFromBuffer()
{
  float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
  dev->ClearRenderTargetView(rttView, ClearColor);
  swapChain->Present(0, 0);
}*/
