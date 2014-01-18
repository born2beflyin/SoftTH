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

#include "dxgiAdapterOutput.h"
#include "helper.h"
#include "version.h"

#include <dxgi.h>

#include "main.h"

#include <INITGUID.H>
DEFINE_GUID(IID_IDXGIAdapter1New, 0x6a07d623, 0x9a07, 0x48e8, 0x98, 0xea, 0x1, 0x19, 0x91, 0x40, 0x94, 0x9c);
DEFINE_GUID(IID_IDXGIOutputNew, 0xd76d1fc1, 0x206e, 0x455e, 0x94, 0x3c, 0x69, 0xca, 0x30, 0x2d, 0x5e, 0xe3);

IDXGIAdapter1New::IDXGIAdapter1New(IDXGIAdapter1 *dxgaNew, IDXGIFactory1 *parentNew)
{
  dbg("IDXGIAdapter1New 0x%08X", this);
  dxga = dxgaNew;
  parent = parentNew;
};

IDXGIAdapter1New::~IDXGIAdapter1New()
{
  dbg("~IDXGIAdapter1New 0x%08X", this);
}

static void tamperDesc(DXGI_ADAPTER_DESC *desc)
{
  // Change device name to SoftTH device
  wcscpy(desc->Description, SOFTTH_VERSIONW);
}

HRESULT IDXGIAdapter1New::GetDesc(DXGI_ADAPTER_DESC *pDesc)
{
  dbg("dxga: GetDesc");
  HRESULT ret = dxga->GetDesc(pDesc);
  tamperDesc(pDesc);
  return ret;
};    

HRESULT IDXGIAdapter1New::GetDesc1(DXGI_ADAPTER_DESC1 *pDesc)
{
  dbg("dxga: GetDesc1");
  HRESULT ret = dxga->GetDesc1(pDesc);
  tamperDesc((DXGI_ADAPTER_DESC*)pDesc);
  return ret;
};  

HRESULT IDXGIAdapter1New::EnumOutputs(UINT Output, IDXGIOutput **ppOutput)
{  
  dbg("dxga: EnumOutputs");
  // Hide all but the primary output
  if(Output > 0)
    return DXGI_ERROR_NOT_FOUND;
  else {
    IDXGIOutput *o;
    HRESULT ret = dxga->EnumOutputs(Output, &o);
    if(ret == S_OK)
      *ppOutput = new IDXGIOutputNew(this, o);    
    return ret;
  }
}




IDXGIOutputNew::IDXGIOutputNew(IDXGIAdapter1New *parentNew, IDXGIOutput *dxgoNew)
{
  dbg("IDXGIOutputNew 0x%08X %d", this, dxgoNew);
  dxgo = dxgoNew;
  parent = parentNew;
}

IDXGIOutputNew::~IDXGIOutputNew()
{
  dbg("~IDXGIOutputNew 0x%08X", this);
}

HRESULT IDXGIOutputNew::GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags, UINT *pNumModes, DXGI_MODE_DESC *pDesc)
{
  const int extraModes = 1; // TODO: get refresh rates
  dbg("dxgo: GetDisplayModeList %s %d", getFormatDXGI(EnumFormat), Flags);
  if(pDesc == 0) {
    // Getting number of modes - add ours
    HRESULT ret = dxgo->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);
    if(*pNumModes > 1) {
      dbg("dxgo: GetDisplayModeList: %d+%d modes", *pNumModes, extraModes);
      *pNumModes+=extraModes;    
    }
    return ret;
  } else {

    if(*pNumModes == 0)
      return dxgo->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);

    // Getting list of modes
    UINT num = *pNumModes-extraModes;
    HRESULT ret = dxgo->GetDisplayModeList(EnumFormat, Flags, &num, pDesc);
    if(ret != S_OK)
      return ret;

    // Add our modes
    DXGI_MODE_DESC *mode = &pDesc[*pNumModes-1];
    mode->Width = config.main.renderResolution.x;
    mode->Height = config.main.renderResolution.y;
    mode->RefreshRate.Numerator = 59950;
    mode->RefreshRate.Denominator = 1000;
    mode->ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    mode->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    mode->Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    dbg("Mode dump:");
    for(DWORD i=0;i<*pNumModes;i++) {
      dbg("Mode %d, %dx%d %d.%dHz scaling:%d so:%d format:%s", i, pDesc[i].Width, pDesc[i].Height, pDesc[i].RefreshRate.Numerator, pDesc[i].RefreshRate.Denominator, pDesc[i].Scaling, pDesc[i].ScanlineOrdering, getFormatDXGI(mode->Format));
    }
    dbg("Mode dump end");

    return ret;
  }

 return dxgo->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);
}