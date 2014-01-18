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

#ifndef __D3D_BUFFER_H__
#define __D3D_BUFFER_H__

#include "d3d.h"
#include "helper.h"

#include <vector>

#undef nil
#undef dbgf

#define nil if(0)
#define dbgf if(0)
//#define dbgf dbg
//#define nil dbg

typedef struct {
  DWORD lockOffset;
  DWORD lockSize;
} LOCKSECTION;
#define LATE_IB_UPDATE    // Dont immediately update indexbuffers
//#define ALWAYS_FULL_UPDATE // Ignore locked sections, always do full discard upate
#define USE_DISCARD_FLAG  // Use discard flag for IB&VB, although D3D spec says its not allowed
//#define RECREATE_ON_REUSE // Re-create vb/ib if locked multiple times per frame


#define BUF_EXTRA_BYTES 32

// IDirect3DIndexBuffer9Managed: Emulates managed index buffers
DEFINE_GUID(IID_IDirect3DIndexBuffer9Managed, 0x7c9dd65e, 0xd3f7, 0x4529, 0xac, 0xee, 0x78, 0x58, 0xaa, 0xbb, 0xcc, 0xdd);
interface IDirect3DIndexBuffer9Managed : public IDirect3DIndexBuffer9
{
  IDirect3DIndexBuffer9Managed(IDirect3DDevice9Ex* device, IDirect3DDevice9New* devFake, UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,HANDLE* pSharedHandle);
  ~IDirect3DIndexBuffer9Managed() {
    dbgf("IDirect3DIndexBuffer9New: ~IDirect3DIndexBuffer9New");
    buf = NULL;
    delete[] bufSys;
  };

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      nil("IDirect3DIndexBuffer9Managed: QueryInterface");
      if(riid == IID_IDirect3DIndexBuffer9Managed || riid == IID_IDirect3DIndexBuffer9) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return buf->QueryInterface(riid, ppvObj);
  };

  STDMETHOD_(ULONG,AddRef)(THIS) 
    {dbgf("IDirect3DIndexBuffer9Managed 0x%08X: AddRef", this);return buf->AddRef();};
  STDMETHOD_(ULONG,Release)(THIS) 
    {dbgf("IDirect3DIndexBuffer9Managed 0x%08X: Release", this);
    ULONG r = buf->Release();
    if(r == 0)
      delete this;
    return r;
    };

  STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) 
    {dbg("WARNING: ibuffer: GetDevice, Not implemented!");return buf->GetDevice(ppDevice);};
  STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) 
    {dbgf("ibuffer: SetPrivateData");return buf->SetPrivateData(refguid, pData, SizeOfData, Flags);};
  STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData) 
    {dbgf("ibuffer: GetPrivateData");return buf->GetPrivateData(refguid, pData, pSizeOfData);};
  STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) 
    {dbgf("ibuffer: FreePrivateData");return buf->FreePrivateData(refguid);};
  STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) 
    {dbgf("ibuffer: SetPriority");return buf->SetPriority(PriorityNew);};
  STDMETHOD_(DWORD, GetPriority)(THIS) 
    {dbgf("ibuffer: GetPriority");return buf->GetPriority();};
  STDMETHOD_(void, PreLoad)(THIS) 
    {dbgf("ibuffer: PreLoad");return buf->PreLoad();};
  STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) 
    {dbgf("ibuffer: GetType");return buf->GetType();};
  STDMETHOD(Lock)(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags) 
    ;//{dbgf("ibuffer: Lock");return buf->Lock(OffsetToLock, SizeToLock, ppbData, Flags);};
  STDMETHOD(Unlock)(THIS) 
    ;//{dbgf("ibuffer: Unlock");return buf->Unlock();};
  STDMETHOD(GetDesc)(THIS_ D3DINDEXBUFFER_DESC *pDesc)
    {dbgf("ibuffer: GetDesc");HRESULT ret = buf->GetDesc(pDesc);pDesc->Pool= D3DPOOL_MANAGED;return ret;};

#ifdef LATE_IB_UPDATE
  IDirect3DIndexBuffer9* GetRealBuffer();
#else
  IDirect3DIndexBuffer9* GetRealBuffer() {return buf;};
#endif

private:
  IDirect3DDevice9Ex *dev;
  IDirect3DDevice9New* devFake;
  IDirect3DIndexBuffer9 *buf; // Video memory buffer
  BYTE *bufSys; // System memory buffer
  DWORD bufSize;

  bool ReCreate();
  DWORD lastRecreate;
  UINT Length;
  DWORD Usage;
  D3DFORMAT Format;

  std::vector<LOCKSECTION> lockSections;
#ifdef LATE_IB_UPDATE
  bool fullDirty;
#endif
};

//#undef dbgf
//#define dbgf dbg

// IDirect3DVertexBuffer9Managed: VB manage-emulation
DEFINE_GUID(IID_IDirect3DVertexBuffer9Managed, 0x7c9dd65e, 0xd3f7, 0x4529, 0xac, 0xfe, 0x12, 0x34, 0xaa, 0xbb, 0xcc, 0xdd);
interface IDirect3DVertexBuffer9Managed : public IDirect3DVertexBuffer9
{
  IDirect3DVertexBuffer9Managed(IDirect3DDevice9Ex* devNew, IDirect3DDevice9New* devFake, UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, HANDLE* pSharedHandle);
  ~IDirect3DVertexBuffer9Managed() {
    dbgf("IDirect3DVertexBuffer9Managed: ~IDirect3DVertexBuffer9Managed");
    buf = NULL;
    delete[] bufSys;
  };

  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
    nil("IDirect3DVertexBuffer9Managed: QueryInterface");
    if(riid == IID_IDirect3DVertexBuffer9Managed || riid == IID_IDirect3DVertexBuffer9) {
      this->AddRef();
      *ppvObj = this;
      return S_OK;
    } else
      return buf->QueryInterface(riid, ppvObj);
  };

  STDMETHOD_(ULONG,AddRef)(THIS) 
    {dbgf("IDirect3DVertexBuffer9Managed 0x%08X: AddRef", this);return buf->AddRef();};
  STDMETHOD_(ULONG,Release)(THIS) {
    dbgf("IDirect3DVertexBuffer9Managed 0x%08X: Release", this);
    ULONG r = buf->Release(); // TODO: check
    if(r == 0)
    {
      delete this;
    }
    return r;
  };

  STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice)
    {dbg("WARNING: vebuffer: GetDevice, Not implemented!");return buf->GetDevice(ppDevice);};
  STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags)
    {dbgf("vebuffer: SetPrivateData");return buf->SetPrivateData(refguid, pData, SizeOfData, Flags);};
  STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData)
    {dbgf("vebuffer: GetPrivateData");return buf->GetPrivateData(refguid, pData, pSizeOfData);};
  STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid)
    {dbgf("vebuffer: FreePrivateData");return buf->FreePrivateData(refguid);};
  STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew)
    {dbgf("vebuffer: SetPriority");return buf->SetPriority(PriorityNew);}; // TODO: skip?
  STDMETHOD_(DWORD, GetPriority)(THIS)
    {dbgf("vebuffer: GetPriority");return buf->GetPriority();}; // TODO: skip?
  STDMETHOD_(void, PreLoad)(THIS)
    {dbgf("vebuffer: PreLoad");return buf->PreLoad();}; // TODO: skip?
  STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS)
    {dbgf("vebuffer: GetType");return buf->GetType();};
  STDMETHOD(Lock)(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags);
  STDMETHOD(Unlock)(THIS);
  STDMETHOD(GetDesc)(THIS_ D3DVERTEXBUFFER_DESC *pDesc)
    {
      dbgf("vebuffer: GetDesc");
      HRESULT ret = buf->GetDesc(pDesc);
      if(ret == D3D_OK) pDesc->Pool = D3DPOOL_MANAGED;
      return ret;
    };
  
  //IDirect3DVertexBuffer9* GetRealBuffer() {return buf;};
  IDirect3DVertexBuffer9* GetRealBuffer();
  HRESULT GetResult() {return result;};
private:
  IDirect3DDevice9Ex* dev;
   IDirect3DDevice9New* devFake;
  /*
  IDirect3DVertexBuffer9 *bufSys; // System memory buffer
  IDirect3DVertexBuffer9 *bufVid; // Video memory buffer
  HRESULT result;
  */

  IDirect3DVertexBuffer9 *buf; // Video memory buffer
  BYTE *bufSys; // System memory buffer
  DWORD bufSize;
  HRESULT result;

  bool ReCreate();
  DWORD lastRecreate;
  UINT Length;
  DWORD Usage;
  DWORD FVF;

  std::vector<LOCKSECTION> lockSections;
  bool fullDirty;
  int locks;
};

// IDirect3DVertexBuffer9Quirk: Quirk mode for buggy software (DCS A-10C beta)
DEFINE_GUID(IID_IDirect3DVertexBuffer9Quirk, 0x7c9dd65e, 0xd3f7, 0x4529, 0xac, 0xfe, 0x12, 0x34, 0xaa, 0xbb, 0xcc, 0xdd);
interface IDirect3DVertexBuffer9Quirk : public IDirect3DVertexBuffer9
{
  IDirect3DVertexBuffer9Quirk(IDirect3DVertexBuffer9* bufnew, DWORD lengthnew) {buf = bufnew;length = lengthnew;};

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      nil("IDirect3DVertexBuffer9Quirk: QueryInterface");
      if(riid == IID_IDirect3DVertexBuffer9Quirk) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return buf->QueryInterface(riid, ppvObj);
  };

  STDMETHOD_(ULONG,AddRef)(THIS) 
    {dbgf("IDirect3DVertexBuffer9Quirk 0x%08X: AddRef", this);return buf->AddRef();};
  STDMETHOD_(ULONG,Release)(THIS) 
    {dbgf("IDirect3DVertexBuffer9Quirk 0x%08X: Release", this);
    ULONG r = buf->Release();
    if(r == 0)
      delete this;
    return r;
    };

  STDMETHOD(GetDevice)(THIS_ IDirect3DDevice9** ppDevice) 
    {dbg("WARNING: vbuffer: GetDevice, Not implemented!");return buf->GetDevice(ppDevice);};
  STDMETHOD(SetPrivateData)(THIS_ REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) 
    {dbgf("vbuffer: SetPrivateData");return buf->SetPrivateData(refguid, pData, SizeOfData, Flags);};
  STDMETHOD(GetPrivateData)(THIS_ REFGUID refguid,void* pData,DWORD* pSizeOfData) 
    {dbgf("vbuffer: GetPrivateData");return buf->GetPrivateData(refguid, pData, pSizeOfData);};
  STDMETHOD(FreePrivateData)(THIS_ REFGUID refguid) 
    {dbgf("vbuffer: FreePrivateData");return buf->FreePrivateData(refguid);};
  STDMETHOD_(DWORD, SetPriority)(THIS_ DWORD PriorityNew) 
    {dbgf("vbuffer: SetPriority");return buf->SetPriority(PriorityNew);};
  STDMETHOD_(DWORD, GetPriority)(THIS) 
    {dbgf("vbuffer: GetPriority");return buf->GetPriority();};
  STDMETHOD_(void, PreLoad)(THIS) 
    {dbgf("vbuffer: PreLoad");return buf->PreLoad();};
  STDMETHOD_(D3DRESOURCETYPE, GetType)(THIS) 
    {dbgf("vbuffer: GetType");return buf->GetType();};
  STDMETHOD(Unlock)(THIS) 
    {dbgf("vbuffer: Unlock");return buf->Unlock();};
  STDMETHOD(GetDesc)(THIS_ D3DVERTEXBUFFER_DESC *pDesc)
    {dbgf("vbuffer: GetDesc");HRESULT ret = buf->GetDesc(pDesc);pDesc->Pool= D3DPOOL_MANAGED;return ret;};

  STDMETHOD(Lock)(THIS_ UINT OffsetToLock,UINT SizeToLock,void** ppbData,DWORD Flags) 
  {
    dbgf("IDirect3DVertexBuffer9Quirk: Lock %d -> %d %s", OffsetToLock, SizeToLock, getLock(Flags));
    /*Flags |= D3DLOCK_NOOVERWRITE;*/
    HRESULT ret = buf->Lock(OffsetToLock, SizeToLock, ppbData, Flags);
    if(ret != D3D_OK) {
      dbgf("IDirect3DVertexBuffer9Quirk: Lock failed!");
      if(OffsetToLock+SizeToLock > length) {
        // Bad code: Trying to lock outside allocated size
        ret = buf->Lock(OffsetToLock, length-OffsetToLock, ppbData, Flags);
        if(ret == D3D_OK)
          dbg("QUIRK MODE: Program attempted to lock vertex buffer outside allocated size, clamped");
      }
    }
    return ret;
  };

  IDirect3DVertexBuffer9* GetRealBuffer() {return buf;};
private:
  IDirect3DVertexBuffer9 *buf;
  DWORD length; 
};


#endif