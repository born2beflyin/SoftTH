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

#ifndef _HELPER_H_
#define _HELPER_H_

#include <windows.h>
#include <stdio.h>
#include <d3d9.h>
#include <dxgi.h>

#include "common.h"

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#define MAKERECT(re,l,r,t,b)	re.left=l;re.right=r;re.top=t;re.bottom=b;

// Added by CJR for SDK 8.1 - 9 Aug 2015
// Global helpers
bool isSoftTHmode(int w, int h);    // Return true if resolution is SoftTH mode
volatile extern int SoftTHActive;   // >0 if SoftTH is currently active and resolution is overridden
extern bool *SoftTHActiveSquashed;  // Pointer to latest SoftTH device squash variable (TODO: this is horrible)

// Common interfaces for DXGI objects
#define DECALE_DXGICOMMONIF(type)\
  /*STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)*/\
    /*{dbg("%s: QueryInterface", #type);return type->QueryInterface(riid, ppvObj);}*/\
  STDMETHOD_(ULONG,AddRef)(THIS)\
    {dbgf("%s: AddRef", #type);return type->AddRef();}\
  STDMETHOD_(ULONG,Release)(THIS) {\
    dbgf("%s: Release...", #type);\
    ULONG r = type->Release();\
    if(r == 0)\
      dbg("%s destroyed", #type), delete(this);\
    dbg("%s 0x%08X: Release: %d", #type, this, r);\
    return r;\
  }\
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID Name, UINT DataSize, const void *pData)\
    {dbg("%s: SetPrivateData %s", #type, matchRiid(Name));return type->SetPrivateData(Name, DataSize, pData);};\
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID Name, const IUnknown *pUnknown)\
    {dbg("%s: SetPrivateDataInterface", #type);return type->SetPrivateDataInterface(Name, pUnknown);};\
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID Name, UINT *pDataSize, void *pData)\
    {dbg("%s: SetPrivateDataInterface", #type);return type->GetPrivateData(Name, pDataSize, pData);};
  /*HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)\
    {dbg("%s: GetParent %s", #type, matchRiid(riid));return type->GetParent(riid, ppParent);};\*/

static int GetRef(IUnknown *p) {
  if(!p)
    return 0;
  p->AddRef();
  return p->Release();
}

//#define GETREFCOUNT(x) ((x->AddRef()) +! (x->Release()))
#define CHECKNULL(x)  if(!x) dbg("WARNING! %s == NULL at line %d, %s", #x, __LINE__, __FILE__);
#define D3DCALL(x)	  if(x != D3D_OK) dbg("WARNING! D3DCall fail line %d, %s", __LINE__, __FILE__);

extern bool emergencyRelease;

#define SAFE_RELEASE(x) if(!emergencyRelease&&x) x->Release();
#define SAFE_RELEASE_LAST(x) if(!emergencyRelease&&x) {if(x->Release() != 0) {dbg("WARNING! Refcount not zero on line %d, %s", __LINE__, __FILE__);}};
#define SAFE_FREE(x) if(!emergencyRelease&&x) while(x->Release()) {};
//#define SAFE_FREE(x) if(x) while(x->Release()) {};

#define FREEZE __asm {joo:jmp joo}
#define BP	__asm int 3;

#define DEGTORAD(ang) ((ang) * PI / 180.0)

#ifndef DEBUGPREFIX
#define DEBUGPREFIX	"SoftTH: "
#define CLEAR_LOG "STH_CLEAR_LOG"	// Special message used to clear log
#endif

#define dbgf if(0)
#define dbg_input if(0)
//#define dbg_input dbg

extern "C"  __declspec(dllexport) void ShowMessage(char *first, ...);
extern "C"  __declspec(dllexport) void dbg(char *first, ...);
void    __cdecl odprintf(const char *format, ...);
void    dbgSimple(char *first, ...);
//void    dbgf(char *first, ...);
char*   getModuleName(HMODULE mod);
void    startTimer();
double  stopTimer();
char*   getD3DError(int v);
char*   processName(void);
RECT*   isNullRect(RECT *r);
char*   strRect(CONST RECT *r);
char*   getBackLogLine(int line);
void    timeWarn(int start, int limit, char *type);
char*   matchRiid(REFIID riid);
int     getRef(IUnknown *o);
void    dumpPP(D3DPRESENT_PARAMETERS* pp);

#define VK_APPLICATION		0x5D // Application key (extended)

void    dumpSurfaceDesc(LPDIRECT3DSURFACE9 surf);
char*   getSwapEffect(D3DSWAPEFFECT se);
char*   getPresentationInterval(UINT pi);
void    d3dClearTarget(IDirect3DDevice9Ex *dev, IDirect3DSurface9 *tgt, IDirect3DSurface9 *tgtD);
char*   getMode(D3DFORMAT fmt);
char*   getFormatDXGI(DXGI_FORMAT fmt);
char*   getUsageDXGI(DXGI_USAGE usage);
char*   getUsage(DWORD usage);
char*   getLock(DWORD usage);
char*   getPool(D3DPOOL pool);
char*   getD3DCreate(DWORD usage);
bool    fileExists(char *name);
void    createDirs(char *fname);

void    memcpyPitched(void* dst, void *src, DWORD size, DWORD pitchDst, DWORD pitchSrc);

BOOL SetPrivilege(
    HANDLE hToken,          // token handle
    LPCTSTR Privilege,      // Privilege to enable/disable
    BOOL bEnablePrivilege   // TRUE to enable.  FALSE to disable
    );


#endif
