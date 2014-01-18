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

//
// The Amazing Global Hooker(tm)
// hooks dll functions with jmp/stub code at beginning of original code
// by Kegetys - http://www.kegetys.net
//

#define USE_DISTORM // Use distorm for instruction decoding

#include "globalHooker.h"

#include <windows.h>
#include "helper.h"
#include <stdio.h>

#ifdef USE_DISTORM
#include <distorm.h>
#pragma comment(lib, "distorm.lib")
#endif

// 32bit jump limit
#define TWOGB 2147483648LL


//#ifndef _WIN64
#if 1

static bool writeHook(GHOOK *h);
static bool restoreHook(GHOOK *h);

// Write JMP to target from ptr
static bool writeJmpOpcode(HANDLE h, void *ptr, void* target) {

	// Code blocks are usually read-only, change page protection
	DWORD oldprot = NULL;
  SIZE_T foo;
	if(IsBadWritePtr(ptr, JMPDATALEN))
		VirtualProtectEx(h, ptr, JMPDATALEN, PAGE_EXECUTE_READWRITE, &oldprot);
	
	const static BYTE jmp = 0xE9;		// JMP opcode
	LONG_PTR wptr = (LONG_PTR) ((BYTE*)((BYTE*)target - (BYTE*)ptr) - JMPDATALEN); // Relative jump address

#ifdef _WIN64
  if(wptr > TWOGB || wptr < -TWOGB)
  {
    // TODO: use 64bit jump instead!
    dbg("WARNING: Jump address over +/- 2GB limit");
    dbg("ERROR: writeJmpOpcode: x64 jmp not implemented!");
    return false;
  }
#endif

  //dbg("writeJmpOpcode: ptr: 0x%016lX target: 0x%016lX offset:0x%016lX", ptr, target, wptr);

	int r1 = WriteProcessMemory(h, ptr, &jmp, 1, &foo); // Write jump...
	int r2 = WriteProcessMemory(h, (void*) ((BYTE*)ptr+1), &wptr, 4, &foo); // Write ptr

	// Restore page protection
	if(oldprot)
		VirtualProtectEx(h, ptr, JMPDATALEN, oldprot, &oldprot);

	if(!r1 || !r2) {
		dbg("writeJmpOpcode: WriteProcessMemory failed!");
		return false;
	} else
		return true;
}

// Remove hook (restore function code)
static bool restoreHook(GHOOK *h) {
	if(!h->active)
		return false; // Not hooked
	HINSTANCE hLib = GetModuleHandle(h->dll);
	if(!hLib) {
		// Hooked module no longer loaded
		dbg("Module no longer loaded");
		return true;
	}

	HANDLE hSelf = NULL; 
	hSelf = OpenProcess(PROCESS_VM_WRITE|PROCESS_VM_OPERATION, FALSE, GetCurrentProcessId());
	if(!hSelf) // TODO: add debug access
		return false;

	// Copy instructions back from stub code to function beginning
	// Stub code must be kept in memory in case another thread is still executing hook
	DWORD oldprot = NULL;
	VirtualProtectEx(hSelf, (void*)h->funcOrig, h->stublen, PAGE_EXECUTE_READWRITE, &oldprot);
	memcpy((void*)h->funcOrig, h->stub, h->stublen);
	VirtualProtectEx(hSelf, (void*)h->funcOrig, h->stublen, oldprot, &oldprot);

	CloseHandle(hSelf);

	dbg("Hook disabled");
	h->active = false;
	return true;
}


void setHooks(GHOOK *hooks) {
  OutputDebugString("hook-king");
  int num = 0;
  while(hooks[num].funcOver)
    writeHook(&hooks[num]), num++;
  //dbg("Hooked %d calls", num);
}

void unsetHooks(GHOOK *hooks) {
  int num = 0;
  while(hooks[num].funcOver)
    restoreHook(&hooks[num]), num++;
  //dbg("Unset %d hooks", num);
}

// Write hook
static bool writeHook(GHOOK *h) {
	if(h->active)
		return true; // Already hooked
	//dbg("Hooking %s/%s", h->dll, h->name);

	memset(h->stub, 0x90, STUBLEN+JMPDATALEN); // Fill stub function with NOP

	// Load library & get proc address
	HINSTANCE hLib = GetModuleHandle(h->dll);
	if(!hLib) {
		dbg("Module not loaded");
		return false;
	}

	//h->funcOrig = (ULONG_PTR*) GetProcAddress((HMODULE) hLib, h->name);
  h->funcOrig = (ULONG_PTR) GetProcAddress((HMODULE) hLib, h->name);
	if(!h->funcOrig) {
		dbg("Export not found in DLL!");
		return false;
	}

	// Walk instructions from beginning of function to find stub data size & stub jmp pos
	BYTE *sJmp = instructionCount((BYTE*) h->funcOrig, 5);
	DWORD slen = (DWORD) (sJmp - (BYTE*)h->funcOrig);// Stub length (without jmp)  
	h->stublen = slen;

	// Open handle to current process
	HANDLE hSelf = NULL; 
	hSelf = OpenProcess(PROCESS_VM_WRITE|PROCESS_VM_OPERATION, FALSE, GetCurrentProcessId());
	if(!hSelf) {
		// Openprocess failed, attempt debug access
		HANDLE hToken;
		if(!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, FALSE, &hToken)) {
			ImpersonateSelf(SecurityImpersonation);
			OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, FALSE, &hToken);
		}
		if(!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
			// Debug access failed, propably non-admin user
		 	dbg("writeHook: SetPrivilege failed - Check user permissions");
			return false; // This isn't going to work
		} else {
			// Hey, it worked! Try again...
			hSelf = OpenProcess(PROCESS_VM_WRITE|PROCESS_VM_OPERATION, FALSE, GetCurrentProcessId());
			if(!hSelf) {
				dbg("OpenProcess failed");
				return false;
			}
		}
	}

	// Create stub code, copy data from function start & append jmp
	memcpy(h->stub, (void*)h->funcOrig, slen);
	writeJmpOpcode(hSelf, h->stub+slen, (void*) (h->funcOrig+slen));

	if(h->stub[0] == 0xE9) { // 0xE9 = jmp instruction
		// JMP at beginning of previous hook code, must change the offset or it will jump to wrong location
		// New correct position is: jump address + original function pointer - stub function pointer
		ULONG_PTR *jp = (ULONG_PTR*) (h->stub+1);
    ULONG_PTR newjp = *jp + ((BYTE*)h->funcOrig - (BYTE*)h->stub);
		dbg("Existing hook detected at %s/%s, redirecting JMP (0x%08X -> 0x%08X)", h->dll, h->name, *jp, newjp);

		*jp = newjp;
	}

	// Mark stub code executable
	DWORD oldprot = NULL;
	VirtualProtectEx(hSelf, h->stub, STUBLEN+JMPDATALEN, PAGE_EXECUTE_READWRITE, &oldprot);

	// Write jmp to beginning of original function
	writeJmpOpcode(hSelf, (void*)h->funcOrig, (void*) h->funcOver);

	CloseHandle(hSelf);
	//FreeLibrary(hLib);

  dbg("Hooked %s/%s", h->dll, h->name);

	h->active = true;
	return true;
}


#ifdef USE_DISTORM
// Count instructions
BYTE* instructionCount(BYTE *func, int tcount) { 

#ifndef _WIN64
  _DecodeType dt = Decode32Bits;
#else
  _DecodeType dt = Decode64Bits;
#endif

#define MAX_INSTRUCTIONS 100

	_DecodeResult res;
	_DecodedInst di[MAX_INSTRUCTIONS];
	unsigned int dic = 0;

  res = distorm_decode(0, (const BYTE *) func, 15, dt, di, MAX_INSTRUCTIONS, &dic);
  if(res == DECRES_INPUTERR)
    dbg("ERROR: distorm_decode failed!");

  DWORD is = 0;
  for(DWORD i=0;i<dic;i++)
  {
    if(is >= tcount)
      return func+is;
    is += di[i].size;
  }
  return 0;
}
#else
// Walk x86 instructions
BYTE* instructionCount(BYTE *func, int tcount) { 
  // No longer needed
  return 0;
}
#endif


#else
// 64bit stubs

void setHooks(GHOOK *hooks) {
}

void unsetHooks(GHOOK *hooks) {
}
#endif
