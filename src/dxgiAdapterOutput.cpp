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

#include <INITGUID.H>
DEFINE_GUID(IID_IDXGIAdapterNew, 0xd2d40e93, 0xbb8f, 0x41df, 0x9b, 0x9c, 0x86, 0xc, 0x67, 0x44, 0x48, 0x3d); // {D2D40E93-BB8F-41df-9B9C-860C6744483D}
//DEFINE_GUID(IID_IDXGIAdapter1New, 0x6a07d623, 0x9a07, 0x48e8, 0x98, 0xea, 0x1, 0x19, 0x91, 0x40, 0x94, 0x9c);
DEFINE_GUID(IID_IDXGIOutputNew, 0xd76d1fc1, 0x206e, 0x455e, 0x94, 0x3c, 0x69, 0xca, 0x30, 0x2d, 0x5e, 0xe3);




static void tamperDesc(DXGI_ADAPTER_DESC *desc)
{
  // Change device name to SoftTH device
  wcscpy(desc->Description, SOFTTH_VERSIONW);
}
static void tamperDesc(DXGI_ADAPTER_DESC1 *desc)
{
  // Change device name to SoftTH device
  wcscpy(desc->Description, SOFTTH_VERSIONW);
}
static void tamperDesc(DXGI_ADAPTER_DESC2 *desc)
{
  // Change device name to SoftTH device
  wcscpy(desc->Description, SOFTTH_VERSIONW);
}



///* IDXGIAdapterNew */
//IDXGIAdapterNew::IDXGIAdapterNew(IDXGIAdapter *dxgaNew, IDXGIFactory *parentNew)
//{
//  dbg("dxgi_a: IDXGIAdapterNew 0x%08X", this);
//  dxga = dxgaNew;
//  parent = parentNew;
//};
//
//IDXGIAdapterNew::~IDXGIAdapterNew()
//{
//  dbg("dxgi_a: ~IDXGIAdapterNew 0x%08X", this);
//}
//
//HRESULT IDXGIAdapterNew::GetDesc(DXGI_ADAPTER_DESC *pDesc)
//{
//  dbg("dxgi_a: GetDesc");
//  HRESULT ret = dxga->GetDesc(pDesc);
//  tamperDesc(pDesc);
//  return ret;
//};
//
//HRESULT IDXGIAdapterNew::EnumOutputs(UINT Output, IDXGIOutput **ppOutput)
//{
//  dbg("dxgi_a: EnumOutputs");
//  // Hide all but the primary output
//  if(Output > 0)
//    return DXGI_ERROR_NOT_FOUND;
//  else {
//    IDXGIOutput *o;
//    HRESULT ret = dxga->EnumOutputs(Output, &o);
//    if(ret == S_OK)
//    {
//      *ppOutput = new IDXGIOutputNew(o, this);
//      /*DXGI_OUTPUT_DESC *pDesc = NULL;
//      dbg("dxga: Booyah");
//      ppOutput[0]->GetDesc(pDesc);
//      dbg((char *)pDesc->DeviceName);*/
//    }
//
//    return ret;
//  }
//}


/*   IDXGIAdapterNew   */
IDXGIAdapterNew::IDXGIAdapterNew(IDXGIAdapter2 *dxgaNew, IDXGIFactoryNew *parentNew)
{
  dbg("dxgi_a2: 0x%08X IDXGIAdapterNew", this);
  dbg("dxgi_a2: -- Real: 0x%08X - SoftTH: 0x%08X - Parent: 0x%08X",dxgaNew,this,parentNew);
  dxga = dxgaNew;
  parent = parentNew;
};

IDXGIAdapterNew::~IDXGIAdapterNew()
{
  dbg("dxgi_a2: 0x%08X ~IDXGIAdapterNew", this);
}

HRESULT IDXGIAdapterNew::GetDesc(DXGI_ADAPTER_DESC *pDesc)
{
  dbg("dxgi_a2: 0x%08X GetDesc", this);
  HRESULT ret = dxga->GetDesc(pDesc);
  tamperDesc(pDesc);
  return ret;
};

HRESULT IDXGIAdapterNew::GetDesc1(DXGI_ADAPTER_DESC1 *pDesc)
{
  dbg("dxgi_a2: 0x%08X GetDesc1", this);
  HRESULT ret = dxga->GetDesc1(pDesc);
  tamperDesc(pDesc);
  return ret;
};

HRESULT IDXGIAdapterNew::GetDesc2(DXGI_ADAPTER_DESC2 *pDesc)
{
  dbg("dxgi_a2: 0x%08X GetDesc2", this);
  HRESULT ret = dxga->GetDesc2(pDesc);
  tamperDesc((DXGI_ADAPTER_DESC*)pDesc);
  return ret;
};

HRESULT IDXGIAdapterNew::EnumOutputs(UINT Output, IDXGIOutput **ppOutput)
{
  dbg("dxgi_a2: 0x%08X EnumOutputs", this);
  // Hide all but the primary output
  if(Output > 0) {
    dbg("dxgi_a2: -- Tried to get more outputs");
    return DXGI_ERROR_NOT_FOUND;
  } else {
    dbg("dxgi_a2: -- First enumeration");
    IDXGIOutput *o;
    HRESULT ret = dxga->EnumOutputs(Output, &o);
    dbg("dxgi_a2: -- Enumerated real output 0x%08X", o);
    if(ret == S_OK)
    {
      *ppOutput = new IDXGIOutputNew((IDXGIOutput1 *) o, this);
      dbg("dxgi_a2: -- Replaced real output with SoftTH output: 0x%08X", *ppOutput);
      /*DXGI_OUTPUT_DESC *pDesc = NULL;
      dbg("dxga: Booyah");
      ppOutput[0]->GetDesc(pDesc);
      dbg((char *)pDesc->DeviceName);*/
    }

    return ret;
  }
}


IDXGIOutputNew::IDXGIOutputNew( IDXGIOutput1 *dxgoNew, IDXGIAdapterNew *parentNew)
{
  dbg("dxgi_o1: 0x%08X IDXGIOutputNew", this);
  dbg("dxgi_o1: -- Real: 0x%08X - SoftTH: 0x%08X - Parent: 0x%08X",dxgoNew,this,parentNew);
  dxgo = dxgoNew;
  parent = parentNew;
}

IDXGIOutputNew::~IDXGIOutputNew()
{
  dbg("dxgi_o1: 0x%08X ~IDXGIOutputNew", this);
}

HRESULT IDXGIOutputNew::GetDesc(DXGI_OUTPUT_DESC *pDesc)
{
  dbg("dxgi_o1: 0x%08X GetDesc", this);

  RECT        rt;
  WCHAR       dname[32] = L"SoftTH";
  HRESULT ret = dxgo->GetDesc(pDesc);

  rt.left = 0.f;
  rt.right = config.main.renderResolution.x;
  rt.top = 0.f;
  rt.bottom = config.main.renderResolution.y;

  wcscpy(pDesc->DeviceName, dname);
  pDesc->DesktopCoordinates = rt;
  pDesc->Rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
  pDesc->AttachedToDesktop = true;
  //pDesc->Monitor = //TODO: finish this

  dbg("dxgi_o1: -- Replaced the real description with the SoftTH description");

  return ret;
}


HRESULT IDXGIOutputNew::GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags, UINT *pNumModes, DXGI_MODE_DESC *pDesc)
{
  const int extraModes = 1; // TODO: get refresh rates
  dbg("dxgi_o1: 0x%08X GetDisplayModeList %s %d", this, getFormatDXGI(EnumFormat), Flags);
  if(pDesc == 0) {
    // Getting number of modes - add ours
    HRESULT ret = dxgo->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);
    if(*pNumModes > 1) {
      dbg("dxgi_o1: -- GetDisplayModeList: %d+%d modes", *pNumModes, extraModes);
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

    // Store the new mode
    newmode = (DXGI_MODE_DESC1 *) mode;

    dbg("dxgi_o1: -- Mode dump:");
    for(DWORD i=0;i<*pNumModes;i++) {
      dbg("dxgi_o1: -- Mode %d, %dx%d %dHz scaling:%d so:%d format:%s", i, pDesc[i].Width, pDesc[i].Height, pDesc[i].RefreshRate.Numerator/pDesc[i].RefreshRate.Denominator, pDesc[i].Scaling, pDesc[i].ScanlineOrdering, getFormatDXGI(mode->Format));
    }
    dbg("dxgi_o1: -- Mode dump end");

    return ret;
  }

  return dxgo->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);
}

HRESULT IDXGIOutputNew::GetDisplayModeList1(DXGI_FORMAT EnumFormat, UINT Flags, UINT *pNumModes, DXGI_MODE_DESC1 *pDesc)
{
  const int extraModes = 1; // TODO: get refresh rates
  dbg("dxgi_o1: 0x%08X GetDisplayModeList1 %s %d", this, getFormatDXGI(EnumFormat), Flags);
  if(pDesc == 0) {
    // Getting number of modes - add ours
    HRESULT ret = dxgo->GetDisplayModeList1(EnumFormat, Flags, pNumModes, pDesc);
    if(*pNumModes > 1) {
      dbg("dxgi_o1: -- GetDisplayModeList1: %d+%d modes", *pNumModes, extraModes);
      *pNumModes+=extraModes;
    }
    return ret;
  } else {

    if(*pNumModes == 0)
      return dxgo->GetDisplayModeList1(EnumFormat, Flags, pNumModes, pDesc);

    // Getting list of modes
    UINT num = *pNumModes-extraModes;
    HRESULT ret = dxgo->GetDisplayModeList1(EnumFormat, Flags, &num, pDesc);
    if(ret != S_OK)
      return ret;

    // Add our modes
    DXGI_MODE_DESC1 *mode = &pDesc[*pNumModes-1];
    mode->Width = config.main.renderResolution.x;
    mode->Height = config.main.renderResolution.y;
    mode->RefreshRate.Numerator = 59950;
    mode->RefreshRate.Denominator = 1000;
    mode->ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    mode->Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    mode->Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Store the new mode
    newmode = mode;

    dbg("dxgi_o1: -- Mode dump:");
    for(DWORD i=0;i<*pNumModes;i++) {
      dbg("dxgi_o1: -- Mode %d, %dx%d %dHz scaling:%d so:%d format:%s", i, pDesc[i].Width, pDesc[i].Height, pDesc[i].RefreshRate.Numerator/pDesc[i].RefreshRate.Denominator, pDesc[i].Scaling, pDesc[i].ScanlineOrdering, getFormatDXGI(mode->Format));
    }
    dbg("dxgi_o1: -- Mode dump end");

    return ret;
  }

  return dxgo->GetDisplayModeList1(EnumFormat, Flags, pNumModes, pDesc);
}

HRESULT IDXGIOutputNew::FindClosestMatchingMode(const DXGI_MODE_DESC *pModeToMatch, DXGI_MODE_DESC *pClosestMatch,IUnknown *pConcernedDevice) {
  dbg("dxgi_o1: 0x%08X FindClosestMatchingMode", this);

  HRESULT ret = dxgo->FindClosestMatchingMode(pModeToMatch, pClosestMatch, pConcernedDevice);

  //pClosestMatch = (DXGI_MODE_DESC *) newmode;
  //dbg("dxgi_o1: -- Replaced the requested mode with the new SoftTH mode");

  return ret;
};

HRESULT IDXGIOutputNew::FindClosestMatchingMode1(const DXGI_MODE_DESC1 *pModeToMatch, DXGI_MODE_DESC1 *pClosestMatch,IUnknown *pConcernedDevice) {
  dbg("dxgi_o1: 0x%08X FindClosestMatchingMode1", this);

  HRESULT ret = dxgo->FindClosestMatchingMode1(pModeToMatch, pClosestMatch, pConcernedDevice);

  //pClosestMatch = newmode;
  //dbg("dxgi_o1: -- Replaced the requested mode with the new SoftTH mode");

  return ret;
};
