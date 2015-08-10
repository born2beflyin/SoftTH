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
// Main DLL loader
//

//#define _CRT_ASSEMBLY_VERSION "8.0.50727.762"
#define _BIND_TO_CURRENT_CRT_VERSION 0
#define _BIND_TO_CURRENT_VCLIBS_VERSION 0

#include <windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include "d3d9.h"
#include "helper.h"
#include "d3d.h"
#include "dxgiFactory.h"
#include "dxgiAdapterOutput.h"
#include "configFile.h"
#include "InputHandler.h"
#include "Dwmapi.h"

#include "win32hooks.h"
#include "globalHooker.h"
#include "hooksSoftTH.h"

#include "Shlobj.h"
#include "main.h"

HINSTANCE hLibD3D9 = NULL; // Real d3d9.dll
HINSTANCE hLibDXGI = NULL; // Real dxgi.dll
HINSTANCE hLibD3D10 = NULL; // Real d3d10.dll
HINSTANCE hLibD3D10_1 = NULL; // Real d3d10.dll
HINSTANCE hLibD3D11 = NULL; // Real d3d11.dll
HINSTANCE hLibD3D12 = NULL; // Real d3d12.dll
HINSTANCE hSelf = NULL; // This d3d9/dxgi.dll

//#define PREHOOKING

char hLibD3D9_path[256]; // Path to real d3d9.dll


// Get the REAL DLL functions
/* D3D 9 */
HRESULT (WINAPI*dllDirect3DCreate9Ex)(UINT SDKVersion, IDirect3D9Ex**) = NULL;
IDirect3D9* (WINAPI*dllDirect3DCreate9)(UINT SDKVersion) = NULL;
HRESULT (WINAPI*dllDXGID3D10CreateDevice)(HMODULE d3d10core,
                                          IDXGIFactory *factory,
                                          IDXGIAdapter *adapter,
                                          UINT flags,
                                          DWORD unknown0,
                                          void **device) = NULL;

/* DXGI */
HRESULT (WINAPI*dllCreateDXGIFactory)(REFIID riid, void **ppFactory) = NULL;
HRESULT (WINAPI*dllCreateDXGIFactory1)(REFIID riid, void **ppFactory) = NULL;
//HRESULT (WINAPI*dllCreateDXGIFactory2)(UINT Flags, REFIID riid, void **ppFactory) = NULL;

/* D3D 10 */
HRESULT (WINAPI*dllD3D10CreateDevice)(IDXGIAdapter *adapter,
                                      D3D10_DRIVER_TYPE DriverType,
                                      HMODULE Software,
                                      UINT Flags,
                                      UINT SDKVersion,
                                      ID3D10Device** ppDevice) = NULL;

extern "C" __declspec(dllexport) HRESULT (WINAPI*dllD3D10CreateDeviceAndSwapChain)(IDXGIAdapter *adapter,
                                                                                   D3D10_DRIVER_TYPE DriverType,
                                                                                   HMODULE Software,
                                                                                   UINT Flags,
                                                                                   UINT SDKVersion,
                                                                                   DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                                                                   IDXGISwapChain **ppSwapChain,
                                                                                   ID3D10Device **ppDevice) = NULL;

/* D3D 10.1 */
HRESULT (WINAPI*dllD3D10CreateDevice1)(IDXGIAdapter *adapter,
                                       D3D10_DRIVER_TYPE DriverType,
                                       HMODULE Software,
                                       UINT Flags,
                                       D3D10_FEATURE_LEVEL1 HardwareLevel,
                                       UINT SDKVersion,
                                       ID3D10Device1** ppDevice) = NULL;

extern "C" __declspec(dllexport) HRESULT (WINAPI*dllD3D10CreateDeviceAndSwapChain1)(IDXGIAdapter *adapter,
                                                                                    D3D10_DRIVER_TYPE DriverType,
                                                                                    HMODULE Software,
                                                                                    UINT Flags,
                                                                                    D3D10_FEATURE_LEVEL1 HardwareLevel,
                                                                                    UINT SDKVersion,
                                                                                    DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
                                                                                    IDXGISwapChain **ppSwapChain,
                                                                                    ID3D10Device1 **ppDevice) = NULL;


/* D3D 11 */
HRESULT (WINAPI*dllD3D11CreateDevice)(IDXGIAdapter *adapter,
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

DLL configFile config; // Main configuration
bool emergencyRelease = false;  // If true, releasing is being done from dll detach (Releasing D3D stuff is already too late)
std::list<GAMMARAMP*> restoreGammaRamps; // Stores default gamma ramps for emergency restore on DLL release

extern GHOOK SoftTHHooks[];
extern GHOOK InitHooks[];
bool didDisableComposition = false;

// Overridden outputdebugstring from d3d9d.dll
void WINAPI OutputDebugStringNew(char* str) {
  size_t len = strlen(str);
  for(DWORD i=0;i<len;i++)
    if(str[i] == 0x0d)
      str[i] = 0x00;
  if(strlen(str) < 2)
    return;
  dbg(str);
};

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD reason, LPVOID lpReserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
    {

#ifdef PREHOOKING
      // Do pre-hooking
      // This will hook GetModuleHandle to fool other hook apps such as steam overlay
      // Callin anything else (even dbg) will often trigger other hooks so it must be done first here
      setHooks(InitHooks);
#endif

      dbg(CLEAR_LOG);
      dbg("%s (%s, %s)", SOFTTH_VERSION, processName(), lpReserved?"static link":"dynamic link");
      dbg("Arguments: <%s>", GetCommandLine());
      dbg("--- DLL_PROCESS_ATTACH ---");

      // Create SoftTH paths
      char mydocs[256] = "";
      SHGetFolderPath(0, CSIDL_PERSONAL, NULL, NULL, mydocs);
      char foo[256];
      sprintf(foo, "%s/SoftTH/Screenshots/", mydocs);
      createDirs(foo);

      hSelf = hModule;

      // Load configuration settings
      config.load();

      bool hooks = true;
      // Set win32 hooks
      if(hooks) {
        dbg("Initializing win32 hooks...");

        // List modules that should not have their code hooked
        addNoHookModule(hModule);
        addNoHookModule(GetModuleHandle("kernel32.dll"));
        addNoHookModule(GetModuleHandle("user32.dll"));

        setHooks(SoftTHHooks);

        // Pin our DLL - cannot allow unloading since hook code is stored by us
        char fn[256];
        GetModuleFileName(hModule, fn, 256);
        HMODULE foo;
        HRESULT ret = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, fn, &foo);
        if(!ret)
          dbg("Failed to pin DLL: error %d", GetLastError());
        else
          dbg("Pinned DLL: <%s>", fn);
      }

      /* Load D3D9 Library */
      {
        bool useDebug = config.main.debugD3D;

        char *lib = "d3d9.dll";
        char *libd = "d3d9d.dll";

        char path[256];
        if(strlen(config.main.dllPathD3D9) < 2)
          sprintf(path, "%s\\system32\\%s", getenv("SystemRoot"), useDebug?libd:lib);
        else
          strcpy(path, config.main.dllPathD3D9);
        dbg("Direct3D DLL Path: <%s>", path);
        strcpy(hLibD3D9_path, path);
        hLibD3D9 = LoadLibrary(path);
	      if(!hLibD3D9)
		      ShowMessage("Direct3D DLL not found!\n'%s'", path), exit(0);

        if(useDebug) {
          // Hook into the OutputDebugStringA function of d3d9d.dll to capture debug output
          hookImport(hLibD3D9, "kernel32.dll", "OutputDebugStringA", (FARPROC)OutputDebugStringNew);
        }

        dllDirect3DCreate9Ex = (HRESULT(__stdcall *)(UINT,IDirect3D9Ex **)) GetProcAddress(hLibD3D9, "Direct3DCreate9Ex");
        dllDirect3DCreate9   = (IDirect3D9*(__stdcall *)(UINT)) GetProcAddress(hLibD3D9, "Direct3DCreate9");
        if(!dllDirect3DCreate9Ex)
		      ShowMessage("Direct3DCreate9Ex not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
        if(!dllDirect3DCreate9)
		      ShowMessage("Direct3DCreate9 not in DLL!\n\n'%s'", path), exit(0);
      }

      /* Load DXGI Library */
      {
        char path[256];
        if(strlen(config.main.dllPathDXGI) < 2)
          sprintf(path, "%s\\system32\\%s", getenv("SystemRoot"), "dxgi.dll");
        else
          strcpy(path, config.main.dllPathDXGI);
        dbg("DXGI DLL Path: <%s>", path);
        hLibDXGI = LoadLibrary(path);
	      if(!hLibDXGI)
		      ShowMessage("DXGI DLL not found!\n'%s'", path), exit(0);

        dllCreateDXGIFactory = (HRESULT(__stdcall *)(REFIID, void**)) GetProcAddress(hLibDXGI, "CreateDXGIFactory");
        dllCreateDXGIFactory1 = (HRESULT(__stdcall *)(REFIID, void**)) GetProcAddress(hLibDXGI, "CreateDXGIFactory1");

        if(!dllCreateDXGIFactory)
		      ShowMessage("CreateDXGIFactory not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
        if(!dllCreateDXGIFactory1)
		      ShowMessage("CreateDXGIFactory1 not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);

        dllDXGID3D10CreateDevice = (HRESULT(__stdcall *)(HMODULE, IDXGIFactory*, IDXGIAdapter*, UINT, DWORD, void**)) GetProcAddress(hLibDXGI, "DXGID3D10CreateDevice");
        if(!dllDXGID3D10CreateDevice)
		      ShowMessage("DXGID3D10CreateDevice not in DLL!\n'%s'", path), exit(0);
      }

      /* Load D3D10 LIbrary */
      {
        char path[256];
        if(strlen(config.main.dllPathD3D10) < 2)
          sprintf(path, "%s\\system32\\%s", getenv("SystemRoot"), "d3d10.dll");
        else
          strcpy(path, config.main.dllPathD3D10);
        dbg("D3D10 DLL Path: <%s>", path);
        hLibD3D10 = LoadLibrary(path);
	      if(!hLibD3D10)
		      ShowMessage("D3D10 DLL not found!\n'%s'", path), exit(0);

        dllD3D10CreateDevice = (HRESULT(__stdcall *)(IDXGIAdapter*,
                                                     D3D10_DRIVER_TYPE,
                                                     HMODULE,
                                                     UINT,
                                                     UINT,
                                                     ID3D10Device**))
                                                     GetProcAddress(hLibD3D10, "D3D10CreateDevice");

        dllD3D10CreateDeviceAndSwapChain = (HRESULT(__stdcall *)(IDXGIAdapter*,
                                                                 D3D10_DRIVER_TYPE,
                                                                 HMODULE,
                                                                 UINT,
                                                                 UINT,
                                                                 DXGI_SWAP_CHAIN_DESC *,
                                                                 IDXGISwapChain **,
                                                                 ID3D10Device **))
                                                                 GetProcAddress(hLibD3D10, "D3D10CreateDeviceAndSwapChain");

        if(!dllD3D10CreateDevice)
		      ShowMessage("D3D10CreateDevice not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
        if(!dllD3D10CreateDeviceAndSwapChain)
		      ShowMessage("D3D10CreateDeviceAndSwapChain not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
      }

      /* Load D3D10.1 Library */
      {
        char path[256];
        if(strlen(config.main.dllPathD3D10_1) < 2)
          sprintf(path, "%s\\system32\\%s", getenv("SystemRoot"), "d3d10_1.dll");
        else
          strcpy(path, config.main.dllPathD3D10_1);
        dbg("D3D10.1 DLL Path: <%s>", path);
        hLibD3D10_1 = LoadLibrary(path);
	      if(!hLibD3D10_1)
		      ShowMessage("D3D10.1 DLL not found!\n'%s'", path), exit(0);

        dllD3D10CreateDevice1 = (HRESULT(__stdcall *)(IDXGIAdapter*,
                                                      D3D10_DRIVER_TYPE,
                                                      HMODULE,
                                                      UINT,
                                                      D3D10_FEATURE_LEVEL1,
                                                      UINT,
                                                      ID3D10Device1**))
                                                      GetProcAddress(hLibD3D10_1, "D3D10CreateDevice1");

        dllD3D10CreateDeviceAndSwapChain1 = (HRESULT(__stdcall *)(IDXGIAdapter*,
                                                                  D3D10_DRIVER_TYPE,
                                                                  HMODULE,
                                                                  UINT,
                                                                  D3D10_FEATURE_LEVEL1,
                                                                  UINT,
                                                                  DXGI_SWAP_CHAIN_DESC *,
                                                                  IDXGISwapChain **,
                                                                  ID3D10Device1 **))
                                                                  GetProcAddress(hLibD3D10_1, "D3D10CreateDeviceAndSwapChain1");

        if(!dllD3D10CreateDevice1)
		      ShowMessage("D3D10CreateDevice1 not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
        if(!dllD3D10CreateDeviceAndSwapChain1)
		      ShowMessage("D3D10CreateDeviceAndSwapChain1 not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
      }

      /* Load D3D11 LIbrary */
      {
        char path[256];
        if(strlen(config.main.dllPathD3D11) < 2)
          sprintf(path, "%s\\system32\\%s", getenv("SystemRoot"), "d3d11.dll");
        else
          strcpy(path, config.main.dllPathD3D11);
        dbg("D3D11 DLL Path: <%s>", path);
        hLibD3D11 = LoadLibrary(path);
	      if(!hLibD3D11)
		      ShowMessage("D3D11 DLL not found!\n'%s'", path), exit(0);

        dllD3D11CreateDevice = (HRESULT(__stdcall *)(IDXGIAdapter *,
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

        if(!dllD3D11CreateDevice)
		      ShowMessage("D3D11CreateDevice not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
        if(!dllD3D11CreateDeviceAndSwapChain)
		      ShowMessage("D3D11CreateDeviceAndSwapChain not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
      }

      if(hooks) {
        if(hLibD3D9) addNoHookModule(hLibD3D9);
        if(hLibDXGI) addNoHookModule(hLibDXGI);
        if(hLibD3D10) addNoHookModule(hLibD3D10);
        if(hLibD3D10_1) addNoHookModule(hLibD3D10_1);
        if(hLibD3D11) addNoHookModule(hLibD3D11);
        // Don't do "addNoHookModule" here for D3D10/11
        // "addNoHookModule" will occur within the D3D10/11 SoftTH DLLs
      }

      break;
    }

    case DLL_PROCESS_DETACH:
    {
      dbg("--- DLL_PROCESS_DETACH (%s) ---", lpReserved?"process terminating":"DLL unloaded");
      if(!lpReserved)
        unsetHooks(SoftTHHooks);
      /*emergencyRelease = true;
      releaseHangingD3D9New();*/

      // Restore any hanging gamma ramps
      std::list<GAMMARAMP*>::iterator i;
      while(!restoreGammaRamps.empty()) {

        i = restoreGammaRamps.begin();
        if(!SetDeviceGammaRamp((*i)->hdc, (void*) &((*i)->ramp)))
          dbg("DLL_PROCESS_DETACH: Restoring gamma ramp failed!");

        ReleaseDC((*i)->hwnd, (*i)->hdc);
        restoreGammaRamps.remove(*i);
      }

			if(hLibD3D9) {
				FreeLibrary(hLibD3D9);
				hLibD3D9 = NULL;
			}
			if(hLibDXGI) {
				FreeLibrary(hLibDXGI);
				hLibDXGI = NULL;
			}
			if(hLibD3D10) {
				FreeLibrary(hLibD3D10);
				hLibD3D10 = NULL;
			}
			if(hLibD3D10_1) {
				FreeLibrary(hLibD3D10_1);
				hLibD3D10_1 = NULL;
			}
			if(hLibD3D11) {
				FreeLibrary(hLibD3D11);
				hLibD3D11 = NULL;
			}
			if(hLibD3D12) {
				FreeLibrary(hLibD3D12);
				hLibD3D12 = NULL;
			}

      if(didDisableComposition) {
        DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
        Sleep(100);
      }
      dbg("end");
      break;
    }

    // Notify mouse hook of thread changes
    case DLL_THREAD_ATTACH:
      ihGlobal.newThread();
      break;
    case DLL_THREAD_DETACH:
      ihGlobal.detachThread();
      break;
  }
  return TRUE;
}

#if DUMP_IMPORTS
#pragma comment(lib, "Dbghelp.lib")
#endif

#if !DXGI
IDirect3D9Ex *riil;

extern "C" _declspec(dllexport) IDirect3D9 * __stdcall Direct3DCreate9(UINT SDKVersion)
{
  dbg("Direct3DCreate9");

  if(!dllDirect3DCreate9Ex)
    return NULL;

#if DUMP_IMPORTS
  dumpAllImports();
  exit(0);
#endif

  #if USE_D3DEX
  // Create D3D9Ex interface
  IDirect3D9Ex *d3dhReal;
  dllDirect3DCreate9Ex(D3D_SDK_VERSION, &d3dhReal);
  #else
  IDirect3D9 *d3dhReal;
  d3dhReal = dllDirect3DCreate9(D3D_SDK_VERSION);
  #endif

  d3dhNew = new IDirect3D9New((IDirect3D9Ex*)d3dhReal);
  return (IDirect3D9*) d3dhNew;
}

extern "C" _declspec(dllexport) HRESULT __stdcall Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ptr)
{
  if(SDKVersion == -D3D_SDK_VERSION) {
    // Called from SoftTH, do what we're supposed to be doing
    return dllDirect3DCreate9Ex(D3D_SDK_VERSION, ptr);
  }
  dbg("Direct3DCreate9Ex");
  IDirect3D9Ex *d3dhReal;

  #if USE_D3DEX
    // Create D3D9Ex interface
    dllDirect3DCreate9Ex(D3D_SDK_VERSION, &d3dhReal);
  #else
    dbg("Not so.");
  #endif

  d3dhNew = new IDirect3D9New(d3dhReal);
  *ptr = (IDirect3D9Ex*) d3dhNew;
  return S_OK;
}
#endif

extern "C" _declspec(dllexport) HRESULT WINAPI newCreateDXGIFactory(REFIID riid, void **ppFactory)
{
  dbg("CreateDXGIFactory");

  //HRESULT ret = dllCreateDXGIFactory(riid, ppFactory);
  HRESULT ret = dllCreateDXGIFactory1(riid, ppFactory);
  dbg("CreateDXGIFactory 0x%08X", *ppFactory);
  if(ret == S_OK) {
    IDXGIFactory1 *dxgifNew = (IDXGIFactory1 *) *ppFactory;
    *ppFactory = new IDXGIFactory1New(dxgifNew);
  } else
    dbg("CreateDXGIFactory failed!");
  return ret;
}

extern "C" _declspec(dllexport) HRESULT WINAPI newCreateDXGIFactory1(REFIID riid, void **ppFactory)
{
  dbg("CreateDXGIFactory1");

  //HRESULT ret = dllCreateDXGIFactory(riid, ppFactory);
  HRESULT ret = dllCreateDXGIFactory1(riid, ppFactory);
  dbg("CreateDXGIFactory1 0x%08X", *ppFactory);
  if(ret == S_OK) {
    IDXGIFactory1 *dxgifNew = (IDXGIFactory1 *) *ppFactory;
    *ppFactory = new IDXGIFactory1New(dxgifNew);
  } else
    dbg("CreateDXGIFactory1 failed!");
  return ret;
}

extern "C" _declspec(dllexport) HRESULT WINAPI DXGID3D10CreateDevice(HMODULE d3d10core, IDXGIFactory *factory, IDXGIAdapter *adapter, UINT flags, DWORD unknown0, void **device)
{
  dbg("DXGID3D10CreateDevice 0x%08X 0x%08X", adapter, *adapter);

  IDXGIFactory1New *fnew;
  if(factory->QueryInterface(IID_IDXGIFactory1New, (void**) &fnew) == S_OK) {
    factory = fnew->getReal();
    fnew->Release();
  }
  IDXGIAdapter1New *anew;
  if(adapter->QueryInterface(IID_IDXGIAdapter1New, (void**) &anew) == S_OK) {
    adapter = anew->getReal();
    anew->Release();
  }

  return dllDXGID3D10CreateDevice(d3d10core, factory, adapter, flags, unknown0, device);
}

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
  dbg("DEXPORT: %s - %s", #hLib, #x);\
	static FARPROC foo = GetProcAddress((HMODULE) hLib, #x);\
	__asm {pop STACK_BASE};\
	__asm {jmp foo};\
	}

extern "C" {
// Direct3D 9
DEXPORT(hLibD3D9, PSGPSampleTexture             );
DEXPORT(hLibD3D9, CheckFullscreen               );
DEXPORT(hLibD3D9, D3DPERF_BeginEvent            );
DEXPORT(hLibD3D9, D3DPERF_EndEvent              );
DEXPORT(hLibD3D9, D3DPERF_SetMarker             );
DEXPORT(hLibD3D9, D3DPERF_SetRegion             );
DEXPORT(hLibD3D9, D3DPERF_QueryRepeatFrame      );
DEXPORT(hLibD3D9, D3DPERF_SetOptions            );
DEXPORT(hLibD3D9, D3DPERF_GetStatus             );
DEXPORT(hLibD3D9, DebugSetMute                  );
DEXPORT(hLibD3D9, DebugSetLevel                 );
DEXPORT(hLibD3D9, Direct3DShaderValidatorCreate9);
DEXPORT(hLibD3D9, PSGPError                     );
DEXPORT(hLibD3D9, ValidatePixelShader           );
DEXPORT(hLibD3D9, ValidateVertexShader          );
//DEXPORT(hLibD3D9, Direct3DCreate9             );

// Direct3D 9 Ex
DEXPORT(hLibD3D9, LoadDebugRuntime          );
//DEXPORT(hLibD3D9, Direct3DCreate9Ex       );

// DXGI
//DEXPORTD(hLibDXGI, CreateDXGIFactory);
//DEXPORTD(hLibDXGI, CreateDXGIFactory1);
DEXPORTD(hLibDXGI, D3DKMTGetDeviceState);
DEXPORTD(hLibDXGI, D3DKMTOpenAdapterFromHdc);
DEXPORTD(hLibDXGI, D3DKMTQueryAdapterInfo);
DEXPORTD(hLibDXGI, D3DKMTWaitForVerticalBlankEvent);
//DEXPORTD(hLibDXGI, DXGID3D10CreateDevice);
DEXPORTD(hLibDXGI, DXGID3D10CreateLayeredDevice);
DEXPORTD(hLibDXGI, DXGID3D10GetLayeredDeviceSize);
DEXPORTD(hLibDXGI, DXGID3D10RegisterLayers);
DEXPORTD(hLibDXGI, DXGIDumpJournal);
DEXPORTD(hLibDXGI, DXGIReportAdapterConfiguration);
DEXPORTD(hLibDXGI, OpenAdapter10);
DEXPORTD(hLibDXGI, OpenAdapter10_2);

// D3D 10


// D3D 10.1


// D3D 11


}
#pragma warning (default : 4731)

#endif
