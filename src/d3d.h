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

#ifndef __D3D_H__
#define __D3D_H__

#include <d3d9.h>
#include "helper.h"

#include <stdio.h>
#include <vector>
#include <list>

#undef nil
#undef dbgf

// USE_D3DEX Disabled: Do not use Direct3D9Ex
// For debug only, SoftTH device wont work with it
//#define USE_D3DEX 1 // Removed by CJR for SDK 8.1 - 9 Aug 2015

// Debug settings for non-ex device (enable various Ex device emulation features)
#define MANAGE_DEBUG_IB         0
#define MANAGE_DEBUG_VOLTEXTURE 0
#define MANAGE_DEBUG_TEXTURE    0
#define MANAGE_DEBUG_SURFACE    0
#define MANAGE_DEBUG_VB         0

//#define ENABLE_POSTPROCESS

interface IDirect3D9New;

extern IDirect3D9New *d3dhNew;  // Direct3D handle passed to application (IDirect3D9New)
extern std::list<IDirect3D9New*> listD3D9New;  // List of created D3D9 handles so they can be released if app doesnt

void releaseHangingD3D9New();

#define nil if(0)
#define dbgf if(0)
#define nulb if(0)
//#define dbgf dbg
//#define nil dbg


#if USE_D3DEX || MANAGE_DEBUG_SURFACE
  // Change pool from managed to default (managed not supported in d3d9ex)
  #define CHECKPOOL(x) if(x==D3DPOOL_MANAGED)x=D3DPOOL_DEFAULT;
#else
  #define CHECKPOOL(x)
#endif

#if USE_D3DEX
interface IDirect3DDevice9New : public IDirect3DDevice9Ex
#else
interface IDirect3DDevice9New : public IDirect3DDevice9
#endif
{
#if USE_D3DEX
  IDirect3DDevice9New(IDirect3DDevice9Ex *device, IDirect3D9Ex *direct3D);
#else
  IDirect3DDevice9New(IDirect3DDevice9 *device, IDirect3D9 *direct3D);
#endif
  ~IDirect3DDevice9New() {dbg("~IDirect3DDevice9New");isEx=false;};

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) {
    dbgf("dev: QueryInterface %08X-%04X-%04X-%08X (%d)", riid.Data1, riid.Data2, riid.Data3, riid.Data4, isEx);
    if(riid == IID_IDirect3DDevice9Ex && !isEx) {
      // Pretend we are not an Ex device
      dbgf("dev: QueryInterface IID_IDirect3DDevice9Ex: E_NOINTERFACE");
      return E_NOINTERFACE;
    }
    if(riid == IID_IDirect3DDevice9 || riid == IID_IDirect3DDevice9Ex) {
      dbgf("dev: QueryInterface %s", matchRiid(riid));
      this->AddRef();
      *ppvObj = this;
      return S_OK;
    }
    HRESULT ret = dev->QueryInterface(riid, ppvObj);
    dbgf("dev: QueryInterface ret: %s", getD3DError(ret));
    return ret;
  }

  STDMETHOD_(ULONG,AddRef)(THIS)
  {dbgf("dev: AddRef");return dev->AddRef();};
  STDMETHOD_(ULONG,Release)(THIS) {
    dbgf("dev: Release %08X", this);
    ULONG ret = dev->Release();
    if(ret == 0)
      delete(this);
    dbgf("dev: Release result: %d", ret);
    return ret;
  }
    //{dbgf("dev: Release");return dev->Release();};

  STDMETHOD(TestCooperativeLevel)(THIS)
    {dbgf("dev: TestCooperativeLevel");return dev->TestCooperativeLevel();};
  STDMETHOD_(UINT, GetAvailableTextureMem)(THIS)
    {dbgf("dev: GetAvailableTextureMem");return dev->GetAvailableTextureMem();};
  STDMETHOD(EvictManagedResources)(THIS)
    {dbgf("dev: EvictManagedResources");return dev->EvictManagedResources();};
  STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9)
    ;//{dbgf("dev: GetDirect3D");*ppD3D9 = (IDirect3D9*)d3dhNew;d3dhNew->AddRef();return D3D_OK;};
    //{dbgf("dev: GetDirect3D");return dev->GetDirect3D(ppD3D9);};
  STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps)
    {dbgf("dev: GetDeviceCaps");return dev->GetDeviceCaps(pCaps);};
  STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode)
    {dbgf("dev: GetDisplayMode");return dev->GetDisplayMode(iSwapChain, pMode);};
  STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters)
    {dbgf("dev: GetCreationParameters");return dev->GetCreationParameters(pParameters);};
  STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
    {dbgf("dev: SetCursorProperties");return dev->SetCursorProperties(XHotSpot, YHotSpot, OriginalFromNewSurface(pCursorBitmap));};
  STDMETHOD_(void, SetCursorPosition)(THIS_ int X,int Y,DWORD Flags)
    {dbgf("dev: SetCursorPosition %dx%d %d", X , Y, Flags);return dev->SetCursorPosition(X, Y, Flags);};
  STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow)
    {dbgf("dev: ShowCursor");return dev->ShowCursor(bShow);};

  // __asm {nop} must be here... ??? Steam overlay places hook here maybe?
  STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)
  {
#ifndef _WIN64
    __asm {nop};
#endif
    dbgf("dev: CreateAdditionalSwapChain");return dev->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);};

  STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
    {dbgf("dev: GetSwapChain");return dev->GetSwapChain(iSwapChain, pSwapChain);};
  STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS)
    {dbgf("dev: GetNumberOfSwapChains");return dev->GetNumberOfSwapChains();};
  STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters)
    {dbg("dev: Reset");return dev->Reset(pPresentationParameters);};
  STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
    {nil("dev: Present");frameCounter++;return dev->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);};
  STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
    {
      dbg("dev: GetBackBuffer %d %d", iSwapChain, iBackBuffer);
      return dev->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
  };
  STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
    {dbgf("dev: GetRasterStatus");return dev->GetRasterStatus(iSwapChain, pRasterStatus);};
  STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs)
    {dbgf("dev: SetDialogBoxMode");return dev->SetDialogBoxMode(bEnableDialogs);};
  STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
    {dbgf("dev: SetGammaRamp");return dev->SetGammaRamp(iSwapChain, Flags, pRamp);};
  STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp)
    {dbgf("dev: GetGammaRamp");return dev->GetGammaRamp(iSwapChain, pRamp);};
  STDMETHOD(CreateTexture)(THIS_ UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
    ;//{dbgf("dev: CreateTexture");CHECKPOOL(Pool);return dev->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);};
  STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
    ;//{dbgf("dev: CreateVolumeTexture");CHECKPOOL(Pool);return dev->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);};
  STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
    ;//{dbgf("dev: CreateCubeTexture");CHECKPOOL(Pool);return dev->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);};
  STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
    ;//{dbgf("dev: CreateVertexBuffer len%d fvf%d %s %s (%d)", Length, FVF, getPool(Pool), getUsage(Usage), pSharedHandle);CHECKPOOL(Pool);return dev->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);};
  STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
    ;//{dbgf("dev: CreateIndexBuffer");CHECKPOOL(Pool);return dev->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);};
  STDMETHOD(CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
    {dbgf("dev: CreateRenderTarget");return dev->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);};
  STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
    {dbgf("dev: CreateDepthStencilSurface");return dev->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);};
  STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
    {dbgf("dev: UpdateSurface");return dev->UpdateSurface(OriginalFromNewSurface(pSourceSurface), pSourceRect, OriginalFromNewSurface(pDestinationSurface), pDestPoint);};
  STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
    {dbgf("dev: UpdateTexture");return dev->UpdateTexture(OriginalFromNewTexture(pSourceTexture), OriginalFromNewTexture(pDestinationTexture));};
  STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
    {dbgf("dev: GetRenderTargetData");return dev->GetRenderTargetData(OriginalFromNewSurface(pRenderTarget), OriginalFromNewSurface(pDestSurface));};
  STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain,IDirect3DSurface9* pDestSurface)
    {dbgf("dev: GetFrontBufferData");return dev->GetFrontBufferData(iSwapChain, pDestSurface);};
  STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
    {dbgf("dev: StretchRect");return dev->StretchRect(OriginalFromNewSurface(pSourceSurface), pSourceRect, OriginalFromNewSurface(pDestSurface), pDestRect, Filter);};
  STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
    {dbgf("dev: ColorFill");return dev->ColorFill(OriginalFromNewSurface(pSurface), pRect, color);};
  STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
    {dbgf("dev: CreateOffscreenPlainSurface %dx%d %s %s", Width, Height, getMode(Format), getPool(Pool));CHECKPOOL(Pool);return dev->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);};
  STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
    {dbgf("dev: SetRenderTarget %d", RenderTargetIndex);
    HRESULT ret =  dev->SetRenderTarget(RenderTargetIndex, OriginalFromNewSurface(pRenderTarget));
    dbgf("dev: SetRenderTarget return");
    return ret;
  };
  STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
    {dbgf("dev: GetRenderTarget");return dev->GetRenderTarget(RenderTargetIndex, ppRenderTarget);};
  STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil)
    {nil("dev: SetDepthStencilSurface");return dev->SetDepthStencilSurface(OriginalFromNewSurface(pNewZStencil));};
  STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface)
    {nil("dev: GetDepthStencilSurface");return dev->GetDepthStencilSurface(ppZStencilSurface);};
  STDMETHOD(BeginScene)(THIS)
    {nil("dev: BeginScene");return dev->BeginScene();};
  STDMETHOD(EndScene)(THIS)
    {nil("dev: EndScene");return dev->EndScene();};
  STDMETHOD(Clear)(THIS_ DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
    {nil("dev: Clear");return dev->Clear(Count, pRects, Flags, Color, Z, Stencil);};
  STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
    {nil("dev: SetTransform");return dev->SetTransform(State, pMatrix);};
  STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
    {nil("dev: GetTransform");return dev->GetTransform(State, pMatrix);};
  STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE Transform,CONST D3DMATRIX* Matrix)
    {dbgf("dev: MultiplyTransform");return dev->MultiplyTransform(Transform, Matrix);};
  STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT9* pViewport)
    {nil("dev: SetViewport");return dev->SetViewport(pViewport);};
  STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport)
    {nil("dev: GetViewport");return dev->GetViewport(pViewport);};
  STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9* pMaterial)
    {nil("dev: SetMaterial");return dev->SetMaterial(pMaterial);};
  STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial)
    {dbgf("dev: GetMaterial");return dev->GetMaterial(pMaterial);};
  STDMETHOD(SetLight)(THIS_ DWORD Index,CONST D3DLIGHT9* Light)
    {nil("dev: SetLight");return dev->SetLight(Index, Light);};
  STDMETHOD(GetLight)(THIS_ DWORD Index,D3DLIGHT9* Light)
    {dbgf("dev: GetLight");return dev->GetLight(Index, Light);};
  STDMETHOD(LightEnable)(THIS_ DWORD Index,BOOL Enable)
    {nil("dev: LightEnable");return dev->LightEnable(Index, Enable);};
  STDMETHOD(GetLightEnable)(THIS_ DWORD Index,BOOL* pEnable)
    {dbgf("dev: GetLightEnable");return dev->GetLightEnable(Index, pEnable);};
  STDMETHOD(SetClipPlane)(THIS_ DWORD Index,CONST float* pPlane)
    {nil("dev: SetClipPlane");return dev->SetClipPlane(Index, pPlane);};
  STDMETHOD(GetClipPlane)(THIS_ DWORD Index,float* pPlane)
    {dbgf("dev: GetClipPlane");return dev->GetClipPlane(Index, pPlane);};
  STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value)
    {nulb("dev: SetRenderState");return dev->SetRenderState(State, Value);};
  STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue)
    {dbgf("dev: GetRenderState");return dev->GetRenderState(State, pValue);};
  STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
    {dbgf("dev: CreateStateBlock");return dev->CreateStateBlock(Type, ppSB);};
  STDMETHOD(BeginStateBlock)(THIS)
    {dbgf("dev: BeginStateBlock");return dev->BeginStateBlock();};
  STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB)
    {dbgf("dev: EndStateBlock");return dev->EndStateBlock(ppSB);};
  STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS9* pClipStatus)
    {dbgf("dev: SetClipStatus");return dev->SetClipStatus(pClipStatus);};
  STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus)
    {dbgf("dev: GetClipStatus");return dev->GetClipStatus(pClipStatus);};
  STDMETHOD(GetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9** ppTexture)
    {dbgf("dev: GetTexture %d", Stage);
    HRESULT ret = dev->GetTexture(Stage, ppTexture);
    if(ret == D3D_OK)
      *ppTexture = NewTextureFromOriginal(*ppTexture);
    return ret;
    };
    //{dbgf("dev: GetTexture %d", Stage);return dev->GetTexture(Stage, ppTexture);};
  STDMETHOD(SetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9* pTexture)
    {nil("dev: SetTexture %d %08X", Stage, pTexture);/*releaseManagedSysmemTexture(pTexture);*/return dev->SetTexture(Stage, OriginalFromNewTexture(pTexture));};
  STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
    {dbgf("dev: GetTextureStageState");return dev->GetTextureStageState(Stage, Type, pValue);};
  STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
    {nil("dev: SetTextureStageState");return dev->SetTextureStageState(Stage, Type, Value);};
  STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
    {dbgf("dev: GetSamplerState");return dev->GetSamplerState(Sampler, Type, pValue);};
  STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
    {nulb("dev: SetSamplerState");return dev->SetSamplerState(Sampler, Type, Value);};
  STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses)
    {dbgf("dev: ValidateDevice");return dev->ValidateDevice(pNumPasses);};
  STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
    {dbgf("dev: SetPaletteEntries");return dev->SetPaletteEntries(PaletteNumber, pEntries);};
  STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber,PALETTEENTRY* pEntries)
    {dbgf("dev: GetPaletteEntries");return dev->GetPaletteEntries(PaletteNumber, pEntries);};
  STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber)
    {dbgf("dev: SetCurrentTexturePalette");return dev->SetCurrentTexturePalette(PaletteNumber);};
  STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber)
    {dbgf("dev: GetCurrentTexturePalette");return dev->GetCurrentTexturePalette(PaletteNumber);};
  STDMETHOD(SetScissorRect)(THIS_ CONST RECT* pRect)
    {dbgf("dev: SetScissorRect");return dev->SetScissorRect(pRect);};
  STDMETHOD(GetScissorRect)(THIS_ RECT* pRect)
    {dbgf("dev: GetScissorRect");return dev->GetScissorRect(pRect);};
  STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware)
    {dbgf("dev: SetSoftwareVertexProcessing");return dev->SetSoftwareVertexProcessing(bSoftware);};
  STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS)
    {dbgf("dev: GetSoftwareVertexProcessing");return dev->GetSoftwareVertexProcessing();};
  STDMETHOD(SetNPatchMode)(THIS_ float nSegments)
    {dbgf("dev: SetNPatchMode");return dev->SetNPatchMode(nSegments);};
  STDMETHOD_(float, GetNPatchMode)(THIS)
    {dbgf("dev: GetNPatchMode");return dev->GetNPatchMode();};
  STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
    {nil("dev: DrawPrimitive");return dev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);};
  STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
    //{nil("dev: DrawIndexedPrimitive");if(GetKeyState('O') >= 0) return dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount); else return D3D_OK;};
    {nil("dev: DrawIndexedPrimitive");return dev->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);};
  STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
    {dbgf("dev: DrawPrimitiveUP");return dev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);};
  STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
    {dbgf("dev: DrawIndexedPrimitiveUP");return dev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);};
  STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
    {dbgf("dev: ProcessVertices");return dev->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);};
  STDMETHOD(CreateVertexDeclaration)(THIS_ CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
    {dbgf("dev: CreateVertexDeclaration");return dev->CreateVertexDeclaration(pVertexElements, ppDecl);};
  STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl)
    {nil("dev: SetVertexDeclaration 0x%08X", pDecl);return dev->SetVertexDeclaration(pDecl);};
  STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl)
    {dbgf("dev: GetVertexDeclaration");return dev->GetVertexDeclaration(ppDecl);};
  STDMETHOD(SetFVF)(THIS_ DWORD FVF)
    {nil("dev: SetFVF");return dev->SetFVF(FVF);};
  STDMETHOD(GetFVF)(THIS_ DWORD* pFVF)
    {dbgf("dev: GetFVF");return dev->GetFVF(pFVF);};
  STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
    {dbgf("dev: CreateVertexShader");return dev->CreateVertexShader(pFunction, ppShader);};
  STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader)
    {nil("dev: SetVertexShader");return dev->SetVertexShader(pShader);};
  STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader)
    {dbgf("dev: GetVertexShader");return dev->GetVertexShader(ppShader);};
  STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
    {nil("dev: SetVertexShaderConstantF");return dev->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);};
  STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount)
    {nil("dev: GetVertexShaderConstantF");return dev->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);};
  STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
    {nil("dev: SetVertexShaderConstantI");return dev->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);};
  STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount)
    {nil("dev: GetVertexShaderConstantI");return dev->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);};
  STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT BoolCount)
    {nil("dev: SetVertexShaderConstantB");return dev->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);};
  STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
    {nil("dev: GetVertexShaderConstantB");return dev->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);};
  STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
    {nil("dev: SetStreamSource");return dev->SetStreamSource(StreamNumber, OriginalFromNewVBuffer(pStreamData), OffsetInBytes, Stride);};
  STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride)
    {dbgf("dev: GetStreamSource");return dev->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride);};
  STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT Setting)
    {dbgf("dev: SetStreamSourceFreq");return dev->SetStreamSourceFreq(StreamNumber, Setting);};
  STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT* pSetting)
    {dbgf("dev: GetStreamSourceFreq");return dev->GetStreamSourceFreq(StreamNumber, pSetting);};
  STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData)
    {nil("dev: SetIndices");return dev->SetIndices(OriginalFromNewIBuffer(pIndexData));};
  STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData)
    {dbg("dev: WARNING: GetIndices, Not implemented");return dev->GetIndices(ppIndexData);};
  STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
    {dbgf("dev: CreatePixelShader");return dev->CreatePixelShader(pFunction, ppShader);};
  STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader)
    {nil("dev: SetPixelShader");return dev->SetPixelShader(pShader);};
  STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader)
    {dbgf("dev: GetPixelShader");return dev->GetPixelShader(ppShader);};
  STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
    {nil("dev: SetPixelShaderConstantF");return dev->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);};
  STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount)
    {nil("dev: GetPixelShaderConstantF");return dev->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);};
  STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
    {nil("dev: SetPixelShaderConstantI");return dev->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);};
  STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount)
    {nil("dev: GetPixelShaderConstantI");return dev->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);};
  STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister,CONST BOOL* pConstantData,UINT BoolCount)
    {nil("dev: SetPixelShaderConstantB");return dev->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);};
  STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
    {nil("dev: GetPixelShaderConstantB");return dev->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);};
  STDMETHOD(DrawRectPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
    {dbgf("dev: DrawRectPatch");return dev->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);};
  STDMETHOD(DrawTriPatch)(THIS_ UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
    {dbgf("dev: DrawTriPatch");return dev->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);};
  STDMETHOD(DeletePatch)(THIS_ UINT Handle)
    {dbgf("dev: DeletePatch");return dev->DeletePatch(Handle);};
  STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
    {dbgf("dev: CreateQuery");return dev->CreateQuery(Type, ppQuery);};
#if USE_D3DEX
  STDMETHOD(SetConvolutionMonoKernel)(THIS_ UINT width,UINT height,float* rows,float* columns)
    {dbgf("dev: SetConvolutionMonoKernel");return dev->SetConvolutionMonoKernel(width, height, rows, columns);};
  STDMETHOD(ComposeRects)(THIS_ IDirect3DSurface9* pSrc,IDirect3DSurface9* pDst,IDirect3DVertexBuffer9* pSrcRectDescs,UINT NumRects,IDirect3DVertexBuffer9* pDstRectDescs,D3DCOMPOSERECTSOP Operation,int Xoffset,int Yoffset)
    {dbgf("dev: ComposeRects");return dev->ComposeRects(pSrc, pDst, pSrcRectDescs, NumRects, pDstRectDescs, Operation, Xoffset, Yoffset);};
  STDMETHOD(PresentEx)(THIS_ CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags)
    {dbgf("dev: PresentEx");frameCounter++;return dev->PresentEx(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);};
  STDMETHOD(GetGPUThreadPriority)(THIS_ INT* pPriority)
    {dbgf("dev: GetGPUThreadPriority");return dev->GetGPUThreadPriority(pPriority);};
  STDMETHOD(SetGPUThreadPriority)(THIS_ INT Priority)
    {dbgf("dev: SetGPUThreadPriority");return dev->SetGPUThreadPriority(Priority);};
  STDMETHOD(WaitForVBlank)(THIS_ UINT iSwapChain)
    {dbgf("dev: WaitForVBlank");return dev->WaitForVBlank(iSwapChain);};
  STDMETHOD(CheckResourceResidency)(THIS_ IDirect3DResource9** pResourceArray,UINT32 NumResources)
    {dbgf("dev: CheckResourceResidency");return dev->CheckResourceResidency(pResourceArray, NumResources);};
  STDMETHOD(SetMaximumFrameLatency)(THIS_ UINT MaxLatency)
    {dbgf("dev: SetMaximumFrameLatency");return dev->SetMaximumFrameLatency(MaxLatency);};
  STDMETHOD(GetMaximumFrameLatency)(THIS_ UINT* pMaxLatency)
    {dbgf("dev: GetMaximumFrameLatency");return dev->GetMaximumFrameLatency(pMaxLatency);};
  STDMETHOD(CheckDeviceState)(THIS_ HWND hDestinationWindow)
    {dbgf("dev: CheckDeviceState");return dev->CheckDeviceState(hDestinationWindow);};
  STDMETHOD(CreateRenderTargetEx)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage)
    {dbgf("dev: CreateRenderTargetEx");return dev->CreateRenderTargetEx(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle, Usage);};
  STDMETHOD(CreateOffscreenPlainSurfaceEx)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage)
    {dbgf("dev: CreateOffscreenPlainSurfaceEx");CHECKPOOL(Pool);return dev->CreateOffscreenPlainSurfaceEx(Width, Height, Format, Pool, ppSurface, pSharedHandle, Usage);};
  STDMETHOD(CreateDepthStencilSurfaceEx)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle,DWORD Usage)
    {dbgf("dev: CreateDepthStencilSurfaceEx");return dev->CreateDepthStencilSurfaceEx(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle, Usage);};
  STDMETHOD(ResetEx)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,D3DDISPLAYMODEEX *pFullscreenDisplayMode)
    {dbgf("dev: ResetEx");return dev->ResetEx(pPresentationParameters, pFullscreenDisplayMode);};
  STDMETHOD(GetDisplayModeEx)(THIS_ UINT iSwapChain,D3DDISPLAYMODEEX* pMode,D3DDISPLAYROTATION* pRotation)
    {dbgf("dev: GetDisplayModeEx");return dev->GetDisplayModeEx(iSwapChain, pMode, pRotation);};
#endif

  DWORD getFrameNumber() const {return frameCounter;};
  void setEx(bool val) {isEx = val;};
private:
#if USE_D3DEX
  IDirect3DDevice9Ex *dev;
  IDirect3D9Ex *d3d;
#else
  IDirect3DDevice9 *dev;
  IDirect3D9 *d3d;
#endif
  bool isEx; // Did app ask for Ex device?
  DWORD frameCounter; // Frame number (increased at present)

  IDirect3DBaseTexture9* NewTextureFromOriginal(IDirect3DBaseTexture9* tex); // Return new texture from original
  IDirect3DBaseTexture9* OriginalFromNewTexture(IDirect3DBaseTexture9* tex); // Return original handle of texture
  IDirect3DSurface9* OriginalFromNewSurface(IDirect3DSurface9* tex); // Return original handle of surface
  IDirect3DIndexBuffer9* OriginalFromNewIBuffer(IDirect3DIndexBuffer9* buf);  // Return original handle of index buffer
  IDirect3DVertexBuffer9* OriginalFromNewVBuffer(IDirect3DVertexBuffer9* buf);  // Return original handle of vertex buffer
  void releaseManagedSysmemTexture(IDirect3DBaseTexture9* tex); // Release managed surface sysmem texture (late release with PERSISTENT_DYNAMIC_TEXTURES)

  friend interface IDirect3DDevice9SoftTH;
};

//#undef dbgf
//#define dbgf if(0)

// New IDirect3D9 interface, maps to Direct3D9Ex
interface IDirect3D9New : public IDirect3D9
{
public:
  IDirect3D9New(IDirect3D9Ex *direct3D) {
    d3d = direct3D;
    updateRefreshRates();
    listD3D9New.push_back(this);
  };

  ~IDirect3D9New() {
    dbg("~IDirect3D9New");
    listD3D9New.remove(this);
    //releaseHanging();
  };

  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj)
    {dbg("d3d: QueryInterface");return d3d->QueryInterface(riid, ppvObj);}
  STDMETHOD_(ULONG,AddRef)(THIS)
    {dbg("d3d: AddRef");return d3d->AddRef();}
  STDMETHOD_(ULONG,Release)(THIS) {
    //dbg("d3d: Release...");
    ULONG r = d3d->Release();
    if(r == 0)
      delete(this);
    dbg("d3d: Release: %d", r);
    return r;
  }

  STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction)
    {dbgf("d3d: RegisterSoftwareDevice");return d3d->RegisterSoftwareDevice(pInitializeFunction);}
  STDMETHOD_(UINT, GetAdapterCount)(THIS)
    ;//{dbg("d3d: GetAdapterCount");return d3d->GetAdapterCount();}
  STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
    ;//{dbg("d3d: GetAdapterIdentifier");return d3d->GetAdapterIdentifier(Adapter, Flags, pIdentifier);}
  STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter,D3DFORMAT Format)
    ;//{dbg("d3d: GetAdapterModeCount");return d3d->GetAdapterModeCount(Adapter, Format);}
  STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
    ;//{dbg("d3d: EnumAdapterModes");return d3d->EnumAdapterModes(Adapter, Format, Mode, pMode);}
  STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter,D3DDISPLAYMODE* pMode)
    ;//{dbg("d3d: WARNING: GetAdapterDisplayMode, not implemented!");return d3d->GetAdapterDisplayMode(Adapter, pMode);}
  STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
    {dbgf("d3d: CheckDeviceType %d %s %s %d",  Adapter, getMode(AdapterFormat), getMode(BackBufferFormat), bWindowed);return d3d->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);}
  STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
    {dbgf("d3d: CheckDeviceFormat %d %s %s %s", Adapter, getMode(AdapterFormat), getUsage(Usage), getMode(CheckFormat));return d3d->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);}
  STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
    {dbgf("d3d: CheckDeviceMultiSampleType");return d3d->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);}
  STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
    {dbgf("d3d: CheckDepthStencilMatch");return d3d->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);}
  STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
    {dbgf("d3d: CheckDeviceFormatConversion");return d3d->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);}
  STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
    {dbgf("d3d: GetDeviceCaps %d", Adapter);return d3d->GetDeviceCaps(Adapter, DeviceType, pCaps);}
  STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter)
    {dbg("d3d: GetAdapterMonitor %d", Adapter);return d3d->GetAdapterMonitor(Adapter);}
  STDMETHOD(CreateDevice)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pp,IDirect3DDevice9** ppReturnedDeviceInterface)
    ;//{dbg("d3d: CreateDevice");return d3d->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pp, ppReturnedDeviceInterface);}

  // D3D9Ex interfaces:
  STDMETHOD_(UINT, GetAdapterModeCountEx)(THIS_ UINT Adapter,CONST D3DDISPLAYMODEFILTER* pFilter )
    {dbg("d3dex: WARNING: GetAdapterModeCountEx, not implemented!");return d3d->GetAdapterModeCountEx(Adapter, pFilter);}
  STDMETHOD(EnumAdapterModesEx)(THIS_ UINT Adapter,CONST D3DDISPLAYMODEFILTER* pFilter,UINT Mode,D3DDISPLAYMODEEX* pMode)
    {dbg("d3dex: WARNING: EnumAdapterModesEx, not implemented!");return d3d->EnumAdapterModesEx(Adapter, pFilter, Mode, pMode);}
  STDMETHOD(GetAdapterDisplayModeEx)(THIS_ UINT Adapter,D3DDISPLAYMODEEX* pMode,D3DDISPLAYROTATION* pRotation)
    {dbg("d3dex: WARNING: GetAdapterDisplayModeEx, not implemented!");return d3d->GetAdapterDisplayModeEx(Adapter, pMode, pRotation);}
  STDMETHOD(CreateDeviceEx)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,D3DDISPLAYMODEEX* pFullscreenDisplayMode,IDirect3DDevice9Ex** ppReturnedDeviceInterface)
  ;//  {dbg("d3dex: WARNING: CreateDeviceEx, not implemented!"); return 0;};
  STDMETHOD(GetAdapterLUID)(THIS_ UINT Adapter,LUID * pLUID)
    {dbgf("d3dex: GetAdapterLUID!");return d3d->GetAdapterLUID(Adapter, pLUID);}
private:
  IDirect3D9Ex *d3d;

  void updateRefreshRates();
  static std::vector<int> refreshRates;

  void releaseHanging();
  std::list<IDirect3DDevice9SoftTH*> listD3D9DeviceSoftTH;

public:
  void destroyed(IDirect3DDevice9SoftTH *ddev);
};



#endif
