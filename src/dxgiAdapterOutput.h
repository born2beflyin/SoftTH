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

// DXGI Adapter and DXGI Output classes

#ifndef __DXGIADAPTER_H__
#define __DXGIADAPTER_H__

#include "helper.h"
#include "main.h"
#include "dxgiFactory.h"

DEFINE_GUID(IID_IDXGIAdapterNew, 0xd2d40e93, 0xbb8f, 0x41df, 0x9b, 0x9c, 0x86, 0xc, 0x67, 0x44, 0x48, 0x3d); // {D2D40E93-BB8F-41df-9B9C-860C6744483D}
//DEFINE_GUID(IID_IDXGIAdapter1New, 0x6a07d623, 0x9a07, 0x48e8, 0x98, 0xea, 0x1, 0x19, 0x91, 0x40, 0x94, 0x9c);
DEFINE_GUID(IID_IDXGIOutputNew, 0xd76d1fc1, 0x206e, 0x455e, 0x94, 0x3c, 0x69, 0xca, 0x30, 0x2d, 0x5e, 0xe3);

//interface IDXGIAdapterNew : IDXGIAdapter
//{
//public:
//  IDXGIAdapterNew(IDXGIAdapter *dxgaNew, IDXGIFactory *parentNew);
//  ~IDXGIAdapterNew();
//
//  DECALE_DXGICOMMONIF(dxga);
//
//  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
//      dbg("dxgi_a: QueryInterface %s", matchRiid(riid));
//      if(riid == IID_IDXGIAdapter ||
//         riid == IID_IDXGIAdapterNew) {
//        this->AddRef();
//        *ppvObj = this;
//        dbg("dxgi_a: Got interface %s 0x%08X", matchRiid(IID_IDXGIAdapterNew), *ppvObj);
//        return S_OK;
//      }
//      return dxga->QueryInterface(riid, ppvObj);
//  };
//
//  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
//    {dbg("dxgi_a: GetParent %s", matchRiid(riid));
//      if(riid == IID_IDXGIFactory) {
//        *ppParent = parent;
//        parent->AddRef();
//        dbg("dxgi_a1: Got parent %s 0x%08X", matchRiid(riid), *ppParent);
//        return S_OK;
//      }
//      return dxga->GetParent(riid, ppParent);
//    };
//
//  HRESULT STDMETHODCALLTYPE EnumOutputs(UINT Output, IDXGIOutput **ppOutput)
//    ;//{dbg("dxgi_a: EnumOutputs");return dxga->EnumOutputs(Output, ppOutput);};
//  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_ADAPTER_DESC *pDesc)
//    ;//{dbg("dxgi_a: GetDesc");return dxga->GetDesc(pDesc);};
//  HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(REFGUID InterfaceName, LARGE_INTEGER *pUMDVersion)
//    {dbg("dxgi_a: CheckInterfaceSupport");return dxga->CheckInterfaceSupport(InterfaceName, pUMDVersion);};
//
//  IDXGIAdapter* getReal() {dbg("Get real adapter 0x%08X", dxga);return dxga;};
//
//private:
//  IDXGIAdapter *dxga;
//  IDXGIFactory *parent;
//};

interface IDXGIAdapterNew : IDXGIAdapter2
{
public:
  IDXGIAdapterNew(IDXGIAdapter2 *dxgaNew, IDXGIFactoryNew *parentNew);
  ~IDXGIAdapterNew();

  DECALE_DXGICOMMONIF(dxga);

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgi_a2: 0x%08X QueryInterface %s", this, matchRiid(riid));
      if(riid == IID_IDXGIAdapter ||
         riid == IID_IDXGIAdapter1 ||
         riid == IID_IDXGIAdapter2 ||
         riid == IID_IDXGIAdapterNew) {
        this->AddRef();
        *ppvObj = this;
        dbg("dxgi_a2: -- Got interface %s 0x%08X", matchRiid(riid), *ppvObj);
        return S_OK;
      }
      return dxga->QueryInterface(riid, ppvObj);
  };

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxgi_a2: 0x%08X GetParent %s", this, matchRiid(riid));
      if(riid == IID_IDXGIFactory ||
         riid == IID_IDXGIFactory1 ||
         riid == IID_IDXGIFactory2 ||
         riid == IID_IDXGIFactoryNew) {
        parent->AddRef();
        *ppParent = parent;
        dbg("dxgi_a2: -- Got parent %s 0x%08X", matchRiid(riid), *ppParent);
        return S_OK;
      }
      return dxga->GetParent(riid, ppParent);
    };

  HRESULT STDMETHODCALLTYPE EnumOutputs(UINT Output, IDXGIOutput **ppOutput)
    ;//{dbg("dxgi_a2: EnumOutputs");return dxga->EnumOutputs(Output, ppOutput);};
  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_ADAPTER_DESC *pDesc)
    ;//{dbg("dxgi_a2: GetDesc");return dxga->GetDesc(pDesc);};
  HRESULT STDMETHODCALLTYPE GetDesc1(DXGI_ADAPTER_DESC1 *pDesc)
    ;//{dbg("dxgi_a2: GetDesc1");return dxga->GetDesc1(pDesc);};
  HRESULT STDMETHODCALLTYPE GetDesc2(DXGI_ADAPTER_DESC2 *pDesc)
    ;//{dbg("dxgi_a2: GetDesc1");return dxga->GetDesc1(pDesc);};
  HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(REFGUID InterfaceName, LARGE_INTEGER *pUMDVersion)
    {dbg("dxgi_a2: 0x%08X CheckInterfaceSupport", this); return dxga->CheckInterfaceSupport(InterfaceName, pUMDVersion);};

  IDXGIAdapter2* getReal() {dbg("dxgi_a2: 0x%08X Get real adapter 0x%08X", this, dxga);return dxga;};

private:
  IDXGIAdapter2 *dxga;
  IDXGIFactoryNew *parent;
};

interface IDXGIOutputNew : IDXGIOutput1
{
public:
  IDXGIOutputNew(IDXGIOutput1 *dxgoNew, IDXGIAdapterNew *parentNew);
  //IDXGIOutputNew(IDXGIAdapter1New *parentNew, IDXGIOutput *dxgoNew);
  ~IDXGIOutputNew();

  DECALE_DXGICOMMONIF(dxgo);

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgi_o1: 0x%08X QueryInterface %s", this, matchRiid(riid));
      if(riid == IID_IDXGIOutput ||
         riid == IID_IDXGIOutput1 ||
         riid == IID_IDXGIOutputNew) {
        this->AddRef();
        *ppvObj = this;
        dbg("dxgi_o1: -- Got interface %s 0x%08X", matchRiid(riid), *ppvObj);
        return S_OK;
      }
      return dxgo->QueryInterface(riid, ppvObj);
  };

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent) {
    dbg("dxgi_o1: 0x%08X GetParent %s", this, matchRiid(riid));
    if(riid == IID_IDXGIAdapter ||
       riid == IID_IDXGIAdapter1 ||
       riid == IID_IDXGIAdapter2 ||
       riid == IID_IDXGIAdapterNew) {
      parent->AddRef();
      *ppParent = parent;
      dbg("dxgi_o1: -- Got parent %s 0x%08X", matchRiid(riid), *ppParent);
      return S_OK;
    }
    return dxgo->GetParent(riid, ppParent);
  };

  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_OUTPUT_DESC *pDesc)
    ;//{dbg("dxgi_o1: GetDesc");HRESULT ret = dxgo->GetDesc(pDesc);return ret;};
  HRESULT STDMETHODCALLTYPE GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags, UINT *pNumModes, DXGI_MODE_DESC *pDesc)
    ;//{dbg("dxgi_o1: GetDisplayModeList");return dxgo->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);};
  HRESULT STDMETHODCALLTYPE GetDisplayModeList1(DXGI_FORMAT EnumFormat, UINT Flags, UINT *pNumModes, DXGI_MODE_DESC1 *pDesc)
    ;//{dbg("dxgi_o1: GetDisplayModeList1");return dxgo->GetDisplayModeList1(EnumFormat, Flags, pNumModes, pDesc);};
  HRESULT STDMETHODCALLTYPE DuplicateOutput(IUnknown *pDevice, IDXGIOutputDuplication **ppOutputDuplication)
    {dbg("dxgi_o1: 0x%08X DuplicateOutput", this);return dxgo->DuplicateOutput(pDevice, ppOutputDuplication);};
  HRESULT STDMETHODCALLTYPE FindClosestMatchingMode(const DXGI_MODE_DESC *pModeToMatch, DXGI_MODE_DESC *pClosestMatch,IUnknown *pConcernedDevice)
    ;//{dbg("dxgi_o1: 0x%08X FindClosestMatchingMode", this);return dxgo->FindClosestMatchingMode(pModeToMatch, pClosestMatch, pConcernedDevice);};
  HRESULT STDMETHODCALLTYPE FindClosestMatchingMode1(const DXGI_MODE_DESC1 *pModeToMatch, DXGI_MODE_DESC1 *pClosestMatch,IUnknown *pConcernedDevice)
    ;//{dbg("dxgi_o1: 0x%08X FindClosestMatchingMode", this);return dxgo->FindClosestMatchingMode(pModeToMatch, pClosestMatch, pConcernedDevice);};
  HRESULT STDMETHODCALLTYPE WaitForVBlank(void)
    {dbg("dxgi_o1: 0x%08X WaitForVBlank", this);return dxgo->WaitForVBlank();};
  HRESULT STDMETHODCALLTYPE TakeOwnership(IUnknown *pDevice, BOOL Exclusive)
    {dbg("dxgi_o1: 0x%08X TakeOwnership", this);return dxgo->TakeOwnership(pDevice, Exclusive);};
  void STDMETHODCALLTYPE ReleaseOwnership(void)
    {dbg("dxgi_o1: 0x%08X ReleaseOwnership", this);return dxgo->ReleaseOwnership();};
  HRESULT STDMETHODCALLTYPE GetGammaControlCapabilities(DXGI_GAMMA_CONTROL_CAPABILITIES *pGammaCaps)
    {dbg("dxgi_o1: 0x%08X GetGammaControlCapabilities", this);return dxgo->GetGammaControlCapabilities(pGammaCaps);};
  HRESULT STDMETHODCALLTYPE SetGammaControl(const DXGI_GAMMA_CONTROL *pArray)
    {dbg("dxgi_o1: 0x%08X SetGammaControl", this);return dxgo->SetGammaControl(pArray);};
  HRESULT STDMETHODCALLTYPE GetGammaControl(DXGI_GAMMA_CONTROL *pArray)
    {dbg("dxgi_o1: 0x%08X GetGammaControl", this);return dxgo->GetGammaControl(pArray);};
  HRESULT STDMETHODCALLTYPE SetDisplaySurface(IDXGISurface *pScanoutSurface)
    {dbg("dxgi_o1: 0x%08X SetDisplaySurface", this);return dxgo->SetDisplaySurface(pScanoutSurface);};
  HRESULT STDMETHODCALLTYPE GetDisplaySurfaceData(IDXGISurface *pDestination)
    {dbg("dxgi_o1: 0x%08X GetDisplaySurfaceData", this);return dxgo->GetDisplaySurfaceData(pDestination);};
  HRESULT STDMETHODCALLTYPE GetDisplaySurfaceData1(IDXGIResource *pDestination)
    {dbg("dxgi_o1: 0x%08X GetDisplaySurfaceData1", this);return dxgo->GetDisplaySurfaceData1(pDestination);};
  HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats)
    {dbg("dxgi_o1: 0x%08X GetFrameStatistics", this);return dxgo->GetFrameStatistics(pStats);};

  IDXGIOutput1* getReal() {dbg("dxgi_o1: 0x%08X Get real output 0x%08X", this, dxgo);return dxgo;};

private:
  IDXGIOutput1    *dxgo;
  IDXGIAdapterNew *parent;
  DXGI_MODE_DESC1  *newmode;
};

#endif
