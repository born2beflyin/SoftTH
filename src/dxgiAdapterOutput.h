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

#include <dxgi.h>
#include "helper.h"

DEFINE_GUID(IID_IDXGIAdapter1New, 0x6a07d623, 0x9a07, 0x48e8, 0x98, 0xea, 0x1, 0x19, 0x91, 0x40, 0x94, 0x9c);
DEFINE_GUID(IID_IDXGIOutputNew, 0xd76d1fc1, 0x206e, 0x455e, 0x94, 0x3c, 0x69, 0xca, 0x30, 0x2d, 0x5e, 0xe3);

interface IDXGIAdapter1New : IDXGIAdapter1
{
public:
  IDXGIAdapter1New(IDXGIAdapter1 *dxgaNew, IDXGIFactory1 *parentNew);
  ~IDXGIAdapter1New();

  DECALE_DXGICOMMONIF(dxga);

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxga: QueryInterface %s 0x%08X", matchRiid(riid), *ppvObj);
      if(riid == IID_IDXGIAdapter1New || riid == IID_IDXGIAdapter1 || riid == IID_IDXGIAdapter) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
        return dxga->QueryInterface(riid, ppvObj);
  };

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxga: GetParent %s 0x%08X", matchRiid(riid), *ppParent);
      if(riid == IID_IDXGIFactory || riid == IID_IDXGIFactory1) {
        *ppParent = parent;
        parent->AddRef();
        return S_OK;
      }
      return dxga->GetParent(riid, ppParent);
    };

  HRESULT STDMETHODCALLTYPE EnumOutputs(UINT Output, IDXGIOutput **ppOutput)
    ;//{dbg("dxga: EnumOutputs");return dxga->EnumOutputs(Output, ppOutput);};
  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_ADAPTER_DESC *pDesc)
    ;//{dbg("dxga: GetDesc");return dxga->GetDesc(pDesc);};
  HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(REFGUID InterfaceName, LARGE_INTEGER *pUMDVersion)
    {dbg("dxga: CheckInterfaceSupport");return dxga->CheckInterfaceSupport(InterfaceName, pUMDVersion);};
  HRESULT STDMETHODCALLTYPE GetDesc1(DXGI_ADAPTER_DESC1 *pDesc)
    ;//{dbg("dxga: GetDesc1");return dxga->GetDesc1(pDesc);};

  IDXGIAdapter1* getReal() {dbg("Get real adapter 0x%08X", dxga);return dxga;};

private:
  IDXGIAdapter1 *dxga;
  IDXGIFactory1 *parent;
};

interface IDXGIOutputNew : IDXGIOutput
{
public:
  IDXGIOutputNew(IDXGIAdapter1New *parentNew, IDXGIOutput *dxgoNew);
  ~IDXGIOutputNew();

  DECALE_DXGICOMMONIF(dxgo);

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
      dbg("dxgo: QueryInterface %s 0x%08X", matchRiid(riid), *ppvObj);
      if(riid == IID_IDXGIOutputNew) {
        this->AddRef();
        *ppvObj = this;
        return S_OK;
      } else
      return dxgo->QueryInterface(riid, ppvObj);
  };

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent)
    {dbg("dxgo: GetParent %s 0x%08X", matchRiid(riid), *ppParent);
    if(riid == IID_IDXGIAdapter || riid == IID_IDXGIAdapter1) {
      *ppParent = parent;
      parent->AddRef();
      return S_OK;
    }
    return dxgo->GetParent(riid, ppParent);};

  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_OUTPUT_DESC *pDesc)
    ;//{dbg("dxgo: GetDesc");HRESULT ret = dxgo->GetDesc(pDesc);return ret;};
  HRESULT STDMETHODCALLTYPE GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags, UINT *pNumModes, DXGI_MODE_DESC *pDesc)
    ;//{dbg("dxgo: GetDisplayModeList");return dxgo->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);};
  HRESULT STDMETHODCALLTYPE FindClosestMatchingMode(const DXGI_MODE_DESC *pModeToMatch, DXGI_MODE_DESC *pClosestMatch,IUnknown *pConcernedDevice)
    {dbg("dxgo: FindClosestMatchingMode");return dxgo->FindClosestMatchingMode(pModeToMatch, pClosestMatch, pConcernedDevice);};
  HRESULT STDMETHODCALLTYPE WaitForVBlank(void)
    {dbg("dxgo: WaitForVBlank");return dxgo->WaitForVBlank();};
  HRESULT STDMETHODCALLTYPE TakeOwnership(IUnknown *pDevice, BOOL Exclusive)
    {dbg("dxgo: TakeOwnership");return dxgo->TakeOwnership(pDevice, Exclusive);};
  void STDMETHODCALLTYPE ReleaseOwnership(void)
    {dbg("dxgo: ReleaseOwnership");return dxgo->ReleaseOwnership();};
  HRESULT STDMETHODCALLTYPE GetGammaControlCapabilities(DXGI_GAMMA_CONTROL_CAPABILITIES *pGammaCaps)
    {dbg("dxgo: GetGammaControlCapabilities");return dxgo->GetGammaControlCapabilities(pGammaCaps);};
  HRESULT STDMETHODCALLTYPE SetGammaControl(const DXGI_GAMMA_CONTROL *pArray)
    {dbg("dxgo: SetGammaControl");return dxgo->SetGammaControl(pArray);};
  HRESULT STDMETHODCALLTYPE GetGammaControl(DXGI_GAMMA_CONTROL *pArray)
    {dbg("dxgo: GetGammaControl");return dxgo->GetGammaControl(pArray);};
  HRESULT STDMETHODCALLTYPE SetDisplaySurface(IDXGISurface *pScanoutSurface)
    {dbg("dxgo: SetDisplaySurface");return dxgo->SetDisplaySurface(pScanoutSurface);};
  HRESULT STDMETHODCALLTYPE GetDisplaySurfaceData(IDXGISurface *pDestination)
    {dbg("dxgo: GetDisplaySurfaceData");return dxgo->GetDisplaySurfaceData(pDestination);};
  HRESULT STDMETHODCALLTYPE GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats)
    {dbg("dxgo: GetFrameStatistics");return dxgo->GetFrameStatistics(pStats);};

  IDXGIOutput* getReal() {dbg("Get real output 0x%08X", &dxgo);return dxgo;};

private:
  IDXGIOutput *dxgo;
  IDXGIAdapter1New *parent;
};

#endif
