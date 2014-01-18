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

#include <stdio.h>
#include <windows.h>
#include "helper.h"
#include "win32hooks.h"

#include "main.h"

#if DUMP_IMPORTS
#include "Dbghelp.h"
#endif

static IMAGE_IMPORT_DESCRIPTOR* getID(HMODULE lib, char *dll)
{
  IMAGE_DOS_HEADER* doshdr = (IMAGE_DOS_HEADER*) lib;
  IMAGE_OPTIONAL_HEADER* opthdr = (IMAGE_OPTIONAL_HEADER*)((BYTE*) lib + doshdr->e_lfanew + 24);
  IMAGE_IMPORT_DESCRIPTOR* impdesc = (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*) lib + opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

  while(impdesc->FirstThunk) {
    char *iname = (char*)((BYTE*)lib+impdesc->Name);
    //OutputDebugString(iname);
    if(!_stricmp(iname, dll))
      return impdesc;
    impdesc++;
  }
  return NULL;
}

static FARPROC* getFPtr(HMODULE lib, IMAGE_THUNK_DATA* oft, IMAGE_THUNK_DATA* ft, char* fname)
{
  while(oft->u1.Function) {
    char *f = (char*)((BYTE*)lib + (DWORD)oft->u1.AddressOfData+2);
    //OutputDebugString(f);
    if(!_stricmp(fname, f))
      return (FARPROC*) &ft->u1.Function;
    oft++;
    ft++;
  }
  return NULL;
}

FARPROC hookImport(HMODULE lib, char *dll, char *func, FARPROC nptr)
{
  IMAGE_IMPORT_DESCRIPTOR *id = getID(lib, dll);
  FARPROC *fptr = getFPtr(lib, (IMAGE_THUNK_DATA*)((BYTE*)lib+id->OriginalFirstThunk), (IMAGE_THUNK_DATA*)((BYTE*)lib+id->FirstThunk), func);
  if(!fptr)
    return 0;

  DWORD op;
  FARPROC old = *fptr;
  VirtualProtect(fptr, 4, PAGE_READWRITE, &op);
  *fptr = nptr;
  VirtualProtect(fptr, 4, op, &op);
  //"OutputDebugStringA"
  return old;
}
/*
static FARPROC WINAPI GetProcAddressNew(HMODULE hModule, LPCSTR lpProcName)
{
  dbg("GetProcAddressNew: %s", lpProcName);
  return GetProcAddress(hModule, lpProcName);
}

static W32HOOK *curHooks = NULL;
static int numCurHooks = 0;
static HMODULE curTgt = NULL;

void setHookTable(HMODULE tgt, W32HOOK *hooks)
{
  // Release old hooks
  for(int i=0;i<numCurHooks;i++) {
    W32HOOK *h = &curHooks[i];
    HMODULE lib = GetModuleHandle(h->dll);
    if(lib) {
      FARPROC fp = GetProcAddress(lib, h->func);
      if(fp) {
        dbg("Releasing hook: %s:%s", h->dll, h->func);
        hookImport(curTgt, h->dll, h->func, h->ptrNew);
      }
    }
  }

  if(!hooks)
    return;

  int numHooks = 0;
  while(hooks[numHooks].dll[0]) numHooks++;

  // Hook APIs on table
  for(int i=0;i<numHooks;i++) {
    W32HOOK *h = &hooks[i];
    dbg("Hooking: %s:%s", h->dll, h->func);
    hookImport(tgt, h->dll, h->func, h->ptrNew);
  }

  curTgt = tgt;
  numCurHooks = numHooks;
  curHooks = new W32HOOK[numCurHooks];
  memcpy(curHooks, hooks, sizeof(W32HOOK)*numHooks);

  // Hook getprocaddress - in case application tries to get handle to a hooked function
  hookImport(tgt, "kernel32.dll", "GetProcAddressA", (FARPROC) GetProcAddressNew);
}


*/

#if DUMP_IMPORTS
static void dumpFPtr(HMODULE lib, IMAGE_THUNK_DATA* oft, IMAGE_THUNK_DATA* ft)
{
  while(oft->u1.Function) {
    char *f = (char*)((BYTE*)lib + (DWORD)oft->u1.AddressOfData+2);

    char buf[5];
    BOOL r = ReadProcessMemory(GetCurrentProcess(), f, buf, 4, NULL);
    if(r)
      dbg("     \\_ import: <%s>", f);
    else
      dbg("     \\_ Cannot access import name!", f);
    /*if(oft->u1.AddressOfData < 0x80000000)
      dbg("     \\_ import: <%s>", f);*/
    oft++;
    ft++;
  }
  return;
}

static void dumpImports(HMODULE lib)
{
  IMAGE_DOS_HEADER* doshdr = (IMAGE_DOS_HEADER*) lib;
  IMAGE_OPTIONAL_HEADER* opthdr = (IMAGE_OPTIONAL_HEADER*)((BYTE*) lib + doshdr->e_lfanew + 24);
  IMAGE_IMPORT_DESCRIPTOR* impdesc = (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*) lib + opthdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

  while(impdesc->FirstThunk) {
    char *iname = (char*)((BYTE*)lib+impdesc->Name);

    char buf[5];
    BOOL r = ReadProcessMemory(GetCurrentProcess(), iname, buf, 4, NULL);
    if(r) 
    {
      dbg(" \\_ DLL: <%s>", iname);
      dumpFPtr(lib, (IMAGE_THUNK_DATA*)((BYTE*)lib+impdesc->OriginalFirstThunk), (IMAGE_THUNK_DATA*)((BYTE*)lib+impdesc->FirstThunk));
    } else {
      dbg("  \\_ Cannot access DLL name!");
    }

    /*if(!_stricmp(iname, dll))
      return impdesc;*/
    impdesc++;
  }
  return;
}

BOOL CALLBACK menum(PCTSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
  dbg("Module: <%s>", ModuleName);
  
  HMODULE mod = NULL;
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)ModuleBase, &mod);
  if(mod)
    dumpImports(mod);
  else
    dbg("  Cannot find module?");
  
  return TRUE;
}

void dumpAllImports()
{
  EnumerateLoadedModulesEx(GetCurrentProcess(), menum, NULL);
  dbg("dumpAllImports: done");
}
#endif