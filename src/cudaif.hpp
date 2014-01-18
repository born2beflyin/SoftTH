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

#ifndef __CUDAIF__H_
#define __CUDAIF__H_

#ifdef USE_NEW_CUDA

#include <d3d9.h>

#include "helper.h"

#include <cuda.h>
#include <cudaD3D9.h>

//#define dbg printf

//#define CUDACALL(x)	if((x) != CUDA_SUCCESS) dbg("WARNING! CUDACALL fail line %d, %s\n", __LINE__, __FILE__);

static char *CudaGetError(CUresult r)
{
  switch(r)
  {
    case CUDA_SUCCESS: return "CUDA_SUCCESS";
    case CUDA_ERROR_DEINITIALIZED: return "CUDA_ERROR_DEINITIALIZED";
    case CUDA_ERROR_NOT_INITIALIZED: return "CUDA_ERROR_NOT_INITIALIZED";
    case CUDA_ERROR_INVALID_CONTEXT: return "CUDA_ERROR_INVALID_CONTEXT";
    case CUDA_ERROR_INVALID_VALUE: return "CUDA_ERROR_INVALID_VALUE";
    case CUDA_ERROR_INVALID_HANDLE: return "CUDA_ERROR_INVALID_HANDLE";
    case CUDA_ERROR_NOT_MAPPED: return "CUDA_ERROR_NOT_MAPPED ";
    case CUDA_ERROR_NOT_MAPPED_AS_POINTER: return "CUDA_ERROR_NOT_MAPPED_AS_POINTER";
    case CUDA_ERROR_ALREADY_MAPPED: return "CUDA_ERROR_ALREADY_MAPPED";
    case CUDA_ERROR_UNKNOWN: return "CUDA_ERROR_UNKNOWN";
    default: return "UNKNOWN";
  }
}

#define CUDACALL(x) {CUresult __r = (x);if(__r != CUDA_SUCCESS) dbg("WARNING! CUDACALL fail line %d, %s (%s)", __LINE__, __FILE__, CudaGetError(__r));}

#define MAX_CUDA_THREADS  16

// A cuda "thread"
class cudaThread
{
public:
  cudaThread(IDirect3DDevice9 *ddev, IDirect3DResource9 *res);
  ~cudaThread();

  CUcontext GetContext() const {return _ctx;};
  CUgraphicsResource GetResource() const {return _tex;};

private:
  CUcontext _ctx; // Cuda context for D3D device
  CUgraphicsResource _tex; // D3D9 texture
};

// Interface for using cuda threads
class cudaIf
{
public:
  cudaIf() {init();};
  ~cudaIf() {deinit();};

  int createThread(IDirect3DDevice9 *ddev, IDirect3DResource9 *res);
  bool copyPeer(int dstID, int srcID);

private:
  void init();
  void deinit();

  cudaThread *_threads[MAX_CUDA_THREADS];
  int _numThreads;
};

#endif 

#endif 