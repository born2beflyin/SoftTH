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

#ifdef USE_NEW_CUDA

#include "cudaif.hpp"

#pragma comment(lib, "cuda") // TODO: use delay load

// Create a new "thread", return ID of it
int cudaIf::createThread(IDirect3DDevice9 *ddev, IDirect3DResource9 *res)
{
  if(_numThreads > MAX_CUDA_THREADS)
  {
    dbg("cudaIf::createThread maximum threads reached");
    return -1;
  }

  _threads[_numThreads] = new cudaThread(ddev, res);
  _numThreads++;
  return _numThreads-1;
}

bool cudaIf::copyPeer(int dstID, int srcID)
{
  cudaThread *dst = _threads[dstID];
  cudaThread *src = _threads[srcID];
  if(!dst || !src)
  {
    dbg("cudaIf::copyPeer invalid ID");
    return false;
  }

  // map destination resource
  CUgraphicsResource dr = dst->GetResource();
  CUdeviceptr dptr = NULL;
  CUarray darr = NULL;
  size_t dsize = 0;
  CUDACALL( cuCtxSetCurrent(dst->GetContext()) );
  
  cuGraphicsResourceSetMapFlags(dr, 2);
  //cuGraphicsResourceSetMapFlags(dr, CU_GRAPHICS_MAP_RESOURCE_FLAGS_WRITEDISCARD); // Write-only
  CUDACALL( cuGraphicsMapResources(1, &dr, 0) );
  CUDACALL( cuGraphicsSubResourceGetMappedArray(&darr, dr, 0, 0) );
  //CUDACALL( cuGraphicsResourceGetMappedPointer(&dptr, &dsize, dr) );

  // map source resource
  CUgraphicsResource sr = src->GetResource();
  CUdeviceptr sptr = NULL;
  CUarray sarr = NULL;
  size_t ssize = 0;
  CUDACALL( cuCtxSetCurrent(src->GetContext()) );
  cuGraphicsResourceSetMapFlags(dr, 1); // Read-only
  //cuGraphicsResourceSetMapFlags(dr, CU_GRAPHICS_MAP_RESOURCE_FLAGS_READONLY); // Read-only
  CUDACALL( cuGraphicsMapResources(1, &sr, 0) );
  CUDACALL( cuGraphicsSubResourceGetMappedArray(&sarr, sr, 0, 0) );
  //CUDACALL( cuGraphicsResourceGetMappedPointer(&sptr, &ssize, sr) );

  if(ssize != dsize)
  {
    dbg("cudaIf::copyPeer: Warning: Size mismatch (%d != %d)", dsize, ssize);
  }

  CUDA_MEMCPY3D_PEER param;
  param.srcXInBytes = 0;
  param.srcY = 0;
  param.srcZ = 0;
  param.srcLOD = 0;
  param.srcMemoryType = CU_MEMORYTYPE_ARRAY;
  param.srcArray = sarr;
  param.srcContext = src->GetContext();

  param.dstXInBytes = 0;
  param.dstY = 0;
  param.dstZ = 0;
  param.dstLOD = 0;
  param.dstMemoryType = CU_MEMORYTYPE_ARRAY;
  param.dstArray = darr;
  param.dstContext = dst->GetContext();

  param.WidthInBytes = 1920*4;
  param.Height = 1200;
  param.Depth = 1;

  CUDACALL( cuMemcpy3DPeer(&param) );

  // Perform copy
  //dbg("copy %d/%d bytes", ssize, dsize);
  //CUDACALL( cuMemcpyPeer(dptr, dst->GetContext(), sptr, src->GetContext(), min(ssize, dsize)) );

  

  // Unmap resources
  CUDACALL( cuCtxSetCurrent(dst->GetContext()) );
  CUDACALL( cuGraphicsUnmapResources(1, &dr, 0) );
  CUDACALL( cuCtxSetCurrent(src->GetContext()) );
  CUDACALL( cuGraphicsUnmapResources(1, &sr, 0) );

  return true;
}


void cudaIf::init()
{
  CUDACALL( cuInit(0) );

  for(int i=0;i<MAX_CUDA_THREADS;i++)
  {
    _threads[i] = NULL;
  }
  _numThreads = 0;
}

void cudaIf::deinit()
{
  for(int i=0;i<MAX_CUDA_THREADS;i++)
  {
    if(_threads[i]) delete _threads[i];
    _threads[i] = NULL;
  }
  _numThreads = 0;
}

cudaThread::cudaThread(IDirect3DDevice9 *ddev, IDirect3DResource9 *res)
{
  dbg("cudaThread()");
  _ctx = NULL;
  _tex = NULL;

  int cflags = CU_CTX_SCHED_AUTO;
  // CU_CTX_SCHED_SPIN
  // CU_CTX_SCHED_YIELD
  // CU_CTX_SCHED_BLOCKING_SYNC

  CUDACALL( cuD3D9CtxCreate(&_ctx, NULL, cflags, ddev) );
  // cuCtxSetCurrent...
  // cuCtxSetLimit... 
  // cuCtxSetCacheConfig...
 
  int rflags = CU_GRAPHICS_REGISTER_FLAGS_NONE;
  // CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST

  CUDACALL( cuGraphicsD3D9RegisterResource(&_tex, res, rflags) );
}

cudaThread::~cudaThread()
{
  dbg("~cudaThread()");
  if(_tex)
  {
    CUDACALL( cuGraphicsUnregisterResource(_tex) );
  }
  if(_ctx)
  {
    CUDACALL( cuCtxDestroy(_ctx) );
  }
}

#endif