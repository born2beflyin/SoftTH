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

// IDirect3D9 hooks

#include <windows.h>
#include <stdio.h>
#include "d3d.h"
#include "d3dtexture.h"
#include "d3dbuffer.h"
#include "d3dSoftTH.h"
#include "main.h"

IDirect3D9New *d3dhNew = NULL;  // Direct3D handle passed to application (IDirect3D9New)
std::vector<int> IDirect3D9New::refreshRates;

#if USE_D3DEX
IDirect3DDevice9Ex *d3dReal = NULL; // Real Direct3D Device
#else
IDirect3DDevice9 *d3dReal = NULL; // Real Direct3D Device
#endif

std::list<IDirect3D9New*> listD3D9New;  // List of created D3D9 handles so they can be released if app doesnt

//#define dbgf dbg

void releaseHangingD3D9New()
{
  // Destroy hanging IDirect3D9New objects
  std::list<IDirect3D9New*>::iterator i;
  while(!listD3D9New.empty()) {
    dbg("Releasing hanging IDirect3D9New object");
    i = listD3D9New.begin();
    delete (*i);
    listD3D9New.remove(*i);
  }
}

static void printDevices(IDirect3D9Ex *d3d)
{
  int ac = d3d->GetAdapterCount();
  for(int i=0;i<ac;i++)
  {
    D3DADAPTER_IDENTIFIER9 ai;
    d3d->GetAdapterIdentifier(i, 0, &ai);

    int Product = HIWORD(ai.DriverVersion.HighPart);
    int Version = LOWORD(ai.DriverVersion.HighPart);
    int SubVersion = HIWORD(ai.DriverVersion.LowPart);
    int Build = LOWORD(ai.DriverVersion.LowPart);
    dbg("Adapter %d: %s (%s v%d.%d.%d.%d) '%s'", i, ai.Description, ai.Driver, Product, Version, SubVersion, Build, ai.DeviceName);
  }
  dbg("");
}

void IDirect3D9New::releaseHanging()
{
  // Destroy hanging listD3D9DeviceSoftTH objects
  std::list<IDirect3DDevice9SoftTH*>::iterator i;
  while(!listD3D9DeviceSoftTH.empty()) {
    dbg("Releasing hanging IDirect3DDevice9SoftTH object");
    i = listD3D9DeviceSoftTH.begin();
    delete (*i);
    listD3D9DeviceSoftTH.remove(*i);
  }
}

// D3D device notifies IDirect3D9New of itself being destroyed
void IDirect3D9New::destroyed(IDirect3DDevice9SoftTH *ddev)
{
  listD3D9DeviceSoftTH.remove(ddev);
}


HRESULT IDirect3D9New::CreateDeviceEx(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pp,D3DDISPLAYMODEEX* pFullscreenDisplayMode,IDirect3DDevice9Ex** ppDev)
{
  dbg("IDirect3D9New::CreateDeviceEx 0x%08X", d3d);
  // TODO: Do we need to do something to pFullscreenDisplayMode?

  printDevices((IDirect3D9Ex*)d3d);
 
  IDirect3DDevice9SoftTH *d3dd = new IDirect3DDevice9SoftTH(this, (IDirect3D9Ex*)d3d, hFocusWindow, BehaviorFlags, pp);
  d3dd->setEx(true);
  HRESULT ret = d3dd->getCreateDeviceResult();
  if(ret == D3D_OK) {
    *ppDev = (IDirect3DDevice9Ex*) d3dd;
    listD3D9DeviceSoftTH.push_back(d3dd);
  } else
    delete d3dd;
  dbg("Created ex device 0x%08X", *ppDev);
  return ret;
}

HRESULT IDirect3D9New::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pp, IDirect3DDevice9** ppDev)
{
  dbg("IDirect3D9New::CreateDevice 0x%08X", d3d);

  //return d3d->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pp, ppDev);

#if !USE_D3DEX
  // Direct3DEx disabled: Debug device
  bool fullscreen = pp->Windowed==0;
  D3DDISPLAYMODEEX mode = {
    sizeof(D3DDISPLAYMODEEX),
    pp->BackBufferWidth,   pp->BackBufferHeight,
    pp->FullScreen_RefreshRateInHz,  pp->BackBufferFormat,
    D3DSCANLINEORDERING_PROGRESSIVE
  };
  IDirect3DDevice9Ex *paska;
  D3DCALL( d3d->CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pp, fullscreen?&mode:NULL, &paska) );

  *ppDev = new IDirect3DDevice9New(paska, d3d);
  return D3D_OK;

#else
  // Real device

  printDevices((IDirect3D9Ex*)d3d);

  IDirect3DDevice9SoftTH *d3dd = new IDirect3DDevice9SoftTH(this, (IDirect3D9Ex*)d3d, hFocusWindow, BehaviorFlags, pp);
  d3dd->setEx(false);
  HRESULT ret = d3dd->getCreateDeviceResult();
  if(ret == D3D_OK) {
    *ppDev = (IDirect3DDevice9*) d3dd;
    listD3D9DeviceSoftTH.push_back(d3dd);
  } else {
    dbg("createdevice failed!");
    delete d3dd;
  }
  dbg("Created device 0x%08X", *ppDev);
  return ret;
#endif
}

#if USE_D3DEX
IDirect3DDevice9New::IDirect3DDevice9New(IDirect3DDevice9Ex *device, IDirect3D9Ex *direct3D)
#else
IDirect3DDevice9New::IDirect3DDevice9New(IDirect3DDevice9 *device, IDirect3D9 *direct3D)
#endif
{
  //dbg("IDirect3DDevice9New CREATED");
  dev = device;
  d3d = direct3D;
  frameCounter = 0;
}

HRESULT IDirect3D9New::GetAdapterDisplayMode(UINT Adapter,D3DDISPLAYMODE* pMode)
{  
  if(Adapter != D3DADAPTER_DEFAULT)
    return D3DERR_INVALIDCALL;
  HRESULT ret = d3d->GetAdapterDisplayMode(Adapter, pMode);
  if(ret == D3D_OK && (SoftTHActive || config.overrides.forceResolution)) {
    pMode->Width = config.main.renderResolution.x;
    pMode->Height = config.main.renderResolution.y;
  }

  dbg("IDirect3D9New: GetAdapterDisplayMode return %dx%d", pMode->Width, pMode->Height);
  return ret;
}


UINT IDirect3D9New::GetAdapterCount()
{
  dbgf("IDirect3D9New::GetAdapterCount");
  //return d3d->GetAdapterCount();
  return 1; // SoftTH adapter only
}

// Get desktop refresh rates list
void IDirect3D9New::updateRefreshRates()
{
  if(!refreshRates.empty())
    return;
  char tmp[256] = "";
  refreshRates.clear();

  int modes = d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
  D3DDISPLAYMODE mode;
  d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
  for(int i=0;i<modes;i++) {
    D3DDISPLAYMODE m;
    d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &m);
    if(m.Width == mode.Width && m.Height == mode.Height) {
      refreshRates.push_back(m.RefreshRate);
      sprintf(tmp, "%s %dHz", tmp, m.RefreshRate);
    }
  }  
  dbg("Detected refresh rates:%s (%d)", tmp, refreshRates.size());  
}

HRESULT IDirect3D9New::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
  dbgf("d3d: EnumAdapterModes (%s)", getMode(Format));
  UINT numModes = d3d->GetAdapterModeCount(Adapter, Format);
  if(numModes>0 && Mode >= numModes && (Format == D3DFMT_A8R8G8B8 || Format == D3DFMT_X8R8G8B8))
  {
    pMode->RefreshRate = refreshRates[Mode-numModes];
    //d3d->GetAdapterDisplayMode(0, pMode); // Fill with desktop refresh rate    

    pMode->Width = config.main.renderResolution.x;
    pMode->Height = config.main.renderResolution.y;
    pMode->Format = Format;
    dbg("Added mode %d: %dx%d %dHz %s (%s)", Mode, pMode->Width, pMode->Height, pMode->RefreshRate, getMode(pMode->Format), getMode(Format));
    return D3D_OK;
  } else {
    HRESULT ret = d3d->EnumAdapterModes(Adapter, Format, Mode, pMode);
    //dbg("Enum mode %d: %dx%d %dHz %s (%s)", Mode, pMode->Width, pMode->Height, pMode->RefreshRate, getMode(pMode->Format), getMode(Format));
    return ret;
  }
}

UINT IDirect3D9New::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
  dbgf("d3d: GetAdapterModeCount");
  int modes = d3d->GetAdapterModeCount(Adapter, Format);
  if((Format == D3DFMT_A8R8G8B8 || Format == D3DFMT_X8R8G8B8) && modes > 0)
    modes+=(int)refreshRates.size();  // We add extra modes for all default adapter refresh rates
  dbgf("GetAdapterModeCount: Return %d modes (%s)", modes, getMode(Format));
  return modes;
}

HRESULT IDirect3D9New::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* id)
{
  dbgf("d3d: GetAdapterIdentifier %d %d %d", Adapter, Flags, id);
  HRESULT ret = d3d->GetAdapterIdentifier(Adapter, Flags, id);
  if(ret == D3D_OK) {
    // Rename device to SoftTH device
    sprintf(id->Description, "%s", SOFTTH_VERSION);
    //strcpy(id->Driver, "SoftTH");
  }
  return ret;
}




HRESULT IDirect3DDevice9New::GetDirect3D(IDirect3D9** ppD3D9)
{
  dbgf("dev: GetDirect3D");
  *ppD3D9 = d3d;
  d3d->AddRef();
  return D3D_OK;
}

void IDirect3DDevice9New::releaseManagedSysmemTexture(IDirect3DBaseTexture9* tex)
{
#if !NEW_MANAGED_TEXTURES
  if(!tex)
    return;
  // Is it a manage-emulated texture?
  IDirect3DTexture9Managed* texm = NULL;
  if(tex->QueryInterface(IID_IDirect3DTexture9Managed, (void**) &texm) == S_OK) {
    tex->Release();
    texm->ReleaseSysmemSurface();
  }
#endif
}

IDirect3DBaseTexture9* IDirect3DDevice9New::NewTextureFromOriginal(IDirect3DBaseTexture9* tex)
{
  if(!tex)
    return tex;

  {
    // Manage-emulated texture
    IDirect3DTexture9Managed *owner = NULL;
    DWORD size = sizeof(owner);
    tex->GetPrivateData(IID_IDirect3DTexture9Managed, &owner, &size);
    if(owner)
    {
      owner->AddRef();
      return owner;
    }
  }

  {
    // Normal texture
    IDirect3DTexture9New *owner = NULL;
    DWORD size = sizeof(owner);
    tex->GetPrivateData(IID_IDirect3DTexture9New, &owner, &size);
    if(owner)
    {
      owner->AddRef();
      return owner;
    }
  }

  {
    // Manage-emulated volume texture
    IDirect3DVolumeTexture9Managed *owner = NULL;
    DWORD size = sizeof(owner);
    tex->GetPrivateData(IID_IDirect3DVolumeTexture9Managed, &owner, &size);
    if(owner)
    {
      owner->AddRef();
      return owner;
    }
  }

  return tex;
}


IDirect3DBaseTexture9* IDirect3DDevice9New::OriginalFromNewTexture(IDirect3DBaseTexture9* tex)
{
  if(!tex)
    return tex;

  dbgf("IDirect3DDevice9New::OriginalFromNewTexture: 0x%08X (0x%08X)", tex, *tex);

  void* nilIF;
  // Manage-emulated texture?
  if(tex->QueryInterface(IID_IDirect3DTexture9Managed, &nilIF) == S_OK) {
    tex->Release();
    return ((IDirect3DTexture9Managed*) tex)->GetRealTexture();
  }
  // Manage-emulated volume texture?
  if(tex->QueryInterface(IID_IDirect3DVolumeTexture9Managed, &nilIF) == S_OK) {
    tex->Release();
    return ((IDirect3DVolumeTexture9Managed*) tex)->GetRealTexture();
  }
  // Overridden texture?
  if(tex->QueryInterface(IID_IDirect3DTexture9New, &nilIF) == S_OK) {
    tex->Release();
    return ((IDirect3DTexture9New*) tex)->GetRealTexture();
  }
  return tex;
}

IDirect3DSurface9* IDirect3DDevice9New::OriginalFromNewSurface(IDirect3DSurface9* surf)
{
  if(!surf)
    return surf; 
  void* nilIF;
  // Is this IID_IDirect3DSurface9New?
  if(surf->QueryInterface(IID_IDirect3DSurface9New, &nilIF) == S_OK) {
    surf->Release();
    IDirect3DSurface9* surfReal = ((IDirect3DSurface9New*) surf)->GetRealSurface();
    return surfReal;
  }
  return surf;
}

IDirect3DIndexBuffer9* IDirect3DDevice9New::OriginalFromNewIBuffer(IDirect3DIndexBuffer9* buf)
{
  if(!buf)
    return buf; 
  void* nilIF;
  // Is this IID_IDirect3DIndexBuffer9New?
  if(buf->QueryInterface(IID_IDirect3DIndexBuffer9Managed, &nilIF) == S_OK) {
    buf->Release();
    return ((IDirect3DIndexBuffer9Managed*) buf)->GetRealBuffer();
  }
  return buf;
}

IDirect3DVertexBuffer9* IDirect3DDevice9New::OriginalFromNewVBuffer(IDirect3DVertexBuffer9* buf)
{
  if(!buf)
    return buf; 
  void* nilIF;
  // Is this IID_IDirect3DVertexBuffer9Quirk?
  if(buf->QueryInterface(IID_IDirect3DVertexBuffer9Managed, &nilIF) == S_OK) {
    buf->Release();
    return ((IDirect3DVertexBuffer9Managed*) buf)->GetRealBuffer();
  }
  if(buf->QueryInterface(IID_IDirect3DVertexBuffer9Quirk, &nilIF) == S_OK) {
    buf->Release();
    return ((IDirect3DVertexBuffer9Quirk*) buf)->GetRealBuffer();
  }
  return buf;
}

HRESULT IDirect3DDevice9New::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
  dbgf("dev: CreateVertexBuffer %s %s fvf%d length%d %d", getUsage(Usage), getPool(Pool), FVF, Length, pSharedHandle);
  /*CHECKPOOL(Pool);*/

#if USE_D3DEX || MANAGE_DEBUG_VB
  if(config.debug.compatibleVB || config.debug.enableVBQuirk)
  {    
    if(config.debug.enableVBQuirk) ONCE dbg("DEBUG: Vertex buffer quirk mode enabled: Using compatibleVB");
    // Not so good performance but works ok
    if(Pool == D3DPOOL_MANAGED) Pool = D3DPOOL_DEFAULT;
  }
  else
  {
    if(config.debug.enableVBQuirk) dbg("ERROR: enableVBQuirk with manage-emulation!");
    if(Pool == D3DPOOL_MANAGED)
    {
      // Emulate manage-vertexbuffer
      *ppVertexBuffer = new IDirect3DVertexBuffer9Managed((IDirect3DDevice9Ex*)dev, this, Length, Usage, FVF, Pool, pSharedHandle);      
      return ((IDirect3DVertexBuffer9Managed*)*ppVertexBuffer)->GetResult();
    }        
  }
#endif

  HRESULT ret = dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
  if(ret == D3D_OK && !(Usage&D3DUSAGE_SOFTWAREPROCESSING)) // TODO: Why does Trackmania crash with D3DUSAGE_SOFTWAREPROCESSING here?
  {
    if(config.debug.enableVBQuirk)
    {
      // Vertex buffer quirk mode for old DCS-10C version (emulate D3D behaviour with illegal functionality)
      ONCE dbg("DEBUG: using vertex buffer quirk mode");      
      *ppVertexBuffer = new IDirect3DVertexBuffer9Quirk(*ppVertexBuffer, Length);
    }
  }
  return ret;
}

HRESULT IDirect3DDevice9New::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle) 
{
  dbgf("dev: CreateIndexBuffer %s %s", getUsage(Usage), getPool(Pool));

  if(config.debug.compatibleIB)
  {
    // Compatible index buffer mode - do not use emulated IB, set dynamic memory instead
    if(Pool == D3DPOOL_MANAGED)
    {
      ONCE dbg("DEBUG: using compatible index buffers (manage-emulation disabled)");
      Pool = D3DPOOL_DEFAULT;
      Usage |= D3DUSAGE_DYNAMIC;
    }
    return dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
  }

#if USE_D3DEX || MANAGE_DEBUG_IB
  /*if(Pool == D3DPOOL_MANAGED && Usage == D3DUSAGE_WRITEONLY)
    Pool = D3DPOOL_DEFAULT; // No need for manage-emulation for writeonly buffers, just set pool to default*/

  if(Pool == D3DPOOL_MANAGED)
  {
    *ppIndexBuffer = new IDirect3DIndexBuffer9Managed((IDirect3DDevice9Ex*)dev, this, Length, Usage, Format, Pool, pSharedHandle);
    // TODO: check for failure
    return D3D_OK;
  }
#endif
  return dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT IDirect3DDevice9New::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
  dbgf("dev: CreateCubeTexture: %d %d levels %s %s %s", EdgeLength, Levels, getPool(Pool), getUsage(Usage), getMode(Format));
  if(Pool == D3DPOOL_MANAGED)
  {
    dbg("WARNING: Emulating managed cube texture with D3DUSAGE_DYNAMIC");
    Pool = D3DPOOL_DEFAULT;
    Usage |= D3DUSAGE_DYNAMIC;
  }

  HRESULT ret = dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
  return ret;
}

HRESULT IDirect3DDevice9New::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
  dbgf("dev: CreateVolumeTexture: %dx%dx%d %d levels %s %s %s", Width, Height, Depth, Levels, getPool(Pool), getUsage(Usage), getMode(Format));
  
#if USE_D3DEX || MANAGE_DEBUG_VOLTEXTURE
  if(Pool == D3DPOOL_MANAGED && config.debug.compatibleTextures)
  {
    //dbg("WARNING: Emulating managed volume texture with D3DUSAGE_DYNAMIC");
    ONCE dbg("DEBUG: using compatible managed volume textures (manage-emulation disabled)");
    Pool = D3DPOOL_DEFAULT;
    Usage |= D3DUSAGE_DYNAMIC;
  }

  if(Pool == D3DPOOL_MANAGED)
  {
    // TODO: add error check
    *ppVolumeTexture = new IDirect3DVolumeTexture9Managed((IDirect3DDevice9Ex*)this, (IDirect3DDevice9Ex*)dev, Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
    return D3D_OK;
  } else {
    return dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
  }
#else
  return dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
#endif
}

HRESULT IDirect3DDevice9New::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle) 
{  
  dbgf("CreateTexture: %dx%d %d levels %s %s %s %d %s", Width, Height, Levels, getPool(Pool), getUsage(Usage), getMode(Format), pSharedHandle, pSharedHandle?"SHARED":"");
  int tb = GetTickCount();

#if USE_D3DEX || MANAGE_DEBUG_TEXTURE
  // Managed resources do not exist in D3D9Ex - need to emulate them :(
  if(Pool == D3DPOOL_MANAGED)
  {
    if(config.debug.compatibleTextures)
    {
      ONCE dbg("DEBUG: using compatible managed textures (manage-emulation disabled)");
      Pool = D3DPOOL_DEFAULT;
      Usage |= D3DUSAGE_DYNAMIC;
    }
    else
    {
      //dbg("CreateTexture: %dx%d %d levels %s %s %s %d %s (manage-emulation)", Width, Height, Levels, getPool(Pool), getUsage(Usage), getMode(Format), pSharedHandle, pSharedHandle?"SHARED":"");
      *ppTexture = new IDirect3DTexture9Managed((IDirect3DDevice9Ex*)this, (IDirect3DDevice9Ex*)dev, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
      IDirect3DTexture9Managed* t = (IDirect3DTexture9Managed*)*ppTexture;
      timeWarn(tb, 1000, "CreateTexture Managed");
      if(t->failed()) {
        HRESULT ret = t->error();
        delete t;
        return ret;
      } else {
        return D3D_OK;
      }
    }
  }
#endif
  

  //dbg("CreateTexture: CreateTexture");
  HRESULT ret = dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);

#if USE_D3DEX || MANAGE_DEBUG_TEXTURE
  timeWarn(tb, 1000, "CreateTexture");
  if(ret != D3D_OK) {
    dbg("CreateTexture failed! %d %d %d %s %s %s (%s)", Width, Height, Levels, getPool(Pool), getUsage(Usage), getMode(Format), getD3DError(ret));
  } else {    
    *ppTexture = new IDirect3DTexture9New((IDirect3DDevice9Ex*)this, *ppTexture);
  }
#endif

  //dbg("CreatedTexture: %dx%d %d levels %s %s %s %d %s 0x%08X 0x%08X", Width, Height, Levels, getPool(Pool), getUsage(Usage), getMode(Format), pSharedHandle, pSharedHandle?"SHARED":"", ppTexture, *ppTexture);

  return ret;
}
