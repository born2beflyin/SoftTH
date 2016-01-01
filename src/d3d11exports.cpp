/*
SoftTH, Software multihead solution for Direct3D
Copyright (C) 2005-2012 Keijo Ruotsalainen, www.kegetys.fi
              2014-     C. Justin Ratcliff, www.softth.net

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

#include <windows.h>
//#include "helper.h"


extern HINSTANCE hLibD3D11;



#ifndef _WIN64
//#if 1

#ifdef _WIN64
#define STACK_BASE rbp // x64: rbp is stack base
#else
#define STACK_BASE ebp // x86: ebp is stack base
#endif

// Passthrough exports
#pragma warning (disable : 4731)
#define DEXPORT(hLib, x) _declspec(dllexport) void x(void) {\
	static FARPROC foo = GetProcAddress((HMODULE) hLib, #x);\
	__asm {pop STACK_BASE};\
	__asm {jmp foo};\
	}

//#define DEXPORTD(hLib, x) _declspec(dllexport) void x(void) {\
//  dbg("DEXPORT: %s - %s", #hLib, #x);\
//	static FARPROC foo = GetProcAddress((HMODULE) hLib, #x);\
//	__asm {pop STACK_BASE};\
//	__asm {jmp foo};\
//	}

extern "C" {
/* Direct3D 11 */
DEXPORT(hLibD3D11, D3D11CoreCreateDevice);
DEXPORT(hLibD3D11, D3D11CoreCreateLayeredDevice);
DEXPORT(hLibD3D11, D3D11CoreGetLayeredDeviceSize);
DEXPORT(hLibD3D11, D3D11CoreRegisterLayers);
//DEXPORT(hLibD3D11, D3D11CreateDevice);
//DEXPORT(hLibD3D11, D3D11CreateDeviceAndSwapChain);
DEXPORT(hLibD3D11, D3DKMTCloseAdapter);
DEXPORT(hLibD3D11, D3DKMTCreateAllocation);
DEXPORT(hLibD3D11, D3DKMTCreateContext);
DEXPORT(hLibD3D11, D3DKMTCreateDevice);
DEXPORT(hLibD3D11, D3DKMTCreateSynchronizationObject);
DEXPORT(hLibD3D11, D3DKMTDestroyAllocation);
DEXPORT(hLibD3D11, D3DKMTDestroyContext);
DEXPORT(hLibD3D11, D3DKMTDestroyDevice);
DEXPORT(hLibD3D11, D3DKMTDestroySynchronizationObject);
DEXPORT(hLibD3D11, D3DKMTEscape);
DEXPORT(hLibD3D11, D3DKMTGetContextSchedulingPriority);
DEXPORT(hLibD3D11, D3DKMTGetDeviceState);
DEXPORT(hLibD3D11, D3DKMTGetDisplayModeList);
DEXPORT(hLibD3D11, D3DKMTGetMultisampleMethodList);
DEXPORT(hLibD3D11, D3DKMTGetRuntimeData);
DEXPORT(hLibD3D11, D3DKMTGetSharedPrimaryHandle);
DEXPORT(hLibD3D11, D3DKMTLock);
DEXPORT(hLibD3D11, D3DKMTOpenAdapterFromHdc);
DEXPORT(hLibD3D11, D3DKMTOpenResource);
DEXPORT(hLibD3D11, D3DKMTPresent);
DEXPORT(hLibD3D11, D3DKMTQueryAdapterInfo);
DEXPORT(hLibD3D11, D3DKMTQueryAllocationResidency);
DEXPORT(hLibD3D11, D3DKMTQueryResourceInfo);
DEXPORT(hLibD3D11, D3DKMTRender);
DEXPORT(hLibD3D11, D3DKMTSetAllocationPriority);
DEXPORT(hLibD3D11, D3DKMTSetContextSchedulingPriority);
DEXPORT(hLibD3D11, D3DKMTSetDisplayMode);
DEXPORT(hLibD3D11, D3DKMTSetDisplayPrivateDriverFormat);
DEXPORT(hLibD3D11, D3DKMTSetGammaRamp);
DEXPORT(hLibD3D11, D3DKMTSetVidPnSourceOwner);
DEXPORT(hLibD3D11, D3DKMTSignalSynchronizationObject);
DEXPORT(hLibD3D11, D3DKMTUnlock);
DEXPORT(hLibD3D11, D3DKMTWaitForSynchronizationObject);
DEXPORT(hLibD3D11, D3DKMTWaitForVerticalBlankEvent);
DEXPORT(hLibD3D11, D3DPerformance_BeginEvent);
DEXPORT(hLibD3D11, D3DPerformance_EndEvent);
DEXPORT(hLibD3D11, D3DPerformance_GetStatus);
DEXPORT(hLibD3D11, D3DPerformance_SetMarker);
DEXPORT(hLibD3D11, EnableFeatureLevelUpgrade);
DEXPORT(hLibD3D11, OpenAdapter10);
DEXPORT(hLibD3D11, OpenAdapter10_2);
}
#pragma warning (default : 4731)

#endif



/*DEXPORTD(hLibD3D11, D3DKMTCloseAdapter);
DEXPORTD(hLibD3D11, D3DKMTDestroyAllocation);
DEXPORTD(hLibD3D11, D3DKMTDestroyContext);
DEXPORTD(hLibD3D11, D3DKMTDestroyDevice);
DEXPORTD(hLibD3D11, D3DKMTDestroySynchronizationObject);
DEXPORTD(hLibD3D11, D3DKMTQueryAdapterInfo);
DEXPORTD(hLibD3D11, D3DKMTSetDisplayPrivateDriverFormat);
DEXPORTD(hLibD3D11, D3DKMTSignalSynchronizationObject);
DEXPORTD(hLibD3D11, D3DKMTUnlock);
DEXPORTD(hLibD3D11, D3DKMTWaitForSynchronizationObject);
DEXPORTD(hLibD3D11, OpenAdapter10);
DEXPORTD(hLibD3D11, OpenAdapter10_2);
DEXPORTD(hLibD3D11, D3D11CoreCreateDevice);
DEXPORTD(hLibD3D11, D3D11CoreCreateLayeredDevice);
DEXPORTD(hLibD3D11, D3D11CoreGetLayeredDeviceSize);
DEXPORTD(hLibD3D11, D3D11CoreRegisterLayers);
//DEXPORTD(hLibD3D11, D3D11CreateDevice);
//DEXPORTD(hLibD3D11, D3D11CreateDeviceAndSwapChain);
DEXPORTD(hLibD3D11, D3DKMTCreateAllocation);
DEXPORTD(hLibD3D11, D3DKMTCreateContext);
DEXPORTD(hLibD3D11, D3DKMTCreateDevice);
DEXPORTD(hLibD3D11, D3DKMTCreateSynchronizationObject);
DEXPORTD(hLibD3D11, D3DKMTEscape);
DEXPORTD(hLibD3D11, D3DKMTGetContextSchedulingPriority);
DEXPORTD(hLibD3D11, D3DKMTGetDeviceState);
DEXPORTD(hLibD3D11, D3DKMTGetMultisampleMethodList);
DEXPORTD(hLibD3D11, D3DKMTGetRuntimeData);
DEXPORTD(hLibD3D11, D3DKMTGetSharedPrimaryHandle);
DEXPORTD(hLibD3D11, D3DKMTLock);
DEXPORTD(hLibD3D11, D3DKMTOpenAdapterFromHdc);
DEXPORTD(hLibD3D11, D3DKMTOpenResource);
DEXPORTD(hLibD3D11, D3DKMTPresent);
DEXPORTD(hLibD3D11, D3DKMTQueryAllocationResidency);
DEXPORTD(hLibD3D11, D3DKMTRender);
DEXPORTD(hLibD3D11, D3DKMTSetAllocationPriority);
DEXPORTD(hLibD3D11, D3DKMTSetContextSchedulingPriority);
DEXPORTD(hLibD3D11, D3DKMTSetDisplayMode);
DEXPORTD(hLibD3D11, D3DKMTSetGammaRamp);
DEXPORTD(hLibD3D11, D3DKMTQueryResourceInfo);
DEXPORTD(hLibD3D11, D3DKMTSetVidPnSourceOwner);
DEXPORTD(hLibD3D11, D3DKMTWaitForVerticalBlankEvent);*/
