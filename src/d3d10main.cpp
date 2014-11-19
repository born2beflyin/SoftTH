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
#include <d3d10.h>
#include "d3d10main.h"
#include "configFile.h"
#include "globalHooker.h"
#include "hooksSoftTH.h"


/* Declare external function, or include the cpp file in the project */
// dbg
extern "C" __declspec(dllimport) void dbg(char*, ...);
// ShowMessage
extern "C" __declspec(dllimport) void ShowMessage(char*, ...);
// addNoHookModule
// configFile::load
// configFile::configFile


// D3D 10 Lib
HINSTANCE hLibD3D10 = NULL; // Real d3d10.dll


/* D3D 10 */
HRESULT (WINAPI*dllD3D10CreateDevice)(IDXGIAdapter *adapter, D3D10_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, UINT SDKVersion, ID3D10Device** ppDevice) = NULL;
HRESULT (WINAPI*dllD3D10CreateDeviceAndSwapChain)(IDXGIAdapter *adapter, D3D10_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, UINT SDKVersion, DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, IDXGISwapChain **ppSwapChain, ID3D10Device **ppDevice) = NULL;


configFile config; // Main configuration


BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD reason, LPVOID lpReserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
    {
      // Load configuration settings
      config.load();

      bool hooks = true;

      // Load d3d10 library
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

        dllD3D10CreateDevice = (HRESULT(__stdcall *)(IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, ID3D10Device**)) GetProcAddress(hLibD3D10, "D3D10CreateDevice");
        dllD3D10CreateDeviceAndSwapChain = (HRESULT(__stdcall *)(IDXGIAdapter*, D3D10_DRIVER_TYPE, HMODULE, UINT, UINT, DXGI_SWAP_CHAIN_DESC *, IDXGISwapChain **, ID3D10Device **)) GetProcAddress(hLibD3D10, "D3D10CreateDeviceAndSwapChain");

        if(!dllD3D10CreateDevice)
		      ShowMessage("D3D10CreateDevice not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
        if(!dllD3D10CreateDeviceAndSwapChain)
		      ShowMessage("D3D10CreateDeviceAndSwapChain not in DLL!\nWindows 7 or newer required!\n'%s'", path), exit(0);
      }

      if(hooks) {
        if(hLibD3D10) addNoHookModule(hLibD3D10);
      }

      break;
    }

    case DLL_PROCESS_DETACH:
    {
			if(hLibD3D10) {
				FreeLibrary(hLibD3D10);
				hLibD3D10 = NULL;
			}

      break;
    }
  }
  return true;
}

/*    Direct3D 10     */
//D3D10CreateDevice
extern "C" _declspec(dllexport) HRESULT WINAPI newD3D10CreateDevice(IDXGIAdapter *adapter, D3D10_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, UINT SDKVersion, ID3D10Device** ppDevice)
{
  dbg("D3D10CreateDevice 0x%08X 0x%08X", adapter, *adapter);

  HRESULT ret = dllD3D10CreateDevice(adapter, DriverType, Software, Flags, SDKVersion, ppDevice);

  /*IDXGIFactory1New *fnew;
  if(factory->QueryInterface(IID_IDXGIFactory1New, (void**) &fnew) == S_OK) {
    factory = fnew->getReal();
    fnew->Release();
  }
  IDXGIAdapter1New *anew;
  if(adapter->QueryInterface(IID_IDXGIAdapter1New, (void**) &anew) == S_OK) {
    adapter = anew->getReal();
    anew->Release();
  }*/

  return ret;
}

//D3D10CreateDeviceAndSwapChain
extern "C" _declspec(dllexport) HRESULT WINAPI newD3D10CreateDeviceAndSwapChain(IDXGIAdapter *adapter, D3D10_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, UINT SDKVersion, DXGI_SWAP_CHAIN_DESC *pSwapChainDesc, IDXGISwapChain **ppSwapChain, ID3D10Device **ppDevice)
{
  dbg("D3D10CreateDeviceAndSwapChain 0x%08X 0x%08X", adapter, *adapter);

  HRESULT ret = dllD3D10CreateDeviceAndSwapChain(adapter, DriverType, Software, Flags, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice);

  /*IDXGIFactory1New *fnew;
  if(factory->QueryInterface(IID_IDXGIFactory1New, (void**) &fnew) == S_OK) {
    factory = fnew->getReal();
    fnew->Release();
  }
  IDXGIAdapter1New *anew;
  if(adapter->QueryInterface(IID_IDXGIAdapter1New, (void**) &anew) == S_OK) {
    adapter = anew->getReal();
    anew->Release();
  }*/

  return ret;
}
