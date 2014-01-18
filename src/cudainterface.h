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

#ifndef __CUDAINTERFACE__H_
#define __CUDAINTERFACE__H_

#ifdef USE_CUDA

#include <stdio.h>
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <process.h>

#include <cuda.h>
#include <builtin_types.h>
#include <cuda_runtime_api.h>
#include <cuda_d3d9_interop.h>

#include <vector>

#define CUDACALL(x)	{int tr = x; if(tr != cudaSuccess) dbg("WARNING! CUDA sucks at line %d, file %s, thread 0x%04X <%s(%d)>", __LINE__, __FILE__, GetCurrentThreadId(), cudaGetErrorString(cudaError(tr)), tr);}
//#define CUDACALL(x)	{dbg("CUDACALL: %s", #x);int tr = x; if(tr != cudaSuccess) dbg("WARNING! cuda call fail line %d, %s <%s %d>", __LINE__, __FILE__, cudaGetErrorString(cudaError(tr)), tr);}

class directCuda;
typedef void* (DIRECTCUDATASK)(directCuda*, void* param);

class directCuda {
public:
	directCuda(LPDIRECT3D9 pd3d, LPDIRECT3DDEVICE9 pdev, char *pname);
	~directCuda();

	bool setTask(DIRECTCUDATASK* ptask, void *param);
	bool directCuda::taskBegin(void);
	void* directCuda::taskWait(void);
	void* directCuda::taskExecute(void);
	bool directCuda::beginThread(void);
	int directCuda::addSurface(LPDIRECT3DSURFACE9 surf);
	LPDIRECT3DSURFACE9 directCuda::getSurface(DWORD id);
	int directCuda::getID(LPDIRECT3DSURFACE9 psurf);
	bool directCuda::validSurface(LPDIRECT3DSURFACE9 psurf);

//private: PFFT
	HANDLE thread;						// Handle to local thread
	LPDIRECT3DDEVICE9 dev;				// Device handle
	LPDIRECT3D9 d3d;					// Direct3D handle
	std::vector<LPDIRECT3DSURFACE9> surf;	// Vector of surfaces used by this thread	
	int devID;			// CUDA device ID
	char name[256];			// A name for this thread for debugging
	bool alive;			// if true main loop is active, when set false, abort main loop
	HANDLE go, done;	// Events for beginning task, when task done
	DIRECTCUDATASK *task;	// Task to execute
	void *taskRetval;		// Task return value
	void *taskParams;		// Task parameters

	size_t *size;	// Surface sizes
	DWORD *dData;	// Surface data pointers
	void *memHost;	// "helper" slot for task-allocated memory (freed upon destroy with cudaFreeHost)

	static void directCuda::threadInitialize(void *poo);
	void __cdecl cudaThread(void);
};

bool isCudaEnabled(void);
bool cudaCheckCudaCompatibility(LPDIRECT3D9 d3dh);

#endif

#endif

