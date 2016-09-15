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

//
// Main DLL loader
//

#include <windows.h>
#include <stdio.h>
#include <string>
#include "d3d11main.h"

using namespace std;

// SoftTH main dll import function/variable prototypes
void (__stdcall * dbg)(char *first, ...) = NULL;
void (__stdcall * ShowMessage)(char *first, ...) = NULL;
//void (__stdcall * addNoHookModule)(HMODULE mod) = NULL;
DLL configFile config; // Main configuration
DLL HINSTANCE hLibD3D11;


/* D3D 11 real function prototypes */
/*HRESULT (WINAPI*dllD3D11CreateDevice)(IDXGIAdapter *adapter,
                                      D3D_DRIVER_TYPE DriverType,
                                      HMODULE Software,
                                      UINT Flags,
                                      const D3D_FEATURE_LEVEL *pFeatureLevels,
                                      UINT FeatureLevels,
                                      UINT SDKVersion,
                                      ID3D11Device** ppDevice,
                                      D3D_FEATURE_LEVEL *pFeatureLevel,
                                      ID3D11DeviceContext **ppImmediateContext) = NULL;

extern "C" __declspec(dllexport) HRESULT (WINAPI*dllD3D11CreateDeviceAndSwapChain)(IDXGIAdapter *adapter,
                                                                                   D3D_DRIVER_TYPE DriverType,
                                                                                   HMODULE Software,
                                                                                   UINT Flags,
                                                                                   const D3D_FEATURE_LEVEL *pFeatureLevels,
                                                                                   UINT FeatureLevels,
                                                                                   UINT SDKVersion,
                                                                                   const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                                                                   IDXGISwapChain **ppSwapChain,
                                                                                   ID3D11Device** ppDevice,
                                                                                   D3D_FEATURE_LEVEL *pFeatureLevel,
                                                                                   ID3D11DeviceContext **ppImmediateContext) = NULL;
*/


/* D3D 11 SoftTH function prototypes */
/*extern "C" __declspec(dllexport) HRESULT (WINAPI*newD3D11CreateDevice)(IDXGIAdapter *adapter,
                                      D3D_DRIVER_TYPE DriverType,
                                      HMODULE Software,
                                      UINT Flags,
                                      const D3D_FEATURE_LEVEL *pFeatureLevels,
                                      UINT FeatureLevels,
                                      UINT SDKVersion,
                                      ID3D11Device** ppDevice,
                                      D3D_FEATURE_LEVEL *pFeatureLevel,
                                      ID3D11DeviceContext **ppImmediateContext) = NULL;

extern "C" __declspec(dllexport) HRESULT (WINAPI*newD3D11CreateDeviceAndSwapChain)(IDXGIAdapter *adapter,
                                                                                   D3D_DRIVER_TYPE DriverType,
                                                                                   HMODULE Software,
                                                                                   UINT Flags,
                                                                                   const D3D_FEATURE_LEVEL *pFeatureLevels,
                                                                                   UINT FeatureLevels,
                                                                                   UINT SDKVersion,
                                                                                   const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                                                                   IDXGISwapChain **ppSwapChain,
                                                                                   ID3D11Device** ppDevice,
                                                                                   D3D_FEATURE_LEVEL *pFeatureLevel,
                                                                                   ID3D11DeviceContext **ppImmediateContext) = NULL;
*/

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD reason, LPVOID lpReserved)
{

  SoftTHMod = NULL;
  //D3D11Mod = NULL;
  hLibSoftTH = NULL; // Main SoftTH dll (dxgi.dll)
  //hLibD3D11 = NULL; // Real d3d11.dll

  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
    {
      bool hooks = true;

      /* Load main SoftTH library */
      {
        // Load the library
        SoftTHMod = new Module;
        char appath[256];
        GetModuleFileName(hModule, appath, 256);
        string path(appath);
        size_t lpos = path.find_last_of("\\");
        path = path.substr(0,lpos);
        path += "\\dxgi.dll";
        const char *mypath = path.c_str();
        if(SoftTHMod->SetHandle(path.c_str()))
          hLibSoftTH = SoftTHMod->GetHandle();
        //else
        //  ShowMessage("Main SoftTH library not found (dxgi.dll)!"), exit(0);

        // Capture functions from main SoftTH library
        dbg = (void(__stdcall *)(char*,...)) GetProcAddress(hLibSoftTH,"dbg");
        ShowMessage = (void(__stdcall *)(char*,...)) GetProcAddress(hLibSoftTH,"ShowMessage");
        //addNoHookModule = (void(__stdcall *)(HMODULE)) GetProcAddress(hLibSoftTH,"addNoHookModule");

        //dbg("d3d11: Main SoftTH functions captured.");
      }

      /* Load D3D11 library */
      {
        /*char path[256];
        if(strlen(config.main.dllPathD3D11) < 2)
          sprintf(path, "%s\\system32\\%s", getenv("SystemRoot"), "d3d11.dll");
        else
          strcpy(path, config.main.dllPathD3D11);
        //dbg("D3D11 DLL Path: <%s>", path);

        D3D11Mod = new Module;
        if(D3D11Mod->SetHandle(path)) {
          hLibD3D11 = D3D11Mod->GetHandle();
          dbg("d3d11: Got D3D11 dll: %s, 0x%08X", path, &hLibD3D11);
        } else
		      ShowMessage("D3D11 DLL not found!\n'%s'", path), exit(0);*/

        /*dllD3D11CreateDevice = (HRESULT(__stdcall *)(IDXGIAdapter *,
                                      D3D_DRIVER_TYPE,
                                      HMODULE,
                                      UINT,
                                      const D3D_FEATURE_LEVEL *,
                                      UINT,
                                      UINT,
                                      ID3D11Device **,
                                      D3D_FEATURE_LEVEL *,
                                      ID3D11DeviceContext **))
                                      GetProcAddress(hLibD3D11, "D3D11CreateDevice");

        dllD3D11CreateDeviceAndSwapChain = (HRESULT(__stdcall *)(IDXGIAdapter *,
                                                                 D3D_DRIVER_TYPE,
                                                                 HMODULE,
                                                                 UINT,
                                                                 const D3D_FEATURE_LEVEL *,
                                                                 UINT,
                                                                 UINT,
                                                                 const DXGI_SWAP_CHAIN_DESC *,
                                                                 IDXGISwapChain **,
                                                                 ID3D11Device **,
                                                                 D3D_FEATURE_LEVEL *,
                                                                 ID3D11DeviceContext **))
                                                                 GetProcAddress(hLibD3D11, "D3D11CreateDeviceAndSwapChain");
        */

        /*newD3D11CreateDevice = (HRESULT(__stdcall *)(IDXGIAdapter *,
                                      D3D_DRIVER_TYPE,
                                      HMODULE,
                                      UINT,
                                      const D3D_FEATURE_LEVEL *,
                                      UINT,
                                      UINT,
                                      ID3D11Device **,
                                      D3D_FEATURE_LEVEL *,
                                      ID3D11DeviceContext **))
                                      GetProcAddress(hLibSoftTH, "D3D11CreateDevice");

        newD3D11CreateDeviceAndSwapChain = (HRESULT(__stdcall *)(IDXGIAdapter *,
                                                                 D3D_DRIVER_TYPE,
                                                                 HMODULE,
                                                                 UINT,
                                                                 const D3D_FEATURE_LEVEL *,
                                                                 UINT,
                                                                 UINT,
                                                                 const DXGI_SWAP_CHAIN_DESC *,
                                                                 IDXGISwapChain **,
                                                                 ID3D11Device **,
                                                                 D3D_FEATURE_LEVEL *,
                                                                 ID3D11DeviceContext **))
                                                                 GetProcAddress(hLibSoftTH, "D3D11CreateDeviceAndSwapChain");
*/

        /*if(!newD3D11CreateDevice)
		      ShowMessage("D3D11CreateDevice not mapped to SoftTH DLL\n"), exit(0);
        if(!newD3D11CreateDeviceAndSwapChain)
		      ShowMessage("D3D11CreateDeviceAndSwapChain not mapped to SoftTH DLL"), exit(0);*/
      }

      if(hooks) {
        // Pin our DLL - cannot allow unloading since hook code is stored by us
        char fn[256];
        GetModuleFileName(hModule, fn, 256);
        HMODULE foo;
        HRESULT ret = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, fn, &foo);
        if(!ret)
          dbg("Failed to pin DLL: error %d", GetLastError());
        //else
        //  dbg("Pinned DLL: <%s>", fn);

        /*if(hLibSoftTH) addNoHookModule(hLibSoftTH);*/
        /*if(hLibD3D11) addNoHookModule(hLibD3D11);*/
      }

      break;
    }

    case DLL_PROCESS_DETACH:
    {

			/*if(hLibD3D11) {
				FreeLibrary(hLibD3D11);
				hLibD3D11 = NULL;
			}*/
      if(hLibSoftTH) {
        //FreeLibrary(hLibSoftTH);
        hLibSoftTH = NULL;
      }

			/*if(D3D11Mod)
      {
        D3D11Mod->Release();
        delete D3D11Mod;
        D3D11Mod = NULL;
      }*/
      if(SoftTHMod)
      {
        SoftTHMod->Release();
        delete SoftTHMod;
        SoftTHMod = NULL;
      }

      break;
    }
  }
  return true;
}

/*    Direct3D 11     */
////D3D11CreateDevice
//extern "C" _declspec(dllexport) HRESULT WINAPI newD3D11CreateDevice(IDXGIAdapter *adapter,
//                                                                    D3D_DRIVER_TYPE DriverType,
//                                                                    HMODULE Software,
//                                                                    UINT Flags,
//                                                                    const D3D_FEATURE_LEVEL *pFeatureLevels,
//                                                                    UINT FeatureLevels,
//                                                                    UINT SDKVersion,
//                                                                    ID3D11Device** ppDevice,
//                                                                    D3D_FEATURE_LEVEL *pFeatureLevel,
//                                                                    ID3D11DeviceContext **ppImmediateContext)
//{
//  dbg("d3d11: D3D11CreateDevice 0x%08X 0x%08X", adapter, *adapter);
//
//  HRESULT ret = dllD3D11CreateDevice(adapter,
//                                     DriverType,
//                                     Software,
//                                     Flags,
//                                     pFeatureLevels,
//                                     FeatureLevels,
//                                     SDKVersion,
//                                     ppDevice,
//                                     pFeatureLevel,
//                                     ppImmediateContext);
//
//  /*IDXGIAdapter1New *anew;
//  if(adapter->QueryInterface(IID_IDXGIAdapter1New, (void**) &anew) == S_OK) {
//    adapter = anew->getReal();
//    anew->Release();
//  }*/
//
//  return ret;
//}

////D3D11CreateDeviceAndSwapChain
//extern "C" _declspec(dllexport) HRESULT WINAPI newD3D11CreateDeviceAndSwapChain(IDXGIAdapter *adapter,
//                                                                                D3D_DRIVER_TYPE DriverType,
//                                                                                HMODULE Software,
//                                                                                UINT Flags,
//                                                                                const D3D_FEATURE_LEVEL *pFeatureLevels,
//                                                                                UINT FeatureLevels,
//                                                                                UINT SDKVersion,
//                                                                                DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
//                                                                                IDXGISwapChain **ppSwapChain,
//                                                                                ID3D11Device** ppDevice,
//                                                                                D3D_FEATURE_LEVEL *pFeatureLevel,
//                                                                                ID3D11DeviceContext **ppImmediateContext)
//{
//  dbg("d3d11: D3D11CreateDeviceAndSwapChain 0x%08X 0x%08X", adapter, *adapter);
//
//  /*dbg("Mode: %dx%d %d.%dHz %s", pSwapChainDesc->BufferDesc.Width, pSwapChainDesc->BufferDesc.Height, pSwapChainDesc->BufferDesc.RefreshRate.Numerator, pSwapChainDesc->BufferDesc.RefreshRate.Denominator, pSwapChainDesc->Windowed?"Windowed":"Fullscreen");
//  dbg("Multisample: %d samples, quality %d", pSwapChainDesc->SampleDesc.Count, pSwapChainDesc->SampleDesc.Quality);
//  dbg("Buffers: %d (Usage %s), Swapeffect: %s", pSwapChainDesc->BufferCount, getUsageDXGI(pSwapChainDesc->BufferUsage), pSwapChainDesc->SwapEffect==DXGI_SWAP_EFFECT_DISCARD?"DISCARD":"SEQUENTIAL");
//
//  dbg("Flags: %s %s %s", pSwapChainDesc->Flags&DXGI_SWAP_CHAIN_FLAG_NONPREROTATED?"NONPREROTATED":"",
//                         pSwapChainDesc->Flags&DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH?"ALLOW_MODE_SWITCH":"",
//                         pSwapChainDesc->Flags&DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE?"GDI_COMPATIBLE":"");*/
//
//  HRESULT ret;
//
//  ret = D3D11CreateDevice(adapter,
//                          DriverType,
//                          Software,
//                          Flags,
//                          pFeatureLevels,
//                          FeatureLevels,
//                          SDKVersion,
//                          ppDevice,
//                          pFeatureLevel,
//                          ppImmediateContext);
//
//  //
//  /*IDXGIAdapter1New *anew;
//  if(adapter->QueryInterface(IID_IDXGIAdapter, (void**) &anew) == S_OK) {
//    adapter = anew->getReal();
//    anew->Release();
//  }*/
//  //
//
//  IDXGIFactory1 *factory;
//  //IDXGIFactory1New *fnew;
//  //CreateDXGIFactory1()
//  if(adapter->GetParent(IID_IDXGIFactory, (void**) &factory) == S_OK)
//  {
//    //factory = fnew->getReal();
//    //fnew->Release();
//    dbg("d3d11: Got parent factory");
//  }
//
//  /*ret =  dllD3D11CreateDeviceAndSwapChain(adapter,
//                                          DriverType,
//                                          Software,
//                                          Flags,
//                                          pFeatureLevels,
//                                          FeatureLevels,
//                                          SDKVersion,
//                                          pSwapChainDesc,
//                                          ppSwapChain,
//                                          ppDevice,
//                                          pFeatureLevel,
//                                          ppImmediateContext);*/
//
//  (*ppSwapChain) = new IDXGISwapChainNew(factory, factory, *ppDevice, pSwapChainDesc);
//
//
//
//  /*IDXGISwapChainNew *scnew;
//  if((*ppSwapChain)->QueryInterface(IID_IDXGISwapChainNew, (void**) &scnew) == S_OK) {
//    (*ppSwapChain) = scnew->getReal();
//    scnew->Release();
//  } else dbg("Booh! No real swap chain!");*/
//
//  /*if(fnew)
//  {
//    fnew->Release();
//    delete fnew;
//    fnew = NULL;
//  }*/
//
//  return ret;
//}

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

#define DEXPORTD(hLib, x) _declspec(dllexport) void x(void) {\
	static FARPROC foo = GetProcAddress((HMODULE) hLib, #x);\
	if (!foo) { dbg("Error getting address (Code: %d)", GetLastError()); }\
  GetModuleFileName((HMODULE) hLib,libfn,256);\
  dbg("DEXPORTD: %s, %s, 0x%08X - %s, 0x%08X", #hLib, libfn, &hLib, #x, foo);\
	__asm {pop STACK_BASE};\
	__asm {jmp foo};\
	}

extern "C" {
/* Direct3D 11 */
DEXPORTD(hLibD3D11, CreateDirect3D11DeviceFromDXGIDevice);
DEXPORTD(hLibD3D11, CreateDirect3D11SurfaceFromDXGISurface);
DEXPORTD(hLibD3D11, D3D11CoreCreateDevice);
DEXPORTD(hLibD3D11, D3D11CoreCreateLayeredDevice);
DEXPORTD(hLibD3D11, D3D11CoreGetLayeredDeviceSize);
DEXPORTD(hLibD3D11, D3D11CoreRegisterLayers);
DEXPORTD(hLibD3D11, D3D11CreateDevice);
//DEXPORTD(hLibD3D11, D3D11CreateDeviceAndSwapChain);
//DEXPORTD(hLibSoftTH, D3D11CreateDevice);
DEXPORTD(hLibSoftTH, D3D11CreateDeviceAndSwapChain);
DEXPORTD(hLibD3D11, D3D11CreateDeviceForD3D12);
DEXPORTD(hLibD3D11, D3D11On12CreateDevice);
DEXPORTD(hLibD3D11, D3DKMTCloseAdapter);
DEXPORTD(hLibD3D11, D3DKMTCreateAllocation);
DEXPORTD(hLibD3D11, D3DKMTCreateContext);
DEXPORTD(hLibD3D11, D3DKMTCreateDevice);
DEXPORTD(hLibD3D11, D3DKMTCreateSynchronizationObject);
DEXPORTD(hLibD3D11, D3DKMTDestroyAllocation);
DEXPORTD(hLibD3D11, D3DKMTDestroyContext);
DEXPORTD(hLibD3D11, D3DKMTDestroyDevice);
DEXPORTD(hLibD3D11, D3DKMTDestroySynchronizationObject);
DEXPORTD(hLibD3D11, D3DKMTEscape);
DEXPORTD(hLibD3D11, D3DKMTGetContextSchedulingPriority);
DEXPORTD(hLibD3D11, D3DKMTGetDeviceState);
DEXPORTD(hLibD3D11, D3DKMTGetDisplayModeList);
DEXPORTD(hLibD3D11, D3DKMTGetMultisampleMethodList);
DEXPORTD(hLibD3D11, D3DKMTGetRuntimeData);
DEXPORTD(hLibD3D11, D3DKMTGetSharedPrimaryHandle);
DEXPORTD(hLibD3D11, D3DKMTLock);
DEXPORTD(hLibD3D11, D3DKMTOpenAdapterFromHdc);
DEXPORTD(hLibD3D11, D3DKMTOpenResource);
DEXPORTD(hLibD3D11, D3DKMTPresent);
DEXPORTD(hLibD3D11, D3DKMTQueryAdapterInfo);
DEXPORTD(hLibD3D11, D3DKMTQueryAllocationResidency);
DEXPORTD(hLibD3D11, D3DKMTQueryResourceInfo);
DEXPORTD(hLibD3D11, D3DKMTRender);
DEXPORTD(hLibD3D11, D3DKMTSetAllocationPriority);
DEXPORTD(hLibD3D11, D3DKMTSetContextSchedulingPriority);
DEXPORTD(hLibD3D11, D3DKMTSetDisplayMode);
DEXPORTD(hLibD3D11, D3DKMTSetDisplayPrivateDriverFormat);
DEXPORTD(hLibD3D11, D3DKMTSetGammaRamp);
DEXPORTD(hLibD3D11, D3DKMTSetVidPnSourceOwner);
DEXPORTD(hLibD3D11, D3DKMTSignalSynchronizationObject);
DEXPORTD(hLibD3D11, D3DKMTUnlock);
DEXPORTD(hLibD3D11, D3DKMTWaitForSynchronizationObject);
DEXPORTD(hLibD3D11, D3DKMTWaitForVerticalBlankEvent);
DEXPORTD(hLibD3D11, D3DPerformance_BeginEvent);
DEXPORTD(hLibD3D11, D3DPerformance_EndEvent);
DEXPORTD(hLibD3D11, D3DPerformance_GetStatus);
DEXPORTD(hLibD3D11, D3DPerformance_SetMarker);
DEXPORTD(hLibD3D11, EnableFeatureLevelUpgrade);
DEXPORTD(hLibD3D11, OpenAdapter10);
DEXPORTD(hLibD3D11, OpenAdapter10_2);
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
