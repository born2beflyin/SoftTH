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

#ifndef __D3D_TEXTURE_H__
#define __D3D_TEXTURE_H__

#define NEW_MANAGED_TEXTURES  1

#define MAX_LEVELS  32
#define ALL_LEVELS -1

#include "d3d.h"
#include "helper.h"

#undef nil
#undef dbgf

interface IDirect3DSurface9New;
interface IDirect3DTexture9New;

#define nil if(0)
#define dbgf if(0)
//#define dbgf dbg
//#define nil dbg

DEFINE_GUID(IID_IDirect3DTexture9New, 0x85c31227, 0x3de5, 0x4f00, 0x9b, 0x3a, 0xf1, 0x1a, 0xaa, 0xbb, 0xcc, 0xdd);

interface IDirect3DTexture9New : public IDirect3DTexture9
{
  IDirect3DTexture9New(IDirect3DDevice9Ex *device, IDirect3DTexture9 *texture) {
    dbgf("IDirect3DTexture9New: IDirect3DTexture9New %d dev0x%08X %d", this, device, texture);
    dev = device;
    tex = texture;

    // Store pointer to owner in actual texture
    if(tex)
    {
      IDirect3DTexture9New *thissi = this;
      tex->SetPrivateData(IID_IDirect3DTexture9New, &thissi, sizeof(this), NULL);
    }
  };
  ~IDirect3DTexture9New() {
    dbgf("IDirect3DTexture9New: ~IDirect3DTexture9New");    
    tex = NULL;
  };

    /*** IUnknown methods ***/
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      nil("IDirect3DTexture9New: QueryInterface %s", matchRiid(riid));
      if(riid == IID_IDirect3DTexture9New || riid == IID_IDirect3DTexture9 || riid == IID_IDirect3DBaseTexture9) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return tex->QueryInterface(riid, ppvObj);
  };
  STDMETHOD_(ULONG,AddRef)(THIS) {
    dbgf("IDirect3DTexture9New: AddRef");
    //dbg("-- ADDREF TEX: %08x", tex);
    return tex->AddRef();
  };
  STDMETHOD_(ULONG,Release)(THIS) {
    dbgf("IDirect3DTexture9New: Release");
    ULONG r = tex->Release();
    dbgf("IDirect3DTexture9New: Release result: %d", r);
    //dbg("-- RELEASE TEX: %08x", tex);
    if(r == 0) {
      // TODO: MEMLEAK: Release IDirect3DSurface9New allocated at GetSurfaceLevel (GetPrivateData(IID_IDirect3DSurface9New))
      //dbg("RELEASE TEX");
      delete this;
    }
    return r;
  };

  STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)  {
    dbgf("IDirect3DTexture9New: GetDevice");
    HRESULT ret = tex->GetDevice(ppDevice);
    if(ret == D3D_OK)
    {
      (*ppDevice)->Release();
      *ppDevice = (IDirect3DDevice9*) dev;
      (*ppDevice)->AddRef();
    }
    return ret;
  }
    //{dbgf("IDirect3DTexture9New: GetDevice");return tex->GetDevice(ppDevice);};
  STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) 
    {dbgf("IDirect3DTexture9New: SetPrivateData");return tex->SetPrivateData(refguid, pData, SizeOfData, Flags);};
  STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData) 
    {dbgf("IDirect3DTexture9New: GetPrivateData");return tex->GetPrivateData(refguid, pData, pSizeOfData);};
  STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) 
    {dbgf("IDirect3DTexture9New: FreePrivateData");return tex->FreePrivateData(refguid);};
  STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) 
    {dbgf("IDirect3DTexture9New: SetPriority");return tex->SetPriority(PriorityNew);};
  STDMETHOD_(DWORD, GetPriority)(THIS) 
    {dbgf("IDirect3DTexture9New: GetPriority");return tex->GetPriority();};
  STDMETHOD_(void, PreLoad)(THIS) 
    {dbgf("IDirect3DTexture9New: PreLoad");return tex->PreLoad();};
  STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) 
    {dbgf("IDirect3DTexture9New: GetType");return tex->GetType();};
  STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew) 
    {dbgf("IDirect3DTexture9New: SetLOD");tex->SetLOD(LODNew);return D3D_OK;};
    //{dbgf("IDirect3DTexture9New: SetLOD");return tex->SetLOD(LODNew);};
  STDMETHOD_(DWORD, GetLOD)(THIS) 
    {dbgf("IDirect3DTexture9New: GetLOD");return tex->GetLOD();};
  STDMETHOD_(DWORD, GetLevelCount)(THIS) 
    {dbgf("IDirect3DTexture9New: GetLevelCount");return tex->GetLevelCount();};
  STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType) 
    {dbgf("IDirect3DTexture9New: SetAutoGenFilterType");return tex->SetAutoGenFilterType(FilterType);};
  STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS) 
    {dbgf("IDirect3DTexture9New: GetAutoGenFilterType");return tex->GetAutoGenFilterType();};
  STDMETHOD_(void, GenerateMipSubLevels)(THIS) 
    {dbgf("IDirect3DTexture9New: GenerateMipSubLevels");return tex->GenerateMipSubLevels();};
  STDMETHOD(GetLevelDesc)(THIS_ UINT Level,D3DSURFACE_DESC *pDesc) 
    {dbgf("IDirect3DTexture9New: GetLevelDesc");
    HRESULT ret = tex->GetLevelDesc(Level, pDesc);
    return ret;
  };
  STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level,IDirect3DSurface9** ppSurfaceLevel) 
    ;//{dbgf("IDirect3DTexture9New: GetSurfaceLevel");return tex->GetSurfaceLevel(Level, ppSurfaceLevel);};
  STDMETHOD(LockRect)(THIS_ UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags) 
    {dbgf("IDirect3DTexture9New: LockRect");return tex->LockRect(Level, pLockedRect, pRect, Flags);};
  STDMETHOD(UnlockRect)(THIS_ UINT Level) 
    {dbgf("IDirect3DTexture9New: UnlockRect");return tex->UnlockRect(Level);};
  STDMETHOD(AddDirtyRect)(THIS_ CONST RECT* pDirtyRect)
    {dbgf("IDirect3DTexture9New: AddDirtyRect");return tex->AddDirtyRect(pDirtyRect);};

  IDirect3DTexture9* GetRealTexture() {dbgf("IDirect3DTexture9New: GetRealTexture: Returning 0x%08X (0x%08x)", tex, *tex);return tex;};
  bool failed() {return fail;};
  HRESULT error() {return retVal;};

private:
  IDirect3DTexture9 *tex;
  IDirect3DDevice9Ex *dev;
  bool fail;
  HRESULT retVal; // Return value in case of failure

  friend interface IDirect3DTexture9Managed;
};

// NEW Manage-emulated texture
DEFINE_GUID(IID_IDirect3DTexture9Managed, 0x85c31227, 0x3de5, 0x4f00, 0x9b, 0x3a, 0xf1, 0x1a, 0xaa, 0xbb, 0xFF, 0xFF);
interface IDirect3DTexture9Managed : public IDirect3DTexture9New
{
  IDirect3DTexture9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9Ex* deviceReal, UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbgf("IDirect3DTexture9Managed: QueryInterface %s", matchRiid(riid));
      if(riid == IID_IDirect3DTexture9Managed || riid == IID_IDirect3DTexture9 || riid == IID_IDirect3DBaseTexture9) {
        dbgf("IDirect3DTexture9Managed: QueryInterface: Returning self (0x%08X)", this);
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return tex?tex->QueryInterface(riid, ppvObj):S_FALSE;
  };

  STDMETHOD(LockRect)(THIS_ UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
  STDMETHOD_(void, GenerateMipSubLevels)(THIS);

  STDMETHOD(UnlockRect)(THIS_ UINT Level);

  IDirect3DTexture9* GetRealTexture();
  void setDirty(int level, const RECT *r) {
    if(level < 0)
    {
      for(int i=0;i<MAX_LEVELS;i++)
      {
        dirty[i]=true;
        if(r) dirtyRects[i].push_back(*r);
      }
    }
    else
    {
      dirty[level]=true;
      if(r) dirtyRects[level].push_back(*r);
    }
  };
  void setClean(int level) {
    if(level < 0)
      for(int i=0;i<MAX_LEVELS;i++)
        dirty[i]=false, dirtyRects[i].clear();
    else
      dirty[level]=false, dirtyRects[level].clear();
  };

  STDMETHOD_(ULONG,Release)(THIS);

private:
  HRESULT createSysTexture();
  HRESULT createVidTexture();
  void invalidateVidTexture();

  int texWidth, texHeight;
  D3DFORMAT texFormat;
  DWORD UsageSys, Usage;
  DWORD LevelsSys, Levels;
  DWORD LevelsReal;

  //bool useDynamicAllocation;
  bool dirty[MAX_LEVELS];
  std::vector<RECT> dirtyRects[MAX_LEVELS];
  //RECT dirtyRects[MAX_LEVELS];

  //IDirect3DTexture9 *texMem;  // SYSTEMMEM pool texture
  // *tex = // SYSTEMMEM pool texture (from parent class)
  IDirect3DTexture9 *texVid;  // DEFAULT pool texture
  IDirect3DDevice9Ex *dev;  // SoftTH device (or other)
  IDirect3DDevice9Ex *devReal; // Real d3d device
};

DEFINE_GUID(IID_IDirect3DSurface9New, 0xcfbaf3a, 0x9ff6, 0x429a, 0x99, 0xb3, 0xa2, 0x79, 0xaa, 0xbb, 0xcc, 0xdd);
interface IDirect3DSurface9New : public IDirect3DSurface9
{
  IDirect3DSurface9New(IDirect3DDevice9Ex *device, IDirect3DSurface9 *surface, IDirect3DTexture9New* ownerNew, DWORD ownerLockLevelNew) {
    dbgf("IDirect3DSurface9New: IDirect3DSurface9New %d dev0x%08X %d %d %d", this, device, surface, ownerNew, ownerLockLevelNew);
    surf = surface;
    dev = device;
    owner = ownerNew;
    ownerLockLevel = ownerLockLevelNew;

    dbgf("addref...");
    int c = surf->AddRef();
    dbgf("release... %d", c);
    refCount = surf->Release();
    dbgf("done!");
  };
  ~IDirect3DSurface9New() {
    dbgf("IDirect3DSurface9New: ~IDirect3DSurface9New");
  };
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbgf("IDirect3DSurface9New: QueryInterface %s", matchRiid(riid));
      if(riid == IID_IDirect3DSurface9New || riid == IID_IDirect3DSurface9) {
        dbgf("IDirect3DSurface9New: QueryInterface Returning self: %d", this);
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return surf->QueryInterface(riid, ppvObj);
  };

  STDMETHOD_(ULONG,AddRef)(THIS) {
    dbgf("IDirect3DSurface9New: AddRef... REF: %d", GetRef(surf));
    //dbg("-- ADDREF SURF: %08x", surf);
    return surf->AddRef();
  };
  STDMETHOD_(ULONG,Release)(THIS) {
    dbgf("IDirect3DSurface9New: Release... REF: %d", GetRef(surf));
    ULONG r = surf?surf->Release():0;
    //dbg("-- RELEASE SURF: %08x", surf);
    dbgf("IDirect3DSurface9New: Release %d", r);
    //if(r < refCount) { // leak wtf 1? oli 0
    if(r == 0) {
      surf = NULL;
      delete this;
    }
    return r;
  };

  STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) {
    dbgf("IDirect3DSurface9New: GetDevice");
    HRESULT ret = surf->GetDevice(ppDevice);
    if(ret == D3D_OK) {
      (*ppDevice)->Release();
      dbgf("IDirect3DSurface9New: GetDevice 0x%08X -> 0x%08X", *ppDevice, dev);
      *ppDevice = (IDirect3DDevice9*) dev;
      dev->AddRef();
    }
    return ret;
  }
    //{dbgf("IDirect3DSurface9New: GetDevice");return surf->GetDevice(ppDevice);};
  STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) 
    {dbgf("IDirect3DSurface9New: SetPrivateData");return surf->SetPrivateData(refguid, pData, SizeOfData, Flags);};
  STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData) 
    {dbgf("IDirect3DSurface9New: GetPrivateData");return surf->GetPrivateData(refguid, pData, pSizeOfData);};
  STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) 
    {dbgf("IDirect3DSurface9New: FreePrivateData");return surf->FreePrivateData(refguid);};
  STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) 
    {dbgf("IDirect3DSurface9New: SetPriority");return surf->SetPriority(PriorityNew);};
  STDMETHOD_(DWORD, GetPriority)(THIS) 
    {dbgf("IDirect3DSurface9New: GetPriority");return surf->GetPriority();};
  STDMETHOD_(void, PreLoad)(THIS) 
    {dbgf("IDirect3DSurface9New: PreLoad");return surf->PreLoad();};
  STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) 
    {dbgf("IDirect3DSurface9New: GetType");return surf->GetType();};
  STDMETHOD(GetContainer)(THIS_ REFIID riid,void** ppContainer) 
  {
    dbgf("IDirect3DSurface9New: GetContainer %s", matchRiid(riid));
    if(riid == IID_IDirect3DTexture9 && owner) {
      dbgf("IDirect3DSurface9New: GetContainer Returning parent: %d", owner);
      owner->AddRef();
      *ppContainer = owner;
      return S_OK;
    } else {
      dbg("IDirect3DSurface9New: GetContainer, not implemented! <%s>", matchRiid(riid));
      return surf->GetContainer(riid, ppContainer);
    }
  };
  STDMETHOD(GetDesc)(THIS_ D3DSURFACE_DESC *pDesc) 
    {dbgf("IDirect3DSurface9New: GetDesc");return surf->GetDesc(pDesc);};
  STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags) 
    {
      dbgf("IDirect3DSurface9New: LockRect");
      if(owner)
      {
        // Check if this is managed texture surface and mark it as dirty if so
        IDirect3DTexture9Managed *mtex = NULL;
        owner->QueryInterface(IID_IDirect3DTexture9Managed, (void**) &mtex);
        if(mtex)
        {
          if(Flags & D3DLOCK_NO_DIRTY_UPDATE) // Remove dirty-update flag - it isn't supposed to work for manage textures
            Flags -= D3DLOCK_NO_DIRTY_UPDATE;
          mtex->setDirty(ownerLockLevel, pRect);
          mtex->Release();
        }
      }
      return surf->LockRect(pLockedRect, pRect, Flags);
    };
    //{dbgf("IDirect3DSurface9New: LockRect");return surf->LockRect(pLockedRect, pRect, Flags);};
  STDMETHOD(UnlockRect)(THIS) 
    {dbgf("IDirect3DSurface9New: UnlockRect");return surf->UnlockRect();};
  STDMETHOD(GetDC)(THIS_ HDC *phdc) 
    {dbgf("IDirect3DSurface9New: GetDC");return surf->GetDC(phdc);};
  STDMETHOD(ReleaseDC)(THIS_ HDC hdc)
    {dbgf("IDirect3DSurface9New: ReleaseDC");return surf->ReleaseDC(hdc);};

  IDirect3DSurface9* GetRealSurface() {return surf;};

private:
  IDirect3DSurface9 *surf;
  IDirect3DDevice9Ex *dev;
  IDirect3DTexture9New* owner;
  DWORD ownerLockLevel;
  int refCount;

  friend interface IDirect3DSurface9Managed;
};


// Manage-emulated volume texture
DEFINE_GUID(IID_IDirect3DVolumeTexture9Managed, 0x67812342, 0x3de5, 0x4f00, 0xa9, 0xa7, 0xf1, 0xa9, 0xaa, 0xbb, 0xFF, 0xFF);
interface IDirect3DVolumeTexture9Managed : public IDirect3DVolumeTexture9
{
public:
  IDirect3DVolumeTexture9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9Ex* deviceReal, UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppTexture,HANDLE* pSharedHandle);

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbgf("IDirect3DVolumeTexture9Managed: QueryInterface %s", matchRiid(riid));
      if(riid == IID_IDirect3DVolumeTexture9Managed) {
        dbgf("IDirect3DVolumeTexture9Managed: QueryInterface: Returning self");
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return tex->QueryInterface(riid, ppvObj);
  };

  STDMETHOD_(ULONG,AddRef)(THIS) {
    dbgf("IDirect3DVolumeTexture9Managed: AddRef... REF: %d", GetRef(tex));
    return tex->AddRef();
  };
  STDMETHOD_(ULONG,Release)(THIS) {
    dbgf("IDirect3DVolumeTexture9Managed: Release... REF: %d", GetRef(tex));
    ULONG r = tex->Release();
    dbgf("IDirect3DVolumeTexture9Managed: Release %d", r);
    if(r == 0) { 
      while(texVid->Release());
      texVid = NULL;
      delete this;
    }
    return r;
  };

  STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) 
    {dbg("vtexm: WARNING: GetDevice, not implemented!");return tex->GetDevice(ppDevice);};
  STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) 
    {dbgf("vtexm:SetPrivateData");return tex->SetPrivateData(refguid, pData, SizeOfData, Flags);};
  STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData) 
    {dbgf("vtexm:GetPrivateData");return tex->GetPrivateData(refguid, pData, pSizeOfData);};
  STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) 
    {dbgf("vtexm:FreePrivateData");return tex->FreePrivateData(refguid);};
  STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) 
    {dbgf("vtexm:SetPriority");return tex->SetPriority(PriorityNew);};
  STDMETHOD_(DWORD, GetPriority)(THIS) 
    {dbgf("vtexm:GetPriority");return tex->GetPriority();};
  STDMETHOD_(void, PreLoad)(THIS) 
    {dbgf("vtexm:PreLoad");return tex->PreLoad();};
  STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) 
    {dbgf("vtexm:GetType");return tex->GetType();};
  STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew) 
    {dbgf("vtexm:SetLOD");return tex->SetLOD(LODNew);};
  STDMETHOD_(DWORD, GetLOD)(THIS) 
    {dbgf("vtexm:GetLOD");return tex->GetLOD();};
  STDMETHOD_(DWORD, GetLevelCount)(THIS) 
    {dbgf("vtexm:GetLevelCount");return tex->GetLevelCount();};
  STDMETHOD(SetAutoGenFilterType)(THIS_ D3DTEXTUREFILTERTYPE FilterType) 
    {dbgf("vtexm:SetAutoGenFilterType");return tex->SetAutoGenFilterType(FilterType);};
  STDMETHOD_(D3DTEXTUREFILTERTYPE, GetAutoGenFilterType)(THIS) 
    {dbgf("vtexm:GetAutoGenFilterType");return tex->GetAutoGenFilterType();};
  STDMETHOD_(void, GenerateMipSubLevels)(THIS) 
    {dbgf("vtexm: WARNING: GenerateMipSubLevels, not implemented!");return tex->GenerateMipSubLevels();};
  STDMETHOD(GetLevelDesc)(THIS_ UINT Level,D3DVOLUME_DESC *pDesc) 
    {dbgf("vtexm:GetLevelDesc");return tex->GetLevelDesc(Level, pDesc);};
  STDMETHOD(GetVolumeLevel)(THIS_ UINT Level,IDirect3DVolume9** ppVolumeLevel) 
    ;//{dbg("vtexm: WARNING: GetVolumeLevel, not implemented!");return tex->GetVolumeLevel(Level, ppVolumeLevel);};
  STDMETHOD(LockBox)(THIS_ UINT Level,D3DLOCKED_BOX* pLockedVolume,CONST D3DBOX* pBox,DWORD Flags) 
    {
      dbgf("vtexm:LockBox %s", getLock(Flags));
      if(pBox)
      {
        ONCE dbg("WARNING: Possible sub-rectangle lock on volume texture");
      }
      return tex->LockBox(Level, pLockedVolume, pBox, Flags);
    };
  STDMETHOD(UnlockBox)(THIS_ UINT Level)
    ;//{dbgf("vtexm:UnlockBox");return tex->UnlockBox(Level);};
  STDMETHOD(AddDirtyBox)(THIS_ CONST D3DBOX* pDirtyBox)
    {dbg("vtexm:AddDirtyBox");return tex->AddDirtyBox(pDirtyBox);};

  IDirect3DVolumeTexture9* GetRealTexture();

  void updateLevel(UINT Level);

  void setDirty(int level) {
    // TODO: support box argument (lockbox)
    if(level < 0)
    {
      for(int i=0;i<MAX_LEVELS;i++)
      {
        dirty[i]=true;
      }
    }
    else
    {
      dirty[level]=true;
    }
  };
  void setClean(int level) {
    if(level < 0)
      for(int i=0;i<MAX_LEVELS;i++)
        dirty[i]=false;
    else
      dirty[level]=false;
  };

private:
  IDirect3DVolumeTexture9 *tex;    // SYSTEMMEM pool texture
  IDirect3DVolumeTexture9 *texVid; // DEFAULT pool texture

  IDirect3DDevice9Ex *dev;  // SoftTH device (or other)
  IDirect3DDevice9Ex *devReal; // Real d3d device

  bool dirty[MAX_LEVELS];
  DWORD LevelsReal;
  UINT width, height, depth;
};

// 
DEFINE_GUID(IID_IDirect3DVolume9New, 0x24f416e6, 0x1f67, 0x4aa7, 0xb8, 0x8e, 0xd3, 0x3f, 0xaa, 0xbb, 0xFF, 0xFF);
interface IDirect3DVolume9New : public IDirect3DVolume9
{
  IDirect3DVolume9New(IDirect3DDevice9Ex *device, IDirect3DVolume9 *surface, IDirect3DVolumeTexture9Managed* ownerNew, DWORD ownerLockLevelNew) {
    dbgf("IDirect3DVolume9New: IDirect3DVolume9New %d dev0x%08X %d %d %d", this, device, surface, ownerNew, ownerLockLevelNew);
    surf = surface;
    dev = device;
    owner = ownerNew;
    ownerLockLevel = ownerLockLevelNew;
  };
  ~IDirect3DVolume9New() {
    dbgf("IDirect3DVolume9New: ~IDirect3DVolume9New");
  };
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbgf("IDirect3DVolume9New: QueryInterface %s", matchRiid(riid));
      if(riid == IID_IDirect3DVolume9New || riid == IID_IDirect3DVolume9) {
        dbgf("IDirect3DVolume9New: QueryInterface Returning self: %d", this);
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return surf->QueryInterface(riid, ppvObj);
  };

  STDMETHOD_(ULONG,AddRef)(THIS) {
    dbgf("IDirect3DVolume9New: AddRef... REF: %d", GetRef(surf));
    return surf->AddRef();
  };
  STDMETHOD_(ULONG,Release)(THIS) {
    dbgf("IDirect3DVolume9New: Release... REF: %d", GetRef(surf));
    ULONG r = surf?surf->Release():0;
    dbgf("IDirect3DVolume9New: Release %d", r);
    if(r == 0) {
      surf = NULL;
      delete this;
    }
    return r;
  };

  STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) {
    dbgf("IDirect3DVolume9New: GetDevice");
    HRESULT ret = surf->GetDevice(ppDevice);
    if(ret == D3D_OK) {
      (*ppDevice)->Release();
      dbgf("IDirect3DVolume9New: GetDevice 0x%08X -> 0x%08X", *ppDevice, dev);
      *ppDevice = (IDirect3DDevice9*) dev;
      dev->AddRef();
    }
    return ret;
  };

  STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) 
    {dbgf("IDirect3DVolume9New: SetPrivateData");return surf->SetPrivateData(refguid, pData, SizeOfData, Flags);};
  STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData) 
    {dbgf("IDirect3DVolume9New: GetPrivateData");return surf->GetPrivateData(refguid, pData, pSizeOfData);};
  STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) 
    {dbgf("IDirect3DVolume9New: FreePrivateData");return surf->FreePrivateData(refguid);};
  STDMETHOD(GetContainer)(THIS_ REFIID riid,void** ppContainer) 
    {dbg("IDirect3DVolume9New: GetContainer, not implemented!");return surf->GetContainer(riid, ppContainer);};
  STDMETHOD(GetDesc)(THIS_ D3DVOLUME_DESC *pDesc) 
    {dbgf("IDirect3DVolume9New: GetDesc");return surf->GetDesc(pDesc);};
  STDMETHOD(LockBox)(THIS_ D3DLOCKED_BOX * pLockedVolume,CONST D3DBOX* pBox,DWORD Flags)
    {
      dbgf("IDirect3DVolume9New: LockBox %s", getLock(Flags));
      if(pBox)
      {
        ONCE dbg("WARNING: Possible sub-rectangle lock on volume surface");
      }
      /*// TODO: enable optimized update like on ordinary texture*/
      if(owner)
      {
        // Check if this is managed texture surface and mark it as dirty if so
        IDirect3DVolumeTexture9Managed *mtex = NULL;
        owner->QueryInterface(IID_IDirect3DVolumeTexture9Managed, (void**) &mtex);
        if(mtex)
        {
          if(Flags & D3DLOCK_NO_DIRTY_UPDATE) // Remove dirty-update flag - it isn't supposed to work for manage textures
            Flags -= D3DLOCK_NO_DIRTY_UPDATE;
          mtex->setDirty(ownerLockLevel); // TODO: add box
          mtex->Release();
        }
      }
      return surf->LockBox(pLockedVolume, pBox, Flags);
    };
  STDMETHOD(UnlockBox)(THIS)
    {
      dbgf("IDirect3DVolume9New: UnlockBox");
      /*
      if(owner)
      {
        // Check if this is managed volume surface, if so update parent
        IDirect3DVolumeTexture9Managed *mtex = NULL;
        owner->QueryInterface(IID_IDirect3DVolumeTexture9Managed, (void**) &mtex);
        if(mtex)
        {
          HRESULT r = surf->UnlockBox();
          if(r == D3D_OK)
          {
            mtex->updateLevel(ownerLockLevel);
            mtex->Release();
          }
          return r;
        }
      }*/
      return surf->UnlockBox();
    };

private:
  IDirect3DVolume9 *surf;
  IDirect3DDevice9Ex *dev;
  IDirect3DVolumeTexture9Managed* owner;
  DWORD ownerLockLevel;
};

#if !NEW_MANAGED_TEXTURES
// Manage-emulated texture
DEFINE_GUID(IID_IDirect3DTexture9Managed, 0x85c31227, 0x3de5, 0x4f00, 0x9b, 0x3a, 0xf1, 0x1a, 0xaa, 0xbb, 0xFF, 0xFF);
interface IDirect3DTexture9Managed : public IDirect3DTexture9New
{
  IDirect3DTexture9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9Ex* deviceReal, UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle);
  ~IDirect3DTexture9Managed();

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      //dbg("IDirect3DTexture9Managed: QueryInterface");
      if(riid == IID_IDirect3DTexture9Managed) {
        dbgf("IDirect3DTexture9Managed: QueryInterface: Returning self (0x%08X)", this);
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return texVid?texVid->QueryInterface(riid, ppvObj):S_FALSE;
  };
  STDMETHOD_(ULONG,AddRef)(THIS);
  STDMETHOD_(ULONG,Release)(THIS);

  STDMETHOD(GetSurfaceLevel)(THIS_ UINT Level,IDirect3DSurface9** ppSurfaceLevel);
  STDMETHOD(LockRect)(THIS_ UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
  STDMETHOD(UnlockRect)(THIS_ UINT Level);
  STDMETHOD_(void, GenerateMipSubLevels)(THIS) 
    //{dbg("IDirect3DTexture9Managed: GenerateMipSubLevels UNIMPLEMENTED");return;};
    {dbgf("IDirect3DTexture9Managed 0x%08X: GenerateMipSubLevels %d", this, texVid);if(texVid)texVid->GenerateMipSubLevels();dbgf("IDirect3DTexture9Managed: GenerateMipSubLevels done");};
  STDMETHOD(GetLevelDesc)(THIS_ UINT Level,D3DSURFACE_DESC *pDesc) 
    {dbgf("IDirect3DTexture9Managed: GetLevelDesc %d - %d - %d", Level, pDesc, tex);
    HRESULT ret = tex->GetLevelDesc(Level, pDesc);
    pDesc->Pool = D3DPOOL_MANAGED;
    return ret;
  };

  STDMETHOD_(DWORD, SetLOD)(THIS_ DWORD LODNew) 
    {dbgf("IDirect3DTexture9New: SetLOD %d", LODNew);lod = LODNew;return D3D_OK;};
  STDMETHOD_(DWORD, GetLOD)(THIS) 
    {dbgf("IDirect3DTexture9New: GetLOD: Return %d", lod);return lod;};

  IDirect3DTexture9* GetRealTexture() {return texVid;}; // Handle passed to D3D when SetTexture is called
  IDirect3DTexture9* GetSRAMTexture() {return texMem;};

  void ReleaseSysmemSurface();
private:
  HRESULT createSysTexture();

#define MAX_LEVELS  32
  DWORD lod;
  RECT curLockRect[MAX_LEVELS];
  bool lockReadonly[MAX_LEVELS];
  bool curLockRectUsed[MAX_LEVELS];
  IDirect3DDevice9Ex *dev;  // SoftTH device (or other)
  IDirect3DDevice9Ex *devReal; // Real d3d device
  IDirect3DTexture9 *texMem;  // SYSTEMMEM pool texture
  IDirect3DTexture9 *texVid;  // DEFAULT pool texture

  int texWidth, texHeight;
  D3DFORMAT texFormat;
  DWORD UsageSys;
  DWORD LevelsSys;

  bool useDynamicAllocation;  // Deprecated
  bool useDynamicAllocationNew;
};
#endif

// Manage-emulated CUBE texture
/*
DEFINE_GUID(IID_IDirect3DCubeTexture9Managed, 0xfff32f81, 0xd953, 0x473a, 0x92, 0x23, 0x93, 0xd6, 0xaa, 0xbb, 0xFF, 0xFF);
interface IDirect3DCubeTexture9Managed : public IDirect3DTexture9New {
public: 
  IDirect3DCubeTexture9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9Ex* deviceReal, UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppTexture,HANDLE* pSharedHandle);
  ~IDirect3DCubeTexture9Managed();

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      //dbg("IDirect3DTexture9Managed: QueryInterface");
      if(riid == IID_IDirect3DCubeTexture9Managed) {
        dbgf("IDirect3DCubeTexture9Managed: QueryInterface: Returning self (0x%08X)", this);
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return texVid?texVid->QueryInterface(riid, ppvObj):S_FALSE;
  };

  STDMETHOD(GetCubeMapSurface)(THIS_ D3DCUBEMAP_FACES FaceType,UINT Level,IDirect3DSurface9** ppCubeMapSurface);
  STDMETHOD(LockRect)(THIS_ D3DCUBEMAP_FACES FaceType,UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
  STDMETHOD(UnlockRect)(THIS_ D3DCUBEMAP_FACES FaceType,UINT Level);
  STDMETHOD(AddDirtyRect)(THIS_ D3DCUBEMAP_FACES FaceType,CONST RECT* pDirtyRect) PURE;

  IDirect3DCubeTexture9* GetRealTexture() {return texVid;}; // Handle passed to D3D when SetTexture is called
  IDirect3DCubeTexture9* GetSRAMTexture() {return texMem;};

private:
  IDirect3DCubeTexture9 *texMem;  // SYSTEMMEM pool texture
  IDirect3DCubeTexture9 *texVid;  // DEFAULT pool texture
};*/


// Surface of a manage-emulated texture
interface IDirect3DSurface9Managed : public IDirect3DSurface9New
{
  IDirect3DSurface9Managed(IDirect3DDevice9Ex* device, IDirect3DSurface9 *surfaceMem, IDirect3DSurface9* surfaceVid, IDirect3DTexture9 **surfaceMemReference);
  ~IDirect3DSurface9Managed();

  STDMETHOD_(ULONG,AddRef)(THIS);
  STDMETHOD_(ULONG,Release)(THIS);
  STDMETHOD(LockRect)(THIS_ D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags);
  STDMETHOD(UnlockRect)(THIS);

private:
  IDirect3DDevice9Ex *dev;
  IDirect3DSurface9 *surfMem;
  IDirect3DTexture9 **surfMemRef;   // Pointer to owner texture of surfMem
  IDirect3DSurface9 *surfVid;

  bool fullLock;
  bool lockReadonly;
  RECT curLockedRect;
  int freeRefs;
};

#endif