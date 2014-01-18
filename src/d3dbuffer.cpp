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

#include "d3dbuffer.h"

#include "main.h"

// These GUIDs are used to detect new interfaces to override back to originals to pass to D3D
#include <INITGUID.H>
DEFINE_GUID(IID_IDirect3DIndexBuffer9Managed, 0x7c9dd65e, 0xd3f7, 0x4529, 0xac, 0xee, 0x78, 0x58, 0xaa, 0xbb, 0xcc, 0xdd);
DEFINE_GUID(IID_IDirect3DVertexBuffer9Quirk, 0x7c9dd65e, 0xd3f7, 0x4529, 0xac, 0xfe, 0x12, 0x34, 0xaa, 0xbb, 0xcc, 0xdd);
DEFINE_GUID(IID_IDirect3DVertexBuffer9Managed, 0x7c9dd65e, 0xd3f7, 0x4529, 0xac, 0x00, 0xff, 0x23, 0xaa, 0xbb, 0xcc, 0xdd);

#ifdef USE_DISCARD_FLAG
#define FLOCKFLAGS D3DLOCK_DISCARD
#else

#ifdef RECREATE_ON_REUSE
#define FLOCKFLAGS NULL
#else
#define FLOCKFLAGS D3DLOCK_NOOVERWRITE
#endif

#endif

static const int recreateThreshold = 3;

IDirect3DIndexBuffer9Managed::IDirect3DIndexBuffer9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9New* wantDevFake, UINT wantLength, DWORD wantUsage,D3DFORMAT wantFormat,D3DPOOL Pool,HANDLE* pSharedHandle)
{
  ONCE dbg("Using Index buffer manage-emulation");
  dbgf("IDirect3DIndexBuffer9Managed 0x%08X: %d bytes, %s %s %s share: 0x%08X", this, Length, getUsage(Usage), getMode(Format), getPool(Pool), pSharedHandle);
  bufSys = NULL;
  buf = NULL;
#ifdef LATE_IB_UPDATE
  fullDirty = false;
#endif
  lockSections.reserve(8); // Reserve a bit of space

  lastRecreate = 0;
  Length = wantLength;
  Usage = wantUsage;
  Format = wantFormat;
  devFake = wantDevFake;

  if(Pool != D3DPOOL_MANAGED) {
    dbg("IDirect3DIndexBuffer9Managed: Non-managed manage-emulation??");
    exit(0);
  }

  Pool = D3DPOOL_DEFAULT;
  if(Usage & D3DUSAGE_DYNAMIC)
    Usage -= D3DUSAGE_DYNAMIC;
  Usage |= D3DUSAGE_WRITEONLY;
  /*
  Usage |= D3DUSAGE_DYNAMIC;
  */

  dev = device;
  if(dev->CreateIndexBuffer(Length, Usage, Format, Pool, &buf, pSharedHandle) != D3D_OK) {
    dbg("IDirect3DIndexBuffer9Managed: CreateIndexBuffer failed!");
    return;
  }

  bufSize = Length;
  bufSys = new BYTE[bufSize+BUF_EXTRA_BYTES];
  ZeroMemory(bufSys, bufSize+BUF_EXTRA_BYTES);
}

bool IDirect3DIndexBuffer9Managed::ReCreate()
{
  bool didRecreate = false;
  if(!buf) return didRecreate;
  if(devFake->getFrameNumber() - lastRecreate < recreateThreshold)
  {
    dbgf("IDirect3DIndexBuffer9Managed 0x%08X - Recreate at frame %d", this, devFake->getFrameNumber());
    int r = buf->Release();
    if(r!=0)
    {
      dbg("IDirect3DIndexBuffer9Managed::ReCreate() Warning: %d refs on old buffer, forcing release", r);
      while(buf->Release()) {};
    }
    didRecreate = true;
    D3DCALL( dev->CreateIndexBuffer(Length, Usage, Format, D3DPOOL_DEFAULT, &buf, NULL) );
  }
  lastRecreate = devFake->getFrameNumber();
  return didRecreate;
}

HRESULT IDirect3DIndexBuffer9Managed::Lock(UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags) 
{
  dbgf("IDirect3DIndexBuffer9Managed 0x%08X: lock: off %d, %d bytes, %s", this, OffsetToLock, SizeToLock, getLock(Flags));

  if(OffsetToLock+SizeToLock > bufSize)
    dbg("WARNING: Application attempted to lock too large indexbuffer (%d > %d)", OffsetToLock+SizeToLock, bufSize);

  if(!bufSys || !buf)
    return D3DERR_INVALIDCALL;
  // Return handle to system memory buffer
  *ppbData = bufSys+OffsetToLock;

#ifdef RECREATE_ON_REUSE
  if(!fullDirty && lockSections.size() == 0)
  {
    // First lock
    if(ReCreate())
    {
      fullDirty = true;
    }
  }
#endif

#ifndef LATE_IB_UPDATE
  LOCKSECTION l;
  l.lockOffset = OffsetToLock;
  l.lockSize = SizeToLock;
  lockSections.push_back(l);
#else
  if((OffsetToLock == 0 && SizeToLock == 0) || (OffsetToLock == 0 && SizeToLock == bufSize))
  {
    // Whole IB locked
    fullDirty = true;
  }
  else
  {
    LOCKSECTION l; // Dirty sections
    l.lockOffset = OffsetToLock;
    l.lockSize = SizeToLock;
    lockSections.push_back(l);
  }
#endif

  return D3D_OK;
}

HRESULT IDirect3DIndexBuffer9Managed::Unlock() 
{
  dbgf("IDirect3DIndexBuffer9Managed 0x%08X: Unlock", this);
  if(!bufSys || !buf)
    return D3DERR_INVALIDCALL;

  /*
  DWORD lockOffset = lockSections[lockSections.size()-1].lockOffset;
  DWORD lockSize = lockSections[lockSections.size()-1].lockSize;

  // Copy sysmem to vidmem
  void *vb;
  if(buf->Lock(lockOffset, lockSize, &vb, D3DLOCK_NOOVERWRITE|D3DLOCK_DONOTWAIT) != D3D_OK) {
    dbg("IDirect3DVertexBuffer9Managed: Unlock: FAILED!");
    return D3D_OK;
  }
  memcpy(vb, bufSys+lockOffset, lockSize?lockSize:bufSize);
  buf->Unlock();
  lockSections.pop_back();
  */


#ifndef LATE_IB_UPDATE
  /*DWORD flags = D3DLOCK_DISCARD; // Crashes A10-C
  if(config.main.debugD3D)
  {
   flags = NULL; // Debug D3D doesn't like discard here
  }
  */

  DWORD flags = NULL;

  if(lockSections.size() == 0)
  {
    dbg("WARNING: Indexbuffer unlock without lock??");
    return D3DERR_INVALIDCALL;
  }

  DWORD lockOffset = lockSections[lockSections.size()-1].lockOffset;
  DWORD lockSize = lockSections[lockSections.size()-1].lockSize;

  // Copy locked buffer to video memory
  void *ib;
  if(buf->Lock(lockOffset, lockSize, &ib, flags) != D3D_OK) {
    dbg("IDirect3DIndexBuffer9Managed: Unlock: Lock failed!");
    return D3DERR_INVALIDCALL;
  }
  memcpy(ib, bufSys+lockOffset, lockSize?lockSize:bufSize);
  buf->Unlock();

  lockSections.pop_back();
#endif

  return D3D_OK;
}

#ifdef LATE_IB_UPDATE
IDirect3DIndexBuffer9* IDirect3DIndexBuffer9Managed::GetRealBuffer()
{
  dbgf("IDirect3DIndexBuffer9Managed 0x%08X: GetRealBuffer", this);

  if(lockSections.size() || fullDirty)
  {
#ifdef ALWAYS_FULL_UPDATE
    fullDirty = true;
#endif

    //dbg("IDirect3DIndexBuffer9Managed 0x%08X: UPDATE");
    if(!fullDirty)
    {
      // Partial lock
      for(int i=0;i<lockSections.size();i++)
      {
        DWORD lockOffset = lockSections[i].lockOffset;
        DWORD lockSize = lockSections[i].lockSize;

#ifdef RECREATE_ON_REUSE
        const DWORD flags = NULL;
        //const DWORD flags = D3DLOCK_NOOVERWRITE;
#else
        // Improves performance - but isn't safe and not allowed by D3D spec
        const DWORD flags = D3DLOCK_NOOVERWRITE;
#endif

        // Copy sysmem to vidmem
        void *vb;
        if(buf->Lock(lockOffset, lockSize, &vb, flags) != D3D_OK) {
          dbg("IDirect3DIndexBuffer9Managed: Unlock: Lock failed!");
          return buf;
        }
        memcpy(vb, bufSys+lockOffset, lockSize?lockSize:bufSize);
        buf->Unlock();
      }
    }
    else
    {
      // Full buffer lock
      // Copy sysmem to vidmem
      void *vb;
      if(buf->Lock(0, 0, &vb, FLOCKFLAGS) != D3D_OK) {
        dbg("IDirect3DVertexBuffer9Managed: Unlock: Lock failed!");
        return buf;
      }
      memcpy(vb, bufSys, bufSize);
      buf->Unlock();      
    }

    lockSections.clear();
    fullDirty = false;
  }

  return buf;
}
#endif

// Managed VB emulation

IDirect3DVertexBuffer9Managed::IDirect3DVertexBuffer9Managed(IDirect3DDevice9Ex* wantDev, IDirect3DDevice9New* wantDevFake, UINT wantLength, DWORD wantUsage, DWORD wantFVF, D3DPOOL Pool, HANDLE* pSharedHandle)
{
  ONCE dbg("Using Vertex buffer manage-emulation");
  dev = wantDev;
  devFake = wantDevFake;
  bufSys = NULL;
  buf = NULL;
  HRESULT ret;
  fullDirty = false;
  locks = 0;

  Length = wantLength;
  Usage = wantUsage;
  FVF = wantFVF;

  lockSections.reserve(8); // Reserve a bit of space
  
  if(Pool != D3DPOOL_MANAGED) {
    dbg("IDirect3DIndexBuffer9Managed: Non-managed manage-emulation??");
    exit(0);
  }

  Pool = D3DPOOL_DEFAULT;
  if(Usage & D3DUSAGE_DYNAMIC)
    Usage -= D3DUSAGE_DYNAMIC;
  Usage |= D3DUSAGE_WRITEONLY;

  ret = dev->CreateVertexBuffer(Length, Usage, FVF, Pool, &buf, pSharedHandle);
  if(ret != D3D_OK)
  {
    result = ret;
    dbg("WARNING: IDirect3DVertexBuffer9Managed: CreateVertexBuffer D3DPOOL_DEFAULT failed!");
    return;
  }

  bufSize = Length;
  bufSys = new BYTE[bufSize+BUF_EXTRA_BYTES];
  ZeroMemory(bufSys, bufSize+BUF_EXTRA_BYTES);

  result = D3D_OK;
}

bool IDirect3DVertexBuffer9Managed::ReCreate()
{
  bool didRecreate = false;
  if(!buf) return didRecreate;
  if(devFake->getFrameNumber() - lastRecreate < recreateThreshold)
  {
    dbg("IDirect3DVertexBuffer9Managed 0x%08X - Recreate at frame %d (last recreate %d)", this, devFake->getFrameNumber(), lastRecreate);
    int r = buf->Release();
    if(r!=0)
    {
      dbg("IDirect3DVertexBuffer9Managed::ReCreate() Warning: %d refs on old buffer, forcing release", r);
      while(buf->Release()) {};
    }
    didRecreate = true;
    D3DCALL( dev->CreateVertexBuffer(Length, Usage, FVF, D3DPOOL_DEFAULT, &buf, NULL) );
  }
  else
    dbg("IDirect3DVertexBuffer9Managed 0x%08X - NO recreate at frame %d (last recreate %d)", this, devFake->getFrameNumber(), lastRecreate);
  lastRecreate = devFake->getFrameNumber();
  return didRecreate;
}

HRESULT IDirect3DVertexBuffer9Managed::Lock(UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags)
{
  dbgf("IDirect3DVertexBuffer9Managed 0x%08X: lock: off %d, %d bytes, %s", this, OffsetToLock, SizeToLock, getLock(Flags));

  if(OffsetToLock+SizeToLock > bufSize)
    dbg("WARNING: Application attempted to lock too large vertexbuffer (%d > %d)", OffsetToLock+SizeToLock, bufSize);

  if(!bufSys || !buf)
    return D3DERR_INVALIDCALL;
  // Return handle to system memory buffer
  *ppbData = bufSys+OffsetToLock;

#ifdef RECREATE_ON_REUSE
  if(!fullDirty && lockSections.size() == 0)
  {
    // First lock
    if(ReCreate())
    {
      fullDirty = true;
    }
    //fullDirty = true;
  }
#endif

  if((OffsetToLock == 0 && SizeToLock == 0) || (OffsetToLock == 0 && SizeToLock == bufSize))
  {
    // Whole VB locked
    fullDirty = true;
  }
  else
  {
    LOCKSECTION l; // Dirty sections
    l.lockOffset = OffsetToLock;
    l.lockSize = SizeToLock;
    lockSections.push_back(l);
  }

  locks++;
  return D3D_OK;
}

HRESULT IDirect3DVertexBuffer9Managed::Unlock()
{
  dbgf("IDirect3DVertexBuffer9Managed 0x%08X: Unlock", this);
  if(!bufSys || !buf)
    return D3DERR_INVALIDCALL;

  locks--;

  /*
  DWORD lockOffset = lockSections[lockSections.size()-1].lockOffset;
  DWORD lockSize = lockSections[lockSections.size()-1].lockSize;

  // Copy sysmem to vidmem
  void *vb;
  if(buf->Lock(lockOffset, lockSize, &vb, D3DLOCK_NOOVERWRITE|D3DLOCK_DONOTWAIT) != D3D_OK) {
    dbg("IDirect3DVertexBuffer9Managed: Unlock: FAILED!");
    return D3D_OK;
  }
  memcpy(vb, bufSys+lockOffset, lockSize?lockSize:bufSize);
  buf->Unlock();
  lockSections.pop_back();
  */

/*
  DWORD flags = D3DLOCK_NOOVERWRITE;

  if(lockSections.size() == 0)
  {
    dbg("WARNING: Vertexbuffer unlock without lock??");
    return D3DERR_INVALIDCALL;
  }

  DWORD lockOffset = lockSections[lockSections.size()-1].lockOffset;
  DWORD lockSize = lockSections[lockSections.size()-1].lockSize;

  // Copy locked buffer to video memory
  void *ib;
  if(buf->Lock(lockOffset, lockSize, &ib, flags) != D3D_OK) {
    dbg("IDirect3DVertexBuffer9Managed: Unlock: Lock failed!");
    return D3DERR_INVALIDCALL;
  }
  memcpy(ib, bufSys+lockOffset, lockSize?lockSize:bufSize);
  buf->Unlock();

  lockSections.pop_back();
*/

  return D3D_OK;
}

IDirect3DVertexBuffer9* IDirect3DVertexBuffer9Managed::GetRealBuffer()
{
  dbgf("IDirect3DVertexBuffer9Managed 0x%08X: GetRealBuffer", this);

  if(locks)
    dbg("WARNING: IDirect3DVertexBuffer9Managed 0x%08X: GetRealBuffer() with %d active locks", locks);

  if(lockSections.size() || fullDirty)
  {
#ifdef ALWAYS_FULL_UPDATE
    fullDirty = true;
#endif

    //dbg("IDirect3DVertexBuffer9Managed 0x%08X: UPDATE");
    if(!fullDirty)
    {
      // Partial buffer lock
      for(int i=0;i<lockSections.size();i++)
      {
        DWORD lockOffset = lockSections[i].lockOffset;
        DWORD lockSize = lockSections[i].lockSize;

#ifdef RECREATE_ON_REUSE
        const DWORD flags = NULL;
#else
        // Improves performance - but isn't safe and not allowed by D3D spec
        const DWORD flags = D3DLOCK_NOOVERWRITE;
#endif

        // Copy sysmem to vidmem
        void *vb;
        if(buf->Lock(lockOffset, lockSize, &vb, flags) != D3D_OK) {
          dbg("IDirect3DVertexBuffer9Managed: Unlock: Lock failed!");
          return buf;
        }
        memcpy(vb, bufSys+lockOffset, lockSize?lockSize:bufSize);
        buf->Unlock();
      }
    }
    else
    {
      // Full buffer lock
      // Copy sysmem to vidmem
      void *vb;
      if(buf->Lock(0, 0, &vb, FLOCKFLAGS/*|D3DLOCK_NOOVERWRITE*/) != D3D_OK) {
        dbg("IDirect3DVertexBuffer9Managed: Unlock: Lock failed!");
        return buf;
      }
      memcpy(vb, bufSys, bufSize);
      buf->Unlock();      
    }

    lockSections.clear();
    fullDirty = false;    
  }

  return buf;
}