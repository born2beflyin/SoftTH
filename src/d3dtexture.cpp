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

#include "main.h"
#include "d3dtexture.h"

// These GUIDs are used to detect new interfaces to override back to originals to pass to D3D
#include <INITGUID.H>
DEFINE_GUID(IID_IDirect3DSurface9New, 0xcfbaf3a, 0x9ff6, 0x429a, 0x99, 0xb3, 0xa2, 0x79, 0xaa, 0xbb, 0xcc, 0xdd);
DEFINE_GUID(IID_IDirect3DTexture9New, 0x85c31227, 0x3de5, 0x4f00, 0x9b, 0x3a, 0xf1, 0x1a, 0xaa, 0xbb, 0xcc, 0xdd);
DEFINE_GUID(IID_IDirect3DTexture9Managed, 0x85c31227, 0x3de5, 0x4f00, 0x9b, 0x3a, 0xf1, 0x1a, 0xaa, 0xbb, 0xFF, 0xFF);
DEFINE_GUID(IID_IDirect3DCubeTexture9Managed, 0xfff32f81, 0xd953, 0x473a, 0x92, 0x23, 0x93, 0xd6, 0xaa, 0xbb, 0xFF, 0xFF);
DEFINE_GUID(IID_IDirect3DVolumeTexture9Managed, 0x67812342, 0x3de5, 0x4f00, 0xa9, 0xa7, 0xf1, 0xa9, 0xaa, 0xbb, 0xFF, 0xFF);
DEFINE_GUID(IID_IDirect3DVolume9New, 0x24f416e6, 0x1f67, 0x4aa7, 0xb8, 0x8e, 0xd3, 0x3f, 0xaa, 0xbb, 0xFF, 0xFF);

// Get surface level of texture
HRESULT IDirect3DTexture9New::GetSurfaceLevel(UINT Level,IDirect3DSurface9** ppSurfaceLevel) 
{
  dbgf("GetSurfaceLevel NEW %d %d %d", tex, this, Level);
  HRESULT ret = tex->GetSurfaceLevel(Level, ppSurfaceLevel);

  if(ret == D3D_OK) {
    /*D3DSURFACE_DESC d;
    (*ppSurfaceLevel)->GetDesc(&d);
    dbgf("surffi: %dx%d", d.Width, d.Height);*/

    IDirect3DSurface9New *prevPtr = NULL;
    DWORD size = sizeof(prevPtr);
    (*ppSurfaceLevel)->GetPrivateData(IID_IDirect3DSurface9New, &prevPtr, &size);
    if(prevPtr) {
      // Return pointer to previous created IDirect3DSurface9New
      *ppSurfaceLevel = prevPtr;
      dbgf("GetSurfaceLevel Using previous pointer %d %d size %d", ppSurfaceLevel, *ppSurfaceLevel, size);
      (*ppSurfaceLevel)->AddRef();
      return ret; 
    } else {
      // Create new IDirect3DSurface9New
      IDirect3DSurface9 *prevPtr = *ppSurfaceLevel;
      *ppSurfaceLevel = new IDirect3DSurface9New(dev, *ppSurfaceLevel, this, Level);
      prevPtr->SetPrivateData(IID_IDirect3DSurface9New, ppSurfaceLevel, size, NULL);
      dbgf("GetSurfaceLevel Created new pointer %d %d", ppSurfaceLevel, *ppSurfaceLevel);
      return ret; 
    }
  }
  return ret; // failed

  /*
  if(ret == D3D_OK)
    *ppSurfaceLevel = new IDirect3DSurface9New(dev, *ppSurfaceLevel, this, Level); 
  return ret;
  */
}

// Get volume level of managed volume texture
HRESULT IDirect3DVolumeTexture9Managed::GetVolumeLevel(THIS_ UINT Level,IDirect3DVolume9** ppVolumeLevel) 
{
  dbgf("GetVolumeLevel");
  HRESULT ret = tex->GetVolumeLevel(Level, ppVolumeLevel);
  if(ret == D3D_OK)
    *ppVolumeLevel = new IDirect3DVolume9New(dev, *ppVolumeLevel, this, Level);
  return ret;
}

  ///////////////////////////////
 // Managed texture emulation //
///////////////////////////////
IDirect3DTexture9Managed::IDirect3DTexture9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9Ex* deviceReal, UINT Width,UINT Height,UINT LevelsWanted,DWORD UsageWanted,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
/*:IDirect3DTexture9New(device, *ppTexture)*/
:IDirect3DTexture9New(device, NULL)
{
  ONCE dbg("Using Texture manage-emulation");
  fail = false;
  texVid = NULL;
  dev = device;
  devReal = deviceReal;

  setClean(ALL_LEVELS);

  texWidth = Width;
  texHeight = Height;
  texFormat = Format;

  Usage = UsageWanted;
  Levels = LevelsWanted;

  UsageSys = Usage;
  LevelsSys = Levels;
  if(UsageSys & D3DUSAGE_AUTOGENMIPMAP)
  {
    // Use autogenmipmap only for video mem texture - sysmem one level only
    UsageSys -= D3DUSAGE_AUTOGENMIPMAP;
    LevelsSys =  1;
  }

  for(int i=0;i<MAX_LEVELS;i++)
    dirtyRects[i].reserve(2); // Preallocate some space

  tex = NULL;
  HRESULT ret = createSysTexture();
  if(ret != D3D_OK) {
    fail = true;
    retVal = ret;
  }

  //IDirect3DTexture9New(device, *ppTexture);
}

HRESULT IDirect3DTexture9Managed::createSysTexture()
{
  dbgf("IDirect3DTexture9Managed: Creating system texture (%dx%d %d levels) %s, %s", texWidth, texHeight, LevelsSys, getMode(texFormat), getUsage(UsageSys));
  if(tex)
    dbg("IDirect3DTexture9Managed::createSysTexture: WARNING: Texture already allocated!");
  HRESULT ret = devReal->CreateTexture(texWidth, texHeight, LevelsSys, UsageSys, texFormat, D3DPOOL_SYSTEMMEM, &tex, NULL);
  if(ret != D3D_OK) {
    dbg("IDirect3DTexture9Managed: CreateTexture D3DPOOL_SYSTEMMEM failed: %s (%dx%d %d levels) %s, %s", getD3DError(ret), texWidth, texHeight, LevelsSys, getMode(texFormat), getUsage(UsageSys));
    fail = true;
    retVal = ret;
  } else {
    LevelsReal = tex->GetLevelCount();
  }
  dbgf("IDirect3DTexture9Managed: Created sysmem texture 0x%08X (0x%08X), %d levels", tex, *tex, LevelsReal);
  return ret;
}

// Create video memory texture
HRESULT IDirect3DTexture9Managed::createVidTexture()
{
  dbgf("IDirect3DTexture9Managed: Creating video memory texture (%dx%d %d levels) %s, %s", texWidth, texHeight, Levels, getMode(texFormat), getUsage(Usage));
  if(texVid)
    dbg("IDirect3DTexture9Managed::createVidTexture: WARNING: Texture already allocated!");
  HRESULT ret = devReal->CreateTexture(texWidth, texHeight, Levels, Usage, texFormat, D3DPOOL_DEFAULT, &texVid, NULL);
  if(ret != D3D_OK) {
    dbg("IDirect3DTexture9Managed: CreateTexture D3DPOOL_DEFAULT failed: %s (%dx%d %d levels) %s, %s", getD3DError(ret), texWidth, texHeight, LevelsSys, getMode(texFormat), getUsage(UsageSys));
    texVid = NULL;
  }
  else
  {
    // Store pointer to self in texture - used in GetTexture to retrieve owner    
    IDirect3DTexture9Managed *thissi = this;
    texVid->SetPrivateData(IID_IDirect3DTexture9Managed, &thissi, sizeof(this), NULL);    
  }
  dbgf("IDirect3DTexture9Managed: Created vidmem texture 0x%08X (0x%08X), %d levels", texVid, *texVid, texVid->GetLevelCount());
  return ret;
}

// Release video memory texture
void IDirect3DTexture9Managed::invalidateVidTexture()
{
  if(!texVid)
    return;
  while(texVid->Release());
  texVid = NULL;
}

// Upload texture to video card
IDirect3DTexture9* IDirect3DTexture9Managed::GetRealTexture()
{
  if(!texVid) 
  {
    // Create vidmem surface...
    createVidTexture();
    setDirty(ALL_LEVELS, NULL);
  }
  if(!texVid) {
    dbg("ERROR: IDirect3DTexture9Managed::GetRealTexture: No video memory texture!");
    return NULL; // Uh-oh
  }

  if(LevelsReal == 1 && dirty[0])
  {
    // Only single level - can use updatetexture (faster!)
    devReal->UpdateTexture(tex, texVid);
    setClean(0);
  }
  else
  {
    // Update each level manually
    for(DWORD i=0;i<LevelsReal;i++) {
      if(dirty[i]) {
        dbgf("IDirect3DTexture9Managed::GetRealTexture 0x%08X: Updating level %d (%s)", this, i, dirtyRects[i].size()==0?"FULL":"PARTIAL");
        IDirect3DSurface9 *svid = NULL, *smem = NULL;

        D3DCALL( texVid->GetSurfaceLevel(i, &svid) );
        D3DCALL( tex->GetSurfaceLevel(i, &smem) );
        if(smem && svid) {
          //dbg("update TEXTURE level %d", i);
          size_t num = dirtyRects[i].size();
          if(num == 0) {
            // No lockrects defined, full surface update
            D3DCALL( devReal->UpdateSurface(smem, NULL, svid, NULL) );
          } else {          
            for(DWORD j=0;j<num;j++) {
              //dbg("%d dirtyrect (level %d) (%dx%d - %dx%d)", j, i, dirtyRects[i][j].left, dirtyRects[i][j].top, dirtyRects[i][j].right, dirtyRects[i][j].bottom);
              POINT p = {dirtyRects[i][j].left, dirtyRects[i][j].top};
              D3DCALL( devReal->UpdateSurface(smem, &dirtyRects[i][j], svid, &p) );
            }
          }
        }
        if(svid) svid->Release();
        if(smem) smem->Release();
        setClean(i);
      }
    }
  }

  dbgf("IDirect3DTexture9Managed: GetRealTexture: Returning 0x%08X (0x%08x)", texVid, *texVid);
  return texVid;
}

HRESULT IDirect3DTexture9Managed::UnlockRect(UINT Level)
{
  HRESULT ret = tex->UnlockRect(Level);
  dbgf("IDirect3DTexture9Managed::UnlockRect %d", Level);  

  return ret;
}

HRESULT IDirect3DTexture9Managed::LockRect(UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
{  
  if(Flags & D3DLOCK_NO_DIRTY_UPDATE) // Remove dirty-update flag - it isn't supposed to work for manage textures
    Flags -= D3DLOCK_NO_DIRTY_UPDATE;
  HRESULT ret = tex->LockRect(Level, pLockedRect, pRect, Flags);
  if(ret == D3D_OK)
  {
    dbgf("IDirect3DTexture9Managed::LockRect: Marked level %d as dirty (%d %s)", Level, pRect, getLock(Flags));
    /*D3DSURFACE_DESC desc;
    tex->GetLevelDesc(Level, &desc);
    dbg("Dirt %d, %s (%dx%d %d levels) %s", Level, strRect(pRect), desc.Width, desc.Height, tex->GetLevelCount(), getLock(Flags));*/
    setDirty(Level, pRect);

    //memset(pLockedRect->pBits, 0, pLockedRect->Pitch*texHeight);
  } else {
    dbg("IDirect3DTexture9Managed::LockRect: Lock failed!");    
  }

  return ret;
}

void IDirect3DTexture9Managed::GenerateMipSubLevels()
{
  if(!this) {
    // Empire: total war crashes here with null this pointer??
    dbg("IDirect3DTexture9Managed: this pointer = NULL!");
    return;
  }
  dbgf("IDirect3DTexture9Managed: GenerateMipSubLevels %dx%d", texWidth, texHeight);
  // Want to create mipmaps, so propably wants the texture available as well
  IDirect3DTexture9 *texTmp = GetRealTexture();
  if(texTmp)
    texTmp->GenerateMipSubLevels();
  return;
}

ULONG IDirect3DTexture9Managed::Release()
{
  dbgf("IDirect3DTexture9Managed: Release... REF: mem%d vid%d", GetRef(tex), GetRef(texVid));

  ULONG r = tex?tex->Release():0;
  if(r == 0)
  {
    dbgf("IDirect3DTexture9Managed: FREED");
    invalidateVidTexture();
    tex = NULL;
    delete this;
  }
  return r;
};


#if !NEW_MANAGED_TEXTURES
  ///////////////////////////////
 // Managed texture emulation //
///////////////////////////////
IDirect3DTexture9Managed::IDirect3DTexture9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9Ex* deviceReal, UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
:IDirect3DTexture9New(device, *ppTexture)
{
  ONCE dbg("Using Texture manage-emulation");
  fail = false;
  texVid = NULL;
  texMem = NULL;
  dbgf("IDirect3DTexture9Managed 0x%08X: New texture (%dx%d) Usage: %s, Levels: %d", this, Width, Height, getUsage(Usage), Levels);
  dev = device;
  devReal = deviceReal;

  texWidth = Width;
  texHeight = Height;
  texFormat = Format;

  HRESULT ret;
  // D3DUSAGE_AUTOGENMIPMAP is not valid for SYSTEMMEM - only use it for DEFAULT pool texture
  UsageSys = Usage;
  LevelsSys = Levels;
  if(UsageSys & D3DUSAGE_AUTOGENMIPMAP)
    UsageSys -= D3DUSAGE_AUTOGENMIPMAP;

  ret = devReal->CreateTexture(Width, Height, Levels, Usage, Format, D3DPOOL_DEFAULT, &texVid, pSharedHandle);
  if(ret != D3D_OK) {
    dbg("IDirect3DTexture9Managed: CreateTexture D3DPOOL_DEFAULT failed: %s (%dx%d %d levels) %s, %s", getD3DError(ret), Width, Height, Levels, getMode(Format), getUsage(Usage));
    fail = true;
    retVal = ret;
  } else {
    // In case of automatic levels generation (zero levels set), SYSTEMMEM doesnt auto-generate mipmaps
    LevelsSys = texVid->GetLevelCount();
  }

  // Use dynamic allocation for FOURCC textures (ie. dxt surfaces) as we assume app doesn't read from them
  useDynamicAllocation = Format>200; // FOURCC is always larger than 200 (I hope)
  //useDynamicAllocation = false;
  //useDynamicAllocationNew = Format>200;
  dbgf("Using %s for %s", useDynamicAllocationNew||useDynamicAllocation?"dynamic allocation":"static allocation", getMode(Format));

  if(!useDynamicAllocation && !useDynamicAllocationNew) {
    // Create system texture now and keep it allocated at all times so app can read from it
    ret = createSysTexture();
    if(ret != D3D_OK) {
      fail = true;
      retVal = ret;
    }
    tex = texMem;
  } else {
    // System texture is created when needed (on lock or getsurfacelevel)
    tex = texVid;
  }
}

void IDirect3DTexture9Managed::ReleaseSysmemSurface()
{
  // Release dynamically allocated sysmem surface - this is called from device SetTexture
  if(!texMem || !useDynamicAllocationNew)
    return;

  dbgf("IDirect3DTexture9Managed: Late-releasing system texture (%d refs)", getRef(texMem));
  if(texMem->Release() == 0)
    texMem = NULL;
}

HRESULT IDirect3DTexture9Managed::createSysTexture()
{
  dbgf("IDirect3DTexture9Managed: Creating system texture (%dx%d %d levels) %s, %s", texWidth, texHeight, LevelsSys, getMode(texFormat), getUsage(UsageSys));
  if(texMem)
    dbg("IDirect3DTexture9Managed::createSysTexture: WARNING: Texture already allocated!");
  HRESULT ret = devReal->CreateTexture(texWidth, texHeight, LevelsSys, UsageSys, texFormat, D3DPOOL_SYSTEMMEM, &texMem, NULL);
  if(ret != D3D_OK) {
    dbg("IDirect3DTexture9Managed: CreateTexture D3DPOOL_SYSTEMMEM failed: %s (%dx%d %d levels) %s, %s", getD3DError(ret), texWidth, texHeight, LevelsSys, getMode(texFormat), getUsage(UsageSys));
    fail = true;
    retVal = ret;
  } else {
    if(useDynamicAllocationNew) {
      texMem->AddRef(); // Keep texture active
    }
    /*if(texFormat > 200)
      return ret;
    D3DLOCKED_RECT lr;
    D3DCALL( texMem->LockRect(0, &lr, NULL, NULL) );
    memset(lr.pBits, 128, lr.Pitch*texHeight);
    texMem->UnlockRect(0);*/
  }

  return ret;
}

// {13D8FDE2-5998-46ef-A42D-6D829A4BDCF7}
static const GUID ID_surfID = { 0x13d8fde2, 0x5998, 0x46ef, { 0xa4, 0x2d, 0x6d, 0x82, 0x9a, 0x4b, 0xdc, 0xf7 } };


HRESULT IDirect3DTexture9Managed::GetSurfaceLevel(UINT Level,IDirect3DSurface9** ppSurfaceLevel) 
{
  dbgf("IDirect3DTexture9Managed:GetSurfaceLevel NEW %d Level %d", tex, Level);  
  HRESULT ret = D3D_OK;

  if(useDynamicAllocation || useDynamicAllocationNew) {
    // Create sysmem surface, it will be released by IDirect3DSurface9Managed when done or by SetTexture
    IDirect3DSurface9 *surfaceVid;
    HRESULT ret = texVid->GetSurfaceLevel(Level, &surfaceVid);
    if(ret == D3D_OK) {
      IDirect3DSurface9 *surfaceSys = NULL;
      if(!texMem)
        createSysTexture();
      if(!texMem) {
        dbg("IDirect3DTexture9Managed::GetSurfaceLevel: No system texture!");
        surfaceVid->Release();
        return D3DERR_INVALIDCALL;
      }
      texMem->GetSurfaceLevel(Level, &surfaceSys);

      IDirect3DSurface9Managed *mSurf = NULL;
      DWORD size = sizeof(DWORD);
      surfaceSys->GetPrivateData(ID_surfID, &mSurf, &size);
      if(!mSurf) {
        // Create new managed surface
        *ppSurfaceLevel = new IDirect3DSurface9Managed(devReal, surfaceSys, surfaceVid, useDynamicAllocationNew?NULL:&texMem);
        surfaceSys->SetPrivateData(ID_surfID, ppSurfaceLevel, size, NULL);
        dbgf("New Pointer: 0x%08X (dynamic)", *ppSurfaceLevel);
      } else {
        // Return previous managed surface
        dbgf("Previous Pointer: 0x%08X (dynamic)", mSurf);
        *ppSurfaceLevel = mSurf;
      }
    }
  } else {
    // Pass sysmem and vidmem surfaces to IDirect3DSurface9Managed
    IDirect3DSurface9 *surfaceSys = NULL;
    ret = tex->GetSurfaceLevel(Level, &surfaceSys);  // tex = texMem
    if(ret == D3D_OK) {
      // Getting surface level of manage-emulated texture: Must emulate surface as well
      IDirect3DSurface9 *surfaceVid;
      texVid->GetSurfaceLevel(Level, &surfaceVid);

      IDirect3DSurface9Managed *mSurf = NULL;
      DWORD size = sizeof(DWORD);
      surfaceSys->GetPrivateData(ID_surfID, &mSurf, &size);
      if(!mSurf) {
        // Create new managed surface
        *ppSurfaceLevel = new IDirect3DSurface9Managed(dev, surfaceSys, surfaceVid, NULL);   
        surfaceSys->SetPrivateData(ID_surfID, ppSurfaceLevel, size, NULL);
        dbgf("New Pointer: 0x%08X (static)", *ppSurfaceLevel);
      } else {
        // Return previous managed surface
        dbgf("Previous Pointer: 0x%08X (static)", mSurf);
        *ppSurfaceLevel = mSurf;
      }
    }
  }
  
  return ret;
}

IDirect3DTexture9Managed::~IDirect3DTexture9Managed()
{
  dbgf("~IDirect3DTexture9Managed");
  if(texMem) while(texMem->Release());
  if(texVid) while(texVid->Release());
  texMem = NULL;
  texVid = NULL;
}

HRESULT IDirect3DTexture9Managed::LockRect(UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags)
{
  if(Level > MAX_LEVELS)
    dbg("ERROR: IDirect3DTexture9Managed::LockRect: Level > MAX_LEVELS");

  if(useDynamicAllocation || useDynamicAllocationNew) {
    // Sysmem texture is needed - create it now
    if(!texMem) {
      D3DCALL( createSysTexture() );
    } else
      texMem->AddRef();
  }

  dbgf("IDirect3DTexture9Managed::LockRect %d %d %d %s", Level, pLockedRect, pRect, getLock(Flags));
  if(pRect)
    curLockRect[Level] = *pRect, curLockRectUsed[Level] = true;
  else
    curLockRectUsed[Level] = false;
  if((Flags & D3DLOCK_READONLY) || (Flags & D3DLOCK_NO_DIRTY_UPDATE))
    lockReadonly[Level] = true;
  else
    lockReadonly[Level] = false;
  return texMem->LockRect(Level, pLockedRect, pRect, Flags);
}

HRESULT IDirect3DTexture9Managed::UnlockRect(UINT Level)
{
  dbgf("IDirect3DTexture9Managed::UnlockRect %d", Level);
  HRESULT ret = texMem->UnlockRect(Level);
  //dbg("IDirect3DTexture9Managed: Uploading texture...");

  if(!lockReadonly[Level]) {
    int t = GetTickCount();
    if(Level == 0 && !useDynamicAllocation && !useDynamicAllocationNew)
      devReal->UpdateTexture(texMem, texVid);
    else {
      IDirect3DSurface9 *svid, *smem;
      D3DCALL( texVid->GetSurfaceLevel(Level, &svid) );
      D3DCALL( texMem->GetSurfaceLevel(Level, &smem) );
      POINT p = {curLockRect[Level].left, curLockRect[Level].top};
      D3DCALL( devReal->UpdateSurface(smem, curLockRectUsed[Level]?&curLockRect[Level]:NULL, svid, curLockRectUsed[Level]?&p:NULL) );
      svid->Release();
      smem->Release();
    }
    timeWarn(t, 1000, "UpdateTexture Managed");
  }

  if(useDynamicAllocation || useDynamicAllocationNew) {
    // Release dynamic texture - can be freed here completely if last ref
    if(texMem->Release() == 0)
      texMem = NULL;
  }
  return ret;
}

ULONG IDirect3DTexture9Managed::AddRef()
{
  dbgf("IDirect3DTexture9Managed: AddRef...");

  ULONG r;
  if(useDynamicAllocation || useDynamicAllocationNew)
    r = texVid?texVid->AddRef():0;
  else
    r = texMem?texMem->AddRef():0;
  dbgf("IDirect3DTexture9Managed: AddRef %d", r);
  return r;
}

ULONG IDirect3DTexture9Managed::Release()
{
  dbgf("IDirect3DTexture9Managed: Release... REF: mem%d vid%d", GetRef(texMem), GetRef(texVid));

  if(useDynamicAllocation || useDynamicAllocationNew) {
    if(!texVid)
      dbg("IDirect3DTexture9Managed: Release: WARNING: Attempt to release already free texture (dynamic)");
    ULONG r = texVid?texVid->Release():0;
    if(r == 0 && texMem) {
      dbgf("release texmem");
      /*SAFE_RELEASE(texMem);
      SAFE_RELEASE_LAST(texMem);*/
      while(texMem->Release());
      texMem = NULL;
    }
    if(r == 0) {
      dbgf("release vidmem");
      texVid = NULL;
      delete this;
    }
    return r;
  } else {
    if(!texMem)
      dbg("IDirect3DTexture9Managed: Release: WARNING: Attempt to release already free texture (static)");
    ULONG r = texMem?texMem->Release():0;
    if(r == 0) {
      SAFE_FREE(texVid);
      texMem = texVid = 0;
      delete this;
    }
    return r;
  }
};
#endif

  ///////////////////////////////
 // Managed texture surface   //
///////////////////////////////
IDirect3DSurface9Managed::IDirect3DSurface9Managed(IDirect3DDevice9Ex* device, IDirect3DSurface9 *surfaceMem, IDirect3DSurface9* surfaceVid, IDirect3DTexture9 **surfaceMemReference)
:IDirect3DSurface9New(device, surfaceMem, NULL, 0)
{
  ONCE dbg("Using Surface manage-emulation");
  dbgf("IDirect3DSurface9Managed NEW");
  surfVid = surfaceVid;
  surfMem = surfaceMem;
  dev = device;
  surf = surfaceMem;
  surfMemRef = surfaceMemReference;
  fullLock = true;
  curLockedRect.left = curLockedRect.right = curLockedRect.bottom = curLockedRect.top = 0;
/*
  freeRefs = getRef(surf)-1;
  dbgf("Manage surface free refs: %d", freeRefs);
  */
}

ULONG IDirect3DSurface9Managed::AddRef() {
  dbgf("IDirect3DSurface9Managed: AddRef... REF: mem%d vid%d", GetRef(surfMem), GetRef(surfVid));
  ULONG r = surf->AddRef();
  ULONG rr = surfVid->AddRef();
  return rr;
}

ULONG IDirect3DSurface9Managed::Release() {
  dbgf("IDirect3DSurface9Managed: Release... REF: mem%d vid%d", GetRef(surfMem), GetRef(surfVid));
  ULONG r = surf->Release();
  ULONG rr = surfVid->Release();
  if(surfMemRef) {
    // Owner keeps one handle - if refs is one then release owner as well
    if(r == 1) {
      dbgf("RELEASING OWNER TEXTURE");
      if((*surfMemRef)->Release() == 0)
        (*surfMemRef) = NULL;
      delete this;
    }
  } else {
    // No additional references. if its free its free
    if(r == 0) {
      dbgf("RELEASING managed surface");
      delete this;
    }
  }
  return rr;
};

IDirect3DSurface9Managed::~IDirect3DSurface9Managed()
{
  dbgf("~IDirect3DSurface9Managed");
}

// Retrieve data from default memory pool surface
static void getDefaultPoolSurfaceData(IDirect3DDevice9Ex *dev, IDirect3DSurface9 *dst, IDirect3DSurface9 *src)
{
  /*
  IDirect3DTexture9 *txt = NULL;
  dst->GetContainer(IID_IDirect3DTexture9, (void**)&txt);
  if(!txt)
    dbg("FU"), exit(0);

  D3DSURFACE_DESC d;
  txt->GetLevelDesc(0, &d);
  int levels = txt->GetLevelCount();
  dev->CreateTexture(d.Width, d.Height, levels, 

  D3DCALL( dev->StretchRect(src, NULL, dst, NULL, D3DTEXF_NONE) );


  //dev->Create
  D3DLOCKED_RECT lr;
  D3DCALL( src->LockRect(&lr, NULL, D3DLOCK_READONLY) );
  dbg("semmosta");
  exit(0);
  */
}

HRESULT IDirect3DSurface9Managed::LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
  if(pRect)
    dbgf("IDirect3DSurface9Managed: LockRect %s, %s (rect: %dx%d - %dx%d)", pRect?"NOT NULL":"NULL", getLock(Flags), pRect->left, pRect->top, pRect->right, pRect->bottom);
  else
    dbgf("IDirect3DSurface9Managed: LockRect %s, %s", pRect?"NOT NULL":"NULL", getLock(Flags));

  if(pRect) {
    dbgf("IDirect3DSurface9Managed 0x%08X: Partial LockRect real (rect: %dx%d - %dx%d) %s", this, pRect->left, pRect->top, pRect->right, pRect->bottom, getLock(Flags));
    curLockedRect = *pRect;
    fullLock = false;
  } else {
    fullLock = true;
  }  

  /*if(Flags & D3DLOCK_READONLY)
    getDefaultPoolSurfaceData(dev, surfMem, surfVid);*/

  if((Flags & D3DLOCK_READONLY) || (Flags & D3DLOCK_NO_DIRTY_UPDATE))
    lockReadonly = true;
  else
    lockReadonly = false;
  HRESULT ret = surf->LockRect(pLockedRect, pRect, Flags);
  if(ret != D3D_OK)
    lockReadonly = true;
  else
    surf->AddRef(); // Prevent surface or owner texture from being released while locked
  return ret;
}

HRESULT IDirect3DSurface9Managed::UnlockRect()
{
  dbgf("IDirect3DSurface9Managed 0x%08X: UnlockRect %s (rect: %dx%d - %dx%d)", this, fullLock?"FULL":"PARTIAL", curLockedRect.left, curLockedRect.top, curLockedRect.right, curLockedRect.bottom);
  HRESULT ret = surf->UnlockRect();

  if(ret == D3D_OK && !lockReadonly) {
    int t = GetTickCount();
    if(fullLock) {
      D3DCALL( dev->UpdateSurface(surfMem, NULL, surfVid, NULL) );
    } else {
      POINT p = {curLockedRect.left, curLockedRect.top};
      HRESULT r = dev->UpdateSurface(surfMem, &curLockedRect, surfVid, &p);
      if(r != D3D_OK) {
        dbg("IDirect3DSurface9Managed 0x%08X::UnlockRect: ERROR: UpdateSurface failed: %s (rect: %dx%d - %dx%d)", this, getD3DError(r), curLockedRect.left, curLockedRect.top, curLockedRect.right, curLockedRect.bottom);
      }
    }
    timeWarn(t, 1000, "UpdateSurface Managed");
    surf->Release();
  }
  return ret;
}

  //////////////////////////////////////
 // Managed volume texture surface   //
//////////////////////////////////////
IDirect3DVolumeTexture9Managed::IDirect3DVolumeTexture9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9Ex* deviceReal, UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppTexture,HANDLE* pSharedHandle)
{
  dev = device;
  devReal = deviceReal;
  tex = texVid = NULL;

  setClean(ALL_LEVELS);

  // Create system memory
  D3DCALL( devReal->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, D3DPOOL_SYSTEMMEM, &tex, pSharedHandle) );
  dbgf("IDirect3DVolumeTexture9Managed: Created sysmem texture 0x%08X (0x%08X)", tex, *tex);

  // Create video memory texture
  D3DCALL( devReal->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, D3DPOOL_DEFAULT, &texVid, pSharedHandle) );

  if(texVid)
  {
    LevelsReal = texVid->GetLevelCount();

    IDirect3DVolumeTexture9Managed *thissi = this;
    texVid->SetPrivateData(IID_IDirect3DVolumeTexture9Managed, &thissi, sizeof(this), NULL);
    
  }
  else
  {
    dbg("ERROR: IDirect3DVolumeTexture9Managed: CreateVolumeTexture failed!");
  }

  width = Width;
  height = Height;
  depth = Depth;
}

IDirect3DVolumeTexture9* IDirect3DVolumeTexture9Managed::GetRealTexture()
{
  // Check for dirty levels, if any, update texture
  // This is problematic: Should only update dirty levels, and from those boxes that are dirty...
  // But it is not possible with D3D9, UpdateSurface does not handle volume levels

  bool needUpdate = false;
  for(DWORD i=0;i<LevelsReal;i++) {
    if(dirty[i]) needUpdate = true;
  }

  if(needUpdate)
  {
    dbgf("DEBUG: Update Volume texture 0x%08X", this);
    updateLevel(0); // Actually updates whole texture
    setClean(ALL_LEVELS);
  }
  return texVid;
};

void IDirect3DVolumeTexture9Managed::updateLevel(UINT Level)
{  
  /*
  IDirect3DVolume9 *src, *dst;

  D3DCALL( tex->GetVolumeLevel(Level, &src) );
  D3DCALL( texVid->GetVolumeLevel(Level, &dst) );

  D3DLOCKED_BOX dstl, srcl;
  D3DCALL( dst->LockBox(&dstl, NULL, D3DLOCK_DISCARD) );
  D3DCALL( src->LockBox(&srcl, NULL, D3DLOCK_READONLY) );

  //DWORD size = srcl.RowPitch*srcl.SlicePitch*height;
  DWORD size = srcl.SlicePitch*depth;
  dbg("Copy %d bytes (%dx%dx%d)", size, width, height, depth);
  memcpy(dstl.pBits, srcl.pBits, size);

  src->UnlockBox();
  dst->UnlockBox();

  src->Release();
  dst->Release();
  */
 
  // TODO: only update the requested level
  devReal->UpdateTexture(tex, texVid);  
}

HRESULT IDirect3DVolumeTexture9Managed::UnlockBox(UINT Level)
{
  dbgf("IDirect3DVolumeTexture9Managed: UnlockBox %d", Level);
  HRESULT ret = tex->UnlockBox(Level);
  if(ret == D3D_OK) {
    setDirty(Level);
    /*
    dbgf("IDirect3DVolumeTexture9Managed update");
    updateLevel(Level);
    //devReal->UpdateTexture(tex, texVid);
    dbgf("IDirect3DVolumeTexture9Managed end");
    */
  }
  return ret;
}
