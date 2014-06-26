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
//  Ugly stuff that makes our life easier is hidden here
//

#include "version.h"
#include "helper.h"

#include "main.h"
#include <windows.h>
#include <stdio.h>
#include "d3d9.h"

#include "Shlobj.h"

//#define WINDOW_FLAGS    WS_EX_TOPMOST
#define WINDOW_FLAGS    NULL


// Show message box
void ShowMessage(char *first, ...) {
  va_list     argptr;
  char        Message[512];

  va_start (argptr,first);
  vsprintf (Message,first,argptr);
  va_end   (argptr);

  dbg("ERROR: %s", Message);
/*
  // Minimize game window
  if(config.focusWindow)
  ShowWindow(config.focusWindow, SW_MINIMIZE);
*/
  MessageBox(NULL, Message,"SoftTH message window",MB_OK|MB_ICONSTOP|MB_SETFOREGROUND|MB_TASKMODAL);
}

#define BL_LINES 64

static char *backLog[BL_LINES];

void backLogAdd(char *str)
{
  static bool init = false;
  if(!init) {
    for(int i=0;i<BL_LINES;i++)
      backLog[i] = NULL;
    init = true;
  }

  delete backLog[BL_LINES-1];
  for(int i=BL_LINES-1;i>=0;i--)
    backLog[i+1] = backLog[i];

  int len = (int) strlen(str);
  backLog[0] = new char[len+1];
  strcpy(backLog[0], str);
}

char* getBackLogLine(int line)
{
  if(line >= BL_LINES || !backLog[line])
    return NULL;
  return backLog[line];
}

void dbgSimple(char *first, ...) {
	va_list     argptr;
	static char        Message[1024];
	strcpy(Message, DEBUGPREFIX);

	va_start (argptr, first);
	vsprintf (Message+strlen(DEBUGPREFIX), first, argptr);
	va_end   (argptr);

  OutputDebugString(Message);
}

extern int startTime;
// Output debug string
void dbg(char *first, ...) {
	static char lastMsg[1024] = "";
	static int repeatCount = 0;
	static char logPath[1024] = "";
	static int init = 1;
	static CRITICAL_SECTION cs;

//#define SIMPLE_DBG
#ifdef SIMPLE_DBG
  {
	  va_list     argptr;
	  static char        Message[1024];
	  strcpy(Message, DEBUGPREFIX);

	  va_start (argptr, first);
	  vsprintf (Message+strlen(DEBUGPREFIX), first, argptr);
    OutputDebugString(Message);
	  va_end   (argptr);
    return;
  }
#endif

	if(init) {
    /*
		int len = GetModuleFileName(GetModuleHandle(NULL), logPath, 1024);
		for(int i=len;i>0;i--)
			if(logPath[i] == '\\' || logPath[i] == '/') {
				logPath[i] = 0x00;
				break;
			}
		strcat(logPath, "\\SoftTH.log");
    */

    char mydocs[256] = "";
    SHGetFolderPath(0, CSIDL_PERSONAL, NULL, NULL, mydocs);
    sprintf(logPath, "%s/SoftTH/SoftTH.log", mydocs);

		init = 0;
		InitializeCriticalSection(&cs);
	}


	//if(config.debugOutput) {
	if(true) {
		EnterCriticalSection(&cs);

		va_list     argptr;
		static char        Message[1024];
		strcpy(Message, DEBUGPREFIX);

		va_start (argptr, first);
		vsprintf (Message+strlen(DEBUGPREFIX), first, argptr);
		va_end   (argptr);


		if(!strncmp(Message+strlen(DEBUGPREFIX), CLEAR_LOG, strlen(CLEAR_LOG))) {
			// Special message to clear log at init
			FILE *f = fopen(logPath, "w");
			if(f) {
				fclose(f);
				f = NULL;
			}
			LeaveCriticalSection(&cs);
			return;
		}


		char temp[2048];
    sprintf(temp, "<%s:%d> %s", processName(), GetCurrentThreadId(), Message);
		OutputDebugString(temp);
    //backLogAdd(temp);
		//OutputDebugString(Message);
/*
#ifdef DEBUG_TIMESTAMPED
		static char	tmp[1024];
		static int	initTime = GetTickCount();
		sprintf(tmp, "%8.3f %s", (GetTickCount()-initTime)/1000.0f, Message);
		strcpy(Message, tmp);
#endif
*/
    static int	initTime = GetTickCount();

    if(true) {
			if(!strncmp(lastMsg, Message, 1024)) {
				// Same message repeating
				repeatCount++;
			} else {
				/*
				// Clear log for fist time
				static FILE *f = fopen(logPath, "w");
				if(f) {
					OutputDebugString("CLEAR LOG");
					fclose(f);
					f = NULL;
				}*/

				// Append to file
				FILE *ff = fopen(logPath, "a");
				if(!ff) {
					OutputDebugString("Cannot access logfile!");
					LeaveCriticalSection(&cs);
					return;
				}

        char t[64];
        sprintf(t, "%d:%8.3f ", GetCurrentThreadId(), (GetTickCount()-initTime)/1000.0f);

				if(repeatCount) {
					char tmp[1024];
          if(repeatCount == 1) {
						sprintf(tmp, "%s", lastMsg);
          } else
						sprintf(tmp, " <Last message repeated %d times>", repeatCount+1);
          fwrite(t, 1, strlen(t), ff);
					fwrite(tmp, 1, strlen(tmp), ff);
					fputc(0x0A, ff);
				}

        fwrite(t, 1, strlen(t), ff);
				fwrite(Message, 1, strlen(Message), ff);
				fputc(0x0A, ff);
				fclose(ff);
				repeatCount = 0;
			}

			strncpy(lastMsg, Message, 1024);
		}

		LeaveCriticalSection(&cs);
//		DeleteCriticalSection(&cs);
	}
}

#include <d3d11.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <dxgi.h>

#include "dxgiSwapChain.h"
#include "dxgiFactory.h"
#include "dxgiAdapterOutput.h"

char* matchRiid(REFIID riid)
{
  if(riid == IID_IDirect3DDevice9Ex) return "IID_IDirect3DDevice9Ex";
  if(riid == IID_IDirect3DDevice9) return "IID_IDirect3DDevice9";

  if(riid == IID_IDXGIObject) return "IID_IDXGIObject";
  if(riid == IID_IDXGIFactory) return "IID_IDXGIFactory";
  if(riid == IID_IDXGIFactory1) return "IID_IDXGIFactory1";
  if(riid == IID_IDXGISwapChain) return "IID_IDXGISwapChain";
  if(riid == IID_IDXGIAdapter) return "IID_IDXGIAdapter";
  if(riid == IID_IDXGIAdapter1) return "IID_IDXGIAdapter1";
  if(riid == IID_IDXGIOutput) return "IID_IDXGIOutput";
  if(riid == IID_IDXGIDevice) return "IID_IDXGIDevice";
  if(riid == IID_IDXGIDevice1) return "IID_IDXGIDevice1";
  if(riid == IID_IDXGIDeviceSubObject) return "IID_IDXGIDeviceSubObject";
  if(riid == IID_IDXGIKeyedMutex) return "IID_IDXGIKeyedMutex";
  if(riid == IID_IDXGIResource) return "IID_IDXGIResource";
  if(riid == IID_IDXGISurface) return "IID_IDXGISurface";
  if(riid == IID_IDXGISurface1) return "IID_IDXGISurface1";

  if(riid == IID_ID3D10Texture1D) return "IID_ID3D10Texture2D";
  if(riid == IID_ID3D10Texture2D) return "IID_ID3D10Texture2D";
  if(riid == IID_ID3D10Texture3D) return "IID_ID3D10Texture2D";
  if(riid == IID_ID3D10Device) return "IID_ID3D10Device";
  if(riid == IID_ID3D10Device1) return "IID_ID3D10Device1";
  if(riid == IID_ID3D11Device) return "IID_ID3D11Device";
  if(riid == IID_ID3D11DeviceContext) return "IID_ID3D11DeviceContext";

  if(riid == IID_IDXGISwapChainNew) return "IID_IDXGISwapChainNew";
  if(riid == IID_IDXGIFactory1New) return "IID_IDXGIFactory1New";
  if(riid == IID_IDXGIAdapter1New) return "IID_IDXGIAdapter1New";
  if(riid == IID_IDXGIOutputNew) return "IID_IDXGIOutputNew";

  if(riid == IID_IDirect3DBaseTexture9) return "IID_IDirect3DBaseTexture9";
  if(riid == IID_IDirect3DTexture9) return "IID_IDirect3DTexture9";

  static char foo[256];
  sprintf(foo, "Unknown: {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", riid.Data1, riid.Data2, riid.Data3, riid.Data4[0],
                riid.Data4[1], riid.Data4[2], riid.Data4[3], riid.Data4[4], riid.Data4[5], riid.Data4[6], riid.Data4[7]);
  return foo;
}

// Dump D3D surface description
void dumpSurfaceDesc(LPDIRECT3DSURFACE9 surf) {
	dbg("dumpSurfaceDesc Surface %08X", surf);
  if(!surf)
    return;

	D3DSURFACE_DESC desc;
	surf->GetDesc(&desc);

	dbg("Size %dx%d", desc.Width, desc.Height);
	dbg("Format %s %08X", getMode(desc.Format), desc.Format);
	dbg("Type %08X", desc.Type);
	dbg("Usage %s %08X", getUsage(desc.Usage), desc.Usage);
	dbg("Pool %s %08X", getPool(desc.Pool), desc.Pool);
	dbg("MultiSampleType %08X", desc.MultiSampleType);
	dbg("MultiSampleQuality %08X", desc.MultiSampleQuality);
}

void dumpPP(D3DPRESENT_PARAMETERS* pp) {
	dbg("-- Begin Present parameters dump:");
	dbg("res: %dx%d %dHz, %s, count %d", pp->BackBufferWidth, pp->BackBufferHeight, pp->FullScreen_RefreshRateInHz, getMode(pp->BackBufferFormat), pp->BackBufferCount);
	dbg("MultiSampleType: %X, MultiSampleQuality: %X", pp->MultiSampleType, pp->MultiSampleQuality);
	dbg("Windowed: %X", pp->Windowed);

	char swapEffect[128];
	switch(pp->SwapEffect) {
		case D3DSWAPEFFECT_COPY:strcpy(swapEffect, "D3DSWAPEFFECT_COPY");break;
		case D3DSWAPEFFECT_DISCARD:strcpy(swapEffect, "D3DSWAPEFFECT_DISCARD");break;
		case D3DSWAPEFFECT_FLIP:strcpy(swapEffect, "D3DSWAPEFFECT_FLIP");break;
		default: sprintf(swapEffect, "Unknown (%d)", pp->SwapEffect);break;
	}

	char presentInterval[128];
	switch(pp->PresentationInterval) {
		case D3DPRESENT_INTERVAL_DEFAULT:strcpy(presentInterval, "D3DPRESENT_INTERVAL_DEFAULT");break;
		case D3DPRESENT_INTERVAL_ONE:strcpy(presentInterval, "D3DPRESENT_INTERVAL_ONE");break;
		case D3DPRESENT_INTERVAL_TWO:strcpy(presentInterval, "D3DPRESENT_INTERVAL_TWO");break;
		case D3DPRESENT_INTERVAL_THREE:strcpy(presentInterval, "D3DPRESENT_INTERVAL_THREE");break;
		case D3DPRESENT_INTERVAL_FOUR:strcpy(presentInterval, "D3DPRESENT_INTERVAL_FOUR");break;
		case D3DPRESENT_INTERVAL_IMMEDIATE:strcpy(presentInterval, "D3DPRESENT_INTERVAL_IMMEDIATE");break;
		default: sprintf(presentInterval, "Unknown (%d)", pp->PresentationInterval);break;
	}
	dbg("SwapEffect: %s, Flags: %08X, PresentationInterval: %s", swapEffect, pp->Flags, presentInterval);
	dbg("SwapEffect: %s, Flags: %08X", swapEffect, pp->Flags);

	char winName[512];
	winName[0] = 0x00;
	GetWindowText(pp->hDeviceWindow, winName, 512);
	dbg("Window: %s (%s)", winName, pp->Windowed?"Window":"Fullscreen");

	dbg("Depth/Stencil: %s (f%X)", pp->EnableAutoDepthStencil?"Enabled":"Disabled", pp->AutoDepthStencilFormat);
	dbg("-- End Present parameters dump");
}

// Return d3d device reference count
/*int getD3DRefCount() {
	if(!d3dd)
		return 0;
	d3dd->AddRef();
	return d3dd->Release();
}*/

int getRef(IUnknown *o) {
  if(!o)
    return -1;
  o->AddRef();
  return o->Release();
}

char* getModuleName(HMODULE mod) {
  static char txt[256];
  GetModuleFileName(mod, txt, 256);
  return txt;
}

// Output matrix to debug
void DebugPrintMatrix(D3DMATRIX mat) {
	dbg("Matrix: %i:", mat);

	dbg("        %.4f, %.4f, %.4f, %.4f", mat._11, mat._12, mat._13, mat._14);
	dbg("        %.4f, %.4f, %.4f, %.4f", mat._21, mat._22, mat._23, mat._24);
	dbg("        %.4f, %.4f, %.4f, %.4f", mat._31, mat._32, mat._33, mat._34);
	dbg("        %.4f, %.4f, %.4f, %.4f", mat._41, mat._42, mat._43, mat._44);
}

// Find next n^2 value
int findNextPowerTwo(int v) {
	int o = 1;
	while(o < v)
		o = o*2;
	return o;
}

// Create all subdirs in filename
void createDirs(char *fname) {
	char *fn = new char[strlen(fname)+1];
	strcpy(fn, fname);

	for(DWORD i=0;i<strlen(fn);i++)
		if(fn[i] == '/' || fn[i] == '\\') {
			fn[i] = 0x00;
			CreateDirectory(fn, NULL);
			fn[i] = '/';
		}

	delete[] fn;
}


bool fileExists(char *name)
{
  FILE *f = fopen(name, "rb");
  if(f) {
    fclose(f);
    return true;
  }
  return false;
}

// Return current process name
char* processName(void) {
	static char name[256] = {NULL};
	if(!name[0]) {
		// Retrieve process name
		char tmp[512];
		GetModuleFileName(GetModuleHandle(NULL), tmp, 512);

		// Strip path
		char *fname = strrchr(tmp, '\\');
		if(fname)
			strcpy(name, fname+1);
		else
			strcpy(name, tmp);
	}
	return name;
}

void timeWarn(int start, int limit, char *type)
{
  int t = GetTickCount()-start;
  if(t >= limit)
    dbg("Task <%s> was slow (%dms)", type, t);
}

void d3dClearTarget(IDirect3DDevice9Ex *dev, IDirect3DSurface9 *tgt, IDirect3DSurface9 *tgtD)
{
  IDirect3DSurface9 *lastbb = NULL;
  IDirect3DSurface9 *lastdepth = NULL;
  D3DCALL( dev->GetRenderTarget(0, &lastbb) );

  if(tgtD)
  {
    D3DCALL( dev->GetDepthStencilSurface(&lastdepth) );
    D3DCALL( dev->SetDepthStencilSurface(tgtD) );
  }

  D3DCALL( dev->SetRenderTarget(0, tgt) );
  D3DCALL( dev->Clear(0, 0, D3DCLEAR_TARGET|(tgtD?D3DCLEAR_ZBUFFER:0)|(tgtD?D3DCLEAR_STENCIL:0), 0, 0, 0) );

  if(tgtD)
  {
    D3DCALL( dev->SetDepthStencilSurface(lastdepth) );
  }
  D3DCALL( dev->SetRenderTarget(0, lastbb) );
  lastbb->Release();
}

char* getUsageDXGI(DXGI_USAGE usage)
{
  if(!usage)
    return "NULL";
  static char ret[512];
  ret[0] = 0x00;
  DWORD u = usage;
  if(u & DXGI_USAGE_BACK_BUFFER) strcat(ret, "BACK_BUFFER "), u-=DXGI_USAGE_BACK_BUFFER;
  if(u & DXGI_USAGE_DISCARD_ON_PRESENT) strcat(ret, "DISCARD_ON_PRESENT "), u-=DXGI_USAGE_DISCARD_ON_PRESENT;
  if(u & DXGI_USAGE_READ_ONLY) strcat(ret, "READ_ONLY "), u-=DXGI_USAGE_READ_ONLY;
  if(u & DXGI_USAGE_RENDER_TARGET_OUTPUT) strcat(ret, "RENDER_TARGET_OUTPUT "), u-=DXGI_USAGE_RENDER_TARGET_OUTPUT;
  if(u & DXGI_USAGE_SHADER_INPUT) strcat(ret, "SHADER_INPUT "), u-=DXGI_USAGE_SHADER_INPUT;
  if(u & DXGI_USAGE_SHARED) strcat(ret, "SHARED "), u-=DXGI_USAGE_SHARED;
  if(u & DXGI_USAGE_UNORDERED_ACCESS) strcat(ret, "UNORDERED_ACCESS "), u-=DXGI_USAGE_UNORDERED_ACCESS;
  return ret;
}

char* getFormatDXGI(DXGI_FORMAT fmt)
{
  switch(fmt) {
	case DXGI_FORMAT_UNKNOWN: return "DXGI_FORMAT_UNKNOWN";
	case DXGI_FORMAT_R32G32B32A32_TYPELESS: return "DXGI_FORMAT_R32G32B32A32_TYPELESS";
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return "DXGI_FORMAT_R32G32B32A32_FLOAT";
	case DXGI_FORMAT_R32G32B32A32_UINT: return "DXGI_FORMAT_R32G32B32A32_UINT";
	case DXGI_FORMAT_R32G32B32A32_SINT: return "DXGI_FORMAT_R32G32B32A32_SINT";
	case DXGI_FORMAT_R32G32B32_TYPELESS: return "DXGI_FORMAT_R32G32B32_TYPELESS";
	case DXGI_FORMAT_R32G32B32_FLOAT: return "DXGI_FORMAT_R32G32B32_FLOAT";
	case DXGI_FORMAT_R32G32B32_UINT: return "DXGI_FORMAT_R32G32B32_UINT";
	case DXGI_FORMAT_R32G32B32_SINT: return "DXGI_FORMAT_R32G32B32_SINT";
	case DXGI_FORMAT_R16G16B16A16_TYPELESS: return "DXGI_FORMAT_R16G16B16A16_TYPELESS";
	case DXGI_FORMAT_R16G16B16A16_FLOAT: return "DXGI_FORMAT_R16G16B16A16_FLOAT";
	case DXGI_FORMAT_R16G16B16A16_UNORM: return "DXGI_FORMAT_R16G16B16A16_UNORM";
	case DXGI_FORMAT_R16G16B16A16_UINT: return "DXGI_FORMAT_R16G16B16A16_UINT";
	case DXGI_FORMAT_R16G16B16A16_SNORM: return "DXGI_FORMAT_R16G16B16A16_SNORM";
	case DXGI_FORMAT_R16G16B16A16_SINT: return "DXGI_FORMAT_R16G16B16A16_SINT";
	case DXGI_FORMAT_R32G32_TYPELESS: return "DXGI_FORMAT_R32G32_TYPELESS";
	case DXGI_FORMAT_R32G32_FLOAT: return "DXGI_FORMAT_R32G32_FLOAT";
	case DXGI_FORMAT_R32G32_UINT: return "DXGI_FORMAT_R32G32_UINT";
	case DXGI_FORMAT_R32G32_SINT: return "DXGI_FORMAT_R32G32_SINT";
	case DXGI_FORMAT_R32G8X24_TYPELESS: return "DXGI_FORMAT_R32G8X24_TYPELESS";
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return "DXGI_FORMAT_D32_FLOAT_S8X24_UINT";
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS";
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT";
	case DXGI_FORMAT_R10G10B10A2_TYPELESS: return "DXGI_FORMAT_R10G10B10A2_TYPELESS";
	case DXGI_FORMAT_R10G10B10A2_UNORM: return "DXGI_FORMAT_R10G10B10A2_UNORM";
	case DXGI_FORMAT_R10G10B10A2_UINT: return "DXGI_FORMAT_R10G10B10A2_UINT";
	case DXGI_FORMAT_R11G11B10_FLOAT: return "DXGI_FORMAT_R11G11B10_FLOAT";
	case DXGI_FORMAT_R8G8B8A8_TYPELESS: return "DXGI_FORMAT_R8G8B8A8_TYPELESS";
	case DXGI_FORMAT_R8G8B8A8_UNORM: return "DXGI_FORMAT_R8G8B8A8_UNORM";
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB";
	case DXGI_FORMAT_R8G8B8A8_UINT: return "DXGI_FORMAT_R8G8B8A8_UINT";
	case DXGI_FORMAT_R8G8B8A8_SNORM: return "DXGI_FORMAT_R8G8B8A8_SNORM";
	case DXGI_FORMAT_R8G8B8A8_SINT: return "DXGI_FORMAT_R8G8B8A8_SINT";
	case DXGI_FORMAT_R16G16_TYPELESS: return "DXGI_FORMAT_R16G16_TYPELESS";
	case DXGI_FORMAT_R16G16_FLOAT: return "DXGI_FORMAT_R16G16_FLOAT";
	case DXGI_FORMAT_R16G16_UNORM: return "DXGI_FORMAT_R16G16_UNORM";
	case DXGI_FORMAT_R16G16_UINT: return "DXGI_FORMAT_R16G16_UINT";
	case DXGI_FORMAT_R16G16_SNORM: return "DXGI_FORMAT_R16G16_SNORM";
	case DXGI_FORMAT_R16G16_SINT: return "DXGI_FORMAT_R16G16_SINT";
	case DXGI_FORMAT_R32_TYPELESS: return "DXGI_FORMAT_R32_TYPELESS";
	case DXGI_FORMAT_D32_FLOAT: return "DXGI_FORMAT_D32_FLOAT";
	case DXGI_FORMAT_R32_FLOAT: return "DXGI_FORMAT_R32_FLOAT";
	case DXGI_FORMAT_R32_UINT: return "DXGI_FORMAT_R32_UINT";
	case DXGI_FORMAT_R32_SINT: return "DXGI_FORMAT_R32_SINT";
	case DXGI_FORMAT_R24G8_TYPELESS: return "DXGI_FORMAT_R24G8_TYPELESS";
	case DXGI_FORMAT_D24_UNORM_S8_UINT: return "DXGI_FORMAT_D24_UNORM_S8_UINT";
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return "DXGI_FORMAT_R24_UNORM_X8_TYPELESS";
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return "DXGI_FORMAT_X24_TYPELESS_G8_UINT";
	case DXGI_FORMAT_R8G8_TYPELESS: return "DXGI_FORMAT_R8G8_TYPELESS";
	case DXGI_FORMAT_R8G8_UNORM: return "DXGI_FORMAT_R8G8_UNORM";
	case DXGI_FORMAT_R8G8_UINT: return "DXGI_FORMAT_R8G8_UINT";
	case DXGI_FORMAT_R8G8_SNORM: return "DXGI_FORMAT_R8G8_SNORM";
	case DXGI_FORMAT_R8G8_SINT: return "DXGI_FORMAT_R8G8_SINT";
	case DXGI_FORMAT_R16_TYPELESS: return "DXGI_FORMAT_R16_TYPELESS";
	case DXGI_FORMAT_R16_FLOAT: return "DXGI_FORMAT_R16_FLOAT";
	case DXGI_FORMAT_D16_UNORM: return "DXGI_FORMAT_D16_UNORM";
	case DXGI_FORMAT_R16_UNORM: return "DXGI_FORMAT_R16_UNORM";
	case DXGI_FORMAT_R16_UINT: return "DXGI_FORMAT_R16_UINT";
	case DXGI_FORMAT_R16_SNORM: return "DXGI_FORMAT_R16_SNORM";
	case DXGI_FORMAT_R16_SINT: return "DXGI_FORMAT_R16_SINT";
	case DXGI_FORMAT_R8_TYPELESS: return "DXGI_FORMAT_R8_TYPELESS";
	case DXGI_FORMAT_R8_UNORM: return "DXGI_FORMAT_R8_UNORM";
	case DXGI_FORMAT_R8_UINT: return "DXGI_FORMAT_R8_UINT";
	case DXGI_FORMAT_R8_SNORM: return "DXGI_FORMAT_R8_SNORM";
	case DXGI_FORMAT_R8_SINT: return "DXGI_FORMAT_R8_SINT";
	case DXGI_FORMAT_A8_UNORM: return "DXGI_FORMAT_A8_UNORM";
	case DXGI_FORMAT_R1_UNORM: return "DXGI_FORMAT_R1_UNORM";
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return "DXGI_FORMAT_R9G9B9E5_SHAREDEXP";
	case DXGI_FORMAT_R8G8_B8G8_UNORM: return "DXGI_FORMAT_R8G8_B8G8_UNORM";
	case DXGI_FORMAT_G8R8_G8B8_UNORM: return "DXGI_FORMAT_G8R8_G8B8_UNORM";
	case DXGI_FORMAT_BC1_TYPELESS: return "DXGI_FORMAT_BC1_TYPELESS";
	case DXGI_FORMAT_BC1_UNORM: return "DXGI_FORMAT_BC1_UNORM";
	case DXGI_FORMAT_BC1_UNORM_SRGB: return "DXGI_FORMAT_BC1_UNORM_SRGB";
	case DXGI_FORMAT_BC2_TYPELESS: return "DXGI_FORMAT_BC2_TYPELESS";
	case DXGI_FORMAT_BC2_UNORM: return "DXGI_FORMAT_BC2_UNORM";
	case DXGI_FORMAT_BC2_UNORM_SRGB: return "DXGI_FORMAT_BC2_UNORM_SRGB";
	case DXGI_FORMAT_BC3_TYPELESS: return "DXGI_FORMAT_BC3_TYPELESS";
	case DXGI_FORMAT_BC3_UNORM: return "DXGI_FORMAT_BC3_UNORM";
	case DXGI_FORMAT_BC3_UNORM_SRGB: return "DXGI_FORMAT_BC3_UNORM_SRGB";
	case DXGI_FORMAT_BC4_TYPELESS: return "DXGI_FORMAT_BC4_TYPELESS";
	case DXGI_FORMAT_BC4_UNORM: return "DXGI_FORMAT_BC4_UNORM";
	case DXGI_FORMAT_BC4_SNORM: return "DXGI_FORMAT_BC4_SNORM";
	case DXGI_FORMAT_BC5_TYPELESS: return "DXGI_FORMAT_BC5_TYPELESS";
	case DXGI_FORMAT_BC5_UNORM: return "DXGI_FORMAT_BC5_UNORM";
	case DXGI_FORMAT_BC5_SNORM: return "DXGI_FORMAT_BC5_SNORM";
	case DXGI_FORMAT_B5G6R5_UNORM: return "DXGI_FORMAT_B5G6R5_UNORM";
	case DXGI_FORMAT_B5G5R5A1_UNORM: return "DXGI_FORMAT_B5G5R5A1_UNORM";
	case DXGI_FORMAT_B8G8R8A8_UNORM: return "DXGI_FORMAT_B8G8R8A8_UNORM";
	case DXGI_FORMAT_B8G8R8X8_UNORM: return "DXGI_FORMAT_B8G8R8X8_UNORM";
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM";
	case DXGI_FORMAT_B8G8R8A8_TYPELESS: return "DXGI_FORMAT_B8G8R8A8_TYPELESS";
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
	case DXGI_FORMAT_B8G8R8X8_TYPELESS: return "DXGI_FORMAT_B8G8R8X8_TYPELESS";
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB";
	case DXGI_FORMAT_BC6H_TYPELESS: return "DXGI_FORMAT_BC6H_TYPELESS";
	case DXGI_FORMAT_BC6H_UF16: return "DXGI_FORMAT_BC6H_UF16";
	case DXGI_FORMAT_BC6H_SF16: return "DXGI_FORMAT_BC6H_SF16";
	case DXGI_FORMAT_BC7_TYPELESS: return "DXGI_FORMAT_BC7_TYPELESS";
	case DXGI_FORMAT_BC7_UNORM: return "DXGI_FORMAT_BC7_UNORM";
	case DXGI_FORMAT_BC7_UNORM_SRGB: return "DXGI_FORMAT_BC7_UNORM_SRGB";
  }
  return "UNKNOWN??";
}

char* getMode(D3DFORMAT fmt)
{
  switch(fmt) {
    case D3DFMT_UNKNOWN: return "D3DFMT_UNKNOWN";
    case D3DFMT_R8G8B8: return "D3DFMT_R8G8B8";
    case D3DFMT_A8R8G8B8: return "D3DFMT_A8R8G8B8";
    case D3DFMT_X8R8G8B8: return "D3DFMT_X8R8G8B8";
    case D3DFMT_R5G6B5: return "D3DFMT_R5G6B5";
    case D3DFMT_X1R5G5B5: return "D3DFMT_X1R5G5B5";
    case D3DFMT_A1R5G5B5: return "D3DFMT_A1R5G5B5";
    case D3DFMT_A4R4G4B4: return "D3DFMT_A4R4G4B4";
    case D3DFMT_R3G3B2: return "D3DFMT_R3G3B2";
    case D3DFMT_A8: return "D3DFMT_A8";
    case D3DFMT_A8R3G3B2: return "D3DFMT_A8R3G3B2";
    case D3DFMT_X4R4G4B4: return "D3DFMT_X4R4G4B4";
    case D3DFMT_A2B10G10R10: return "D3DFMT_A2B10G10R10";
    case D3DFMT_A8B8G8R8: return "D3DFMT_A8B8G8R8";
    case D3DFMT_X8B8G8R8: return "D3DFMT_X8B8G8R8";
    case D3DFMT_G16R16: return "D3DFMT_G16R16";
    case D3DFMT_A2R10G10B10: return "D3DFMT_A2R10G10B10";
    case D3DFMT_A16B16G16R16: return "D3DFMT_A16B16G16R16";
    case D3DFMT_A8P8: return "D3DFMT_A8P8";
    case D3DFMT_P8: return "D3DFMT_P8";
    case D3DFMT_L8: return "D3DFMT_L8";
    case D3DFMT_A8L8: return "D3DFMT_A8L8";
    case D3DFMT_A4L4: return "D3DFMT_A4L4";
    case D3DFMT_V8U8: return "D3DFMT_V8U8";
    case D3DFMT_L6V5U5: return "D3DFMT_L6V5U5";
    case D3DFMT_X8L8V8U8: return "D3DFMT_X8L8V8U8";
    case D3DFMT_Q8W8V8U8: return "D3DFMT_Q8W8V8U8";
    case D3DFMT_V16U16: return "D3DFMT_V16U16";
    case D3DFMT_A2W10V10U10: return "D3DFMT_A2W10V10U10";
    case D3DFMT_UYVY: return "D3DFMT_UYVY";
    case D3DFMT_R8G8_B8G8: return "D3DFMT_R8G8_B8G8";
    case D3DFMT_YUY2: return "D3DFMT_YUY2";
    case D3DFMT_G8R8_G8B8: return "D3DFMT_G8R8_G8B8";
    case D3DFMT_DXT1: return "D3DFMT_DXT1";
    case D3DFMT_DXT2: return "D3DFMT_DXT2";
    case D3DFMT_DXT3: return "D3DFMT_DXT3";
    case D3DFMT_DXT4: return "D3DFMT_DXT4";
    case D3DFMT_DXT5: return "D3DFMT_DXT5";
    case D3DFMT_D16_LOCKABLE: return "D3DFMT_D16_LOCKABLE";
    case D3DFMT_D32: return "D3DFMT_D32";
    case D3DFMT_D15S1: return "D3DFMT_D15S1";
    case D3DFMT_D24S8: return "D3DFMT_D24S8";
    case D3DFMT_D24X8: return "D3DFMT_D24X8";
    case D3DFMT_D24X4S4: return "D3DFMT_D24X4S4";
    case D3DFMT_D16: return "D3DFMT_D16";
    case D3DFMT_D32F_LOCKABLE: return "D3DFMT_D32F_LOCKABLE";
    case D3DFMT_D24FS8: return "D3DFMT_D24FS8";
    case D3DFMT_D32_LOCKABLE: return "D3DFMT_D32_LOCKABLE";
    case D3DFMT_S8_LOCKABLE: return "D3DFMT_S8_LOCKABLE";
    case D3DFMT_L16: return "D3DFMT_L16";
    case D3DFMT_VERTEXDATA: return "D3DFMT_VERTEXDATA";
    case D3DFMT_INDEX16: return "D3DFMT_INDEX16";
    case D3DFMT_INDEX32: return "D3DFMT_INDEX32";
    case D3DFMT_Q16W16V16U16: return "D3DFMT_Q16W16V16U16";
    case D3DFMT_MULTI2_ARGB8: return "D3DFMT_MULTI2_ARGB8";
    case D3DFMT_R16F: return "D3DFMT_R16F";
    case D3DFMT_G16R16F: return "D3DFMT_G16R16F";
    case D3DFMT_A16B16G16R16F: return "D3DFMT_A16B16G16R16F";
    case D3DFMT_R32F: return "D3DFMT_R32F";
    case D3DFMT_G32R32F: return "D3DFMT_G32R32F";
    case D3DFMT_A32B32G32R32F: return "D3DFMT_A32B32G32R32F";
    case D3DFMT_CxV8U8: return "D3DFMT_CxV8U8";
    case D3DFMT_A1: return "D3DFMT_A1";
    case D3DFMT_A2B10G10R10_XR_BIAS: return "D3DFMT_A2B10G10R10_XR_BIAS";
    case D3DFMT_BINARYBUFFER: return "D3DFMT_BINARYBUFFER";
    default: {
      //return "Unknown??";
      static char foo[512];
      char *fc = (char*) &fmt;
      sprintf(foo, "(Unknown format: 0x%08X (%c%c%c%c))", fmt, fc[0],fc[1],fc[2],fc[3]);
      return foo;
      break;
    }
  }

  return "intel sucks";
}
char* getSwapEffect(D3DSWAPEFFECT se)
{
  switch(se) {
    case D3DSWAPEFFECT_DISCARD: return "D3DSWAPEFFECT_DISCARD";
    case D3DSWAPEFFECT_FLIP: return "D3DSWAPEFFECT_FLIP";
    case D3DSWAPEFFECT_COPY: return "D3DSWAPEFFECT_COPY";
    case D3DSWAPEFFECT_OVERLAY: return "D3DSWAPEFFECT_OVERLAY";
    case D3DSWAPEFFECT_FLIPEX: return "D3DSWAPEFFECT_FLIPEX";
    default: return "Unknown";
  }
}

char* getPresentationInterval(UINT pi)
{
  switch(pi) {
    //case D3DPRESENT_DONOTFLIP: return "D3DPRESENT_DONOTFLIP";
    //case D3DPRESENT_DONOTWAIT: return "D3DPRESENT_DONOTWAIT";
    case D3DPRESENT_FORCEIMMEDIATE: return "D3DPRESENT_FORCEIMMEDIATE";
    case D3DPRESENT_INTERVAL_DEFAULT: return "D3DPRESENT_INTERVAL_DEFAULT";
    case D3DPRESENT_INTERVAL_ONE: return "D3DPRESENT_INTERVAL_ONE";
    case D3DPRESENT_INTERVAL_TWO: return "D3DPRESENT_INTERVAL_TWO";
    case D3DPRESENT_INTERVAL_THREE: return "D3DPRESENT_INTERVAL_THREE";
    case D3DPRESENT_INTERVAL_FOUR: return "D3DPRESENT_INTERVAL_FOUR";
    case D3DPRESENT_INTERVAL_IMMEDIATE: return "D3DPRESENT_INTERVAL_IMMEDIATE";
    default: return "Unknown";
  }
}

char* getPool(D3DPOOL pool)
{
  switch(pool) {
    case D3DPOOL_DEFAULT: return "D3DPOOL_DEFAULT";
    case D3DPOOL_MANAGED: return "D3DPOOL_MANAGED";
    case D3DPOOL_SYSTEMMEM: return "D3DPOOL_SYSTEMMEM";
    case D3DPOOL_SCRATCH: return "D3DPOOL_SCRATCH";
    default: return "Unknown";
  }
}

char* getUsage(DWORD usage)
{
  if(!usage)
    return "NULL";
  static char ret[512];
  ret[0] = 0x00;
  DWORD u = usage;
  if(u & D3DUSAGE_AUTOGENMIPMAP) strcat(ret, "AUTOGENMIPMAP "), u-=D3DUSAGE_AUTOGENMIPMAP;
  if(u & D3DUSAGE_DEPTHSTENCIL) strcat(ret, "DEPTHSTENCIL "), u-=D3DUSAGE_DEPTHSTENCIL;
  if(u & D3DUSAGE_DMAP) strcat(ret, "DMAP "), u-=D3DUSAGE_DMAP;
  if(u & D3DUSAGE_DONOTCLIP) strcat(ret, "DONOTCLIP "), u-=D3DUSAGE_DONOTCLIP;
  if(u & D3DUSAGE_DYNAMIC) strcat(ret, "DYNAMIC "), u-=D3DUSAGE_DYNAMIC;
  if(u & D3DUSAGE_NPATCHES) strcat(ret, "NPATCHES "), u-=D3DUSAGE_NPATCHES;
  if(u & D3DUSAGE_POINTS) strcat(ret, "POINTS "), u-=D3DUSAGE_POINTS;
  if(u & D3DUSAGE_RENDERTARGET) strcat(ret, "RENDERTARGET "), u-=D3DUSAGE_RENDERTARGET;
  if(u & D3DUSAGE_RTPATCHES) strcat(ret, "RTPATCHES "), u-=D3DUSAGE_RTPATCHES;
  if(u & D3DUSAGE_SOFTWAREPROCESSING) strcat(ret, "SOFTWAREPROCESSING "), u-=D3DUSAGE_SOFTWAREPROCESSING;
  if(u & D3DUSAGE_WRITEONLY) strcat(ret, "WRITEONLY "), u-=D3DUSAGE_WRITEONLY;

  if(u & D3DUSAGE_QUERY_FILTER) strcat(ret, "QUERY_FILTER "), u-=D3DUSAGE_QUERY_FILTER;
  if(u & D3DUSAGE_QUERY_LEGACYBUMPMAP) strcat(ret, "QUERY_LEGACYBUMPMAP "), u-=D3DUSAGE_QUERY_LEGACYBUMPMAP;
  if(u & D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING) strcat(ret, "QUERY_POSTPIXELSHADER_BLENDING "), u-=D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING;
  if(u & D3DUSAGE_QUERY_SRGBREAD) strcat(ret, "QUERY_SRGBREAD "), u-=D3DUSAGE_QUERY_SRGBREAD;
  if(u & D3DUSAGE_QUERY_SRGBWRITE) strcat(ret, "QUERY_SRGBWRITE "), u-=D3DUSAGE_QUERY_SRGBWRITE;
  if(u & D3DUSAGE_QUERY_VERTEXTEXTURE) strcat(ret, "QUERY_VERTEXTEXTURE "), u-=D3DUSAGE_QUERY_VERTEXTEXTURE;
  if(u & D3DUSAGE_QUERY_WRAPANDMIP) strcat(ret, "QUERY_WRAPANDMIP "), u-=D3DUSAGE_QUERY_WRAPANDMIP;

  if(u) {
    char foo[256];
    sprintf(foo, "(?? 0x%08X)", u);
    strcat(ret, foo);
  }
  return ret;
}

char* getD3DCreate(DWORD usage)
{
  if(!usage)
    return "NULL";
  static char ret[1024];
  ret[0] = 0x00;
  if(usage & D3DCREATE_ADAPTERGROUP_DEVICE) strcat(ret, "D3DCREATE_ADAPTERGROUP_DEVICE ");
  if(usage & D3DCREATE_DISABLE_DRIVER_MANAGEMENT) strcat(ret, "D3DCREATE_DISABLE_DRIVER_MANAGEMENT ");
  if(usage & D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX) strcat(ret, "D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX ");
  if(usage & D3DCREATE_DISABLE_PRINTSCREEN) strcat(ret, "D3DCREATE_DISABLE_PRINTSCREEN ");
  if(usage & D3DCREATE_DISABLE_PSGP_THREADING) strcat(ret, "D3DCREATE_DISABLE_PSGP_THREADING ");
  if(usage & D3DCREATE_ENABLE_PRESENTSTATS) strcat(ret, "D3DCREATE_ENABLE_PRESENTSTATS ");
  if(usage & D3DCREATE_FPU_PRESERVE) strcat(ret, "D3DCREATE_FPU_PRESERVE ");
  if(usage & D3DCREATE_HARDWARE_VERTEXPROCESSING) strcat(ret, "D3DCREATE_HARDWARE_VERTEXPROCESSING ");
  if(usage & D3DCREATE_MIXED_VERTEXPROCESSING) strcat(ret, "D3DCREATE_MIXED_VERTEXPROCESSING ");
  if(usage & D3DCREATE_MULTITHREADED) strcat(ret, "D3DCREATE_MULTITHREADED ");
  if(usage & D3DCREATE_NOWINDOWCHANGES) strcat(ret, "D3DCREATE_NOWINDOWCHANGES ");
  if(usage & D3DCREATE_PUREDEVICE) strcat(ret, "D3DCREATE_PUREDEVICE ");
  if(usage & D3DCREATE_SCREENSAVER) strcat(ret, "D3DCREATE_SCREENSAVER ");
  if(usage & D3DCREATE_SOFTWARE_VERTEXPROCESSING) strcat(ret, "D3DCREATE_SOFTWARE_VERTEXPROCESSING ");
  return ret;
}


char* getLock(DWORD usage)
{
  if(!usage)
    return "NULL";
  static char ret[512];
  ret[0] = 0x00;
  if(usage & D3DLOCK_DISCARD) strcat(ret, "DISCARD ");
  if(usage & D3DLOCK_DONOTWAIT) strcat(ret, "DONOTWAIT ");
  if(usage & D3DLOCK_NO_DIRTY_UPDATE) strcat(ret, "NO_DIRTY_UPDATE ");
  if(usage & D3DLOCK_NOOVERWRITE) strcat(ret, "NOOVERWRITE ");
  if(usage & D3DLOCK_NOSYSLOCK) strcat(ret, "NOSYSLOCK ");
  if(usage & D3DLOCK_READONLY) strcat(ret, "READONLY ");
  return ret;
}

// Return d3d error as string
#define ERRORVALUE(x) case x: sprintf(m, "%s", #x);return m;
char* getD3DError(int v) {
	static char m[512];

	switch(v) {
    ERRORVALUE(D3DERR_DEVICEHUNG)
    ERRORVALUE(D3DERR_DEVICELOST)
    ERRORVALUE(D3DERR_DEVICEREMOVED)
    ERRORVALUE(D3DERR_OUTOFVIDEOMEMORY)
		ERRORVALUE(D3DERR_INVALIDCALL)
		ERRORVALUE(E_OUTOFMEMORY)
		ERRORVALUE(D3DERR_NOTAVAILABLE)
		ERRORVALUE(D3DERR_DEVICENOTRESET)
    ERRORVALUE(S_PRESENT_OCCLUDED)
    ERRORVALUE(S_PRESENT_MODE_CHANGED)
		ERRORVALUE(E_FAIL)
		ERRORVALUE(D3D_OK)
		ERRORVALUE(S_FALSE)
		//ERRORVALUE(DDERR_DIRECTDRAWALREADYCREATED)
		//ERRORVALUE(DDERR_GENERIC)
		//ERRORVALUE(DDERR_INVALIDDIRECTDRAWGUID)
		//ERRORVALUE(DDERR_INVALIDPARAMS)
		//ERRORVALUE(DDERR_NODIRECTDRAWHW)
		//ERRORVALUE(DDERR_OUTOFMEMORY)
/*
		ERRORVALUE(DDERR_EXCLUSIVEMODEALREADYSET)
		ERRORVALUE(DDERR_HWNDALREADYSET)
		ERRORVALUE(DDERR_HWNDSUBCLASSED)
		ERRORVALUE(DDERR_INVALIDOBJECT)
		//ERRORVALUE(DDERR_INVALIDPARAMS)
		//ERRORVALUE(DDERR_OUTOFMEMORY)
*/
	}
	sprintf(m, "Unknown (%d)", v);
	return m;
}

LARGE_INTEGER tFreq, tStart;
void startTimer() {
	QueryPerformanceFrequency(&tFreq);
	QueryPerformanceCounter(&tStart);
}

double stopTimer() {
	LARGE_INTEGER tEnd;
    QueryPerformanceCounter(&tEnd);
    return (double(tEnd.QuadPart - tStart.QuadPart) / tFreq.QuadPart);
}

// Return "ticks" in high accuracy, pause support
double gticks = 0;
double GetHFTicks() {
	static LARGE_INTEGER tFreq, t;
	static double last;
	if(tFreq.QuadPart == 0)
		QueryPerformanceFrequency(&tFreq);
	QueryPerformanceCounter(&t);

	//if(!(LFSState.Flags & ISS_PAUSED)) {
		gticks+= ((double) ((double) t.QuadPart / tFreq.QuadPart)*1000)-last;
	//}

	last = (double) ((double) t.QuadPart / tFreq.QuadPart)*1000;
	return gticks;
}

// memcpy with pitch values
void memcpyPitched(void* dst, void *src, DWORD size, DWORD pitchDst, DWORD pitchSrc) {
  if(pitchDst == pitchSrc)
		memcpy(dst, src, size); // that was easy
	else {
		DWORD cpyPitch = pitchDst<pitchDst?pitchDst:pitchSrc;
		DWORD copied = 0;
		while(true) {
      memcpy(dst, src, cpyPitch);
			dst = ((BYTE*)dst) + pitchDst;
			src = ((BYTE*)src) + pitchSrc;
			copied += cpyPitch;
			if(copied >= size)
				break;
		}
	}
}

BOOL SetPrivilege(
	HANDLE hToken,  // token handle
	LPCTSTR Privilege,  // Privilege to enable/disable
	BOOL bEnablePrivilege  // TRUE to enable. FALSE to disable
)
{
	TOKEN_PRIVILEGES tp = { 0 };
	// Initialize everything to zero
	LUID luid;
	DWORD cb=sizeof(TOKEN_PRIVILEGES);
	if(!LookupPrivilegeValue( NULL, Privilege, &luid ))
		return FALSE;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if(bEnablePrivilege) {
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	} else {
		tp.Privileges[0].Attributes = 0;
	}
	AdjustTokenPrivileges( hToken, FALSE, &tp, cb, NULL, NULL );
	if (GetLastError() != ERROR_SUCCESS)
		return FALSE;

	return TRUE;
}

RECT* isNullRect(RECT *r)
{
  if(r->left == 0 && r->right == 0 && r->top == 0 && r->bottom == 0)
    return NULL;
  else
    return r;
}

char* strRect(CONST RECT *r)
{
  static char foo[256];
  if(!r)
    strcpy(foo, "NULL");
  else
    sprintf(foo, "%dx%d/%dx%d", r->left, r->top, r->right, r->bottom);
  return foo;
}
